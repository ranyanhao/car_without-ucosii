/**
  ******************************************************************************
  * @file    spi.h
  * @author  YANDLD
  * @version V2.5
  * @date    2014.3.26
  * @brief   www.beyondcore.net   http://upcmcu.taobao.com 
  * @note    此文件为芯片SPI模块的底层功能函数
  ******************************************************************************
  */

#ifndef __CH_LIB_SPI_H__
#define __CH_LIB_SPI_H__

#include <stdint.h>
  
//!< SPI模块设备
#define HW_SPI0     (0x00)
#define HW_SPI1     (0x01)
#define HW_SPI2     (0x02)
  
#define HW_CTAR0    (0x00)  
#define HW_CTAR1    (0x01)  

#define HW_SPI_CS0  (0x00)
#define HW_SPI_CS1  (0x01)
#define HW_SPI_CS2  (0x02)
#define HW_SPI_CS3  (0x03)

//!< SPI帧格式选择
typedef enum
{
    kSPI_CPOL0_CPHA0,
    kSPI_CPOL0_CPHA1,
    kSPI_CPOL1_CPHA0,
    kSPI_CPOL1_CPHA1
}SPI_FrameFormat_Type;

typedef enum
{
    kSPI_Master,
    kSPI_Slave,
} SPI_Mode_Type;


//!< interrupt and DMA select
typedef enum
{
    kSPI_IT_TCF_Disable,    //!< 禁止TCF中断(TCF:传输一次完成)
    kSPI_IT_TCF,            //!< SPI传输一次完成中断使能
}SPI_ITDMAConfig_Type;


#define kSPI_PCS_ReturnInactive   (0x00)      //!< 传输完成后CS信号保持片选中状态
#define kSPI_PCS_KeepAsserted     (0x01)      //!< 传输完成后CS信号保持未选中状态

#define kSPI_MSBFirst             (0x00)      //!< 先发送最高位
#define kSPI_LSBFirst             (0x01)      //!< 先发送最低位

//!< 初始化结构
typedef struct
{
    uint32_t                instance;               //!< 模块号
	SPI_Mode_Type           mode;                   //!< 主从模式
    uint8_t                 dataSize;               //!< 每帧数据有多少位 通常为8或16
    uint8_t                 bitOrder;               //!< 先发高位还是先发地位
    SPI_FrameFormat_Type    frameFormat;            //!< 四种帧格式选择
    uint32_t                baudrate;               //!< 速率
    uint32_t                ctar;
}SPI_InitTypeDef;

//!< 快速初始化结构

#define SPI0_SCK_PC05_SOUT_PC06_SIN_PC07   (0xca90U)
#define SPI0_SCK_PD01_SOUT_PD02_SIN_PD03   (0xc298U)
#define SPI1_SCK_PE02_SOUT_PE01_SIN_PE03   (0xc2a1U)
#define SPI0_SCK_PA15_SOUT_PA16_SIN_PA17   (0xde80U)
#define SPI2_SCK_PB21_SOUT_PB22_SIN_PB23   (0xea8aU)
#define SPI2_SCK_PD12_SOUT_PD13_SIN_PD14   (0xd89aU)

//!< Callback Type
typedef void (*SPI_CallBackType)(void);

//!< API functions
void SPI_Init(SPI_InitTypeDef * SPI_InitStruct);
uint16_t SPI_ReadWriteByte(uint32_t instance,uint32_t ctar, uint16_t data, uint16_t CSn, uint16_t csState);
void SPI_ITDMAConfig(uint32_t instance, SPI_ITDMAConfig_Type config);
void SPI_CallbackInstall(uint32_t instance, SPI_CallBackType AppCBFun);
uint32_t SPI_QuickInit(uint32_t MAP, SPI_FrameFormat_Type frameFormat, uint32_t baudrate);
void SPI_FrameConfig(uint32_t instance, uint32_t ctar, SPI_FrameFormat_Type frameFormat, uint8_t dataSize, uint8_t bitOrder, uint32_t baudrate);


#endif

