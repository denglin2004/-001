#include "bsp_can.h"      // 包含CAN总线相关函数和定义的头文件
CanRxMsg CanRxStructure;  // CAN接收消息结构体
uint8_t canrxbuf[3][8] = {0};

struct CANFilterStruct_t {
    union      // 滤波规则联合体（包含2组滤波值，用于ID过滤）
    {
        union  // 滤波值的访问联合体（区分标准帧和扩展帧）
        {
            // 扩展帧ID访问结构（29位扩展ID）
            struct
            {
                uint32_t : 1;        // 预留位（1位，无实际意义）
                uint32_t RTR : 1;    // 远程传输请求位（1位：0=数据帧，1=远程帧）
                uint32_t IDE : 1;    // 标识符扩展位（1位：0=标准帧，1=扩展帧）
                uint32_t ExID : 29;  // 29位扩展ID
            } Access_Ex;

            // 标准帧ID访问结构（11位标准ID）
            struct
            {
                uint32_t : 1;        // 预留位（1位）
                uint32_t RTR : 1;    // 远程传输请求位（1位）
                uint32_t IDE : 1;    // 标识符扩展位（1位）
                uint32_t : 18;       // 预留位（18位，因标准ID仅11位）
                uint32_t StID : 11;  // 11位标准ID
            } Access_St;
        };

        // 滤波值的数值存储联合体（支持16位拆分和32位整体访问）
        union {
            struct {
                uint16_t FR_16_L;  // 滤波值低16位
                uint16_t FR_16_H;  // 滤波值高16位
            };

            uint32_t FR_32;  // 滤波值32位整体
        };
    } FR[2];                 // 存储2组滤波值（通常一组为ID，一组为掩码）

    union                    // 滤波器控制参数联合体
    {
        struct
        {
            uint16_t en : 1;     // 使能位（1位：0=禁用，1=使能）
            uint16_t mode : 4;   // 滤波模式（4位：ID列表/ID掩码等）
            uint16_t scale : 3;  // 滤波尺度（3位：16位/32位滤波）
        };

        uint16_t ctrl_byte;                      // 控制字节（整体存储上述参数）
    };
} CANFilterStruct[CANSOFTFILTER_MAX_GROUP_NUM];  // 软件滤波器数组（支持多组滤波）

// CAN2滤波器起始组号（用于区分CAN1和CAN2的滤波器组）
int CAN2FilterStartBank = CANSOFTFILTER_MAX_GROUP_NUM;


// 声明CAN1接收中断函数（使用WCH快速中断属性）
void USB_LP_CAN1_RX0_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
// CAN1接收中断服务函数（处理FIFO0的接收中断）
void USB_LP_CAN1_RX0_IRQHandler() 
{
    int i;
    uint8_t px;   // 接收数据长度
    u8 pbuf[20];  // 接收数据缓冲区（CAN最大8字节）

    // 检查CAN1的FIFO0消息挂起中断（FMP0：FIFO0消息 pending）
    if (CAN_GetITStatus (CAN1, CAN_IT_FMP0)) 
    {
        // 接收CAN消息，返回数据长度
        px = CAN_Receive_Msg (pbuf);
        if (px > 8)    px = 8;  // 限制最大长度为8字节
                     // 按ID分类存储数据到canrxbuf
        switch (CanRxStructure.StdId) {
        case 0x2A0:  // 匹配2A0，存入canrxbuf[0]
            for (i = 0; i < 8; i++)
                canrxbuf[0][i] = CanRxStructure.Data[i];
            break;
        case 0x2A1:  // 匹配2A1，存入canrxbuf[1]
            for (i = 0; i < 8; i++)
                canrxbuf[1][i] = CanRxStructure.Data[i];
            break;
        case 0x2A2:  // 匹配2A2，存入canrxbuf[2]
            for (i = 0; i < 8; i++)
                canrxbuf[2][i] = CanRxStructure.Data[i];
            break;
        default:  // 未知ID，不处理
            break;
        }


        // 清除CAN1的FIFO0中断挂起位
        CAN_ClearITPendingBit (CAN1, CAN_IT_FMP0);
    }
}

void bsp_can_data_rec(uint8_t *can_rxbuf1,uint8_t *can_rxbuf2,uint8_t* can_rxbuf3)
{
    for(uint8_t i = 0; i < 8; i++)
    {
        can_rxbuf1[i]=canrxbuf[0][i];
    }
    for(uint8_t i = 0; i < 8; i++)
    {
        can_rxbuf2[i]=canrxbuf[1][i];
    }
    for(uint8_t i = 0; i < 8; i++)
    {
        can_rxbuf3[i]=canrxbuf[2][i];
    }

}

