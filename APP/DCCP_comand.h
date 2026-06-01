#ifndef  __DCCP_COMAND_H__
#define  __DCCP_COMAND_H__

#include "ch32v30x.h"

#ifdef __cplusplus
extern "C" {
#endif

/* DCCP命令处理临时变量（定义） */
/* DCCP 命令处理临时变量 */
typedef struct {
    uint8_t foc_speed;      /* FOC电机转速临时值 */
    uint8_t car_speed; 
    uint8_t lift_high; 
} DCCP_Temp_Typedef;
extern DCCP_Temp_Typedef g_dccp_temp ;



void S_Comand_Control_Car(void);
void Button_Control_Car(void);
void DCCP_comand_process(void) ;
void DCCP_RecDataPacket( void );
void DCCP_Disp_Process(void);   
void DCCP_SendFrameInfo(uint16_t Send_Len );
#endif  /* __DCCP_COMAND_H__ */
