


QQ:925295580
e-mail:925295580@qq.com
Time:201512
author:王均伟



       SW

   beta---(V0.1)


1、TCP/IP基础协议栈
.支持UDP
.支持TCP
.支持ICMP

2、超轻量级efs文件系统
.支持unix标准文件API
.支持SDHC标准。兼容V1.0和V2.0大容量SDK卡-16G卡无压力。（驱动部分参考开源 :-)）
.超低内存占用，仅用一个512字节的高速缓存来对文集遍历。

3、支持1-write DS18B20 	温度传感器
.支持单总线严格时序
.支持ROM搜索，遍历树叶子，允许一条总线挂接多个温度传感器
.数据自动转换为HTML文件

4、TCP/IP应用
.支持TFTP服务端，可以完成文件的下载任务。（此部分来自GITHUB，增加部分TIMEOUT 事件）tftp -i
.支持NETBIOS服务。
.支持一个TCP服务器，本地端口8080.	测试服务
.支持一个TCP客户端，本地端口40096	远端端口2301 远端IP192.168.0.163	用来做数据交互
.支持一个UDP广播，  本地端口02	    广播地址255.255.255.255	  用来把温度采集的数据发广播
.支持一个HTTP服务器 本地端口80  	http:ip  访问之	  关联2只18B20温度传感器显示在简单的SD卡的网页上

5、系统编译后
Program Size: Code=51264 RO-data=28056 RW-data=1712 ZI-data=55048  

6、网络配置

ipaddr：192, 168, 0, 253-209
netmask：255, 255, 0, 0
gw：192, 168, 0, 20
macaddress：xxxxxxxxxx


tks for GitHUB

	    HW

 stm32f107+DP83848CVV+ds18B20*2+SDHC card (4GB)

7、修改了一个SDcrad的BUG，有一个判断语句写错了，fix后可以支持2GB和4GB以上的两种卡片了。20160122
8、增加了一个宏在main.h中
#define MYSELFBOARD
如果定义了，那么表示使用李伟给我的开发板
如果不定义就选择我自己画的那块板子。
没有什么本质区别，框架一样，只是IO口稍有改动。20160122


 20160123
9、增加一个SMTP应用，可以通过定义USED_SMTP来使能 。
10、完成smtp.c的移植和测试。可以向我的邮箱发送邮件。邮箱需要设置关闭SSL。并且需要修改一下源码 的几个宏定义。
11、把采集的温度数据20分钟发一封邮件到我自己的邮箱中。完成。  必须用通过认真的IP否则过不去防火墙

12、调整了一下发邮箱的时间为5分钟一封邮件，@163邮箱的限制大约是300封邮件


20160402

13、测试中发现有内存泄露的情况,通过增加内存的信息通过TCP输出的 2301端口debug后发现
  异常的如下
 总内存数（字节）：6144
 已用内存（字节）：5816
 剩余内存数（字节）：328
 使用标示：1
 正常的如下
 总内存数（字节）：6144
 已用内存（字节）：20
 剩余内存数（字节）：6124
 使用标示：1
 显然memalloc使内存溢出查找代码因为除了SMTP应用程序使用malloc外其他不具有使用的情况。
 所以肯定是SMTP出问题
 进一步分析代码为SMTP的smtp_send_mail（）中
 smtp_send_mail_alloced(struct smtp_session *s)
 函数使用的
 s = (struct smtp_session *)SMTP_STATE_MALLOC((mem_size_t)mem_len);
 分配了一块内存没有事正常的释放。
 这样反复
 几次最终导致这块应用代码不能正常返回一块完整的 mem_le大小的内存块而一直保留了328字节的剩余内存。
 这最终导致了所有依赖mem的应用程序全部获取不到足够的内存块。而出现的内存溢出。
 继续分析 释放的内存句柄  (struct smtp_session *) s
 发现几处问题

 1）非正常中止	“风险”
   if (smtp_verify(s->to, s->to_len, 0) != ERR_OK) {
    return ERR_ARG;
  }
  if (smtp_verify(s->from, s->from_len, 0) != ERR_OK) {
    return ERR_ARG;
  }
  if (smtp_verify(s->subject, s->subject_len, 0) != ERR_OK) {
    return ERR_ARG;
  }
  由于没有对  smtp_send_mail_alloced 函数进行判断所以如果此处返回会造成函数不能正常中止
  也就会导致 (struct smtp_session *) s	没有机会释放（因为在不正常中止时是在后面处理的）
  但是考虑到源数据是固定的从片上flash中取得的，这种几率几乎没有。但是存在风险。所以统一改为
  if (smtp_verify(s->to, s->to_len, 0) != ERR_OK) {
    	 err = ERR_ARG;
     goto leave;
  }
  if (smtp_verify(s->from, s->from_len, 0) != ERR_OK) {
    	 err = ERR_ARG;
     goto leave;
  }
  if (smtp_verify(s->subject, s->subject_len, 0) != ERR_OK) {
    	 err = ERR_ARG;
     goto leave;
  }

  2）、非正常TCP连接，主要原因。
  原来的函数为：
  if(tcp_bind(pcb, IP_ADDR_ANY, SMTP_PORT)!=ERR_OK)
	{
	return	ERR_USEl;
  	   
	}
  显然还是同样的会造成malloc 分配了但是没有被调用，修改为
  if(tcp_bind(pcb, IP_ADDR_ANY,SMTP_PORT)!=ERR_OK)
  {
	err = ERR_USE;
    goto leave;		   
  }

   这样	  leave中就会自动处理释放掉这个非正常中止的而造成的内存的溢出问题。
   leave:
  smtp_free_struct(s);
  return err;

 归根结底是一个问题。那就是必须保证malloc 和free 成对出现。



 14、NETBIOS 名字服务增加在lwipopts.h中增加
 #define NETBIOS_LWIP_NAME "WJW-BOARD"
 正确的名称
 这样可以使用如下格式找到板子的IP地址
 ping 	wjw-board 
 而不用指定IP地址
 20160410



 /*测试中发现长时间运行后SMTP还有停止不发的情况，内存的问题上面已经解决，下面尝试修改进行解决，并继续测试-见17条*/
  20160427

 15、修改SNMTP的timout超时时间统一为2分钟，因为我的邮件重发时间为3分钟。默认的10分钟太长。先修改之。不是他影响的。fix

 /** TCP poll timeout while sending message body, reset after every
 * successful write. 3 minutes def:(3 * 60 * SMTP_POLL_INTERVAL / 2)*/
