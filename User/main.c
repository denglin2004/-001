#include "debug.h"
#include "hal_rgb.h"
#include "hal_beep.h"
#include "hal_adc.h"

#include "can_app.h"
#include "usart_app.h"
#include "bsp_spi.h"

#include "scheduler.h"
#include "SSD1306.h"
#include "Timer.h"

int main (void) 
{
    // ========== 系统初始化阶段 ==========
    SystemInit();             // 系统时钟、外设时钟初始化
    Delay_Init(); 
    TIM8_sys_Init (959, 99);   // 定时器8中断初始化，用于数据输入输出处理调度计时
    TIM1_user_Init (959, 49);  // 定时器1中断初始化，用于自定义任务调度计时
    Drv_RGB_Init();
    Drv_BEEP_Init();
    Drv_Pot_Init();
    usart_app_Init();
    can_app_Init();
    SSD1306_Init();
    SPI_Flash_Init();
    Scheduler_Setup();
  // Flash_Model = SPI_Flash_ReadID();  // 调试查看：Flash_Model = 芯片ID
    while (1) 
  {
     Scheduler_Run();
  }
}

 