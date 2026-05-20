// #include "check.h"
// #include <string.h>


// // ==========================================
// // 实现：8位和校验计算
// // ==========================================
//  uint8_t MW_CalcSum8(uint8_t *data, uint16_t len)
// {
//     uint8_t sum = 0;
//     if(data == NULL || len == 0) return sum;

//     for(uint16_t i = 0; i < len; i++)
//     {
//         sum += data[i]; // 累加所有字节
//     }
//     return sum; // 返回低8位和
// }

// // ==========================================
// // 实现：CRC8 校验计算（标准多项式 0x07）
// // ==========================================
// uint8_t MW_CalcCRC8(uint8_t *data, uint16_t len)
// {
//     if(data == NULL || len == 0) return 0;

//     uint8_t crc = 0;
//     for(uint16_t i = 0; i < len; i++)
//     {
//         crc ^= data[i]; // 初始异或
//         for(uint8_t j = 0; j < 8; j++) // 逐位处理
//         {
//             if(crc & 0x80)
//                 crc = (crc << 1) ^ CRC8_POLY;
//             else
//                 crc <<= 1;
//         }
//     }
//     return crc;
// }

// // // ==========================================
// // // 实现：校验验证（解析帧时调用）
// // // ==========================================
// // uint8_t MW_VerifyCheck(MW_Ctrl_t *p, uint8_t *data, uint16_t len)
// // {
// //     // 入参合法性检查
// //     if(p == NULL || data == NULL || len == 0) return 0;

// //     // 未使能校验，直接通过
// //     if(!p->cfg.check_en) return 1;

// //     uint8_t check_val = 0;
// //     uint16_t check_len = len - 1; // 最后1字节是校验值（8位和/CRC8）
    
// //     // 无有效数据（仅校验字节），校验失败
// //     if(check_len == 0) return 0;

// //     // 根据校验类型计算并验证
// //     switch(p->cfg.check_type)
// //     {
// //         case MW_CHECK_SUM8:
// //             check_val = MW_CalcSum8(data, check_len);
// //             return (check_val == data[check_len]) ? 1 : 0;

// //         case MW_CHECK_CRC8:
// //             check_val = MW_CalcCRC8(data, check_len);
// //             return (check_val == data[check_len]) ? 1 : 0;

// //         // 无校验/不支持的类型，默认通过
// //         case MW_CHECK_NONE:
// //         default:
// //             return 1;
// //     }
// // }
