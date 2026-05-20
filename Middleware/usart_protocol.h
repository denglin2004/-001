// #ifndef __USART_PROTOCOL_H
// #define __USART_PROTOCOL_H

// #ifdef __cplusplus
// extern "C" {
// #endif

// #include <stdint.h>

// // ==========================================
// // 1. 中间件自己定义的端口枚举 (APP 层只认这个)
// // ==========================================
// typedef enum
// {
//     MW_PORT_1_ESP32= 0,
//     MW_PORT_2_DCCP,
//     MW_PORT_3_DCCP,
//     MW_PORT_4_NONE,
//     MW_PORT_5_K230,
//     MW_PORT_MAX
// } MW_Port_t;

// // ==========================================
// typedef struct
// {
//     uint8_t  frame_h[3];          // 3字节帧头
//     uint32_t frame_tail[2];       // 2种可选帧尾 (32位)
//     uint8_t   check_en;           // 校验使能 (0=关闭, 1=开启)
// } MW_Protocol_Config_t;


// typedef void (*MW_Callback_t)(uint8_t *data, uint16_t len);
// // ==========================================
// // 3. API 接口声明
// // ==========================================

// /**
//  * @brief 初始化串口协议中间件
//  * @param port: 中间件端口号 (MW_PORT_1...)
//  * @param baudrate: 波特率
//  */
// void MW_Init(MW_Port_t port, uint32_t baudrate);

// /**
//  * @brief 注册应用层回调 (当中间件收到完整数据帧时调用)
//  * @param port: 中间件端口号
//  * @param cb: 回调函数指针
//  */
// void MW_RegisterCallback(MW_Port_t port, MW_Callback_t cb);

// /**
//  * @brief 中间件主循环处理 (必须在 main 的 while(1) 中调用)
//  */
// void MW_Process(void);

// /**
//  * @brief 发送数据 (自动封装 EE B1 11 ... 帧尾)
//  * @param port: 中间件端口号
//  * @param data: 要发送的纯净数据指针
//  * @param len: 数据长度
//  * @param tail_sel: 0=使用帧尾0xFFFDFFFF, 1=使用帧尾0xFFFCFFFF
//  */
// void MW_Send(MW_Port_t port, uint8_t *data, uint16_t len, uint8_t tail_sel);

// #ifdef __cplusplus
// }
// #endif

// #endif

#ifndef __USART_PROTOCOL_H
#define __USART_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// ==========================================
// 1. 中间件逻辑层端口枚举 (APP 层可见)
// ==========================================
typedef enum
{
    MW_PORT_1_ESP32= 0,
    MW_PORT_2_DCCP,
    MW_PORT_3_DCCP,
    MW_PORT_4_NONE,
    MW_PORT_5_K230,
    MW_PORT_MAX
} MW_Port_t;

// ==========================================
// 新增：校验类型枚举 (APP 层可选择)
// ==========================================
typedef enum
{
    MW_CHECK_NONE = 0,    // 无校验（默认）
    MW_CHECK_SUM8,        // 8位和校验
    MW_CHECK_CRC8,        // CRC8校验（常用标准：多项式0x07）
    MW_CHECK_CRC16        // 可选扩展：CRC16（可根据需求开启）
} MW_CheckType_t;

// ==========================================
// 新增：协议配置结构体 (APP 层可修改)
// ==========================================
typedef struct
{
    uint8_t frame_h[3];       // 帧头（3字节，默认：0xEE,0xB1,0x11）
    uint32_t frame_tail[2];   // 帧尾（2种，默认：0xFFFDFFFF / 0xFFFCFFFF）
    MW_CheckType_t check_type;// 校验类型（默认：无校验）
    uint8_t check_en;         // 校验使能（默认：0，关闭）
} MW_Protocol_Config_t;

// ==========================================
// 2. 回调函数类型 (APP 层可见)
// ==========================================
typedef void (*MW_Callback_t)(uint8_t *data, uint16_t len);

// ==========================================
// 3. API 接口列表 (新增配置设置接口)
// ==========================================

/**
 * @brief 初始化串口协议中间件
 * @param port: 中间件端口号 (MW_PORT_1...)
 * @param baudrate: 波特率
 */
void MW_Init(MW_Port_t port, uint32_t baudrate);

/**
 * @brief 注册回调函数（接收完整帧时触发）
 * @param port: 中间件端口号
 * @param cb: 回调函数指针
 */
void MW_RegisterCallback(MW_Port_t port, MW_Callback_t cb);

/**
 * @brief 中间件轮询处理（需放在 main 循环）
 */
void MW_Process(void);

/**
 * @brief 发送数据（自动组帧：帧头+数据+校验+帧尾）
 * @param port: 中间件端口号
 * @param data: 待发送数据指针
 * @param len: 数据长度
 * @param tail_sel: 帧尾选择（0=frame_tail[0], 1=frame_tail[1]）
 */
void MW_Send(MW_Port_t port, uint8_t *data, uint16_t len, uint8_t tail_sel);

/**
 * @brief 新增：设置协议配置（APP层调用，不调用则用默认值）
 * @param port: 中间件端口号
 * @param cfg: 协议配置结构体指针（传入NULL则恢复默认配置）
 */
void MW_SetProtocolConfig(MW_Port_t port, const MW_Protocol_Config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif