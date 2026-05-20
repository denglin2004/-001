#include "debug.h"
#include "hal_rgb.h"
#include "hal_beep.h"

#include "can_app.h"
#include "usart_app.h"

#include "scheduler.h"
#include "SSD1306.h"
#include "Timer.h"

int main (void) 
{
    // ========== 系统初始化阶段 ==========
    SystemInit();             // 系统时钟、外设时钟初始化
    Delay_Init(); 
    TIM1_INT_Init (959, 49);  // 定时器1中断初始化，用于任务调度计时
    Drv_RGB_Init();
    Drv_BEEP_Init();
    usart_app_Init();
    can_app_Init();
    SSD1306_Init();
    Scheduler_Setup();
    while (1) 
  {
     Scheduler_Run();
  }
}

 