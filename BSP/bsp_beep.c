#include "bsp_beep.h"



// ================= 2. 初始化函数 =================
void BSP_BEEP_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 1. 使能 GPIO 时钟
    RCC_APB2PeriphClockCmd(BEEP_GPIO_CLK, ENABLE);

    // 2. 配置引脚为推挽输出 (50MHz)
    GPIO_InitStructure.GPIO_Pin   = BEEP_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BEEP_GPIO_PORT, &GPIO_InitStructure);

    // 3. 初始状态：全灭（安全起见）
    BEEP_SET_PIN(BEEP_PIN, 0);
}


