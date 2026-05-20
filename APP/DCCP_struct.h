#ifndef __DCCP_STRUCT_H__
#define __DCCP_STRUCT_H__
#include "ch32v30x.h"
#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)
//////////////////增加大彩显示屏接收数据帧结构///////////////////////////////////////////
struct strDCCPRecData 
{                                       // 大彩触屏显示数据
	unsigned char 	Frame_Head;		    // 帧头
	unsigned char 	Frame_cmd_type;	    //命令类型
	unsigned char 	Frame_ctrl_msg;	    //消息类型
	unsigned short 	Frame_ImageID;      //画面ID
	unsigned short 	Frame_ControlID;    //控件ID
	unsigned char  	Frame_control_type; //控件类型
	unsigned char  	Frame_param[8];     //可变长度参数，最多8个字节;	//显示值
	unsigned long  	Frame_Tail;		    //帧尾	
};

typedef union unDCCP_Data 
{
	unsigned char str[20];
	struct strDCCPRecData sDCCP;
};

union unDCCP_Data  DCCP_Rec = { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0,0 };
	
//////////发送给大彩显示屏帧结构///////////////////////////////
struct strDCCPSendData 
{                                       //大彩触屏显示数据
	unsigned char 	Frame_SendHead;		//帧头
	unsigned char 	Frame_Sendcmd_type;	  //命令类型
	unsigned char 	Frame_Sendctrl_msg;	   //消息类型
	unsigned short 	Frame_SendImageID;		 //画面ID
	unsigned short 	Frame_SendControlID;		//控件ID
	unsigned char  	Frame_Control_type;    //控件类型
	unsigned char  	Frame_Sendparam[8];     //可变长度参数，最多16个字节;	//显示值
	unsigned long  	Frame_Tail;		    //帧尾	
};

 union unDCCP_SendData {
	unsigned char str[20];
	struct strDCCPSendData sDCCP ;
}; 

union unDCCP_SendData DCCP_Send = { 0xEE, 0, 0, 0,  0, 0, 0, 0,  
    0, 0, 0, 0,  0, 0, 0, 0,  0xFF, 0xFC, 0xFF,0xFF };
/////////////////////////////////////
#pragma pack(1)

////////////接收大彩显示屏设定数据//////////////////////////////////	
unsigned short  Cur_DMT_Speed = 0 ;	
unsigned short  Cur_DMT_Temp = 0 ;
unsigned short  Cur_DMT_VOL =  0 ;
unsigned short  Cur_DMT_I =    0 ;
unsigned short  Cur_DMT_High = 0 ;	
///////////发送给彩屏定义////////////////////////
#define  DISP_DMTSPEED	    0     //打磨机转速
#define  DISP_DMTTEMP	    1     //打磨头温度
#define  DISP_DMTVOL	    2     //打磨头电压
#define  DISP_DMTCUR	    3     //打磨头电流
#define  DISP_DMTHIGH	    4     //打磨头高度	
#define  DISP_TASK_PROGRESS 5     //打磨头高度	
unsigned char  Disp_Turn = 0 ;	
	
// unsigned char 	DMT_Car_ExForm = 0;   //运动方式，00表示不动，01表示前进，02表示后退，03表示向左，04表示向右
// uint16_t DMT_Car_ExForm_cnt[4]={0};//各方向单独运动时间计数器
// uint8_t DMT_Car_ExForm_timer[4]={0};//各方向单独运动时间
// uint8_t DMT_Car_Timer_Enable[4] = {0};  // 定义4个运动方向的计时使能标记（0=停止计时，1=开始计时）// [0]前进 [1]后退 [2]左转 [3]右转
// uint8_t DMT_Car_ExForm_flag=0;         //各方向单独运动标志位
// uint8_t DMT_Car_Run_S_X=0;
// uint8_t DMT_Car_Run_S_Y=0;
//#define   SET_CMD_DMTSTART        0x01
//#define   SET_CMD_CAR_DMTSTART    0x02  
//#define   SET_CMD_WORKMODE        0x07    //设定工作模式

#define   SET_CMD_DMTSTART	      0x07    //设定打磨头启动  1：启动   0：停止
#define   SET_CMD_DMTSPEED	      0x08    //设定打磨头转速  
#define   SET_CMD_DMTUP	          0x0B    //设置打磨头上升
#define   SET_CMD_DMTDOWN         0x0C    //设置打磨头下降

#define   SET_CMD_CAR_FORWORD     0x03
#define   SET_CMD_CAR_BACKWORD    0x06
#define   SET_CMD_CAR_TURNLEFT    0x04
#define   SET_CMD_CAR_TURNRIGHT   0x05
typedef struct {
    uint8_t  ExForm_mode;       // 0=空闲 1=按键控制运动 2=自动控制
    uint8_t  ExForm_diretcion;  // 0=停止 1=前 2=后 3=左 4=右
    
    uint32_t start_tick[4];     // ? 4个方向独立计时（修复共用BUG）
    uint32_t run_time[4];       // 松手后计算的运动时间(ms)
    uint8_t  timer_en[4];       // 按钮按下计时使能(0=未按 1=按下)
    uint8_t  run_flag[4];       // ? 运动完成标志(0=未完成 1=已完成)
    uint8_t  last_cmdParam[4];
    uint8_t  Run_S_X;
    uint8_t  Run_S_Y;
} Car_Ctrl_Typedef;

// 全局实例
static Car_Ctrl_Typedef g_car = {0};

// 时间系数(1ms一次tick则=1)
#define TICK_MS       1000

#endif  /* __DCCP_COMAND_H__ */
