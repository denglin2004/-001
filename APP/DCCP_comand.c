#include "DCCP_comand.h"
#include "DCCP_struct.h"
#include "can_app.h"
#include "usart_app.h"
#include "usart_protocol.h"
#include <string.h>
#include "hal_rgb.h"
#include "Timer.h"
#include "core_control.h"



// 时间系数（根据你的需求定义）
#define K_time      1   
/*
************************************************************************************** 
* 函 数 名: DatachagASCII
* 功能说明: 无符号数字转换为ASCII码字符串（用于屏幕显示）
* 入口参数: dispWord-待转换的数字   str-存储转换后的ASCII字符串
*           Len_Change-转换的数字长度
* 返 回 值: 无
* 关键修改：高位补0（0x30）替代空格（0x20），消除20 20问题
************************************************************************************** 
*/
void DatachagASCII (unsigned char * str, unsigned int dispWord , unsigned char Len_Change){
	unsigned int tmp_Data;	
	signed char i;	

	tmp_Data = dispWord;
	// 从高位到低位逐位转换
	for( i = Len_Change; i >=0 ; i-- ) {	
		if( tmp_Data > 0 ) {
			str[ i ] = tmp_Data % 10 + 0x30;  // 数字转ASCII字符
			tmp_Data /= 10;
		}
		else {
			str[ i ] = 0x30;  // ? 核心修改：高位补0（0x30），不再补空格
		}
	}
}

/*
************************************************************************************** 
* 函 数 名: ASCIIchagData
* 功能说明: ASCII码字符串转换为无符号数字
* 入口参数: Rstr-待转换的ASCII字符串   LEN_Rstr-字符串长度
* 返 回 值: 转换后的数字
************************************************************************************** 
*/
unsigned int ASCIIchagData (unsigned char * Rstr, unsigned char LEN_Rstr){
	unsigned int Rtmp_Data = 0 ;	
	unsigned int Rtmp_Data0 = 0 ;
	unsigned char Ri;	
	// 逐位解析ASCII字符为十进制数字
	for( Ri = 0 ; Ri < LEN_Rstr ; Ri++ ) {	
		if( Rstr[Ri] != 0 ){
			Rtmp_Data0 = (Rstr[ Ri ] - 0x30) ;
			Rtmp_Data = Rtmp_Data*10 + Rtmp_Data0;
		}
		else{
			break;
		}
	}	
	return Rtmp_Data ;
}
/*
**************************************************************************************
* 函 数 名: Car_Dir_Control
* 功能说明: 按钮按下/松开 处理（计时）
* 按下：开始计时
* 松开：计算运动时间，启动小车运动
**************************************************************************************
*/
static void Car_Dir_Control(uint8_t idx, uint8_t dir, uint8_t cmdParam)
{
    // 仅非按键运动模式可用，模式2直接退出
    if(g_car.ExForm_mode == 2) return;

    // 【按钮按下：仅第一次收到1时执行1次，循环调用不重复刷新计时】
    if((cmdParam == 0x01) && (g_car.timer_en[idx] == 0))
    {
        g_car.timer_en[idx] = 1;
        g_car.start_tick[idx] = GetTick();  // 记录按键按下的起始时刻
        g_car.run_flag[idx] = 0;            // 清除运动完成标志
        Drv_RGB_Green();                     // 按键按下调试灯
    }
    // 【按钮松开：仅第一次收到0时执行1次，循环调用不重复执行】
    else if((cmdParam == 0x00) && (g_car.timer_en[idx] == 1))
    {
        g_car.timer_en[idx] = 0;
        // 计算按键按下的总时长 = 小车需要运动的总时长(ms)
        g_car.run_time[idx] = (GetTick() - g_car.start_tick[idx]) * TICK_MS;
        g_car.ExForm_mode = 1;                // 进入按键运动模式
        g_car.ExForm_diretcion = dir;         // 锁定本次运动方向
        g_car.start_tick[idx] = GetTick();    // 【关键】记录小车运动开始的时刻
    }
}



