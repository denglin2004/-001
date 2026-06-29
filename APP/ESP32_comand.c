/**
 * @file    ESP32_comand.c
 * @brief   ESP32通信指令处理模块
 * @details 实现ESP32模块的数据接收、指令解析和执行功能
 *          支持两种控制源：
 *          - 小智语音助手: 通过设备地址0x02-0x04控制
 *          - 云端(OneNet): 通过设备地址0x05-0x08控制
 *          支持的功能：
 *          - 底盘方向控制（前进/后退/转向）
 *          - 升降模块控制（上升/下降）
 *          - FOC电机控制（启停/调速）
 *          - 一键S路径任务触发
 * @version 2.0
 * @date    2026-05-04
 */

/* =================== 头文件包含 =================== */
#include "ESP32_comand.h"
#include "ESP32_struct.h"
#include "usart_app.h"
#include "can_app.h"
#include "hal_usart.h"
#include "string.h"
#include "usart_protocol.h"
#include "core_control.h"
#include <stdlib.h>
#include <time.h>

/* =================== 全局变量定义 =================== */
ESP32_RecData_t ESP32_Rec                    = {0};   // ESP32通用接收缓冲区

/* 小智语音助手数据缓冲区 */
ESP32_RecData_t ESP32_Xiaozhi_Reccar         = {0};   // 小智-底盘控制
ESP32_RecData_t ESP32_Xiaozhi_RecLift        = {0};   // 小智-升降模块
ESP32_RecData_t ESP32_Xiaozhi_RecFOC         = {0};   // 小智-FOC电机
ESP32_RecData_t ESP32_Xiaozhi_RecOneKeycmd   = {0};   // 小智-一键指令

/* 云端(OneNet)数据缓冲区 */
ESP32_RecData_t ESP32_Yun_Reccar             = {0};   // 云端-底盘控制
ESP32_RecData_t ESP32_Yun_RecLift            = {0};   // 云端-升降模块
ESP32_RecData_t ESP32_Yun_RecFOC             = {0};   // 云端-FOC电机
ESP32_RecData_t ESP32_Yun_RecOneKeycmd       = {0};   // 云端-一键指令

/* =================== 数据接收分发函数 =================== */

/**
 * @brief  ESP32接收数据包分发函数
 * @note   根据设备地址(DevAddr)将接收到的15字节数据分发到对应的缓冲区
 *         设备地址定义见ESP32_struct.h
 */
