#include "stm32f107.h"
#include "stm32f10x_it.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "ds18b20.h"  
#include "types.h" 
#include "filesystem.h"
#include <stdio.h>
#include <string.h>
#include "httpd.h"
#include "main.h"
 //温度采集时间为1S

#ifdef MYSELFBOARD
//500ms	 一次
#define TEMP_CREATTRM_INTERVAL  /*(250*4)*2 */	(250*4)*2
#else
#define TEMP_CREATTRM_INTERVAL  /*(250*4)*2 */	(250*4)*2
#endif


#ifdef MYSELFBOARD
#define Read18b20IO() GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3) 
#else
#define Read18b20IO() GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_0) 
#endif

#define Delay_US(x)   Delay_us(x)
#define DS18B20_NUM   2    
#define MAXNUM        2  
void Set18b20IOout(void);
void Set18b20IOin(void);
extern __IO uint32_t LocalTime;
static uint8_t  fac_us=0;//us延时倍乘数
static uint16_t fac_ms=0;//ms延时倍乘数
void Init_DS18B20_IO(void)
{
   //
 #ifdef MYSELFBOARD
 //我自己的开发板的单总线口
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE , ENABLE);
  /* Configure PE3 pins */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
 #else
//我自己的板子的单总线口
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD , ENABLE);
  /* Configure PD0 pins is 1-WRITE bus*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

 #endif


} 	
void Set18b20IOin(void)
{

#ifdef MYSELFBOARD
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE , ENABLE);
  /* Configure PE3 pins */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
#else
 GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD , ENABLE);
  /* Configure PD0 pins */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOD, &GPIO_InitStructure);



#endif
}  
 void Set18b20IOout(void)
{
  #ifdef MYSELFBOARD

  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE , ENABLE);
  /* Configure PE3 pins */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
 #else

   GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD , ENABLE);
  /* Configure PD0 pins */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_Init(GPIOD, &GPIO_InitStructure);


 #endif
}            
//延时Nms
//注意Nms的范围
//Nms<=0xffffff*8/SYSCLK
//对72M条件下,Nms<=1864 
 
//初始化延迟函数
void Delay_init(uint8_t SYSCLK)
{
 SysTick->CTRL&=0xfffffffb;//选择内部时钟 HCLK/8
 fac_us=SYSCLK/8;      
 fac_ms=(u16)fac_us*1000;
} 

  void Delay_us(uint32_t Nus)
  {
 
  SysTick->LOAD=Nus*fac_us;       //时间加载      
 SysTick->CTRL|=0x01;            //开始倒数    
 while(!(SysTick->CTRL&(1<<16)));//等待时间到达 
 SysTick->CTRL=0X00000000;       //关闭计数器
 SysTick->VAL=0X00000000;        //清空计数器   
  
  }   

  void Delay_ms(uint16_t nms)
  {
   SysTick->LOAD=(u32)nms*fac_ms; //时间加载  
 SysTick->CTRL|=0x01;               //开始倒数    
 while(!(SysTick->CTRL&(1<<16)));   //等待时间到达 
 SysTick->CTRL&=0XFFFFFFFE;         //关闭计数器
 SysTick->VAL=0X00000000;           //清空计数器 
  
  }

