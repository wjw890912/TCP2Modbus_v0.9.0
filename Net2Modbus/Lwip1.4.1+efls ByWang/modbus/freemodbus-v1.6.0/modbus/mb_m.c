/* 
 * FreeModbus Libary: A portable Modbus implementation for Modbus ASCII/RTU.
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * File: $Id: mbrtu_m.c,v 1.60 2013/08/20 11:18:10 Armink Add Master Functions $
 */


/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/

#include "mb.h"
#include "mb_m.h"
#include "mbconfig.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbfunc.h"

#include "mbport.h"
#if MB_MASTER_RTU_ENABLED == 1
#include "mbrtu.h"
#endif
#if MB_MASTER_ASCII_ENABLED == 1
#include "mbascii.h"
#endif
#if MB_MASTER_TCP_ENABLED == 1
#include "mbtcp.h"
#endif

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0

#ifndef MB_PORT_HAS_CLOSE
#define MB_PORT_HAS_CLOSE 0
#endif

/* ----------------------- Static variables ---------------------------------*/

static UCHAR    ucMBMasterDestAddress;
static BOOL     xMBRunInMasterMode = FALSE;
static BOOL     xMasterIsBusy = FALSE;

static enum
{
    STATE_ENABLED,
    STATE_DISABLED,
    STATE_NOT_INITIALIZED
} eMBState = STATE_NOT_INITIALIZED;

/* Functions pointer which are initialized in eMBInit( ). Depending on the
 * mode (RTU or ASCII) the are set to the correct implementations.
 * Using for Modbus Master,Add by Armink 20130813
 */
static peMBFrameSend peMBMasterFrameSendCur;
static pvMBFrameStart pvMBMasterFrameStartCur;
static pvMBFrameStop pvMBMasterFrameStopCur;
static peMBFrameReceive peMBMasterFrameReceiveCur;
static pvMBFrameClose pvMBMasterFrameCloseCur;

/* Callback functions required by the porting layer. They are called when
 * an external event has happend which includes a timeout or the reception
 * or transmission of a character.
 * Using for Modbus Master,Add by Armink 20130813
 */
BOOL( *pxMBMasterFrameCBByteReceived ) ( void );
BOOL( *pxMBMasterFrameCBTransmitterEmpty ) ( void );
BOOL( *pxMBMasterPortCBTimerExpired ) ( void );

BOOL( *pxMBMasterFrameCBReceiveFSMCur ) ( void );
BOOL( *pxMBMasterFrameCBTransmitFSMCur ) ( void );

/* An array of Modbus functions handlers which associates Modbus function
 * codes with implementing functions.
 */

 #if 0 /*需要屏蔽掉这段代码因为我不需要真正意义的modbus，仅用底层的收发框架*/
