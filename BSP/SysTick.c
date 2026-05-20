// #include "SysTick.h"
// void SYSTICK_Init(int64_t ticks)
// {
//     SysTick->SR &= ~(1 << 0);//clear State flag
//     SysTick->CMP = ticks;
//     SysTick->CNT = 0;
//     SysTick->CTLR = 0xF;

//     NVIC_SetPriority(SysTicK_IRQn, 15);
//     NVIC_EnableIRQ(SysTicK_IRQn);
// }

// uint32_t counter = 0;
// void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
// void SysTick_Handler(void)
// {
//     if(SysTick->SR == 1)
//     {
//         SysTick->SR = 0;//clear State flag
//         counter++;
//     }
// }

// // 实现毫秒级时间获取函数
// uint32_t GetTick(void) 
// {
//     return  counter;
// }
