/******************** (C) COPYRIGHT 2017 ANO Tech ********************************
 * �ļ���    : scheduler.c
 * ��������  : ����������������̶�Ƶ��ִ�в�ͬ����?
**********************************************************************************/

#include "scheduler.h"
#include "Timer.h"
#include "SSD1306.h"
#include "usart_app.h"
#include "can_app.h"
#include "bsp_can.h"
#include "DCCP_comand.h"
#include "ESP32_comand.h"
#include "core_control.h"
#include "hal_usart.h"
#include "safety_monitor.h"
//////////////////////////////////////////////////////////////////////
// �����������û��Զ���������������ͬƵ�����У�
//////////////////////////////////////////////////////////////////////

/**
 * @brief  1000Hz����ÿ1msִ��һ�Σ�
 */
static void Loop_1000Hz(void)
{
    // ��ʵʱ����������̬���㡢�������ɼ��ȣ�
}

/**
 * @brief  500Hz����ÿ2msִ��һ�Σ�
 */
static void Loop_500Hz(void)
{
    // ��Ƶ��������
}

/**
 * @brief  200Hz����ÿ5msִ��һ�Σ�
 */
static void Loop_200Hz(void)
{
    // �и�Ƶ��������
}

/**
 * @brief  100Hz����ÿ10msִ��һ�Σ�
 */
static void Loop_100Hz(void)
{
    onekey_task_run();  // һ������������
}

/**
 * @brief  50Hz����ÿ20msִ��һ�Σ������ݽ���
 */
static void Loop_50Hz(void)
{
    SafetyMonitor_Task(); 
}

/**
 * @brief  20Hz����ÿ50msִ��һ�Σ������ݷ���/������
 */
static void Loop_20Hz(void)
{
    mian_task_run();  // ����������
}

/**
 * @brief  2Hz����ÿ500msִ��һ�Σ���OLED��ʾˢ��
 */
static void Loop_2Hz(void)
{
    // // ��ʾ�̶���������
    // SSD1306_ShowStr(0, 0, "ADRESS", 8, 0);
    // SSD1306_ShowStr(0, 1, "FUNCTION", 8, 0);
    // SSD1306_ShowStr(0, 2, "DATA0", 8, 0);
    // SSD1306_ShowStr(0, 3, "DATA1", 8, 0);
    // SSD1306_ShowStr(0, 4, "DATA2", 8, 0);
    // SSD1306_ShowStr(0, 5, "DATA3", 8, 0);
    // SSD1306_ShowStr(0, 6, "DATA4", 8, 0);
    // SSD1306_ShowStr(0, 7, "DATA5", 8, 0);

    // // ѭ����ʾESP32���յ�����
    // for(uint8_t i = 0; i < 7; i++)
    // {
    //     SSD1306_ShowNum(48, i, ESP32_rx_t.ESP32_rx_buf[i+3], 6, 8, 0);
    // }

    DCCP_Disp_Process();  // DCCP��ʾ����
    data_to_onnet();
	// ��ʾ״̬��ռ�ݵ�0-2�У�
      OLED_DisplayStatus(0);
      // ��5-7����ʾCAN��������


    //   SSD1306_ShowStr(0, 7, "XY:", 8, 0);
    //   SSD1306_ShowNum(0, 6, CAN2A0_Rec.sCAN.ack, 3, 8, 0);
    //   SSD1306_ShowNum(40, 6,CAN2A1_Rec.sCAN.pzd_function_code, 3, 8, 0);
    //   SSD1306_ShowNum(80, 6,CAN2A2_Rec.sCAN.FOC_Start, 3, 8, 0);    
    //   SSD1306_ShowNum(40, 7, grindcar_ctrl.foc.foc_speed_set, 3, 8, 0);

 

}

//////////////////////////////////////////////////////////////////////
// ����������������
//////////////////////////////////////////////////////////////////////

// ϵͳ������ȱ��������������������Ƶ��
static sched_task_t sched_tasks[] =
{
    {Loop_1000Hz, 1000, 0, 0},  // 1000Hz ����
    {Loop_500Hz,  500,  0, 0},  // 500Hz  ����
    {Loop_200Hz,  200,  0, 0},  // 200Hz  ����
    {Loop_100Hz,  100,  0, 0},  // 100Hz  ����
    {Loop_50Hz,    50,  0, 0},  // 50Hz   ����
    {Loop_20Hz,    20,  0, 0},  // 20Hz   ����
    {Loop_2Hz,      2,  0, 0},  // 2Hz    ����
};

// �Զ�������������
#define TASK_NUM (sizeof(sched_tasks) / sizeof(sched_task_t))

/**
 * @brief  ��������ʼ��������ÿ�������ִ�м����ms��
 * @param  ��
 * @return ��
 */
void Scheduler_Setup(void)
{
    uint8_t index = 0;

    // �����������񣬼���ʱ����
    for (index = 0; index < TASK_NUM; index++)
    {
        // ����Ƶ�ʼ���������λ��ms��
        sched_tasks[index].interval_ticks = 1000 / sched_tasks[index].rate_hz;

        // ��С����?1ms����ֹ�������?
        if (sched_tasks[index].interval_ticks < 1)
        {
            sched_tasks[index].interval_ticks = 1;
        }
    }
}

/**
 * @brief  ��������ѭ��������while(1)�У���ѯ�жϲ�ִ������
 * @param  ��
 * @return ��
 */
void Scheduler_Run(void)
{
    uint8_t index = 0;

    // ��ѯ�����������?
    for (index = 0; index < TASK_NUM; index++)
    {
        uint32_t tnow = GetTick();  // ��ȡ��ǰϵͳʱ�䣨ms��

        // �жϣ�ʱ�䵽�� �� ִ������
        if (tnow - sched_tasks[index].last_run >= sched_tasks[index].interval_ticks)
        {
            // �����ϴ�����ʱ�䣨�ۼӷ�ʽ����ʱ��Ư�ƣ�
            sched_tasks[index].last_run += sched_tasks[index].interval_ticks;

            // ִ��������
            sched_tasks[index].task_func();
        }
    }
}