// /*
// **************************************************************************************
// * 函 数 名: Button_Control_Car
// * 功能说明: 按钮松开后，按计时时间驱动小车运动（主循环轮询调用）
// **************************************************************************************
// */
// void Button_Control_Car(void)
// {
//     // 所有变量统一在函数开头声明，完全兼容CH32的C89编译标准，彻底解决编译报错
//     uint8_t idx = 0;
//     uint32_t pass_time = 0;
//     // 仅按键控制模式执行，非目标模式直接退出
//     if(g_car.ExForm_mode != 1) return;
//     if(g_car.ExForm_diretcion == 0) return;

//     // 方向匹配对应通道索引
//     switch(g_car.ExForm_diretcion)
//     {
//         case 0x01: idx = 0; break;
//         case 0x02: idx = 1; break;
//         case 0x03: idx = 2; break;
//         case 0x04: idx = 3; break;
//         default: return;
//     }

//     // 该方向运动已完成，直接退出避免重复执行
//     if(g_car.run_flag[idx] == 1) return;
    
//     // 【核心BUG修复】正确计算小车已运动时长(ms)
//     // 公式：当前时刻 - 运动开始时刻 = 真实已运行时间，完全符合物理逻辑
//     pass_time = (GetTick() - g_car.start_tick[idx]) * TICK_MS;

//     // ================== 核心运动控制逻辑 ==================
//     // 1. 运动时长未到：持续执行运动逻辑+调试RGB
//     if(pass_time < g_car.run_time[idx])
//     { 

//         switch(g_car.ExForm_diretcion)
//         {
//             case 0x01: 
//                 S_ComandTo_Car(1, 50, 1, 1);  // 前进电机控制，调试完成可放开
//                 Drv_RGB_Blue();         // 调试RGB，循环执行不影响调试效果
//                 break;
//             case 0x02: 
//                 S_ComandTo_Car(2, 50, 1, 2);  // 后退电机控制
//                 Drv_RGB_SetColor(RGB_COLOR_WHITE);
//                 break;
//             case 0x03: 
//                 S_ComandTo_Car(3, 50, 1, 3);  // 左转向电机控制
//                 Drv_RGB_SetColor(RGB_COLOR_CYAN);
//                 break;
//             case 0x04: 
//                 S_ComandTo_Car(4, 50, 1, 4);  // 右转向电机控制
//                 Drv_RGB_SetColor(RGB_COLOR_MAGENTA);
//                 break;
//             default: break;
//         }
//     }
//     // 2. 运动时长已到：执行停止逻辑+调试RGB
//     else
//     {

//         // 保留你原有的2次循环逻辑，确保电机可靠停止
//         for(uint8_t i=0;i<2;i++)
//         {

//             S_ComandTo_Car(5, 00, 1, 5);  // 电机停止指令，调试完成可放开
//             Drv_RGB_SetColor(RGB_COLOR_RED);
//         }

