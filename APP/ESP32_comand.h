/**
 * @file    ESP32_comand.h
 * @brief   ESP32通信指令处理模块头文件
 * @details 声明ESP32模块的数据接收、指令处理和上报函数
 *          支持小智语音助手和云端(OneNet)两种控制源
 * @version 2.0
 * @date    2026-05-04
 */

#ifndef __ESP32_COMAND_H__
#define __ESP32_COMAND_H__

#include "usart_app.h"

/**
 * @brief  ESP32接收数据包分发函数
 * @note   根据设备地址将接收到的数据分发到对应的缓冲区
 */
void ESP32_RecDataPacket(void);

/**
 * @brief  ESP32指令处理主函数
 * @note   解析并执行来自小智和云端的控制指令
 */
void ESP32_comand_process(void);

/**
 * @brief  向云端上报数据函数
 * @note   组装随机数据帧并发送到K230模块
 */
void data_to_onnet(void);

#endif /* __ESP32_COMAND_H__ */