#define SMTP_TIMEOUT_DATABLOCK  ( 1 * 60 * SMTP_POLL_INTERVAL / 2)
/** TCP poll timeout while waiting for confirmation after sending the body.
 * 10 minutes def:(10 * 60 * SMTP_POLL_INTERVAL / 2)*/
#define SMTP_TIMEOUT_DATATERM   (1 * 60 * SMTP_POLL_INTERVAL / 2)
/** TCP poll timeout while not sending the body.
 * This is somewhat lower than the RFC states (5 minutes for initial, MAIL
 * and RCPT) but still OK for us here.
 * 2 minutes def:( 2 * 60 * SMTP_POLL_INTERVAL / 2)*/
#define SMTP_TIMEOUT            ( 1 * 60 * SMTP_POLL_INTERVAL / 2)
 20160427
 
  16、增加监控SMTP TCP 部分的变量数组
   smtp_Tcp_count[0]//tcp new count 
    smtp_Tcp_count[1]//bind count
	 smtp_Tcp_count[2]connect count 
	 smtp_Tcp_count[3]bind fail save the all pcb list index number
	 that all use debug long time running on smtp .
 20160427

17、发现不是SMTP的问题似乎邮箱出问题了，重新修改以上15条参数全部为2分钟 ，30*4*0.5S=1MIN *2=2min 

/** TCP poll interval. Unit is 0.5 sec. */
#define SMTP_POLL_INTERVAL      4
/** TCP poll timeout while sending message body, reset after every
 * successful write. 3 minutes def:(3 * 60 * SMTP_POLL_INTERVAL / 2)*/
#define SMTP_TIMEOUT_DATABLOCK  30*2
/** TCP poll timeout while waiting for confirmation after sending the body.
 * 10 minutes def:(10 * 60 * SMTP_POLL_INTERVAL / 2)*/
#define SMTP_TIMEOUT_DATATERM   30*2
/** TCP poll timeout while not sending the body.
 * This is somewhat lower than the RFC states (5 minutes for initial, MAIL
 * and RCPT) but still OK for us here.
 * 2 minutes def:( 2 * 60 * SMTP_POLL_INTERVAL / 2)*/
#define SMTP_TIMEOUT
20160429            30*2
18、加长了KEEPALIVBE时间为
	pcb->so_options |= SOF_KEEPALIVE;
   pcb->keep_idle = 1500+150;// ms
    pcb->keep_intvl = 1500+150;// ms
   pcb->keep_cnt = 2;// report error after 2 KA without response
 20160429


19、增加几个SMTP结果的变量	smtp_Tcp_count[10]	upsize 10 dword

20、增加监控SMTP TCP 部分的变量数组
   smtp_Tcp_count[0]//tcp new count 
    smtp_Tcp_count[1]//bind count
	 smtp_Tcp_count[2]connect count 
	 smtp_Tcp_count[3]bind fail save the all pcb list index number
add smtp send result 
              smtp_Tcp_count[4]|= (smtp_result);  
			  smtp_Tcp_count[4]|= (srv_err<<8);
			  smtp_Tcp_count[4]|= (err<<24);

			  if(err==ERR_OK){smtp_Tcp_count[5]++;}	//smtp成功次数统计

			   if(arg!=(void*)0)
			   {
				smtp_Tcp_count[6]=0xAAAAAAAA ;	 //有参数
			   }
			   else
			   {
			   
			   smtp_Tcp_count[6]=0x55555555 ;	//没有参数
			   }
20160430
21、

 以上测试中发现运行到9天左右就会不再执行SMTP代码返回数据如下： 低字节在前--高字节在后
 【Receive from 192.168.0.253 : 40096】：
5D 11 00 00     5D 11 00 00    58 11 00 00    00 00 00 00     04 00 00 F6  48 11 00 00  55 55 55 55 

上面的数据可知：
tcp new count=0x115d
bind count=0x115d
connect count=0x1158
bind fail  number=0
smtp_result=  4（SMTP_RESULT_ERR_CLOSED）
srv_err=00
tcp err=0xf6是负数需要NOT +1=(-10) 错误代码为  ERR_ABRT	  Connection aborted

以上数据定格，不在变化，说明这个和TCP baind 没有关系，是TCP new和之前的调用问题，所以继续锁定这个问题找。

20160513

22、把速度调快，10秒钟一次SMTP 连接。修改SMTP 应用程序的超时时间为8秒钟同时增加
smtp_Tcp_count[8]++;来计数总的调用SMTP的次数
smtp_Tcp_count[7]++;来计数SMTP 用的TCP new之前的次数。排除一下TCP new的问题！
如果这个变量一直变化而后面的没有变化这证明TCP —new出错。反之再向前推，直到调用它的地方一点点排除。

