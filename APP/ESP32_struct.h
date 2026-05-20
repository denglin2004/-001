#ifndef __ESP32_STRUCT_H__
#define __ESP32_STRUCT_H__

#include "ch32v30x.h"
#ifdef __cplusplus
extern "C" {
#endif



// 核心：取消编译器字节对齐，保证结构体长度=15字节（串口通信必备）
#pragma pack(1)  
struct strESP32RecData 
{
    // 帧头 3字节 (偏移0-2) 固定 0xAA 0x55 0xA5
    unsigned char  Frame_HeadA;		
    unsigned char  Frame_HeadB;	    
    unsigned char  Frame_HeadC;	    
    
    // 设备地址 1字节 (偏移3) 0x00广播/0x01底盘/0x02升降/0x03打磨头
    unsigned char  Device_Addr;     
    
    // 功能码 1字节 (偏移4) 0x01动作/0x02参数/0x04一键广播
    unsigned char  Func_Code;       
    
    // 数据域 6字节 (偏移5-10) 核心参数区
    unsigned char  Data[6];     
    
    // 帧尾 4字节 (偏移11-14) 固定 0xFF 0xFD 0xFF 0xFF
    unsigned char  Frame_Tail[4];	
};


typedef union unESP32_Data 
{
	unsigned char str[15];
	struct strESP32RecData  sAESP32;
};
union unESP32_Data  ESP32_Rec = { 0 };

struct strESP32RecDataCar 
{

    unsigned char  Frame_HeadA;		
    unsigned char  Frame_HeadB;	    
    unsigned char  Frame_HeadC;	    
    unsigned char  Device_Addr;     
    unsigned char  Func_Code;       
    unsigned char  Enable;     
    unsigned char  Mode;    
    unsigned char  Direction;   
    unsigned char  Speed; 
    unsigned char  Data_Spare[2];   
    unsigned char  Frame_Tail[4];	
};


typedef union unESP32car 
{
	unsigned char str[15];
	struct strESP32RecDataCar  sAESP32;
};
union unESP32car  ESP32_Reccar = { 0 };


struct strESP32RecDataBUjing 
{

    unsigned char  Frame_HeadA;		
    unsigned char  Frame_HeadB;	    
    unsigned char  Frame_HeadC;	    
    unsigned char  Device_Addr;     
    unsigned char  Func_Code;        

    unsigned char  Frame_Tail[4];	
};


typedef union unESP32BUjing  
{
	unsigned char str[15];
	struct strESP32RecDataBUjing   sAESP32;
};
union unESP32car  ESP32_RecBUjing  = { 0 };
// ************************* 设备地址定义 *************************
#define DEV_BROADCAST       0x00
#define DEV_CAR_CHASSIS     0x01
#define DEV_LIFT_MODULE     0x02
#define DEV_GRINDER_HEAD    0x03

// ************************* 功能码定义 *************************
#define FUNC_ACTION_CTRL    0x01
#define FUNC_PARAM_SET      0x02
#define FUNC_AUTO_ONEKEY    0x04

// ************************* 一键指令编号 *************************
#define ONEKEY_STOP_ALL      0x00
#define ONEKEY_SINGLE_GRIND  0x01
#define ONEKEY_AREA_GRIND    0x02

// ************************* 小车底盘动作定义（和你原始代码完全兼容） *************************
#define DIR_STOP            0
#define DIR_FORWARD         1
#define DIR_BACKWARD        2
#define DIR_TURN_LEFT       3
#define DIR_TURN_RIGHT      4
#define DIR_SHIFT_LEFT      5
#define DIR_SHIFT_RIGHT     6

// ************************* 升降模块动作定义 *************************
#define LIFT_STOP           0
#define LIFT_UP             1
#define LIFT_DOWN           2

// ************************* 打磨头动作定义 *************************
#define GRINDER_STOP        0
#define GRINDER_START       1
#endif  
