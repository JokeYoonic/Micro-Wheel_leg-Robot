// -----------------------------------------------------------------------------
// Copyright (c) 2024 Mu Shibo
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// -----------------------------------------------------------------------------

#include "robot.h" 

Wrobot wrobot; //机器人控制对象
RobotProtocol::RobotProtocol(uint8_t len)
{
    _len = len;
    _now_buf = new uint8_t[_len];
    _old_buf = new uint8_t[_len];

    for(int i=0;i<_len;i++) {
        _now_buf[i] = 0;
    }

    _now_buf[0] = 0xAA;
    _now_buf[1] = 0x55;
}
//构造函数初始化了协议的长度_len，并分配了两个缓冲区_now_buf和_old_buf用于存储当前和之前的协议数据。
//初始化时，将缓冲区的所有元素设为0，并将前两个字节设为固定的帧头0xAA和0x55

RobotProtocol::~RobotProtocol()
{
    delete [] _now_buf;
    delete [] _old_buf;
}
//析构函数释放了之前分配的缓冲区内存

void RobotProtocol::spinOnce(void)
{
    int flag = checkBufRefresh();
    if(flag) 
    {
        //UART_WriteBuf(); //这个会将web端的控制信息转成串口协议发出

        //Serial.println(wrobot.dir);//测试数据获取
        //Serial.println("date have send\n");
        //Serial.printf("height:%d\n", wrobot.height);
        //Serial.printf("roll:%d\n", wrobot.pitch);
        //Serial.printf("linear:%d\n", wrobot.linear);
//        Serial.printf("\n");
//        Serial.printf("joy_X:%d\n", wrobot.joyx);
//        Serial.printf("joy_Y:%d\n", wrobot.joyy);
    }
}
//spinOnce函数用于检查缓冲区中的数据是否发生变化(如果发生变化，则调用UART_WriteBuf函数将数据发送出去).








/**************如下同时用于串口输出控制协议***************/

void RobotProtocol::UART_WriteBuf(void)
{
    for(int i=0; i<_len; i++) {
        Serial.write(_now_buf[i]);
    }
}
//UART_WriteBuf函数用于将当前缓冲区中的数据通过串口发送出去。

int RobotProtocol::checkBufRefresh(void)
{
    int ret = 0;
    for(int i=0; i<_len; i++)
    {
        if(_now_buf[i] != _old_buf[i]) {
            ret = 1;
            break;
        }else {
            ret = 0;
        }
    }

    for(int i=0; i<_len; i++) {
        _old_buf[i]= _now_buf[i];
    }
    return ret;
}
//checkBufRefresh函数用于检查当前缓冲区中的数据是否与之前缓冲区中的数据相同，如果不同则返回1，否则返回0。
//同时，将当前缓冲区中的数据复制到之前缓冲区中，以便下一次比较。

void RobotProtocol::parseBasic(StaticJsonDocument<300> &doc)
{
 
    _now_buf[2] = BASIC;

    String dir = doc["dir"];
    if(dir == "stop") {
        _now_buf[3] = STOP;
        wrobot.dir = STOP;  
    } else {
        if(dir == "jump") {      	
            _now_buf[3] = JUMP;
            wrobot.dir = JUMP;
        }else if(dir == "forward") { 
            _now_buf[3] = FORWARD; 
            wrobot.dir = FORWARD;
        }else if(dir == "back") { 	
            _now_buf[3] = BACK; 
            wrobot.dir = BACK;
        }else if(dir == "left") { 	
            _now_buf[3] = LEFT;
            wrobot.dir = LEFT;
        }else if(dir == "right") { 
            _now_buf[3] = RIGHT;
            wrobot.dir = RIGHT;
        }else{
            _now_buf[3] = STOP;
            wrobot.dir = STOP;
        }
    }

    int height = doc["height"];
    _now_buf[4] = height;
    wrobot.height = height;

    int roll = doc["roll"];
    wrobot.roll = roll;
    if(roll >= 0) {
        _now_buf[5] = 0;
    }else {
        _now_buf[5] = 1;
    }
    _now_buf[6] = abs(roll);    
    
    int linear = doc["linear"];
    wrobot.linear = linear;
    if(linear >= 0) {
        _now_buf[7] = 0;
    }else {
        _now_buf[7] = 1;
    }
    _now_buf[8] = abs(linear); 
    
    int angular = doc["angular"];
    wrobot.angular = angular;
    if(angular >= 0) {
        _now_buf[9] = 0;
    }else {
        _now_buf[9] = 1;
    }
    _now_buf[10] = abs(angular); 
   

    int stable = doc["stable"];
    wrobot.go = stable;
    if(stable) {
        _now_buf[11] = 1;
    }else {
        _now_buf[11] = 0;
    }
    
    int joy_x = doc["joy_x"];
    wrobot.joyx = joy_x;
    if(joy_x >= 0) {
        _now_buf[12] = 0;
    }else {
        _now_buf[12] = 1;
    }
    _now_buf[13] = abs(joy_x); 


    int joy_y = doc["joy_y"];
    wrobot.joyy = joy_y;
    if(joy_y >= 0) {
        _now_buf[14] = 0;
    }else {
        _now_buf[14] = 1;
    }
    _now_buf[15] = abs(joy_y); 
}
//parseBasic函数用于解析web端发送过来的基本控制协议，并将解析后的数据存储到当前缓冲区中。
//同时，将解析后的数据存储到wrobot结构体中，以便在其他地方使用。
//包括方向、高度、倾斜角度、线速度、角速度、稳定性、摇杆X轴和Y轴数据等。