继续追踪这个停止TCP 连接的问题。
20160513


 /********为了接口陈新的485而做*******************/

23、增加modbus RTU 主机部分底层TXRX的代码，打算使用RTU 和TCP 做成透传485.这边不处理，只转发。
定义了一个宏

//定义了则使用MODBUS RTU TX/RX底层收发 (注意应用层没有使用。因为应用层打算交给服务器做，这边仅仅做RTU透传)
#define USED_MODBUS

18:52调试TX通过。更换了TIM3和USART2的Remap

20160613

24、更新STM32的固件库，使用2011版本的，原因是原来的2009版本的CL系列的串口驱动有问题。波特率不正常。换为2011的正常了，
    MODBUS RTU的流程做了修改。发送屏蔽掉CRC校验的产生，。直接透传。
	注意是MODBUS 这个串口从软件上看也是半双工的	。
20160614

25、上面的24条问题最终结果是晶振问题导致的，和固件库没有关系。


#if !defined  HSE_VALUE
 #ifdef STM32F10X_CL   
  #define HSE_VALUE    ((uint32_t)8000000) /*!< Value of the External oscillator in Hz */
 #else 
  #define HSE_VALUE    ((uint32_t)8000000) /*!< Value of the External oscillator in Hz */
 #endif /* STM32F10X_CL */
#endif /* HSE_VALUE */


  这里	 #define HSE_VALUE    ((uint32_t)8000000) /*!< Value of the External oscillator in Hz */
  要定义为你自己的外部晶振值。
20160615

26、增加了服务器接口和陈新，这一版要准备用在大鹏数据采集上做网管，所以定了简答的协议，这边直观转发。

 IP4_ADDR(&ip_addr,114,215,155,179 );//陈新服务器

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

 服务器发送：FF AA + 类型 +【modbus RTU 数据域 】+\r\n
 网关回复  ：FF 55 + 类型 +【modbus RTU 数据域 】+\r\n

*/

 27、根据服务器要求修改网关的上报数据

 getid 命令改为

 AB +00 +[三字节ID]+CD +'$'+'\r'+'\n'   9个字节

 修改上报数据位

 FF 55 + 类型（1字节）+ID（3字节）+[modbus-RTU数据域] + '$'+'\r'+'\n'

20180710

28、开启DHCP 
20160710

29、修改了服务器接口和主要是修改了IP地址其他不变。
 IP4_ADDR(&ip_addr,60,211,192,14);//济宁大鹏客户的IP服务器地址。
20160920


30、在stm32f10x.h中增加UID的地址映射，以便直接获取ID号

 typedef struct
 {
 	uint32_t UID0_31;
 	uint32_t UID32_63;
	uint32_t UID64_96;
 }ID_TypeDef;
#define UID_BASE              ((uint32_t)0x1FFFF7E8) /*uid 96 by wang jun wei */
#define UID    ((ID_TypeDef*) UID_BASE)
20160920
31、修改ID从CPU的UID中运算获得
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
 20160920
32、增加MAC地址从芯片ID获取。
 void GetMacAddr(void)
{

uint32_t cid;
cid=(UID->UID0_31+UID->UID32_63+UID->UID64_95);//sum ID
macaddress[0]=0x00;
macaddress[1]=0x26;
macaddress[2]=cid;
macaddress[3]=cid>>8;
macaddress[4]=cid>>16;
macaddress[5]=cid>>24;

}
20160921
33、修复MODBUS上报的错误ID

	 Mb2TcpBuff[0]=0xFF;
	 Mb2TcpBuff[1]=0x55;
	 Mb2TcpBuff[2]=Statues_MB;
	 Mb2TcpBuff[3]=UID_STM32[2];
	 Mb2TcpBuff[4]=UID_STM32[3];
	 Mb2TcpBuff[5]=UID_STM32[4];
	 memcpy(&Mb2TcpBuff[6],ucMBFrame,Mb2TcpLenth);//数据复制进TCP发送buf中
	 Mb2TcpBuff[Mb2TcpLenth+6]=0x24;
	 Mb2TcpBuff[Mb2TcpLenth+6+1]=0x0d;
	 Mb2TcpBuff[Mb2TcpLenth+6+2]=0x0a;	
20160921 


###########################################################################################

     新协议

##########################################################################################

34、在陈新新平台上修改通讯协议为

帧头/通信方向/应用类/网关ID/设备分类/设备类型/设备id/数据/帧尾/间隔符