// 初始化CAN软件滤波器（将硬件滤波配置转换为软件滤波参数）
void CAN_SoftFilterInit (CAN_FilterInitTypeDef *CAN_FilterInitStruct) {
    // 检查滤波器组号是否超出最大支持数量，超出则退出
    if (CAN_FilterInitStruct->CAN_FilterNumber > CANSOFTFILTER_MAX_GROUP_NUM) {
        return;
    }

    // 根据硬件滤波的使能状态设置软件滤波器使能位
    if (CAN_FilterInitStruct->CAN_FilterActivation) {
        CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].en = 1;  // 使能滤波器
    } else {
        CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].en = 0;  // 禁用滤波器
    }

    // 复制硬件滤波的ID和掩码到软件滤波器（高低16位拆分存储）
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].FR[0].FR_16_H = CAN_FilterInitStruct->CAN_FilterIdHigh;
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].FR[0].FR_16_L = CAN_FilterInitStruct->CAN_FilterIdLow;
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].FR[1].FR_16_H = CAN_FilterInitStruct->CAN_FilterMaskIdHigh;
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].FR[1].FR_16_L = CAN_FilterInitStruct->CAN_FilterMaskIdLow;

    // 复制滤波模式和尺度（ID列表/掩码模式，16位/32位滤波）
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].mode = CAN_FilterInitStruct->CAN_FilterMode;
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].scale = CAN_FilterInitStruct->CAN_FilterScale;
}

// 通过软件滤波器接收CAN消息（根据软件配置的滤波规则筛选消息）
void CAN_ReceiveViaSoftFilter (CAN_TypeDef *CANx, uint8_t FIFONumber, CanRxMsg *RxMessage) {
    int group;                 // 滤波器组索引
    int start_bank, end_bank;  // 滤波器组的起始和结束索引（区分CAN1和CAN2）

    // 根据CAN外设类型（CAN1/CAN2）设置滤波器组范围
    if (CANx == CAN1) {
        start_bank = 0;
        end_bank = CAN2FilterStartBank;  // CAN1使用0到CAN2起始组之间的滤波器
    } else {
        start_bank = CAN2FilterStartBank;
        end_bank = CANSOFTFILTER_MAX_GROUP_NUM;  // CAN2使用CAN2起始组到最大组的滤波器
    }

    // 遍历指定范围内的滤波器组
    for (group = start_bank; group < end_bank; group++) {
        // 仅处理已使能的滤波器
        if (CANFilterStruct[group].en) {
            // 获取接收消息的ID（屏蔽最低1位，因最低位可能为保留位）
            uint32_t temp = CANx->sFIFOMailBox[0].RXMIR & (~0x1);

            // 根据滤波器控制字节（模式+尺度）选择滤波逻辑
            switch ((uint8_t)CANFilterStruct[group].ctrl_byte & ~0x1)  // 屏蔽使能位，仅保留模式和尺度
            {
            // 32位ID列表模式（消息ID需与滤波ID完全匹配）
            case CANSOFTFILER_PREDEF_CTRLBYTE_ID32:
                // 若消息ID与两组滤波ID均不匹配，则跳过当前滤波器
                if ((CANFilterStruct[group].FR[0].FR_32 != temp) && (CANFilterStruct[group].FR[1].FR_32 != temp)) {
                    continue;
                } else {
                    // 匹配成功，接收消息并返回
                    CAN_Receive (CANx, CAN_FIFO0, RxMessage);
                    return;
                }
                break;

            // 32位ID掩码模式（消息ID与滤波ID按掩码匹配）
            case CANSOFTFILER_PREDEF_CTRLBYTE_MASK32:
                // 按掩码计算：(滤波ID & 掩码) 需与 (消息ID & 掩码) 相等
                if ((CANFilterStruct[group].FR[0].FR_32 & CANFilterStruct[group].FR[1].FR_32) ^ (temp & CANFilterStruct[group].FR[1].FR_32)) {
                    continue;  // 不匹配，跳过
                } else {
                    // 匹配成功，接收消息并返回
                    CAN_Receive (CANx, CAN_FIFO0, RxMessage);
                    return;
                }
                break;

            // 未定义的滤波模式，直接返回
            default:
                return;
                break;
            }
        }
    }

    // 所有滤波器均不匹配，释放FIFO（丢弃该消息）
    CAN_FIFORelease (CANx, CAN_FIFO0);
}

