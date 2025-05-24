#include "Number.h"
#include <fstream>
#include <iostream>
#include <iomanip>

NumberClassifier::NumberClassifier() {
    loadModel();
}

NumberClassifier::~NumberClassifier() = default;

void NumberClassifier::loadModel() {
    number_net_ = dnn::readNet("/home/zhao/Aimbot/model/mlp.onnx");
    ifstream label_file("/home/zhao/Aimbot/model/label.txt");
    string line;
    while (getline(label_file, line)) {
        class_names_.push_back(line);
    }
}

void NumberClassifier::extractAndClassifyNumbers(Mat& frame, vector<Armor>& armors) {
    const int warp_height = 28;
    const int light_length = 12;
    const int small_armor_width = 32;
    const int large_armor_width = 54;

    for (auto& armor : armors) {
        float aspect_ratio = (float)armor.rect.width / armor.rect.height;
        int warp_width = (aspect_ratio > 2.5f) ? large_armor_width : small_armor_width;

        const int top_light_y = (warp_height - light_length) / 2;
        const int bottom_light_y = top_light_y + light_length;

        Point2f src_points[4] = {
            Point2f(armor.rect.x, armor.rect.y + armor.rect.height),
            Point2f(armor.rect.x, armor.rect.y),
            Point2f(armor.rect.x + armor.rect.width, armor.rect.y),
            Point2f(armor.rect.x + armor.rect.width, armor.rect.y + armor.rect.height)
        };

        Point2f dst_points[4] = {
            Point2f(0, bottom_light_y),
            Point2f(0, top_light_y),
            Point2f(warp_width-1, top_light_y),
            Point2f(warp_width-1, bottom_light_y)
        };

        Mat warp_matrix = getPerspectiveTransform(src_points, dst_points);
        Mat warped_image;
        warpPerspective(frame, warped_image, warp_matrix, Size(warp_width, warp_height));

        Rect roi_rect((warp_width - 20)/2, 0, 20, warp_height);
        armor.number_img = warped_image(roi_rect).clone();

        cvtColor(armor.number_img, armor.number_img, COLOR_BGR2GRAY);
        threshold(armor.number_img, armor.number_img, 0, 255, THRESH_BINARY | THRESH_OTSU);

        classifyNumber(armor);
        printArmorInfo(armor);
    }
}

void NumberClassifier::classifyNumber(Armor& armor) {
    if (armor.number_img.empty()) {
        armor.has_number = false;
        return;
    }
    
    Mat input = armor.number_img.clone();
    input.convertTo(input, CV_32F);
    input /= 255.0f;

    Mat blob = dnn::blobFromImage(input);
    number_net_.setInput(blob);
    Mat outputs = number_net_.forward();

    // 后处理
    float max_prob = *max_element(outputs.begin<float>(), outputs.end<float>());
    Mat softmax_prob;
    cv::exp(outputs - max_prob, softmax_prob);
    float sum = static_cast<float>(cv::sum(softmax_prob)[0]);
    softmax_prob /= sum;

    // 获取结果
    Point class_id_point;
    // 改为 double 类型
    double max_val;                                
    minMaxLoc(softmax_prob.reshape(1, 1), nullptr, &max_val, nullptr, &class_id_point);
    armor.confidence = static_cast<float>(max_val);  // 转换回 float
    armor.number = class_names_[class_id_point.x];
    armor.has_number = (armor.number == "1") ||  (armor.number == "2")||(armor.number == "3") || (armor.number == "4")|| (armor.number == "5")&&
    (armor.confidence > number_confidence_threshold_);
}
void NumberClassifier::printArmorInfo(const Armor& armor) {
    if(!armor.has_number){return;}
    cout << "Armor Info :" << endl;
    cout << "Armor Number :"<<armor.number << endl;
    cout << "Position: [" << armor.rect.x << ", " << armor.rect.y << ", "
         << armor.rect.width << ", " << armor.rect.height << "]" << endl;
         cout << "Corner Points:" << endl;
         cout << "1. Bottom-Left:  (" << armor.rect.x << ", " << armor.rect.y + armor.rect.height << ")" << endl;
         cout << "2. Top-Left:     (" << armor.rect.x << ", " << armor.rect.y << ")" << endl;
         cout << "3. Top-Right:    (" << armor.rect.x + armor.rect.width << ", " << armor.rect.y << ")" << endl;
         cout << "4. Bottom-Right: (" << armor.rect.x + armor.rect.width << ", " << armor.rect.y + armor.rect.height << ")" << endl;
         cout << "--------------------------------" << endl;
}