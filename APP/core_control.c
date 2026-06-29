/**
 * @file    core_control.c
 * @brief   打磨车主控核心控制模块
 * @details 实现打磨车状态机控制逻辑，包括：
 *          - 主状态机流转: IDLE -> PREPARE -> WORKING <-> TURNING -> COMPLETE
 *          - 子状态分步执行: 发送指令 -> 等待应答 -> 延时等待 -> 下一步
 *          - CAN总线指令下发与应答检测
 *          - 小车底盘、FOC电机、步进电机协同控制
 * @version 2.0
 * @date    2026-05-04
 */

/* =================== 头文件包含 =================== */
#include "core_control.h"
#include "DCCP_comand.h"
#include "ESP32_comand.h"
#include "can_app.h"
#include "bsp_can.h"
#include "usart_app.h"
#include "usart_protocol.h"
#include <string.h>
#include "hal_rgb.h"
#include "Timer.h"
#include "SSD1306.h"

/* =================== 全局变量定义 =================== */
S_TaskRequest_t g_s_task_req = {0};              // 统一任务请求参数
S_Comand_Ctrl_Params_t grindcar_ctrl = {0};      // 打磨车主控制参数

static MainState_t g_main_state = STATE_IDLE;    // 当前主状态
static SubState_t g_sub_state = SUBSTATE_NONE;   // 当前子状态

/* =================== 内部函数前向声明 =================== */
static void SendCommand(ActionType_t action, uint8_t Seq_id);
static int8_t CheckResponse(ActionType_t action, uint32_t timeout_ms, uint32_t send_time);
static void state_prepare(void);
static void working(void);
static void turning(void);
static void complete(void);

/* =================== 公共函数实现 =================== */

/**
 * @brief  主任务运行函数
 * @note   在主循环中调用，处理来自DCCP串口和ESP32的指令
 */
void mian_task_run(void)
{
    DCCP_comand_process();       // 处理DCCP串口指令
    ESP32_comand_process();      // 处理小智语音 + 云端控制指令
}

/**
 * @brief  一键任务运行函数
 * @note   处理按键控制，并将统一任务请求参数桥接到主控制结构体
 */
void onekey_task_run(void)
{
    Button_Control_Car();
    SSD1306_ShowNum(40, 6,CAN2A1_Rec.sCAN.pzd_high, 3, 8, 0);
    /* 消费统一任务请求：将 g_s_task_req 参数桥接到 grindcar_ctrl */
    if (g_s_task_req.trigger == 1)
    {
        g_s_task_req.trigger = 0;  // 清除触发标志，防止重复触发

        /* 参数映射 */
        grindcar_ctrl.Step_Total              = g_s_task_req.step_x;
        grindcar_ctrl.Loop_Total              = g_s_task_req.loop_y;
        grindcar_ctrl.foc_speed_set           = g_s_task_req.foc_speed;
        grindcar_ctrl.car_farward_speed_set   = g_s_task_req.car_speed;
        grindcar_ctrl.car_backward_speed_set  = g_s_task_req.car_speed;
        grindcar_ctrl.car_turn_speed_set      = g_s_task_req.car_speed;
        grindcar_ctrl.grind_down_set          = g_s_task_req.lift_high;

        /* 初始化任务状态 */
        grindcar_ctrl.task_S_flag   = 1;  // 标记任务开始
        grindcar_ctrl.complete_step = 0;
        grindcar_ctrl.Loop_Finished = 0;
        memset(&grindcar_ctrl.car,   0, sizeof(grindcar_ctrl.car));
        memset(&grindcar_ctrl.foc,   0, sizeof(grindcar_ctrl.foc));
        memset(&grindcar_ctrl.grind, 0, sizeof(grindcar_ctrl.grind));
    }

    S_Comand_Control_Car();
}

/* =================== 内部函数实现 =================== */

/**
 * @brief  发送CAN控制指令
 * @param  action  动作类型 (ActionType_t枚举值)
 * @param  Seq_id  指令序号，用于应答匹配
 * @note   根据动作类型分发到对应的CAN发送函数
 */
