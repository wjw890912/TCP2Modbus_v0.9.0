/**
  ******************************************************************************
  * @file    netconf.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   Network connection configuration
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "main.h"
#include "netconf.h"
#include "lwipopts.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
#define LCD_DELAY             3000
#define KEY_DELAY 			  3000
#define MAX_DHCP_TRIES        4
#define SELECTED              1
#define NOT_SELECTED		  (!SELECTED)
#define CLIENTMAC6            2


/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct netif netif;
__IO uint32_t TCPTimer = 0;
__IO uint32_t ARPTimer = 0;

#ifdef LWIP_DHCP
__IO uint32_t DHCPfineTimer = 0;
__IO uint32_t DHCPcoarseTimer = 0;
static uint32_t IPaddress = 0;
#endif


uint8_t LedToggle = 4;
uint8_t	Server = 0;

/* Private function prototypes -----------------------------------------------*/
extern void client_init(void);
extern void server_init(void);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
#ifdef MYSELFBOARD
uint8_t macaddress[6]={0,0x26,0x77,0x96,0x58,0xa9};	//MAC 地址
#else
uint8_t macaddress[6]={0,0x26,0x77,0x96,0x58,0x86};	//MAC 地址
#endif

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
void LwIP_Init(void)
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;

  GetMacAddr();	//GET mac

  /* Initializes the dynamic memory heap defined by MEM_SIZE.*/
  mem_init();

  /* Initializes the memory pools defined by MEMP_NUM_x.*/
  memp_init();


#if LWIP_DHCP
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;

#else

 #ifdef MYSELFBOARD
/*设置IP地址。DHCP时强制为0*/
  IP4_ADDR(&ipaddr, 192, 168, 0, 240);
  IP4_ADDR(&netmask, 255, 255, 0, 0);
  IP4_ADDR(&gw, 192, 168, 0, 20);
 #else
/*设置IP地址。DHCP时强制为0*/
  IP4_ADDR(&ipaddr, 192, 168, 254, 24);
  IP4_ADDR(&netmask, 255, 255, 0, 0);
  IP4_ADDR(&gw, 192, 168, 0, 20);

 #endif


#endif

  Set_MAC_Address(macaddress);

  /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
            struct ip_addr *netmask, struct ip_addr *gw,
            void *state, err_t (* init)(struct netif *netif),
            err_t (* input)(struct pbuf *p, struct netif *netif))
    
   Adds your network interface to the netif_list. Allocate a struct
  netif and pass a pointer to this structure as the first argument.
  Give pointers to cleared ip_addr structures when using DHCP,
  or fill them with sane numbers otherwise. The state pointer may be NULL.

  The init function pointer must point to a initialization function for
  your ethernet netif interface. The following code illustrates it's use.*/
  netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  /*  Registers the default network interface.*/
  netif_set_default(&netif);


#if LWIP_DHCP
  /*  Creates a new DHCP client for this interface on the first call.
  Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
  the predefined regular intervals after starting the client.
  You can peek in the netif->dhcp struct for the actual DHCP status.*/
  dhcp_start(&netif);
#endif

  /*  When the netif is fully configured this function must be called.*/
  netif_set_up(&netif);

}

/**
  * @brief  Called when a frame is received
  * @param  None
  * @retval None
  */
void LwIP_Pkt_Handle(void)
{
  /* Read a received packet from the Ethernet buffers and send it to the lwIP for handling */
  ethernetif_input(&netif);
}

/**
  * @brief  LwIP periodic tasks
  * @param  localtime the current LocalTime value
  * @retval None
  */
 extern void TftpTrm250ms(void); //250MS调用一次
 void ETH_Recive_Poll(void);

 
void LwIP_Periodic_Handle(__IO uint32_t localtime)
{
   ETH_Recive_Poll();
  /* TCP periodic process every 250 ms */
  if (localtime - TCPTimer >= 250/*TCP_TMR_INTERVAL*/)
  {

    TCPTimer =  localtime;
    tcp_tmr();
	TftpTrm250ms();
  }
  /* ARP periodic process every 5s */
  if (localtime - ARPTimer >= 5000/*ARP_TMR_INTERVAL*/)
  {
    ARPTimer =  localtime;
    etharp_tmr();
  }

#if LWIP_DHCP
  /* Fine DHCP periodic process every 500ms */
  if (localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
  {
    DHCPfineTimer =  localtime;
    dhcp_fine_tmr();
  }
  /* DHCP Coarse periodic process every 60s */
  if (localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
  {
    DHCPcoarseTimer =  localtime;
    dhcp_coarse_tmr();
  }
#endif

}
 /*
  extern char link;
  extern struct tcp_pcb *wpcb;
  extern void  tcp_client_callback(void);

  uint32_t timeout_tcp=0;
  uint32_t time_connect=0;
  uint32_t SystemRunTime=0;	
void Display_Periodic_Handle(__IO uint32_t localtime)
{ 
				static unsigned char tm=0;
			  
  // 250 ms 
  if (localtime - DisplayTimer >= LCD_TIMER_MSECS)
  {
			 if(tm>240)
			 {
			 	 tm=0;
				SystemRunTime++;
			 }
			 else
			 {
			 tm++;
			 }
    DisplayTimer = localtime;
	timeout_tcp++;
	time_connect++;
		 		//	LINK=1断开连接
				
				   if( time_connect==4*5)	//5s检查一次是否需要重练
				   {
				   			 time_connect=0;
							 					 
							 if((link==1)&&(IPaddress!=0))	  //IP不为空+需要重练 =执行重练
							  {
							 	   timeout_tcp=0;
							 	  tcp_client_callback(); //调用RAW TCP建立
							  }
		  			}
    //We have got a new IP address so update the display 
    if (IPaddress != netif.ip_addr.addr)
    {
      __IO uint8_t iptab[4];
      uint8_t iptxt[20];

      // Read the new IP address 
      IPaddress = netif.ip_addr.addr;

      iptab[0] = (uint8_t)(IPaddress >> 24);
      iptab[1] = (uint8_t)(IPaddress >> 16);
      iptab[2] = (uint8_t)(IPaddress >> 8);
      iptab[3] = (uint8_t)(IPaddress);

      sprintf((char*)iptxt, "   %d.%d.%d.%d    ", iptab[3], iptab[2], iptab[1], iptab[0]);

      // Display the new IP address 
#if LWIP_DHCP
      if (netif.flags & NETIF_FLAG_DHCP)
      {        
				   	
	   // if(Server)
	   // {
	      
		 
		  // Initialize the server application 
	     // server_init(); 
	   // }
	   // else
	   // {
		  
		  // Initialize the client application 
	     // client_init();
	    //}	        
      }
      else
#endif
      {
        // Display the IP address

      }           
    }

#if LWIP_DHCP
    
    else if (IPaddress == 0)
    {
   
      // If no response from a DHCP server for MAX_DHCP_TRIES times 
	  // stop the dhcp client and set a static IP address 
	  if (netif.dhcp->tries > MAX_DHCP_TRIES)
      {
        struct ip_addr ipaddr;
        struct ip_addr netmask;
        struct ip_addr gw;

        dhcp_stop(&netif);

        IP4_ADDR(&ipaddr, 192, 168, 1, 254);
        IP4_ADDR(&netmask, 255, 255, 255, 0);
        IP4_ADDR(&gw, 192, 168, 1, 1);

        netif_set_addr(&netif, &ipaddr , &netmask, &gw);

      }
    }
#endif
  } 
}
  	*/
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
