#include "usart_protocol.h"
// 仅 .c 文件可见的头文件（原有）
#include "hal_usart.h"
#include <string.h>

// ==========================================
// 协议默认配置（沿用原有硬编码值）
// ==========================================
#define MW_DEFAULT_H1        0xEE
#define MW_DEFAULT_H2        0xB1
#define MW_DEFAULT_H3        0x11
#define MW_DEFAULT_TAIL0     0xFFFDFFFF
#define MW_DEFAULT_TAIL1     0xFFFCFFFF
#define MW_DEFAULT_CHECK_TYPE MW_CHECK_NONE
#define MW_DEFAULT_CHECK_EN  0
#define MW_RX_BUF_MAX   128
#define CRC8_POLY 0x07

// ==========================================
// 新增：CRC8 计算参数（标准多项式 0x07）
// ==========================================

// ==========================================
// 原有：状态机枚举
// ==========================================
typedef enum
{
    S_H1, S_H2, S_H3, S_DATA, S_TAIL
} RxState_t;

// ==========================================
// 扩展：内部控制结构体（新增配置项）
// ==========================================
typedef struct
{
    uint8_t  buf[MW_RX_BUF_MAX];
    uint16_t idx;
    uint32_t tail_shifter;
    RxState_t state;
    MW_Callback_t app_cb;
    MW_Protocol_Config_t cfg;  // 新增：协议配置（默认初始化）
    uint8_t check_buf;         // 新增：校验值缓存（8位和/CRC8）
    uint16_t check_buf16;      // 新增：CRC16缓存
} MW_Ctrl_t;

// ==========================================
// 原有：宏定义 + 内部资源
// ==========================================

static MW_Ctrl_t s_ctrl[MW_PORT_MAX] = {0};
// 原有：端口映射
static const BSP_USART_TypeDef MW_2_HAL_Map[MW_PORT_MAX] =
{
    [MW_PORT_1_ESP32] = BSP_USART1,
    [MW_PORT_2_DCCP] = BSP_USART2,
    [MW_PORT_3_DCCP] = BSP_USART3,
    [MW_PORT_4_NONE] = BSP_UART4,
    [MW_PORT_5_K230] = BSP_UART5,
};

// ==========================================
// 新增：私有函数声明（校验计算+默认配置初始化）
// ==========================================
static void ParseByte(MW_Ctrl_t *p, uint8_t b);
static void MW_ResetDefaultConfig(MW_Protocol_Config_t *cfg);
static uint8_t MW_CalcSum8(uint8_t *data, uint16_t len);
static uint8_t MW_CalcCRC8(uint8_t *data, uint16_t len);
static uint16_t MW_CalcCRC16(uint8_t *data, uint16_t len);  // 可选扩展
static uint8_t MW_VerifyCheck(MW_Ctrl_t *p, uint8_t *data, uint16_t len);

// ==========================================
// 原有：HAL 回调函数（无需修改）
// ==========================================
static void HalCb_Port1(uint8_t *data, uint16_t len)
{ for(int i=0; i<len; i++) ParseByte(&s_ctrl[MW_PORT_1_ESP32], data[i]); }

static void HalCb_Port2(uint8_t *data, uint16_t len)
{ for(int i=0; i<len; i++) ParseByte(&s_ctrl[MW_PORT_2_DCCP], data[i]); }

static void HalCb_Port3(uint8_t *data, uint16_t len)
{ for(int i=0; i<len; i++) ParseByte(&s_ctrl[MW_PORT_3_DCCP], data[i]); }

static void HalCb_Port4(uint8_t *data, uint16_t len)
{ for(int i=0; i<len; i++) ParseByte(&s_ctrl[MW_PORT_4_NONE], data[i]); }

static void HalCb_Port5(uint8_t *data, uint16_t len)
{ for(int i=0; i<len; i++) ParseByte(&s_ctrl[MW_PORT_5_K230], data[i]); }

