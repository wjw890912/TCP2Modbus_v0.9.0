// File Name:SI4432.C
// Author:小ARM菜菜
// Date: 2012年
 //Q Q:925295580

#include <stdio.h>
#include"SI4432.H"
#include "stm32f10x.h"
#include "main.h"
#include <string.h>
#include "lwip/opt.h"
#include "lwip/tcp_impl.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include "filesystem.h"
#include "stm32f10x.h"
#include "flash_if.h"
#ifdef USED_SI4432
extern  uint8_t tx_timeout_en,tx_timeout;
 uint8_t RSSI;
unsigned char RE_RSSI(void)
{
	  
	RSSI = spi_rw(0x26, 0x00);
	return RSSI;


}


 signed char  save_rec_data(unsigned char *recbuf_pt)
{
			 uint8_t leg,i;

	

		 if(!GET_NIRQ)
			{
			  i = RE_RSSI();
             clr_interruput_si4432();
		        leg =spi_rw(0x4b,0);              
				SCK_DOWN;
				nSEL_DOWN; 
				spi_byte(0x7f);		
				for(i=0;i<leg;i++)	
				{
					*(recbuf_pt+i) = spi_byte(0x00);
				}
				nSEL_UP;
				spi_rw(0x07|0x80, SI4432_PWRSTATE_READY);	
				  
			   spi_rw(0x03,0x00);; //read the Interrupt Status1 register
               spi_rw(0x04,0x00);; //read the Interrupt Status2 register
			 rx_data();	
			
		      return leg; 
			  }
			
		  	  return 0;
}
 
void clr_interruput_si4432(void)
{

 spi_rw(0x03,0x00);	
 spi_rw(0x04,0x00);	

}


 //初始化延迟函数
extern void Delay_init(uint8_t SYSCLK);
extern void Delay_us(uint32_t Nus);
extern void Delay_ms(uint16_t nms);


void DrvSYS_Delay(int t)
{

    Delay_us(t);

}


uint8_t RX_Buf[64];
uint8_t RxSi4432Len=0;
extern void Si4432SendDataToSever(uint32_t Time);
void  Si4432Thread(uint32_t t)
{
	   
	if( Check_si4432_HW()==1)	
	{
	
	RxSi4432Len= save_rec_data(RX_Buf);
		if(RxSi4432Len>0)
		{
	     Si4432SendDataToSever(0);
		}						  		  		 
	 }




}
void initsi4432(void)
{
	   int k;
	 

   GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC , ENABLE);
  /* Configure PA4-NSEL PA5-SCLK PA7-MOSI pins */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_7 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
   /* Configure PA6-MISO pins */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* Configure PC4-NIRQ pins */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	/* Configure PC5-SDN pins */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

	 Delay_init(72);

      DrvSYS_Delay(10000);
	  SDN_DOWN;
 	  DrvSYS_Delay(200000);

	   k=spi_rw(0x36, 0);

	  SI4432_init();  
  	  TX0_RX0;
	  k=100+1 ;
	
  
}
 

void rx_data(void)
{	
	  uint8_t k;
	 	  k=k;

		k=spi_rw(0x02, 0);

	spi_rw(0x07|0x80, SI4432_PWRSTATE_READY);
	k=spi_rw(0x07, 0);
	k=spi_rw(0x02, 0);
 	TX0_RX1;	

	spi_rw(0x08|0x80, 0x03);  
	spi_rw(0x08|0x80, 0x00);  
		
	spi_rw(0x07|0x80,SI4432_PWRSTATE_RX );
		k=spi_rw(0x07, 0);   
	spi_rw(0x05|0x80, SI4432_Rx_packet_received_interrupt);  	

		
}		


