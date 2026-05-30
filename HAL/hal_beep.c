#include "hal_beep.h"

void Drv_BEEP_Init(void)
{
BSP_BEEP_Init();
}

void BEEP_On(void)
{
    BEEP_SET_PIN(BEEP_PIN, 1);
}

void BEEP_Off(void)
{
    BEEP_SET_PIN(BEEP_PIN, 0);
}
