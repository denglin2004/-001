/**
 * @file    ESP32_struct.h
 * @brief   ESP32通信协议结构体定义
 * @details 定义ESP32模块的通信帧结构、设备地址、功能码等
 *          支持小智语音助手和云端(OneNet)两种控制源
 *          协议帧格式: 帧头(3B) + 设备地址(1B) + 功能码(1B) + 数据域(6B) + 帧尾(4B) = 15B
 * @version 2.0
 * @date    2026-05-04
 */

#ifndef __ESP32_STRUCT_H__
#define __ESP32_STRUCT_H__

#include "ch32v30x.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

/* ======================== 帧头结构 ======================== */
/**
 * @brief ESP32通信帧头（3字节）
 */
typedef struct {
    uint8_t HeadA;              // 帧头字节1: 固定0xAA
    uint8_t HeadB;              // 帧头字节2: 固定0x55
    uint8_t HeadC;              // 帧头字节3: 固定0xA5
} ESP32_FrameHead_t;

/* ======================== 帧尾结构 ======================== */
/**
 * @brief ESP32通信帧尾（4字节）
 */
typedef struct {
    uint8_t Tail[4];            // 帧尾: 固定 0xFF 0xFD 0xFF 0xFF
} ESP32_FrameTail_t;

/* ======================== 数据域结构（按设备类型解析） ======================== */

/**
 * @brief 底盘控制数据（小智/云端共用）
 */
typedef struct {
    uint8_t Enable;             // 使能标志
    uint8_t Mode;               // 控制模式
    uint8_t Direction;          // 运动方向
    uint8_t Speed;              // 运动速度
    uint8_t Data_Spare[2];      // 预留数据
} ESP32_DataCar_t;

/**
 * @brief 升降模块数据
 */
typedef struct {
    uint8_t Data[6];            // 升降控制数据
} ESP32_DataLift_t;

/**
 * @brief FOC打磨电机数据
 */
typedef struct {
    uint8_t Enable;             // FOC使能标志
    uint8_t Reserved[5];        // 预留数据
} ESP32_DataFOC_t;

/**
 * @brief 一键指令数据
 */
typedef struct {
    uint8_t Move_Direction;     // 移动方向
    uint8_t Move_Distance;      // 移动距离
    uint8_t S_X;                // S路径X轴步数
    uint8_t S_Y;                // S路径Y轴循环数
    uint8_t Reserved[2];        // 预留数据
} ESP32_DataOneKey_t;

/**
 * @brief 数据域联合体：同一段6字节按不同设备类型解析
 */
typedef union {
    uint8_t             Raw[6];         // 原始字节
    ESP32_DataCar_t     Car;            // 底盘控制数据
    ESP32_DataLift_t    Lift;           // 升降模块数据
    ESP32_DataFOC_t     FOC;            // FOC电机数据
    ESP32_DataOneKey_t  OneKey;         // 一键指令数据
} ESP32_DataUnion_t;

/* ======================== 完整接收帧结构体 ======================== */
/**
 * @brief ESP32完整接收帧结构（共15字节）
 */
typedef struct {
    ESP32_FrameHead_t   Head;       // 帧头 3B (偏移0-2)
    uint8_t             DevAddr;    // 设备地址 1B (偏移3)
    uint8_t             FuncCode;   // 功能码   1B (偏移4)
    ESP32_DataUnion_t   Data;       // 数据域 6B (偏移5-10)
    ESP32_FrameTail_t   Tail;       // 帧尾 4B (偏移11-14)
} ESP32_RecFrame_t;                 // 总计 15B

/**
 * @brief 原始字节视图联合体（用于整帧拷贝）
 */
typedef union {
    uint8_t             Buf[15];    // 字节缓冲区
    ESP32_RecFrame_t    Frame;      // 帧结构体
} ESP32_RecData_t;

/* ======================== 全局变量声明 ======================== */
extern ESP32_RecData_t  ESP32_Rec;                  // ESP32接收缓冲区
extern ESP32_RecData_t  ESP32_Xiaozhi_Reccar;       // 小智-底盘控制数据
extern ESP32_RecData_t  ESP32_Xiaozhi_RecLift;      // 小智-升降模块数据
extern ESP32_RecData_t  ESP32_Xiaozhi_RecFOC;       // 小智-FOC电机数据
extern ESP32_RecData_t  ESP32_Yun_Reccar;           // 云端-底盘控制数据
extern ESP32_RecData_t  ESP32_Yun_RecLift;          // 云端-升降模块数据
extern ESP32_RecData_t  ESP32_Yun_RecFOC;           // 云端-FOC电机数据
extern ESP32_RecData_t  ESP32_Yun_RecOneKeycmd;     // 云端-一键指令数据
extern ESP32_RecData_t  ESP32_Xiaozhi_RecOneKeycmd; // 小智-一键指令数据

/* ======================== 设备地址定义 ======================== */
/* 小智语音助手控制源 */
#define DEV_BROADCAST            0x01   // 广播地址
#define DEV_CAR_CHASSIS          0x02   // 小智-底盘控制
#define DEV_LIFT_MODULE          0x03   // 小智-升降模块
#define DEV_GRINDER_HEAD         0x04   // 小智-打磨头(FOC)

/* 云端(OneNet)控制源 */
#define DEV_CAR_CHASSIS_ONENET   0x05   // 云端-底盘控制
#define DEV_LIFT_MODULE_ONENET   0x06   // 云端-升降模块
#define DEV_GRINDER_HEAD_ONENET  0x07   // 云端-打磨头(FOC)
#define DEV_ONEKEY_ONENET        0x08   // 云端-一键指令
#define DEV_ONEKEY_XIAOZHI       0x09   // 小智-一键指令

/* ======================== 功能码定义 ======================== */
#define FUNC_ACTION_CTRL    0x01       // 动作控制
#define FUNC_PARAM_SET      0x02       // 参数设置
#define FUNC_AUTO_ONEKEY    0x04       // 一键自动

/* ======================== 一键指令定义 ======================== */
#define ONEKEY_STOP_ALL      0x00      // 全部停止
#define ONEKEY_SINGLE_GRIND  0x01      // 单次打磨
#define ONEKEY_AREA_GRIND    0x02      // 区域打磨

/* ======================== 底盘方向定义 ======================== */
#define DIR_STOP            0          // 停止
#define DIR_FORWARD         1          // 前进
#define DIR_BACKWARD        2          // 后退
#define DIR_TURN_LEFT       3          // 左转
#define DIR_TURN_RIGHT      4          // 右转
#define DIR_SHIFT_LEFT      5          // 左移
#define DIR_SHIFT_RIGHT     6          // 右移

/* ======================== 升降模块动作定义 ======================== */
#define LIFT_STOP           0          // 升降停止
#define LIFT_UP             1          // 上升
#define LIFT_DOWN           2          // 下降

/* ======================== 打磨头启停定义 ======================== */
#define GRINDER_STOP        0          // 打磨头停止
#define GRINDER_START       1          // 打磨头启动

#ifdef __cplusplus
}
#endif

#endif /* __ESP32_STRUCT_H__ */
