/**
  ******************************************************************************
  * @file    client.c
  * @author  MCD Application Team
  * @version wangjunwei
  * @date    20150717
  * @brief   A sample TCP client  Test
 */

#include "main.h"
#include <string.h>
#include <stdio.h>
#include "lwip/opt.h"
#include "lwip/tcp_impl.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include "filesystem.h"
#include "stm32f10x.h"
#include "flash_if.h"


#define TCP_PORT      40096
//5s 一次TCP连接	 (250ms*4)*5
#define TCP_CREATTRM_INTERVAL  (250*4)*5
//系统运行时间1S更新一次
#define SYSRUN_TIMER_MSECS         500
//2s	SENT TCP DATA
#define TCPSent_CREATTRM_INTERVAL (250*4)*2 
//500ms	SENT UDP DATA
#define UDPSent_CREATTRM_INTERVAL (250*2)
/* Private function prototypes -----------------------------------------------*/
void udp_client_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
void tcp_client_err(void *arg, err_t err);
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

void  tcp_client_callback(void);
char Tcplink=1;	//TCP连接 1是代表可以重练 0表示不可以当前已经在连接中了
char Udplink=0;	//UDP连接 0是代表可以重练 1表示不可以当前已经在连接中了
struct tcp_pcb *wpcb; 
uint32_t retry_TCP_connect=0;
uint32_t SystemRunTime=0;
uint32_t TCPCreatTrm =0;
uint32_t RunSecTimer = 0;
struct udp_pcb *upcb;
u8 Udpbuf[10];
char UID_STM32[16];

extern uint32_t flash_net_buf[2];
extern uint8_t ipaddr_sever[4];	//sever IP
extern uint16_t port_sever;		//sever PORT
void udp_client_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	  	    struct pbuf *ps;
			char data[1024];
   if(p != NULL)
   {
	   if(p->tot_len<1024)
		{
			           // data = p->payload;	

						memcpy(data,p->payload,p->tot_len);
						 
						 pbuf_free(p); //第一时间释放pbuf


		                if (strncmp(data, "getid", 5) == 0)
						{

						   uint32_t id;

						  UID_STM32[0]=0xAB;//mark

						  UID_STM32[1]=0x00;//备用

						  id=(UID->UID0_31+UID->UID32_63+UID->UID64_95);//sum ID
						  
				 		  UID_STM32[2]=id;
						  UID_STM32[3]=id>>8;//
						  UID_STM32[4]=id>>16; // build ID 	lost The Hight 8bit data save the low 3 byte data as it ID

						  UID_STM32[5]=0xCD; //mark

						  UID_STM32[6]=0x24;
						  UID_STM32[7]=0x0d;  //efl
						  UID_STM32[8]=0x0a;

						  ps = pbuf_alloc(PBUF_TRANSPORT, 9, PBUF_RAM);
						  if(ps!=(0))
						  {
						  memcpy((u8*)ps->payload,(u8*)&UID_STM32[0],9);
						  udp_sendto(upcb, ps,addr, port);
						  }
						  pbuf_free(ps);

		                  }
						  else if (strncmp(data, "getrun", 6) == 0)
					     {
						  UID_STM32[0]=0xAA;
						  UID_STM32[4]=retry_TCP_connect;
						  UID_STM32[3]=retry_TCP_connect>>8;
						  UID_STM32[2]=retry_TCP_connect>>16;	  //重练次数
						  UID_STM32[1]=retry_TCP_connect>>24;

						  UID_STM32[8]=SystemRunTime;
						  UID_STM32[7]=SystemRunTime>>8;		  //运行时间
						  UID_STM32[6]=SystemRunTime>>16;
						  UID_STM32[5]=SystemRunTime>>24;
									
						  UID_STM32[9]=0xBB;
						  UID_STM32[10]=0x0d;
						  UID_STM32[11]=0x0a;
					      ps = pbuf_alloc(PBUF_TRANSPORT, 12, PBUF_RAM);
						  if(ps!=(0))
						  {
						  memcpy((u8*)ps->payload,(u8*)&UID_STM32[0],12);
						  udp_sendto(upcb, ps,addr, port);
						  }
						  pbuf_free(ps);


					     }
						 else if (strncmp(data, "getiap", 6) == 0)
						 {
						 	   //获取IAP当前的配置
							char iap[10];
							  GetIpFromFlash(flash_net_buf);//读取2个字到buf中
							  iap[0] = 0xAB;
							  iap[1] = flash_net_buf[2]>>24;//高八位
							  iap[2] = flash_net_buf[2]>>16;
							  iap[3] = flash_net_buf[2]>>8;
							  iap[4] = flash_net_buf[2]>>0;//低八位
							  iap[5]=0xCD; //mark
						      iap[6]=0x24;
						      iap[7]=0x0d;  //efl
						      iap[8]=0x0a;
						
						  ps = pbuf_alloc(PBUF_TRANSPORT, 10, PBUF_RAM);
						  if(ps!=(0))
						  {
						  memcpy((u8*)ps->payload,(u8*)&iap[0],17);
						  udp_sendto(upcb, ps,addr, port);
						  }
						  pbuf_free(ps);
						 }
						 else if (strncmp(data, "setiap", 6) == 0)
						 {

						 		 //设置上电IAP区域运行							   
								 flash_net_buf[2]=0x55555555;
								WriteIpToFlash(flash_net_buf,3);
					
								ps = pbuf_alloc(PBUF_TRANSPORT, 4, PBUF_RAM);
								if(ps!=(0))
						       {
								memcpy((u8*)ps->payload,"OK\r\n",4);
								udp_sendto(upcb, ps,addr, port);
								}
								pbuf_free(ps);

						 
						 }
						  else if (strncmp(data, "setapp", 6) == 0)
						 {
						        //设置上电APP区域运行
						 	   						   
								flash_net_buf[2]=0xAAAAAAAA;
								WriteIpToFlash(flash_net_buf,3);
					
								ps = pbuf_alloc(PBUF_TRANSPORT, 4, PBUF_RAM);
								if(ps!=(0))
						        {
								memcpy((u8*)ps->payload,"OK\r\n",4);
								udp_sendto(upcb, ps,addr, port);
								}
								pbuf_free(ps);
						 
						 }
						  else if ((strncmp(data, "setchl", 6) == 0))
							{
								/*设置频道*/
								   flash_net_buf[3]=data[6];
								WriteIpToFlash(flash_net_buf,3);

							ps = pbuf_alloc(PBUF_TRANSPORT, 4, PBUF_RAM);
							  if(ps!=(0))
							  {
							  memcpy((u8*)ps->payload,"OK\r\n",4);
							  udp_sendto(upcb, ps,addr, port);
							  }
							  pbuf_free(ps);

							}
						 else if ((strncmp(data, "getchl", 6) == 0))
							{
							char str[11];
							  GetIpFromFlash(flash_net_buf);//读取2个字到buf中
							  str[0] = 0xAB;
							  str[1] = 'C';
							  str[2] = 'H';
							  str[3] = flash_net_buf[3]>>24;//高八位
							  str[4] = flash_net_buf[3]>>16;
							  str[5] = flash_net_buf[3]>>8;
							  str[6] = flash_net_buf[3]>>0;//低八位
							  str[7]=0xCD; //mark
						      str[8]=0x24;
						      str[9]=0x0d;  //efl
						      str[10]=0x0a;
							   ps = pbuf_alloc(PBUF_TRANSPORT, 12, PBUF_RAM);
						  if(ps!=(0))
						  {
						  memcpy((u8*)ps->payload,(u8*)&str[0],11);
						  udp_sendto(upcb, ps,addr, port);
						  }
						  pbuf_free(ps);
						   	
							}
						 else if (strncmp(data, "getconfig", 9) == 0)
						 {
							  char str[17];
							  GetIpFromFlash(flash_net_buf);//读取2个字到buf中
							  str[0] = 0xAB;
							  str[1] = 'I';
							  str[2] = 'P';
							  str[3] = flash_net_buf[0]>>24;//高八位
							  str[4] = flash_net_buf[0]>>16;
							  str[5] = flash_net_buf[0]>>8;
							  str[6] = flash_net_buf[0]>>0;//低八位
							  str[7] = 'P';
							  str[8] = 'O';
							  str[9] = 'R';
							  str[10] = 'T';
							  str[11] = flash_net_buf[1]>>8;//高8位
							  str[12] = flash_net_buf[1];//低8位
							  str[13]=0xCD; //mark
						      str[14]=0x24;
						      str[15]=0x0d;  //efl
						      str[16]=0x0a;
						
						   ps = pbuf_alloc(PBUF_TRANSPORT, 17, PBUF_RAM);
						  if(ps!=(0))
						  {
						  memcpy((u8*)ps->payload,(u8*)&str[0],17);
						  udp_sendto(upcb, ps,addr, port);
						  }
						  pbuf_free(ps);
						}
						else if ((strncmp(data, "setconfig", 9) == 0)&&
						    ((strncmp(&data[17], ":",1) == 0)))
		  				 {

								ipaddr_sever[0]=data[10];//取出最高IP位
								ipaddr_sever[1]=data[12];
								ipaddr_sever[2]=data[14];
								ipaddr_sever[3]=data[16];

								port_sever     =data[18];
								port_sever<<=8;
								port_sever     +=data[19];


					
								flash_net_buf[0]=((uint32_t)(ipaddr_sever[0]<<24))+
								                 ((uint32_t)(ipaddr_sever[1]<<16))+
												 ((uint32_t)(ipaddr_sever[2]<<8))+
												 ((uint32_t)(ipaddr_sever[3]<<0));
						
								flash_net_buf[1]= port_sever;
								
								WriteIpToFlash(flash_net_buf,3);
					
								ps = pbuf_alloc(PBUF_TRANSPORT, 4, PBUF_RAM);
								if(ps!=(0))
						       {
								memcpy((u8*)ps->payload,"OK\r\n",4);
								udp_sendto(upcb, ps,addr, port);
								}
								pbuf_free(ps);

										//[setconfig.1.2.3.4:56] ascii 字符
										//1.2.3.4分别表示IP地址的4个字节，用逗号隔开，高位在前地位在后
										//56，表示端口号，高位在前地位在后，用：引导
										//73 65 74 63 6F 6E 66 69 67 2E 31 2E 32 2E 33 2E 34 3A 35 36  HEX显示
							/*
										 例子：
										 我要配置服务器的IP为192.168.0.4，端口2301，
										 转换十六进制为
										 IP地址192.168.0.4，端口2301，-> HEX IP地址：0xc0,0xa8,0x00,0x04 端口：0x08fd
										 则就要发送如下命令
										 {73 65 74 63 6F 6E 66 69 67 2E  C0  2E  A8  2E  00  2E  04  3A  08  FD } 
										 {[0][1][2][3][4][5][6][7][8][9][10][11][12][13][14][15][16][17][18][19]}
										 
							*/
							
							
							
							

						   }
						    else
						   if (strncmp(data, "getver", 6) == 0)
						   {
								//获取版本号
						   unsigned char const DataStr[]=__DATE__; //12
						   unsigned char const TimeStr[]=__TIME__; //9
						   unsigned char vesion[40];
						   		
						   memcpy(&vesion[0],"version=",8);
							vesion[8]=' ';
							vesion[9]=MAIN_SN+0x30;//主序号
							vesion[10]='.';
							vesion[11]=SECOND_SN+0x30;//副序号
							vesion[12]='.';
							vesion[13]=SUB_SN+0x30;//子序号
							vesion[14]=' ';
						   
						   memcpy(&vesion[15],DataStr,12);
						   memcpy(&vesion[27],TimeStr,9);
						   vesion[26]=' ';

					      ps = pbuf_alloc(PBUF_TRANSPORT, 40, PBUF_RAM);
						  if(ps!=(0))
						  {
						  memcpy((u8*)ps->payload,(u8*)&vesion[0],40);
						  udp_sendto(upcb, ps,addr, port);
						  }
						  pbuf_free(ps);

						   }
						   else
						   if (strncmp(data, "reboot", 6) == 0)
						   {
						   	   //系统重启

								   #ifndef USED_WATCHDOG
								  RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);//使能看门狗
								 /*
								    @arg WWDG_Prescaler_1: WWDG counter clock = (PCLK1/4096)/1
							  *     @arg WWDG_Prescaler_2: WWDG counter clock = (PCLK1/4096)/2
							  *     @arg WWDG_Prescaler_4: WWDG counter clock = (PCLK1/4096)/4
							  *     @arg WWDG_Prescaler_8: WWDG counter clock = (PCLK1/4096)/8
								 */
								  WWDG_SetPrescaler(WWDG_Prescaler_1);//配置看门狗时钟分频系数
								  WWDG_SetWindowValue(0x40);//设置窗口时间
								  WWDG_SetCounter(0x7F);//喂狗
								  WWDG_Enable(0x7F);//启动看门狗
								  while(1);//WAITE :system will RESET
								  #else
								  while(1);//WAITE :system will RESET
						          #endif


						   
						   }



		
		}//if(p->tot_len<1024)
		else
		{
		 pbuf_free(p);
		}



	//接收固定长度的广播到数据并转发广播出去 test data len >10

	/*
	 if(p->tot_len<10){
	    memcpy((u8*)&Udpbuf[0],(u8*)p->payload,p->tot_len);
		ps = pbuf_alloc(PBUF_TRANSPORT, p->tot_len, PBUF_RAM);
		memcpy((u8*)ps->payload,(u8*)&Udpbuf[0],p->tot_len);
		udp_sendto(upcb, ps,IP_ADDR_BROADCAST, 21223);
		     pbuf_free(ps);
	  				  }

	   pbuf_free(p); */

	   //pbuf_free(p);

   }//if(p != NULL)	  
	  
}

