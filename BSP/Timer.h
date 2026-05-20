#ifndef __TIMER_H
#define __TIMER_H

#include <stdlib.h>  // 包含malloc所需的头文件
#include "ch32v30x.h"
typedef struct {
    uint16_t Pid_Timer;
    uint16_t CanRec_Timer;
    uint16_t CanSend_Timer;
    uint16_t Run_Timer;
    uint16_t Usart_Timer;
    uint16_t OLED_Timer;

}Task_HandleTypeDer ;


#define CanRecive_rate 50
#define CanSend_rate 100
#define Pid_rate 15
#define Usr_rate 111
#define Run_rate 20
#define OLED_rate 75
extern Task_HandleTypeDer  Task_Timer;
extern  uint32_t sys_tick;
void TIM1_INT_Init( u16 arr, u16 psc);
uint64_t GetTick(void);
#endif