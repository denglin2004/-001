#include "Timer.h"
#include "usart_app.h"
#include "can_app.h"

uint32_t sys_tick = 0;  // 全局毫秒计数器
void TIM1_user_Init( u16 arr, u16 psc)
{

    NVIC_InitTypeDef NVIC_InitStructure={0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE );

    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 1;
    TIM_TimeBaseInit( TIM1, &TIM_TimeBaseInitStructure);

    TIM_ClearITPendingBit( TIM1, TIM_IT_Update );

    NVIC_InitStructure.NVIC_IRQChannel =TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;
    NVIC_InitStructure.NVIC_IRQChannelCmd =ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    TIM_Cmd( TIM1, ENABLE );

}

void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM1, TIM_IT_Update)==SET)
    {   
    TIM_ClearITPendingBit( TIM1, TIM_IT_Update );
    sys_tick++;
    }
    
}
// 实现毫秒级时间获取函数
uint64_t GetTick(void) 
{
    return  sys_tick;
}
void TIM8_sys_Init( u16 arr, u16 psc)
{

    NVIC_InitTypeDef NVIC_InitStructure={0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE );

    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 1;
    TIM_TimeBaseInit( TIM8, &TIM_TimeBaseInitStructure);

    TIM_ClearITPendingBit( TIM8, TIM_IT_Update );

    NVIC_InitStructure.NVIC_IRQChannel =TIM8_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;
    NVIC_InitStructure.NVIC_IRQChannelCmd =ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM8, TIM_IT_Update, ENABLE);
    TIM_Cmd( TIM8, ENABLE );

}


void sys_task(void)
{
    	static u16 tmp_cnt[2];
	//计10ms
	tmp_cnt[0]++;
	tmp_cnt[0] %= 5;
	if (tmp_cnt[0] == 0)
	{   //串口数据处理
        usart_app_Run();
		//计100ms
		tmp_cnt[1]++;
		tmp_cnt[1] %= 25;
		if (tmp_cnt[1] == 0)
		{
        //can总线数据处理  
        app_can_data_packet() ;
		}
	}




}
void TIM8_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM8_UP_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM8, TIM_IT_Update)==SET)
    {   
    TIM_ClearITPendingBit( TIM8, TIM_IT_Update);
    sys_task();
    }
    
}