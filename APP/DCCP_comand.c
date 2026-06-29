/**
 * @file    DCCP_comand.c
 * @brief   大彩串口屏通信指令处理模块
 * @details 实现大彩串口屏(DCCP)的数据收发、指令解析和显示更新功能
 *          支持的功能：
 *          - 自动S路径参数设置（画面1）
 *          - 小车手动方向控制（画面2）
 *          - 参数调试设置（画面3）
 *          - 实时数据轮询显示（转速/温度/电压/电流/高度/进度）
 * @version 2.0
 * @date    2026-05-04
 */

/* =================== 头文件包含 =================== */
#include "DCCP_comand.h"
#include "DCCP_struct.h"
#include "can_app.h"
#include "usart_app.h"
#include "usart_protocol.h"
#include <string.h>
#include "hal_rgb.h"
#include "Timer.h"
#include "core_control.h"
#include "SSD1306.h"

/* =================== 全局变量定义 =================== */
DCCP_Temp_Typedef g_dccp_temp = {0};    // 存储彩屏参数设置的临时变量

/* =================== 内部函数前向声明 =================== */
static void Car_Dir_Control(uint8_t idx, uint8_t dir, uint8_t cmdParam);

/* =================== 工具函数 =================== */

/**
 * @brief  数字转ASCII字符串函数
 * @param  str: 输出的ASCII字符串缓冲区
 * @param  dispWord: 要转换的无符号整数
 * @param  Len_Change: 转换后的字符串长度（不足补前导0）
 * @note   用于向大彩屏发送显示数据
 */
void DatachagASCII(unsigned char *str, unsigned int dispWord, unsigned char Len_Change)
{
    unsigned int tmp_Data;
    signed char i;

    tmp_Data = dispWord;
    /* 从低位到高位逐位转换 */
    for (i = Len_Change; i >= 0; i--)
    {
        if (tmp_Data > 0)
        {
            str[i] = tmp_Data % 10 + 0x30;  // 数字转ASCII字符（0x30='0'的ASCII码）
            tmp_Data /= 10;
        }
        else
        {
            str[i] = 0x30;  // 高位补0
        }
    }
}

/**
 * @brief  ASCII字符串转数字函数
 * @param  Rstr: 输入的ASCII字符串缓冲区
 * @param  LEN_Rstr: 字符串最大长度
 * @retval 转换后的无符号整数
 * @note   用于解析大彩屏下发的数字输入参数
 */
unsigned int ASCIIchagData(unsigned char *Rstr, unsigned char LEN_Rstr)
{
    unsigned int Rtmp_Data = 0;
    unsigned int Rtmp_Data0 = 0;
    unsigned char Ri;

    /* 从高位到低位逐位转换 */
    for (Ri = 0; Ri < LEN_Rstr; Ri++)
    {
        if (Rstr[Ri] != 0)  // 遇到字符串结束符'\0'停止
        {
            Rtmp_Data0 = (Rstr[Ri] - 0x30);  // ASCII字符转数字
            Rtmp_Data = Rtmp_Data * 10 + Rtmp_Data0;
        }
        else
        {
            break;
        }
    }
    return Rtmp_Data;
}

/* =================== 小车手动控制函数 =================== */

/**
 * @brief  小车方向控制函数（处理手动模式下的按钮事件）
 * @param  idx: 方向索引（0-3对应前后左右）
 * @param  dir: 方向值（0x01-0x04对应前后左右）
 * @param  cmdParam: 按钮状态（0x01=按下, 0x00=松开）
 * @note   实现按钮点动控制，记录按下时间作为运动持续时间
 */
