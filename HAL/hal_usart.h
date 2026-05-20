#ifndef __HAL_USART_H_
#define __HAL_USART_H_          
#include "bsp_usart.h"   

#include <stdint.h>

#define UART_RXD_MAX       128
#define HAL_USART_CNT      5
extern uint16_t TEST_RX[16];
// 回调函数类型
typedef void (*HAL_UART_CallbackTypeDef)(uint8_t *buf, uint16_t len);

// 初始化（调用BSP初始化）
void HAL_UART_Init(BSP_USART_TypeDef usartx, uint32_t baudrate);

// 注册接收回调
void HAL_UART_RegisterCallback(BSP_USART_TypeDef usartx, HAL_UART_CallbackTypeDef cb);

// 发送数据（HAL实现）
void HAL_UART_Send(BSP_USART_TypeDef usartx, uint8_t *data, uint16_t len);

// 主循环轮询处理
void HAL_UART_Process(void);
#endif