static xMBFunctionHandler xMasterFuncHandlers[MB_FUNC_HANDLERS_MAX] = {
#if MB_FUNC_OTHER_REP_SLAVEID_ENABLED > 0
	//TODO Add Master function define
    {MB_FUNC_OTHER_REPORT_SLAVEID, eMBFuncReportSlaveID},
#endif
#if MB_FUNC_READ_INPUT_ENABLED > 0
    {MB_FUNC_READ_INPUT_REGISTER, eMBMasterFuncReadInputRegister},
#endif
#if MB_FUNC_READ_HOLDING_ENABLED > 0
    {MB_FUNC_READ_HOLDING_REGISTER, eMBMasterFuncReadHoldingRegister},
#endif
#if MB_FUNC_WRITE_MULTIPLE_HOLDING_ENABLED > 0
    {MB_FUNC_WRITE_MULTIPLE_REGISTERS, eMBMasterFuncWriteMultipleHoldingRegister},
#endif
#if MB_FUNC_WRITE_HOLDING_ENABLED > 0
    {MB_FUNC_WRITE_REGISTER, eMBMasterFuncWriteHoldingRegister},
#endif
#if MB_FUNC_READWRITE_HOLDING_ENABLED > 0
    {MB_FUNC_READWRITE_MULTIPLE_REGISTERS, eMBMasterFuncReadWriteMultipleHoldingRegister},
#endif
#if MB_FUNC_READ_COILS_ENABLED > 0
    {MB_FUNC_READ_COILS, eMBMasterFuncReadCoils},
#endif
#if MB_FUNC_WRITE_COIL_ENABLED > 0
    {MB_FUNC_WRITE_SINGLE_COIL, eMBMasterFuncWriteCoil},
#endif
#if MB_FUNC_WRITE_MULTIPLE_COILS_ENABLED > 0
    {MB_FUNC_WRITE_MULTIPLE_COILS, eMBMasterFuncWriteMultipleCoils},
#endif
#if MB_FUNC_READ_DISCRETE_INPUTS_ENABLED > 0
    {MB_FUNC_READ_DISCRETE_INPUTS, eMBMasterFuncReadDiscreteInputs},
#endif
};
#endif
/* ----------------------- Start implementation -----------------------------*/
eMBErrorCode
eMBMasterInit( eMBMode eMode, UCHAR ucPort, ULONG ulBaudRate, eMBParity eParity )
{
    eMBErrorCode    eStatus = MB_ENOERR;

	switch (eMode)
	{
#if MB_MASTER_RTU_ENABLED > 0
	case MB_RTU:
		pvMBMasterFrameStartCur = eMBMasterRTUStart;
		pvMBMasterFrameStopCur = eMBMasterRTUStop;
		peMBMasterFrameSendCur = eMBMasterRTUSend;
		peMBMasterFrameReceiveCur = eMBMasterRTUReceive;
		pvMBMasterFrameCloseCur = MB_PORT_HAS_CLOSE ? vMBMasterPortClose : NULL;
		pxMBMasterFrameCBByteReceived = xMBMasterRTUReceiveFSM;
		pxMBMasterFrameCBTransmitterEmpty = xMBMasterRTUTransmitFSM;
		pxMBMasterPortCBTimerExpired = xMBMasterRTUTimerExpired;

		eStatus = eMBMasterRTUInit(ucPort, ulBaudRate, eParity);
		break;
#endif
#if MB_MASTER_ASCII_ENABLED > 0
		case MB_ASCII:
		pvMBMasterFrameStartCur = eMBMasterASCIIStart;
		pvMBMasterFrameStopCur = eMBMasterASCIIStop;
		peMBMasterFrameSendCur = eMBMasterASCIISend;
		peMBMasterFrameReceiveCur = eMBMasterASCIIReceive;
		pvMBMasterFrameCloseCur = MB_PORT_HAS_CLOSE ? vMBMasterPortClose : NULL;
		pxMBMasterFrameCBByteReceived = xMBMasterASCIIReceiveFSM;
		pxMBMasterFrameCBTransmitterEmpty = xMBMasterASCIITransmitFSM;
		pxMBMasterPortCBTimerExpired = xMBMasterASCIITimerT1SExpired;

		eStatus = eMBMasterASCIIInit(ucPort, ulBaudRate, eParity );
		break;
#endif
	default:
		eStatus = MB_EINVAL;
		break;
	}

	if (eStatus == MB_ENOERR)
	{
		if (!xMBMasterPortEventInit())
		{
			/* port dependent event module initalization failed. */
			eStatus = MB_EPORTERR;
		}
		else
		{
			eMBState = STATE_DISABLED;
		}
	}
	return eStatus;
}

eMBErrorCode
eMBMasterClose( void )
{
    eMBErrorCode    eStatus = MB_ENOERR;

    if( eMBState == STATE_DISABLED )
    {
        if( pvMBMasterFrameCloseCur != NULL )
        {
            pvMBMasterFrameCloseCur(  );
        }
    }
    else
    {
        eStatus = MB_EILLSTATE;
    }
    return eStatus;
}

