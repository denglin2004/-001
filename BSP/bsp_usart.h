#ifndef __BSP_USART_H
#define __BSP_USART_H

#include "ch32v30x.h"

// 串口枚举（仅用于初始化区分）
typedef enum
{
    BSP_USART1 = 0,
    BSP_USART2 = 1,
    BSP_USART3 = 2,
    BSP_UART4  = 3,
    BSP_UART5  = 4,
} BSP_USART_TypeDef;
// BSP 唯一接口：只初始化硬件
void BSP_USART_Init(BSP_USART_TypeDef usartx, uint32_t baudrate);

#endif   