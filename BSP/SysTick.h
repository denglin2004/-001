#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "ch32v30x.h"
void SYSTICK_Init(int64_t ticks);
uint32_t GetTick(void);
#endif   