void udp_client_callback_app(void)
{
                          
   /* Create a new UDP control block  */
   upcb = udp_new();   
   upcb->so_options |= SOF_BROADCAST;
 
   /* Bind the upcb to any IP address and the UDP_PORT port*/
   udp_bind(upcb, IP_ADDR_ANY, 21228);  
   /* Set a receive callback for the upcb */
   udp_recv(upcb, udp_client_callback, NULL);

}
  char cansent=0;
  extern char* Getmeminf();
  extern uint32_t smtp_Tcp_count[10];
   char TcpRecvBuf[1500];
   uint32_t TcpRecvLenth=0;
   char Data[2]={0xFF,0xAA};
   char Data1[2]={0xFF,0xBB};
   char Statues_MB=0x00;
/*服务器发下的协议类型 
协议帧格式
帧头+类型+数据域+\r\n
帧头：
一帧网关和服务器交互的开始同时还担负判读数据上传还是下发的任务。
【XFF+0X55】：表示数据上传到服务器
【0XFF+0XAA】: 表示是数据下发到网关
类型：
0x01:表示土壤温湿度传感器
0x02表示光照传感器
0x03 表示PH 值传感器
0x04 表示氧气含量传感器
数据域
不同的类型的传感器数据域，数据域就是厂家提供的MODBUS-RTU的协议直接搭载上。

 服务器发送：FF AA + 类型 +【modbus RTU 数据域 】
 网关回复  ：FF 55 + ID + 类型 +【modbus RTU 数据域 】+55 FF +\r\n

*/



	uint8_t SeverIP[4] = {0};
	uint32_t SeverPORT = 0;


