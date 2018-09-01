#ifndef __SI4432PROTO_H__
#define __SI4432PROTO_H__




/*无线控制相关的设备类型*/
typedef enum
{
	SWITCH_86 = 0x01,         /*86盒开关*/
	SWITCH_LAMP,              /*灯头开关*/
	WINDOW_CURTAINS,          /*窗帘*/
	TEMPHUMI_SENSOR,          /*温湿度传感器*/
	SWITCH,             /*STM32交换机*/
	CTRLSIG_315,              /*315控制信号*/
	INFRAREDTV_CTRL,          /*红外电视遥控终端*/
	
	DEVTYPE_MAX               /*最大值*/
	
}eDevType_SI4432Ctrl;


/*通道号*/
typedef enum
{
	CHAN_1 = 0x01,            /*通道1*/
	CHAN_2,                   /*通道2*/
	CHAN_3,                   /*通道3*/
	CHAN_ALL,                 /*所有通道*/
	CHAN_INVALID              /*通道无效*/
	
}eChan_Num;


/*控制命令*/

/*灯具类*/
#define LIGHTCTRL_OPEN          0x01          /*开灯*/
#define LIGHTCTRL_CLOSE         0x00          /*关灯*/
#define LIGHTCTRL_MODULATE      0x03          /*调光*/
#define LIGHTCTRL_REPORT        0x04          /*灯状态返回*/
#define LIGHTCTRL_LEARN_REQ     0x05          /*从设备ID学习请求*/
#define LIGHTCTRL_LEARN_OK      0x06          /*主设备ID学习回复*///从设备发起，

/*窗帘类*/
#define WINDOWCTRL_T_OPEN       0x11          /*水平开窗（全开）*/
#define WINDOWCTRL_T_CLOSE      0x12          /*水平关窗（全关）*/
#define WINDOWCTRL_T_MODULATE   0x13          /*调整水平开窗（窗帘进度数据搭载到数据域）*/
#define WINDOWCTRL_T_REPORT     0x14          /*返回水平窗子的状态+进度（进度填充数据域例如：全开是百分之百，全关是百分之0，以此类推！）*/
#define WINDOWCTRL_L_OPEN       0x15          /*上升（到顶）*/
#define WINDOWCTRL_L_CLOSE      0x16          /*下降（到底）*/
#define WINDOWCTRL_L_MODULATE   0x17          /*调整上下开窗（窗帘进度数据搭载到数据域）*/
#define WINDOWCTRL_L_REPORT     0x18          /*返回垂直窗子的状态+进度（进度填充数据域例如：全开是百分之百，全关是百分之0，以此类推！）*/

/*红外遥控类*/
#define INFRARED_BUF_READ       0x21          /*读取红外设备的缓冲区（编号+缓冲区节，数据共10个字节，驻留在数据域）*/
#define INFRARED_BUF_REPORT     0x22          /*回复红外设备的缓冲区（编号+缓冲区节，数据共10个字节驻留在数据域）*/
#define INFRARED_CODE_SEND      0x23          /*发送红外遥控编码（编码数据在数据域搭载）*/
#define INFRARED_CODE_REPORT    0x24          /*返回红外遥控控码的状态（成功或者失败，搭载到数据域）*/

/*温湿度采集类*/
#define TEMPERATURE_REQUEST     0x31          /*要求上行温度数据（数据搭载到数据域）*/
#define HUMIDITY_REQUEST        0x32          /*要求上行湿度数据（数据搭载到数据域）*/
#define TEMPERATURE_REPORT      0x33          /*返回设备采集到的温度数据（数据搭载到数据域）*/
#define HUMIDITY_REPORT         0x34          /*返回设备采集到的湿度数据（数据搭载到数据域）*/

/*315报警类*/


/*控制命令 END*/



  struct switch_struct
 {
 
   unsigned char Channel_number; //通道号
   unsigned char state;  //灯状态
    unsigned char  Spare;  //备用
 
 };

struct Protocol_Structure
{

   unsigned int dst;  //目标地址
   unsigned int  src;  //源地址
   unsigned char   dev_type; //设备类型
  struct switch_struct s;

};

/*解析无线协议帧函数*/
unsigned char RecvCmdFromSI4432(unsigned char *pBuf,unsigned char leg);
void Light_State_Retrn (struct Protocol_Structure *pFram,eChan_Num Channel, unsigned char  state);
unsigned char *Package_Data_Stream(struct Protocol_Structure*pbuf,unsigned char *buf);

#endif