static void Car_Dir_Control(uint8_t idx, uint8_t dir, uint8_t cmdParam)
{
    /* 如果是自动模式（模式2），忽略手动控制 */
    if (g_car.ExForm_mode == 2) return;

    /* 按钮按下事件：只在第一次收到0x01时执行一次，防止循环重复触发 */
    if ((cmdParam == 0x01) && (g_car.timer_en[idx] == 0))
    {
        g_car.timer_en[idx] = 1;
        g_car.start_tick[idx] = GetTick();  // 记录按钮按下的开始时间
        g_car.run_flag[idx] = 0;            // 清除运动完成标志
        Drv_RGB_Green();                     // 按钮按下时亮绿灯
    }
    /* 按钮松开事件：只在第一次收到0x00时执行一次 */
    else if ((cmdParam == 0x00) && (g_car.timer_en[idx] == 1))
    {
        g_car.timer_en[idx] = 0;
        /* 计算按钮按下的持续时间 = 小车需要运动的时间(ms) */
        g_car.run_time[idx] = (GetTick() - g_car.start_tick[idx]) * TICK_MS;
        g_car.ExForm_mode = 1;              // 进入按钮点动模式
        g_car.ExForm_diretcion = dir;       // 设置小车运动方向
        g_car.start_tick[idx] = GetTick();  // 重新计时，记录小车运动开始时间
    }
}

/**
 * @brief  按钮控制小车运动主函数
 * @note   在主循环中周期性调用，实现按钮点动控制小车运动
 *         运动时间由按钮按下持续时间决定
 */
void Button_Control_Car(void)
{
    /* 如果是自动模式，忽略手动控制 */
    if (g_car.ExForm_mode == 2)
    {
        return;
    }

    uint8_t idx = 0;
    uint32_t pass_time = 0;

    /* 发送计数器，用于控制CAN指令发送频率 */
    static u32 send_cnt[4] = {10, 10, 10, 10};
    /* 发送阈值，每10次循环发送一次CAN指令 */
    const u32 SEND_CNT_THRESHOLD = 10;

    /* 只有在按钮点动模式下才执行 */
    if (g_car.ExForm_mode != 1) return;
    /* 没有设置方向时直接返回 */
    if (g_car.ExForm_diretcion == 0) return;

    /* 根据方向值获取对应的索引 */
    switch (g_car.ExForm_diretcion)
    {
    case 0x01: idx = 0; break;  // 前进
    case 0x02: idx = 1; break;  // 后退
    case 0x03: idx = 2; break;  // 左转
    case 0x04: idx = 3; break;  // 右转
    default: return;
    }

    /* 如果该方向运动已经完成，直接返回 */
    if (g_car.run_flag[idx] == 1) return;

    /* 计算小车已经运动的时间 */
    pass_time = (GetTick() - g_car.start_tick[idx]) * TICK_MS;

    /* 如果运动时间未到，继续发送运动指令 */
    if (pass_time < g_car.run_time[idx])
    {
        send_cnt[idx]--;
        /* 控制发送频率，防止CAN总线拥堵 */
        if (send_cnt[idx] > 0)
        {
            return;
        }
        /* 重置发送计数器 */
        send_cnt[idx] = SEND_CNT_THRESHOLD;

        /* 根据方向发送对应的CAN控制指令 */
        switch (g_car.ExForm_diretcion)
        {
        case 0x01:
            S_ComandTo_Car(1, 50, 1, 1);               // 前进指令，速度50
            Drv_RGB_Blue();                              // 运动时亮蓝灯
            break;
        case 0x02:
            S_ComandTo_Car(2, 50, 1, 2);               // 后退指令，速度50
            Drv_RGB_SetColor(RGB_COLOR_WHITE);          // 运动时亮白灯
            break;
        case 0x03:
            S_ComandTo_Car(3, 50, 1, 3);               // 左转指令，速度50
            Drv_RGB_SetColor(RGB_COLOR_CYAN);           // 运动时亮青灯
            break;
        case 0x04:
            S_ComandTo_Car(4, 50, 1, 4);               // 右转指令，速度50
            Drv_RGB_SetColor(RGB_COLOR_MAGENTA);        // 运动时亮品红灯
            break;
        default: break;
        }
    }
    /* 运动时间到，停止小车 */
    else
    {
        S_ComandTo_Car(5, 0, 1, 5);                    // 停止指令
        Drv_RGB_SetColor(RGB_COLOR_RED);                // 停止时亮红灯
        send_cnt[idx] = SEND_CNT_THRESHOLD;
        g_car.run_flag[idx] = 1;                        // 标记该方向运动完成
        g_car.ExForm_diretcion = 0;                     // 清除方向
        g_car.ExForm_mode = 0;                          // 退出按钮点动模式
    }
}

/* =================== 大彩屏数据收发函数 =================== */

