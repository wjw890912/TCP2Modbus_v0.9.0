#ifndef DS18B20_H_  
#define DS18B20_H_  
  
//DS18B20指令  
typedef enum   
{  
    SEARCH_ROM          =   0xf0,   //搜索ROM指令  
    READ_ROM            =   0x33,   //读取ROM指令  
    MATH_ROM            =   0x55,   //匹配ROM指令  
    SKIP_ROM            =   0xcc,   //忽略ROM指令  
    ALARM_SEARCH        =   0xec,   //报警索索指令  
    CONVERT_T           =   0x44,   //温度转换指令  
    WRITE_SCRATCHPAD    =   0x4e,   //写暂存器指令  
    READ_SCRATCHPAD     =   0xbe,   //读取暂存器指令  
    COPY_SCRATCHPAD     =   0x48,   //拷贝暂存器指令  
    RECALL_E2           =   0xb8,   //召回EEPROM指令  
    READ_POWER_SUPPLY   =   0xb4,   //读取电源模式指令  
} DS18B20_CMD;  
  
  
  
//DS18B20 ROM编码  
typedef struct  
{  
    u8  DS18B20_CODE;   //DS18B20单总线编码:0x19  
    u8  SN_1;           //序列号第1字节  
    u8  SN_2;           //序列号第2字节  
    u8  SN_3;           //序列号第3字节  
    u8  SN_4;           //序列号第4字节  
    u8  SN_5;           //序列号第5字节  
    u8  SN_6;           //序列号第6字节  
    u8  crc8;           //CRC8校验码     
} DS18B20_ROM_CODE;   
  
  
  
#define DS18B20_Init()      (DeviceClockEnable(DEV_GPIOB,ENABLE))       //使能GPIOB时钟  
  
s16 DS18B20_ReadTemper(void);   //读取DS18B20温度  
void DS18B20_WriteData(u8 data);  
  
u8 DS18B20_SearchROM(u8 (*pID)[8],u8 Num);  //搜索ROM;  
s16 DS18B20_ReadDesignateTemper(u8 pID[8]); //读取指定ID的DS18B20温度  
void Init_DS18B20_IO(void);
void Delay_init(uint8_t SYSCLK);
void Delay_us(uint32_t Nus);
void Delay_ms(uint16_t nms);
void TemperatureThread(uint32_t Time);
  
  
  
#endif /*DS18B20_H_*/  