#ifdef   RS485_MASTER_CALL
extern uint8_t TCP2RS485CMDLIST[1400];	//用来接收来自服务器的轮训发送数据帧
extern uint32_t DevIndex; //用来索引上表中的设备指令，
#endif

  err_t  tcp_client_reciver(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{

				static char Tdata[1500]; //数组太大定义为静态
					 int tcplen;
			
 if (err == ERR_OK && p != NULL)
  {

    /* Inform TCP that we have taken the data. */
    tcp_recved(tpcb, p->tot_len);
		// data = p->payload;
		tcplen=p->tot_len;	 
		 if(tcplen<1500) 
		 {
		memcpy(Tdata,p->payload,p->tot_len);//抓紧复制出来数据
		 }	    				 
	            pbuf_free(p);//第一时间释放pbuf	
		

					if (strncmp(Tdata, Data, 2) == 0)
					 {
						 						 
							//char *ptr=(char*)p->payload;
							char *ptr=(char*)Tdata;
							  
								ptr+=3;	//jump FF AA and other there the ptr is point to MODBUS-RTU   
							 // if(p->tot_len<1500) //asseter the buffer lenth 
							 // {
	
							 TcpRecvLenth=tcplen-5-3;//upadta the recive data lenth	     AA FF  24 0D 0A =5 BYTES
							   
							 memcpy(TcpRecvBuf,ptr,TcpRecvLenth); // data copy to appliction data buffer 											 s
							   //}


				  	   }
					   
					    else
					   #ifdef   RS485_MASTER_CALL
					   	if (strncmp(Tdata, Data1, 2) == 0)
						{

						  	char *ptr=(char*)Tdata;
							   
								ptr+=2;	//jump FF BB and other there the ptr is point to MODBUS-RTU   
							  //if(p->tot_len<1500) //asseter the buffer lenth 
							 // {
	
							 TcpRecvLenth=tcplen-5-2;//upadta the recive data lenth	     BB FF  24 0D 0A =5 BYTES
							 memset(TCP2RS485CMDLIST, 0, 1024);//先清空BUFF 
							 memcpy(TCP2RS485CMDLIST,ptr,TcpRecvLenth); // data copy to appliction data buffer 											 s
							   	DevIndex=0;//索引在每次更新的时候都被重置，重头开始
							   	TcpRecvLenth=0;//这是配置数据不需要立即发送485，modbus_main.c里面会自动处理，这里只要把数据塞进去即可

						    tcp_write(tpcb, "FFBBsave\r\n", sizeof( "FFBBsave\r\n"), 1);
						    tcp_output(tpcb);

							  // }
					   
					   }
					  else
					  #endif
					if (strncmp(Tdata, "mem", 3) == 0)
					 {
						   char *ptr;
						   uint16_t len;

		                 ptr=Getmeminf();
						 len=strlen(ptr);
						 tcp_write(tpcb, ptr, len, 1);
						 tcp_output(tpcb);
					  }
					  else
						if (strncmp(Tdata, "start", 5) == 0)
					 {
							cansent=1;
						 tcp_write(tpcb, "startsent\r\n", sizeof( "startsent\r\n"), 1);
						 tcp_output(tpcb);
					 }
					 else
					  	if (strncmp(Tdata, "stop", 4) == 0)
					 {
							cansent=0;
						 tcp_write(tpcb, "startsent\r\n", sizeof( "startsent\r\n"), 1);
						 tcp_output(tpcb);
					 }
					 else
		             if (strncmp(Tdata, "getrun", 6) == 0)
					 {
						  UID_STM32[0]=0xAA;
						  UID_STM32[4]=retry_TCP_connect;
						  UID_STM32[3]=retry_TCP_connect>>8;
						  UID_STM32[2]=retry_TCP_connect>>16;	  //重练次数
						  UID_STM32[1]=retry_TCP_connect>>24;

						  UID_STM32[8]=SystemRunTime;
						  UID_STM32[7]=SystemRunTime>>8;		  //运行时间
						  UID_STM32[6]=SystemRunTime>>16;
						  UID_STM32[5]=SystemRunTime>>24;
									
						  UID_STM32[9]=0xBB;
						  UID_STM32[10]=0x0d;
						  UID_STM32[11]=0x0a;
					 tcp_write(tpcb, &UID_STM32, 12, 1);
							 tcp_output(tpcb);
					 }
					 else
					 if (strncmp(Tdata, "getid", 5) == 0)
						{

						   uint32_t id;
						   id=(UID->UID0_31+UID->UID32_63+UID->UID64_95);//sum ID
						   //FFAA 01 01 000001 01 0001 000000(设备id用四个零) 00（数据用00代替） AAFF 240D0A 

						  UID_STM32[0]=0xFF;//mark
						  UID_STM32[1]=0xAA;

						  UID_STM32[2]=0x01;

						  UID_STM32[3]=0x01;

				 		  UID_STM32[4]=id;
						  UID_STM32[5]=id>>8;//
						  UID_STM32[6]=id>>16; // build ID 	lost The Hight 8bit data save the low 3 byte data as it ID

						  UID_STM32[7]=0x01;

						  UID_STM32[8]=0x00;
						  UID_STM32[9]=0x26; //485转TCP网关

						  UID_STM32[10]=0;
						  UID_STM32[11]=0;//
						  UID_STM32[12]=0; // dev ID  =0x000000

						  UID_STM32[13]=0x00;
				
						  UID_STM32[14]=0xAA; //mark
						  UID_STM32[15]=0xFF; //mark

						  UID_STM32[16]=0x24;
						  UID_STM32[17]=0x0d;  //efl
						  UID_STM32[18]=0x0a;
							 tcp_write(tpcb, &UID_STM32, 19, 1);
							 tcp_output(tpcb);


		                  }
						  else if ((strncmp(Tdata, "setchl", 6) == 0))
							{
								/*设置频道*/
							   flash_net_buf[3]=Tdata[6];
							   	WriteIpToFlash(flash_net_buf,3);
							  	tcp_write(tpcb,"OK！\r\n",sizeof("OK！\r\n"), 1);
							 tcp_output(tpcb);
							}
						 else if ((strncmp(Tdata, "setconfig", 9) == 0)&&
						    ((strncmp(&Tdata[17], ":",1) == 0)))
		  				 {

								ipaddr_sever[0]=Tdata[10];//取出最高IP位
								ipaddr_sever[1]=Tdata[12];
								ipaddr_sever[2]=Tdata[14];
								ipaddr_sever[3]=Tdata[16];

								port_sever     =Tdata[18];
								port_sever<<=8;
								port_sever     +=Tdata[19];


					
								flash_net_buf[0]=((uint32_t)(ipaddr_sever[0]<<24))+
								                 ((uint32_t)(ipaddr_sever[1]<<16))+
												 ((uint32_t)(ipaddr_sever[2]<<8))+
												 ((uint32_t)(ipaddr_sever[3]<<0));
						
								flash_net_buf[1]= port_sever;
								
								WriteIpToFlash(flash_net_buf,0);
					
						  	tcp_write(tpcb,"OK！\r\n",sizeof("OK！\r\n"), 1);
							 tcp_output(tpcb);
						
						}
						else if ((strncmp(Tdata, "getchl", 6) == 0))
							{
							char str[11];
							  GetIpFromFlash(flash_net_buf);//读取2个字到buf中
							  str[0] = 0xAB;
							  str[1] = 'C';
							  str[2] = 'H';
							  str[3] = flash_net_buf[3]>>24;//高八位
							  str[4] = flash_net_buf[3]>>16;
							  str[5] = flash_net_buf[3]>>8;
							  str[6] = flash_net_buf[3]>>0;//低八位
							  str[7]=0xCD; //mark
						      str[8]=0x24;
						      str[9]=0x0d;  //efl
						      str[10]=0x0a;

						   	tcp_write(tpcb,str,11, 1);
							 tcp_output(tpcb);
							}
						else if (strncmp(Tdata, "getconfig", 9) == 0)
						  {
							  char str[17];
							  GetIpFromFlash(flash_net_buf);//读取2个字到buf中
							  str[0] = 0xAB;
							  str[1] = 'I';
							  str[2] = 'P';
							  str[3] = flash_net_buf[0]>>24;//高八位
							  str[4] = flash_net_buf[0]>>16;
							  str[5] = flash_net_buf[0]>>8;
							  str[6] = flash_net_buf[0]>>0;//低八位
							  str[7] = 'P';
							  str[8] = 'O';
							  str[9] = 'R';
							  str[10] = 'T';
							  str[11] = flash_net_buf[1]>>8;//高8位
							  str[12] = flash_net_buf[1];//低8位
							  str[13]=0xCD; //mark
						      str[14]=0x24;
						      str[15]=0x0d;  //efl
						      str[16]=0x0a;

						   	tcp_write(tpcb,str,17, 1);
							 tcp_output(tpcb);

						  }	  
						else if (strncmp(Tdata, "reboot", 6) == 0)
						   {
						   	   //系统重启


						  RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);//使能看门狗
						 /*
						    @arg WWDG_Prescaler_1: WWDG counter clock = (PCLK1/4096)/1
					  *     @arg WWDG_Prescaler_2: WWDG counter clock = (PCLK1/4096)/2
					  *     @arg WWDG_Prescaler_4: WWDG counter clock = (PCLK1/4096)/4
					  *     @arg WWDG_Prescaler_8: WWDG counter clock = (PCLK1/4096)/8
						 */
						  WWDG_SetPrescaler(WWDG_Prescaler_1);//配置看门狗时钟分频系数
						  WWDG_SetWindowValue(0x40);//设置窗口时间
						  WWDG_SetCounter(0x7F);//喂狗
						  WWDG_Enable(0x7F);//启动看门狗
						  while(1);//WAITE :system will RESET
						   }
						 
	 
   }


if ((err == ERR_OK && p == NULL)||(err<0))
  {
	Tcplink=1;
    tcp_close(wpcb);

  }		 
  
	return ERR_OK;
} 
 void tcp_client_err(void *arg, err_t err)
 {
	  if(err==ERR_RST) //主机复位
	  {
		 //host well be free the PCB dont close PCB

	    Tcplink=1;
	  	return ;  //DO nothing..
	  }

 	  if(err==ERR_ABRT)
	  {
 	 Tcplink=1;
  // tcp_close(wpcb);
   	  	return ;  //DO nothing..
 	   }
	 
	 //   tcp_close(wpcb);
	   	 Tcplink=1;

	 
 }

void tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{


}


 void  tcp_client_callback(void)
  {	

	  	struct ip_addr ip_addr;	

			wpcb= (struct tcp_pcb*)0;
			  /* Create a new TCP control block  */
			  wpcb = tcp_new();
			
			  /* Assign to the new pcb a local IP address and a port number */
			 if(tcp_bind(wpcb, IP_ADDR_ANY, TCP_PORT)!=ERR_OK)
			   {
			   		//failed
					 return ;
			   
			   }
			  //115.28.168.92 
			  //IP4_ADDR(&ip_addr, 192,168,0,163);
		      //IP4_ADDR(&ip_addr,114,215,155,179 );//陈新服务器
			  IP4_ADDR(&ip_addr,ipaddr_sever[0],ipaddr_sever[1],ipaddr_sever[2],ipaddr_sever[3]);//济宁客户的服务器的IP地址
			/* Connect to the server: send the SYN *///TCP_PORT
		    tcp_connect(wpcb, &ip_addr, port_sever, tcp_client_connected);
			//tcp_poll(wpcb,tcp_client_poll,2);
			tcp_err( wpcb,tcp_client_err);  //register err
			tcp_recv(wpcb,tcp_client_reciver);  //register recv

				 
}

/**
  * @brief  This function is called when the connection with the remote 
  *         server is established
  * @param arg user supplied argument
  * @param tpcb the tcp_pcb which received data
  * @param err error value returned by the tcp_connect 
  * @retval error value
  */