/**
 * @brief  大彩屏接收数据组包函数
 * @note   处理大小端和帧结构，大彩屏是小端模式，16位数据低字节在前
 *         手动交换字节顺序以适配MCU内存布局
 */
void DCCP_RecDataPacket(void)
{
    uint8_t Rec_i = 0;

    /* 帧头和前几个固定字节，手动交换16位数据的高低字节 */
    DCCP_Rec.str[0] = DCCP_rx_t.DCCP_rx_buf[0];       // 帧头
    DCCP_Rec.str[1] = DCCP_rx_t.DCCP_rx_buf[1];       // 命令类型
    DCCP_Rec.str[2] = DCCP_rx_t.DCCP_rx_buf[2];       // 消息类型
    DCCP_Rec.str[4] = DCCP_rx_t.DCCP_rx_buf[3];       // 画面ID低字节
    DCCP_Rec.str[3] = DCCP_rx_t.DCCP_rx_buf[4];       // 画面ID高字节
    DCCP_Rec.str[6] = DCCP_rx_t.DCCP_rx_buf[5];       // 控件ID低字节
    DCCP_Rec.str[5] = DCCP_rx_t.DCCP_rx_buf[6];       // 控件ID高字节
    DCCP_Rec.str[7] = DCCP_rx_t.DCCP_rx_buf[7];       // 事件类型

    /* 复制可变长度的参数部分 */
    for (Rec_i = 0; Rec_i <= (DCCP_rx_t.DCCP_rx_len - 13); Rec_i++)
    {
        DCCP_Rec.str[8 + Rec_i] = DCCP_rx_t.DCCP_rx_buf[8 + Rec_i];
    }

    /* 组装32位帧尾（大端模式，高字节在前） */
    DCCP_Rec.sDCCP.Frame_Tail =
        ((unsigned long)DCCP_rx_t.DCCP_rx_buf[DCCP_rx_t.DCCP_rx_len - 4] << 24) |
        ((unsigned long)DCCP_rx_t.DCCP_rx_buf[DCCP_rx_t.DCCP_rx_len - 3] << 16) |
        ((unsigned long)DCCP_rx_t.DCCP_rx_buf[DCCP_rx_t.DCCP_rx_len - 2] << 8) |
        ((unsigned long)DCCP_rx_t.DCCP_rx_buf[DCCP_rx_t.DCCP_rx_len - 1]);
}

/**
 * @brief  大彩屏命令处理主函数
 * @note   解析并执行大彩屏下发的所有指令
 *         支持3个画面的控件事件处理：
 *         - 画面0x01: 自动S路径参数设置
 *         - 画面0x02: 小车手动控制
 *         - 画面0x03: 参数调试
 */