static void SendCommand(ActionType_t action, uint8_t Seq_id)
{
    switch (action)
    {
    case ACTION_BUJING_GET_BLANCE:
        S_ComandTo_BuJing(1, 1, 0, 0, Seq_id);       // 获取平衡面
        break;
    case ACTION_BUJING_UP:
        S_ComandTo_BuJing(3, 1, 1, 1600, Seq_id);    // 步进上升1600步
        break;
    case ACTION_BUJING_DOWN:
        S_ComandTo_BuJing(3, 1, 0, 1600, Seq_id);    // 步进下降1600步
        break;
    case ACTION_BUJING_GRIND:
        S_ComandTo_BuJing(2, 1, 0, 0, Seq_id);       // 打磨动作
        break;
    case ACTION_CAR_FORWARD:
        S_ComandTo_Car(1, 100, 1, Seq_id);   // 前进
        break;
    case ACTION_CAR_BACKWARD:
        S_ComandTo_Car(2, 100, 1, Seq_id);  // 后退
        break;
    case ACTION_CAR_TURN_LEFT:
        S_ComandTo_Car(3, 100, 1, Seq_id);      // 左转
        break;
    case ACTION_CAR_STOP:
        S_ComandTo_Car(5, 0, 2, Seq_id);             // 停止
        break;
    case ACTION_FOC_START:
        S_ComandTo_FOC(1, 255, Seq_id);  // FOC启动
        break;
    case ACTION_FOC_STOP:
        S_ComandTo_FOC(2, 0, Seq_id);                // FOC停止
        break;
    default:
        break;
    }
}

/**
 * @brief  检查CAN应答
 * @param  action      动作类型
 * @param  timeout_ms  超时时间(ms)
 * @param  send_time   指令发送时间戳
 * @retval  1: 应答成功
 * @retval -1: 超时
 * @retval  0: 等待中
 * @note   通过检查对应CAN接收缓冲区的应答标志判断是否完成
 */
static int8_t CheckResponse(ActionType_t action, uint32_t timeout_ms, uint32_t send_time)
{
    switch (action)
    {
    /* 小车停止应答检查 */
    case ACTION_CAR_STOP:
        if (CAN2A0_Rec.sCAN.ack == 0x02)
        {
            for (uint8_t i = 0; i < 8; i++)
                canrxbuf[0][i] = 0;
            return 1;
        }
        break;

    /* 步进上升应答检查 */
    case ACTION_BUJING_UP:
        if (CAN2A1_Rec.sCAN.pzd_function_code == 0x03)
        {
            for (uint8_t i = 0; i < 1; i++)
                canrxbuf[1][i] = 0;
            return 1;
        }
        break;

    /* 获取平衡面应答检查 */
    case ACTION_BUJING_GET_BLANCE:
        if (CAN2A1_Rec.sCAN.pzd_function_code == 0x01)
        {
            for (uint8_t i = 0; i < 1; i++)
                canrxbuf[1][i] = 0;
            return 1;
        }
        break;

    /* 步进下降应答检查 */
    case ACTION_BUJING_DOWN:
        if (CAN2A1_Rec.sCAN.pzd_function_code == 0x03)
        {
            for (uint8_t i = 0; i < 1; i++)
                canrxbuf[1][i] = 0;
            return 1;
        }
        break;

    /* 打磨动作应答检查 */
    case ACTION_BUJING_GRIND:
        if (CAN2A1_Rec.sCAN.pzd_function_code == 0x02)
        {
            for (uint8_t i = 0; i < 1; i++)
                canrxbuf[1][i] = 0;
            return 1;
        }
        break;

    /* FOC启动应答检查 */
    case ACTION_FOC_START:
        if (CAN2A2_Rec.sCAN.FOC_Start == 0x01)
        {
            for (uint8_t i = 0; i < 1; i++)
                canrxbuf[2][i] = 0;
            return 1;
        }
        break;

    /* FOC停止应答检查 */
    case ACTION_FOC_STOP:
        if (CAN2A2_Rec.sCAN.FOC_Start == 0x02)
        {
            for (uint8_t i = 0; i < 1; i++)
                canrxbuf[2][i] = 0;
            return 1;
        }
        break;

    default:
        break;
    }

    /* 超时检测 */
    if ((GetTick() - send_time) >= timeout_ms)
    {
        return -1;
    }

    return 0;  // 继续等待
}

/**
 * @brief  准备状态处理函数
 * @note   执行流程: 下降打磨头 -> 等待到位 -> 延时 -> 获取平衡面 -> 进入工作状态
 */
