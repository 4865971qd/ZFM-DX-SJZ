#include "Aimbot.h"
#include <algorithm>

// 构造函数
Aimbot::Aimbot(const string& videoPath) : cap_(videoPath) {}

// 析构函数
Aimbot::~Aimbot() {
    release();
}

void Aimbot::release() {
    cap_.release();
}

bool Aimbot::initialize() {
    if (!cap_.isOpened()) {
        cerr << "Error opening video source" << endl;
        return false;
    }
    return true;
}

bool Aimbot::processNextFrame(Mat& outputFrame) {
    if (!cap_.read(frame_)) return false;
    
    // 完整处理流程
    Mat blueMask = extractBlueChannel(frame_);
    Mat binary;
    threshold(blueMask, binary, 100, 255, THRESH_BINARY);
    findLightRects(binary);
    pairLightsAndFindArmorPlates();
    drawResults(frame_);
    
    frame_.copyTo(outputFrame);
    return true;
}


Mat Aimbot::extractBlueChannel(const Mat& frame) {
    Mat channels[3];
    split(frame, channels);
    return channels[0];
}

void Aimbot::findLightRects(const Mat& binary) {
    lightRects_.clear();
    vector<vector<Point>> contours;
    findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        if (contour.size() > 5) {
            RotatedRect rect = minAreaRect(contour);
            float ratio = max(rect.size.width, rect.size.height) / 
                         min(rect.size.width, rect.size.height);
            if (ratio > 2.0 && ratio < 5.0) {
                lightRects_.push_back(rect);
            }
        }
    }
}

void Aimbot::pairLightsAndFindArmorPlates() {
    armors_.clear();
    if (lightRects_.size() < 2) return;

    sort(lightRects_.begin(), lightRects_.end(), [](const RotatedRect& a, const RotatedRect& b) {
        return a.center.x < b.center.x;
    });
// 相邻灯条配对
for (size_t i = 0; i < lightRects_.size() - 1; i++) {
    RotatedRect rect1 = lightRects_[i];
    RotatedRect rect2 = lightRects_[i+1];

    // 配对条件阈值
    const float angleThreshold = 15.0f;
    const float sizeThreshold = 0.3f;
    const float maxDistance = 300.0f;

    // 1. 角度差检查
    float angleDiff = abs(rect1.angle - rect2.angle);
    if (angleDiff > angleThreshold) continue;

    // 2. 大小差异检查
    float sizeDiff = abs(rect1.size.area() - rect2.size.area()) / 
                    max(rect1.size.area(), rect2.size.area());
    if (sizeDiff > sizeThreshold) continue;

    // 3. 距离检查
    float centerDistance = norm(rect1.center - rect2.center);
    if (centerDistance > maxDistance) continue;

    // 创建装甲板
    Point2f points1[4], points2[4];
    rect1.points(points1);
    rect2.points(points2);

    vector<Point> combinedPoints = {points1[0], points1[1], points1[2], points1[3], 
                                  points2[0], points2[1], points2[2], points2[3]};
    Rect armorRect = boundingRect(combinedPoints);

    Armor armor;
    armor.rect = armorRect;
    armors_.push_back(armor);
    }
    number_classifier_.extractAndClassifyNumbers(frame_, armors_);
}
void Aimbot::drawResults(Mat& frame) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    for (auto& armor : armors_) {
        if (!armor.has_number) continue;
        
        // 计算中心坐标
        Point2f center(armor.rect.x + armor.rect.width/2, 
            armor.rect.y + armor.rect.height/2);

// 更新数据
serial_data_.wyaw.f = center.x;
serial_data_.wpitch.f = center.y;
serial_data_.number = stoi(armor.number);
new_data_ = true;

        // 绘制装甲板矩形（绿色）
        rectangle(frame, armor.rect, Scalar(0, 255, 0), 2);

        // 绘制角点（红色）
        vector<Point> corners = {
            Point(armor.rect.x, armor.rect.y + armor.rect.height), // 左下
            Point(armor.rect.x, armor.rect.y),                     // 左上
            Point(armor.rect.x + armor.rect.width, armor.rect.y),  // 右上
            Point(armor.rect.x + armor.rect.width, armor.rect.y + armor.rect.height) // 右下
        };
        
        for (const auto& p : corners) {
            circle(frame, p, 3, Scalar(0, 0, 255), -1);
        }

        // 显示数字和置信度（黄色）
        string text = armor.number + " (" + to_string(int(armor.confidence*100)) + "%)";
        putText(frame, text, Point(armor.rect.x, armor.rect.y-5), 
               FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 255), 1);
        }
    }

    void Aimbot::sendArmorData(const allmsg& data) {
        se_.Serial_w(data); // 调用串口发送函数
    }

allmsg Aimbot::getSerialData() {
    std::lock_guard<std::mutex> lock(data_mutex_);
    new_data_ = false; // 重置数据标志
    return serial_data_;
}