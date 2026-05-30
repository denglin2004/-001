#include "usart_app.h"
#include "usart_protocol.h"
#include <stdio.h>
#include "hal_beep.h"
#include "hal_rgb.h"
#include <string.h>
#include "ESP32_comand.h"
#include "can_app.h"
#include "DCCP_comand.h"

DCCP_rx_hander DCCP_rx_t = {0};
ESP32_rx_hander ESP32_rx_t = {0};
// ===================== 串口中断回调函数 =====================
static void ESP32_UART1_Callback (uint8_t *data, uint16_t len) 
{
    if (len < 1)
        return;
    for (uint8_t i = 0; i < len; i++) 
    {
        ESP32_rx_t.ESP32_rx_buf[i] = data[i];
    }
    ESP32_rx_t.ESP32_rx_len = len;
    ESP32_RecDataPacket() ;
}

static void DCCP_UART2_Callback (uint8_t *data, uint16_t len)
{
    if (len < 1)
        return;
    for (uint8_t i = 0; i < len; i++) 
    {
        DCCP_rx_t.DCCP_rx_buf[i] = data[i];
    }
    DCCP_rx_t.DCCP_rx_len = len;
    DCCP_RecDataPacket();  
}

static void DCCP_UART3_Callback (uint8_t *data, uint16_t len) {
    if (len < 1)
        return;
    //MW_Send (MW_PORT_3_DCCP, data, len, 0);
}

static void NONE_UART4_Callback (uint8_t *data, uint16_t len) {
    if (len < 1)
        return;
    MW_Send (MW_PORT_4_NONE, data, len, 0);
}

static void K230_UART5_Callback (uint8_t *data, uint16_t len) {
    if (len < 1)
        return;
   // MW_Send (MW_PORT_5_K230, data, len, 0);
}

// ===================== 通用协议配置函数（完美版） =====================
/**
 * @brief 配置指定串口的协议参数
 * @param mw_port_index: 串口号 1~5
 * @param check_en: 校验使能 1=开启 0=关闭
 * @param check_type: 校验类型 MW_CHECK_NONE/SUM8/CRC8
 * @param frame_h1: 帧头第1字节
 * @param frame_h2: 帧头第2字节
 * @param frame_h3: 帧头第3字节
 * @param frame_tail1: 帧尾0
 * @param frame_tail2: 帧尾1
 */
void Protocol_set (u8 mw_port_index, u8 check_en, MW_CheckType_t check_type,
                   u8 frame_h1, u8 frame_h2, u8 frame_h3, u32 frame_tail1, u32 frame_tail2) {
    MW_Protocol_Config_t cfg;
    memset (&cfg, 0, sizeof (cfg));  // 结构体初始化

    // ============== 统一赋值：所有配置都用传入的参数 ==============
    cfg.check_en = check_en;
    cfg.check_type = check_type;
    cfg.frame_h[0] = frame_h1;
    cfg.frame_h[1] = frame_h2;
    cfg.frame_h[2] = frame_h3;
    cfg.frame_tail[0] = frame_tail1;
    cfg.frame_tail[1] = frame_tail2;

    // ============== 根据端口号配置 ==============
    switch (mw_port_index) {
    case 1:
        MW_SetProtocolConfig (MW_PORT_1_ESP32, &cfg);
        break;
    case 2:
        MW_SetProtocolConfig (MW_PORT_2_DCCP, &cfg);
        break;
    case 3:
        MW_SetProtocolConfig (MW_PORT_3_DCCP, &cfg);
        break;
    case 4:
        MW_SetProtocolConfig (MW_PORT_4_NONE, &cfg);
        break;
    case 5:
        MW_SetProtocolConfig (MW_PORT_5_K230, &cfg);
        break;
    default:
        // 非法端口，不配置
        break;
    }
}

// ===================== 应用初始化 =====================
void usart_app_Init (void) {


        // --- 串口1 初始化 + 自定义协议 ---
    MW_Init (MW_PORT_1_ESP32, 115200);
    Protocol_set (1, 0, 0,
                  0xAA, 0x55, 0xA5,
                  0xFFFDFFFF, 0xFFFCFFFF);
    MW_RegisterCallback (MW_PORT_1_ESP32, ESP32_UART1_Callback);

    // --- 串口2 初始化 + 自定义协议 ---
    MW_Init (MW_PORT_2_DCCP, 9600);
    // Protocol_set (2, 1, MW_CHECK_SUM8,
    //               0xAA, 0xBB, 0xCC,
    //               0xFFFDFFFF, 0xFFFCFFFF);
    MW_RegisterCallback (MW_PORT_2_DCCP, DCCP_UART2_Callback);

    // Protocol_set(3, 1, MW_CHECK_SUM8,
    //              0xAA, 0xBB, 0xCC,
    //              0xFFFDFFFF, 0xFFFCFFFF);
    MW_Init (MW_PORT_3_DCCP,   9600);
    MW_RegisterCallback (MW_PORT_3_DCCP, DCCP_UART3_Callback);


    MW_Init (MW_PORT_5_K230,   9600);
    MW_RegisterCallback (MW_PORT_5_K230, K230_UART5_Callback);
}

// ===================== 主循环 =====================
void usart_app_Run (void) 
{
    MW_Process();
}