static void state_prepare(void)
{
    switch (g_sub_state)
    {
    case SUBSTATE_SEND_CMD:
        /* 发送步进下降指令 */
        SendCommand(ACTION_BUJING_DOWN, 1);
        grindcar_ctrl.time_wait = GetTick();
        g_sub_state = SUBSTATE_WAIT_ACK;
        break;

    case SUBSTATE_WAIT_ACK:
    {
        /* 等待下降应答，超时80000ms */
        int8_t result = CheckResponse(ACTION_BUJING_DOWN, 80000, grindcar_ctrl.time_wait);
        if (result == 1 || result == -1)  // 应答成功或超时
        {
                g_sub_state = SUBSTATE_WAIT_TIME;
                grindcar_ctrl.time_wait = GetTick();
        }
                    /* TOF距离检测：距离在有效范围内表示下降到位 */
            // if (CAN2A1_Rec.sCAN.pzd_high > 0 &&
            //     CAN2A1_Rec.sCAN.pzd_high <= 120)
            // {
            //     g_sub_state = SUBSTATE_WAIT_TIME;
            //     grindcar_ctrl.time_wait = GetTick();
            // }
        else if ((GetTick() - grindcar_ctrl.time_wait) >= 2000)  // 2秒超时重发
        {
            g_sub_state = SUBSTATE_SEND_CMD;
        }
    }
    break;

    case SUBSTATE_WAIT_TIME:
        /* 等待2秒确保打磨头完全到位 */
        if ((GetTick() - grindcar_ctrl.time_wait) >= 2000)
        {
            g_sub_state = SUBSTATE_GET_BLANCE_PLANE;
        }
        break;

    case SUBSTATE_GET_BLANCE_PLANE:
    {
        /* 发送获取平衡面指令（仅发送一次） */
        if (!grindcar_ctrl.grind.grind_get_blance_sent)
        {
            SendCommand(ACTION_BUJING_GET_BLANCE, 5);
            grindcar_ctrl.grind.grind_get_blance_sent = 1;
            grindcar_ctrl.time_wait = GetTick();
        }

        /* 等待平衡面获取完成 */
        int8_t result = CheckResponse(ACTION_BUJING_GET_BLANCE, 80000,
                                       grindcar_ctrl.time_wait);
        if (result == 1 || result == -1)
        {
            /* 平衡面获取完成，进入工作状态 */
            grindcar_ctrl.car.is_forward_phase = 1;
            grindcar_ctrl.grind.grind_get_blance_sent = 1;
            g_main_state = STATE_WORKING;
            g_sub_state = SUBSTATE_SEND_CMD;
        }
        else if ((GetTick() - grindcar_ctrl.time_wait) >= 2000)  // 2秒超时重发
        {
            g_sub_state = SUBSTATE_GET_BLANCE_PLANE;
            grindcar_ctrl.grind.grind_get_blance_sent = 0;
        }
    }
    break;

    default:
        g_sub_state = SUBSTATE_SEND_CMD;
        break;
    }
}

/**
 * @brief  工作状态处理函数
 * @note   执行流程: FOC停止 -> 前进/后退 -> 停止 -> FOC启动 -> 打磨 -> 检查转向条件
 */