void DCCP_comand_process(void)
{
    /* ========== 画面ID=0x01：小车自动S路径规划参数输入画面 ========== */
    if (DCCP_Rec.sDCCP.Frame_ImageID == 0x01)
    {
        switch (DCCP_Rec.sDCCP.Frame_ControlID)
        {
        case SET_CMD_DMTHIGHT:  // 打磨头高度设置输入框
        {
            u16 high_temp = 0;
            /* 手动解析2位ASCII数字（大彩屏下发的是ASCII字符串） */
            high_temp += (DCCP_Rec.sDCCP.Frame_param[0] != 0) ? (DCCP_Rec.sDCCP.Frame_param[0] - 48) * 10 : 0;
            high_temp += (DCCP_Rec.sDCCP.Frame_param[1] != 0) ? (DCCP_Rec.sDCCP.Frame_param[1] - 48) : 0;

            /* 向步进电机发送高度控制指令 */
            S_ComandTo_BuJing(3, 1, 1, high_temp, 10);
            DCCP_Rec.sDCCP.Frame_ControlID = 0;  // 清除控件ID，防止重复处理
        }
        break;

        case SET_CMD_DMTUP:  // 打磨头上升按钮
        {
            DCCP_Rec.sDCCP.Frame_ControlID = 0;
            /* 按钮松开时执行（防止长按重复触发） */
            if (DCCP_Rec.sDCCP.Frame_param[1] == 0x00)
            {
                S_ComandTo_BuJing(3, 1, 1, 800, 11);  // 上升800步
            }
        }
        break;

        case SET_CMD_DMTDOWN:  // 打磨头下降按钮
        {
            DCCP_Rec.sDCCP.Frame_ControlID = 0;
            /* 按钮松开时执行 */
            if (DCCP_Rec.sDCCP.Frame_param[1] == 0x00)
            {
                S_ComandTo_BuJing(3, 1, 1, 800, 12);  // 下降800步
            }
        }
        break;

        case 0x0A:
            g_car.Run_S_X = DCCP_Rec.sDCCP.Frame_param[0];  // X轴步数设置
            break;
        case 0x0B:
            g_car.Run_S_Y = DCCP_Rec.sDCCP.Frame_param[0];  // Y轴循环数设置
            break;

        default:
            break;
        }

        /* 如果X轴和Y轴步数都已设置，写入统一任务请求结构 */
        if (g_car.Run_S_X && g_car.Run_S_Y)
        {
            static uint8_t Run_S_X_old, Run_S_Y_old = 0;
            /* 只有当步数发生变化时才重新触发 */
            if (g_car.Run_S_X != Run_S_X_old || g_car.Run_S_Y != Run_S_Y_old)
            {
                g_s_task_req.step_x    = g_car.Run_S_X - 48;  // ASCII转数字
                g_s_task_req.loop_y    = g_car.Run_S_Y - 48;
                g_s_task_req.foc_speed = g_dccp_temp.foc_speed;
                g_s_task_req.car_speed = g_dccp_temp.car_speed;
                g_s_task_req.lift_high = g_dccp_temp.lift_high;
                g_s_task_req.source    = PARAM_SRC_DCCP;
                g_s_task_req.trigger   = 1;                    // 请求启动S任务
                Run_S_X_old = g_car.Run_S_X;
                Run_S_Y_old = g_car.Run_S_Y;
            }
            /* 清除手动模式状态 */
            g_car.ExForm_diretcion = 0;
            memset(g_car.timer_en, 0, sizeof(g_car.timer_en));
        }
    }

    /* ========== 画面ID=0x02：小车手动控制画面 ========== */
    if (DCCP_Rec.sDCCP.Frame_ImageID == 0x02)
    {
        uint8_t par = DCCP_Rec.sDCCP.Frame_param[1];  // 按钮状态（0x01=按下, 0x00=松开）
        switch (DCCP_Rec.sDCCP.Frame_ControlID)
        {
        case SET_CMD_CAR_FORWORD:   Car_Dir_Control(0, 0x01, par); break;  // 前进按钮
        case SET_CMD_CAR_BACKWORD:  Car_Dir_Control(1, 0x02, par); break;  // 后退按钮
        case SET_CMD_CAR_TURNLEFT:  Car_Dir_Control(2, 0x03, par); break;  // 左转按钮
        case SET_CMD_CAR_TURNRIGHT: Car_Dir_Control(3, 0x04, par); break;  // 右转按钮
        default: break;
        }
    }

    /* ========== 画面ID=0x03：参数调试画面 ========== */
    if (DCCP_Rec.sDCCP.Frame_ImageID == 0x03)
    {
        switch (DCCP_Rec.sDCCP.Frame_ControlID)
        {
        case 0x03:
            g_car.Run_distance = DCCP_Rec.sDCCP.Frame_param[3];  // 单次运行距离
            DCCP_Rec.sDCCP.Frame_ControlID = 0;
            break;
        case 0x04:
            g_car.Run_cnt_x = DCCP_Rec.sDCCP.Frame_param[3];    // X轴运行次数
            DCCP_Rec.sDCCP.Frame_ControlID = 0;
            break;
        case 0x05:
            g_car.Run_cnt_y = DCCP_Rec.sDCCP.Frame_param[3];    // Y轴运行次数
            DCCP_Rec.sDCCP.Frame_ControlID = 0;
            break;
        case 0x06:
            g_grind.high = DCCP_Rec.sDCCP.Frame_param[3];       // 打磨头高度
            S_ComandTo_BuJing(3, 1, 1, g_grind.high, 12);       // 发送上升控制指令
            DCCP_Rec.sDCCP.Frame_ControlID = 0;
            break;
        case 0x07:
            g_grind.high = DCCP_Rec.sDCCP.Frame_param[3];       // 打磨头高度
            S_ComandTo_BuJing(3, 1, 0, g_grind.high, 12);       // 发送下降控制指令
            DCCP_Rec.sDCCP.Frame_ControlID = 0;
            break;
        case 0x08:
            g_grind.high = DCCP_Rec.sDCCP.Frame_param[3];       // 打磨头高度
            S_ComandTo_BuJing(3, 1, 1, g_grind.high, 12);       // 发送上升控制指令
            DCCP_Rec.sDCCP.Frame_ControlID = 0;
            break;
        case 0x09:
            /* FOC电机转速：将百分比(0-100)转换为PWM值(0-255) */
            g_foc.speed = (u8)((float)(DCCP_Rec.sDCCP.Frame_param[3] / 100.0) * 255);
            g_dccp_temp.foc_speed = g_foc.speed;
            S_ComandTo_FOC(1, g_dccp_temp.foc_speed, 15);       // 发送转速控制指令
            DCCP_Rec.sDCCP.Frame_ControlID = 0;
            break;

        default:
            break;
        }
    }
}

