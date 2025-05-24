#include "Aimbot.h"
#include <opencv2/highgui.hpp>
#include <chrono>
using namespace std::chrono;  
using namespace cv;

int main() {
    Aimbot aimbot("/home/zhao/Aimbot/src/Number.avi");

    aimbot.initSerial();  // 初始化串口
    
    // 串口发送线程
    std::thread serial_thread([&]{
        while(true) {
            if (aimbot.hasNewData()) {         // 检查是否有新数据
                allmsg current_data = aimbot.getSerialData(); // 安全获取数据
                aimbot.sendArmorData(current_data); // 发送数据
            }
std::this_thread::sleep_for(std::chrono::milliseconds(10));     
   }
    });

    if (!aimbot.initialize()) {
        return -1;
    }

    Mat processedFrame;
    while (aimbot.processNextFrame(processedFrame)) {
        imshow("Armor Detection", processedFrame);
        if (waitKey(30) == 27) break;
    }

    aimbot.release();
    serial_thread.join();
    aimbot.closeSerial();
    return 0;

}