#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include "ch32v30x.h"
u16 SPI_Flash_ReadID(void);
void SPI_Flash_Init(void);
extern volatile u16 Flash_Model; 
#endif
