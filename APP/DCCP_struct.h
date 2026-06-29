/**
 * @file    DCCP_struct.h
 * @brief   大彩串口屏通信协议结构体定义
 * @details 定义大彩串口屏(DCCP)的收发帧结构体、控件ID、参数常量等
 *          支持大彩屏的画面切换、参数设置、手动控制等功能
 * @version 2.0
 * @date    2026-05-04
 */

#ifndef __DCCP_STRUCT_H__
#define __DCCP_STRUCT_H__

#include "ch32v30x.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

/* =================== 大彩屏接收帧结构体 =================== */
/**
 * @brief 大彩屏接收数据帧结构
 * @note  大彩屏采用小端模式，16位数据低字节在前
 */
struct strDCCPRecData
{
    unsigned char   Frame_Head;         // 帧头
    unsigned char   Frame_cmd_type;     // 命令类型
    unsigned char   Frame_ctrl_msg;     // 消息类型
    unsigned short  Frame_ImageID;      // 画面ID
    unsigned short  Frame_ControlID;    // 控件ID
    unsigned char   Frame_control_type; // 控件类型
    unsigned char   Frame_param[8];     // 可变长度参数(最大8字节)
    unsigned long   Frame_Tail;         // 帧尾
};

/**
 * @brief 大彩屏接收数据联合体（字节数组与结构体互转）
 */
typedef union unDCCP_Data
{
    unsigned char str[20];
    struct strDCCPRecData sDCCP;
};

/* 大彩屏接收缓冲区实例 */
union unDCCP_Data DCCP_Rec = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/* =================== 大彩屏发送帧结构体 =================== */
/**
 * @brief 大彩屏发送数据帧结构
 */
struct strDCCPSendData
{
    unsigned char   Frame_SendHead;     // 帧头
    unsigned char   Frame_Sendcmd_type; // 命令类型
    unsigned char   Frame_Sendctrl_msg; // 消息类型
    unsigned short  Frame_SendImageID;  // 画面ID
    unsigned short  Frame_SendControlID;// 控件ID
    unsigned char   Frame_Control_type; // 控件类型
    unsigned char   Frame_Sendparam[8]; // 可变长度参数(最大8字节)
    unsigned long   Frame_Tail;         // 帧尾
};

/**
 * @brief 大彩屏发送数据联合体
 */
typedef union unDCCP_SendData
{
    unsigned char str[20];
    struct strDCCPSendData sDCCP;
};

/* 大彩屏发送缓冲区实例（默认帧尾 0xFF 0xFC 0xFF 0xFF） */
union unDCCP_SendData DCCP_Send = {0xEE, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0xFF, 0xFC, 0xFF, 0xFF};

/* =================== 大彩屏显示数据变量 =================== */
unsigned short Cur_DMT_Speed = 0;       // 打磨头转速
unsigned short Cur_DMT_Temp  = 0;       // 打磨头温度
unsigned short Cur_DMT_VOL   = 0;       // 打磨头电压
unsigned short Cur_DMT_I     = 0;       // 打磨头电流
unsigned short Cur_DMT_High  = 0;       // 打磨头高度
unsigned char  Disp_Turn     = 0;       // 显示轮询计数器

/* =================== 显示控件ID定义 =================== */
#define DISP_DMTSPEED       0           // 打磨头转速显示
#define DISP_DMTTEMP        1           // 打磨头温度显示
#define DISP_DMTVOL         2           // 打磨头电压显示
#define DISP_DMTCUR         3           // 打磨头电流显示
#define DISP_DMTHIGH        4           // 打磨头高度显示
#define DISP_TASK_PROGRESS  5           // 任务完成进度显示

/* =================== 设置命令ID定义 =================== */
#define SET_CMD_DMTSTART    0x07        // 打磨头启停控制: 1=启动, 0=停止
#define SET_CMD_DMTSPEED    0x08        // 打磨头转速设置
#define SET_CMD_DMTHIGHT    0x09        // 打磨头高度设置
#define SET_CMD_DMTUP       0x17        // 打磨头上升控制
#define SET_CMD_DMTDOWN     0x18        // 打磨头下降控制

/* =================== 小车方向命令ID定义 =================== */
#define SET_CMD_CAR_FORWORD   0x03      // 小车前进
#define SET_CMD_CAR_BACKWORD  0x06      // 小车后退
#define SET_CMD_CAR_TURNLEFT  0x04      // 小车左转
#define SET_CMD_CAR_TURNRIGHT 0x05      // 小车右转

/* =================== 小车控制参数结构体 =================== */
/**
 * @brief 小车手动/自动控制参数
 */
typedef struct {
    uint8_t  ExForm_mode;               // 控制模式: 0=空闲, 1=按钮点动, 2=自动运行
    uint8_t  ExForm_diretcion;          // 运动方向: 0=停止, 1=前, 2=后, 3=左, 4=右

    uint32_t start_tick[4];             // 4个方向的按钮按下时间戳
    uint32_t run_time[4];               // 各方向运动持续时间(ms)
    uint8_t  timer_en[4];               // 按钮计时使能标志(0=未按下, 1=按下)
    uint8_t  run_flag[4];               // 运动完成标志(0=未完成, 1=已完成)
    uint8_t  last_cmdParam[4];          // 上次命令参数缓存
    uint8_t  Run_S_X;                   // S路径X轴步数
    uint8_t  Run_S_Y;                   // S路径Y轴循环数
    uint8_t  Run_distance;              // 单次运行距离
    uint8_t  Run_cnt_x;                 // X轴运行计数
    uint8_t  Run_cnt_y;                 // Y轴运行计数
} Car_Ctrl_Typedef;

/* =================== FOC电机控制参数结构体 =================== */
/**
 * @brief FOC无刷电机控制参数
 */
typedef struct {
    uint8_t speed;                      // FOC电机转速设定值
} Foc_Ctrl_Typedef;

/* =================== 打磨头控制参数结构体 =================== */
/**
 * @brief 打磨头升降控制参数
 */
typedef struct {
    uint8_t i;                          // 预留参数
    uint8_t high;                       // 打磨头高度
    uint8_t up;                         // 上升步数
    uint8_t down;                       // 下降步数
} GrindCtrl_Typedef;

/* =================== 静态控制参数实例 =================== */
static Car_Ctrl_Typedef   g_car   = {0};   // 小车控制参数
static Foc_Ctrl_Typedef   g_foc   = {0};   // FOC控制参数
static GrindCtrl_Typedef  g_grind = {0};   // 打磨头控制参数

/* =================== 时间常量定义 =================== */
#define TICK_MS     1000                // 时间单位转换系数
#define K_time      1                   // 时间系数

#ifdef __cplusplus
}
#endif

#endif /* __DCCP_STRUCT_H__ */