//         // 运动完成状态标记与复位
//         g_car.run_flag[idx] = 1;    // 标记当前通道运动完成
//         g_car.ExForm_diretcion = 0; // 清空运动方向
//         g_car.ExForm_mode = 0;      // 回归空闲模式，等待下一次按键
//     }
// }
/*
/*
**************************************************************************************
* 函 数 名: Button_Control_Car
* 功能说明: 按钮松开后，按计时时间驱动小车运动（固定5ms调用一次）
* 修    复: 修复计数溢出、通道干扰、状态卡死等所有BUG，兼容C89标准
**************************************************************************************
*/
void Button_Control_Car(void)
{
    // 所有变量统一在函数开头声明，完全兼容CH32的C89编译标准
    uint8_t idx = 0;
    uint8_t i = 0;
    uint32_t pass_time = 0;
    // 【核心修复】4个通道独立静态计数数组，切换方向互不干扰，函数调用值不丢失
    static u32 send_cnt[4] = {10,10,10,10};
    // 【可调整】每N次5ms调用，发送1次指令，示例：10次=50ms发1次
    const u32 SEND_CNT_THRESHOLD = 10;

    // 仅按键控制模式执行，非目标模式直接退出
    if(g_car.ExForm_mode != 1) return;
    if(g_car.ExForm_diretcion == 0) return;

    // 方向匹配对应通道索引
    switch(g_car.ExForm_diretcion)
    {
        case 0x01: idx = 0; break;
        case 0x02: idx = 1; break;
        case 0x03: idx = 2; break;
        case 0x04: idx = 3; break;
        default: return;
    }

    // 该方向运动已完成，直接退出避免重复执行
    if(g_car.run_flag[idx] == 1) return;

    // 计算小车已运动时长(ms)
    pass_time = (GetTick() - g_car.start_tick[idx]) * TICK_MS;

    // ================== 核心运动控制逻辑（修复版） ==================
    // 1. 运动时长未到：固定间隔发送指令，杜绝高频连发
    if(pass_time < g_car.run_time[idx])
    { 
        // 计数递减，只有减到0，才发送1次指令
        send_cnt[idx]--;
        if(send_cnt[idx] > 0)
        {
            return; // 未到发送阈值，直接退出，不发指令
        }
        // 到达发送阈值，重置计数，执行1次指令发送
        send_cnt[idx] = SEND_CNT_THRESHOLD;

        // 仅间隔到达时，才发送一次控制指令+设置RGB
        switch(g_car.ExForm_diretcion)
        {
            case 0x01: 
                S_ComandTo_Car(1, 50, 1, 1);
                Drv_RGB_Blue();
                break;
            case 0x02: 
                S_ComandTo_Car(2, 50, 1, 2);
                Drv_RGB_SetColor(RGB_COLOR_WHITE);
                break;
            case 0x03: 
                S_ComandTo_Car(3, 50, 1, 3);
                Drv_RGB_SetColor(RGB_COLOR_CYAN);
                break;
            case 0x04: 
                S_ComandTo_Car(4, 50, 1, 4);
                Drv_RGB_SetColor(RGB_COLOR_MAGENTA);
                break;
            default: break;
        }
    }
    // 2. 运动时长已到：执行停止逻辑，修复高频连发+卡死问题
    else
    {
        // 【修复】去掉无意义的for循环连发，仅发2次保证可靠性，无高频中断
        S_ComandTo_Car(5, 00, 1, 5);
        S_ComandTo_Car(5, 00, 1, 5);
        Drv_RGB_SetColor(RGB_COLOR_RED);

        // 【必做】重置当前通道的计数，为下一次按键启动做准备，彻底解决失控问题
        send_cnt[idx] = SEND_CNT_THRESHOLD;

        // 运动完成状态标记与复位，保证状态机正常运行，不会卡死
        g_car.run_flag[idx] = 1;
        g_car.ExForm_diretcion = 0;
        g_car.ExForm_mode = 0;
    }
}


