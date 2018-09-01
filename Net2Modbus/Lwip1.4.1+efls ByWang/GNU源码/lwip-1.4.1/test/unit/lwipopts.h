/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Simon Goldschmidt
 *
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

/* Prevent having to link sys_arch.c (we don't test the API layers in unit tests) */
#define NO_SYS                          1
#define LWIP_NETCONN                    0
#define LWIP_SOCKET                     0

/* Minimal changes to opt.h required for tcp unit tests: 6*1024*/
#define MEM_SIZE                        6*1024


/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         0

/* TCP Maximum segment size. */
#define TCP_MSS                 (1500 - 40)	  /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF          (2*TCP_MSS)   //(2*TCP_MSS)

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN     (6 * TCP_SND_BUF)/TCP_MSS   //(6 * TCP_SND_BUF)/TCP_MSS

/**
 * MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP segments.
 * (requires the LWIP_TCP option)
 */
 #define MEMP_NUM_TCP_SEG     (6 * TCP_SND_BUF)/TCP_MSS	  //(6 * TCP_SND_BUF)/TCP_MSS

/* TCP receive window. */
#define TCP_WND                 (2*TCP_MSS)


/* Minimal changes to opt.h required for etharp unit tests: */
#define ETHARP_SUPPORT_STATIC_ENTRIES   0
/*shoud be keep aliving.....*/
#define LWIP_TCP_KEEPALIVE              1


/*DHCP allow open*/
#define LWIP_DHCP                       1

/*AUTOIP if DHCP are filed  we can set to static IP addr*/
//#define LWIP_AUTOIP                     1
/*Allow DHCP and AUTOIP to be both enabled on
 * the same interface at the same time*/
//#define LWIP_DHCP_AUTOIP_COOP           1



/*if turn on DHCP  The name is : wjw_ip*/
#define LWIP_NETIF_HOSTNAME             1


#define NETBIOS_LWIP_NAME "WJW-BOARD"

//#define TCP_LISTEN_BACKLOG              0
//#define TCP_DEFAULT_LISTEN_BACKLOG      2


#endif /* __LWIPOPTS_H__ */
