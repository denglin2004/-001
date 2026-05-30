#include "can_app.h"
#include "hal_can.h"
union unCAN_2A0_Data CAN2A0_Rec = {0, 0, 0, 0};  // 接受小车数据的联合体CAN2A0_Rec
union unCAN_2A1_Data CAN2A1_Rec = {0, 0, 0, 0};  // 接受平整度调整模块数据的联合体CAN2A1_Rec
union unCAN_2A2_Data CAN2A2_Rec = {0, 0, 0, 0};  // 接受FOC模块数据的联合体CAN2A2_Rec

void can_app_Init (void) 
{
    hal_can_init();
}

void app_can_data_packet(void) 
{
    u8 canrxbuf[3][8] = {0};
    hal_can_data_rec (canrxbuf[0], canrxbuf[1], canrxbuf[2]);
    for (uint8_t i = 0; i < 8; i++) 
    {
        CAN2A0_Rec.str[i] = canrxbuf[0][i];
    }
    for (uint8_t i = 0; i < 8; i++) {
        CAN2A1_Rec.str[i] = canrxbuf[1][i];
    }
    for (uint8_t i = 0; i < 8; i++) {
        CAN2A2_Rec.str[i] = canrxbuf[2][i];
    }
}

/**
 * @brief 发送小车自动控制指令
 * @param direction 小车运行方向
 * @param car_speed 小车运行速度
 * @param ack_type 对方应答类型
 * @return 无
 * @note 仅在自动运行模式(DMT_Car_ExForm_flag==2)下发送指令
 *       通过CAN总线发送8字节数据帧到小车控制器
 *       CAN-ID: 0x2B0
 *       数据格式: [指令标识][控制模式][方向][速度][应答][预留][预留][预留]
 */
void S_ComandTo_Car (uint8_t direction, uint8_t car_speed, uint8_t ack_type, 
                                        uint8_t Sequence_ID) 
{


    uint8_t sendbuf[8] = {0};               // 初始化8字节发送缓冲区
    sendbuf[0] = 1;                         // 小车指令标识 (1=有效指令)
    sendbuf[1] = 2;                         // 自动控制模式 (2=自动模式)
    sendbuf[2] = direction;                 // 小车运行方向设定
    sendbuf[3] = (car_speed > 20) ? 20 : (car_speed < 0 ? 0 : car_speed); // 小车运行速度设定
    sendbuf[4] = ack_type;                  // 对方应答类型
    sendbuf[5] = 0;                         // 预留参数位
    sendbuf[6] = 0;                         // 预留参数位
    sendbuf[7] = Sequence_ID;               // 预留参数位


    hal_Can_Send_Data (sendbuf, 8, 0x2B0);  // 通过CAN总线发送指令到小车控制器


}

/**
 * @brief 发送步进电机控制指令（十进制拆分协议）
 * @param function 步进电机功能码
 * @param enable 步进电机使能控制 (1=使能, 0=禁止)
 * @param dir 步进电机运行方向/指令：
 *            0 = 停止运行
 *            1 = 升起动作
 *            2 = 下降动作
 *            3 = 找平操作
 * @param distance 十进制距离值（自动拆分：前两位→高字节，后两位→低字节）
 *                 例：1600 → 高字节16，低字节0；580 → 高字节5，低字节80
 * @param Seq_id 指令序列号
 * @return 无
 * @note 通过CAN总线发送8字节数据帧到步进电机控制器
 *       CAN-ID: 0x2B1
 *       数据格式: [功能码][使能][指令][距离高字节][距离低字节][预留][预留][序列号]
 *       ? 调用：S_ComandTo_BuJing(3, 1, 1, 1600, 1)
 *       ? 发送：03 01 01 10 00 00 00 01（字节3=16，字节4=0）
 */
void S_ComandTo_BuJing (uint8_t function, uint8_t enable, uint8_t dir, 
                        uint16_t distance, uint8_t Seq_id) {
    static uint16_t pluse=0;
    uint8_t sendbuf[8] = {0};               // 初始化8字节发送缓冲区
    sendbuf[0] = function;                  // 字节0：功能码
    sendbuf[1] = enable;                    // 字节1：使能控制
    sendbuf[2] = dir;                       // 字节2：运行指令
    pluse=distance*100;
    sendbuf[3] = pluse / 100;            // 字节3：十进制前两位 → 高字节
    sendbuf[4] = pluse % 100;            // 字节4：十进制后两位 → 低字节
    sendbuf[5] = 0;                         // 字节5：预留
    sendbuf[6] = 0;                         // 字节6：预留
    sendbuf[7] = Seq_id;                    // 字节7：序列号
    hal_Can_Send_Data (sendbuf, 8, 0x2B1);  // 发送CAN帧
}

/**
 * @brief 发送FOC电机控制指令
 * @param eable FOC电机使能控制
 * @param Foc_Speed FOC电机速度设定值
 * @return 无
 * @details 通过CAN总线向FOC电机控制器发送控制指令，
 *          控制打磨小车的FOC电机启停和速度
 */
void S_ComandTo_FOC (uint8_t eable, uint8_t Foc_Speed, uint8_t Sequence_ID) {
    uint8_t sendbuf[8] = {0};
    sendbuf[0] = eable;
    sendbuf[1] = Foc_Speed;
    sendbuf[7] = Sequence_ID;
    hal_Can_Send_Data (sendbuf, 8, 0x2B2);  // CAN-ID:0x2B2 发送FOC电机指令
}
