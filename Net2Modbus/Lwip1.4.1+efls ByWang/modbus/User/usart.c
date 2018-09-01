#include "stm32f10x_it.h"
#include "usart.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"


/* USART1_REMAP = 0 */
#define UART1_GPIO_TX		GPIO_Pin_9
#define UART1_GPIO_RX		GPIO_Pin_10
#define UART1_GPIO			GPIOA
 /*
 		init USART1
 
 */
 void Hw_Usart_init(void)
 {
		 GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	#if 0
	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	

	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

		    //RCC
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOD, ENABLE);

		    //GPIO
	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = UART1_GPIO_RX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(UART1_GPIO, &GPIO_InitStructure);
	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = UART1_GPIO_TX;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(UART1_GPIO, &GPIO_InitStructure);

		   //NVIC
		/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

		  //USART Config
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_Init(USART1, &USART_InitStructure);
	USART_ClockInit(USART1, &USART_ClockInitStructure);

	/* enable interrupt */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

   USART_Cmd(USART1, ENABLE);

   	
	UART1_SendStr("Start System.....\r\n",sizeof("Start System.....\r\n")); 
	UART1_SendStr("Welcome >>>>>>>>>\r\n",sizeof("Welcome >>>>>>>>>\r\n")); 
	UART1_SendStr("Over <<<<<<<<<<<<\r\n",sizeof("Over <<<<<<<<<<<<\r\n")); 
	  
 
   	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

		#endif
	
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//======================时钟初始化=======================================
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	//======================IO初始化=======================================	
	//USART2_TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	//USART2_RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	//配置485发送和接收模式
//    TODO   暂时先写B13 等之后组网测试时再修改
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//======================串口初始化=======================================
	USART_InitStructure.USART_BaudRate = 9600;
	//设置校验模式
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE);

	//=====================中断初始化======================================
	//设置NVIC优先级分组为Group2：0-3抢占式优先级，0-3的响应式优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

		   while(1)
		   {


			 	USART_SendData(USART2, 0xaa);

		   
		   }

 }


 /*
 
 	 UART1_SendStr
 */
 void UART1_SendStr(char *buff,uint16_t Len)
{
   uint16_t i;
   for(i=0;i<Len;i++)
   {
      USART_SendData(USART1,*buff++);
     while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
   }
}
