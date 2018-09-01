/**
  ******************************************************************************
  * @file    client.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   A sample UDP/TCP client
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
#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>


/* Private typedef -----------------------------------------------------------*/
#define UDP_SERVER_PORT      7
#define UDP_CLIENT_PORT      4
#define TCP_PORT      4

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static struct tcp_pcb *TcpPCB;

/* Private function prototypes -----------------------------------------------*/
void udp_client_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
void tcp_client_err(void *arg, err_t err);
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize the client application.
  * @param  None
  * @retval None
  */
   void  tcp_client_callback(void);
void client_init(void)
{
  // struct udp_pcb *upcb;
  // struct pbuf *p;
    tcp_client_callback();                              
   /* Create a new UDP control block  */
  // upcb = udp_new();   
   
   /* Connect the upcb  */
  // udp_connect(upcb, IP_ADDR_BROADCAST, UDP_SERVER_PORT);

  // p = pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_RAM);

   /* Send out an UDP datagram to inform the server that we have strated a client application */
   //udp_send(upcb, p);   

   /* Reset the upcb */
  // udp_disconnect(upcb);
   
   /* Bind the upcb to any IP address and the UDP_PORT port*/
  // udp_bind(upcb, IP_ADDR_ANY, UDP_CLIENT_PORT);
   
   /* Set a receive callback for the upcb */
   //udp_recv(upcb, udp_client_callback, NULL);

   /* Free the p buffer */
  // pbuf_free(p);
  
}

/**
  * @brief  This function is called when a datagram is received
   * @param arg user supplied argument (udp_pcb.recv_arg)
   * @param upcb the udp_pcb which received data
   * @param p the packet buffer that was received
   * @param addr the remote IP address from which the packet was received
   * @param port the remote port from which the packet was received
  * @retval None
  */
	char link=1;
   struct tcp_pcb *wpcb; 
   extern uint32_t timeout_tcp;
   extern uint32_t SystemRunTime;
   uint32_t retry_TCP_connect=0;
  err_t  tcp_client_reciver(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{

					char *data;
					char UID_STM32[16];
				   	char IDon[7] ={0x67,0x74,0,0,1,0x0d,0x0a};
					char IDoff[7]={0x67,0x74,0,0,0,0x0d,0x0a};
					char IDs[7]  ={0x67,0x74,0,0,2,0x0d,0x0a};

				timeout_tcp=0;
 if (err == ERR_OK && p != NULL)
  {

    /* Inform TCP that we have taken the data. */
    tcp_recved(tpcb, p->tot_len);
		 data = p->payload;
		             if (strncmp(data, "getru", 5) == 0)
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
					 if (strncmp(data, "getid", 5) == 0)
						{
				 		  UID_STM32[0]=0xAB;
						  UID_STM32[1]=0x31;
						  UID_STM32[2]=0x32;
						  UID_STM32[3]=0x33;
						  UID_STM32[4]=0x34;

						  UID_STM32[5]=0x35;
						  UID_STM32[6]=0x36;
						  UID_STM32[7]=0x37;
						  UID_STM32[8]=0x38;
									
						  UID_STM32[9]=0x39;
						  UID_STM32[10]=0x40;
						  UID_STM32[11]=0x41;
						  UID_STM32[12]=0x42;

						  UID_STM32[13]=0xCD;
						  UID_STM32[14]=0x0d;
						  UID_STM32[15]=0x0a;
							 tcp_write(tpcb, &UID_STM32, 16, 1);
							 tcp_output(tpcb);

		                  }
						if(memcmp(&IDoff,data, 5)==0)
						{
						  	tcp_write(tpcb, &IDoff, 7, 1);
							 tcp_output(tpcb);
						
						}
						else if(memcmp(&IDon,data, 5)==0)
						{
							  tcp_write(tpcb, &IDon, 7, 1);
							 tcp_output(tpcb);
						
						}
						else if(memcmp(&IDs,data, 5)==0)
						{
								tcp_write(tpcb, &IDon, 7, 1);
							 tcp_output(tpcb);
						}

	



	   
	 pbuf_free(p);	
		


   }
if ((err == ERR_OK && p == NULL)||(err<0))
  {
	link=1;
    tcp_close(wpcb);

  }		 
  
	return ERR_OK;
} 
 void tcp_client_err(void *arg, err_t err)
 {
 	  if(err!=ERR_ABRT)
	  {
 	 link=1;
   tcp_close(wpcb);
   
 	   }
	   else
	   {
	   	 link=1;
	   }
	 
 }

void tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{


}
	// static char first=0;
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
			
			
			 			  //222.174.153.132
			  IP4_ADDR(&ip_addr, 192,168,0,163);


		
			 
			   // IP4_ADDR(&ip_addr, 192,168,1,3);
			/* Connect to the server: send the SYN *///TCP_PORT
		    tcp_connect(wpcb, &ip_addr, 2301, tcp_client_connected);
			//	tcp_poll(wpcb,tcp_client_poll,2);
			tcp_err( wpcb,tcp_client_err);  //register err
			tcp_recv(wpcb,tcp_client_reciver);  //register recv


		



   

}


void udp_client_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
  struct tcp_pcb *wpcb;
  __IO uint8_t iptab[4];
//  uint8_t iptxt[20];
struct ip_addr ip_addr;	
 

  /* Create a new TCP control block  */
  wpcb = tcp_new();

  /* Assign to the new pcb a local IP address and a port number */
  tcp_bind(wpcb, IP_ADDR_ANY, TCP_PORT);
 
 

   IP4_ADDR(&ip_addr, 192, 168, 1, 2);
  /* Connect to the server: send the SYN *///TCP_PORT
  tcp_connect(wpcb, &ip_addr, 2302, tcp_client_connected);
  tcp_recv(wpcb,tcp_client_reciver);

  /* Free the p buffer */
  pbuf_free(p);
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
  
  	   link=0;
 	   retry_TCP_connect++;
	tpcb->so_options |= SOF_KEEPALIVE;
    tpcb->keep_idle = 500;// ms
    tpcb->keep_intvl = 500;// ms
    tpcb->keep_cnt = 2;// report error after 2 KA without response

   tcp_write(tpcb, "connecting....", 14, 1);
	tcp_output(tpcb);

  TcpPCB = tpcb;
  
  return ERR_OK;
}



/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
