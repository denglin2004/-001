#include "ESP32_comand.h"
#include "ESP32_struct.h"
#include "usart_app.h"
#include "can_app.h"
#include "hal_usart.h"
#include "string.h"
#include "usart_protocol.h"
#include "core_control.h"
#include <stdlib.h>   // rand() ���������
#include <time.h>     // time() �������

// ======================== 全局变量定义 ========================
ESP32_RecData_t  ESP32_Rec                = { 0 };

ESP32_RecData_t  ESP32_Xiaozhi_Reccar     = { 0 };
ESP32_RecData_t  ESP32_Xiaozhi_RecLift    = { 0 };
ESP32_RecData_t  ESP32_Xiaozhi_RecFOC     = { 0 };
ESP32_RecData_t  ESP32_Xiaozhi_RecOneKeycmd = { 0 };

ESP32_RecData_t  ESP32_Yun_Reccar         = { 0 };
ESP32_RecData_t  ESP32_Yun_RecLift        = { 0 };
ESP32_RecData_t  ESP32_Yun_RecFOC         = { 0 };
ESP32_RecData_t  ESP32_Yun_RecOneKeycmd   = { 0 };


/*
**************************************************************************************
* �? �? �?: ESP32_RecDataPacket
* 功能说明: ESP32协�??接收数据包分�?
**************************************************************************************
*/
void ESP32_RecDataPacket(void)
{srand((unsigned int)time(NULL));
    memcpy(ESP32_Rec.Buf, ESP32_rx_t.ESP32_rx_buf, 15);

    switch (ESP32_Rec.Frame.DevAddr)
    {
    case DEV_CAR_CHASSIS:
        memcpy(ESP32_Xiaozhi_Reccar.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_LIFT_MODULE:
        memcpy(ESP32_Xiaozhi_RecLift.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_GRINDER_HEAD:
        memcpy(ESP32_Xiaozhi_RecFOC.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_CAR_CHASSIS_ONENET:
        memcpy(ESP32_Yun_Reccar.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_LIFT_MODULE_ONENET:
        memcpy(ESP32_Yun_RecLift.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_GRINDER_HEAD_ONENET:
        memcpy(ESP32_Yun_RecFOC.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_ONEKEY_ONENET:
        memcpy(ESP32_Yun_RecOneKeycmd.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_ONEKEY_XIAOZHI:
        memcpy(ESP32_Xiaozhi_RecOneKeycmd.Buf, ESP32_Rec.Buf, 15);
        break;
    default:
        break;
    }
}


void ESP32_comand_process(void)
{
    uint8_t addr = ESP32_Rec.Frame.DevAddr;

    switch (addr)
    {
    case DEV_CAR_CHASSIS:
    {
        if (ESP32_Xiaozhi_Reccar.Frame.Data.Car.Speed >= 20)
        {
            ESP32_Xiaozhi_Reccar.Frame.Data.Car.Speed = 20;
        }
        S_ComandTo_Car(ESP32_Xiaozhi_Reccar.Frame.Data.Car.Direction,
                        ESP32_Xiaozhi_Reccar.Frame.Data.Car.Speed, 2, 1);
        memset(&ESP32_Xiaozhi_Reccar, 0, sizeof(ESP32_Xiaozhi_Reccar));
    }
    break;

    case DEV_LIFT_MODULE:
    {
        switch (ESP32_Xiaozhi_RecLift.Frame.FuncCode)
        {
        // case 0x01: S_ComandTo_BuJing(...); break;
        // case 0x02: S_ComandTo_BuJing(...); break;
        }
    }
    break;

    case DEV_GRINDER_HEAD:
    {
        switch (ESP32_Xiaozhi_RecFOC.Frame.FuncCode)
        {
        case 0x01:
            S_ComandTo_FOC(ESP32_Xiaozhi_RecFOC.Frame.Data.FOC.Enable, 250, 4);
            memset(&ESP32_Xiaozhi_RecFOC, 0, sizeof(ESP32_Xiaozhi_RecFOC));
            break;
        case 0x02:
            S_ComandTo_FOC(1, ESP32_Xiaozhi_RecFOC.Frame.Data.Raw[1], 5);
            memset(&ESP32_Xiaozhi_RecFOC, 0, sizeof(ESP32_Xiaozhi_RecFOC));
            break;
        }
    }
    break;
    case DEV_CAR_CHASSIS_ONENET:
    {

        S_ComandTo_Car(ESP32_Yun_Reccar.Frame.Data.Car.Direction,
                        ESP32_Yun_Reccar.Frame.Data.Car.Speed, 2, 1);
        memset(&ESP32_Yun_Reccar, 0, sizeof(ESP32_Yun_Reccar));
    }
    break;

    case DEV_LIFT_MODULE_ONENET:
    {
        switch (ESP32_Yun_RecLift.Frame.FuncCode)
        {
        // case 0x01: S_ComandTo_BuJing(...); break;
        // case 0x02: S_ComandTo_BuJing(...); break;
        }
    }
    break;

    case DEV_GRINDER_HEAD_ONENET:
    {
        switch (ESP32_Yun_RecFOC.Frame.FuncCode)
        {
        case 0x01:
            S_ComandTo_FOC(ESP32_Yun_RecFOC.Frame.Data.FOC.Enable, 250, 4);
            memset(&ESP32_Yun_RecFOC, 0, sizeof(ESP32_Yun_RecFOC));
            break;
        case 0x02:
            S_ComandTo_FOC(1, ESP32_Yun_RecFOC.Frame.Data.Raw[1], 5);
            memset(&ESP32_Yun_RecFOC, 0, sizeof(ESP32_Yun_RecFOC));
            break;
        }
    }
    break;

    // ========== 云端一键S路径任务触发 ==========
    case DEV_ONEKEY_ONENET:
    {
        if (grindcar_ctrl.task_S_cnt != 1)  // 任务未运行时才接受
        {
            g_s_task_req.step_x    = ESP32_Yun_RecOneKeycmd.Frame.Data.OneKey.S_X;
            g_s_task_req.loop_y    = ESP32_Yun_RecOneKeycmd.Frame.Data.OneKey.S_Y;
            g_s_task_req.foc_speed = 200;
            g_s_task_req.car_speed = 10;
            g_s_task_req.lift_high = 500;
            g_s_task_req.source    = PARAM_SRC_YUNDUAN;
            g_s_task_req.trigger   = 1;
        }
        memset(&ESP32_Yun_RecOneKeycmd, 0, sizeof(ESP32_Yun_RecOneKeycmd));
    }
    break;

    // ========== 小智一键S路径任务触发 ==========
    case DEV_ONEKEY_XIAOZHI:
    {
        if (grindcar_ctrl.task_S_cnt != 1)
        {
            g_s_task_req.step_x    = ESP32_Xiaozhi_RecOneKeycmd.Frame.Data.OneKey.S_X;
            g_s_task_req.loop_y    = ESP32_Xiaozhi_RecOneKeycmd.Frame.Data.OneKey.S_Y;
            g_s_task_req.foc_speed = 200;
            g_s_task_req.car_speed = 10;
            g_s_task_req.lift_high = 500;
            g_s_task_req.source    = PARAM_SRC_XIAOZHI;
            g_s_task_req.trigger   = 1;
        }
        memset(&ESP32_Xiaozhi_RecOneKeycmd, 0, sizeof(ESP32_Xiaozhi_RecOneKeycmd));
    }
    break;

    default:
    break;
    }

    ESP32_Rec.Frame.DevAddr = 0;
}

void data_to_onnet(void)
{
    uint8_t Frame_Info[15];
// ��ֻ��Ҫ�ڳ����ʼ��ʱִ��һ�Ρ�����������ӣ���ÿ�ο������������һ��


Frame_Info[0]  = 0xAA;        // ֡ͷ�̶�
Frame_Info[1]  = 0x55;        // ֡ͷ�̶�
Frame_Info[2]  = 0x05;        // �̶�����

// ������Щλ�� ���� 0~255 ����� (uint8_t ��Χ)
Frame_Info[3]  = rand() % 256;  // ��� 0-255
Frame_Info[4]  = rand() % 256;
Frame_Info[5]  = rand() % 256;
Frame_Info[6]  = rand() % 256;

Frame_Info[7]  = rand() % 256;
Frame_Info[8]  = rand() % 256;
Frame_Info[9]  = rand() % 256;
Frame_Info[10] = rand() % 256;
Frame_Info[11] = 0xFF;
Frame_Info[12] = 0xFC;
Frame_Info[13] = 0xFF;
Frame_Info[14] = 0xFF;

// ����
//MW_Send(MW_PORT_5_K230, Frame_Info, 15, 0);
MW_Send(MW_PORT_5_K230, Frame_Info, 15, 0);

}