typedef void (*HalCbFunc_t)(uint8_t*, uint16_t);
static const HalCbFunc_t HalCbFuncTable[MW_PORT_MAX] =
{
    [MW_PORT_1_ESP32] = HalCb_Port1,
    [MW_PORT_2_DCCP] = HalCb_Port2,
    [MW_PORT_3_DCCP] = HalCb_Port3,
    [MW_PORT_4_NONE] = HalCb_Port4,
    [MW_PORT_5_K230] = HalCb_Port5,
};

// ==========================================
// 新增：恢复默认配置
// ==========================================
static void MW_ResetDefaultConfig(MW_Protocol_Config_t *cfg)
{
    cfg->frame_h[0] = MW_DEFAULT_H1;
    cfg->frame_h[1] = MW_DEFAULT_H2;
    cfg->frame_h[2] = MW_DEFAULT_H3;
    cfg->frame_tail[0] = MW_DEFAULT_TAIL0;
    cfg->frame_tail[1] = MW_DEFAULT_TAIL1;
    cfg->check_type = MW_DEFAULT_CHECK_TYPE;
    cfg->check_en = MW_DEFAULT_CHECK_EN;
}

// ==========================================
// 新增：8位和校验计算
// ==========================================
static uint8_t MW_CalcSum8(uint8_t *data, uint16_t len)
{
    uint8_t sum = 0;
    for(uint16_t i=0; i<len; i++)
    {
        sum += data[i];
    }
    return sum;
}

// ==========================================
// 新增：CRC8 校验计算（标准多项式 0x07）
// ==========================================
static uint8_t MW_CalcCRC8(uint8_t *data, uint16_t len)
{
    uint8_t crc = 0;
    for(uint16_t i=0; i<len; i++)
    {
        crc ^= data[i];
        for(uint8_t j=0; j<8; j++)
        {
            if(crc & 0x80)
                crc = (crc << 1) ^ CRC8_POLY;
            else
                crc <<= 1;
        }
    }
    return crc;
}

// ==========================================
// 新增：校验验证（解析帧时调用）
// ==========================================
static uint8_t MW_VerifyCheck(MW_Ctrl_t *p, uint8_t *data, uint16_t len)
{
    if(!p->cfg.check_en) return 1;  // 未使能校验，直接通过

    uint8_t check_val = 0;
    uint16_t check_len = len - 1;  // 最后1字节是校验值（8位和/CRC8）
    if(check_len == 0) return 0;   // 无数据，校验失败

    switch(p->cfg.check_type)
    {
        case MW_CHECK_SUM8:
            check_val = MW_CalcSum8(data, check_len);
            return (check_val == data[check_len]) ? 1 : 0;

        case MW_CHECK_CRC8:
            check_val = MW_CalcCRC8(data, check_len);
            return (check_val == data[check_len]) ? 1 : 0;

        default: return 1;  // 无校验/不支持的类型，默认通过
    }
}

// ==========================================
// 原有：MW_Init（扩展默认配置初始化）
// ==========================================
void MW_Init(MW_Port_t port, uint32_t baudrate)
{
    if(port >= MW_PORT_MAX) return;

    // 1. 初始化中间件状态
    s_ctrl[port].state = S_H1;
    s_ctrl[port].idx = 0;
    s_ctrl[port].tail_shifter = 0;
    s_ctrl[port].app_cb = NULL;
    s_ctrl[port].check_buf = 0;
    s_ctrl[port].check_buf16 = 0;

    // 新增：初始化默认协议配置
    MW_ResetDefaultConfig(&s_ctrl[port].cfg);

    // 2. 转换 ID 并调用 HAL 初始化
    BSP_USART_TypeDef hal_port = MW_2_HAL_Map[port];
    HAL_UART_Init(hal_port, baudrate);

    // 3. 注册回调到 HAL
    HAL_UART_RegisterCallback(hal_port, HalCbFuncTable[port]);
}

