/**
  ******************************************************************************
  * @file    flash_if.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    07/16/2010 
  * @brief   This file provides high level routines to manage internal Flash 
  *          programming (erase and write). 
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

/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"
#include "main.h"
#include <stdio.h>
#include <string.h>


#define IP_ADDR_FLASH (0x0803f800)	//IP save flash position
#define IP_PORT_FLASH (0x0803f800+4)	 //port save flash position
#define IAP_MODE_FLAGE_FLASH (0x0803f800+8)	 //IAP Mode flage save flash 	position
#define RF_CHANNEL_FLAGE_FLASH (0x0803f800+12)	 //rf channel flage save flash 	position
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Unlocks Flash for write access
  * @param  None
  * @retval None
  */
void FLASH_If_Init(void)
{ 
__disable_irq();
  FLASH_Unlock(); 
 __enable_irq();
}

/**
  * @brief  This function does an erase of all user flash area
  * @param  StartSector: start of user flash area
  * @retval 0 if success, -1 if error
  */
int8_t FLASH_If_Erase(uint32_t StartSector)
{
  uint32_t FlashAddress;
   __disable_irq();
  FlashAddress = StartSector;
  
  while (FlashAddress <= (uint32_t) USER_FLASH_LAST_PAGE_ADDRESS)
  {
    if (FLASH_ErasePage(FlashAddress) == FLASH_COMPLETE)
    {
      FlashAddress += FLASH_PAGE_SIZE;
    }
    else
    {

	 __enable_irq();
      return -1;
    }
  }

   __enable_irq();
  return 0;


}

/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned)
  * @param  FlashAddress: start address for writing data buffer
  * @param  Data: pointer on data buffer
  * @param  DataLength: length of data buffer (unit is 32-bit word)   
  * @retval None
  */
void FLASH_If_Write(__IO uint32_t* FlashAddress, uint32_t* Data ,uint16_t DataLength)
{
  uint32_t i = 0;
  	  __disable_irq();
  for (i = 0; i < DataLength; i++)
  {
    if (*FlashAddress <= (USER_FLASH_END_ADDRESS-4))
    {
      if (FLASH_ProgramWord(*FlashAddress, *(uint32_t*)(Data + i)) == FLASH_COMPLETE)
      {
        *FlashAddress += 4;
      }
      else
      {

	   __enable_irq();
        return;
      }
    }
    else
    {
	 __enable_irq();
      return;
    }
	
  }
   __enable_irq();
}





/**
  * @brief  This function read a data buffer in flash (data are 32-bit aligned)
  * @param  FlashAddress: start address for read data buffer
  * @param  Data: pointer on data buffer
  * @param  DataLength: length of data buffer (unit is 32-bit word)   
  * @retval None
  */
void FLASH_If_DataFlashRead(__IO uint32_t FlashAddress, uint32_t* Data ,uint16_t DataLength)
{
  uint32_t i = 0;
  	    __disable_irq();
  for (i = 0; i < DataLength; i++)
  {
    if (FlashAddress <= (USER_FLASH_END_ADDRESS-4))
    {
     // if (FLASH_ProgramWord(*FlashAddress, *(uint32_t*)(Data + i)) == FLASH_COMPLETE)
      //{
	   *Data = *((__IO uint32_t*)FlashAddress+(i*4));
        //FlashAddress += 4;
     // }
      //else
     // {
      //  return;
     // }
    }
   // else
   // {
    //  return;
   // }
  }

   __enable_irq();
}



//cereat new some function as dataflash area from applicton main flash that them is in the end 


void FLASH_If_DataFlashWrite(uint32_t FlashAddress, uint32_t* Data ,uint16_t DataLength)
{
  uint32_t i = 0;
  	  
 	  __disable_irq();
  for (i = 0; i < DataLength; i++)
  {
    if (FlashAddress <= ((uint32_t )USER_FLASH_END_ADDRESS-4))
    {
      if (FLASH_ProgramWord(FlashAddress, *(uint32_t*)(Data + i)) == FLASH_COMPLETE)
      {
        FlashAddress += 4;
      }
      else
      {
	   __enable_irq();
        return;
      }
    }
    else
    {
	 __enable_irq();
      return;
    }
  }
}
 /*
 获取FLASH 中的IP地址，如果没有，则使用目前默认的地址
 
 */


 void GetIpFromFlash(uint32_t *ptr)
 {

          uint32_t mid[4];
 			//关闭全局中断
     
		 FLASH_If_Init();
		 FLASH_If_DataFlashRead(IP_ADDR_FLASH, &mid[0] ,1); //ip
		 FLASH_If_DataFlashRead(IP_PORT_FLASH, &mid[1] ,1); //port
		 FLASH_If_DataFlashRead(IAP_MODE_FLAGE_FLASH, &mid[2] ,1); //iap mode
		 FLASH_If_DataFlashRead(RF_CHANNEL_FLAGE_FLASH, &mid[3] ,1); //rf channel

		 if((mid[0]==0xffffffff)|(mid[1]==0xffffffff)|(mid[3]==0xffffffff))	//if IP == empt then write define IP
			 {
				//123.59.96.233 ->hex :0x7B 0x3B 0x60 0xE9
				//:5679			 ->hex :0X162F
			    mid[0] = 0x7B3B60E9;
				mid[1] = 0x162F;
				mid[3] = 0;	 //通道号
			    FLASH_ErasePage(IP_ADDR_FLASH);
				FLASH_If_DataFlashWrite(IP_ADDR_FLASH, &mid[0], 4); //write 4WORD 


			 }
		   


 			   memcpy((uint8_t*)ptr,(uint8_t*)mid,16);


	  	//开启全局中断

 }
 /*
 write IP to flash 
 */

 void WriteIpToFlash(uint32_t *ptr,uint8_t len)
 {
          uint32_t temp[4];
			//关闭全局中断
	__disable_irq();
	
 		  FLASH_If_Init();
		  memcpy((uint8_t*)temp,ptr,16);//数据复制
		  FLASH_ErasePage(IP_ADDR_FLASH);
		  FLASH_If_DataFlashWrite(IP_ADDR_FLASH, &temp[0], 4); //write 4WORD 

		  	//开启全局中断
	__enable_irq();
 }




/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
