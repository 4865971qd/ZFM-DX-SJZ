#ifndef SERIAL_H
#define SERIAL_H
#include <chrono> // 包含时间处理库
#include <thread> // 包含线程库
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <opencv2/opencv.hpp>
using namespace cv;

typedef union
{
    float f;
    unsigned char c[4];
}f2uc;//共用体 数据转换




typedef struct {
    f2uc wyaw;      // 改为x坐标
    f2uc wpitch;     // 改为y坐标
    unsigned char number;  // 新增数字编号
} allmsg;//结构体 xy的坐标


typedef struct
{

    f2uc ryaw;//x;
    f2uc rpitch;//y;
    
}indata;//结构体 xy的坐标



class serial
{
public:
    void Serial_op();//打开串口
    void Serial_initialize();
    int Serial_w(allmsg msg);
    int Serial_r(allmsg all_msg);
    void Serial_cl();
    double len;
    unsigned char senddata[15];
    int fd; /*File Descriptor*/
    indata in;

};

#endif // SERIAL_H