err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
  
  	   Tcplink=0;
 	   retry_TCP_connect++;
	tpcb->so_options |= SOF_KEEPALIVE;
    tpcb->keep_idle = 1000;// ms
    tpcb->keep_intvl = 1000;// ms
    tpcb->keep_cnt = 2;// report error after 2 KA without response

   //tcp_write(tpcb, "connecting....", 14, 1);
//	tcp_output(tpcb);
  
  return ERR_OK;
}


 
void TcpTestThread(uint32_t Time)
{

if (Time - TCPCreatTrm >= TCP_CREATTRM_INTERVAL)
  {
    TCPCreatTrm =  Time;
	 
	 if(Tcplink)//if link =1,will be connectting to sever ..
	 {
	
	  FS_Write(1,"TCP_Retry\r\n", sizeof("TCP_Retry\r\n"));
	  tcp_client_callback();
	 
	 }
	 if(Udplink==0)//just actived once 
	 {
	   Udplink=1; 
	   FS_Write(1,"UDP_Retry\r\n", sizeof("UDP_Retry\r\n"));
	   udp_client_callback_app();
	   

	 }
    
  }






}

void SynruntimHook(uint32_t Time)
{
 		 
 if ( Time  - RunSecTimer  >= SYSRUN_TIMER_MSECS)
  {
		RunSecTimer =  Time;

		SystemRunTime++;  //  系统运行时间单位1s一次
	  	
  }

}

