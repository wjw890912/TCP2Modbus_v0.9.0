/**
  ******************************************************************************
  * @file    flash_if.h 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    07/16/2010 
  * @brief   Header for flash_if.c module
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_IF_H
#define __FLASH_IF_H
#include "main.h"

#define USER_FLASH_LAST_PAGE_ADDRESS  0x0803F000/*0x0803F800*/
#define USER_FLASH_END_ADDRESS        0x0803FFFF  
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define USER_FLASH_SIZE   (USER_FLASH_END_ADDRESS - USER_FLASH_FIRST_PAGE_ADDRESS)
#define FLASH_PAGE_SIZE   0x800

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void FLASH_If_Write(__IO uint32_t* Address, uint32_t* Data, uint16_t DataLength);
int8_t FLASH_If_Erase(uint32_t StartSector);
void FLASH_If_Init(void);
void FLASH_If_DataFlashRead(__IO uint32_t FlashAddress, uint32_t* Data ,uint16_t DataLength);
void FLASH_If_DataFlashWrite(uint32_t FlashAddress, uint32_t* Data ,uint16_t DataLength);
void GetIpFromFlash(uint32_t *ptr);
void WriteIpToFlash(uint32_t *ptr,uint8_t len);
#endif /* __FLASH_IF_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
