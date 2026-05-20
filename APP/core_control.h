#ifndef __CORE_CONTROL_H__
#define __CORE_CONTROL_H__  
#include "ch32v30x.h"

// =================== 重构后的状态机枚举 ===================
// 主状态机 - 顶层控制
typedef enum {
    STATE_IDLE,                    // 空闲状态：系统初始化
    STATE_PREPARE,                // 准备状态：设备就位
    STATE_WORKING,                // 工作状态：执行打磨作业
    STATE_TURNING,                // 转向阶段：左移
    STATE_COMPLETE                // 完成状态：循环计数
} MainState_t;

// 子状态机 - 工作状态内部细分
typedef enum {
    SUBSTATE_NONE,                // 无子状态
    SUBSTATE_MOVE_FORWARD,        // 前进子状态
    SUBSTATE_MOVE_BACKWARD,       // 后退子状态
	  SUBSTATE_STOP,                // 小车停止子状态
    SUBSTATE_GRINDING,            // 打磨子状态
    SUBSTATE_FOC_START,           // 磨头启动
    SUBSTATE_FOC_STOP,            // 磨头停止
    SUBSTATE_WAITING              // 延时等待子状态
} SubState_t;

// 动作类型枚举
typedef enum {
    ACTION_NONE,                  // 无动作
    ACTION_BUJING_UP,             // 升降机构上升
    ACTION_BUJING_DOWN,           // 升降机构下降
    ACTION_BUJING_GRIND,          // 打磨找平
    ACTION_CAR_FORWARD,           // 小车前进
    ACTION_CAR_BACKWARD,          // 小车后退
    ACTION_CAR_TURN_LEFT,         // 小车左移
    ACTION_CAR_STOP,              // 小车停止
    ACTION_FOC_START,             // 打磨头启动
    ACTION_FOC_STOP,              // 打磨头关闭
} ActionType_t;

// =================== 【核心优化】封装控制参数结构体 ===================
typedef struct {
    uint8_t  Step_Total;          // 总步数
    uint8_t  Loop_Total;          // 总循环数
    uint8_t  Forward_Count;       // 前进计数
    uint8_t  Backward_Count;      // 后退计数
    unsigned short Loop_Finished;       // 已完成循环
    uint32_t action_start_time;   // 动作启动时间
    uint32_t task_start_time;     // 任务启动时间
    uint32_t task_S_cnt;          // 任务标志
} Car_Ctrl_Params_t;
extern Car_Ctrl_Params_t car_ctrl ;
void S_Comand_Control_Car(void);
void mian_task_run (void);
#endif 