static void working(void)
{
    switch (g_sub_state)
    {
    case SUBSTATE_SEND_CMD:
        
        SendCommand(ACTION_FOC_START, 2);
        grindcar_ctrl.time_wait = GetTick();
        g_sub_state = SUBSTATE_WAIT_ACK;
        break;

    case SUBSTATE_WAIT_ACK:
    {
        
        int8_t result = CheckResponse(ACTION_FOC_START, 80000, grindcar_ctrl.time_wait);
        if (result == 1 || result == -1)
        {
            g_sub_state = SUBSTATE_WAIT_TIME;
            grindcar_ctrl.time_wait = GetTick();
        }
        else if ((GetTick() - grindcar_ctrl.time_wait) >= 2000)  // 2秒超时重发
        {
            g_sub_state = SUBSTATE_SEND_CMD;
        }
    }
    break;

    case SUBSTATE_WAIT_TIME:
        /* 等待2秒系统稳定 */
        if ((GetTick() - grindcar_ctrl.time_wait) >= 6000)
        {
            grindcar_ctrl.car.stop_sent = 0;
            /* 根据当前阶段选择前进或后退状态 */
            g_sub_state = grindcar_ctrl.car.is_forward_phase ?
                          SUBSTATE_MOVE_FORWARD : SUBSTATE_MOVE_BACKWARD;
        }
        break;

    /* ---- 前进状态 ---- */
    case SUBSTATE_MOVE_FORWARD:
    {
        static uint8_t fwd_init = 0;
        if (!fwd_init)
        {
            SendCommand(ACTION_CAR_FORWARD, 3);      // 发送前进指令
            Drv_RGB_SetColor(RGB_COLOR_BLUE);         // 蓝色指示前进
            grindcar_ctrl.car.time_move = GetTick();
            fwd_init = 1;
        }

        /* 检查前进时间是否达到设定值 */
        if ((GetTick() - grindcar_ctrl.car.time_move) >= S_forward_time)
        {
            fwd_init = 0;
            grindcar_ctrl.car.stop_sent = 0;
            g_sub_state = SUBSTATE_STOP;
        }
    }
    break;

    /* ---- 后退状态 ---- */
    case SUBSTATE_MOVE_BACKWARD:
    {
        static uint8_t bwd_init = 0;
        if (!bwd_init)
        {
            SendCommand(ACTION_CAR_BACKWARD, 4);     // 发送后退指令
            Drv_RGB_SetColor(RGB_COLOR_GREEN);        // 绿色指示后退
            grindcar_ctrl.car.time_move = GetTick();
            bwd_init = 1;
        }

        /* 检查后退时间是否达到设定值 */
        if ((GetTick() - grindcar_ctrl.car.time_move) >= S_backward_time)
        {
            bwd_init = 0;
            grindcar_ctrl.car.stop_sent = 0;
            g_sub_state = SUBSTATE_STOP;
        }
    }
    break;

    /* ---- 停止状态 ---- */
    case SUBSTATE_STOP:
    {
        if (!grindcar_ctrl.car.stop_sent)
        {
            SendCommand(ACTION_CAR_STOP, 5);         // 发送停止指令
            Drv_RGB_SetColor(RGB_COLOR_RED);          // 红色指示停止
            grindcar_ctrl.car.time_stop = GetTick();
            grindcar_ctrl.car.stop_sent = 1;
        }

        /* 等待停止应答 */
        int8_t result = CheckResponse(ACTION_CAR_STOP, 80000,
                                       grindcar_ctrl.car.time_stop);
        if (result == 1 || result == -1)
        {
            grindcar_ctrl.car.stop_sent = 0;
            grindcar_ctrl.foc.foc_sent = 0;
            g_sub_state = SUBSTATE_FOC_START;         // 切换到FOC启动状态
        }
        else if ((GetTick() - grindcar_ctrl.car.time_stop) >= 2000)  // 2秒超时重发
        {
            g_sub_state = SUBSTATE_STOP;
        }
    }
    break;

    /* ---- FOC启动状态 ---- */
    case SUBSTATE_FOC_START:
    {
        static uint8_t foc_init = 0;
        if (!foc_init)
        {
            SendCommand(ACTION_FOC_START, 6);        // 发送FOC启动指令
            grindcar_ctrl.foc.time_foc = GetTick();
            foc_init = 1;
        }

        /* 等待FOC启动应答 */
        int8_t result = CheckResponse(ACTION_FOC_START, 80000, grindcar_ctrl.foc.time_foc);
        if (result == 1 || result == -1)
        {
            foc_init = 0;
            grindcar_ctrl.grind.grind_sent = 0;
            g_sub_state = SUBSTATE_GRINDING;          // 切换到打磨状态
        }
        else if ((GetTick() - grindcar_ctrl.foc.time_foc) >= 2000)  // 2秒超时重发
        {
            g_sub_state = SUBSTATE_FOC_START;
            foc_init = 0;
        }
    }
    break;

    /* ---- 打磨状态 ---- */
    case SUBSTATE_GRINDING:
    {
        if (!grindcar_ctrl.grind.grind_sent)
        {
            SendCommand(ACTION_BUJING_GRIND, 7);     // 发送打磨指令
            grindcar_ctrl.grind.time_grind = GetTick();
            grindcar_ctrl.grind.grind_sent = 1;
        }

        /* 等待打磨应答 */
        int8_t result = CheckResponse(ACTION_BUJING_GRIND, 80000,
                                       grindcar_ctrl.grind.time_grind);
        if (result == 1 || result == -1)
        {
            grindcar_ctrl.grind.grind_sent = 0;
            g_sub_state = SUBSTATE_CHECK_TURN_COND;   // 切换到转向条件检查
        }
        else if ((GetTick() - grindcar_ctrl.grind.time_grind) >= 2000)  // 2秒超时重发
        {
            g_sub_state = SUBSTATE_GRINDING;
            grindcar_ctrl.grind.grind_sent = 0;
        }
    }
    break;

    /* ---- 转向条件检查状态 ---- */
    case SUBSTATE_CHECK_TURN_COND:
    {
        /* 根据当前阶段递增计数 */
        if (grindcar_ctrl.car.is_forward_phase)
        {
            grindcar_ctrl.car.Forward_Count++;
        }
        else
        {
            grindcar_ctrl.car.Backward_Count++;
        }

        /* 判断是否需要转向：前进阶段达到总步数 或 后退阶段达到循环次数 */
        if ((grindcar_ctrl.car.is_forward_phase &&
             grindcar_ctrl.car.Forward_Count >= grindcar_ctrl.Step_Total) ||
            (!grindcar_ctrl.car.is_forward_phase &&
             grindcar_ctrl.car.Backward_Count >= grindcar_ctrl.Loop_Total))
        {
            g_main_state = STATE_TURNING;    // 进入转向状态
            g_sub_state = SUBSTATE_SEND_CMD;
        }
        else
        {
            g_sub_state = SUBSTATE_SEND_CMD;  // 继续当前循环
        }
    }
    break;

    default:
        g_sub_state = SUBSTATE_SEND_CMD;
        break;
    }
}