/*
************************************************************************************** 
* 函 数 名: DCCP_RecDataPacket
* 功能说明: DCCP协议接收数据包解析
* 入口参数: 无
* 返 回 值: 无
* 创建时间: 2025-4-11
************************************************************************************** 
*/
void DCCP_RecDataPacket( void )
{
	 
  
    // 拷贝帧头固定字段
	 uint8_t Rec_i = 0;
     DCCP_Rec.str[0]  = DCCP_rx_t.DCCP_rx_buf[0];
	 DCCP_Rec.str[1]  = DCCP_rx_t.DCCP_rx_buf[1];   
     DCCP_Rec.str[2]  = DCCP_rx_t.DCCP_rx_buf[2];   
     DCCP_Rec.str[4]  = DCCP_rx_t.DCCP_rx_buf[3];                               
	 DCCP_Rec.str[3]  = DCCP_rx_t.DCCP_rx_buf[4];
	 DCCP_Rec.str[6]  = DCCP_rx_t.DCCP_rx_buf[5];
	 DCCP_Rec.str[5]  = DCCP_rx_t.DCCP_rx_buf[6];
	 DCCP_Rec.str[7]  = DCCP_rx_t.DCCP_rx_buf[7];

	 /*******************拷贝数据段**************************************/
	for(Rec_i = 0; Rec_i <= (DCCP_rx_t.DCCP_rx_len - 13); Rec_i++)   				
	{
		 DCCP_Rec.str[8+Rec_i] = DCCP_rx_t.DCCP_rx_buf[8+Rec_i]; 		
	}
	// 拼接4字节帧尾
	DCCP_Rec.sDCCP.Frame_Tail  = 
	((unsigned long )DCCP_rx_t.DCCP_rx_buf[DCCP_rx_t.DCCP_rx_len-4]<<24)|
	((unsigned long )DCCP_rx_t.DCCP_rx_buf[DCCP_rx_t.DCCP_rx_len-3]<<16)|
	((unsigned long )DCCP_rx_t.DCCP_rx_buf[DCCP_rx_t.DCCP_rx_len-2]<<8)|
	((unsigned long )DCCP_rx_t.DCCP_rx_buf[DCCP_rx_t.DCCP_rx_len-1]);

}

/*
************************************************************************************** 
* 函 数 名: DCCP_comand_process
* 功能说明: 屏命令解析（适配结构体变量）
************************************************************************************** 
*/
void DCCP_comand_process(void)  
{
    uint8_t par = DCCP_Rec.sDCCP.Frame_param[1];

    // 按键控制帧解析
    if(DCCP_Rec.sDCCP.Frame_Tail == 0xFFFDFFFF)
    {
        switch(DCCP_Rec.sDCCP.Frame_ControlID) 
        {
            // case SET_CMD_DMTSTART:      DMT_START = (par == 1) ? 1 : 0; break;
            // case SET_CMD_DMTUP:         DMT_Up = (par == 1) ? 1 : 0; break;
            // case SET_CMD_DMTDOWN:       DMT_Down = (par == 1) ? 1 : 0; break;
            // case SET_CMD_DMTSPEED:      DMT_SET_Speed = par; break;
            case SET_CMD_CAR_FORWORD: 	Car_Dir_Control(0, 0x01, par); break;
            case SET_CMD_CAR_BACKWORD:  Car_Dir_Control(1, 0x02, par); break;
            case SET_CMD_CAR_TURNLEFT:  Car_Dir_Control(2, 0x03, par); break;
            case SET_CMD_CAR_TURNRIGHT: Car_Dir_Control(3, 0x04, par); break;
            default: break;
        }
    }

    // 自动运行参数解析
    if(DCCP_Rec.sDCCP.Frame_Tail == 0xFFFCFFFF)
    {
        switch(DCCP_Rec.sDCCP.Frame_ControlID)
        {
            case 0x0A: g_car.Run_S_X = DCCP_Rec.sDCCP.Frame_param[0]; break;
            case 0x0B: g_car.Run_S_Y = DCCP_Rec.sDCCP.Frame_param[0]; break;
            default: break;
        }
        // XY参数有效，切换自动模式
        if(g_car.Run_S_X && g_car.Run_S_Y)
		{
            g_car.ExForm_mode = 2;
            g_car.ExForm_diretcion = 0;
            memset(g_car.timer_en, 0, sizeof(g_car.timer_en));
        }
    }
}  
          