unsigned char tx_data(unsigned char *pt,unsigned char leg)
{

	unsigned char i,ItStatus,k;
	uint8_t Temp=0,cir=1;
	uint16_t ct=0;
	k=k;
	 
	spi_rw(0x05|0x80, SI4432_PACKET_SENT_INTERRUPT);
 		 
	spi_rw(0x07|0x80, SI4432_PWRSTATE_READY);	// rf 模块进入Ready 模式
	k=spi_rw(0x07, 0);
	k=spi_rw(0x02, 0); 	
	TX1_RX0;	
    spi_rw(0x08|0x80, 0x03);   
	spi_rw(0x08|0x80, 0x00); 

	spi_rw(0x34|0x80, 64); // 
	spi_rw(0x3e|0x80, leg); //
  	for (i = 0; i<leg; i++)
	{
		spi_rw(0x7f|0x80,*(pt+i));//
	}
  	 
     spi_rw(0x07|0x80, SI4432_PWRSTATE_TX);

 while(cir==1)
  {
     Temp=spi_rw(0x03,0x00);
     Temp=Temp&0x04;
     if(Temp!=0){cir=0;}
	 /*Operating Modes Response Time see datasheet p20 the TIMEMIN is 200us 
	 but in-severs use we mesurment time is min 350us that in M051 seril MCUs 
	 thus WE need 1000us TIME  make enough time conver Mode
   */
     DrvSYS_Delay(350);
     ct++;
     if(ct>=350){cir=0;}
  }
 
  if(ct>=350)
  {

 	 spi_rw(0x07|0x80,0x01);
	 SI4432_init();
	 rx_data();
     return 0; 	    //  TX sent fail
  }
  else 
  {
     ItStatus = spi_rw(0x03,0x00); //read the Interrupt Status1 register
     ItStatus = spi_rw(0x04,0x00); //read the Interrupt Status2 register
	 //read 03 04 REG by clear tx interruput status 
     spi_rw(0x07|0x80,0x01);	   //tx sent success		  
     SI4432_init();
   	 rx_data();
     return 1;
  }


	 
}
 /* 
				 START
				 ch0 
				 f=433Mhz 
				 type=GFSK 
				 speed=19.2Kbps
				 Manchester =off 
				 crystal tx/rx =20 ppm
				 AFC=ENABLE
				 Max.Rb Error =<1% 
				 Frequency Deviation(KHZ)=50
				 RXBW(KHZ)=200
				
				 */

  extern uint32_t flash_net_buf[4];
void Si4432RegisterSetV26()
{


 //以下使用Si4432-Register-Settings_RevV-v26表生成


   uint8_t RF_Chl= flash_net_buf[3];//取出来最低位8bit 通道号
              spi_rw(0x1c|0x80, 0xac);
			  spi_rw(0x1d|0x80, 0x40);
			
			  spi_rw(0x20|0x80, 0x9c);
			  spi_rw(0x21|0x80, 0x00);
			  spi_rw(0x22|0x80, 0xd1);
			  spi_rw(0x23|0x80, 0xb7);
			  spi_rw(0x24|0x80, 0x00);
			  spi_rw(0x25|0x80, 0x53);
			
			
			  spi_rw(0x30|0x80, 0xac);
			  spi_rw(0x32|0x80, 0x8c);
			  spi_rw(0x33|0x80, 0x02);
			  spi_rw(0x34|0x80, 0x08);
			  spi_rw(0x35|0x80, 0x2a);
			  spi_rw(0x36|0x80, 0x2d);
			  spi_rw(0x37|0x80, 0xd4);
			  spi_rw(0x38|0x80, 0x0);
			  spi_rw(0x39|0x80, 0x0);
			  spi_rw(0x3a|0x80, 0x0);
			  spi_rw(0x3b|0x80, 0x0);
			  spi_rw(0x3c|0x80, 0x0);
			  spi_rw(0x3d|0x80, 0x0);
			  spi_rw(0x3e|0x80, 0x0);
			  spi_rw(0x3f|0x80, 0x0);
			  spi_rw(0x40|0x80, 0x0);
			  spi_rw(0x41|0x80, 0x0);
			  spi_rw(0x42|0x80, 0x0);
			  spi_rw(0x43|0x80, 0xff);
			  spi_rw(0x44|0x80, 0xff);
			  spi_rw(0x45|0x80, 0xff);
			  spi_rw(0x46|0x80, 0xff);
			  spi_rw(0x56|0x80, 0x00);
			
			
			  spi_rw(0x6e|0x80, 0x9d);
			  spi_rw(0x6f|0x80, 0x49);
			
			
			  spi_rw(0x70|0x80, 0x2c);
			  spi_rw(0x71|0x80, 0x2b);
			  spi_rw(0x72|0x80, 0x50);
			
   
	switch(RF_Chl){
		  case 0:{
			 	  //中心频率433.0MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x4b);
			  spi_rw(0x77|0x80, 0x00);
			break;}

		   case 1:{

		   	  //中心频率433.1MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x4d);
			  spi_rw(0x77|0x80, 0x80);
		   
		    break;}
			case 2:{

		   	  //中心频率433.2MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x50);
			  spi_rw(0x77|0x80, 0x00);
		   
		    break;}
			case 3:{

		   	  //中心频率433.3MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x52);
			  spi_rw(0x77|0x80, 0x80);
		   
		    break;}
			case 4:{
			    //中心频率433.4MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x55);
			  spi_rw(0x77|0x80, 0x00);
			
			
			break;}
			case 5:{
				   //中心频率433.5MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x57);
			  spi_rw(0x77|0x80, 0x80);
			break;}
			case 6:{
				  //中心频率433.6MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x5A);
			  spi_rw(0x77|0x80, 0x00);
			break;}
			case 7:{
				   //中心频率433.7MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x5C);
			  spi_rw(0x77|0x80, 0x80);
			break;}
			case 8:{
				 //中心频率433.8MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x5F);
			  spi_rw(0x77|0x80, 0x00);
			break;}
			case 9:{
			    //中心频率433.9MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x61);
			  spi_rw(0x77|0x80, 0x80);
			break;}
			case 10:{
			//中心频率432.0MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x32);
			  spi_rw(0x77|0x80, 0x00);
			break;}
			case 11:{
				//中心频率432.1MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x34);
			  spi_rw(0x77|0x80, 0x80);
			break;}
			case 12:{
				  //中心频率432.2MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x37);
			  spi_rw(0x77|0x80, 0x00);
			break;}
			case 13:{
				//中心频率432.3MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x39);
			  spi_rw(0x77|0x80, 0x80);
			break;}
			case 14:{
			   //中心频率432.4MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x3C);
			  spi_rw(0x77|0x80, 0x00);

			break;}
			case 15:{
			   //中心频率432.5MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x3E);
			  spi_rw(0x77|0x80, 0x80);
			
			break;}
			case 16:{
				//中心频率432.6MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x41);
			  spi_rw(0x77|0x80, 0x00);
			break;}
			case 17:{
			   //中心频率432.7MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x43);
			  spi_rw(0x77|0x80, 0x80);
			break;}
			case 18:{
			   //中心频率432.8MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x46);
			  spi_rw(0x77|0x80, 0x00);
			
			break;}
			case 19:{
				 //中心频率432.9MHZ
			  spi_rw(0x75|0x80, 0x53);
			  spi_rw(0x76|0x80, 0x48);
			  spi_rw(0x77|0x80, 0x80);
			break;}


		 }

 //以上使用Si4432-Register-Settings_RevV-v26表生成
}
//REG配置		
void SI4432_init(void)
{

  	clr_interruput_si4432();

	spi_rw(0x06|0x80, 0x00);  //  关闭不需要的中断
	
	spi_rw(0x07|0x80, 1);   // 进入 Ready 模式xton=1
	 


	spi_rw(0x09|0x80, 0x64);  //  负载电容


	spi_rw(0x0a|0x80, 0x05);	// 关闭低频输出
	spi_rw(0x0b|0x80, 0xea);  // GPIO 0 当做普通输出口
	spi_rw(0x0c|0x80, 0xea);  //GPIO 1 当做普通输出口
	spi_rw(0x0d|0x80, 0xea);  // /GPIO 2 输出收到的数据

	Si4432RegisterSetV26();	

    spi_rw(0x6d|0x80, 0x07);  // set power  00:Min sent power 07:MAX sent Power

    spi_rw(0x79|0x80, 0x0);  // 不需要跳频
    spi_rw(0x7a|0x80, 0x0);  // 不需要跳频		  
}

 //check the SI4432 chip is connect to us or check the SPI conmmucation is ok 
 unsigned char Check_si4432_HW(void)
 {
 	 unsigned char state;
	state = spi_rw(0,0);//read chip ID that always is 0X08 more ref SII443x datasheet REG discription  
    if(state!=0x08) 
	{
		return 0; // no device or the SPI read fail 
	}

	 return 1;	  // the chip is running fine  
 }

 /*
   read 8bit data 
 
 */