/**
 * @brief  转向状态处理函数
 * @note   执行流程: 左转 -> 停止 -> 路径决策（前进/后退/完成）
 */
static void turning(void)
{
    switch (g_sub_state)
    {
    case SUBSTATE_SEND_CMD:
    {
        static uint8_t turn_init = 0;
        if (!turn_init)
        {
            SendCommand(ACTION_CAR_TURN_LEFT, 8);    // 发送左转指令
            grindcar_ctrl.car.time_turn = GetTick();
            turn_init = 1;
        }

        /* 转向持续500ms */
        if ((GetTick() - grindcar_ctrl.car.time_turn) >= 500)
        {
            turn_init = 0;
            grindcar_ctrl.car.stop_sent = 0;
            g_sub_state = SUBSTATE_STOP;
        }
    }
    break;

    /* ---- 转向后停止 ---- */
    case SUBSTATE_STOP:
        if (!grindcar_ctrl.car.stop_sent)
        {
            SendCommand(ACTION_CAR_STOP, 5);          // 发送停止指令
            grindcar_ctrl.car.time_stop = GetTick();
            grindcar_ctrl.car.stop_sent = 1;
        }

        /* 等待停止应答 */
        int8_t result = CheckResponse(ACTION_CAR_STOP, 80000,
                                       grindcar_ctrl.car.time_stop);
        if (result == 1 || result == -1)
        {
            grindcar_ctrl.car.stop_sent = 0;
            g_sub_state = SUBSTATE_PATH_DECISION;     // 切换到路径决策
        }
        else if ((GetTick() - grindcar_ctrl.car.time_stop) >= 2000)  // 2秒超时重发
        {
            g_sub_state = SUBSTATE_STOP;
            grindcar_ctrl.car.stop_sent = 0;
        }
        break;

    /* ---- 路径决策状态 ---- */
    case SUBSTATE_PATH_DECISION:
        if (grindcar_ctrl.car.is_forward_phase)
        {
            /* 当前是前进阶段：转向后切换到后退阶段 */
            grindcar_ctrl.car.is_forward_phase = 0;
            g_main_state = STATE_WORKING;
            g_sub_state = SUBSTATE_SEND_CMD;
        }
        else
        {
            /* 当前是后退阶段：完成一轮循环 */
            grindcar_ctrl.Loop_Finished++;            // 循环完成数+1
            grindcar_ctrl.car.is_forward_phase = 1;   // 切换回前进阶段
            grindcar_ctrl.car.Forward_Count = 0;      // 前进计数清零
            grindcar_ctrl.car.Backward_Count = 0;     // 后退计数清零

            if (grindcar_ctrl.Loop_Finished < grindcar_ctrl.Loop_Total)
            {
                /* 未达总循环数，继续下一轮 */
                g_main_state = STATE_WORKING;
                g_sub_state = SUBSTATE_SEND_CMD;
            }
            else
            {
                /* 已达总循环数，进入完成状态 */
                grindcar_ctrl.complete_step = 0;
                g_main_state = STATE_COMPLETE;
            }
        }
        break;

    default:
        g_sub_state = SUBSTATE_SEND_CMD;
        break;
    }
}

