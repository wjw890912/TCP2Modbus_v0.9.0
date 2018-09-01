/*****************************************************************************\
*              efs - General purpose Embedded Filesystem library              *
*          --------------------- -----------------------------------          *
*                                                                             *
* Filename : sd.c                                                             *
* Revision : Initial developement                                             *
* Description : This file contains the functions needed to use efs for        *
*               accessing files on an SD-card.                                *
*                                                                             *
* This program is free software; you can redistribute it and/or               *
* modify it under the terms of the GNU General Public License                 *
* as published by the Free Software Foundation; version 2                     *
* of the License.                                                             *
                                                                              *
* This program is distributed in the hope that it will be useful,             *
* but WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
* GNU General Public License for more details.                                *
*                                                                             *
* As a special exception, if other files instantiate templates or             *
* use macros or inline functions from this file, or you compile this          *
* file and link it with other works to produce a work based on this file,     *
* this file does not by itself cause the resulting work to be covered         *
* by the GNU General Public License. However the source code for this         *
* file must still be made available in accordance with section (3) of         *
* the GNU General Public License.                                             *
*                                                                             *
* This exception does not invalidate any other reasons why a work based       *
* on this file might be covered by the GNU General Public License.            *
*                                                                             *
*                                                    (c)2006 Lennart Yseboodt *
*                                                    (c)2006 Michael De Nil   *
\*****************************************************************************/

/*****************************************************************************/
#include "interface/sd.h"
#include "stm32f10x_spi.h"
#include "stm32f10x.h"
#include "config.h"
 esint8 sdhc_readSector(hwInterface *iface,euint32 address, euint8* buf, euint16 len);
 euint16  get_spi_BaudRatePrescaler(euint32  max_hz);
/*****************************************************************************/
msd_card_type SDcard;//SD 卡 V1.0?V2.0? it has save...
 euint32 max_clock;