/*
************************************************************************************** 
* 函 数 名: DCCP_Disp_Process
* 功能说明: 显示屏数据刷新处理（轮询显示FOC状态）
* 入口参数: 无
* 返 回 值: 无
************************************************************************************** 
*/
void DCCP_Disp_Process(void)
{
    // 读取CAN总线反馈的FOC电机实时参数（保留原有注释，方便后续对接CAN）
    //   Cur_DMT_Speed = CAN2A2_Rec.sCAN.FOC_Speed ;	
	//   Cur_DMT_Temp  = CAN2A2_Rec.sCAN.FOC_Temp  ; 
	//   Cur_DMT_VOL   = CAN2A2_Rec.sCAN.FOC_VOL ;
	//   Cur_DMT_I     = CAN2A2_Rec.sCAN.FOC_I ;
	//   Cur_DMT_High  = CAN2A1_Rec.sCAN.pzd_high; 

	  Cur_DMT_Speed = 180 ;	
	  Cur_DMT_Temp  = 22 ; 
	  Cur_DMT_VOL   = 24 ;
	  Cur_DMT_I     = 1 ;
	  Cur_DMT_High  = 5; 

	// 轮询切换显示参数
	switch(Disp_Turn){
		case DISP_DMTSPEED:  // 显示FOC转速
				DCCP_Send.sDCCP.Frame_Sendcmd_type = 0xB1 ;
				DCCP_Send.sDCCP.Frame_Sendctrl_msg = 0x10 ;
				DCCP_Send.sDCCP.Frame_SendImageID  = 0x0001 ;
				DCCP_Send.sDCCP.Frame_SendControlID = 0x001A;
				// 限制最大值为999
				if( Cur_DMT_Speed >= 999){
				  Cur_DMT_Speed = 999 ;      
				}
				// 数字转ASCII并发送
				DatachagASCII ( DCCP_Send.sDCCP.Frame_Sendparam,  Cur_DMT_Speed, 2 );
				DCCP_SendFrameInfo( 3 );	
        Disp_Turn = DISP_DMTTEMP ;
				break;
		
		case DISP_DMTTEMP:  // 显示FOC温度
				DCCP_Send.sDCCP.Frame_Sendcmd_type = 0xB1 ;
				DCCP_Send.sDCCP.Frame_Sendctrl_msg = 0x10 ;
				DCCP_Send.sDCCP.Frame_SendImageID  = 0x0001 ;
				DCCP_Send.sDCCP.Frame_SendControlID = 0x0004;
				// 限制最大值为999
				if( Cur_DMT_Temp >= 999){
				  Cur_DMT_Temp = 999 ;      
				}
				DatachagASCII ( DCCP_Send.sDCCP.Frame_Sendparam,  Cur_DMT_Temp,2 );
				DCCP_SendFrameInfo( 3 );		
        Disp_Turn = DISP_DMTVOL ;
				break;
			
		case DISP_DMTVOL:  // 显示FOC电压
				DCCP_Send.sDCCP.Frame_Sendcmd_type = 0xB1 ;
				DCCP_Send.sDCCP.Frame_Sendctrl_msg = 0x10 ;
				DCCP_Send.sDCCP.Frame_SendImageID  = 0x0001 ;
				DCCP_Send.sDCCP.Frame_SendControlID = 0x0005;
				// 限制最大值为999
				if( Cur_DMT_VOL >= 999){
				  Cur_DMT_VOL = 999 ;      
			 }
				DatachagASCII ( DCCP_Send.sDCCP.Frame_Sendparam,  Cur_DMT_VOL, 2 );
				DCCP_SendFrameInfo( 3 );		
        Disp_Turn = DISP_DMTCUR ;
				break;
	
	 case DISP_DMTCUR:  // 显示FOC电流
				DCCP_Send.sDCCP.Frame_Sendcmd_type = 0xB1 ;
				DCCP_Send.sDCCP.Frame_Sendctrl_msg = 0x10 ;
				DCCP_Send.sDCCP.Frame_SendImageID  = 0x0001 ;
				DCCP_Send.sDCCP.Frame_SendControlID = 0x0006;
				// 限制最大值为999
				if( Cur_DMT_I >= 999){
				  Cur_DMT_I = 999 ;      
				}
				DatachagASCII ( DCCP_Send.sDCCP.Frame_Sendparam,  Cur_DMT_I,2 );
				DCCP_SendFrameInfo( 3 );		
        Disp_Turn = DISP_DMTHIGH ;
				break;	
	 case DISP_DMTHIGH:  // 显示高度值
				DCCP_Send.sDCCP.Frame_Sendcmd_type = 0xB1 ;
				DCCP_Send.sDCCP.Frame_Sendctrl_msg = 0x10 ;
				DCCP_Send.sDCCP.Frame_SendImageID  = 0x0001 ;
				DCCP_Send.sDCCP.Frame_SendControlID = 0x0008;
				// 限制最大值为999
				if( Cur_DMT_High >= 999){
				  Cur_DMT_High = 999 ;      
				}
				DatachagASCII ( DCCP_Send.sDCCP.Frame_Sendparam,  Cur_DMT_High,2 );
				DCCP_SendFrameInfo( 3 );		
        Disp_Turn = DISP_TASK_PROGRESS  ;
				break;			
	 case DISP_TASK_PROGRESS :  // 显示进度完成度
				DCCP_Send.sDCCP.Frame_Sendcmd_type = 0xB1 ;
				DCCP_Send.sDCCP.Frame_Sendctrl_msg = 0x10 ;
				DCCP_Send.sDCCP.Frame_SendImageID  = 0x0001 ;
				DCCP_Send.sDCCP.Frame_SendControlID = 0x001B;
				if( car_ctrl.Loop_Finished >= 999){
				   car_ctrl.Loop_Finished= 999 ;      
				}
				DatachagASCII ( DCCP_Send.sDCCP.Frame_Sendparam,  car_ctrl.Loop_Finished ,2 );
				DCCP_SendFrameInfo( 3 );				
				DCCP_SendFrameInfo( 3 );		
        Disp_Turn = DISP_DMTSPEED ;
				break;	
	 
		default: 
					// 默认显示转速
					Disp_Turn = DISP_DMTSPEED ;
				break;
	}
}