/* =================== 大彩屏显示更新函数 =================== */

/**
 * @brief  大彩屏显示数据处理函数
 * @note   周期性向大彩屏发送实时数据
 *         采用轮询方式，每次只发送一组数据，防止串口拥堵
 *         显示项目：转速 -> 温度 -> 电压 -> 电流 -> 高度 -> 进度
 */
void DCCP_Disp_Process(void)
{
    /* 获取实时数据（实际项目中应从CAN总线获取FOC和步进电机的实时数据） */
    Cur_DMT_Speed = 100;
    Cur_DMT_Temp  = 22;
    Cur_DMT_VOL   = 24;
    Cur_DMT_I     = 1;
    Cur_DMT_High  = CAN2A1_Rec.sCAN.pzd_high;

    /* 轮询发送不同的显示数据 */
    switch (Disp_Turn)
    {
    case DISP_DMTSPEED:  // 显示FOC电机转速
        DCCP_Send.sDCCP.Frame_Sendcmd_type  = 0xB1;
        DCCP_Send.sDCCP.Frame_Sendctrl_msg  = 0x10;
        DCCP_Send.sDCCP.Frame_SendImageID   = 0x0001;
        DCCP_Send.sDCCP.Frame_SendControlID = 0x001A;
        if (Cur_DMT_Speed >= 999) Cur_DMT_Speed = 999;
        DatachagASCII(DCCP_Send.sDCCP.Frame_Sendparam, Cur_DMT_Speed, 2);
        DCCP_SendFrameInfo(3);
        Disp_Turn = DISP_DMTTEMP;
        break;

    case DISP_DMTTEMP:  // 显示FOC电机温度
        DCCP_Send.sDCCP.Frame_Sendcmd_type  = 0xB1;
        DCCP_Send.sDCCP.Frame_Sendctrl_msg  = 0x10;
        DCCP_Send.sDCCP.Frame_SendImageID   = 0x0001;
        DCCP_Send.sDCCP.Frame_SendControlID = 0x0004;
        if (Cur_DMT_Temp >= 999) Cur_DMT_Temp = 999;
        DatachagASCII(DCCP_Send.sDCCP.Frame_Sendparam, Cur_DMT_Temp, 2);
        DCCP_SendFrameInfo(3);
        Disp_Turn = DISP_DMTVOL;
        break;

    case DISP_DMTVOL:  // 显示FOC电机电压
        DCCP_Send.sDCCP.Frame_Sendcmd_type  = 0xB1;
        DCCP_Send.sDCCP.Frame_Sendctrl_msg  = 0x10;
        DCCP_Send.sDCCP.Frame_SendImageID   = 0x0001;
        DCCP_Send.sDCCP.Frame_SendControlID = 0x0005;
        if (Cur_DMT_VOL >= 999) Cur_DMT_VOL = 999;
        DatachagASCII(DCCP_Send.sDCCP.Frame_Sendparam, Cur_DMT_VOL, 2);
        DCCP_SendFrameInfo(3);
        Disp_Turn = DISP_DMTCUR;
        break;

    case DISP_DMTCUR:  // 显示FOC电机电流
        DCCP_Send.sDCCP.Frame_Sendcmd_type  = 0xB1;
        DCCP_Send.sDCCP.Frame_Sendctrl_msg  = 0x10;
        DCCP_Send.sDCCP.Frame_SendImageID   = 0x0001;
        DCCP_Send.sDCCP.Frame_SendControlID = 0x0006;
        if (Cur_DMT_I >= 999) Cur_DMT_I = 999;
        DatachagASCII(DCCP_Send.sDCCP.Frame_Sendparam, Cur_DMT_I, 2);
        DCCP_SendFrameInfo(3);
        Disp_Turn = DISP_DMTHIGH;
        break;

    case DISP_DMTHIGH:  // 显示打磨头高度
        DCCP_Send.sDCCP.Frame_Sendcmd_type  = 0xB1;
        DCCP_Send.sDCCP.Frame_Sendctrl_msg  = 0x10;
        DCCP_Send.sDCCP.Frame_SendImageID   = 0x0001;
        DCCP_Send.sDCCP.Frame_SendControlID = 0x0008;
        if (Cur_DMT_High >= 999) Cur_DMT_High = 999;
        DatachagASCII(DCCP_Send.sDCCP.Frame_Sendparam, Cur_DMT_High, 2);
        DCCP_SendFrameInfo(3);
        Disp_Turn = DISP_TASK_PROGRESS;
        break;

    case DISP_TASK_PROGRESS:  // 显示任务完成进度
        DCCP_Send.sDCCP.Frame_Sendcmd_type  = 0xB1;
        DCCP_Send.sDCCP.Frame_Sendctrl_msg  = 0x10;
        DCCP_Send.sDCCP.Frame_SendImageID   = 0x0001;
        DCCP_Send.sDCCP.Frame_SendControlID = 0x001B;
        if (grindcar_ctrl.Loop_Finished >= 999) grindcar_ctrl.Loop_Finished = 999;
        DatachagASCII(DCCP_Send.sDCCP.Frame_Sendparam, grindcar_ctrl.Loop_Finished, 2);
        DCCP_SendFrameInfo(3);
        DCCP_SendFrameInfo(3);  // 重复发送一次，确保可靠
        Disp_Turn = DISP_DMTSPEED;  // 重新回到第一个显示项
        break;

    default:
        Disp_Turn = DISP_DMTSPEED;
        break;
    }
}