帧头：2字节，FFAA
通信方向：1字节，01网关到服务器、02服务器到网关
子系统Id：业务子系统的ID号
网关ID：3个字节，设备配置产生
设备分类：1个字节 01网关、02设备、03直连 写死
设备类型：2个字节、设备配置产生
设备id：3个字节、设备配置产生，直连设备id用三个字节
数据：MODBUS-RTU、自定义
帧尾：2字节 AAFF
间隔符：3字节 240D0A



  0-1+FFAA (帧头)
  2+通信方向(01:网关到服务器02:服务器到网关)
  3+子系统Id：业务子系统的ID号
  456+网关ID(3个字节，设备配置产生)
  7+设备分类（1个字节 01网关、02设备、03直连 写死）
  89+设备类型（2个字节、设备配置产生）
  101112+设备id（3个字节、设备配置产生，直连设备id用三个字节）
  13-n+数据：MODBUS-RTU、自定义
  n+1+2+帧尾：2字节 AAFF
  n+3+4+5间隔符：3字节 240D0A

  网关ID为010203的服务器到网关协议为
  FF AA + 02 + 01 +  01  02 03 + 01 + 00 00 +  00   00   00 + MODBUS-RTU	 +	AA    FF +   24     0D     0A
  [0][1]  [2]  [3]	[4] [5] [6]	 [7]  [8][9]  [10] [11]	[12]  [13]~[n]			[n+1] [n+2]	 [n+3] [n+3] [n+4]







  网关ID为010203的网关到服务器协议为
  FF AA +   01 + 01  +   01  02 03    +   01    + 00 00     +  00   00   00     + MODBUS-RTU	 +	 AA    FF +   24     0D     0A
  [0][1]   [2]  [3]	    [4] [5] [6]	      [7]     [8][9]       [10] [11][12]      [13]~[n]			[n+1] [n+2]	 [n+3] [n+4] [n+5]

   FFAA       [0][1] 
   01	      [2]
   01 	      [3]
   01 02 03	  [4][5][6]       
   01         [7]
   00 00      [8][9]
   000000     [10][11][12] 
   MODBUS-RTU [13]~[n]
   AAFF 	  [n+1][n+2]
   240D0A	  [n+3][n+4][n+5]







 Mb2TcpLenth=usLength;//更新接受到的数据长度，准备启动TCP发送
 Mb2TcpBuff[0]=0xFF;
 Mb2TcpBuff[1]=0xAA;          //帧头

 Mb2TcpBuff[2]=0x01;         //上传数据到服务器标示

 Mb2TcpBuff[3]=0x01;         //子系统Id：业务子系统的ID号 ,z暂时未用，值默认为01即可


 Mb2TcpBuff[4]=UID_STM32[2];
 Mb2TcpBuff[5]=UID_STM32[3];
 Mb2TcpBuff[6]=UID_STM32[4]; //网关ID(3个字节，设备配置产生)

 Mb2TcpBuff[7]=0x01;         //设备分类（1个字节 01网关、02设备、03直连 写死）


 Mb2TcpBuff[8]=0x01;					  
 Mb2TcpBuff[9]=0x01;         //设备类型（2个字节、设备配置产生）


 Mb2TcpBuff[10]=UID_STM32[2];
 Mb2TcpBuff[11]=UID_STM32[3];
 Mb2TcpBuff[12]=UID_STM32[4]; //设备id(3个字节，设备配置产生),

memcpy(&Mb2TcpBuff[13],ucMBFrame,Mb2TcpLenth);//MODBUS-RTU数据复制进TCP发送buf中


 Mb2TcpBuff[Mb2TcpLenth+13+0]=0xAA;
 Mb2TcpBuff[Mb2TcpLenth+13+1]=0xFF;	 //帧尾

 Mb2TcpBuff[Mb2TcpLenth+13+2]=0x24;	
 Mb2TcpBuff[Mb2TcpLenth+13+3]=0x0D;
 Mb2TcpBuff[Mb2TcpLenth+13+4]=0x0A;	//间隔符	
  
 Mb2TcpLenth+=18;//计算帧总长度，最终作为发送标示发送到TCP buf


     	     	                        			 	 





  
帧头/通信方向/应用类/网关ID/设备分类/设备类型/设备id/数据/帧尾/间隔符

帧头：2字节，FFAA
通信方向：1字节，01网关到服务器、02服务器到网关
子系统Id：业务子系统的ID号
网关ID：3个字节，设备配置产生
设备分类：1个字节 01网关、02设备、03直连 写死
设备类型：2个字节、设备配置产生
设备id：3个字节、设备配置产生，直连设备id用三个字节
数据：MODBUS-RTU、自定义
帧尾：2字节 AAFF
间隔符：3字节 240D0A

直连设备协议中，网关id部分是000000

网关id 
FFAA(帧头2字节) 01(通信方向1字节) 01(子系统Id1字节) 000001(网关ID3字节) 01(设备分类1字节) 0001(设备类型2字节) 000000(设备id 3字节) 00(数据N字节) AAFF(帧尾2字节) 240D0A(间隔符3字节) 

FFAA 01 01 000002 01 0001 000000 00 AAFF 240D0A 

网关上报设备信息

FFAA 01 01 000001 02 0001 000001 FE030605DC09EC00C876F8 AAFF 240D0A
FFAA 01 01 000001 02 0001 000001 FE030605DC09EC00C876F8 AAFF 240D0A
FFAA 01 01 000001 02 0001 000002 FF030605DC09EC00C876F8 AAFF 240D0A
FFAA 01 01 000001 02 0001 000003 FE030605DC09EC00C876F8 AAFF 240D0A

直连
FFAA 01 01 000000 03 0001 000001 FE030605DC09EC00C876F8 AAFF 240D0A
FFAA 01 01 000000 03 0001 000002 FE030605DC09EC00C876F8 AAFF 240D0A
FFAA 01 01 000000 03 0001 000003 FE030605DC09EC00C876F8 AAFF 240D0A


getid
FF AA 01 01 9D A1 1A 01 00 01 9D A1 1A 00 AA FF 24 0D 0A


000000-Tx:01 03 00 00 00 0E C4 0E
000002-Tx:03 03 00 00 00 0E C5 EC


 FFAA 01 01 000001 02 0001 000003 01030000000EC40E AAFF 240D0A

网关上报数据：

FF AA 01 01 00 00 00 01 01 01 00 00 00 1C FC 00 00 00 00 E0 00 00 00 E0 00 AA FF 24 0D 0A