/*
************************************************************************************** 
* 函 数 名: DCCP_SendFrameInfo
* 功能说明: 组装并发送DCCP协议数据帧
* 入口参数: Send_Len-发送参数长度
* 返 回 值: 无
************************************************************************************** 
*/
void DCCP_SendFrameInfo(uint16_t Send_Len )
{
	uint8_t  Frame_Info[32];
	uint8_t i = 0;

	// 组装帧头数据（保持原有逻辑，适配协议字节顺序）
	Frame_Info[0] = DCCP_Send.str[0] ; 			
	Frame_Info[1] = DCCP_Send.str[1] ; 			
	Frame_Info[2] = DCCP_Send.str[2]; 		  
	Frame_Info[3] = DCCP_Send.str[4] ; 			
	Frame_Info[4] = DCCP_Send.str[3] ; 			
	Frame_Info[5] = DCCP_Send.str[6]; 		  
	Frame_Info[6] = DCCP_Send.str[5] ; 			

	// 组装参数数据
	for(i = 0; i < Send_Len; i++)   				
	{
		Frame_Info[7+i] = DCCP_Send.sDCCP.Frame_Sendparam[i]; 		
	}

	// 组装帧尾
    Frame_Info[7+Send_Len] =  DCCP_Send.str[16] ; 
	Frame_Info[8+Send_Len] =  DCCP_Send.str[17] ; 
	Frame_Info[9+Send_Len] =  DCCP_Send.str[18] ; 
	Frame_Info[10+Send_Len] = DCCP_Send.str[19] ; 

	// 发送帧数据到大彩屏
	MW_Send(MW_PORT_2_DCCP,Frame_Info, (11+Send_Len), 0);
}