void Write18b20IO(u8 flage) 
{

	 #ifdef MYSELFBOARD
	  if(flage)
	  {
	  GPIO_SetBits(GPIOE, GPIO_Pin_3);
	  }else
	  { 
	  GPIO_ResetBits(GPIOE, GPIO_Pin_3);
	  }
  	  #else
	  if(flage)
	  {
	  GPIO_SetBits(GPIOD, GPIO_Pin_0);
	  }else
	  { 
	  GPIO_ResetBits(GPIOD, GPIO_Pin_0);
	  }
	  #endif
}
u8 DS18B20_Reset(void)  
{  
    u8 i = 0;  
      
    Set18b20IOout();        //主机端口推挽输出模式  
    Write18b20IO(1);  
    Delay_US(1);  
    Write18b20IO(0);        //拉低总线480us~240us  
    Delay_US(500);          //>480US延时  
    Write18b20IO(1);  
    Delay_US(2);            //复位完成  
    Set18b20IOin();         //主机端口浮空输入模式  
    while(Read18b20IO())    //等待低电平应答信号  
    {  
        i ++;  
        Delay_US(1);  
        if(i > 100)  
        {  
           // uart_printf("DS18B20 error!\r\n");  
            return FALSE;   //等待超时,初始化失败,返回FALSE;  
        }  
    }  
   // Delay_US(250);          //跳过回复信号  
   Delay_US(410);          //跳过回复信号 
    return TRUE;            //检测到DS18B20,并且初始化成功  
}  
__inline u8 DS18B20_ReadBit(void)  
{  
    u8 data = 0;  
   
    Set18b20IOout();    //主机端口推挽输出模式  
    Write18b20IO(0);    //拉低总线10-15us  
    Delay_US(12);  
   Write18b20IO(1);    //释放总线  
    Set18b20IOin();     //主机端口浮空输入模式  
    Delay_US(10);  
    if(Read18b20IO())   //读取数据,读取后大约延时40-45us  
       data = 0x01;
	
    Delay_US(40); 
    return data;  
}  
u8 DS18B20_ReadData(void)  
{ 
    u8 i,data = 0;  
     __disable_irq();//  相当于 CPSID I   
    for(i = 0;i < 8;i ++)  
    {  
  
        data >>= 1;  
        if(DS18B20_ReadBit())  
            data |= 0x80;  
    }
	__enable_irq();//   相当于 CPSIE I   
    return data;  
}  
  
