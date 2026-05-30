/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2024/05/22
* Description        : SPI操作W25Qxx Flash（引脚宏定义，方便修改）
*******************************************************************************/
#include "stdio.h"
#include "ch32v30x.h"
#include "bsp_spi.h"

/************************** 引脚宏定义 (核心：仅修改此处即可换引脚) **************************/
#define FLASH_CS_PORT            GPIOA
#define FLASH_CS_PIN             GPIO_Pin_4      // CS片选引脚 (原PA2，现改为PA4)
#define FLASH_CS_CLK             RCC_APB2Periph_GPIOA

#define SPI_SCK_PORT             GPIOA
#define SPI_SCK_PIN              GPIO_Pin_5      // SPI时钟
#define SPI_MISO_PORT            GPIOA
#define SPI_MISO_PIN             GPIO_Pin_6      // SPI主入从出
#define SPI_MOSI_PORT            GPIOA
#define SPI_MOSI_PIN             GPIO_Pin_7      // SPI主出从入
/******************************************************************************************/

/* Winbond SPIFalsh ID */
#define W25Q80                   0XEF13
#define W25Q16                   0XEF14
#define W25Q32                   0XEF15
#define W25Q64                   0XEF16
#define W25Q128                  0XEF17

/* Winbond SPIFalsh Instruction List */
#define W25X_WriteEnable         0x06
#define W25X_WriteDisable        0x04
#define W25X_ReadStatusReg       0x05
#define W25X_WriteStatusReg      0x01
#define W25X_ReadData            0x03
#define W25X_PageProgram         0x02
#define W25X_SectorErase         0x20
#define W25X_ChipErase           0xC7
#define W25X_ManufactDeviceID    0x90

/* Global Variable */
u8       SPI_FLASH_BUF[4096];
const u8 TEXT_Buf[] = {"CH32F103 SPI FLASH W25Qxx"};
#define SIZE    sizeof(TEXT_Buf)
volatile u8       datap[SIZE];  // 调试查看Flash读取数据的变量
volatile u16 Flash_Model; 
/*********************************************************************
 * @fn      SPI1_ReadWriteByte
 * @brief   SPI1单字节读写
 *****************************************************************************/
u8 SPI1_ReadWriteByte(u8 TxData)
{
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, TxData);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI1);
}

/*********************************************************************
 * @fn      SPI_Flash_Init
 * @brief   SPI+FLASH引脚初始化 (使用宏定义)
 *****************************************************************************/
void SPI_Flash_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    SPI_InitTypeDef  SPI_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);

    // FLASH CS 片选引脚初始化
    GPIO_InitStructure.GPIO_Pin = FLASH_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(FLASH_CS_PORT, &GPIO_InitStructure);
    GPIO_SetBits(FLASH_CS_PORT, FLASH_CS_PIN);  // 默认拉高CS

    // SCK 推挽复用
    GPIO_InitStructure.GPIO_Pin = SPI_SCK_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(SPI_SCK_PORT, &GPIO_InitStructure);

    // MISO 浮空输入
    GPIO_InitStructure.GPIO_Pin = SPI_MISO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(SPI_MISO_PORT, &GPIO_InitStructure);

    // MOSI 推挽复用
    GPIO_InitStructure.GPIO_Pin = SPI_MOSI_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(SPI_MOSI_PORT, &GPIO_InitStructure);

    // SPI配置
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);
}

/*********************************************************************
 * @fn      SPI_Flash_ReadSR
 * @brief   读取状态寄存器
 *****************************************************************************/
u8 SPI_Flash_ReadSR(void)
{
    u8 byte = 0;
    GPIO_WriteBit(FLASH_CS_PORT, FLASH_CS_PIN, 0);
    SPI1_ReadWriteByte(W25X_ReadStatusReg);
    byte = SPI1_ReadWriteByte(0Xff);
    GPIO_WriteBit(FLASH_CS_PORT, FLASH_CS_PIN, 1);
    return byte;
}

/*********************************************************************
 * @fn      SPI_Flash_Wait_Busy
 * @brief   等待Flash空闲
 *****************************************************************************/
void SPI_Flash_Wait_Busy(void)
{
    while((SPI_Flash_ReadSR() & 0x01) == 0x01);
}

/*********************************************************************
 * @fn      SPI_FLASH_Write_Enable
 * @brief   写使能
 *****************************************************************************/
void SPI_FLASH_Write_Enable(void)
{
    GPIO_WriteBit(FLASH_CS_PORT, FLASH_CS_PIN, 0);
    SPI1_ReadWriteByte(W25X_WriteEnable);
    GPIO_WriteBit(FLASH_CS_PORT, FLASH_CS_PIN, 1);
}

/*********************************************************************
 * @fn      SPI_Flash_ReadID
 * @brief   读取Flash芯片ID
 *****************************************************************************/
u16 SPI_Flash_ReadID(void)
{
    u16 Temp = 0;
    GPIO_WriteBit(FLASH_CS_PORT, FLASH_CS_PIN, 0);
    SPI1_ReadWriteByte(W25X_ManufactDeviceID);
    SPI1_ReadWriteByte(0x00);
    SPI1_ReadWriteByte(0x00);
    SPI1_ReadWriteByte(0x00);
    Temp |= SPI1_ReadWriteByte(0xFF) << 8;
    Temp |= SPI1_ReadWriteByte(0xFF);
    GPIO_WriteBit(FLASH_CS_PORT, FLASH_CS_PIN, 1);
    return Temp;
}

