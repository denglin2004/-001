#ifndef __BSP_BEEP_H
#define __BSP_BEEP_H

#include "ch32v30x.h"
#include <stdint.h>
#include <stdbool.h>

// ================= 1. 硬件引脚定义（方便移植，这里改引脚即可） =================
#define BEEP_GPIO_PORT       GPIOB
#define BEEP_GPIO_CLK        RCC_APB2Periph_GPIOB

#define BEEP_PIN        GPIO_Pin_8    // 红色灯引脚

// ================= 2. 电平逻辑定义（明确低电平亮） =================
#define BEEP_ON          Bit_RESET     // 低电平亮
#define BEEP_OFF         Bit_SET       // 高电平灭


// ================= 1. 底层辅助宏（内部使用，简化代码） =================
// 控制单个引脚电平
#define BEEP_SET_PIN(pin, state)   GPIO_WriteBit(BEEP_GPIO_PORT, (pin), (state))

void BSP_BEEP_Init(void);


#endif
