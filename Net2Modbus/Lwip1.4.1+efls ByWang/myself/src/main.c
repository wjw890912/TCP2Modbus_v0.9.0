/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
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
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32_eth.h"
#include "netconf.h"
#include "main.h"
#include "filesystem.h"
#include "ds18b20.h"
#include "flash_if.h"  


#ifdef USED_MODBUS
#include "stm32f10x_it.h"
#include "usart.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SYSTEMTICK_PERIOD_MS  10

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
uint32_t timingdelay;

/* Private function prototypes -----------------------------------------------*/
void System_Periodic_Handle(void);
void InitTIM(void);
void TIM2_IRQHandler(void);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */

extern void ModbusSenttset(uint32_t Time);
extern void UDPSendData(uint32_t Time);
extern void SendDataToSever(uint32_t Time);  
extern void tcp_sever_test(void);
extern void FileSystemInit(void);
extern void FileSystemThread(void); 
extern void httpd_init(void);
extern int IAP_main(void);
extern void initsi4432(void);
extern  void  Si4432Thread(uint32_t t);

char poll_net_data_lock=0;

uint32_t flash_net_buf[4];
uint8_t ipaddr_sever[4];
uint16_t port_sever;
uint32_t iap_mode;//是否进入IAP模式。

int main(void)
{
  
		   {		
				  //开机加载FLASH 参数
		    GetIpFromFlash(flash_net_buf);//读取2个字到buf中
            ipaddr_sever[0]=flash_net_buf[0]>>24;//取出最高IP位
			ipaddr_sever[1]=flash_net_buf[0]>>16;
			ipaddr_sever[2]=flash_net_buf[0]>>8;
			ipaddr_sever[3]=flash_net_buf[0]>>0;//取出最低IP位
			port_sever     =flash_net_buf[1];
			iap_mode	   =flash_net_buf[2];//取出来IAP mode
		
		   }


		 #ifdef USE_IAP_TFTP
/*
		 
		iap_mode==0xAAAAAAAA 或者 iap_mode==0xFFFFFFFF 
		这个表示上电后需要跳转到APP程序里运行，默认是跳转到APP区的，如果跳转失败则会留在IAP区
		
		iap_mode==0x55555555		
		这个表示上电后留在IAP区，等待升级程序
		 
*/
		if((iap_mode==0xAAAAAAAA))
		{
		  // 跳转到APP程序中运行，启动APP程序
		 IAP_EndSetFlash();
		 //如果正确的执行这里永远不会到来，上面就去了应用程序中运行，但是当返回了
		 //表示APP应用程序区还是空的，或者上次烧写不正确。这时候程序留在IAP区中，等待升级。
		  
		}
		else
		if((iap_mode==0x55555555)|(iap_mode==0xFFFFFFFF))
		{
			//留在IAP程序启动 等待升级程序
		
		}

		#endif
	 /* Setup STM32 system (clocks, Ethernet, GPIO, NVIC) */
		  System_Setup();
		 

		  #ifdef USED_SI4432
		   initsi4432();
		   rx_data();
		  #endif
		  	 
	      InitTIM();//systick 服务 由TIM代替 systick 专用于DS18B20的US延时服务
			 
	      #ifdef USED_FILESYSTEM
		   FileSystemInit(); 
	       FileSystemThread();    
		  #endif 
		     
		  /* Initilaize the LwIP satck */
		  LwIP_Init();

		 // netbios_init();
		 
		  //tftpd_init();

		  //tcp_sever_test();

		  #ifdef USED_FILESYSTEM && USED_HTTP
		  /*Infinite loop */
		  httpd_init();
		  #endif

		  #ifdef USED_SMTP
		  my_smtp_test("The Board is Power up...");
		  #endif

		 
		  #ifdef USED_WATCHDOG
		  WatchDogInit();
          #endif

		   #ifdef USE_IAP_TFTP
		  IAP_main();
		  #endif


		  while (1)
		  {   

		   	  #ifdef USED_DS18B20
			  TemperatureThread(LocalTime);
			  #endif
			  TcpTestThread(LocalTime);
		      SendDataToSever(LocalTime);
			  #ifdef UDPBOARDCAST_ENABLE 
		      UDPSendData(LocalTime);
			  #endif
		      LwIP_Periodic_Handle(LocalTime);
			  #ifdef USED_MODBUS
			  ModbusSenttset(LocalTime); //检查是否有TCP接受到数据，如果有发送 PASS RTU
			 (void)eMBMasterPoll();//空闲检查 modbus RTU 数据
			  #endif
			  #ifdef USED_WATCHDOG
              KeepWatchDogLive();
              #endif
			  #ifdef USED_SI4432
			  Si4432Thread(LocalTime);
			  #endif
		  }
}

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
 // LocalTime += SYSTEMTICK_PERIOD_MS
}

 /*
		Systick 用于DS18B20的精确延时上。
		TIM2用于本地的时间服务
		20151020

  */

 extern void SynruntimHook(uint32_t Time);
void TIM2_IRQHandler(void)
{
	
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		
		TIM_ClearFlag(TIM2, TIM_FLAG_Update);	     //清中断标记
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	 //清除定时器TIM2溢出中断标志位

		LocalTime += SYSTEMTICK_PERIOD_MS;

		SynruntimHook(LocalTime);//同步时间
  	
	}
	
}	


void InitTIM(void)
{	
    uint16_t 	usPrescalerValue;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	//====================================时钟初始化===========================
	//使能定时器2时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	//====================================定时器初始化===========================
	//定时器时间基配置说明
	//HCLK为72MHz，APB1经过2分频为36MHz
	//TIM2的时钟倍频后为72MHz（硬件自动倍频,达到最大）
	//TIM2的分频系数为3599，时间基频率为72 / (1 + Prescaler) = 20KHz,基准为50us
	//TIM最大计数值为usTim1Timerout50u	
	usPrescalerValue = (uint16_t) (72000000 / 20000) - 1;
	//预装载使能
	TIM_ARRPreloadConfig(TIM2, ENABLE);
	//====================================中断初始化===========================
	//设置NVIC优先级分组为Group2：0-3抢占式优先级，0-3的响应式优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//清除溢出中断标志位
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	//定时器2溢出中断关闭
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
	//定时器3禁能
	TIM_Cmd(TIM2, DISABLE);

	TIM_TimeBaseStructure.TIM_Prescaler = usPrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = (uint16_t)(10 * 1000 / 50);//10ms
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	   
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM2, 0);
	TIM_Cmd(TIM2, ENABLE);	


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


#ifdef USED_WATCHDOG
void WatchDogInit(void)
{
 
	/*	
	
		Min/max IWDG timeout period (in ms) at 40 kHz (LSI)
				Min timeout RL[11:0]= 0x00   Max timeout RL[11:0]= 0xFF
IWDG_Prescaler_4:             min 0.1ms  ~       max 409.6ms
IWDG_Prescaler_8:             min 0.2ms  ~       max 819.2ms 
IWDG_Prescaler_16:            min 0.4ms  ~       max 1638.4ms 
IWDG_Prescaler_32:            min 0.8ms  ~       max 3276.8ms 
IWDG_Prescaler_64:            min 1.6ms  ~       max 6553.6ms 
IWDG_Prescaler_128:           min 3.2ms  ~       max 13107.2ms 
IWDG_Prescaler_256:           min 6.4ms  ~       max 26214.4ms  
	*/
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_32); //设置超时时间为409.6ms
	IWDG_SetReload(0xFFF);
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
	IWDG_Enable();
}
 
 void KeepWatchDogLive(void)
 {

 	IWDG_ReloadCounter();
 	
 }

#endif





/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
