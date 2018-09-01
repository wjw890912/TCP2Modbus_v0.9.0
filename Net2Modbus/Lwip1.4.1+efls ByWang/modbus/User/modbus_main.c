#include <string.h>
#include "stm32f10x_it.h"
#include "usart.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "mbframe.h"
#include "main.h"

 uint32_t Tick;
 void Master_Send(char Slave_addr,char *Data,int len);
/* ----------------------- Start implementation -----------------------------*/

  #define T32_INTERVAL  (250*4)*1

	  static  uint32_t t32=0;

extern char TcpRecvBuf[1500];
extern uint32_t TcpRecvLenth;

#ifdef   RS485_MASTER_CALL
extern char UID_STM32[16];
extern uint8_t Mb2TcpBuff[256];
uint8_t TCP2RS485CMDLIST[1400]={0};	//用来接收来自服务器的轮训发送数据帧
uint32_t MasterCallTrm;
uint32_t DevIndex=0; //
#endif		  
 extern unsigned char RE_RSSI(void);
 void ModbusSenttset(uint32_t Time)
 {
 		   
#ifdef   RS485_MASTER_CALL	 
  
          char MasterA[24];//存储命令
		  static int MsaterTime=1000;//查询时间
		  static char *p,*sptr;
	      static int len=0;
		  uint32_t id;

#endif


		 if(TcpRecvLenth>0)
		 {


				   {
				      uint32_t  len;
				      len=	TcpRecvLenth; //save
					  #ifdef USED_SI4432
						if( Check_si4432_HW()==1)	
	
						{
							//无线数据包正常直接发送
							tx_data(TcpRecvBuf, len);//发送成功;//发送成功
						
						}
					#endif

						Master_Send(6,TcpRecvBuf,len);
						

				 	
				  
					 }

					  TcpRecvLenth=0; //restart data lenth 
		 }

         #ifdef RS485_MASTER_CALL
		 else
		 {
					 
									 
				if (Time - MasterCallTrm >= MsaterTime)//设置间隔1s
				  {
				
			              MasterCallTrm =  Time;
				  	
					
							   if(TCP2RS485CMDLIST[0]!=0)	//如果列表中有配置数据
							   {
								  	  //解析并发送
								  		  if(DevIndex==0)
										 {
										 sptr =   &TCP2RS485CMDLIST[0]; //取BUFF地址
										 }
					
										  p = memchr(sptr,(0x41+DevIndex),1);		   //查关键字
							
										 if((p)&&(*(p+1)=='#')) //确认位置
										 {
											
											  len =	 *(p+2); 
										 	 memcpy(MasterA,(p+3),len-4);

											

											      id=(UID->UID0_31+UID->UID32_63+UID->UID64_95);//sum ID 

												 UID_STM32[2]=0x01;         //上传数据到服务器标示
												
												 UID_STM32[3]=0x01;         //子系统Id：业务子系统的ID号 ,z暂时未用，值默认为01即可
												
												 UID_STM32[4]=id;
												 UID_STM32[5]=id>>8;
												 UID_STM32[6]=id>>16; //网关ID(3个字节，设备配置产生)												
												
												 UID_STM32[8]=0x88;					  
												 UID_STM32[9]=0x88;         //主动上报类型
												
												
												 Mb2TcpBuff[10]=0;
												 Mb2TcpBuff[11]=0;
												 Mb2TcpBuff[12]=0; //设备id(3个字节，设备涿),
												 #ifdef USED_SI4432

												if( Check_si4432_HW()==1)	
												{
											  	
												//无线数据包正常直接发送
												tx_data((char *)MasterA, len-4);//

												 }
												  #endif

											 Master_Send(6,(char *)MasterA,len-4);
											 if(DevIndex==0)
											 {
											 //只使用第一个携带的时间值作为轮训时基
											 MsaterTime = (*((p+3+len)-4)<<24)+(*((p+3+len)-3)<<16)+(*((p+3+len)-2)<<8)+(*((p+3+len)-1)<<0);
											 
											 if(MsaterTime<1)
											 {
											 	 //如果小于1MS
											 MsaterTime=1;//最小1ms
											 }
											 
											 }
											 sptr =  p+3+len;//下一帧
					
											   DevIndex++;  //下一个设备
					
										 }
										 else
										 {
										 	DevIndex=0;//没有数据了重新开始break;
										 }
					
										
							
										 if(DevIndex==255)DevIndex=0;//最大上报数据设备15个设备 改为254
					
					
								}
							


					 }//END IF 

							   		 
		 }
		 #endif
		 
		 
		/*存储服务器下发的从机设备列表 
		【6个指令一个类型，这里最大存储30个，包括间隔30字节和定时数据2字节】
		  [Start]
		  [1] :["A#"]+[长度（命令字节数和时间值4字节）]+[M O D B U S - R T U ] + [MS高位在前低位在后]
		  [2] :["B#"]+[长度（命令字节数和时间值4字节）]+[M O D B U S - R T U ] + [MS高位在前低位在后]
		  [.] :[".."]+[  ..................] + [..................]
		  [16]:["P#"]+[长度（命令字节数和时间值4字节）]+[M O D B U S - R T U ] + [MS高位在前低位在后] 
		  [End]


          char MasterA[24];//存储命令
		  int MsaterTime;//查询时间
		  char *p,*sptr;
	      int len,i;
		  char test[]=
		  {

			0x41,0x23, 0x0C,   0xC8, 0x03, 0x00, 0x02, 0x00, 0x01, 0x34, 0x53,  0x01,0x02,0x03,0xe8, 
			 
			0x42,0x23, 0x0C,   0xC9 ,0x03, 0x00, 0x02, 0x00, 0x01, 0x35, 0x82,  0x00,0x00,0x03,0xe8, 
			 
			0x43,0x23, 0x0C,   0xCA ,0x03, 0x00, 0x02, 0x00, 0x01, 0x35, 0xB1,  0x00,0x00,0x03,0xe8,  

			0x44,0x23, 0x0C,   0xCB ,0x03, 0x00, 0x02, 0x00, 0x01, 0x34, 0x60,  0x01,0x02,0x03,0xe8,  

			0x45,0x23, 0x0C,   0xCC ,0x03, 0x00, 0x02, 0x00, 0x01, 0x35, 0xD7,  0x00,0x00,0x03,0xe8,  

			0x00,0x00, 0x00,   0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00,0x00,0x00,0x00  
		  };
		  
	41 23   0F  01 10 00 00 00 01 02 00 01 67 90   00 00 00 00 
	42 23   0F  01 10 00 01 00 01 02 00 01 66 41   00 00 00 00 
	43 23   0F  01 10 00 02 00 01 02 00 01 66 72   00 00 00 00 
	44 23   0F  01 10 00 03 00 01 02 00 01 67 A3   00 00 00 00 




			  sptr =   &test[0]; //区BUFF地址

			  for(i=0;i<16;i++)
			  {


					  p = memchr(sptr,(0x41+i),1);		   //查关键字
		
					 if((p)&&(*(p+1)=='#')) //确认位置
					 {
						
						  len =	 *(p+2); 
					 	 memcpy(MasterA,(p+3),len-4);
						 MsaterTime = (*((p+3+len)-4)<<24)+(*((p+3+len)-3)<<16)+(*((p+3+len)-2)<<8)+(*((p+3+len)-1)<<0);

						 sptr =  p+3+len;//下一帧
					 }
					 else
					 {
					 	break;
					 }


					
			  }




		   
		 */ 

 			   #if 0
 	 		   char  usedata[10] = "stm32f107";
		if (Time - t32 >= T32_INTERVAL)
		  {
		    t32 =  Time;
		
		    
			 	Master_Send(6,(char *)&usedata[0],9);
			//Master_Send(6,"rs485 master sent data ",sizeof("rs485 master sent data "));
		
		
		
		  }
 		   #endif
 
 }


