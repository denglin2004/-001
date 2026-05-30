#include "bsp_rgb.h"

// ================= 1. 底层辅助宏（内部使用，简化代码） =================
// 控制单个引脚电平


// ================= 2. 初始化函数 =================
void BSP_RGB_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 1. 使能 GPIO 时钟
    RCC_APB2PeriphClockCmd(RGB_GPIO_CLK, ENABLE);

    // 2. 配置引脚为推挽输出 (50MHz)
    GPIO_InitStructure.GPIO_Pin   = RGB_PIN_RED | RGB_PIN_GREEN | RGB_PIN_BLUE;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(RGB_GPIO_PORT, &GPIO_InitStructure);

    // 3. 初始状态：全灭（安全起见）
    RGB_SET_PIN(RGB_PIN_RED,   RGB_LED_OFF);
    RGB_SET_PIN(RGB_PIN_GREEN, RGB_LED_OFF);
    RGB_SET_PIN(RGB_PIN_BLUE,  RGB_LED_OFF);
}
