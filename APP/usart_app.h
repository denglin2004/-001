#ifndef __USART_APP_H__
#define __USART_APP_H__
#include <stdio.h>
typedef struct
{
    uint8_t DCCP_rx_buf[20];
    uint8_t DCCP_rx_len;
} DCCP_rx_hander;
extern DCCP_rx_hander DCCP_rx_t;

typedef struct
{
    uint8_t ESP32_rx_buf[20];
    uint8_t ESP32_rx_len;
} ESP32_rx_hander;
extern ESP32_rx_hander ESP32_rx_t;

void usart_app_Init(void);
void usart_app_Run(void);
#endif
