#ifndef AIMBOT_H
#define AIMBOT_H

#include <opencv2/opencv.hpp>
#include <vector>
#include "Number.h"
#include "serial.h"

using namespace cv;
using namespace std;

class Aimbot {
public:
    Aimbot(const string& videoPath = "/home/zhao/Aimbot_111/src/Number.avi");
    ~Aimbot();  // 析构函数声明
    
    // 主流程接口
    bool initialize();
    bool processNextFrame(Mat& outputFrame);
    void release();

    // 串口相关接口
    void initSerial() { se_.Serial_op(); se_.Serial_initialize(); }
    void closeSerial() { se_.Serial_cl(); }
    void sendArmorData(const allmsg& data);

    // 数据访问接口
    bool hasNewData() const { return new_data_; }
    allmsg getSerialData();

private:
    VideoCapture cap_;
    NumberClassifier number_classifier_;
    std::mutex data_mutex_;
    ::allmsg serial_data_;
    bool new_data_ = false;
    ::serial se_;
    Mat frame_;
    vector<RotatedRect> lightRects_;
    vector<Armor> armors_;
    
    // 内部处理函数
    Mat extractBlueChannel(const Mat& frame);
    void findLightRects(const Mat& binary);
    void pairLightsAndFindArmorPlates();
    void drawResults(Mat& frame);
};

#endif // AIMBOT_H
