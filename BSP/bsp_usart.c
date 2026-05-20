#include "bsp_usart.h"

// 获取串口基地址
static USART_TypeDef *BSP_USART_GetHandle(BSP_USART_TypeDef usartx)
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

// BSP 唯一实现：纯硬件初始化，无任何收发逻辑
void BSP_USART_Init(BSP_USART_TypeDef usartx, uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    USART_TypeDef *USARTx = BSP_USART_GetHandle(usartx);
    
    // 新增：用于存储不同串口的优先级变量
    uint8_t irq_preempt_prio = 1;
    uint8_t irq_sub_prio = 1;

    // ========== 1. 时钟使能 & 引脚配置 ==========
    switch(usartx)
    {
        case BSP_USART1:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;  // TX
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOA, &GPIO_InitStructure);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // RX
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOA, &GPIO_InitStructure);
            NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
            irq_preempt_prio = 1; // 最高优先级
            irq_sub_prio = 1;
            break;

        case BSP_USART2:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOA, &GPIO_InitStructure);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOA, &GPIO_InitStructure);
            NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
            irq_preempt_prio = 2; 
            irq_sub_prio = 1;
            break;

        case BSP_USART3:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOB, &GPIO_InitStructure);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOB, &GPIO_InitStructure);
            NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
            irq_preempt_prio = 2;
            irq_sub_prio = 2;
            break;
            
        case BSP_UART4:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
            NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
            irq_preempt_prio = 3;
            irq_sub_prio = 1;
            break;
            
        case BSP_UART5:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOC, &GPIO_InitStructure);
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(GPIOD, &GPIO_InitStructure);
            NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
            irq_preempt_prio = 3;
            irq_sub_prio = 2;
            break;
    }

    // ========== 2. 串口参数配置 ==========
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USARTx, &USART_InitStructure);

    // ========== 3. 中断配置 (使用上面分配的变量) ==========
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = irq_preempt_prio; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = irq_sub_prio;             
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // ========== 4. 开启接收中断 + 串口外设 ==========
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
    USART_Cmd(USARTx, ENABLE);
}
