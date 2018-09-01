/**
  ******************************************************************************
  * @file    sd_stm32.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20151009
  * @brief   efs sd card driver's low level interface for STM32F107 
  ******************************************************************************
**/
#include "stm32f10x_spi.h"
#include "interface/sd.h"
#include "config.h"
#include "main.h"
 #ifdef MYSELFBOARD
/* Select MSD Card: ChipSelect pin low  */
#define MSD_CS_LOW()     GPIO_ResetBits(GPIOA, GPIO_Pin_8)
/* Deselect MSD Card: ChipSelect pin high */
#define MSD_CS_HIGH()    GPIO_SetBits(GPIOA, GPIO_Pin_8)
	//	PA8:is a SD_CS
 #else
/* Select MSD Card: ChipSelect pin low  */
#define MSD_CS_LOW()     GPIO_ResetBits(GPIOA, GPIO_Pin_4)
/* Deselect MSD Card: ChipSelect pin high */
#define MSD_CS_HIGH()    GPIO_SetBits(GPIOA, GPIO_Pin_4)
	//	PA4:is a SD_CS	on my braod	!!!
#endif
static void SPI_Config(void);

void SPI_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  SPI_InitTypeDef   SPI_InitStructure;

  /* GPIOA and GPIOC Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | /*RCC_APB2Periph_GPIOC | */RCC_APB2Periph_AFIO, ENABLE);

 // GPIO_PinRemapConfig(GPIO_Remap_SPI3, ENABLE);

  /* SPI1 Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  /* Configure SPI1 pins: SCK, MISO and MOSI */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);



   #ifdef MYSELFBOARD
  /* Configure PA8 pin: CS pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  #else
    /* Configure PA8 pin: CS pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
   #endif


  /* SPI1 Config */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure);

  /* SPI1 enable */
  SPI_Cmd(SPI1, ENABLE);

  ///SD  卡使用 spi 1操作
}

/*****************************************************************************/

esint8 if_readBuf(hwInterface* file, euint32 address, euint8* buf)
{
  return(sd_readSector(file, address, buf, 512));
}
/*****************************************************************************/

esint8 if_writeBuf(hwInterface* file, euint32 address, euint8* buf)
{
  return(sd_writeSector(file, address, buf));
}
/*****************************************************************************/

esint8 if_setPos(hwInterface* file, euint32 address)
{
  return(0);
}
/*****************************************************************************/

// Utility-functions which does not toogle CS.
// Only needed during card-init. During init
// the automatic chip-select is disabled for SSP
	  //修改为SPI1
static euint8 my_if_spiSend(hwInterface *iface, euint8 outgoing)
{
  euint8 incoming;

  SPI_I2S_SendData(SPI1, outgoing);
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  //while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)==SET);
  incoming = SPI_I2S_ReceiveData(SPI1);

  return(incoming);

}
/*****************************************************************************/

void if_spiInit(hwInterface *iface)
{
  euint8 i;
   //硬件SPI初始化
  SPI_Config();

  MSD_CS_HIGH();

  /* Send 20 spi commands with card not selected */
  for (i = 0;i < 21;i++)
    my_if_spiSend(iface, 0xff);
}
/*****************************************************************************/
		   //修改为SPI1
euint8 if_spiSend(hwInterface *iface, euint8 outgoing)
{
  euint8 incoming;

  MSD_CS_LOW();

  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

  SPI_I2S_SendData(SPI1, outgoing);

 //while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
//while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)==SET);
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  incoming = SPI_I2S_ReceiveData(SPI1);

  MSD_CS_HIGH();

  return(incoming);
}
/*****************************************************************************/

esint8 if_initInterface(hwInterface* file, eint8* opts)
{
  euint32 sc;
  //初始化硬件SPI 并且唤醒SD卡
  if_spiInit(file); /* init at low speed */
	//初始化SD卡并判断是1.X还是2.X打上标记。
  if (sd_Init(file) < 0)
  {
    DBG((TXT("Card failed to init, breaking up...\n")));
    return(-1);
  }
  //判断SD卡状态无实际意义
  if (sd_State(file) < 0)
  {
    DBG((TXT("Card didn't return the ready state, breaking up...\n")));
    return(-2);
  }
	
  sd_getDriveSize(file, &sc);//取得SD卡的扇区总字节数，
  file->sectorCount = sc / 512;	 //计算扇区总数，只要把字节总数除以512就好
  if ( (sc % 512) != 0)
  {
    file->sectorCount--;  //对不非整数个的扇区处理舍去一个凑够整数
  }

  DBG((TXT("Drive Size is %lu Bytes (%lu Sectors)\n"), sc, file->sectorCount));
  DBG((TXT("Init done...\n")));

  return(0);
}


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