void ESP32_RecDataPacket(void)
{
    srand((unsigned int)time(NULL));

    /* 将接收到的15字节数据拷贝到通用缓冲区 */
    memcpy(ESP32_Rec.Buf, ESP32_rx_t.ESP32_rx_buf, 15);
    /* 根据设备地址分发到对应的缓冲区 */
    switch (ESP32_Rec.Frame.DevAddr)
    {
    
    /* ---- 小智语音助手控制源 ---- */
    case DEV_CAR_CHASSIS:           // 0x02: 小智-底盘控制
        memcpy(ESP32_Xiaozhi_Reccar.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_LIFT_MODULE:           // 0x03: 小智-升降模块
        memcpy(ESP32_Xiaozhi_RecLift.Buf, ESP32_Rec.Buf, 15);
        
        break;
    case DEV_GRINDER_HEAD:          // 0x04: 小智-FOC电机
        memcpy(ESP32_Xiaozhi_RecFOC.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_ONEKEY_XIAOZHI:        // 0x09: 小智-一键指令
        memcpy(ESP32_Xiaozhi_RecOneKeycmd.Buf, ESP32_Rec.Buf, 15);
        break;

    /* ---- 云端(OneNet)控制源 ---- */
    case DEV_CAR_CHASSIS_ONENET:    // 0x05: 云端-底盘控制
        memcpy(ESP32_Yun_Reccar.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_LIFT_MODULE_ONENET:    // 0x06: 云端-升降模块
        memcpy(ESP32_Yun_RecLift.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_GRINDER_HEAD_ONENET:   // 0x07: 云端-FOC电机
        memcpy(ESP32_Yun_RecFOC.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_ONEKEY_ONENET:         // 0x08: 云端-一键指令
        memcpy(ESP32_Yun_RecOneKeycmd.Buf, ESP32_Rec.Buf, 15);
        break;

    default:
        break;
    }
}

/* =================== 指令处理函数 =================== */

/**
 * @brief  ESP32指令处理主函数
 * @note   解析并执行来自小智和云端的控制指令
 *         支持的功能：底盘控制、升降控制、FOC控制、一键任务触发
 */
void ESP32_comand_process(void)
{
    uint8_t addr = ESP32_Rec.Frame.DevAddr;

    switch (addr)
    {
    /* ========== 小智-底盘控制 ========== *///YES
    case DEV_CAR_CHASSIS:
    {
        /* 速度限幅，最大20 */
        if (ESP32_Xiaozhi_Reccar.Frame.Data.Car.Speed >= 20)
        {
            ESP32_Xiaozhi_Reccar.Frame.Data.Car.Speed = 20;
        }
        /* 发送底盘控制指令 */
        S_ComandTo_Car(ESP32_Xiaozhi_Reccar.Frame.Data.Car.Direction,
                       ESP32_Xiaozhi_Reccar.Frame.Data.Car.Speed, 2, 1);
        memset(&ESP32_Xiaozhi_Reccar, 0, sizeof(ESP32_Xiaozhi_Reccar));
    }
    break;

    /* ========== 小智-升降模块 ========== *///NO
    case DEV_LIFT_MODULE:
    {
        switch (ESP32_Xiaozhi_RecLift.Frame.FuncCode)
        {
        case 0x01:
          {
             switch (ESP32_Xiaozhi_RecLift.Frame.Data.Lift.Data[0])
            {
            case 0x01: S_ComandTo_BuJing(3, 1, 1, 1600, 0);    // 步进上升1600步
            memset(&ESP32_Xiaozhi_RecLift, 0, sizeof(ESP32_Xiaozhi_RecLift));
            break;
            case 0x02: S_ComandTo_BuJing(3, 1, 0, 1600, 0);    // 步进下降1600步
            memset(&ESP32_Xiaozhi_RecLift, 0, sizeof(ESP32_Xiaozhi_RecLift));
            break;
            default:
            break;
           }
          }
          break;
        case 0x02: 
          {
            static u16 high_temp=0;
            static u8 high_temp_send=0;
            high_temp=ESP32_Xiaozhi_RecLift.Frame.Data.Lift.Data[0]<<8|ESP32_Xiaozhi_RecLift.Frame.Data.Lift.Data[1];
            high_temp_send=high_temp;
            S_ComandTo_BuJing(3, 1, 1,high_temp_send, 0);    // 步进上升high_tem步
            memset(&ESP32_Xiaozhi_RecLift, 0, sizeof(ESP32_Xiaozhi_RecLift));
          }
          break;
        }

    }
    break;

    /* ========== 小智-FOC电机 ========== *///YES
    case DEV_GRINDER_HEAD:
    {
        switch (ESP32_Xiaozhi_RecFOC.Frame.FuncCode)
        {
        case 0x01:
            /* FOC启停控制 */
            S_ComandTo_FOC(ESP32_Xiaozhi_RecFOC.Frame.Data.FOC.Enable, 250, 4);
            memset(&ESP32_Xiaozhi_RecFOC, 0, sizeof(ESP32_Xiaozhi_RecFOC));
            break;
        case 0x02:
            /* FOC速度控制 */
            S_ComandTo_FOC(1, ESP32_Xiaozhi_RecFOC.Frame.Data.Raw[1], 5);
            memset(&ESP32_Xiaozhi_RecFOC, 0, sizeof(ESP32_Xiaozhi_RecFOC));
            break;
        }
    }
    break;

    /* ========== 云端-底盘控制 ========== */
    case DEV_CAR_CHASSIS_ONENET:
    {
        /* 发送底盘控制指令 */
        S_ComandTo_Car(ESP32_Yun_Reccar.Frame.Data.Car.Direction,
                       ESP32_Yun_Reccar.Frame.Data.Car.Speed, 2, 1);
        memset(&ESP32_Yun_Reccar, 0, sizeof(ESP32_Yun_Reccar));
    }
    break;

    /* ========== 云端-升降模块 ========== */
    case DEV_LIFT_MODULE_ONENET:
    {
        switch (ESP32_Yun_RecLift.Frame.FuncCode)
        {
         case 0x01: S_ComandTo_BuJing(3, 1, 1, 1600, 0);    // 步进上升1600步
         //case 0x02: S_ComandTo_BuJing(3, 1, 0, 1600, 0);    // 步进下降1600步
         case 0x03: S_ComandTo_BuJing(3, 1, 1, ESP32_Yun_RecLift.Frame.Data.Lift.Data[1], 0);    // 设置高度
        }
    }
    break;

    /* ========== 云端-FOC电机 ========== */
    case DEV_GRINDER_HEAD_ONENET:
    {
        switch (ESP32_Yun_RecFOC.Frame.FuncCode)
        {
        case 0x01:
            /* FOC启停控制 */
            S_ComandTo_FOC(ESP32_Yun_RecFOC.Frame.Data.FOC.Enable, 250, 4);
            memset(&ESP32_Yun_RecFOC, 0, sizeof(ESP32_Yun_RecFOC));
            break;
        case 0x02:
            /* FOC速度控制 */
            S_ComandTo_FOC(1, ESP32_Yun_RecFOC.Frame.Data.Raw[5], 5);
            memset(&ESP32_Yun_RecFOC, 0, sizeof(ESP32_Yun_RecFOC));
            break;
        }
    }
    break;

    /* ========== 云端一键S路径任务触发 ========== */
    case DEV_ONEKEY_ONENET:
    {
        /* 只有任务未运行时才接受新任务 */
        if (grindcar_ctrl.task_S_flag != 1)
        {
            g_s_task_req.step_x    = ESP32_Yun_RecOneKeycmd.Frame.Data.OneKey.S_X;
            g_s_task_req.loop_y    = ESP32_Yun_RecOneKeycmd.Frame.Data.OneKey.S_Y;
            g_s_task_req.foc_speed = 200;
            g_s_task_req.source    = PARAM_SRC_YUNDUAN;
            g_s_task_req.trigger   = 1;
        }
        memset(&ESP32_Yun_RecOneKeycmd, 0, sizeof(ESP32_Yun_RecOneKeycmd));
    }
    break;

    /* ========== 小智一键S路径任务触发 ========== */
    case DEV_ONEKEY_XIAOZHI:
    {
        /* 只有任务未运行时才接受新任务 */
        if (grindcar_ctrl.task_S_flag != 1)
        {
            g_s_task_req.step_x    = ESP32_Xiaozhi_RecOneKeycmd.Frame.Data.OneKey.S_X;
            g_s_task_req.loop_y    = ESP32_Xiaozhi_RecOneKeycmd.Frame.Data.OneKey.S_Y;
            g_s_task_req.foc_speed = 200;
            g_s_task_req.source    = PARAM_SRC_XIAOZHI;
            g_s_task_req.trigger   = 1;
        }
        memset(&ESP32_Xiaozhi_RecOneKeycmd, 0, sizeof(ESP32_Xiaozhi_RecOneKeycmd));
    }
    break;

    default:
        break;
    }

    /* 清除设备地址，防止重复处理 */
    ESP32_Rec.Frame.DevAddr = 0;
}

/* =================== 数据上报函数 =================== */

/**
 * @brief  向云端上报数据函数
 * @note   组装随机数据帧并发送到K230模块
 *         帧格式: 帧头(3B) + 随机数据(8B) + 帧尾(4B) = 15B
 *         注意：需要在系统初始化时调用srand()设置随机种子
 */
void data_to_onnet(void)
{
    uint8_t Frame_Info[15];

    /* 帧头 */
    Frame_Info[0]  = 0xAA;             // 帧头字节1
    Frame_Info[1]  = 0x55;             // 帧头字节2
    Frame_Info[2]  = 0x05;             // 帧头字节3

    /* 填充随机数据（0-255范围） */
    Frame_Info[3]  = rand() % 256;
    Frame_Info[4]  = rand() % 256;
    Frame_Info[5]  = rand() % 256;
    Frame_Info[6]  = rand() % 256;
    Frame_Info[7]  = rand() % 256;
    Frame_Info[8]  = rand() % 256;
    Frame_Info[9]  = rand() % 256;
    Frame_Info[10] = rand() % 256;

    /* 帧尾 */
    Frame_Info[11] = 0xFF;
    Frame_Info[12] = 0xFC;
    Frame_Info[13] = 0xFF;
    Frame_Info[14] = 0xFF;

    /* 通过串口5发送到K230模块 */
    MW_Send(MW_PORT_5_K230, Frame_Info, 15, 0);
}