uint8_t Mb2TcpBuff[256];
uint32_t Mb2TcpLenth=0;
char num=0,num1=0;
uint32_t TCPSentCreatTrm=0;
 void SendDataToSever(uint32_t Time)
 {			err_t erro;
	
  

 		 if((Tcplink==0)&&(Mb2TcpLenth>0)/*&&((Time - TCPSentCreatTrm >= TCPSent_CREATTRM_INTERVAL)*/)
		 {
		        //TCPSentCreatTrm =  Time;
		   //连接成功啦可以发送数据


				   erro= tcp_write(wpcb, Mb2TcpBuff, Mb2TcpLenth, 1);
					  Mb2TcpLenth=0; //写入tcpbuff后释放掉这个长度计数
				 if(erro==ERR_OK )
			        {
				
				
				         if(tcp_output(wpcb)==ERR_OK)
						 {
						 
							//成功的发送了一包
						 }
						 else
						 {
						   //	Tcplink=1;
   							// tcp_close(wpcb);
						 }
					}
					else
					{
						  num++;
						  if(num>=50)
						  {	   num=0;
									  switch(erro)
									  {
								/*	  case -1:{FS_Write(1,"Out of memory error\r\n", sizeof("Out of memory error\r\n"));break;}
									  case -2:{FS_Write(1,"Buffer error", sizeof("Buffer error"));break;}
									  case -3:{FS_Write(1,"Timeout", sizeof("Timeout"));break;}
									  case -4:{FS_Write(1,"Routing problem", sizeof("Routing problem"));break;}
									  case -5:{FS_Write(1,"Operation in progress", sizeof("Operation in progress"));break;}
									  case -6:{FS_Write(1,"Illegal value", sizeof("Illegal value"));break;}
									  case -7:{FS_Write(1,"Operation would block", sizeof("Operation would block"));break;}
									  case -8:{FS_Write(1,"Address in use", sizeof("Address in use"));break;}
									  case -9:{FS_Write(1,"Already connected", sizeof("Already connected"));break;}
									  case -10:{FS_Write(1,"Connection aborted", sizeof("Connection aborted"));break;}
									  case -11:{FS_Write(1,"Connection reset", sizeof("Connection reset"));break;}
									  case -12:{FS_Write(1,"Connection closed", sizeof("Connection closed"));break;}
									  case -13:{FS_Write(1,"Not connected", sizeof("Not connected"));break;}
									  case -14:{FS_Write(1,"Illegal argument", sizeof("Illegal argument"));break;}
									  case -15:{FS_Write(1,"Low-level netif error", sizeof("Low-level netif error"));break;}
									   */
									  }
									  
									   if(erro!=(-1))
									   {
									 //  	Tcplink=1;
				  					 //  tcp_close(wpcb);
									   }

						}
					}

		 }
		 else
		 if(Mb2TcpLenth>0)
		 {
		  
			 Mb2TcpLenth=0;// if no TCP connection but we recived a farme of usart.	we just make the lenth to "0" other do nothing .

		 }
		 else
		 {
		 	 //none succefull connection and no usart data ，do nothing ...
		 
		 }
 
 
 }


  #ifdef USED_SI4432
