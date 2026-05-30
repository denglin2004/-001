#ifndef __TIMER_H
#define __TIMER_H

#include <stdlib.h>  // 包含malloc所需的头文件
#include "ch32v30x.h"


extern  uint32_t sys_tick;
void TIM1_user_Init( u16 arr, u16 psc);
void TIM8_sys_Init( u16 arr, u16 psc);

uint64_t GetTick(void);
#endif