/********************************** (C) COPYRIGHT  *******************************
* File Name          : debug.h
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : This file contains all the functions prototypes for UART
*                      Printf , Delay functions.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#ifndef __DEBUG_H
#define __DEBUG_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stdio.h"
#include "ch32v30x.h"

#define DEBUG_UART1    1
#define DEBUG_UART2    2
#define DEBUG_UART3    3
#define DEBUG_UART5    5

#ifndef DEBUG
#define DEBUG   DEBUG_UART5
#endif

#define SDI_PR_CLOSE   0
#define SDI_PR_OPEN    1

#ifndef SDI_PRINT
#define SDI_PRINT   SDI_PR_CLOSE
#endif


// ===================== 新增：UART5 接收缓冲区 =====================
#define UART5_RX_BUF_LEN 128
// 定义4个轮胎的PID结构体 ← 就是这里补上，解决 'tire' undeclared
 typedef struct {
     float Kp;
     float Ki;
     float Kd;
 } TirePID;

extern uint16_t uart5_rx_idx;
extern uint8_t  uart5_rx_done;
extern TirePID tire[4];

extern const uint32_t VOFA_FRAME_HEAD;
extern const uint32_t VOFA_FRAME_TAIL;
// void VOFA_Send_PID_Data(void);  // 专门发送4电机PID数据
void Delay_Init(void);
void Delay_Us (uint32_t n);
void Delay_Ms (uint32_t n);
void USART_Printf_Init(uint32_t baudrate);
void SDI_Printf_Enable(void);
void UART5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART5_IRQHandler(void) ;

#ifdef __cplusplus
}
#endif 
#endif 


