#ifndef __HAL_CAN_H
#define __HAL_CAN_H

#include "ch32v30x.h"
#include <stdio.h>

void hal_can_init (void);

u8 hal_Can_Send_Data (u8 *msg, u8 len,u16 canid);
void hal_can_data_rec(uint8_t* canrxbuf1,uint8_t *canrxbuf2,uint8_t* canrxbuf3);

#endif   
