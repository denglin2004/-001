/*
 * UI.c
 *
 *  Created on: Apr 21, 2024
 *      Author: liu
 */
#include "UI.h"

void UI_Init(void){
    SSD1306_Init();//oledµÄIICÓ²¼þÊäÈë
}
void UI_Show(void){
    SSD1306_ShowStr(0, 0, "successful initialization", SSD1306_SIZE_16, SSD1306_PO);
}