/**
 * @brief  完成状态处理函数
 * @note   执行流程: 停止小车 -> 停止FOC -> 复位到空闲状态
 */
static void complete(void)
{
    switch (grindcar_ctrl.complete_step)
    {
    case 0:
        /* 步骤0：发送小车停止指令 */
        SendCommand(ACTION_CAR_STOP, 10);
        grindcar_ctrl.car.time_stop = GetTick();
        grindcar_ctrl.complete_step = 1;
        break;

    case 1:
    {
        /* 步骤1：等待小车停止应答 */
        int8_t result = CheckResponse(ACTION_CAR_STOP, 80000,
                                       grindcar_ctrl.car.time_stop);
        if (result == 1 || result == -1)
        {
            grindcar_ctrl.complete_step = 2;
        }
        else if ((GetTick() - grindcar_ctrl.car.time_stop) >= 2000)  // 2秒超时重发
        {
            grindcar_ctrl.complete_step = 1;
        }
    }
    break;

    case 2:
        /* 步骤2：发送FOC停止指令 */
        SendCommand(ACTION_FOC_STOP, 11);
        grindcar_ctrl.foc.time_foc = GetTick();
        grindcar_ctrl.complete_step = 3;
        break;

    case 3:
    {
        /* 步骤3：等待FOC停止应答，完成后返回空闲状态 */
        int8_t result = CheckResponse(ACTION_FOC_STOP, 80000,
                                       grindcar_ctrl.foc.time_foc);
        if (result == 1 || result == -1)
        {
            grindcar_ctrl.complete_step = 0;
            g_main_state = STATE_IDLE;            // 返回空闲状态
            grindcar_ctrl.task_S_flag = 2;        // 标记任务已完成
        }
        else if ((GetTick() - grindcar_ctrl.foc.time_foc) >= 2000)  // 2秒超时重发
        {
            grindcar_ctrl.complete_step = 2;
        }
    }
    break;
    }
}

/**
 * @brief  打磨车主控制函数
 * @note   打磨车自动化系统的核心，通过状态机协调小车前进/后退、
 *         FOC电机启停、打磨头升降等动作，实现S型路径自动打磨
 */