void DS18B20_WriteBit(u8 bit)  
{  
 
    Set18b20IOout();    //主机端口推挽输出模式  
    Write18b20IO(0);        //拉低总线10-15us  
    Delay_US(12);  
    Write18b20IO(bit & 0x01);//   //写入数据位,保持20-45us  
    Delay_US(30);  
    Write18b20IO(1);        //释放总线  
    Delay_US(5); 
   
}  
  void DS18B20_WriteData(u8 data)  
{  
    u8 i;  
    __disable_irq();//  相当于 CPSID I   
    for(i = 0;i < 8;i ++)  
    {  
        DS18B20_WriteBit(data);  
        data >>= 1;  
    } 
	__enable_irq();//   相当于 CPSIE I  
}  
s16 DS18B20_ReadTemper(void)  
{  
    u8 th, tl;  
    s16 data;  
      
    if(DS18B20_Reset() == FALSE)      
    {  
        return 0xffff;  //返回错误  
    }  
  
    DS18B20_WriteData(0xcc);    //跳过读序列号  
    DS18B20_WriteData(0x44);    //启动温度转换  
    DS18B20_Reset();  
    DS18B20_WriteData(0xcc);    //跳过读序列号  
    DS18B20_WriteData(0xbe);    //读取温度  
    tl = DS18B20_ReadData();    //读取低八位  
    th = DS18B20_ReadData();    //读取高八位  
    data = th;  
    data <<= 8;  
    data |= tl;  
    data *= 6.25;               //温度值扩大100倍，精确到2位小数  
      
    return data;  
}  
u8 DS18B20_Read2Bit(void)  
{  
    u8 i,data = 0;  
      
    for(i = 0;i < 2;i ++)  
    {  
  
        data <<= 1;  
        if(DS18B20_ReadBit())  
            data |= 1;  
    }  
    return data;  
}    
u8 DS18B20_SearchROM(u8 (*pID)[8],u8 Num)  
{   
    unsigned char k,l,chongtuwei,m,n;  
    unsigned char zhan[(MAXNUM-1)];  
    unsigned char ss[64];  
    unsigned char s=0; 
	u8 num = 0; 
    l=0;  
      
    do  
    {  
	 
        DS18B20_Reset();  
        DS18B20_WriteData(0xf0);      
        for(m=0;m<8;m++)  
        {  
            s=0;  
            for(n=0;n<8;n++)  
            {  
                k=DS18B20_Read2Bit();//读两位数据  
                k=k&0x03;  
                s>>=1;  
                if(k==0x01)//01读到的数据为0 写0 此位为0的器件响应  
                {             
                    DS18B20_WriteBit (0);  
                    ss[(m*8+n)]=0;  
                }  
                else if(k==0x02)//读到的数据为1 写1 此位为1的器件响应  
                {  
                    s=s|0x80;  
                    DS18B20_WriteBit (1);  
                    ss[(m*8+n)]=1;  
                }  
                else if(k==0x00)//读到的数据为00 有冲突位 判断冲突位   
                {               //如果冲突位大于栈顶写0 小于栈顶写以前数据 等于栈顶写1  
                    chongtuwei=m*8+n+1;                   
                    if(chongtuwei>zhan[l])  
                    {                         
                        DS18B20_WriteBit (0);  
                        ss[(m*8+n)]=0;                                                
                        zhan[++l]=chongtuwei;                         
                    }  
                    else if(chongtuwei<zhan[l])  
                    {  
                        s=s|((ss[(m*8+n)]&0x01)<<7);  
                        DS18B20_WriteBit (ss[(m*8+n)]);  
                    }  
                    else if(chongtuwei==zhan[l])  
                    {  
                        s=s|0x80;  
                        DS18B20_WriteBit (1);  
                        ss[(m*8+n)]=1;  
                        l=l-1;  
                    }
				  
                }  
                else  
                {  
                    return num; //搜索完成,//返回搜索到的个数  
                }  
            }  
            pID[num][m]=s;        
        }  
        num=num+1;  
    }  
    while(zhan[l]!=0&&(num<MAXNUM));   
      
    return num;     //返回搜索到的个数  
}  
  s16 DS18B20_ReadDesignateTemper( u8 pID[8])  
{  
    u8 th, tl;  
    s16 data;  
      
    if(DS18B20_Reset() == FALSE)      
    {  
        return 0xffff;              //返回错误  
    }  
  
    DS18B20_WriteData(0xcc);        //跳过读序列号  
    DS18B20_WriteData(0x44);        //启动温度转换  
    DS18B20_Reset();  
    DS18B20_WriteData(0x55);        //发送序列号匹配命令  
    for(data = 0;data < 8;data ++)   //发送8byte的序列号     
    {  
       DS18B20_WriteData(pID[data]);  
    }  
    Delay_US(10);  
    DS18B20_WriteData(0xbe);    //读取温度  
    tl = DS18B20_ReadData();    //读取低八位  
    th = DS18B20_ReadData();    //读取高八位  

    data = th;  
    data <<= 8;  
    data |= tl;  
    data *= 6.25;               //温度值扩大100倍，精确到2位小数  
      
    return data;  
} 
extern int OWFirst(void);
extern int  OWSearch(void);
extern unsigned char* GetRomAddr(void);
u32 time_run=0;
u8  temp[33];
u8 ID_Buff[DS18B20_NUM][8];//={{0x28,0xb0,0x4a,0x24,0x07,0x00,0x00,0x49},{0x28,0x84,0x17,0x24,0x07,0x00,0x00,0x01}};  
void DS1820main(void)  
{   
static u8 fistsech=0;
    s16 temp1,temp2;
    //u8 buff[16];  
  //  u8 i,j,num=DS18B20_NUM;  

  
   
	
		if(fistsech==0)
		{
		Delay_init(72);//72M
		Init_DS18B20_IO();
		if(1==OWFirst())	//第一次搜索到ROM数据
		{
		  memcpy(&ID_Buff[0][0],GetRomAddr(),8);//复制ID数据到IDbuff
		  
  		 for(temp1=1;temp1<DS18B20_NUM;temp1++)
		  {
		    
			if( OWSearch()==1)
			{
			 memcpy(&ID_Buff[temp1][0],GetRomAddr(),8);//复制ID数据到IDbuff
			}
			else
			{
			break;//没有器件
			} 

		  }
		}
		else{
		
		return;
		}


		temp[0]= '\t'; 

		temp[1]='T';
		temp[2]='i';
		temp[3]='m';
		temp[4]='e';
		temp[5]=':';
		temp[11]= '\t'; 

		temp[12]='T';
		temp[13]='e';
		temp[14]='m';
		temp[15]='p';
		temp[16]=':';

		temp[23]='\t';
		temp[20]='.' ;

		temp[27]='.' ;
		temp[30]= '\t';

		temp[31]= '\r';
		temp[32]= '\n';

		fistsech=1;
		}

  		//读取2个指定ID的温度传感器数值
		// tempA是在顶部没有划的，tempB顶部划了一下:-)
        temp1 = DS18B20_ReadDesignateTemper(ID_Buff[0]);  
   		temp2 = DS18B20_ReadDesignateTemper(ID_Buff[1]);
	  
	
		
		temp[6]=time_run/10000+0x30;
		temp[7]=time_run%10000/1000+0x30;
		temp[8]=time_run%10000%1000/100+0x30;
		temp[9]=time_run%10000%1000%100/10+0x30;
		temp[10]=time_run%10000%1000%100%10+0x30;
	
		
		
		temp[17]= temp1/10000+0x30;
		temp[18]= temp1%10000/1000+0x30;
		temp[19]= temp1%10000%1000/100+0x30;
	
		temp[21]= temp1%10000%1000%100/10+0x30;
		temp[22]= temp1%10000%1000%100%10+0x30;

		

		temp[24]=temp2/10000+0x30;
		temp[25]= temp2%10000/1000+0x30;
		temp[26]= temp2%10000%1000/100+0x30;
	
		temp[28]= temp2%10000%1000%100/10+0x30;
		temp[29]= temp2%10000%1000%100%10+0x30;
		
		


}
 #ifdef USED_SMTP
