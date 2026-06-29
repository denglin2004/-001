/**
 * @file    core_control.h
 * @brief   打磨车主控核心控制模块头文件
 * @details 定义打磨车状态机、控制参数结构体及相关枚举类型
 *          实现打磨车的自动化打磨流程控制，包括：
 *          - 小车底盘前进/后退/转向控制
 *          - FOC电机启停控制
 *          - 步进电机升降控制
 *          - 打磨头找平/打磨控制
 * @version 2.0
 * @date    2026-05-04
 */

#ifndef __CORE_CONTROL_H__
#define __CORE_CONTROL_H__

#include "ch32v30x.h"

/* =================== 主状态枚举 =================== */
/**
 * @brief 打磨车主状态定义
 * @note  状态流转: IDLE -> PREPARE -> WORKING <-> TURNING -> COMPLETE -> IDLE
 */
typedef enum {
    STATE_IDLE,                    // 空闲状态：等待任务触发
    STATE_PREPARE,                 // 准备状态：下降打磨头并获取平衡面
    STATE_WORKING,                 // 工作状态：执行前进/后退打磨循环
    STATE_TURNING,                 // 转向状态：完成一行后转向下一行
    STATE_COMPLETE                 // 完成状态：停止所有电机并复位
} MainState_t;

/* =================== 子状态枚举 =================== */
/**
 * @brief 子状态定义，用于各主状态内部的分步执行
 */
typedef enum {
    SUBSTATE_NONE = 0,             // 无子状态
    SUBSTATE_SEND_CMD,             // 下发CAN指令
    SUBSTATE_WAIT_ACK,             // 等待CAN应答
    SUBSTATE_WAIT_TIME,            // 延时等待
    SUBSTATE_GET_BLANCE_PLANE,     // 获取平衡面（找平）
    SUBSTATE_MOVE_FORWARD,         // 小车前进
    SUBSTATE_MOVE_BACKWARD,        // 小车后退
    SUBSTATE_STOP,                 // 小车停止
    SUBSTATE_GRINDING,             // 打磨执行
    SUBSTATE_CHECK_TURN_COND,      // 检查转向条件
    SUBSTATE_PATH_DECISION,        // 路径决策（前进/后退/转向）
    SUBSTATE_FOC_START,            // FOC电机启动
    SUBSTATE_FOC_STOP,             // FOC电机停止
} SubState_t;

/* =================== 参数来源枚举 =================== */
/**
 * @brief 参数来源定义，标识任务参数的来源渠道
 */
typedef enum {
    PARAM_SRC_NONE = 0,            // 无来源
    PARAM_SRC_DCCP,                // 来自DCCP串口指令
    PARAM_SRC_XIAOZHI,             // 来自小智语音助手
    PARAM_SRC_YUNDUAN,             // 来自云端控制
} ParamSource_t;

/* =================== 动作类型枚举 =================== */
/**
 * @brief 动作类型定义，用于SendCommand函数分发执行
 */
typedef enum {
    ACTION_NONE,                   // 无动作
    ACTION_BUJING_GET_BLANCE,      // 步进电机获取平衡面
    ACTION_BUJING_UP,              // 步进电机上升（升降台上）
    ACTION_BUJING_DOWN,            // 步进电机下降（升降台下）
    ACTION_BUJING_GRIND,           // 步进电机打磨动作
    ACTION_CAR_FORWARD,            // 小车前进
    ACTION_CAR_BACKWARD,           // 小车后退
    ACTION_CAR_TURN_LEFT,          // 小车左转
    ACTION_CAR_STOP,               // 小车停止
    ACTION_FOC_START,              // FOC打磨头启动
    ACTION_FOC_STOP,               // FOC打磨头停止
} ActionType_t;

/* =================== 底盘控制子结构 =================== */
/**
 * @brief 小车底盘控制参数
 */
