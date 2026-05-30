#ifndef __BSP_RGB_H
#define __BSP_RGB_H

#include "ch32v30x.h"
#include <stdint.h>
#include <stdbool.h>

#define RGB_SET_PIN(pin, state)   GPIO_WriteBit(RGB_GPIO_PORT, (pin), (state))
// ================= 1. 硬件引脚定义（方便移植，这里改引脚即可） =================
#define RGB_GPIO_PORT       GPIOC
#define RGB_GPIO_CLK        RCC_APB2Periph_GPIOC

#define RGB_PIN_RED         GPIO_Pin_7    // 红色灯引脚
#define RGB_PIN_GREEN       GPIO_Pin_6    // 绿色灯引脚
#define RGB_PIN_BLUE        GPIO_Pin_8    // 蓝色灯引脚

// ================= 2. 电平逻辑定义（明确低电平亮） =================
#define RGB_LED_ON          Bit_RESET     // 低电平亮
#define RGB_LED_OFF         Bit_SET       // 高电平灭

// ================= 3. 颜色枚举（可选，让上层调用更优雅） =================
typedef enum {
    RGB_COLOR_OFF = 0,      // 全灭
    RGB_COLOR_RED,          // 红
    RGB_COLOR_GREEN,        // 绿
    RGB_COLOR_BLUE,         // 蓝
    RGB_COLOR_YELLOW,       // 黄 (红+绿)
    RGB_COLOR_CYAN,         // 青 (绿+蓝)
    RGB_COLOR_MAGENTA,      // 紫 (红+蓝)
    RGB_COLOR_WHITE         // 白 (全亮)
} RGB_Color_t;

void BSP_RGB_Init(void);

#endif