/*********************************************************************
 * @fn      SPI_Flash_Erase_Sector
 * @brief   扇区擦除(4KB)
 *****************************************************************************/
void SPI_Flash_Erase_Sector(u32 Dst_Addr)
{
    Dst_Addr *= 4096;
    SPI_FLASH_Write_Enable();
    SPI_Flash_Wait_Busy();
    GPIO_WriteBit(FLASH_CS_PORT, FLASH_CS_PIN, 0);
    SPI1_ReadWriteByte(W25X_SectorErase);
    SPI1_ReadWriteByte((u8)((Dst_Addr) >> 16));
    SPI1_ReadWriteByte((u8)((Dst_Addr) >> 8));
    SPI1_ReadWriteByte((u8)Dst_Addr);
    GPIO_WriteBit(FLASH_CS_PORT, FLASH_CS_PIN, 1);
    SPI_Flash_Wait_Busy();
}

/*********************************************************************
 * @fn      SPI_Flash_Read
 * @brief   读取Flash数据
 *****************************************************************************/
void SPI_Flash_Read(u8 *pBuffer, u32 ReadAddr, u16 size)
{
    u16 i;
    GPIO_WriteBit(FLASH_CS_PORT, FLASH_CS_PIN, 0);
    SPI1_ReadWriteByte(W25X_ReadData);
    SPI1_ReadWriteByte((u8)((ReadAddr) >> 16));
    SPI1_ReadWriteByte((u8)((ReadAddr) >> 8));
    SPI1_ReadWriteByte((u8)ReadAddr);

    for(i = 0; i < size; i++)
    {
        pBuffer[i] = SPI1_ReadWriteByte(0XFF);
    }
    GPIO_WriteBit(FLASH_CS_PORT, FLASH_CS_PIN, 1);
}

/*********************************************************************
 * @fn      SPI_Flash_Write_Page
 * @brief   按页写入
 *****************************************************************************/
void SPI_Flash_Write_Page(u8 *pBuffer, u32 WriteAddr, u16 size)
{
    u16 i;
    SPI_FLASH_Write_Enable();
    GPIO_WriteBit(FLASH_CS_PORT, FLASH_CS_PIN, 0);
    SPI1_ReadWriteByte(W25X_PageProgram);
    SPI1_ReadWriteByte((u8)((WriteAddr) >> 16));
    SPI1_ReadWriteByte((u8)((WriteAddr) >> 8));
    SPI1_ReadWriteByte((u8)WriteAddr);

    for(i = 0; i < size; i++)
    {
        SPI1_ReadWriteByte(pBuffer[i]);
    }
    GPIO_WriteBit(FLASH_CS_PORT, FLASH_CS_PIN, 1);
    SPI_Flash_Wait_Busy();
}

/*********************************************************************
 * @fn      SPI_Flash_Write_NoCheck
 * @brief   无校验写入
 *****************************************************************************/
void SPI_Flash_Write_NoCheck(u8 *pBuffer, u32 WriteAddr, u16 size)
{
    u16 pageremain;
    pageremain = 256 - WriteAddr % 256;
    if(size <= pageremain) pageremain = size;

    while(1)
    {
        SPI_Flash_Write_Page(pBuffer, WriteAddr, pageremain);
        if(size == pageremain) break;
        
        pBuffer += pageremain;
        WriteAddr += pageremain;
        size -= pageremain;
        pageremain = (size > 256) ? 256 : size;
    }
}

/*********************************************************************
 * @fn      SPI_Flash_Write
 * @brief   Flash写入（自动擦除）
 *****************************************************************************/
void SPI_Flash_Write(u8 *pBuffer, u32 WriteAddr, u16 size)
{
    u32 secpos;
    u16 secoff, secremain, i;

    secpos = WriteAddr / 4096;
    secoff = WriteAddr % 4096;
    secremain = 4096 - secoff;
    if(size <= secremain) secremain = size;

    while(1)
    {
        SPI_Flash_Read(SPI_FLASH_BUF, secpos * 4096, 4096);
        for(i = 0; i < secremain; i++)
        {
            if(SPI_FLASH_BUF[secoff + i] != 0XFF) break;
        }

        if(i < secremain)
        {
            SPI_Flash_Erase_Sector(secpos);
            for(i = 0; i < secremain; i++)
            {
                SPI_FLASH_BUF[i + secoff] = pBuffer[i];
            }
            SPI_Flash_Write_NoCheck(SPI_FLASH_BUF, secpos * 4096, 4096);
        }
        else
        {
            SPI_Flash_Write_NoCheck(pBuffer, WriteAddr, secremain);
        }

        if(size == secremain) break;
        
        secpos++;
        secoff = 0;
        pBuffer += secremain;
        WriteAddr += secremain;
        size -= secremain;
        secremain = (size > 4096) ? 4096 : size;
    }
}




