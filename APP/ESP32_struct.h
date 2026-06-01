#ifndef __ESP32_STRUCT_H__
#define __ESP32_STRUCT_H__

#include "ch32v30x.h"
#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

// ======================== 帧头 ========================
typedef struct {
    uint8_t HeadA;          // 0xAA
    uint8_t HeadB;          // 0x55
    uint8_t HeadC;          // 0xA5
} ESP32_FrameHead_t;

// ======================== 帧尾 ========================
typedef struct {
    uint8_t Tail[4];        // 0xFF 0xFD 0xFF 0xFF
} ESP32_FrameTail_t;

// ======================== 数据域（按设备类型解析） ========================

// 底盘控制数据（小智/云端共用）
typedef struct {
    uint8_t Enable;
    uint8_t Mode;
    uint8_t Direction;
    uint8_t Speed;
    uint8_t Data_Spare[2];
} ESP32_DataCar_t;

// 升降模块数据
typedef struct {
    uint8_t Data[6];
} ESP32_DataLift_t;

// FOC 打磨电机数据
typedef struct {
    uint8_t Enable;
    uint8_t Reserved[5];
} ESP32_DataFOC_t;

// 一键指令数据
typedef struct {
    uint8_t Move_Direction;
    uint8_t Move_Distance;
    uint8_t S_X;
    uint8_t S_Y;
    uint8_t Reserved[2];
} ESP32_DataOneKey_t;

// 数据域联合体：同一段 6 字节按不同设备类型解析
typedef union {
    uint8_t             Raw[6];
    ESP32_DataCar_t     Car;
    ESP32_DataLift_t    Lift;
    ESP32_DataFOC_t     FOC;
    ESP32_DataOneKey_t  OneKey;
} ESP32_DataUnion_t;

// ======================== 完整接收帧结构体 ========================
typedef struct {
    ESP32_FrameHead_t   Head;       // 帧头 3B (偏移0-2)
    uint8_t             DevAddr;    // 设备地址 1B (偏移3)
    uint8_t             FuncCode;   // 功能码   1B (偏移4)
    ESP32_DataUnion_t   Data;       // 数据域 6B (偏移5-10)
    ESP32_FrameTail_t   Tail;       // 帧尾 4B (偏移11-14)
} ESP32_RecFrame_t;                 // 总计 15B

// 原始字节视图（用于整帧拷贝）
typedef union {
    uint8_t             Buf[15];
    ESP32_RecFrame_t    Frame;
} ESP32_RecData_t;

// ======================== 全局变量声明 ========================
extern ESP32_RecData_t  ESP32_Rec;
extern ESP32_RecData_t  ESP32_Xiaozhi_Reccar;
extern ESP32_RecData_t  ESP32_Xiaozhi_RecLift;
extern ESP32_RecData_t  ESP32_Xiaozhi_RecFOC;
extern ESP32_RecData_t  ESP32_Yun_Reccar;
extern ESP32_RecData_t  ESP32_Yun_RecLift;
extern ESP32_RecData_t  ESP32_Yun_RecFOC;
extern ESP32_RecData_t  ESP32_Yun_RecOneKeycmd;
extern ESP32_RecData_t  ESP32_Xiaozhi_RecOneKeycmd;

// ======================== 设备地址定义 ========================
#define DEV_BROADCAST            0x01
#define DEV_CAR_CHASSIS          0x02
#define DEV_LIFT_MODULE          0x03
#define DEV_GRINDER_HEAD         0x04

#define DEV_CAR_CHASSIS_ONENET   0x05
#define DEV_LIFT_MODULE_ONENET   0x06
#define DEV_GRINDER_HEAD_ONENET  0x07
#define DEV_ONEKEY_ONENET        0x08
#define DEV_ONEKEY_XIAOZHI       0x09

// ========================功能码定义 ========================
#define FUNC_ACTION_CTRL    0x01
#define FUNC_PARAM_SET      0x02
#define FUNC_AUTO_ONEKEY    0x04

// ======================== 一键指令定义 ========================
#define ONEKEY_STOP_ALL      0x00
#define ONEKEY_SINGLE_GRIND  0x01
#define ONEKEY_AREA_GRIND    0x02

// ======================== 底盘方向定义 ========================
#define DIR_STOP            0
#define DIR_FORWARD         1
#define DIR_BACKWARD        2
#define DIR_TURN_LEFT       3
#define DIR_TURN_RIGHT      4
#define DIR_SHIFT_LEFT      5
#define DIR_SHIFT_RIGHT     6

// ======================== 升降模块动作定义 ========================
#define LIFT_STOP           0
#define LIFT_UP             1
#define LIFT_DOWN           2

// ======================== 打磨头启停定义 ========================
#define GRINDER_STOP        0
#define GRINDER_START       1

#ifdef __cplusplus
}
#endif
#endif
