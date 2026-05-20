#ifndef _DELAY_H_
#define _DELAY_H_

#include "ch32v30x.h"

// 声明延时初始化函数
void Delay_Init(void);

// 声明微秒延时函数
void Delay_Us(uint32_t n);

// 声明毫秒延时函数
void Delay_Ms(uint32_t n);

#endif
