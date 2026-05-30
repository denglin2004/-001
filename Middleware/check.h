// #ifndef __CHECK_H
// #define __CHECK_H

// #ifdef __cplusplus
// extern "C" {
// #endif

// #include <stdint.h>


// typedef struct _MW_Ctrl_t MW_Ctrl_t;

// typedef enum
// {
//     MW_CHECK_NONE  = 0,  // 无校验
//     MW_CHECK_SUM8  = 1,  // 8位和校验
//     MW_CHECK_CRC8  = 2   // CRC8校验（标准多项式 0x07）
// } MW_CheckType_t;

// typedef struct
// {
//     uint8_t check_en   : 1;  // 校验使能位（0=关闭，1=开启）
//     uint8_t check_type : 3;  // 校验类型（MW_CheckType_t）
//     uint8_t reserve    : 4;  // 预留位
// } MW_CheckCfg_t;

// #define CRC8_POLY    0x07  // CRC8 标准多项式（x^8+x^2+x+1）
// uint8_t MW_CalcSum8(uint8_t *data, uint16_t len);
// uint8_t MW_CalcCRC8(uint8_t *data, uint16_t len);
// uint8_t MW_VerifyCheck(MW_Ctrl_t *p, uint8_t *data, uint16_t len);

// #ifdef __cplusplus
// }
// #endif

// #endif 