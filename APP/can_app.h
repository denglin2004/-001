#ifndef __CAN_APP_H_
#define __CAN_APP_H_

#include "ch32v30x.h"
struct strCAN_2A0_RecData {
	unsigned char ack;
      
};

union unCAN_2A0_Data{
	unsigned char str[8];
	struct strCAN_2A0_RecData sCAN;
};


///////////////////////////////////////////////////////
struct strCAN_2A1_RecData {
	unsigned char pzd_function_code;		// ????
	unsigned char pzd_doned;		// ????
	unsigned char pzd_high;		// ????
};

union unCAN_2A1_Data {
	unsigned char str[8];
	struct strCAN_2A1_RecData sCAN;
};



struct strCAN_2A2_RecData {
	
	unsigned short FOC_Start;		
	unsigned short FOC_Temp;	
	unsigned short FOC_VOL;		
	unsigned short FOC_I;	
	unsigned short FOC_Speed;		
	unsigned short FOC_High;	
};

union unCAN_2A2_Data {
	unsigned short str[8];
	struct strCAN_2A2_RecData sCAN;
};
extern union unCAN_2A0_Data CAN2A0_Rec;
extern union unCAN_2A1_Data CAN2A1_Rec;
extern union unCAN_2A2_Data CAN2A2_Rec; 
void can_app_Init(void);
void app_can_data_packet (void) ;
void S_ComandTo_Car(uint8_t direction,uint8_t car_speed,uint8_t ack_type,uint8_t Sequence_ID);
void S_ComandTo_BuJing(uint8_t function,uint8_t enable,
                   uint8_t dir,uint8_t distance_high,uint8_t distance_low, uint8_t Seq_id);
void S_ComandTo_FOC(uint8_t eable,uint8_t Foc_Speed,uint8_t Sequence_ID);
#endif