35、修改keepalive时间为1000MS 500ms间隔太小，掉线太频繁。	  20170914

36、修改上层统一下发数据报文截取长度为前个字节，上报不变，getid 不变

上层统一下发： FFAA 02 011000000001020000A650 AAFF 240D0A

解析这个。修改代码为：

	char *ptr=(char*)p->payload;
	uint16_t len;
	ptr+=3;	//jump FF AA and other there the ptr is point to MODBUS-RTU   
	if(p->tot_len<1500) //asseter the buffer lenth 
	{
	
	TcpRecvLenth=p->tot_len-5-3;//upadta the recive data lenth	 AA FF  24 0D 0A =5 BYTES
							   
	memcpy(TcpRecvBuf,ptr,TcpRecvLenth); // data copy to appliction data buffer 



 例子：
 TCP SEVER :FFAA 02 01030000000EC40E AAFF 240D0A
 REPO: FF AA 00 00 00 00 00 02 00 00 00 00 00 01 03 1C 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 01 00 01 00 01 1F 27 AA FF 24 0D 0A  

  20170915


 37、更新协议
 通用协议解析
  
-------------------网关、直连设备协议-------------------
帧头/通信方向/应用类/网关ID/设备分类/设备类型/设备id/数据/帧尾/间隔符
 
帧头：2字节，FFAA
通信方向：1字节，01网关到服务器、02服务器到网关
子系统Id：1字节，业务子系统的ID号
网关ID：3个字节，设备配置产生
设备分类：1个字节 01网关(网关上报id时用01)、02设备(网关上报设备数据用02)、03直连 写死
设备类型：2个字节、设备配置产生
设备id：3个字节、设备配置产生，直连设备id用三个字节
数据：MODBUS-RTU、自定义
帧尾：2字节 AAFF
间隔符：3字节 240D0A

直连设备协议中，网关id部分是000000
网关id 
FFAA 01 01 000001 01 0001 000000(设备id用六个零) 00（数据用00代替） AAFF 240D0A 
网关上报设备信息
FFAA 01 01 000001 02 0001 000001 FE030605DC09EC00C876F8 AAFF 240D0A
直连
FFAA 01 01 000000 03 0001 000001 FE030605DC09EC00C876F8 AAFF 240D0A

 
 -------------------客户端ID协议-------------------
帧头：2字节，FFAA
通信方向：1字节，01网关到服务器、02服务器到网关
子系统Id：1字节 业务子系统的ID号
设备类型：1字节 网关还是直连设备00,01
app是否初次连接服务：00初次连接app需要将app对应的用户账号对应的所有网关id一次发送过来，01发送信息
Client Id：3个字节,网页是五个字节
Client 类型：01 android、02 ios、03 web
网关or直连设备的：id，初次建立连接没有，发送消息时有
数据：MODBUS-RTU、自定义，初次建立连接为00
帧尾：2字节 AAFF
间隔符：3字节 240D0A
App Client ID
FFAA 02 01 00 00 000001 01 AAFF 240D0A 240D0A 
FFAA 02 01 00 00 000001 01 000001000002000003 AAFF 240D0A
FFAA 02 01 00 00 0000000001 03 000001 AAFF 网页初次连接不带分隔符
 
 -------------------客户端发软网关数据协议-------------------
帧头：2字节，FFAA
通信方向：1字节，01网关到服务器、02服务器到网关
子系统Id：1字节 业务子系统的ID号
设备类型：1字节 网关还是直连设备00,01
app是否初次连接服务：00初次连接app需要将app对应的用户账号对应的所有网关id一次发送过来，01发送信息
Client 类型：01 android、02 ios、03 web
Client Id：3个字节,网页是五个字节
网关or直连设备的：id，初次建立连接没有，发送消息时有
数据：MODBUS-RTU、自定义，初次建立连接为00
帧尾：2字节 AAFF
间隔符：3字节 240D0A
App Client Data
FFAA 02 01 00 01 01 000001  000001 FE030605DC09EC00C876F8 AAFF 240D0A
FFAA 02 01 00 01 03 0000000001  000001 FE030605DC09EC00C876F8 AAFF 240D0A网页初次连接不带分隔符
 
 -------------------客户端发网关数据协议-------------------
FFAA 02 011000000001020000A650 AAFF 240D0A

20170915

38、移植V0.3.1的解决方案，


因济宁需要6套网关，重新加入FLASH操作，
在falshif.c中增加2个函数操作fash，用最后一个程序Flash块来记录IP和端口号信息
void GetIpFromFlash(uint32_t *ptr)
void WriteIpToFlash(uint32_t *ptr,uint8_t len)

增加几个变量，用来记录sever IP
ipaddr_sever[0]=flash_net_buf[0]>>24;//取出最高IP位
			ipaddr_sever[1]=flash_net_buf[0]>>16;
			ipaddr_sever[2]=flash_net_buf[0]>>8;
			ipaddr_sever[3]=flash_net_buf[0]>>0;//取出最低IP位
			port_sever     =flash_net_buf[1];


操作步骤
1、使用PC的UDP协议发送广播，端口21228 发送 getid 
2、设备收到后回复网关芯片的ID号,
3、使用如下命令配置服务器的IP地址和端口号

			//[setconfig.1.2.3.4:56] ascii 字符
			//1.2.3.4分别表示IP地址的4个字节，用逗号隔开，高位在前地位在后
			//56，表示端口号，高位在前地位在后，用：引导
			//73 65 74 63 6F 6E 66 69 67 2E 31 2E 32 2E 33 2E 34 3A 35 36  HEX显示

			 例子：
			 我要配置服务器的IP为192.168.0.4，端口2301，
			 转换十六进制为
			 IP地址192.168.0.4，端口2301，-> HEX IP地址：0xc0,0xa8,0x00,0x04 端口：0x08fd
			 则就要发送如下命令
			 测试
			 {73 65 74 63 6F 6E 66 69 67 2E  C0  2E  A8  2E  00  2E  04  3A  08  FD } 
			 {[0][1][2][3][4][5][6][7][8][9][10][11][12][13][14][15][16][17][18][19]}
			
			  济宁：	60,211,192,14)
			 {73 65 74 63 6F 6E 66 69 67 2E  3c  2E  d3  2E  c0  2E  0e  3A  08  FD}

			 配置成功后返回:OK 
			 