typedef struct {
    uint8_t  Forward_Count;        // 前进步数计数
    uint8_t  Backward_Count;       // 后退步数计数
    uint8_t  is_forward_phase;     // 行进阶段标志: 1=前进阶段, 0=后退阶段
    uint8_t  stop_sent;            // 停止指令已发送标志
    uint32_t time_move;            // 移动计时器
    uint32_t time_turn;            // 转向计时器
    uint32_t time_stop;            // 停止计时器
} CarCtrl_t;

/* =================== FOC电机控制子结构 =================== */
/**
 * @brief FOC无刷电机控制参数
 */
typedef struct {
    uint8_t  foc_sent;             // FOC电机指令已发送标志
    uint32_t time_foc;             // FOC电机计时器
} FOCCtrl_t;

/* =================== 打磨/找平控制子结构 =================== */
/**
 * @brief 打磨头控制参数
 */
typedef struct {
    uint8_t  grind_sent;           // 打磨指令已发送标志
    uint8_t  grind_get_blance_sent; // 找平指令已发送标志
    uint32_t time_grind;           // 打磨计时器
} GrindCtrl_t;

/* =================== 主控参数结构体 =================== */
/**
 * @brief 打磨车主控制参数结构体
 * @note  包含所有控制参数和子模块控制句柄
 */
typedef struct {
    /* —— 公共参数 —— */
    uint8_t           Step_Total;      // X方向打磨步数（每行前进次数）
    uint8_t           Loop_Total;      // S路径循环次数（总行数）
    uint16_t          grind_up_set;    // 打磨头上升高度设定值
    uint16_t          grind_down_set;  // 打磨头下降高度设定值
    uint16_t          grind_hifh_set;  // 打磨头高度阈值设定
    uint8_t           foc_speed_set;   // FOC电机速度设定值
    uint8_t           car_farward_speed_set;  // 小车前进速度设定
    uint8_t           car_backward_speed_set; // 小车后退速度设定
    uint8_t           car_turn_speed_set;     // 小车转向速度设定
    unsigned short    Loop_Finished;   // 已完成S路径循环次数
    uint8_t           complete_step;   // 完成状态分步执行标志
    uint8_t           task_S_flag;     // 任务使能标志: 0=空闲, 1=运行中, 2=已完成
    uint32_t          task_start_time; // 任务开始时间戳
    uint32_t          time_wait;       // 通用等待计时器

    /* —— 子模块控制 —— */
    CarCtrl_t   car;                   // 底盘控制句柄
    FOCCtrl_t   foc;                   // FOC电机控制句柄
    GrindCtrl_t grind;                 // 打磨/找平控制句柄
} S_Comand_Ctrl_Params_t;

/* =================== 统一任务触发参数结构体 =================== */
/**
 * @brief 任务请求结构体，用于各通信接口统一触发打磨任务
 * @note  通过设置trigger=1触发任务，系统会自动将参数桥接到grindcar_ctrl
 */
typedef struct {
    ParamSource_t source;        // 参数来源
    uint8_t       step_x;        // X轴步数
    uint8_t       loop_y;        // Y轴循环次数
    uint8_t       foc_speed;     // FOC速度
    uint8_t       car_speed;     // 底盘速度
    uint16_t      lift_high;     // 升降高度
    uint8_t       trigger;       // 触发标志: 1=请求启动
} S_TaskRequest_t;

/* =================== 全局变量声明 =================== */
extern S_Comand_Ctrl_Params_t grindcar_ctrl;  // 打磨车主控制参数
extern S_TaskRequest_t g_s_task_req;           // 统一任务请求参数

/* =================== 宏定义 =================== */
#define S_forward_time  500     // 前进动作持续时间(ms)
#define S_backward_time 500     // 后退动作持续时间(ms)

/* =================== 函数声明 =================== */
/**
 * @brief  主任务运行函数，处理DCCP和ESP32通信指令
 */
void mian_task_run(void);

/**
 * @brief  一键任务运行函数，处理按键控制和任务参数桥接
 */
void onekey_task_run(void);

/**
 * @brief  OLED显示打磨车状态信息
 * @param  line 起始显示行号
 */
void OLED_DisplayStatus(uint8_t line);

#endif /* __CORE_CONTROL_H__ */
