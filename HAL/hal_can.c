#include "hal_can.h"
#include "bsp_can.h"
#include <string.h>

void hal_can_init (void) 
{
//CAN_Mode_Init( CAN_SJW_1tq, CAN_BS2_5tq, CAN_BS1_6tq, 8, CAN_Mode_Normal );//500Kbp
CAN_Mode_Init (CAN_SJW_1tq, CAN_BS2_6tq, CAN_BS1_9tq, 6, CAN_Mode_Normal);  // 500Kbp
}


u8 hal_Can_Send_Data (u8 *msg, u8 len,u16 canid) 
{
    u8 Can_send_Flag;
    Can_send_Flag = CAN_Send_Msg (msg, len,canid);
    return Can_send_Flag;
}

void hal_can_data_rec(uint8_t* canrxbuf1,uint8_t *canrxbuf2,uint8_t* canrxbuf3)
{
bsp_can_data_rec(canrxbuf1,canrxbuf2,canrxbuf3);
}