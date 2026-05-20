#include "usart_app.h"
#include "usart_protocol.h"
#include <stdio.h>
#include "hal_beep.h"
#include "hal_rgb.h"
#include <string.h>



DCCP_rx_hander DCCP_rx_t = {0};
ESP32_rx_hander ESP32_rx_t = {0};
// ===================== ���ڻص����� =====================
static void ESP32_UART1_Callback (uint8_t *data, uint16_t len) 
{
    if (len < 1)
        return;
    // 修复：避免复制超出 ESP32_rx_t.ESP32_rx_buf 的范围（缓冲区大小为 20）
    if (len > sizeof(ESP32_rx_t.ESP32_rx_buf)) {
        len = sizeof(ESP32_rx_t.ESP32_rx_buf);
    }
    for (uint8_t i = 0; i < len; i++) 
    {
        ESP32_rx_t.ESP32_rx_buf[i] = data[i];
    }
    ESP32_rx_t.ESP32_rx_len = len;
    //MW_Send (MW_PORT_1_ESP32, data, len, 0);
}

static void DCCP_UART2_Callback (uint8_t *data, uint16_t len)
{
    if (len < 1)
        return;
    // 修复：避免复制超出 DCCP_rx_t.DCCP_rx_buf 的范围（缓冲区大小为 20）
    if (len > sizeof(DCCP_rx_t.DCCP_rx_buf)) {
        len = sizeof(DCCP_rx_t.DCCP_rx_buf);
    }
    for (uint8_t i = 0; i < len; i++) 
    {
        DCCP_rx_t.DCCP_rx_buf[i] = data[i];
    }
    DCCP_rx_t.DCCP_rx_len = len;
    //MW_Send(MW_PORT_2_DCCP, data, len, 0);
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
    MW_Send (MW_PORT_5_K230, data, len, 0);
}

// ===================== ͨ��Э�����ú����������棩 =====================
/**
 * @brief ����ָ�����ڵ�Э�����
 * @param mw_port_index: ���ں� 1~5
 * @param check_en: У��ʹ�� 1=���� 0=�ر�
 * @param check_type: У������ MW_CHECK_NONE/SUM8/CRC8
 * @param frame_h1: ֡ͷ��1�ֽ�
 * @param frame_h2: ֡ͷ��2�ֽ�
 * @param frame_h3: ֡ͷ��3�ֽ�
 * @param frame_tail1: ֡β0
 * @param frame_tail2: ֡β1
 */
void Protocol_set (u8 mw_port_index, u8 check_en, MW_CheckType_t check_type,
                   u8 frame_h1, u8 frame_h2, u8 frame_h3, u32 frame_tail1, u32 frame_tail2) {
    MW_Protocol_Config_t cfg;
    memset (&cfg, 0, sizeof (cfg));  // �ṹ���ʼ��

    // ============== ͳһ��ֵ���������ö��ô���Ĳ��� ==============
    cfg.check_en = check_en;
    cfg.check_type = check_type;
    cfg.frame_h[0] = frame_h1;
    cfg.frame_h[1] = frame_h2;
    cfg.frame_h[2] = frame_h3;
    cfg.frame_tail[0] = frame_tail1;
    cfg.frame_tail[1] = frame_tail2;

    // ============== ���ݶ˿ں����� ==============
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
        // �Ƿ��˿ڣ�������
        break;
    }
}

// ===================== Ӧ�ó�ʼ�� =====================
void usart_app_Init (void) {


        // --- ����1 ��ʼ�� + �Զ���Э�� ---
    MW_Init (MW_PORT_1_ESP32, 115200);
    Protocol_set (1, 0, 0,
                  0xAA, 0x55, 0xA5,
                  0xFFFDFFFF, 0xFFFCFFFF);
    MW_RegisterCallback (MW_PORT_1_ESP32, ESP32_UART1_Callback);

    // --- ����2 ��ʼ�� + �Զ���Э�� ---
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
}

// ===================== ��ѭ�� =====================
void usart_app_Run (void) 
{
    MW_Process();
}