unsigned char spi_byte(unsigned char data)
{

	unsigned char i;
	
	for (i = 0; i < 8; i++)		// 控制SCK 和 SDI，发射一个字节的命令，同事读取1个字节的数据
	{				// 没有包括nSEL的控制
		if (data & 0x80)
			SDI_UP;
		else
			SDI_DOWN;
			
		data <<= 1;
		SCK_UP;
		
		if (GET_SDO)
			data |= 0x01;
		else
			data &= 0xfe;
			
		SCK_DOWN;
	}
	
	return (data);
}

/*--------------------
  read/write by SPI function  NOW we just used SI4432 SPI
  eg:
  write: spi_rw(reg|0x80, data)
  reade: data = spi_rw(reg, 0)

-----------------------*/
unsigned char spi_rw(unsigned char addr, unsigned char data)
{

	unsigned char i;
	SCK_DOWN;
	nSEL_DOWN;
	
	for (i = 0; i < 8; i++) 
	{
		if (addr & 0x80)
			SDI_UP;
		else
			SDI_DOWN;
		addr <<= 1;
		SCK_UP;
	
		SCK_DOWN;
	 
	}
	  // 	DrvSYS_Delay(2);

	for (i = 0; i < 8; i++) 
	{
		if (data & 0x80)
			SDI_UP;
		else
			SDI_DOWN;
		data <<= 1;
		SCK_UP;
		if (GET_SDO)
			data |= 0x01;
		else
			data &= 0xfe;
		 
		SCK_DOWN;
	}
	nSEL_UP;
	SCK_UP;
	return data;
}

#endif