// ==========================================
// 新增：MW_SetProtocolConfig（APP层调用）
// ==========================================
void MW_SetProtocolConfig(MW_Port_t port, const MW_Protocol_Config_t *cfg)
{
    if(port >= MW_PORT_MAX) return;

    if(cfg == NULL)
    {
        // 传入NULL，恢复默认配置
        MW_ResetDefaultConfig(&s_ctrl[port].cfg);
    }
    else
    {
        // 拷贝用户配置（浅拷贝，帧头/帧尾直接赋值）
        memcpy(&s_ctrl[port].cfg, cfg, sizeof(MW_Protocol_Config_t));
    }
}

// ==========================================
// 原有：MW_RegisterCallback（无需修改）
// ==========================================
void MW_RegisterCallback(MW_Port_t port, MW_Callback_t cb)
{
    if(port < MW_PORT_MAX)
    {
        s_ctrl[port].app_cb = cb;
    }
}

// ==========================================
// 原有：MW_Process（无需修改）
// ==========================================
void MW_Process(void)
{
    HAL_UART_Process();
}

// ==========================================
// 修改：MW_Send（适配动态配置+校验）
// ==========================================
void MW_Send(MW_Port_t port, uint8_t *data, uint16_t len, uint8_t tail_sel)
{
    if(port >= MW_PORT_MAX) return;
 
    uint8_t tx_buf[MW_RX_BUF_MAX + 10] = {0};
    uint16_t idx = 0;
    uint16_t data_len = len;
    memcpy(&tx_buf[idx], data, data_len);
    // 5. 调用HAL发送
    HAL_UART_Send(MW_2_HAL_Map[port], tx_buf, data_len);
}

// ==========================================
// 校验正常 + 接收完整帧（帧头+数据+校验+帧尾）
// ==========================================
static void ParseByte(MW_Ctrl_t *p, uint8_t b)
{
    switch(p->state)
    {
        case S_H1: 
 
            if(p->idx < MW_RX_BUF_MAX) p->buf[p->idx++] = b;
            p->state = (b == p->cfg.frame_h[0]) ? S_H2 : S_H1; 
            break;
            
        case S_H2: 

            if(p->idx < MW_RX_BUF_MAX) p->buf[p->idx++] = b;
            p->state = (b == p->cfg.frame_h[1]) ? S_H3 : S_H1; 
            break;
            
        case S_H3:

            if(p->idx < MW_RX_BUF_MAX) p->buf[p->idx++] = b;
            if(b == p->cfg.frame_h[2]) 
            {
                p->state = S_DATA;
                p->tail_shifter = 0;
                p->check_buf = 0;
                p->check_buf16 = 0;
            } 
            else 
            {
                p->state = S_H1;
                p->idx = 0; // 帧头错误，清空缓存
            }
            break;

        case S_DATA:
            // 存储数据+校验+帧尾
            if(p->idx < MW_RX_BUF_MAX) 
            {
                p->buf[p->idx++] = b;
            }
            // 帧尾匹配
            p->tail_shifter = (p->tail_shifter << 8) | b;

            if(p->tail_shifter == p->cfg.frame_tail[0] || p->tail_shifter == p->cfg.frame_tail[1])
            {
                uint16_t full_len = p->idx; 
                uint16_t check_len = full_len - 3 - 4; 
                if(p->app_cb != NULL && check_len > 0)
                {
                    // 校验正常（帧头不参与校验，完全符合协议）
                    if(MW_VerifyCheck(p, p->buf + 3, check_len))
                    {
                        // 回调完整帧：帧头+数据+校验+帧尾
                        p->app_cb(p->buf, full_len);
                    }
                }
                // 重置
                p->state = S_H1;
                p->idx = 0;
                p->tail_shifter = 0;
            }
            break;

        default: 
            p->state = S_H1; 
            p->idx = 0;
            break;
    }
}