/**
  ******************************************************************************
  * @file    main.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    07/16/2010 
  * @brief   This file contains all the functions prototypes for the main.c 
  *          file.
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
#ifndef __MAIN_IAP_H
#define __MAIN_IAP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f107.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/


//#define USE_IAP_HTTP   /* enable IAP using HTTP */
//#define USE_LCD        /* enable LCD  */  

/* Flash Addresses*/   
/* !!! Be sure that USER_FLASH_FIRST_PAGE_ADDRESS do not overlap with IAP code
* When TFTP,HTTP,LCD and DHCP are used, depending on the used compiler
* the following values can be used as start address for the user application code 
*/ 

#if defined ( __CC_ARM   )
        #define USER_FLASH_FIRST_PAGE_ADDRESS  0x08010000/*0x08009000*/
#elif defined (__ICCARM__)
        #define USER_FLASH_FIRST_PAGE_ADDRESS 0x0800A000
#elif defined   (  __GNUC__  )						    
        #define USER_FLASH_FIRST_PAGE_ADDRESS 0x08010000  
#elif defined   (  __TASKING__  )
        #define USER_FLASH_FIRST_PAGE_ADDRESS 0x0800E000
#endif


#define USER_FLASH_LAST_PAGE_ADDRESS  0x0803F000/*0x0803F800*/
#define USER_FLASH_END_ADDRESS        0x0803FFFF  
   


/*UserID and Password definition*/
#define USERID  "admin"
#define PASSWORD "admin"
#define LOGIN_SIZE   18 + sizeof(USERID) + sizeof(PASSWORD)
    
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */  
void Time_Update(void);
void Delay(uint32_t nCount);
 void IAP_EndSetFlash(void);
 void IAP_tftpd_init(void);
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */


/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/

