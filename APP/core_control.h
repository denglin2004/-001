#ifndef __CORE_CONTROL_H__
#define __CORE_CONTROL_H__
#include "ch32v30x.h"

// =================== 主状态枚举 ===================
typedef enum {
    STATE_IDLE,                    // 空闲状态
    STATE_PREPARE,                 // 准备状态
    STATE_WORKING,                 // 工作状态
    STATE_TURNING,                 // 转向状态
    STATE_COMPLETE                 // 完成状态
} MainState_t;

// =================== 子状态枚举 ===================
typedef enum {
    SUBSTATE_NONE = 0,             // 无子状态
    SUBSTATE_SEND_CMD,             // 下发指令
    SUBSTATE_WAIT_ACK,             // 等待CAN应答
    SUBSTATE_WAIT_TIME,            // 延时等待
    SUBSTATE_MOVE_FORWARD,         // 前进
    SUBSTATE_MOVE_BACKWARD,        // 后退
    SUBSTATE_STOP,                 // 小车停止
    SUBSTATE_GRINDING,             // 打磨中
    SUBSTATE_FOC_START,            // 磨头启动
    SUBSTATE_FOC_STOP,             // 磨头停止
} SubState_t;

// =================== 动作类型枚举 ===================
typedef enum {
    ACTION_NONE,                   // 无动作
    ACTION_BUJING_UP,              // 升降台上升
    ACTION_BUJING_DOWN,            // 升降台下降
    ACTION_BUJING_GRIND,           // 升降台打磨作业
    ACTION_CAR_FORWARD,            // 小车前进
    ACTION_CAR_BACKWARD,           // 小车后退
    ACTION_CAR_TURN_LEFT,          // 小车左转
    ACTION_CAR_STOP,               // 小车停止
    ACTION_FOC_START,              // 打磨头启动
    ACTION_FOC_STOP,               // 打磨头停止
} ActionType_t;

// =================== 底盘控制子结构 ===================
typedef struct {

    uint8_t  Forward_Count;        // 前进步数计数
    uint8_t  Backward_Count;       // 后退步数计数
    uint8_t  is_forward_phase;     // 行进阶段标志: 1=前进阶段, 0=后退阶段
    uint8_t  stop_sent;            // 停止指令已发送标志
    uint32_t time_move;            // 移动计时器
    uint32_t time_turn;            // 转向计时器
    uint32_t time_stop;            // 停止计时器
} CarCtrl_t;

// =================== FOC电机控制子结构 ===================
typedef struct {
    uint8_t  foc_sent;             // FOC电机指令已发送标志
    uint32_t time_foc;             // FOC电机计时器
    uint8_t  foc_speed_set;        // FOC电机速度设定值
} FOCCtrl_t;

// =================== 打磨/找平控制子结构 ===================
typedef struct {
    uint8_t  grind_sent;           // 打磨指令已发送标志
    uint32_t time_grind;           // 打磨计时器
} GrindCtrl_t;

// =================== 主控参数结构体 ===================
typedef struct {
    // —— 公共变量 ——
    uint8_t           Step_Total;      // 指令下方的X方向的打磨次数  
    uint8_t           Loop_Total;      // 指令下方的S路径的循环次数

    unsigned short    Loop_Finished;   // 已完成S路径的循环次数
    uint8_t           complete_step;   // 完成状态分步执行标志
    uint8_t           task_S_cnt;      // 任务使能标志: 0=空闲, 1=运行中, 2=已完成
    uint32_t          task_start_time; // 任务开始时间戳
    uint32_t          time_wait;       // 通用等待计时器

    // —— 子模块控制 ——
    CarCtrl_t   car;                   // 底盘控制
    FOCCtrl_t   foc;                   // FOC电机控制
    GrindCtrl_t grind;                 // 打磨/找平控制
    
} S_Comand_Ctrl_Params_t;

extern S_Comand_Ctrl_Params_t grindcar_ctrl;
#define S_foward_time 500
#define S_backward_time 500
void mian_task_run (void);
void onekey_task_run (void);
void OLED_DisplayStatus (uint8_t line);
#endif