4、使用getconfig,查询当前的服务器IP地址和端口号

2018.4.16


39、修改网络数据收发机制，把原来的数据多了丢弃的方式抓换为全部塞进lwip的方式，
 网络重大改动，底层原来的处理是以太网中断后读取一包数据然后退出中断，在主循环中检测这一包数据，然后调用LWIP的input,送入协议栈中
 在做无线的测试时我发现似乎有丢失的情况，并且TCP链接也不稳定，存在经常断开的情况。
 对这一现象进行分析，是因为我的故意丢包造成的。这种结构会造成在数据很多的情况下，中断来了读取了数据，但是主循环并没有时间处理，等
 LWIP在处理的时候，有来了一包数据，这个数据把原来的数据又覆盖了	，这样会造成失序，所以我修改了结构。
 我直接在主循环中检测以太网包，如果有包那么一次性全部读取完毕再去处理别的。这样从网卡读取的数据就都进入了LWIP协议栈中，不会丢包也
 不会出现失序问题，这个我今天晚上改完，需要长时间测试。如果OK那么这将是里程碑的意义。

 在中断函数屏蔽掉处理函数
 void ETH_IRQHandler(void)
{

#if 0
  /* Handles all the received frames */
  while(ETH_GetRxPktSize() != 0) 
  {		
   //LwIP_Pkt_Handle();
   Read_Eth_Packet();
  }
 #endif

在主程序轮训里面把数据一次性读取到LWIP中，因为Lwip有很大的BUFF，所以存储这些不是问题
 void ETH_Recive_Poll(void)
{

   
	 /* Handles all the received frames */
  while(ETH_GetRxPktSize() != 0) 
  {		
   //LwIP_Pkt_Handle();
   Read_Eth_Packet();
  


	if(frameds.length>0) //处理以太网数据
	{
     LwIP_Pkt_Handle();//处理早已经接收放入内存的数据。
	 memset(&frameds, 0, sizeof(FrameTypeDef));
	}	
  }


  
   #if 0
	if(frameds.length>0) //处理以太网数据
	{
     LwIP_Pkt_Handle();//处理早已经接收放入内存的数据。
	 memset(&frameds, 0, sizeof(FrameTypeDef));
	}	

   #endif




}


 我已经做过对比测试。
 1、如果把上面的#if 0 打开-也就是老程序	，然后UDP不断去网关获取getid，狂发，然后在开TCP，会发现TCP链接极其不稳定。并且发送应用层数据
 TCP也无法正常接收到，（这是因为大量的数据充斥在网卡里面，而我处理不过来就丢了。造成了大量数据丢失）

 2、使用新的结构测试，#if 0关闭，就是新的结构，发现UDP即使不断的去查询ID，网关在及时回复的情况下，依然可以从容的链接TCP并且毫无延迟的
 发送应用层数据。

 之前在测试中发现有时候网关频繁掉线，以为是网络问题，原来根在这里。也是当时埋下的一个坑。

 跟进测试。看看稳定不稳定。


 2018.4.16


 40、 在modbus——main.c中增加一个服务器下发配置。适应新和陈新定制的协议
 如下描述
 服务器下发我去解析，把数据解析出来之后循环发送给地下的设备。
 开启此功能需要、在main.h中定义
 /*定义后允许使用服务器下发FFBB配置指令进行配置后自动轮训485设备*/
#define RS485_MASTER_CALL

 -------------------服务器给网关发送网关需要定时查询数据的设备---------------------------------------------------------
FFBB 4123 0C C803000200013453 00000BB8 4223 0C D40300020001360F 00000BB8 BBFF240D0A
41->A和23->#是索引,OC命令和时间的长度(C803000200013453 00000BB8)，C803000200013453设备的实际查询命令modbus协议,00000BB8四字节毫秒为单位 
例如：FFBB 4123  0C  01 03 00 00 00 01 84 0A 000003E8   42 23 0C 02 03 00 00 00 01 84 39  000003E8  BB FF 24 0D 0A	
分别读取HOLD寄存器地址1和地址2，从0开始1个单元的数据
FFBB[起始标示]
+ （标示符实际不带）
4123[1st标识]+YY[后面的长度包括时间]MODBUS-RTU+000000xx[时间] 例如 ：0C 01 03 00 00 00 01 84 0A 000003E8 0C是表示从01到E8一共12字节，01-0A是命令数据，000003E8是间隔时间单位是ms,3e8转换为十进制是1000，所以是1000ms
+ （标示符实际不带）
4223[2st标识]+YY[后面的长度包括时间]MODBUS-RTU+00000000[以后的时间无意义，以1st的时间为准]
.  .											.	   .
.  .	  										.	   .
.  .											.	   .
+ （标示符实际不带）
4n23[2st标识]+YY[后面的长度包括时间]MODBUS-RTU+00000000[以后的时间无意义，以1st的时间为准]
+ 
BBFF240D0A[结束标识]

以上数据最多是1024字节，超出则BUFF溢出。并且必须一包TCP数据送出。定时单位最小30MS，小于30MS的定时值按照30MS进行。
可以发送FFBB0000000000000000000000000000BBFF240D0A来清空485主动查询命令
以下数据是发送的真实数据，查询了5条设备值，地址可以重复，命令可以随便。	下去什么转发什么
FFBB4123 0C 01 03 00 00 00 04 44 09 00000033  4223 0C 02 03 00 00 00 04 44 3A 00000033 4323 0C 03 03 00 02 00 01 24 28 00000033 4423 0C 04 03 00 00 00 02 C4 5E 00000033 4523 0C 04 03 00 02 00 02 65 9E    00000033  BBFF240D0A 
 

2018.4.16

41、 开启	SystemRunTime++;  //  系统运行时间单位1s一次
允许统计运行时间，单位是秒
可以使用getrun命令获取

2018.4.18

42、UDPsent重新加宏进行屏蔽方便统一

/*定义了则使能测试应用数据发送广播*/
//#define UDPBOARDCAST_ENABLE 

43、在UDP&TCP解析中增加一条命令：系统重启 reboot

利用WWDG复位CPU
RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);//使能看门狗
  WWDG_SetPrescaler(WWDG_Prescaler_1);//配置看门狗时钟分频系数
  WWDG_SetWindowValue(0x40);//设置窗口时间
  WWDG_SetCounter(0x7F);//喂狗
  WWDG_Enable(0x7F);//启动看门狗
  while(1);//WAITE :system will RESET

  2018.4.18

