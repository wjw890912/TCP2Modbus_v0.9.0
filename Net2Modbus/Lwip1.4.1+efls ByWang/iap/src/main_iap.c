/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    07/16/2010 
  * @brief   Main program body
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
#include "stm32_eth.h"
#include "ethernetif.h"
#include "netconf.h"
#include "main_iap.h"
#include "httpserver_iap.h" 
#include "tftpserver_iap.h"
#include "flash_if.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
typedef  void (*pFunction)(void);

/* Private define ------------------------------------------------------------*/
//#define SYSTEMTICK_PERIOD_MS  10

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
//uint32_t timingdelay;
pFunction Jump_To_Application;
uint32_t JumpAddress;

/* Private function prototypes -----------------------------------------------*/
//void System_Periodic_Handle(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */

uint8_t Iap_State =0;//iap	flage ,if it =1 
uint32_t Flash_First_Page_Address =0x08010000;
int IAP_main(void)
{
  
 
  
  /* Test if Key push-button on STM3210C-EVAL Board is not pressed */
  if (/*STM_EVAL_PBGetState(Button_KEY) != 0x00*/0)
  { /* Key push-button not pressed: jump to user application */
  
    /* Check if valid stack address (RAM address) then jump to user application */
   // if (((*(__IO uint32_t*)USER_FLASH_FIRST_PAGE_ADDRESS) & 0x2FFE0000 ) == 0x20000000)
    //{
      /* Jump to user application */
     // JumpAddress = *(__IO uint32_t*) (USER_FLASH_FIRST_PAGE_ADDRESS + 4);
     // Jump_To_Application = (pFunction) JumpAddress;
      /* Initialize user application's Stack Pointer */
     // __set_MSP(*(__IO uint32_t*) USER_FLASH_FIRST_PAGE_ADDRESS);
    //  Jump_To_Application();
   // }
   // else
    //{/* Otherwise, do nothing */
      /* LED3 (RED) ON to indicate bad software (when not valid stack address) */
      ///STM_EVAL_LEDInit(LED3);
      ///STM_EVAL_LEDOn(LED3);
      /* do nothing */
     // while(1);
    //}
  }
  /* enter in IAP mode */
  else
  {
     
#ifdef USE_IAP_TFTP    
    /* Initialize the TFTP server */
    IAP_tftpd_init();
#endif    
	 
	 /*  {
		 static uint32_t temp2=0xa5a58888,temp3=0x8888a5a5;
		 FLASH_If_Init();
		 FLASH_If_Erase(0x0803f800);
		 FLASH_If_DataFlashWrite(0x0803f800, &temp2, 1);
		 FLASH_If_DataFlashRead(0x0803f800, &temp3 ,1);
		

		}  */
  
  }
  	
  return 0;
}

 #if 0

/**
  * @brief  Inserts a delay time.
  * @param  nCount: number of 10ms periods to wait for.
  * @retval None
  */
void Delay(uint32_t nCount)
{
  /* Capture the current local time */
  timingdelay = LocalTime + nCount;  

  /* wait until the desired delay finish */  
  while(timingdelay > LocalTime)
  {     
  }
}



 
/**
  * @brief  Updates the system local time
  * @param  None
  * @retval None
  */
void Time_Update(void)
{
  LocalTime += SYSTEMTICK_PERIOD_MS;
}

/**
  * @brief  Handles the periodic tasks of the system
  * @param  None
  * @retval None
  */
void System_Periodic_Handle(void)
{
#ifdef USE_LCD 
  
  /* Update the LCD display and the LEDs status */
  /* Manage the IP address setting */
  Display_Periodic_Handle(LocalTime);
  
#endif
  
  /* LwIP periodic services are done here */
  LwIP_Periodic_Handle(LocalTime);
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}

#endif

#endif
extern uint32_t flash_net_buf[3];
extern void WriteIpToFlash(uint32_t *ptr,uint8_t len);
 void IAP_EndSetFlash(void)
  {
	    
		 
	/*	 static uint32_t temp;

		 FLASH_If_Init();
		 FLASH_If_DataFlashRead(0x0803f800, &temp ,1);
	 if((temp==0xffffffff)|(temp==0x8000000))//the first write IAP__FLASH_ADDR=0x801ffff
		 {

		    temp = 0x08010000;
			Flash_First_Page_Address=0x08010000;				   // now the applicto is at : 0x8000000
		    FLASH_ErasePage(0x0803f800);
			FLASH_If_DataFlashWrite(0x0803f800, &temp, 1); //write
			NVIC_SetVectorTable(0x8000000, 0x10000); 
		 
		 }
		 else
		 if(temp==0x08010000)
		 {
		 
			temp = 0x08000000;				   // now the applicto is at : 0x801ffff 
			Flash_First_Page_Address=0x08000000;
		    FLASH_ErasePage(0x0803f800);
			FLASH_If_DataFlashWrite(0x0803f800, &temp, 1); //write KEY 
			NVIC_SetVectorTable(0x8000000, 0x00000); 

		 }
		 else{
		 
		 	   FLASH_ErasePage(0x0803f800);
		 }



	 ////关闭全局中断
	__disable_irq();
				
		  */

		 /* Check if valid stack address (RAM address) then jump to user application */
    if (((*(__IO uint32_t*)Flash_First_Page_Address) & 0x2FFE0000 ) == 0x20000000)
    {

			flash_net_buf[2]=0xAAAAAAAA;//app
			WriteIpToFlash(flash_net_buf,3);

      /* Jump to user application */
      JumpAddress = *(__IO uint32_t*) (Flash_First_Page_Address + 4);
      Jump_To_Application = (pFunction) JumpAddress;
      /* Initialize user application's Stack Pointer */
      __set_MSP(*(__IO uint32_t*) Flash_First_Page_Address);
      Jump_To_Application();


	 }

	  //如果上面的代码没有被正确执行，则系统停留在原来的程序区

}

 #if 0
void JumpToAPP(void)

{

   u32 IapSpInitVal;

   u32 IapJumpAddr;

   void (*pIapFun)(void);
	iapFlg = (u32*)0x2000000C;

	 if (*iapFlg == 0xA5A5A5A5) 
	 {
		 *iapFlg = 0;
		 return;
	 }

   Stm32_Clock_DeInit(); //关闭外设
                         //NVIC_SystemReset();
                         //NVIC_DeInit();

   INTX_DISABLE(); //关中断（）如IAP关中断 APP如果没用UCOS系统，APP


   IapSpInitVal = *(u32 *)APP1_ADDR;

   IapJumpAddr = *(u32 *)(APP1_ADDR + 4);

   if ((IapSpInitVal & 0x2FFE0000) == 0x20000000) //检查栈顶地址是否合法.

   {

      MSR_MSP(IapSpInitVal);
      
      pIapFun = (void (*)(void))IapJumpAddr;

      (*pIapFun)();

   }
   else
   {
      INTX_ENABLE();
   }
}

#endif
/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