void S_Comand_Control_Car(void)
{
    /* 检查任务是否已完成，若task_S_flag==2则直接返回 */
    if (grindcar_ctrl.task_S_flag == 2)
        return;

    /* 从DCCP参数更新控制设定值 */
    grindcar_ctrl.foc_speed_set           = g_dccp_temp.foc_speed;      // FOC速度设定
    grindcar_ctrl.car_backward_speed_set  = g_dccp_temp.car_speed;      // 后退速度设定
    grindcar_ctrl.car_farward_speed_set   = g_dccp_temp.car_speed;      // 前进速度设定
    grindcar_ctrl.car_turn_speed_set      = g_dccp_temp.car_speed;      // 转向速度设定
    grindcar_ctrl.grind_down_set          = g_dccp_temp.lift_high;      // 打磨头下降高度设定

    /* 参数有效性检查 */
    if (grindcar_ctrl.Step_Total == 0 || grindcar_ctrl.Loop_Total == 0)
    {
        Drv_RGB_SetColor(RGB_COLOR_RED);   // 设置RGB为红色，提示异常
        g_main_state = STATE_IDLE;          // 切换到空闲状态
        return;
    }

    /* 主状态机调度：根据当前主状态执行对应的控制逻辑 */
    switch (g_main_state)
    {
    /* ===================== 空闲状态 ===================== */
    case STATE_IDLE:
        /* 初始化打磨车各模块参数 */
        grindcar_ctrl.car.Forward_Count   = 0;       // 前进计数清零
        grindcar_ctrl.car.Backward_Count  = 0;       // 后退计数清零
        grindcar_ctrl.car.is_forward_phase = 1;      // 初始为前进阶段
        grindcar_ctrl.car.stop_sent       = 0;       // 停止指令标志清零
        grindcar_ctrl.foc.foc_sent        = 0;       // FOC指令标志清零
        grindcar_ctrl.grind.grind_sent    = 0;       // 打磨指令标志清零
        grindcar_ctrl.complete_step       = 0;       // 完成步骤清零
        grindcar_ctrl.Loop_Finished       = 0;       // 循环完成数清零

        g_main_state = STATE_PREPARE;                // 切换到准备状态
        g_sub_state  = SUBSTATE_SEND_CMD;            // 子状态设为发送指令
        Drv_RGB_SetColor(RGB_COLOR_YELLOW);          // 黄色指示准备中
        break;

    /* ===================== 准备状态 ===================== */
    case STATE_PREPARE:
        state_prepare();
        break;

    /* ===================== 工作状态 ===================== */
    case STATE_WORKING:
        working();
        break;

    /* ===================== 转向状态 ===================== */
    case STATE_TURNING:
        turning();
        break;

    /* ===================== 完成状态 ===================== */
    case STATE_COMPLETE:
        complete();
        break;
    }
}

/* =================== OLED显示函数 =================== */

/**
 * @brief  获取主状态名称字符串
 * @param  state 主状态枚举值
 * @return 状态名称字符串指针
 */
static const char *GetMainStateName(MainState_t state)
{
    switch (state)
    {
    case STATE_IDLE:     return "IDLE   ";
    case STATE_PREPARE:  return "PREPARE";
    case STATE_WORKING:  return "WORKING";
    case STATE_TURNING:  return "TURNING";
    case STATE_COMPLETE: return "COMPLETE";
    default:             return "UNKNOWN";
    }
}

/**
 * @brief  获取子状态名称字符串
 * @param  state 子状态枚举值
 * @return 状态名称字符串指针
 */
static const char *GetSubStateName(SubState_t state)
{
    switch (state)
    {
    case SUBSTATE_NONE:             return "NONE";
    case SUBSTATE_SEND_CMD:         return "SEND-CMD";
    case SUBSTATE_WAIT_ACK:         return "WAIT_ACK";
    case SUBSTATE_WAIT_TIME:        return "WAIT_TIM";
    case SUBSTATE_MOVE_FORWARD:     return "FORWARD";
    case SUBSTATE_MOVE_BACKWARD:    return "BACK";
    case SUBSTATE_STOP:             return "STOP";
    case SUBSTATE_GRINDING:         return "GRIND";
    case SUBSTATE_CHECK_TURN_COND:  return "CHECK_TURN";
    case SUBSTATE_PATH_DECISION:    return "PATH_DECIS";
    case SUBSTATE_FOC_START:        return "FOC_ON";
    case SUBSTATE_FOC_STOP:         return "FOC_OFF";
    default:                        return "UNKNOWN";
    }
}

/**
 * @brief  OLED显示打磨车状态信息
 * @param  line 起始显示行号
 * @note   显示内容：主状态|子状态、前进/后退计数、循环计数、总步数/总循环数
 */
void OLED_DisplayStatus(uint8_t line)
{
    char buf[16];

    /* 第一行：主状态|子状态 */
    sprintf(buf, "%s|%s",
            GetMainStateName(g_main_state),
            GetSubStateName(g_sub_state));
    SSD1306_ShowStr(0, line, buf, 8, 0);

    /* 第二行：前进计数、后退计数、循环完成数 */
    sprintf(buf, "F%3d B%3d L%3d",
            grindcar_ctrl.car.Forward_Count,
            grindcar_ctrl.car.Backward_Count,
            grindcar_ctrl.Loop_Finished);
    SSD1306_ShowStr(0, line + 2, buf, 8, 0);

    /* 第三行：总步数、总循环数 */
    sprintf(buf, "S%3d L%3d",
            grindcar_ctrl.Step_Total,
            grindcar_ctrl.Loop_Total);
    SSD1306_ShowStr(0, line + 4, buf, 8, 0);
}