/**
 * @brief  大彩屏数据帧发送函数
 * @param  Send_Len: 参数部分的长度
 * @note   组装完整的通信帧并发送
 *         处理大小端转换，手动交换16位数据的高低字节（适配大彩屏小端模式）
 */
void DCCP_SendFrameInfo(uint16_t Send_Len)
{
    uint8_t Frame_Info[32];
    uint8_t i = 0;

    /* 组装帧头和固定字节，手动交换16位数据的高低字节 */
    Frame_Info[0] = DCCP_Send.str[0];  // 帧头
    Frame_Info[1] = DCCP_Send.str[1];  // 命令类型
    Frame_Info[2] = DCCP_Send.str[2];  // 消息类型
    Frame_Info[3] = DCCP_Send.str[4];  // 画面ID低字节
    Frame_Info[4] = DCCP_Send.str[3];  // 画面ID高字节
    Frame_Info[5] = DCCP_Send.str[6];  // 控件ID低字节
    Frame_Info[6] = DCCP_Send.str[5];  // 控件ID高字节

    /* 复制参数部分 */
    for (i = 0; i < Send_Len; i++)
    {
        Frame_Info[7 + i] = DCCP_Send.sDCCP.Frame_Sendparam[i];
    }

    /* 组装帧尾 */
    Frame_Info[7 + Send_Len]  = DCCP_Send.str[16];
    Frame_Info[8 + Send_Len]  = DCCP_Send.str[17];
    Frame_Info[9 + Send_Len]  = DCCP_Send.str[18];
    Frame_Info[10 + Send_Len] = DCCP_Send.str[19];

    /* 通过串口2发送数据帧到大彩屏 */
    MW_Send(MW_PORT_2_DCCP, Frame_Info, (11 + Send_Len), 0);
}
