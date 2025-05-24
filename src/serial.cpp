#include "serial.h"
#include <iostream>
using namespace std;
serial se;

void serial::Serial_op()
{
    //char buf[] = "hello world!";
  //  fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY );//打开虚拟串口（数据线）
      fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY );//打开串口 （ch340）
    if(fd < 0)
    {
        perror("Can not open Serial_Port！");
        exit(EXIT_FAILURE);

    }

}


void serial::Serial_initialize()
{
    struct termios uart_cfg_opt;
    speed_t speed = B115200;
    if (-1 == tcgetattr(fd, &uart_cfg_opt))
         exit(EXIT_FAILURE);
    tcflush(fd, TCIOFLUSH);//刷新输入输出缓冲区
    cfsetospeed(&uart_cfg_opt, speed);//输入速度
    cfsetispeed(&uart_cfg_opt, speed);//输出速度
    if (-1 == tcsetattr(fd , TCSANOW, &uart_cfg_opt))
        exit(EXIT_FAILURE);
    uart_cfg_opt.c_cc[VTIME] = 1;//输入字符超时值0.1秒
    uart_cfg_opt.c_cc[VMIN] = 0;//最小读取字符数
    //数据长度设置部分
    uart_cfg_opt.c_cflag &= ~CSIZE;//清除数据位大小
    uart_cfg_opt.c_cflag |= CS8;//重新设置数据位大小8位
    uart_cfg_opt.c_iflag &= ~INPCK;//关闭奇偶校验功能
    uart_cfg_opt.c_cflag &= ~PARODD;//奇校验
    uart_cfg_opt.c_cflag &= ~CSTOPB;//偶校验
    //使用原始数据模式
    uart_cfg_opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    uart_cfg_opt.c_iflag &= ~(INLCR | IGNCR | ICRNL | IXON | IXOFF);
    uart_cfg_opt.c_oflag &= ~(INLCR | IGNCR | ICRNL);
    uart_cfg_opt.c_oflag &= ~(ONLCR | OCRNL);
    //应用新设置
    if (-1 == tcsetattr(fd,TCSANOW, &uart_cfg_opt))//tcsetattr获取终端设备的当前设置，立即生效
         exit(EXIT_FAILURE);
    tcflush(fd,TCIOFLUSH);//刷新输入输出缓冲区
}

int serial::Serial_w(allmsg all_msg) {
    unsigned char senddata[11];  // 帧头1 + number1 + x4 + y4 + 帧尾1 = 11
    senddata[0] = 0xAA;          // 帧头
    senddata[1] = all_msg.number;
    
    // x坐标
    senddata[2] = all_msg.wyaw.c[0];
    senddata[3] = all_msg.wyaw.c[1];
    senddata[4] = all_msg.wyaw.c[2];
    senddata[5] = all_msg.wyaw.c[3];
    
    // y坐标
    senddata[6] = all_msg.wpitch.c[0];
    senddata[7] = all_msg.wpitch.c[1];
    senddata[8] = all_msg.wpitch.c[2];
    senddata[9] = all_msg.wpitch.c[3];
    
    senddata[10] = 0x55;         // 帧尾
    
    len = write(fd, senddata, 11);
    return len;
}

int serial::Serial_r(allmsg all_msg)
{
    unsigned char senddata[10];
    len = read(fd, senddata, 10);
    if(len < 0)
    {
        cout<<"read data error \n"<<endl;
    }
    if((senddata[0]== 0x2b) && (senddata[9] == 0x4a))
    {
        in.ryaw.c[0]=senddata[1];
        in.ryaw.c[1]=senddata[2];
        in.ryaw.c[2]=senddata[3];
        in.ryaw.c[3]=senddata[4];

        in.rpitch.c[0]=senddata[5];
        in.rpitch.c[1]=senddata[6];
        in.rpitch.c[2]=senddata[7];
        in.rpitch.c[3]=senddata[8];
    }
  //  cout<< "收asd: "<<in.ryaw.f <<","<< in.rpitch.f <<endl;

}

void serial::Serial_cl()
{
    close(fd);
}

