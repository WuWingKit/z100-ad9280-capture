/* SPI-only coordinator: PL master -> MCU slave -> PS DDR -> UART.
 * No TX/RX handshake pins. Hardware Flash isolation is required before use.
 */
#include "xparameters.h"
#include "xgpio.h"
#include "xuartps.h"
#include "xil_printf.h"
#include "xtime_l.h"
#include "sleep.h"
#include <stdint.h>
#define BYTES 47136u
static XGpio io;static XUartPs uart;
static uint32_t control;static uint16_t request_id;
static uint8_t data[BYTES] __attribute__((aligned(64)));
static int primed;
static uint16_t crcword(uint16_t c,uint16_t v){unsigned i;for(i=0;i<16;i++){unsigned b=((c>>15)^((v>>(15-i))&1))&1;c=(uint16_t)(c<<1);if(b)c^=0x1021;}return c;}
static int expired(XTime t,unsigned ms){XTime now;XTime_GetTime(&now);return now-t>(XTime)COUNTS_PER_SECOND*ms/1000;}
static int word(uint16_t tx,uint16_t *rx){XTime t;uint32_t s;
 control^=1;XGpio_DiscreteWrite(&io,1,((uint32_t)tx<<16)|control);XTime_GetTime(&t);
 do{s=XGpio_DiscreteRead(&io,2);if(expired(t,20))return 0;}while((s&2)||((s&1)!=(control&1)));
 *rx=(uint16_t)(s>>16);usleep(100);return 1;
}
/* Response has 7 header words, up to 64 payload bytes, final CRC16. */
static int exchange(uint16_t op,uint32_t offset,uint16_t length,uint8_t *payload,uint16_t *flags){
 uint16_t q[8],h[7],w,c=0xffff,id=++request_id;unsigned i,n;
 q[0]=0xd5a0;q[1]=op;q[2]=id;q[3]=(uint16_t)offset;q[4]=(uint16_t)(offset>>16);q[5]=length;q[6]=0;
 for(i=0;i<7;i++){c=crcword(c,q[i]);}
 q[7]=c;
 for(i=0;i<8;i++)if(!word(q[i],&w))return 0;
 for(i=0;i<64;i++){if(!word(0xffff,&w))return 0;if(w==0x5ad0)break;}
 if(i==64){return 0;}
 h[0]=w;
 for(i=1;i<7;i++)if(!word(0xffff,&h[i]))return 0;
 if(h[1]!=op||h[2]!=id||h[4]!=length||h[5]!=(uint16_t)offset||h[6]!=(uint16_t)(offset>>16))return 0;
 c=0xffff;for(i=0;i<7;i++)c=crcword(c,h[i]);n=(length+1)/2;
 for(i=0;i<n;i++){if(!word(0xffff,&w))return 0;c=crcword(c,w);payload[2*i]=(uint8_t)(w>>8);if(2*i+1<length)payload[2*i+1]=(uint8_t)w;}
 if(!word(0xffff,&w)||w!=c)return 0;
 *flags=h[3];return !(h[3]&0x8000);
}
static int wait_ready(uint16_t *f){XTime t;XTime_GetTime(&t);
 do{if(!exchange(1,0,0,0,f))return 0;if(*f&4)return 0;if(expired(t,5000))return 0;usleep(1000);}while(*f&1);return 1;
}
static void capture(int prime){uint16_t f=0,seq;unsigned off,i;
 if(prime&&primed){xil_printf("ERROR already primed; reset both sessions before p\r\n");return;}
 if(!prime&&!primed){xil_printf("ERROR prime with p first\r\n");return;}
 if(!exchange(2,0,0,0,&f)||!wait_ready(&f)){xil_printf("SPI TRIGGER/STATUS FAILED flags=%04x\r\n",f);return;}
 if(prime){primed=1;xil_printf("SPI PRIME COMPLETE flags=%04x\r\n",f);return;}
 if(!(f&2)){xil_printf("ERROR no previous frame\r\n");return;}
 
 for(off=0;off<BYTES;off+=64){unsigned len=BYTES-off;if(len>64)len=64;
  if(!exchange(3,off,(uint16_t)len,data+off,&f)){xil_printf("SPI READ FAILED offset=%d\r\n",off);return;}
 }
 seq=(uint16_t)data[28]|((uint16_t)data[29]<<8);
 if(!exchange(4,seq,0,0,&f)){xil_printf("SPI ACK FAILED seq=%d\r\n",seq);return;}
 xil_printf("SPI FRAME DDR STORED bytes=%d seq=%d CRC16_PER_CHUNK=PASS\r\n",BYTES,seq);
 xil_printf("BEGIN RADAR_RAW bytes=%d\r\n",BYTES);
 for(i=0;i<BYTES;i++)xil_printf("%02x%s",data[i],i%16==15?"\r\n":" ");
 xil_printf("END RADAR_RAW\r\n");
}
int main(void){XUartPs_Config *u=XUartPs_LookupConfig(XPAR_XUARTPS_0_DEVICE_ID);uint8_t ch,c=0;uint16_t f=0;
 if(!u||XUartPs_CfgInitialize(&uart,u,u->BaseAddress)!=XST_SUCCESS)return 1;
 XUartPs_SetBaudRate(&uart,115200);XUartPs_SetOperMode(&uart,XUARTPS_OPER_MODE_NORMAL);
 if(XGpio_Initialize(&io,XPAR_AXI_GPIO_0_DEVICE_ID)!=XST_SUCCESS)return 2;
 XGpio_SetDataDirection(&io,1,0);XGpio_SetDataDirection(&io,2,~0u);
 XGpio_DiscreteWrite(&io,1,4);usleep(1000);XGpio_DiscreteWrite(&io,1,0);
 xil_printf("Z100 SPI-ONLY MASTER / NO AUTO TRANSFER / FLASH ISOLATION REQUIRED\r\n");
 while(1)if(XUartPs_Recv(&uart,&ch,1)){
  if(ch=='\r'||ch=='\n'){if(c=='s'){
   if(exchange(1,0,0,0,&f))xil_printf("SPI STATUS flags=%04x busy=%d frame=%d fault=%d\r\n",f,f&1,(f>>1)&1,(f>>2)&1);
   else xil_printf("SPI STATUS FAILED\r\n");
  }else if(c=='p')capture(1);else if(c=='c')capture(0);
  else if(c){xil_printf("s=status p=first capture c=capture/read previous frame\r\n");}
  c=0;
  }else if(!c)c=ch;
 }
}
