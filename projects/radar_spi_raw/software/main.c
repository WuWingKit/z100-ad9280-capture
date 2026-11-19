/* Z100 external radar SPI / FPGA-generated SYNC / complete-frame DDR receiver.
 * UART commands: p=prime, c=external capture + IQ statistics + full UART dump,
 * r=reset FPGA/DMA (MCU session must be reset separately), h=help.
 * No automatic hardware activity or Flash programming.
 */
#include "xparameters.h"
#include "xgpio.h"
#include "xaxidma.h"
#include "xuartps.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xtime_l.h"
#include "sleep.h"
#include <stdint.h>
#include <string.h>
#define FRAME_BYTES 47136u
#define BUFFER_BYTES 47296u
#define CLOCK_HZ 100000000u
static XGpio control,config;
static XAxiDma dma;
static XUartPs uart;
static u8 buffer[BUFFER_BYTES] __attribute__((aligned(64)));
static u32 cmd;
static int last_frame_valid;
static unsigned dump_bytes;
static XTime last_sync;
static int had_sync;
static u32 status(void){return XGpio_DiscreteRead(&control,2);}
static int elapsed(XTime t,unsigned ms){XTime now;XTime_GetTime(&now);return now-t>(XTime)COUNTS_PER_SECOND*ms/1000;}
static u32 crc32(const u8 *p,unsigned n){u32 c=~0u;unsigned i,j;for(i=0;i<n;i++){c^=p[i];for(j=0;j<8;j++)c=(c>>1)^((0u-(c&1u))&0xEDB88320u);}return ~c;}
static void print_status(void){
    u32 s=status(),state=(s>>24)&15;
    xil_printf("STATUS raw=%08x busy=%d error=%d ready=%d primed=%d mcu_ready=%d state=%d words_or_fault_low8=%d fault_code=%02x\r\n",
      s,s&1,(s>>1)&1,(s>>6)&1,(s>>7)&1,(s>>31)&1,state,(s>>8)&(state==7 ? 255 : 65535),state==7 ? (s>>16)&255 : 0);
}
static int reset_fpga(void){
    XTime t;cmd=0;last_frame_valid=0;had_sync=0;
    XGpio_DiscreteWrite(&control,1,4);
    XAxiDma_Reset(&dma);XTime_GetTime(&t);
    while(!XAxiDma_ResetIsDone(&dma))if(elapsed(t,500))return 0;
    XAxiDma_IntrDisable(&dma,XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DEVICE_TO_DMA);
    usleep(1000);XGpio_DiscreteWrite(&control,1,0);usleep(1000);
    return 1;
}
static void send_request(int prime){
    /* Never schedule faster than 50ms between SYNC rising edges. */
    while(had_sync&&!elapsed(last_sync,50))usleep(100);
    cmd=(cmd&17u)|(prime ? 2u : 8u);cmd^=1;
    XTime_GetTime(&last_sync);had_sync=1;XGpio_DiscreteWrite(&control,1,cmd);
}
static void prime(void){
    XTime t;
    if(!(status()&64)||status()&128){xil_printf("ERROR prime requires idle new session; reset BOTH MCU and FPGA first\r\n");return;}
    send_request(1);XTime_GetTime(&t);
    usleep(10);
    while(!(status()&64)){
        if((status()&2)||elapsed(t,1000)){xil_printf("PRIME FAILED\r\n");print_status();return;}
    }
    xil_printf("PRIME ACK: first SYNC completed; MCU READY fell then rose; no SPI frame expected\r\n");
}
static void capture(void){
    XTime t;u32 s,sr,errors=0,i;
    last_frame_valid=0;
    if((status()&192)!=192){xil_printf("ERROR not primed/ready; use p only after MCU session reset\r\n");return;}
    last_frame_valid=0;memset(buffer,0xA5,sizeof(buffer));
    Xil_DCacheFlushRange((UINTPTR)buffer,sizeof(buffer));
    XAxiDma_IntrAckIrq(&dma,XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DEVICE_TO_DMA);
    if(XAxiDma_SimpleTransfer(&dma,(UINTPTR)buffer,FRAME_BYTES,XAXIDMA_DEVICE_TO_DMA)!=XST_SUCCESS){xil_printf("DMA ARM FAILED\r\n");return;}
    send_request(0);XTime_GetTime(&t);
    do{
        s=status();sr=XAxiDma_ReadReg(dma.RegBase,XAXIDMA_RX_OFFSET+XAXIDMA_SR_OFFSET);
        if((s&2)||(s&16)||(sr&XAXIDMA_ERR_ALL_MASK)||elapsed(t,1000)){
            xil_printf("CAPTURE FAILED dma=%08x; save log, wait SPI idle and reset BOTH endpoints before reprime\r\n",sr);
            print_status();XAxiDma_Reset(&dma);return;
        }
    }while(!(sr&XAXIDMA_IRQ_IOC_MASK)||XAxiDma_Busy(&dma,XAXIDMA_DEVICE_TO_DMA)||!(s&(1u<<28)));
    Xil_DCacheInvalidateRange((UINTPTR)buffer,sizeof(buffer));
    for(i=FRAME_BYTES;i<sizeof(buffer);i++)if(buffer[i]!=0xA5)errors++;
    dump_bytes=(s>>8)&65535;
    xil_printf("RAW_CAPTURE stored_bytes=%d guard_errors=%d local_crc=%08x\r\n",dump_bytes,errors,crc32(buffer,dump_bytes));
    for(i=0;i<4;i++){
        XGpio_DiscreteWrite(&control,1,cmd|128u|(i<<5));usleep(10);
        xil_printf("RAW_DIAG page=%d value=%08x\r\n",i,status());
    }
    XGpio_DiscreteWrite(&control,1,cmd);
    last_frame_valid=errors==0;cmd=(cmd^16u)&~8u;XGpio_DiscreteWrite(&control,1,cmd);

}
/* Signed IQ is a two-complement interpretation, not calibrated voltage.
 * Accumulators fit signed 32 bits: 11776 * 32768 < 2^31.
 */
