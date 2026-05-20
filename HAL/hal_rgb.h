#ifndef __HAL_RGB_H__
#define __HAL_RGB_H__
#include "bsp_rgb.h"


// ================= 4. 函数接口声明 =================
void Drv_RGB_Init(void);                    // 初始化
void Drv_RGB_SetColor(RGB_Color_t color);  // 设置颜色（推荐用这个）
void Drv_RGB_On(void);                      // 全亮（兼容旧接口）
void Drv_RGB_Off(void);                     // 全灭（兼容旧接口）

// 单色控制（按需使用）
void Drv_RGB_Red(void);
void Drv_RGB_Green(void);
void Drv_RGB_Blue(void);

#endif  