extern uint8_t RX_Buf[64];
extern uint8_t RxSi4432Len;
char num2=0;
void Si4432SendDataToSever(uint32_t Time)
 {			err_t erro;
			uint8_t Rf2TcpBuff[100];
			uint16_t Rf2TcpLenth;

 		 if((Tcplink==0)&&(RxSi4432Len>0))
		 {
		        //TCPSentCreatTrm =  Time;
		   //连接成功啦可以发送数据
			Rf2TcpLenth=RxSi4432Len;//更新接受到的数据长度，准备启动TCP发送
			 Rf2TcpBuff[0]=0xFF;
			 Rf2TcpBuff[1]=0xAA;          //帧头
			
			 Rf2TcpBuff[2]=UID_STM32[2];         //上传数据到服务器标示
			
			 Rf2TcpBuff[3]=UID_STM32[3];         //子系统Id：业务子系统的ID号 ,z暂时未用，值默认为01即可
			
			
			 Rf2TcpBuff[4]=UID_STM32[4];
			 Rf2TcpBuff[5]=UID_STM32[5];
			 Rf2TcpBuff[6]=UID_STM32[6]; //网关ID(3个字节，设备配置产生)
			
			 Rf2TcpBuff[7]=0x02;         //设备分类（1个字节 01网关、02设备、03直连 写死）
			
			
			 Rf2TcpBuff[8]=UID_STM32[8];					  
			 Rf2TcpBuff[9]=UID_STM32[9];         //设备类型（2个字节、设备配置产生）
			
			
			 Rf2TcpBuff[10]=0;
			 Rf2TcpBuff[11]=0;
			 Rf2TcpBuff[12]=0; //设备id(3个字节，设备配置产生),
			
			memcpy(&Rf2TcpBuff[13],RX_Buf,Rf2TcpLenth);//rf数据复制进TCP发送buf中
			
			
			 Rf2TcpBuff[Rf2TcpLenth+13+0]=0xAA;
			 Rf2TcpBuff[Rf2TcpLenth+13+1]=0xFF;	 //帧尾
			
			 Rf2TcpBuff[Rf2TcpLenth+13+2]=0x24;	
			 Rf2TcpBuff[Rf2TcpLenth+13+3]=0x0D;
			 Rf2TcpBuff[Rf2TcpLenth+13+4]=0x0A;	//间隔符	
			  
			 Rf2TcpLenth+=18;//计算帧总长度，最终作为发送标示发送到TCP buf
			

				   erro= tcp_write(wpcb, Rf2TcpBuff, Rf2TcpLenth, 1);
					  RxSi4432Len=0; //写入tcpbuff后释放掉这个长度计数
				 if(erro==ERR_OK )
			        {
				
				
				         if(tcp_output(wpcb)==ERR_OK)
						 {
						 
							//成功的发送了一包
						 }
						 else
						 {
						   //	Tcplink=1;
   							// tcp_close(wpcb);
						 }
					}
					else
					{
						  num2++;
						  if(num2>=50)
						  {	   num2=0;
									  switch(erro)
									  {
								/*	  case -1:{FS_Write(1,"Out of memory error\r\n", sizeof("Out of memory error\r\n"));break;}
									  case -2:{FS_Write(1,"Buffer error", sizeof("Buffer error"));break;}
									  case -3:{FS_Write(1,"Timeout", sizeof("Timeout"));break;}
									  case -4:{FS_Write(1,"Routing problem", sizeof("Routing problem"));break;}
									  case -5:{FS_Write(1,"Operation in progress", sizeof("Operation in progress"));break;}
									  case -6:{FS_Write(1,"Illegal value", sizeof("Illegal value"));break;}
									  case -7:{FS_Write(1,"Operation would block", sizeof("Operation would block"));break;}
									  case -8:{FS_Write(1,"Address in use", sizeof("Address in use"));break;}
									  case -9:{FS_Write(1,"Already connected", sizeof("Already connected"));break;}
									  case -10:{FS_Write(1,"Connection aborted", sizeof("Connection aborted"));break;}
									  case -11:{FS_Write(1,"Connection reset", sizeof("Connection reset"));break;}
									  case -12:{FS_Write(1,"Connection closed", sizeof("Connection closed"));break;}
									  case -13:{FS_Write(1,"Not connected", sizeof("Not connected"));break;}
									  case -14:{FS_Write(1,"Illegal argument", sizeof("Illegal argument"));break;}
									  case -15:{FS_Write(1,"Low-level netif error", sizeof("Low-level netif error"));break;}
									   */
									  }
									  
									   if(erro!=(-1))
									   {
									 //  	Tcplink=1;
				  					 //  tcp_close(wpcb);
									   }

						}
					}

		 }
		 else
		 if(RxSi4432Len>0)
		 {
		  
			 RxSi4432Len=0;// if no TCP connection but we recived a farme of usart.	we just make the lenth to "0" other do nothing .

		 }
		 else
		 {
		 	 //none succefull connection and no usart data ，do nothing ...
		 
		 }
 
 
 }

 #endif

 #ifdef UDPBOARDCAST_ENABLE 
 extern  u8  temp[33]; //温度字符串DS1820
  uint8_t Udpbf[1024]={0x55};
  uint32_t UDPSentCreatTrm=0;
 
 void UDPSendData(uint32_t Time)
 {
 
   	     struct pbuf *Udps;
		 ip_addr_t dst_ip_addr;

		 IP4_ADDR(&dst_ip_addr, 192,168,0,163);
		if((Udplink)&&(cansent)&&((Time - UDPSentCreatTrm >= UDPSent_CREATTRM_INTERVAL)))
		{ 
			   UDPSentCreatTrm=	Time ;
		Udps = pbuf_alloc(PBUF_TRANSPORT, 32, PBUF_RAM);
				if(Udps!=NULL)
				{
				   
							
				memcpy((u8*)Udps->payload,(u8*)&temp[0],32);
		
				udp_sendto(upcb,Udps,IP_ADDR_BROADCAST/*&dst_ip_addr*//* IP_ADDR*/,8080);
				
				 pbuf_free(Udps);
				}
				else
				{
					   num1++;
				   if(num1==50)
				   {	
				   num1=0;
			//	   UART_Write(UART0,"Oups is NULL\r\n", sizeof("Oups is NULL\r\n"));


				   }
				
				}

	 	}
 
 }


#endif

 err_t Tcp_recv(void *arg, struct tcp_pcb *pcb,  struct pbuf *p, err_t err)
{
	 if (err == ERR_OK && p != NULL)
  {
  



	   pbuf_free(p);

  }
  if ((err == ERR_OK && p == NULL)||(err<0))
  {
	 
	 tcp_close(pcb);
	
  }


}

static void conn_err(void *arg, err_t err)
{
 
}
err_t Tcp_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{ 

  pcb->so_options |= SOF_KEEPALIVE;
  pcb->keep_idle = 500;// ms
  pcb->keep_intvl = 500;// ms
  pcb->keep_cnt = 2;// report error after 2 KA without response
  //tcp_arg(pcb, pcb);
  tcp_recv(pcb, Tcp_recv);
  tcp_err(pcb, conn_err);
// tcp_poll(pcb, http_poll, 10);
  return ERR_OK;
}


 void tcp_sever_test(void)
{
  struct tcp_pcb *pcb;
  /*create new pcb*/
  pcb = tcp_new();
  
  if (!pcb)
  {
    return ;
  }
  /* bind PORT traffic to pcb */
  tcp_bind(pcb, IP_ADDR_ANY, 8080);
  /* start listening on port 80 */
  pcb = tcp_listen(pcb);
  /* define callback function for TCP connection setup */
  tcp_accept(pcb, Tcp_accept);
}



/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