// 设置CAN2滤波器的起始组号（用于划分CAN1和CAN2的滤波器组）
void CAN_SoftSlaveStartBank (uint8_t CAN_BankNumber) {
    CAN2FilterStartBank = CAN_BankNumber;
}

/**
 * @brief 初始化CAN控制器模式及相关配置
 * @param tsjw 同步跳转宽度 (SJW)，用于配置CAN通信的同步阶段时间
 * @param tbs2 时间段2 (BS2)，用于配置CAN通信的相位缓冲段2时间
 * @param tbs1 时间段1 (BS1)，用于配置CAN通信的传播段和相位缓冲段1时间
 * @param brp 预分频器值，用于计算CAN通信波特率
 * @param mode CAN工作模式 (正常模式/回环模式等)
 */
void CAN_Mode_Init (u8 tsjw, u8 tbs2, u8 tbs1, u16 brp, u8 mode) {
    GPIO_InitTypeDef GPIO_InitSturcture = {0};            // GPIO初始化结构体
    CAN_InitTypeDef CAN_InitSturcture = {0};              // CAN初始化结构体
    CAN_FilterInitTypeDef CAN_FilterInitSturcture = {0};  // CAN滤波器初始化结构体

    // 使能AFIO复用功能时钟、GPIOA端口时钟
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);
    // 使能CAN1外设时钟
    RCC_APB1PeriphClockCmd (RCC_APB1Periph_CAN1, ENABLE);

    // 配置CAN发送引脚PA12
    GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_AF_PP;    // 复用推挽输出模式
    GPIO_InitSturcture.GPIO_Speed = GPIO_Speed_50MHz;  // 输出速率50MHz
    GPIO_Init (GPIOA, &GPIO_InitSturcture);

    // 配置CAN接收引脚PA11
    GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入模式
    GPIO_Init (GPIOA, &GPIO_InitSturcture);

    // 配置CAN初始化参数
    CAN_InitSturcture.CAN_TTCM = DISABLE;   // 禁用时间触发通信模式
    CAN_InitSturcture.CAN_ABOM = DISABLE;   // 禁用自动离线管理
    CAN_InitSturcture.CAN_AWUM = DISABLE;   // 禁用自动唤醒模式
    CAN_InitSturcture.CAN_NART = ENABLE;    // 使能自动重传功能
    CAN_InitSturcture.CAN_RFLM = DISABLE;   // 禁用接收FIFO锁定模式
    CAN_InitSturcture.CAN_TXFP = DISABLE;   // 禁用发送FIFO优先级
    CAN_InitSturcture.CAN_Mode = mode;      // 设置CAN工作模式
    CAN_InitSturcture.CAN_SJW = tsjw;       // 设置同步跳转宽度
    CAN_InitSturcture.CAN_BS1 = tbs1;       // 设置时间段1
    CAN_InitSturcture.CAN_BS2 = tbs2;       // 设置时间段2
    CAN_InitSturcture.CAN_Prescaler = brp;  // 设置预分频器
    CAN_Init (CAN1, &CAN_InitSturcture);    // 初始化CAN1控制器

    // 设置滤波器编号为0
    CAN_FilterInitSturcture.CAN_FilterNumber = 0;

    // 如果使用标准帧格式
#if (Frame_Format == Standard_Frame)


    /* 标识符/掩码模式，32位滤波器，标准ID: 0x2B0 */
    CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdMask;   // 掩码模式
    CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_32bit;  // 32位滤波
    CAN_FilterInitSturcture.CAN_FilterIdHigh = (0x2A0 << 5);          // 滤波器ID高16位
    CAN_FilterInitSturcture.CAN_FilterIdLow = 0;                      // 滤波器ID低16位
    CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0xFF80;            // 滤波器掩码高16位
    CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0x0000;             // 滤波器掩码低16位

    // 如果不使用软件滤波
#ifndef USE_SOFT_FILTER

#endif


#ifndef USE_SOFT_FILTER

#endif

    // 如果使用扩展帧格式
#elif (Frame_Format == Extended_Frame)
    /* 标识符/掩码模式，32位滤波器，扩展ID: 0x12124567 */
    CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdMask;   // 掩码模式
    CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_32bit;  // 32位滤波
    CAN_FilterInitSturcture.CAN_FilterIdHigh = 0x9092;                // 滤波器ID高16位
    CAN_FilterInitSturcture.CAN_FilterIdLow = 0x2B3C;                 // 滤波器ID低16位
    CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0xFFFF;            // 滤波器掩码高16位
    CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0xFFFE;             // 滤波器掩码低16位