esint8 sd_Init(hwInterface *iface)
{
	esint16 i;
	euint8 resp,respSD[7];
	euint32 OCR;

	/*发送CMD0命令使SD卡进入IDLE模式*/
	/* Send CMD0 (GO_IDLE_STATE) to put MSD in SPI mode*/
	/* Try to send reset command up to 100 times */
	i=100;
	do{
		sd_Command(iface,GO_IDLE_STATE , 0, 0);
		resp=sd_Resp8b(iface);
	}
	while(resp!=1 && i--);
	
	if(resp!=1){
		if(resp==0xff){
			return(-1);
		}
		else{
			sd_Resp8bError(iface,resp);
			return(-2);
		}
	} /*CMD0 end*/

	/*发送CMD8命令判断SD卡是V1.x还是V2.x*/
	  do{
					    sd_Command_crc(iface,SEND_IF_COND, 0,0x01AA, 0x87) ;//V2.0需要CRC否则不响应V1.X只需要CMD0crc
						respSD[0]=sd_Resp8b(iface);
						respSD[1]=sd_Resp8b(iface);
						respSD[2]=sd_Resp8b(iface);
						respSD[3]=sd_Resp8b(iface);
						respSD[4]=sd_Resp8b(iface);
						respSD[5]=sd_Resp8b(iface);
					 	respSD[6]=sd_Resp8b(iface);
					  
					if(respSD[0] & (1<<2))//
					{
					   //SD卡是V1.0的卡也就是卡容量在<2GB
						  /* illegal command, SD V1.x or MMC card */
					   	 SDcard=MSD_CARD_TYPE_SD_V1_X;
						/* Wait till card is ready initialising (returns 0 on CMD1) */
						/* Try up to 32000 times. */
									i=32000;
									do{
										sd_Command(iface,1, 0, 0);
										
										resp=sd_Resp8b(iface);
										if(resp!=0)
											sd_Resp8bError(iface,resp);
									}
									while(resp==1 && i--);
									
									if(resp!=0){
										sd_Resp8bError(iface,resp);
										return(-3);
									}
									
									return(0);
					}
					else 
					{
					//SD卡是V2.0的卡也就是卡容量在2GB<V2.0<32GB
					/* SD V2.0 or later or SDHC or SDXC memory card! */
					 SDcard= MSD_CARD_TYPE_SD_V2_X;
					
							 if((0xAA == respSD[4]) && (0x00 == respSD[3]))
		                    {
		                        /* SD2.0 not support current voltage */
		                       	   return -4;
		                    }
				
				
				
					}


	 }
	while(respSD[4]!=0xAA);
   /*card is 2.x that read OCR register slecte SDHC*/
	if(SDcard == MSD_CARD_TYPE_SD_V2_X)
	{
	
					    sd_Command_crc(iface,READ_OCR, 0,0,0) ; //read OCR
						respSD[0]=sd_Resp8b(iface);
						respSD[1]=sd_Resp8b(iface);
						respSD[2]=sd_Resp8b(iface);
						respSD[3]=sd_Resp8b(iface);
						respSD[4]=sd_Resp8b(iface);
					    if((respSD[0] & 0xFE) != 0)
				            {
				               return -5;  //NO OCR Resp..
				      /*It look CMD58 as illegal command so it is not SD card*/ 
				           
				            }
				
			
			            OCR = respSD[1];
			            OCR = (OCR<<8) + respSD[2];
			            OCR = (OCR<<8) + respSD[3];
			            OCR = (OCR<<8) + respSD[4];
			      
			
					  /* --Send ACMD41 to make card ready */
					  /* try CMD55 + ACMD41 */
			            do
			            {
			               
			
			                  /* CMD55 APP_CMD */
							  sd_Command_crc(iface,APP_CMD, 0,0,0x65) ; //read OCR
			               	  respSD[0]=sd_Resp8b(iface);
			                if((respSD[0] & 0xFE) != 0)
			                {    
			                   /*Not SD ready*/
			                   	 return -7;
			                }
			
			                /* ACMD41 SD_SEND_OP_COND */
							sd_Command_crc(iface,SD_SEND_OP_COND, 0x4000,0,0x77) ; 
			               	  respSD[0]=sd_Resp8b(iface);
			         
			
			                if((respSD[0] & 0xFE) != 0)
			                {
			                   
			               /* Not SD card*/
								return -8;

			                }
			            }
			            while(respSD[0] != MSD_RESPONSE_NO_ERROR);	 /* try CMD55 + ACMD41 */

					    /* --Read OCR again */
	      
         
          			    sd_Command_crc(iface,READ_OCR, 0,0,0) ; //read OCR
						respSD[0]=sd_Resp8b(iface);
						respSD[1]=sd_Resp8b(iface);
						respSD[2]=sd_Resp8b(iface);
						respSD[3]=sd_Resp8b(iface);
						respSD[4]=sd_Resp8b(iface);

			            if((respSD[0] & 0xFE) != 0)
			            {
			               
			/* It look 2nd CMD58 as illegal command so it is not SD card*/
			               	return -9;
			            }
			          
			
			            OCR = respSD[1];
			            OCR = (OCR<<8) + respSD[2];
			            OCR = (OCR<<8) + respSD[3];	 	
			            OCR = (OCR<<8) + respSD[4];
			        
			            if((OCR & 0x40000000) != 0)
			            {
			             /* It is SD2.0 SDHC Card*/
			                SDcard = MSD_CARD_TYPE_SD_SDHC;
			            }
			            else
			            {
			               /*It is SD2.0 standard capacity Card*/
							SDcard = MSD_CARD_TYPE_UNKNOWN ;
						}

	
	
	}





}
/*****************************************************************************/
static euint8 crc7(const euint8 *buf, int len)
{
    unsigned char	i, j, crc, ch, ch2, ch3;

    crc = 0;

    for (i = 0; i < len; i ++)
    {
        ch = buf[i];

        for (j = 0; j < 8; j ++, ch <<= 1)
        {
            ch2 = (crc & 0x40) ? 1 : 0;
            ch3 = (ch & 0x80) ? 1 : 0;

            if (ch2 ^ ch3)
            {
                crc ^= 0x04;
                crc <<= 1;
                crc |= 0x01;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}

void sd_Command(hwInterface *iface,euint8 cmd, euint16 paramx, euint16 paramy)
{
	if_spiSend(iface,0xff);

	if_spiSend(iface,0x40 | cmd);
	if_spiSend(iface,(euint8) (paramx >> 8)); /* MSB of parameter x */
	if_spiSend(iface,(euint8) (paramx)); /* LSB of parameter x */
	if_spiSend(iface,(euint8) (paramy >> 8)); /* MSB of parameter y */
	if_spiSend(iface,(euint8) (paramy)); /* LSB of parameter y */
															               
	if_spiSend(iface,0x95); /* Checksum (should be only valid for first command (0) */

	if_spiSend(iface,0xff); /* eat empty command - response */
}

void sd_Command_crc(hwInterface *iface,euint8 cmd, euint16 paramx, euint16 paramy,euint8 crc)
{
	  euint8 cmd_buffercrc[4];

	if_spiSend(iface,0xff);

	if_spiSend(iface,0x40 | cmd);
	if_spiSend(iface,(euint8) (paramx >> 8)); /* MSB of parameter x */
	if_spiSend(iface,(euint8) (paramx)); /* LSB of parameter x */
	if_spiSend(iface,(euint8) (paramy >> 8)); /* MSB of parameter y */
	if_spiSend(iface,(euint8) (paramy)); /* LSB of parameter y */
	if(crc == 0x00)
    {	cmd_buffercrc[0]= (cmd | 0x40);
		cmd_buffercrc[1]= (euint8) (paramx >> 8);
		cmd_buffercrc[2]= (euint8) (paramx);
		cmd_buffercrc[3]= (euint8) (paramy >> 8);
		cmd_buffercrc[4]= (euint8) (paramy);
        crc = crc7(&cmd_buffercrc[0], 5);
        crc = (crc<<1) | 0x01;
    }														               
	if_spiSend(iface,crc); /* Checksum (should be only valid for first command (0) */

	if_spiSend(iface,0xff); /* eat empty command - response */
}
/*****************************************************************************/

euint8 sd_Resp8b(hwInterface *iface)
{
	euint8 i;
	euint8 resp;
	
	/* Respone will come after 1 - 8 pings */
	for(i=0;i<8;i++){
		resp = if_spiSend(iface,0xff);
		if(resp != 0xff)
			return(resp);
	}
		
	return(resp);
}
/*****************************************************************************/

euint16 sd_Resp16b(hwInterface *iface)
{
	euint16 resp;
	
	resp = ( sd_Resp8b(iface) << 8 ) & 0xff00;
	resp |= if_spiSend(iface,0xff);
	
	return(resp);
}
/*****************************************************************************/

void sd_Resp8bError(hwInterface *iface,euint8 value)
{
	switch(value)
	{
		case 0x40:
			//DBG((TXT("Argument out of bounds.\n")));
			break;
		case 0x20:
			//DBG((TXT("Address out of bounds.\n")));
			break;
		case 0x10:
			//DBG((TXT("Error during erase sequence.\n")));
			break;
		case 0x08:
			//DBG((TXT("CRC failed.\n")));
			break;
		case 0x04:
			//DBG((TXT("Illegal command.\n")));
			break;
		case 0x02:
			//DBG((TXT("Erase reset (see SanDisk docs p5-13).\n")));
			break;
		case 0x01:
			//DBG((TXT("Card is initialising.\n")));
			break;
		default:
			//DBG((TXT("Unknown error 0x%x (see SanDisk docs p5-13).\n"),value));
			break;
	}
}
/*****************************************************************************/

esint8 sd_State(hwInterface *iface)
{
	eint16 value;
	
	sd_Command(iface,13, 0, 0);
	value=sd_Resp16b(iface);

	switch(value)
	{
		case 0x000:
			return(1);
			//break;
		case 0x0001:
			//DBG((TXT("Card is Locked.\n")));
			break;
		case 0x0002:
			//DBG((TXT("WP Erase Skip, Lock/Unlock Cmd Failed.\n")));
			break;
		case 0x0004:
			//DBG((TXT("General / Unknown error -- card broken?.\n")));
			break;
		case 0x0008:
			//DBG((TXT("Internal card controller error.\n")));
			break;
		case 0x0010:
			//DBG((TXT("Card internal ECC was applied, but failed to correct the data.\n")));
			break;
		case 0x0020:
			//DBG((TXT("Write protect violation.\n")));
			break;
		case 0x0040:
			//DBG((TXT("An invalid selection, sectors for erase.\n")));
			break;
		case 0x0080:
			//DBG((TXT("Out of Range, CSD_Overwrite.\n")));
			break;
		default:
			if(value>0x00FF)
				sd_Resp8bError(iface,(euint8) (value>>8));
			else
				//DBG((TXT("Unknown error: 0x%x (see SanDisk docs p5-14).\n"),value));
			break;
	}
	return(-1);
}
/*****************************************************************************/

/* ****************************************************************************
 * WAIT ?? -- FIXME
 * CMDWRITE
 * WAIT
 * CARD RESP
 * WAIT
 * DATA BLOCK OUT
 *      START BLOCK
 *      DATA
 *      CHKS (2B)
 * BUSY...
 */

esint8 sd_writeSector(hwInterface *iface,euint32 address, euint8* buf)
{
	euint32 place;
	euint16 i;
	euint16 t=0;
	if(	SDcard==MSD_CARD_TYPE_SD_SDHC)
	{
	/*write SDHC a Sector*/
	  place=address;
	}
	else
	{
	/*DBG((TXT("Trying to write %u to sector %u.\n"),(void *)&buf,address));*/
	place=512*address;
	}
	sd_Command(iface,CMDWRITE, (euint16) (place >> 16), (euint16) place);

	sd_Resp8b(iface); /* Card response */

	if_spiSend(iface,0xfe); /* Start block */
	for(i=0;i<512;i++) 
		if_spiSend(iface,buf[i]); /* Send data */
	if_spiSend(iface,0xff); /* Checksum part 1 */
	if_spiSend(iface,0xff); /* Checksum part 2 */

	if_spiSend(iface,0xff);

	while(if_spiSend(iface,0xff)!=0xff){
		t++;
		/* Removed NOP */
	}
	/*DBG((TXT("Nopp'ed %u times.\n"),t));*/

	return(0);
}
/*****************************************************************************/

/* ****************************************************************************
 * WAIT ?? -- FIXME
 * CMDCMD
 * WAIT
 * CARD RESP
 * WAIT
 * DATA BLOCK IN
 * 		START BLOCK
 * 		DATA
 * 		CHKS (2B)
 */

esint8 sd_readSector(hwInterface *iface,euint32 address, euint8* buf, euint16 len)
{
	euint8 cardresp;
	euint8 firstblock;
	euint8 c;
	euint16 fb_timeout=0xffff;
	euint32 i;
	euint32 place;
	 if(SDcard==MSD_CARD_TYPE_SD_SDHC)
	{
	/*read SDHC a Sector*/
	 place=	address;
	}
	else
	{
	/*DBG((TXT("sd_readSector::Trying to read sector %u and store it at %p.\n"),address,&buf[0]));*/
	place=512*address;
	}
	sd_Command(iface,CMDREAD, (euint16) (place >> 16), (euint16) place);
	
	cardresp=sd_Resp8b(iface); /* Card response */ 
	if(cardresp!=0)
	{
	
	  return -3;
	}
	/* Wait for startblock */
	do
		firstblock=sd_Resp8b(iface); 
	while(firstblock==0xff && fb_timeout--);

	if(cardresp!=0x00 || firstblock!=0xfe){
		sd_Resp8bError(iface,firstblock);
		return(-1);
	}
	
	for(i=0;i<512;i++){
		c = if_spiSend(iface,0xff);
		if(i<len)
			buf[i] = c;
	}

	/* Checksum (2 byte) - ignore for now */
	if_spiSend(iface,0xff);
	if_spiSend(iface,0xff);

	return(0);
}
/*****************************************************************************/

/* ****************************************************************************
 calculates size of card from CSD 
 (extension by Martin Thomas, inspired by code from Holger Klabunde)
 */
esint8 sd_getDriveSize(hwInterface *iface, euint32* drive_size )
{	  SPI_InitTypeDef   SPI_InitStructure;
	 euint8 cardresp, i, by;
	 euint8 iob[16];
	 euint16 c_size, c_size_mult, read_bl_len;
	 euint8  CSD_STRUCTURE;
	 euint32 C_SIZE;
     euint32 card_capacity;
     euint8 tmp8;
     euint16 tmp16;
     euint32 tmp32;
	
	sd_Command(iface, CMDREADCSD, 0, 0);
	
	do {
		cardresp = sd_Resp8b(iface);
	} while ( cardresp != 0xFE );

	//DBG((TXT("CSD:")));
	for( i=0; i<16; i++) {
		iob[i] = sd_Resp8b(iface);
		//DBG((TXT(" %02x"), iob[i]));
	}
	//DBG((TXT("\n")));

	if_spiSend(iface,0xff);
	if_spiSend(iface,0xff);
	  if(SDcard == MSD_CARD_TYPE_SD_SDHC)
  	   {
	           
			
			{	
			    /* get CSD_STRUCTURE */
	            tmp8 = iob[0] & 0xC0; /* 0b11000000 */
	            CSD_STRUCTURE = tmp8 >> 6;
	
			 }
		   if(CSD_STRUCTURE == 1)//V2.X
		   {
		   		//MSD_DEBUG("[info] CSD Version 2.0\r\n");

                    /* get TRAN_SPEED 8bit [103:96] */
                    tmp8 = iob[3];
                    if(tmp8 == 0x32)
                    {
                        max_clock = 1000 * 1000 * 10; /* 10Mbit/s. */
                    }
                    else if(tmp8 == 0x5A)
                    {
                       max_clock = 1000 * 1000 * 50; /* 50Mbit/s. */
                    }
                    else if(tmp8 == 0x0B)
                    {
                       max_clock = 1000 * 1000 * 100; /* 100Mbit/s. */
                        /* UHS50 Card sets TRAN_SPEED to 0Bh (100Mbit/sec), */
                        /* for both SDR50 and DDR50 modes. */
                    }
                    else if(tmp8 == 0x2B)
                    {
                       max_clock = 1000 * 1000 * 200; /* 200Mbit/s. */
                        /* UHS104 Card sets TRAN_SPEED to 2Bh (200Mbit/sec). */
                    }
                    else
                    {
                       max_clock = 1000 * 1000 * 1; /* 1Mbit/s default. */
                    }
                   // MSD_DEBUG("[info] TRAN_SPEED: 0x%02X, %dMbit/s.\r\n", tmp8, msd->max_clock/1000/1000);
				 get_spi_BaudRatePrescaler(max_clock);
                    /* get C_SIZE 22bit [69:48] */
                    tmp32 = iob[7] & 0x3F; /* 0b00111111 */
                    tmp32 = tmp32<<8;
                    tmp32 += iob[8];
                    tmp32 = tmp32<<8;
                    tmp32 += iob[9];
                    C_SIZE = tmp32;
                   // MSD_DEBUG("[info] CSD : C_SIZE : %d\r\n", C_SIZE);
                 return  *drive_size  = (C_SIZE<<9)*1024; //单位是字节BYTE
		   	//返回数据应该是SD卡的总字节，因为后面要对她进行除以512.计算扇区。
		   }

		  
      }
	
	c_size = iob[6] & 0x03; // bits 1..0
	c_size <<= 10;
	c_size += (euint16)iob[7]<<2;
	c_size += iob[8]>>6;

	by= iob[5] & 0x0F;
	read_bl_len = 1;
	read_bl_len <<= by;

	by=iob[9] & 0x03;
	by <<= 1;
	by += iob[10] >> 7;
	
	c_size_mult = 1;
	c_size_mult <<= (2+by);
	
	*drive_size = (euint32)(c_size+1) * (euint32)c_size_mult * (euint32)read_bl_len;
	
	return 0;
}


euint16  get_spi_BaudRatePrescaler(euint32  max_hz)
{


    euint16  SPI_BaudRatePrescaler;
      SPI_InitTypeDef SPI_InitStructure;

	  return 1;
    /* STM32F10x SPI MAX 18Mhz */
    if(max_hz >= 72000000/2 && 72000000/2 <= 18000000)
    {
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    }
    else if(max_hz >= 72000000/4)
    {
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    }
    else if(max_hz >= 72000000/8)
    {
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    }
    else if(max_hz >= 72000000/16)
    {
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    }
    else if(max_hz >= 72000000/32)
    {
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    }
    else if(max_hz >= 72000000/64)
    {
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    }
    else if(max_hz >= 72000000/128)
    {
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
    }
    else
    {
        /* min prescaler 256 */
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    }

   // return SPI_BaudRatePrescaler;

   /* SPI1 Config */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure);

  /* SPI1 enable */
  SPI_Cmd(SPI1, ENABLE);


}

 /*******************************************/
 /*用于SDHC型2G<v2.x<32G的SD卡的读写扇区*/
 esint8 sdhc_readSector(hwInterface *iface,euint32 address, euint8* buf, euint16 len)
{
			  euint8	cardresp;
       		  euint32 place;
			  place= address;
	   sd_Command_crc(iface,CMDREAD, (euint16) (place >> 16), (euint16) place,0);
	   cardresp=sd_Resp8b(iface); /* Card response */
	   if((cardresp != MSD_RESPONSE_NO_ERROR))
        {
           // MSD_DEBUG("[err] read SINGLE_BLOCK #%d fail!\r\n", pos);
         
            	return -1;
        }


}
static esint8 sdhc_writeSector(hwInterface *iface,euint32 address, euint8* buf)
{
	   
	
}







