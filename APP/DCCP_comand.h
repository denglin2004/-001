#ifndef  __DCCP_COMAND_H__
#define  __DCCP_COMAND_H__

#include "ch32v30x.h"
#ifdef __cplusplus
extern "C" {
#endif


void Button_Control_Car(void);
void DCCP_comand_process(void) ;
void DCCP_RecDataPacket( void );
void DCCP_Disp_Process(void);   
void DCCP_SendFrameInfo(uint16_t Send_Len );
#endif  /* __DCCP_COMAND_H__ */