eMBErrorCode
eMBMasterEnable( void )
{
    eMBErrorCode    eStatus = MB_ENOERR;

    if( eMBState == STATE_DISABLED )
    {
        /* Activate the protocol stack. */
        pvMBMasterFrameStartCur(  );
        eMBState = STATE_ENABLED;
    }
    else
    {
        eStatus = MB_EILLSTATE;
    }
    return eStatus;
}

eMBErrorCode
eMBMasterDisable( void )
{
    eMBErrorCode    eStatus;

    if( eMBState == STATE_ENABLED )
    {
        pvMBMasterFrameStopCur(  );
        eMBState = STATE_DISABLED;
        eStatus = MB_ENOERR;
    }
    else if( eMBState == STATE_DISABLED )
    {
        eStatus = MB_ENOERR;
    }
    else
    {
        eStatus = MB_EILLSTATE;
    }
    return eStatus;
}

extern 	char UID_STM32[16];
extern  uint8_t Mb2TcpBuff[256];
extern uint32_t Mb2TcpLenth;
extern char Statues_MB;
eMBErrorCode
eMBMasterPoll( void )
{
    static UCHAR   *ucMBFrame;
    static UCHAR    ucRcvAddress;
    static UCHAR    ucFunctionCode;
    static USHORT   usLength;
    static eMBException eException;
//    int             i;
    eMBErrorCode    eStatus = MB_ENOERR;
    eMBMasterEventType    eEvent;

    /* Check if the protocol stack is ready. */
    if( eMBState != STATE_ENABLED )
    {
        return MB_EILLSTATE;
    }

    /* Check if there is a event available. If not return control to caller.
     * Otherwise we will handle the event. */
    if( xMBMasterPortEventGet( &eEvent ) == TRUE )
    {
        switch ( eEvent )
        {
        case EV_MASTER_READY:
            break;

        case EV_MASTER_FRAME_RECEIVED:

			eStatus = peMBMasterFrameReceiveCur( &ucRcvAddress, &ucMBFrame, &usLength );

			if ( eStatus == MB_ENOERR  )
			{
				( void ) xMBMasterPortEventPost( EV_MASTER_EXECUTE );
			}
			else
			{
				( void ) xMBMasterPortEventPost( EV_MASTER_ERROR_PROCESS );
			}
			#if 0
			/* Check if the frame is for us. If not ,send an error process event. */
			if ( ( eStatus == MB_ENOERR ) && ( ucRcvAddress == ucMBMasterGetDestAddress() ) )
			{
				( void ) xMBMasterPortEventPost( EV_MASTER_EXECUTE );
			}
			else
			{
				( void ) xMBMasterPortEventPost( EV_MASTER_ERROR_PROCESS );
			}
			#endif

			break;

        case EV_MASTER_EXECUTE:
				//接受到了一帧RTU 数据

		    //Mb2TcpBuff[256];
			   if(usLength<256)
			   {

            /* Mb2TcpLenth=usLength;//更新接受到的数据长度，准备启动TCP发送
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
			 Mb2TcpLenth+=9;*/

			 
			 Mb2TcpLenth=usLength;//更新接受到的数据长度，准备启动TCP发送
			 Mb2TcpBuff[0]=0xFF;
			 Mb2TcpBuff[1]=0xAA;          //帧头
			
			 Mb2TcpBuff[2]=UID_STM32[2];         //上传数据到服务器标示
			
			 Mb2TcpBuff[3]=UID_STM32[3];         //子系统Id：业务子系统的ID号 ,z暂时未用，值默认为01即可
			
			
			 Mb2TcpBuff[4]=UID_STM32[4];
			 Mb2TcpBuff[5]=UID_STM32[5];
			 Mb2TcpBuff[6]=UID_STM32[6]; //网关ID(3个字节，设备配置产生)
			
			 Mb2TcpBuff[7]=0x02;         //设备分类（1个字节 01网关、02设备、03直连 写死）
			
			
			 Mb2TcpBuff[8]=UID_STM32[8];					  
			 Mb2TcpBuff[9]=UID_STM32[9];         //设备类型（2个字节、设备配置产生）
			
			
			 Mb2TcpBuff[10]=0;
			 Mb2TcpBuff[11]=0;
			 Mb2TcpBuff[12]=0; //设备id(3个字节，设备配置产生),
			
			memcpy(&Mb2TcpBuff[13],ucMBFrame,Mb2TcpLenth);//MODBUS-RTU数据复制进TCP发送buf中
			
			
			 Mb2TcpBuff[Mb2TcpLenth+13+0]=0xAA;
			 Mb2TcpBuff[Mb2TcpLenth+13+1]=0xFF;	 //帧尾
			
			 Mb2TcpBuff[Mb2TcpLenth+13+2]=0x24;	
			 Mb2TcpBuff[Mb2TcpLenth+13+3]=0x0D;
			 Mb2TcpBuff[Mb2TcpLenth+13+4]=0x0A;	//间隔符	
			  
			 Mb2TcpLenth+=18;//计算帧总长度，最终作为发送标示发送到TCP buf
			


			   }
			 eException = MB_EX_ILLEGAL_FUNCTION;
			
		#if 0
            ucFunctionCode = ucMBFrame[MB_PDU_FUNC_OFF];
            eException = MB_EX_ILLEGAL_FUNCTION;

           for( i = 0; i < MB_FUNC_HANDLERS_MAX; i++ )
            {
               // No more function handlers registered. Abort. 
                if( xMasterFuncHandlers[i].ucFunctionCode == 0 )
                {
                    break;
                }
                else if( xMasterFuncHandlers[i].ucFunctionCode == ucFunctionCode )
                {
                	vMBMasterSetCBRunInMasterMode(TRUE);
                    eException = xMasterFuncHandlers[i].pxHandler( ucMBFrame, &usLength );
                    vMBMasterSetCBRunInMasterMode(FALSE);
                    break;
                }
            }  

            /* If receive frame has exception .The receive function code highest bit is 1.*/
            if(ucFunctionCode >> 7) eException = (eMBException)ucMBFrame[MB_PDU_DATA_OFF];
            /* If master has exception ,Master will send error process.Otherwise the Master is idle.*/
            if (eException != MB_EX_NONE) ( void ) xMBMasterPortEventPost( EV_MASTER_ERROR_PROCESS );
            else vMBMasterSetIsBusy( FALSE );
			#endif
            break;

        case EV_MASTER_FRAME_SENT:
        	/* Master is busy now. */
        	vMBMasterSetIsBusy( TRUE );
        	vMBMasterGetPDUSndBuf( &ucMBFrame );
			eStatus = peMBMasterFrameSendCur( ucMBMasterGetDestAddress(), ucMBFrame, ucMBMasterGetPDUSndLength() );
            break;
        case EV_MASTER_ERROR_PROCESS:
        	vMBMasterSetIsBusy( FALSE );
        	break;
        }
    }
    return MB_ENOERR;
}
/* Get whether the Modbus Master is busy.*/
BOOL xMBMasterGetIsBusy( void )
{
	return xMasterIsBusy;
}
/* Set whether the Modbus Master is busy.*/
void vMBMasterSetIsBusy( BOOL IsBusy )
{
	xMasterIsBusy = IsBusy;
}
/* Get whether the Modbus Master is run in master mode.*/
BOOL xMBMasterGetCBRunInMasterMode( void )
{
	return xMBRunInMasterMode;
}
/* Set whether the Modbus Master is run in master mode.*/
void vMBMasterSetCBRunInMasterMode( BOOL IsMasterMode )
{
	xMBRunInMasterMode = IsMasterMode;
}
/* Get Modbus Master send destination address*/
UCHAR ucMBMasterGetDestAddress( void )
{
	return ucMBMasterDestAddress;
}
/* Set Modbus Master send destination address*/
void vMBMasterSetDestAddress( UCHAR Address )
{
	ucMBMasterDestAddress = Address;
}

#endif
