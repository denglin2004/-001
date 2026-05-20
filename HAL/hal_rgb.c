#include "hal_rgb.h"
#include "bsp_rgb.h"
void Drv_RGB_Init(void)
{
BSP_RGB_Init();
}
// ================= 3. 核心颜色控制函数（推荐） =================
void Drv_RGB_SetColor(RGB_Color_t color)
{
    // 先全部熄灭，再点亮需要的灯
    RGB_SET_PIN(RGB_PIN_RED,   RGB_LED_OFF);
    RGB_SET_PIN(RGB_PIN_GREEN, RGB_LED_OFF);
    RGB_SET_PIN(RGB_PIN_BLUE,  RGB_LED_OFF);

    switch (color)
    {
        case RGB_COLOR_RED:
            RGB_SET_PIN(RGB_PIN_RED, RGB_LED_ON);
            break;
        case RGB_COLOR_GREEN:
            RGB_SET_PIN(RGB_PIN_GREEN, RGB_LED_ON);
            break;
        case RGB_COLOR_BLUE:
            RGB_SET_PIN(RGB_PIN_BLUE, RGB_LED_ON);
            break;
        case RGB_COLOR_YELLOW: // 红 + 绿
            RGB_SET_PIN(RGB_PIN_RED,   RGB_LED_ON);
            RGB_SET_PIN(RGB_PIN_GREEN, RGB_LED_ON);
            break;
        case RGB_COLOR_CYAN:   // 绿 + 蓝
            RGB_SET_PIN(RGB_PIN_GREEN, RGB_LED_ON);
            RGB_SET_PIN(RGB_PIN_BLUE,  RGB_LED_ON);
            break;
        case RGB_COLOR_MAGENTA:// 红 + 蓝
            RGB_SET_PIN(RGB_PIN_RED,  RGB_LED_ON);
            RGB_SET_PIN(RGB_PIN_BLUE, RGB_LED_ON);
            break;
        case RGB_COLOR_WHITE:  // 全亮
            RGB_SET_PIN(RGB_PIN_RED,   RGB_LED_ON);
            RGB_SET_PIN(RGB_PIN_GREEN, RGB_LED_ON);
            RGB_SET_PIN(RGB_PIN_BLUE,  RGB_LED_ON);
            break;
        case RGB_COLOR_OFF:
        default:
            break; // 保持全灭
    }
}
 
// ================= 4. 简单接口（兼容你的旧代码） =================
void Drv_RGB_On(void)
{
    Drv_RGB_SetColor(RGB_COLOR_WHITE);
}
 
void Drv_RGB_Off(void)
{
    Drv_RGB_SetColor(RGB_COLOR_OFF);
}

// ================= 5. 单色控制接口 =================
void Drv_RGB_Red(void)
{
    Drv_RGB_SetColor(RGB_COLOR_RED);
}

void Drv_RGB_Green(void)
{
    Drv_RGB_SetColor(RGB_COLOR_GREEN);
}

void Drv_RGB_Blue(void)
{
    Drv_RGB_SetColor(RGB_COLOR_BLUE);
}
