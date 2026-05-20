#include "ESP32_comand.h"
#include "ESP32_struct.h"
#include "usart_app.h"
#include "can_app.h"

/*
************************************************************************************** 
* �� �� ��: E_RecDataPacket
* ����˵��: DCCPЭ��������ݰ�����
* ��ڲ���: ��
* �� �� ֵ: ��
* ����ʱ��: 2025-4-11
************************************************************************************** 
*/
void ESP32_RecDataPacket( void )
{
	/*******************�������ݶ�**************************************/
	// 修复：避免越界。ESP32_rx_t.ESP32_rx_buf 长度为 20，但 ESP32_Rec.str 长度为 15。
	// 仅拷贝接收到的字节，最多 15 字节；并在不够时填 0 以保持确定性状态。
	if (ESP32_rx_t.ESP32_rx_len == 0) return;
	uint8_t copy_len = (ESP32_rx_t.ESP32_rx_len < 15) ? ESP32_rx_t.ESP32_rx_len : 15;
	for (u8 Rec_i = 0; Rec_i < copy_len; Rec_i++)
	{
		ESP32_Rec.str[Rec_i] = ESP32_rx_t.ESP32_rx_buf[Rec_i];
	}
	// 将剩余字节置零，避免使用未初始化数据
	for (u8 i = copy_len; i < 15; i++) {
		ESP32_Rec.str[i] = 0;
	}

	switch (ESP32_Rec.sAESP32.Device_Addr) 
	{
			case DEV_CAR_CHASSIS :  
				// 拷贝完整的 15 字节数据到车体结构（来源已保证不越界）
				for (u8 i = 0; i < 15; i++)
				{
					ESP32_Reccar.str[i] = ESP32_Rec.str[i];
				}
			break;
			case DEV_LIFT_MODULE :  
				for (u8 i = 0; i < 15; i++)
				{
					ESP32_RecBUjing.str[i] = ESP32_Rec.str[i];
				}
			break;
			// case DEV_GRINDER_HEAD :  ; break;
			default: break;

	}
}

/*
************************************************************************************** 
* �� �� ��: DCCP_comand_process
* ����˵��: ���������������ṹ�������
************************************************************************************** 
*/
void ESP32_comand_process(void)  
{
    uint8_t ADRESS = ESP32_Rec.sAESP32.Device_Addr;

            // S_ComandTo_Car(ESP32_Reccar.sAESP32.Direction,
            // ESP32_Reccar.sAESP32.Speed,1,1); 
    // ��������֡����

        switch(ADRESS) 
        {

            case DEV_CAR_CHASSIS :  
            // S_ComandTo_Car(ESP32_Reccar.sAESP32.Direction,
            // ESP32_Reccar.sAESP32.Speed,1, 1); 
            S_ComandTo_Car(1,
            28     ,1,1);   
            break;
            case DEV_LIFT_MODULE :  ; break;
            case DEV_GRINDER_HEAD :  ; break;
            default: break;
        }
    


}  
      