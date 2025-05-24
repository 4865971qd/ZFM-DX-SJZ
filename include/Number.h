#ifndef NUMBER_H
#define NUMBER_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <opencv2/dnn.hpp>

using namespace cv;
using namespace std;

struct Armor {
    Rect rect;
    Mat number_img;
    string number;
    float confidence;
    bool has_number = false;

    ~Armor() {
        number_img.release(); // 显式释放Mat
    }
};

class NumberClassifier {
public:
    NumberClassifier();
    void extractAndClassifyNumbers(Mat& frame, vector<Armor>& armors);
    void classifyNumber(Armor& armor);
    void printArmorInfo(const Armor& armor);
    virtual ~NumberClassifier(); // 声明虚析构函数
private:
    dnn::Net number_net_;
    vector<string> class_names_;
    float number_confidence_threshold_ = 0.7;
    Size number_roi_size_ = Size(20, 28);
    
    void loadModel();
    bool containsNumber(const Mat& roi);
};

#endif // NUMBER_H