44、在TCP中	增加服务器IP和端口配置命令，和UDP的一样的
 else if ((strncmp(data, "setconfig", 9) == 0)&&
((strncmp(&data[17], ":",1) == 0)))

 	//[setconfig.1.2.3.4:56] ascii 字符
			//1.2.3.4分别表示IP地址的4个字节，用逗号隔开，高位在前地位在后
			//56，表示端口号，高位在前地位在后，用：引导
			//73 65 74 63 6F 6E 66 69 67 2E 31 2E 32 2E 33 2E 34 3A 35 36  HEX显示

			 例子：
			 我要配置服务器的IP为192.168.0.4，端口2301，
			 转换十六进制为
			 IP地址192.168.0.4，端口2301，-> HEX IP地址：0xc0,0xa8,0x00,0x04 端口：0x08fd
			 则就要发送如下命令
			 测试
			 {73 65 74 63 6F 6E 66 69 67 2E  C0  2E  A8  2E  00  2E  04  3A  08  FD } 
			 {[0][1][2][3][4][5][6][7][8][9][10][11][12][13][14][15][16][17][18][19]}
			
		

			  济宁：	123,59,96,233:5679)
			 {73 65 74 63 6F 6E 66 69 67 2E  7b  2E 3b  2E  60  2E  e9  3A  16  2F}

			   陈新：	182.61.22.232:5679)
			 {73 65 74 63 6F 6E 66 69 67 2E  b6  2E 3d  2E  16  2E  e8  3A  16  2F}

			 配置成功后返回:OK 
			 使用getconfig,查询当前的服务器IP地址和端口号
2018.4.18


45、修改485队列最大数量16个设备改为254个设备，济宁的就先那样吧。以后再做就按照254个设备，方便搜索用。
if(i==255)i=0;//最大上报数据设备15个设备 改为254
2018.4.22
46、修改接受FFBB指令时候没有清空buff的数据连续，导致485继续发原来的数据，济宁的那8个可以用FFBB+一串00来清除BUFF	00的数量可以使600个字节
 memset(TCP2RS485CMDLIST, 0, 1024);//先清空BUFF 
 	i=0; //清空的时候I的值还原，以便下一次快速进行 
 2018.4.22

 47、删除modbus_main.c中的i循环次数改为DevIndex，并在TCP接受到FFBB的时候给他强制重置为0，从头开始。上面46条修改的内容删除
 改动目的和46条一样

  2018.4.23

 48、解决在正常收发的同时额外485发送引起冲突带来的主485串口中断死循环问题，并debug了好长时间，终于定位到此问题，
 此问题主要是没有考虑到收中断和发中断之外还有异常中断处理，导致了等他们都判断失败后，没有清除中断标志，导致了UART一直在中断
 系统陷入死循环中。
 解决之道
 1.按照datasheet上的UART-SR说明，在ORE置位后，先读取一次SR值，然后在读取DR值，可以解决这个问题。
 2.更改了if else 结构。提高效率。 不做重复判读。
 属于非常规测试。正常使用过程中不会出现，但是有几率会出现，就得解决
 追踪了3天。我操
 	else
	{	
	   //异常状态
		      static  uint32_t err;
			/*A USART interrupt is generated whenever ORE=1 or RXNE=1 in the USART_SR register
			 It is cleared by a software sequence 
	->>>(an read to the USART_SR register followed by a read to the USART_DR register).
			  
			*/
			  err =USART2->SR ;	//read SR
			  err =USART2->DR ; // after read DR
	
	}
	 

 20180522

 49、更改了运行时计数从主循环中拿出来，然后放进定时器的HOOK中。并增加了这个函数。

 void SynruntimHook(uint32_t Time)
 主要是因为在主循环里面计数不准确。这样依旧不准确，需要根据实际情况对照计算，
 不过这个值不是很重要，主要在是看他是否在连续运行即可。

 20180522


