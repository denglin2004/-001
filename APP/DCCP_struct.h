#ifndef __DCCP_STRUCT_H__
#define __DCCP_STRUCT_H__
#include "ch32v30x.h"
#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)
//////////////////���Ӵ����ʾ����������֡�ṹ///////////////////////////////////////////
struct strDCCPRecData 
{                                       // ��ʴ�����ʾ����
	unsigned char 	Frame_Head;		    // ֡ͷ
	unsigned char 	Frame_cmd_type;	    //��������
	unsigned char 	Frame_ctrl_msg;	    //��Ϣ����
	unsigned short 	Frame_ImageID;      //����ID
	unsigned short 	Frame_ControlID;    //�ؼ�ID
	unsigned char  	Frame_control_type; //�ؼ�����
	unsigned char  	Frame_param[8];     //�ɱ䳤�Ȳ��������8���ֽ�;	//��ʾֵ
	unsigned long  	Frame_Tail;		    //֡β	
};

typedef union unDCCP_Data 
{
	unsigned char str[20];
	struct strDCCPRecData sDCCP;
};

union unDCCP_Data  DCCP_Rec = { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0,0 };
	
//////////���͸������ʾ��֡�ṹ///////////////////////////////
struct strDCCPSendData 
{                                       //��ʴ�����ʾ����
	unsigned char 	Frame_SendHead;		//֡ͷ
	unsigned char 	Frame_Sendcmd_type;	  //��������
	unsigned char 	Frame_Sendctrl_msg;	   //��Ϣ����
	unsigned short 	Frame_SendImageID;		 //����ID
	unsigned short 	Frame_SendControlID;		//�ؼ�ID
	unsigned char  	Frame_Control_type;    //�ؼ�����
	unsigned char  	Frame_Sendparam[8];     //�ɱ䳤�Ȳ��������16���ֽ�;	//��ʾֵ
	unsigned long  	Frame_Tail;		    //֡β	
};

 union unDCCP_SendData {
	unsigned char str[20];
	struct strDCCPSendData sDCCP ;
}; 

union unDCCP_SendData DCCP_Send = { 0xEE, 0, 0, 0,  0, 0, 0, 0,  
    0, 0, 0, 0,  0, 0, 0, 0,  0xFF, 0xFC, 0xFF,0xFF };
/////////////////////////////////////
#pragma pack(1)

////////////���մ����ʾ���趨����//////////////////////////////////	
unsigned short  Cur_DMT_Speed = 0 ;	
unsigned short  Cur_DMT_Temp = 0 ;
unsigned short  Cur_DMT_VOL =  0 ;
unsigned short  Cur_DMT_I =    0 ;
unsigned short  Cur_DMT_High = 0 ;	
unsigned char  Disp_Turn = 0 ;	
///////////���͸���������////////////////////////
#define  DISP_DMTSPEED	    0     //��ĥ��ת��
#define  DISP_DMTTEMP	    1     //��ĥͷ�¶�
#define  DISP_DMTVOL	    2     //��ĥͷ��ѹ
#define  DISP_DMTCUR	    3     //��ĥͷ����
#define  DISP_DMTHIGH	    4     //��ĥͷ�߶�	
#define  DISP_TASK_PROGRESS 5     //������ɽ���

	


#define   SET_CMD_DMTSTART	      0x07    //�趨��ĥͷ����  1������   0��ֹͣ
#define   SET_CMD_DMTSPEED	      0x08    //�趨��ĥͷת��  
#define   SET_CMD_DMTHIGHT	      0x09   //�趨��ĥͷ�߶�  
#define   SET_CMD_DMTUP	          0x17    //���ô�ĥͷ����
#define   SET_CMD_DMTDOWN         0x18    //���ô�ĥͷ�½�

#define   SET_CMD_CAR_FORWORD     0x03
#define   SET_CMD_CAR_BACKWORD    0x06
#define   SET_CMD_CAR_TURNLEFT    0x04
#define   SET_CMD_CAR_TURNRIGHT   0x05

typedef struct {
    uint8_t  ExForm_mode;       // 0=���� 1=���������˶� 2=�Զ�����
    uint8_t  ExForm_diretcion;  // 0=ֹͣ 1=ǰ 2=�� 3=�� 4=��
    
    uint32_t start_tick[4];     // ? 4�����������ʱ���޸�����BUG��
    uint32_t run_time[4];       // ���ֺ������˶�ʱ��(ms)
    uint8_t  timer_en[4];       // ��ť���¼�ʱʹ��(0=δ�� 1=����)
    uint8_t  run_flag[4];       // ? �˶���ɱ�־(0=δ��� 1=�����)
    uint8_t  last_cmdParam[4];
    uint8_t  Run_S_X;
    uint8_t  Run_S_Y;
	uint8_t  Run_distance;
	uint8_t  Run_cnt_x;
	uint8_t  Run_cnt_y;
} Car_Ctrl_Typedef;

typedef struct {
     uint8_t speed;

} Foc_Ctrl_Typedef;


typedef struct {
 uint8_t i;
 uint8_t high;
 uint8_t up;
 uint8_t down;
}GrindCtrl_Typedef;


static Car_Ctrl_Typedef g_car = {0};
static Foc_Ctrl_Typedef g_foc = {0};
static GrindCtrl_Typedef g_grind = {0};

#define TICK_MS       1000
#define K_time      1   

#endif  /* __DCCP_COMAND_H__ */
