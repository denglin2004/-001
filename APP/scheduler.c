/******************** (C) COPYRIGHT 2017 ANO Tech ********************************
 * 作者    ：邓林
 * 描述    ：任务调度
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

typedef struct
{
void(*task_func)(void);
uint16_t rate_hz;
uint16_t interval_ticks;
uint32_t last_run;
}sched_task_t;

//////////////////////////////////////////////////////////////////////
//用户程序调度器
//////////////////////////////////////////////////////////////////////

static void Loop_1000Hz(void) //1ms执行一次
{
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
}

static void Loop_500Hz(void) //2ms执行一次
{
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
}

static void Loop_200Hz(void) //5ms执行一次
{
	//////////////////////////////////////////////////////////////////////
    mian_task_run ();
	//////////////////////////////////////////////////////////////////////
}

static void Loop_100Hz(void) //10ms执行一次
{
	//////////////////////////////////////////////////////////////////////
    
	for(uint8_t i = 0; i < 8; i++)
{
	   SSD1306_ShowNum(48,i,ESP32_rx_t.ESP32_rx_buf[i+3],6,8,0);
 
}

	//////////////////////////////////////////////////////////////////////
}

static void Loop_50Hz(void) //20ms执行一次所有数据接受
{
	//////////////////////////////////////////////////////////////////////
   usart_app_Run();
   app_can_data_packet() ;
            //    S_ComandTo_Car(ESP32_rx_t.ESP32_rx_buf[7],
            // ESP32_rx_t.ESP32_rx_buf[7],1,1); 
			//S_ComandTo_Car(1, 60, 1, 1);
	//////////////////////////////////////////////////////////////////////
}

static void Loop_20Hz(void) //50ms执行一次所有数据打包
{
	//////////////////////////////////////////////////////////////////////
   DCCP_RecDataPacket();
   ESP32_RecDataPacket();
	//////////////////////////////////////////////////////////////////////
}

static void Loop_2Hz(void) //500ms执行一次
{
	//    SSD1306_ShowNum(0,1,CAN2A0_Rec.sCAN.ack,6,8,0);
	//    SSD1306_ShowNum(0,2,CAN2A1_Rec.str[0],6,8,0);
	//    SSD1306_ShowNum(0,3,CAN2A2_Rec.str[0],6,8,0);

	//    SSD1306_ShowNum(0,4,canrxbuf[0][0],6,8,0);
	//    SSD1306_ShowNum(0,5,canrxbuf[1][0],6,8,0);
	//    SSD1306_ShowNum(0,6,canrxbuf[2][0],6,8,0);
  SSD1306_ShowStr(0,0,"ADRESS",8,0);
  SSD1306_ShowStr(0,1,"FUNCTION",8,0);
  SSD1306_ShowStr(0,2,"DATA0",8,0);
  SSD1306_ShowStr(0,3,"DATA1",8,0);
  SSD1306_ShowStr(0,4,"DATA2",8,0);
  SSD1306_ShowStr(0,5,"DATA3",8,0);
  SSD1306_ShowStr(0,6,"DATA4",8,0);
  SSD1306_ShowStr(0,7,"DATA5",8,0);

	   DCCP_Disp_Process();   
	 // S_ComandTo_Car(5, 60, 2, 1);
}
//////////////////////////////////////////////////////////////////////
//调度器初始化
//////////////////////////////////////////////////////////////////////
//系统任务配置，创建不同执行频率的“线程”
static sched_task_t sched_tasks[] =
	{
		{Loop_1000Hz, 1000, 0, 0},
		{Loop_500Hz, 500, 0, 0},
		{Loop_200Hz, 200, 0, 0},
		{Loop_100Hz, 100, 0, 0},
		{Loop_50Hz, 50, 0, 0},
		{Loop_20Hz, 20, 0, 0},
		{Loop_2Hz, 2, 0, 0},
};
//根据数组长度，判断线程数量
#define TASK_NUM (sizeof(sched_tasks) / sizeof(sched_task_t))

void Scheduler_Setup(void)
{
	uint8_t index = 0;
	//初始化任务表
	for (index = 0; index < TASK_NUM; index++)
	{
		//计算每个任务的延时周期数
		sched_tasks[index].interval_ticks = 1000 / sched_tasks[index].rate_hz;
		//最短周期为1，也就是1ms
		if (sched_tasks[index].interval_ticks < 1)
		{
			sched_tasks[index].interval_ticks = 1;
		}
	}
}
//这个函数放到main函数的while(1)中，不停判断是否有线程应该执行
void Scheduler_Run(void)
{
	uint8_t index = 0;
	//循环判断所有线程，是否应该执行

	for (index = 0; index < TASK_NUM; index++)
	{
		//获取系统当前时间，单位MS
		uint64_t tnow = GetTick();
		//进行判断，如果当前时间减去上一次执行的时间，大于等于该线程的执行周期，则执行线程
		if (tnow - sched_tasks[index].last_run >= sched_tasks[index].interval_ticks)
		{

			//更新线程的执行时间，用于下一次判断
			sched_tasks[index].last_run = tnow;
			//执行线程函数，使用的是函数指针
			sched_tasks[index].task_func();
		}
	}
}