extern void my_smtp_test(char *str);
#endif	
uint32_t TempCreatTrm=0;
uint16_t Day=0,Count_mail=0;
uint8_t Assic[6]={0};
char mailsentbox[18];
void TemperatureThread(uint32_t Time)
{
	

	  
	  time_run=Time/1000;
		if(time_run==86400)//24h
		{

		Day++;
		Assic[0]='\n';
		Assic[1]=Day/100+0x30;
		Assic[2]=Day%100/10+0x30;
		Assic[3]=Day%100%10+0x30;
		Assic[4]=Day%100%10+0x30;
		Assic[5]='\n';
		time_run=0;
		FS_Write(2,(char*)Assic,6);
		}
   			 
if (Time - TempCreatTrm >= TEMP_CREATTRM_INTERVAL)
  {


    TempCreatTrm =  Time;

	DS1820main();//采集一次温度数据存入TEMPA和B中已经转换为ASCII了直接写入文件系统中即可。。
  	#ifdef USED_SMTP
	  memcpy(&mailsentbox[0],&temp[6],5);//复制ID数据到IDbuff

	  memcpy(&mailsentbox[5],&temp[17],6);//复制ID数据到IDbuff

	  memcpy(&mailsentbox[11],&temp[24],6);//复制ID数据到IDbuff
	  mailsentbox[17] ='\0';
	#endif

	#ifndef TEMP_WRITE_HTML
    FS_Write(2,(char*)temp,33);
	#else
	FS_Writehtml((char*)temp,33);
	#endif
   #ifdef USED_SMTP
	Count_mail++;
	 #ifdef MYSELFBOARD
	if(Count_mail>=5)//90-3分钟发一封邮件
	#else
	if(Count_mail>=90)//90-3分钟发一封邮件
	#endif
	{
	Count_mail=0;
	my_smtp_test(mailsentbox); 
	}
   #endif

  }

}