50、 在main.h中增加版本定义

#define   MAIN_SN        0      //主序号
#define   SECOND_SN      6     //副序号
#define   SUB_SN         0    //子序号

可以在UDP命令中使用getver命令查询版本和编译信息   最后的编译时间日期都会有显示。

 20180522


 51、修改了最小MODBUS发送间隔
 	 if(MsaterTime<1)
	 {
	 	 //如果小于1MS
	 MsaterTime=1;//最小1ms
	 }

	 把原来的的最小50，放开为1ms

	 20180522

 52、修改  TCP2RS485CMDLIST[1024]为TCP的最大载荷TCP2RS485CMDLIST[1400],加大命令载荷长度。
     MODBUS 查询指令不要超过1400即可。
 20180522


53、增加IAP功能。  
	
1、IAP区是0x08000000-0x08010000;之间的64KB区域	  编译IAP区的选项卡ROM应该是0x08000000开始
2、APP区是0x08010000-0x08040000	之间的192KB区域	  编译APP区的选项卡ROM应该是0x08010000开始

在UDP命令中增加

   1、"getiap" 用来查询上电运行IAP还是APP的代码
         命令代号参考 
		iap_mode==0xAAAAAAAA 或者 iap_mode==0xFFFFFFFF 
		这个表示上电后需要跳转到APP程序里运行，默认是跳转到APP区的，如果跳转失败则会留在IAP区	
		iap_mode==0x55555555		
		这个表示上电后留在IAP区，等待升级程序


	2、"setiap"
	     设置为上电停留IAP区
	
		 
	3、"setapp"
		 设置为上电停留在APP区，默认是这个配置

设置IAP程序的版本号为0.7.0
#define   MAIN_SN        0      //主序号
#define   SECOND_SN      7     //副序号
#define   SUB_SN         0    //子序号


在MAIN.H中增加一个编译开关

#define USE_IAP_TFTP   /* enable IAP using TFTP */
如果开启了编译完毕的是IAP程序，关闭了则是APP程序

/*
定义后作为IAP程序，关闭后作为应用程序
特别注意：
IAP程序编译开始地址是0x08000000			   
APP程序编译开始地址是0x08010000  */

20180524

54、修改了看门狗程序。
上面的修改完成后发现查询GETIAP或者SETIAP切换的时候又死机，进入hardfault的情况。这个继续找，这个版本保险起见
增加看门狗，在main.h中开启
//定义后允许使用看门狗 
//#define USED_WATCHDOG

20180524


55、修改IAP模式下，udpcallbacke 结构把数据复制出来后立即拷贝进临时存储中，然后立马释放pbuf。我测试时发现开启IAP的情况下
频繁给21228UD端口下发指令（好几个网络助手一起发）极易出现死机或者进入hardfault的情况，debug发现要么是memalloc没有获取到数据
要么是memalloc执行hardfault.因为UDP结构特殊需要发送端构造pbuf，有所重叠，所以在他处理不过来的时候会失序。导致内存出问题。
 TCP那边除了配置数据有回复外，其他的基本不常用。正常数据接收后第一时间释放pbuf也做了修改
 这次修改涉及核心的通信，并且TCP和UDP都有修改。需要长时间测试。

修改后，3个助手狂发getver命令都没有再出现很快死机的情况。这个我再测。这个解决54条的问题、
 
 综上所述，收到远程数据后第一个时间就是把他们复制出来，然后立马释放pbuf.才可进行其他操作。


 版本增加1

 #define   MAIN_SN        0      //主序号
#define   SECOND_SN      8     //副序号
#define   SUB_SN         0    //子序号


20180525


56、在工程中增加si4432程序,移植了原来做的测试的程序，并需要在main.h中开启宏定义，接受程序轮训和发送程序添加，发送位置分别
位于485发送处，但是做了条件限制。只能发送数据为5字节的数据。超出长度或者不足都丢弃，只转485。无线从服务器端下来的数据只能是5字节--切记

要测试== 测试

 /*定义之后允许使用433无线*/
#define   USED_SI4432

更新版本标识
#define   MAIN_SN        0      //主序号
#define   SECOND_SN      9     //副序号
#define   SUB_SN         0    //子序号

20180730


远程升级发现一个bug,在运行app程序时，如果改为setiap ，在进行TFTP发现会卡死。只有setiap之后再重启，进入iap程序后在进行TFTP则正常
在iap程序中也经常会出现升级TFTP卡住的情况，有时候有，有时候没有，很是奇怪，现象应该是内部的BUFF用完了，只可以回复getid之类的，而getver这种没法回复。pbuf没有了
这证明有程序使用了他们。很有可能是TFTP出错，卡住，守护程序没有解脱他们。使得他们一直占用内存。
我发现每次如果进入iap程序就开始TFTP都会成功不会死机，但是要是等待一会，让他自己跑一段时间，再去TFTP就会死机。--20180731


修改了擦除芯片IAP标示区为FF时启动IAP程序。不转到APP，只有是AAAAA时才转到应用程序中去。这个版本不建议使用IAP





57、增加一条配置RF频道指令TCP发	：73 65 74 63 68 6C xx  xx表示频道号：从0-255,但是频道最大目前20个
  else if ((strncmp(Tdata, "setchl", 9) == 0))
							{
								/*设置频道*/
							   flash_net_buf[3]=Tdata[6];

							  	tcp_write(tpcb,"OK！\r\n",sizeof("OK！\r\n"), 1);
							 tcp_output(tpcb);
							}
20180815