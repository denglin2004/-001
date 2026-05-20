#ifndef __I2C_SOFT_H
#define __I2C_SOFT_H

#include "ch32v30x.h"  // CH32V307核心库头文件，包含所有外设定义
#include <stdint.h>    // 标准整数类型（uint8_t、uint16_t等）
#include "Delay.h"

#define I2C_SOFT_GLOBAL_DELAY() Delay_Us(1)

typedef enum
{
	ACK	 = Bit_RESET,
	NACK = Bit_SET,
}ack_t;

typedef struct
{
	void (*Init)(void);
	void (*Start)(void);
	void (*Stop)(void);
	ack_t (*SendData)(uint8_t);
	uint8_t (*ReceiveData) (ack_t);
}I2C_soft_t;

#define I2C_PORT   GPIOB
#define I2C_SCL_PIN1    GPIO_Pin_0
#define I2C_SDA_PIN1    GPIO_Pin_1

#define I2C_SET_SCL()   GPIO_WriteBit(I2C_PORT, I2C_SCL_PIN1, Bit_SET)
#define I2C_RESET_SCL() GPIO_WriteBit(I2C_PORT, I2C_SCL_PIN1, Bit_RESET)
#define I2C_SET_SDA()   GPIO_WriteBit(I2C_PORT, I2C_SDA_PIN1, Bit_SET)
#define I2C_RESET_SDA() GPIO_WriteBit(I2C_PORT, I2C_SDA_PIN1, Bit_RESET)

#define I2C_RD_SDA()    GPIO_ReadOutputDataBit(I2C_PORT, I2C_SDA_PIN1)

extern I2C_soft_t  I2C_Soft;

#endif
