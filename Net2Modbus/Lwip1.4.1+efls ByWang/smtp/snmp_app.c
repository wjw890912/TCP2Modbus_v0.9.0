 #include "main.h"
 #include "smtp.h"

 #ifdef USED_SMTP
 
extern uint32_t smtp_Tcp_count[10];
 void my_smtp_result_fn(void *arg, u8_t smtp_result, u16_t srv_err, err_t err)
  {			
			  smtp_Tcp_count[4]|= (smtp_result);
			  smtp_Tcp_count[4]|= (srv_err<<8);
			  smtp_Tcp_count[4]|= (err<<24);

			  if(err==ERR_OK){smtp_Tcp_count[5]++;}

			   if(arg!=(void*)0)
			   {
				smtp_Tcp_count[6]=0xAAAAAAAA ;
			   }
			   else
			   {
			   
			   smtp_Tcp_count[6]=0x55555555 ;
			   }
			   //·¢ËÍÍê³É
   /* printf("mail (%p) sent with results: 0x%02x, 0x%04x, 0x%08x\n", arg,
           smtp_result, srv_err, err); */
}

void my_smtp_test(char *str)
{

	   smtp_Tcp_count[8]++;
    smtp_set_server_addr("220.181.12.18"/*"192.168.0.163"*/);
  //  -> set both username and password as NULL if no auth needed
    smtp_set_auth("wjw890912@163.com","wjw86831414718");

    smtp_send_mail("wjw890912@163.com", "wjw890912@163.com", "I am from MCU pass the SMTP", (const char *) str/*"DS18B20"*/, my_smtp_result_fn,
                   0);
	
}

#endif										   