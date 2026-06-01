#include "ESP32_comand.h"
#include "ESP32_struct.h"
#include "usart_app.h"
#include "can_app.h"
#include "hal_usart.h"
#include "string.h"
#include "usart_protocol.h"
#include <stdlib.h>   // rand() 随机数函数
#include <time.h>     // time() 随机种子

// ======================== 鍏ㄥ眬鍙橀噺瀹氫箟 ========================
ESP32_RecData_t  ESP32_Rec                = { 0 };

ESP32_RecData_t  ESP32_Xiaozhi_Reccar     = { 0 };
ESP32_RecData_t  ESP32_Xiaozhi_RecLift    = { 0 };
ESP32_RecData_t  ESP32_Xiaozhi_RecFOC     = { 0 };

ESP32_RecData_t  ESP32_Yun_Reccar         = { 0 };
ESP32_RecData_t  ESP32_Yun_RecLift        = { 0 };
ESP32_RecData_t  ESP32_Yun_RecFOC         = { 0 };
ESP32_RecData_t  ESP32_Yun_RecOneKeycmd   = { 0 };

/*
**************************************************************************************
* 鍑? 鏁? 鍚?: ESP32_RecDataPacket
* 鍔熻兘璇存槑: ESP32鍗忚??鎺ユ敹鏁版嵁鍖呭垎鍙?
**************************************************************************************
*/
void ESP32_RecDataPacket(void)
{srand((unsigned int)time(NULL));
    memcpy(ESP32_Rec.Buf, ESP32_rx_t.ESP32_rx_buf, 15);

    switch (ESP32_Rec.Frame.DevAddr)
    {
    case DEV_CAR_CHASSIS:
        memcpy(ESP32_Xiaozhi_Reccar.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_LIFT_MODULE:
        memcpy(ESP32_Xiaozhi_RecLift.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_GRINDER_HEAD:
        memcpy(ESP32_Xiaozhi_RecFOC.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_CAR_CHASSIS_ONENET:
        memcpy(ESP32_Yun_Reccar.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_LIFT_MODULE_ONENET:
        memcpy(ESP32_Yun_RecLift.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_GRINDER_HEAD_ONENET:
        memcpy(ESP32_Yun_RecFOC.Buf, ESP32_Rec.Buf, 15);
        break;
    case DEV_ONEKEY_ONENET:
        memcpy(ESP32_Yun_RecOneKeycmd.Buf, ESP32_Rec.Buf, 15);
        break;
    default:
        break;
    }
}

/*
**************************************************************************************
* 鍑? 鏁? 鍚?: ESP32_comand_process
* 鍔熻兘璇存槑: 瑙ｆ瀽鎺ユ敹鏁版嵁骞跺垎鍙戝埌瀵瑰簲鎵ц?岀粨鏋?
**************************************************************************************
*/
void ESP32_comand_process(void)
{
    uint8_t addr = ESP32_Rec.Frame.DevAddr;

    switch (addr)
    {
    case DEV_CAR_CHASSIS:
    {
        if (ESP32_Xiaozhi_Reccar.Frame.Data.Car.Speed >= 20)
        {
            ESP32_Xiaozhi_Reccar.Frame.Data.Car.Speed = 20;
        }
        S_ComandTo_Car(ESP32_Xiaozhi_Reccar.Frame.Data.Car.Direction,
                        ESP32_Xiaozhi_Reccar.Frame.Data.Car.Speed, 2, 1);
        memset(&ESP32_Xiaozhi_Reccar, 0, sizeof(ESP32_Xiaozhi_Reccar));
    }
    break;

    case DEV_LIFT_MODULE:
    {
        switch (ESP32_Xiaozhi_RecLift.Frame.FuncCode)
        {
        // case 0x01: S_ComandTo_BuJing(...); break;
        // case 0x02: S_ComandTo_BuJing(...); break;
        }
    }
    break;

    case DEV_GRINDER_HEAD:
    {
        switch (ESP32_Xiaozhi_RecFOC.Frame.FuncCode)
        {
        case 0x01:
            S_ComandTo_FOC(ESP32_Xiaozhi_RecFOC.Frame.Data.FOC.Enable, 250, 4);
            memset(&ESP32_Xiaozhi_RecFOC, 0, sizeof(ESP32_Xiaozhi_RecFOC));
            break;
        case 0x02:
            S_ComandTo_FOC(1, ESP32_Xiaozhi_RecFOC.Frame.Data.Raw[1], 5);
            memset(&ESP32_Xiaozhi_RecFOC, 0, sizeof(ESP32_Xiaozhi_RecFOC));
            break;
        }
    }
    break;
    case DEV_CAR_CHASSIS_ONENET:
    {

        S_ComandTo_Car(ESP32_Yun_Reccar.Frame.Data.Car.Direction,
                        ESP32_Yun_Reccar.Frame.Data.Car.Speed, 2, 1);
        memset(&ESP32_Yun_Reccar, 0, sizeof(ESP32_Yun_Reccar));
    }
    break;

    case DEV_LIFT_MODULE_ONENET:
    {
        switch (ESP32_Yun_RecLift.Frame.FuncCode)
        {
        // case 0x01: S_ComandTo_BuJing(...); break;
        // case 0x02: S_ComandTo_BuJing(...); break;
        }
    }
    break;

    case DEV_GRINDER_HEAD_ONENET:
    {
        switch (ESP32_Yun_RecFOC.Frame.FuncCode)
        {
        case 0x01:
            S_ComandTo_FOC(ESP32_Yun_RecFOC.Frame.Data.FOC.Enable, 250, 4);
            memset(&ESP32_Yun_RecFOC, 0, sizeof(ESP32_Yun_RecFOC));
            break;
        case 0x02:
            S_ComandTo_FOC(1, ESP32_Yun_RecFOC.Frame.Data.Raw[1], 5);
            memset(&ESP32_Yun_RecFOC, 0, sizeof(ESP32_Yun_RecFOC));
            break;
        }
    }
    break;

    default:
    break;
    }

    ESP32_Rec.Frame.DevAddr = 0;
}

void data_to_onnet(void)
{
    uint8_t Frame_Info[15];
// 【只需要在程序初始化时执行一次】生成随机种子，让每次开机随机数都不一样


Frame_Info[0]  = 0xAA;        // 帧头固定
Frame_Info[1]  = 0x55;        // 帧头固定
Frame_Info[2]  = 0x05;        // 固定不变

// 下面这些位置 生成 0~255 随机数 (uint8_t 范围)
Frame_Info[3]  = rand() % 256;  // 随机 0-255
Frame_Info[4]  = rand() % 256;
Frame_Info[5]  = rand() % 256;
Frame_Info[6]  = rand() % 256;

Frame_Info[7]  = rand() % 256;
Frame_Info[8]  = rand() % 256;
Frame_Info[9]  = rand() % 256;
Frame_Info[10] = rand() % 256;
Frame_Info[11] = 0xFF;
Frame_Info[12] = 0xFC;
Frame_Info[13] = 0xFF;
Frame_Info[14] = 0xFF;

// 发送
//MW_Send(MW_PORT_5_K230, Frame_Info, 15, 0);
MW_Send(MW_PORT_5_K230, Frame_Info, 15, 0);

}