int
modbus_main( void )
{
  char  usedata[10] = { 3, 0, 0, 0,0xa};
    eMBErrorCode    eStatus;
	// char *p;

	/* init system setting */
//	SystemInit();
	/*NVIC init*/
//	NVIC_Configuration();
	/* init sysTick*/
  // SysTick_Configuration();
	 /* init USART*/
	 Hw_Usart_init();
	
	 while(1)
	 {

    eStatus = eMBMasterInit( MB_RTU, 2, 9600,MB_PAR_NONE );

   // eStatus = eMBSetSlaveID( 0x34, TRUE, ucSlaveID, 3 );
 
    /* Enable the Modbus Protocol Stack. */
    eStatus = eMBMasterEnable();

	/*	vMBMasterSetDestAddress(6);
		 vMBMasterGetPDUSndBuf( &p );
		 *p=3;
		 p++;
		 *p=0;
		  p++;
		 *p=0x0;
		  p++;
		 *p=0;
		  p++;
		 *p=5;


		  */
		    for( ;; )
		    {
				 	if(Tick>=20)
			 	{
					Master_Send(6,(char *)&usedata[0],5);
			/*	vMBMasterGetPDUSndBuf( &p );
				 eMBMasterRTUSend( 6, p,5 );  */
				 Tick=0;
				}
		
		        (void)eMBMasterPoll();
		    
		    }
      }

 }


void Master_Send(char Slave_addr,char *Data,int len)
{ 

	//Slave_addr无实际意义，因为在底层发送函数中我已经屏蔽掉了这个ID，所有数据以透传方式传输 所以不用关心这个字段。


         char *p ;
		 int i;

		  if(len>MB_PDU_SIZE_MAX)return ;// if the data lenth up the PDU  lenth we need stop it .

		  //vMBMasterSetDestAddress(Slave_addr);// we dont need slave addrs
		  vMBMasterGetPDUSndBuf( &p );   //The 1st  we get the pdu buffer point
			 for(i=0;i<len;i++)
			 {
			 	 *p=*Data;
				 p++;				     //The 2st we fill in data to the buffer .
				 Data++;
			 }

		 	vMBMasterGetPDUSndBuf( &p ); // The 3st we reget the pdu snetBuffer point
		    eMBMasterRTUSend( Slave_addr, p, len );	// 	start up snet status machin and excution sent data on RTU 
}