#endif

    // 配置滤波器关联到FIFO0
    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    // 使能滤波器
    CAN_FilterInitSturcture.CAN_FilterActivation = ENABLE;

    // 如果使用中断
#ifdef USE_INTERRUPT
    // 使能CAN1的FIFO0消息挂起中断
    CAN_ITConfig (CAN1, CAN_IT_FMP0, ENABLE);
    // 使能CAN1接收中断
    NVIC_EnableIRQ (USB_LP_CAN1_RX0_IRQn);
#endif

    // 如果使用软件滤波
#ifdef USE_SOFT_FILTER
    // 配置CAN控制器寄存器，启用软件滤波功能
    (*(__IO uint32_t *)(0x40006600)) |= 0x1;
    (*(__IO uint32_t *)(0x4000660C)) |= 0x3;
    (*(__IO uint32_t *)(0x40006640)) = 0;
    (*(__IO uint32_t *)(0x40006644)) = 0;
    (*(__IO uint32_t *)(0x4000661C)) |= 0x3;
    (*(__IO uint32_t *)(0x40006600)) &= ~0x1;
    // 初始化软件滤波器
    CAN_SoftFilterInit (&CAN_FilterInitSturcture);
#else
    // 初始化硬件滤波器
    CAN_FilterInit (&CAN_FilterInitSturcture);
#endif  // USE_SOFT_FILTER
}

/**
 * @brief 发送CAN消息
 * @param msg 指向要发送的数据缓冲区的指针
 * @param len 要发送的数据长度（最大8字节）
 * @return 发送成功返回最后一个发送字节，失败返回对应错误位置字节
 */
u8 CAN_Send_Msg (u8 *msg, u8 len, u16 can_id) {
    u8 mbox;                  // 发送邮箱号
    u16 i = 0;                // 超时计数器

    CanTxMsg CanTxStructure;  // CAN发送消息结构体

    // 如果使用标准帧格式
#if (Frame_Format == Standard_Frame)
    CanTxStructure.StdId = can_id;         // 设置标准ID
    CanTxStructure.IDE = CAN_Id_Standard;  // 标准帧标志

    // 如果使用扩展帧格式
#elif (Frame_Format == Extended_Frame)
    CanTxStructure.ExtId = 0x12124567;     // 设置扩展ID
    CanTxStructure.IDE = CAN_Id_Extended;  // 扩展帧标志

#endif

    CanTxStructure.RTR = CAN_RTR_Data;  // 数据帧（非远程帧）
    CanTxStructure.DLC = len;           // 数据长度

    // 复制要发送的数据到发送结构体
    for (i = 0; i < len; i++) {
        CanTxStructure.Data[i] = msg[i];
    }

    // 发送CAN消息，获取使用的邮箱号
    mbox = CAN_Transmit (CAN1, &CanTxStructure);
    i = 0;

    // 等待发送完成或超时
    while ((CAN_TransmitStatus (CAN1, mbox) != CAN_TxStatus_Ok) && (i < 0xFFF)) {
        i++;
    }

    // 返回发送结果（实际这里逻辑有问题，应返回成功/失败标志）
    if (i == 0xFFF) {
        return 0;  // 超时返回
    } else {
        return 1;  // 成功返回
    }
}

/**
 * @brief 接收CAN消息
 * @param buf 指向接收数据缓冲区的指针
 * @return 接收到的数据长度，0表示没有接收到数据
 */
u8 CAN_Receive_Msg (u8 *buf) {
    u8 i;  // 循环计数器


    // 检查FIFO0中是否有等待处理的消息
    if (CAN_MessagePending (CAN1, CAN_FIFO0) == 0) {
        return 0;  // 没有消息，返回0
    }

    // 如果使用软件滤波
#ifdef USE_SOFT_FILTER
    CAN_ReceiveViaSoftFilter (CAN1, CAN_FIFO0, &CanRxStructure);
#else
    // 直接接收消息
    CAN_Receive (CAN1, CAN_FIFO0, &CanRxStructure);
#endif  // USE_SOFT_FILTER

    // 将接收到的数据复制到输出缓冲区
    for (i = 0; i < 8; i++) {
        buf[i] = CanRxStructure.Data[i];
    }

    // 返回接收到的数据长度
    return CanRxStructure.DLC;
}
