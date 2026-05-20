#include "hal_usart.h"
#include <string.h>
#include "Timer.h"

// 接收缓存结构体（HAL私有）
typedef struct
{
    uint8_t  buf[UART_RXD_MAX];
    uint16_t len;
    uint32_t last_time; // 【新增】记录这包数据最后更新的时间
    HAL_UART_CallbackTypeDef cb;
} HAL_UART_RxTypeDef;

// 5个串口缓存
static HAL_UART_RxTypeDef s_uart[HAL_USART_CNT] = {0};

// 获取串口寄存器
static USART_TypeDef *HAL_UART_GetHandle(BSP_USART_TypeDef usartx)
{
    switch(usartx)
    {
        case BSP_USART1: return USART1;
        case BSP_USART2: return USART2;
        case BSP_USART3: return USART3;
        case BSP_UART4:  return UART4;
        case BSP_UART5:  return UART5;
        default:         return USART1;
    }
}

// 1. HAL初始化 → 调用BSP初始化硬件
void HAL_UART_Init(BSP_USART_TypeDef usartx, uint32_t baudrate)
{
    BSP_USART_Init(usartx, baudrate); // 只调用BSP初始化
}

// 2. 注册回调
void HAL_UART_RegisterCallback(BSP_USART_TypeDef usartx, HAL_UART_CallbackTypeDef cb)
{
    if(usartx < HAL_USART_CNT)
        s_uart[usartx].cb = cb;
}

// 3. 发送数据（HAL实现，直接操作寄存器，BSP不参与）
void HAL_UART_Send(BSP_USART_TypeDef usartx, uint8_t *data, uint16_t len)
{
    USART_TypeDef *USARTx = HAL_UART_GetHandle(usartx);
    for(uint16_t i=0; i<len; i++)
    {
        while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
        USART_SendData(USARTx, data[i]);
    }
}

// 4. 主循环数据处理
void HAL_UART_Process(void)
{
    for(uint8_t i=0; i<HAL_USART_CNT; i++)
    {
        if(s_uart[i].len > 0 && 
           s_uart[i].cb != NULL && 
           (GetTick() - s_uart[i].last_time > 20))
        {
            // 条件满足，把这一整包数据传给中间件
            s_uart[i].cb(s_uart[i].buf, s_uart[i].len);
            // 处理完后，清空缓冲区
            memset(s_uart[i].buf, 0, UART_RXD_MAX);
            s_uart[i].len = 0;
            s_uart[i].last_time = 0;
        }
    }
}
u8 ii=0;
uint16_t TEST_RX[16]={0};
/************************ 中断服务函数（HAL层实现，BSP不碰） ************************/
void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART1_IRQHandler(void)
{

    uint8_t temp;
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        temp = USART_ReceiveData(USART1);
        
    if(ii<15)
   {
   TEST_RX[ii++]=temp ;  
   }
   else 
   {
       ii=0;
   }
        if(s_uart[BSP_USART1].len < UART_RXD_MAX)
            s_uart[BSP_USART1].buf[s_uart[BSP_USART1].len++] = temp;
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART2_IRQHandler(void)
{
    uint8_t temp;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        temp = USART_ReceiveData(USART2);
        if(s_uart[BSP_USART2].len < UART_RXD_MAX)
        {
            s_uart[BSP_USART2].buf[s_uart[BSP_USART2].len++] = temp;
            s_uart[BSP_USART2].last_time = GetTick(); 
        }

        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART3_IRQHandler(void)
{
    uint8_t temp;
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        temp = USART_ReceiveData(USART3);
        if(s_uart[BSP_USART3].len < UART_RXD_MAX)
            s_uart[BSP_USART3].buf[s_uart[BSP_USART3].len++] = temp;
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}

void USART4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART4_IRQHandler(void)
{
    uint8_t temp;
    if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
    {
        temp = USART_ReceiveData(UART4);
        if(s_uart[BSP_UART4].len < UART_RXD_MAX)
            s_uart[BSP_UART4].buf[s_uart[BSP_UART4].len++] = temp;
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
    }
}

void USART5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART5_IRQHandler(void)
{
    uint8_t temp;
    if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
    {
        temp = USART_ReceiveData(UART5);
        if(s_uart[BSP_UART5].len < UART_RXD_MAX)
            s_uart[BSP_UART5].buf[s_uart[BSP_UART5].len++] = temp;
        USART_ClearITPendingBit(UART5, USART_IT_RXNE);
    }
}
