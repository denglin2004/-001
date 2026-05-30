#ifndef __BSP_CAN_H_
#define __BSP_CAN_H_

#include "ch32v30x.h"

 #define TX_MODE   0
 #define RX_MODE   1
 #define Standard_Frame   1
 #define Extended_Frame   0
//#define CAN_MODE   TX_MODE
#define CAN_MODE   RX_MODE
 #define Frame_Format   Standard_Frame
//#define Frame_Format   Extended_Frame
 #define USE_INTERRUPT
//#define USE_SOFT_FILTER
 #define CANSOFTFILTER_MAX_GROUP_NUM 2           // The maximum recommended configuration is 14. 								
 #define CANSOFTFILER_PREDEF_CTRLBYTE_MASK32 ((CAN_FilterScale_32bit << 5) | (CAN_FilterMode_IdMask << 1))
 #define CANSOFTFILER_PREDEF_CTRLBYTE_ID32   ((CAN_FilterScale_32bit << 5) | (CAN_FilterMode_IdList << 1))
extern uint8_t canrxbuf[3][8];
 void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
 void CAN_SoftSlaveStartBank(uint8_t CAN_BankNumber);
 void CAN_SoftFilterInit(CAN_FilterInitTypeDef* CAN_FilterInitStruct);
 void CAN_Mode_Init(u8 tsjw, u8 tbs2, u8 tbs1, u16 brp, u8 mode);
 void CAN_ReceiveViaSoftFilter(CAN_TypeDef* CANx, uint8_t FIFONumber, CanRxMsg* RxMessage);
u8 CAN_Send_Msg( u8 *msg, u8 len,u16 can_id );
u8 CAN_Receive_Msg(u8 *buf);
void bsp_can_data_rec(uint8_t *can_rxbuf1,uint8_t *can_rxbuf2,uint8_t* can_rxbuf3);
extern uint8_t canrxbuf[3][8]; 
 #endif
