#ifndef __CORE_CONTROL_H__
#define __CORE_CONTROL_H__
#include "ch32v30x.h"

// =================== 主状态枚�? ===================
typedef enum {
    STATE_IDLE,                    // 空闲状�?
    STATE_PREPARE,                 // 准�?�状�?
    STATE_WORKING,                 // 工作状�?
    STATE_TURNING,                 // �?向状�?
    STATE_COMPLETE                 // 完成状�?
} MainState_t;

// =================== 子状态枚�? ===================
typedef enum {
    SUBSTATE_NONE = 0,             // 无子状�?
    SUBSTATE_SEND_CMD,             // 下发指令
    SUBSTATE_WAIT_ACK,             // 等待CAN应答
    SUBSTATE_WAIT_TIME,            // 延时等待
    SUBSTATE_MOVE_FORWARD,         // 前进
    SUBSTATE_MOVE_BACKWARD,        // 后退
    SUBSTATE_STOP,                 // 小车停�??
    SUBSTATE_GRINDING,             // 打磨�?
    SUBSTATE_FOC_START,            // 磨头�?�?
    SUBSTATE_FOC_STOP,             // 磨头停�??
} SubState_t;

// =================== 参数来源枚举 ===================
typedef enum {
    PARAM_SRC_NONE = 0,
    PARAM_SRC_DCCP,
    PARAM_SRC_XIAOZHI,
    PARAM_SRC_YUNDUAN,
} ParamSource_t;

// =================== 动作类型枚举 ===================
typedef enum {
    ACTION_NONE,                   // 无动�?
    ACTION_BUJING_UP,              // 升降台上�?
    ACTION_BUJING_DOWN,            // 升降台下�?
    ACTION_BUJING_GRIND,           // 升降台打磨作�?
    ACTION_CAR_FORWARD,            // 小车前进
    ACTION_CAR_BACKWARD,           // 小车后退
    ACTION_CAR_TURN_LEFT,          // 小车左转
    ACTION_CAR_STOP,               // 小车停�??
    ACTION_FOC_START,              // 打磨头启�?
    ACTION_FOC_STOP,               // 打磨头停�?
} ActionType_t;

// =================== 底盘控制子结�? ===================
typedef struct {

    uint8_t  Forward_Count;        // 前进步数计数
    uint8_t  Backward_Count;       // 后退步数计数
    uint8_t  is_forward_phase;     // 行进阶�?�标�?: 1=前进阶�??, 0=后退阶�??
    uint8_t  stop_sent;            // 停�?�指令已发送标�?
    uint32_t time_move;            // 移动计时�?
    uint32_t time_turn;            // �?向�?�时�?
    uint32_t time_stop;            // 停�?��?�时�?


} CarCtrl_t;

// =================== FOC电机控制子结�? ===================
typedef struct {
    uint8_t  foc_sent;             // FOC电机指令已发送标�?
    uint32_t time_foc;             // FOC电机计时�?

} FOCCtrl_t;

// =================== 打磨/找平控制子结�? ===================
typedef struct {
    uint8_t  grind_sent;           // 打磨指令已发送标�?
    uint32_t time_grind;           // 打磨计时�?

} GrindCtrl_t;

// =================== 主控参数结构�? ===================
typedef struct {
    // —�? �?共变�? —�?
    uint8_t           Step_Total;      // 指令下方的X方向的打磨�?�数  
    uint8_t           Loop_Total;      // 指令下方的S�?径的�?�?次数
    uint16_t grind_up_set;        // 打磨指令值的�? 
    uint16_t grind_down_set;        // 打磨指令值的�?
    uint16_t grind_hifh_set;        // 打磨指令值的�?
    uint8_t  foc_speed_set;        // FOC电机速度设定�?
    uint8_t  car_farward_speed_set;            // �ٶ�
    uint8_t  car_backward_speed_set;
    uint8_t  car_turn_speed_set;
    unsigned short    Loop_Finished;   // 已完成S�?径的�?�?次数
    uint8_t           complete_step;   // 完成状态分步执行标�?
    uint8_t           task_S_cnt;      // 任务使能标志: 0=空闲, 1=运�?�中, 2=已完�?
    uint32_t          task_start_time; // 任务开始时间戳
    uint32_t          time_wait;       // 通用等待计时�?

    // —�? 子模块控�? —�?
    CarCtrl_t   car;                   // 底盘控制
    FOCCtrl_t   foc;                   // FOC电机控制
    GrindCtrl_t grind;                 // 打磨/找平控制
    
} S_Comand_Ctrl_Params_t;

// =================== 统一任务触发参数结构体 ===================
typedef struct {
    ParamSource_t source;        // 参数来源
    uint8_t       step_x;        // X轴步数
    uint8_t       loop_y;        // Y轴循环数
    uint8_t       foc_speed;     // FOC转速
    uint8_t       car_speed;     // 底盘速度
    uint16_t      lift_high;     // 升降高度
    uint8_t       trigger;       // 触发标志: 1=请求启动
} S_TaskRequest_t;

extern S_Comand_Ctrl_Params_t grindcar_ctrl;
extern S_TaskRequest_t g_s_task_req;

#define S_foward_time 500
#define S_backward_time 500
void mian_task_run (void);
void onekey_task_run (void);
void OLED_DisplayStatus (uint8_t line);
#endif