/* Text HEX retains the exact wire header and raw IQ bytes. The PC export
 * utility converts this losslessly to binary, metadata JSON and IQ CSV.
 * This transfer takes about 13 seconds at 115200 baud; no next SYNC is issued
 * during output. RX commands q/r are handled after this blocking dump ends.
 */
static void dump_frame(void){
    unsigned i;
    if(!last_frame_valid){xil_printf("NO VERIFIED FRAME\r\n");return;}
    xil_printf("BEGIN RADAR_RAW bytes=%d\r\n",dump_bytes);
    for(i=0;i<dump_bytes;i++)xil_printf("%02x%s",buffer[i],(i%16==15)?"\r\n":" ");
    xil_printf("END RADAR_RAW\r\n");
}
static void handle(u8 c){
    switch(c){
    case 'p':prime();break;
    case 'c':capture();if(last_frame_valid){dump_frame();}break;
    case 's':print_status();break;
    case 'r':case 'q':
        xil_printf(reset_fpga() ? "FPGA/DMA RESET; MCU RAM/state NOT reset, reset MCU separately before p\r\n" : "RESET FAILED\r\n");break;
    case 'd':dump_frame();break;
    default:xil_printf("p=prime SYNC; c=receive external SPI + IQ summary + full UART output; s=status; d=raw dump; r=FPGA reset. Enter required.\r\n");break;
    }
}
int main(void){
    XUartPs_Config *uc=XUartPs_LookupConfig(XPAR_XUARTPS_0_DEVICE_ID);
    XAxiDma_Config *dc;
    u8 ch,command_char=0;
    if(!uc||XUartPs_CfgInitialize(&uart,uc,uc->BaseAddress)!=XST_SUCCESS)return 1;
    XUartPs_SetBaudRate(&uart,115200);XUartPs_SetOperMode(&uart,XUARTPS_OPER_MODE_NORMAL);
    if(XGpio_Initialize(&control,XPAR_AXI_GPIO_0_DEVICE_ID)!=XST_SUCCESS||XGpio_Initialize(&config,XPAR_AXI_GPIO_CONFIG_DEVICE_ID)!=XST_SUCCESS)return 2;
    XGpio_SetDataDirection(&control,1,0);XGpio_SetDataDirection(&control,2,~0u);
    XGpio_SetDataDirection(&config,1,0);XGpio_SetDataDirection(&config,2,0);
    XGpio_DiscreteWrite(&config,1,CLOCK_HZ/20);XGpio_DiscreteWrite(&config,2,CLOCK_HZ/5);
    dc=XAxiDma_LookupConfig(XPAR_AXIDMA_0_DEVICE_ID);
    if(!dc||XAxiDma_CfgInitialize(&dma,dc)!=XST_SUCCESS||XAxiDma_HasSg(&dma))return 3;
    if(!reset_fpga())return 4;
    xil_printf("\r\nZ100 RADAR SPI RAW / UART0 115200 8N1 / EXTERNAL SOURCE ONLY / NO AUTO SYNC\r\n");handle('h');
    while(1)if(XUartPs_Recv(&uart,&ch,1)==1){
        if(ch=='\r'||ch=='\n'){if(command_char){handle(command_char);command_char=0;}}
        else if(!command_char)command_char=ch;
    }
}
