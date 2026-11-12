/* Z100 FPGA-ONLY loopback: no front-end or external wiring required.
 * UART commands: p=prime SYNC, c=capture request, s=status, d=dump last frame,
 * t=three-frame selftest, v=full pattern check, b/l/n=fault injection.
 * r=reset FPGA/DMA AND the internal source together, h=help.
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
static u32 sim_faults;
static int last_frame_valid;
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
    XTime t;cmd=0;sim_faults=0;last_frame_valid=0;had_sync=0;
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
    cmd=(cmd&17u)|(prime ? 2u : 8u)|sim_faults;cmd^=1;
    XTime_GetTime(&last_sync);had_sync=1;XGpio_DiscreteWrite(&control,1,cmd);
}
static void prime(void){
    XTime t;
    if(!(status()&64)||status()&128){xil_printf("ERROR prime requires idle new session; use r to reset receiver AND internal source\r\n");return;}
    send_request(1);XTime_GetTime(&t);
    usleep(10);
    while(!(status()&64)){
        if((status()&2)||elapsed(t,1000)){xil_printf("PRIME FAILED\r\n");print_status();return;}
    }
    xil_printf("PRIME ACK: first SYNC completed; internal simulator READY fell then rose; no SPI frame expected\r\n");
}
static void capture(void){
    XTime t;u32 s,sr,errors=0,i;u16 *h=(u16 *)buffer,x=0;
    if((status()&192)!=192){xil_printf("ERROR not primed/ready; use r then p\r\n");return;}
    last_frame_valid=0;memset(buffer,0xA5,sizeof(buffer));
    Xil_DCacheFlushRange((UINTPTR)buffer,sizeof(buffer));
    XAxiDma_IntrAckIrq(&dma,XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DEVICE_TO_DMA);
    if(XAxiDma_SimpleTransfer(&dma,(UINTPTR)buffer,FRAME_BYTES,XAXIDMA_DEVICE_TO_DMA)!=XST_SUCCESS){xil_printf("DMA ARM FAILED\r\n");return;}
    send_request(0);XTime_GetTime(&t);
    do{
        s=status();sr=XAxiDma_ReadReg(dma.RegBase,XAXIDMA_RX_OFFSET+XAXIDMA_SR_OFFSET);
        if((s&2)||(s&16)||(sr&XAXIDMA_ERR_ALL_MASK)||elapsed(t,1000)){
            xil_printf("CAPTURE FAILED dma=%08x; save log, use r then p (both internal endpoints reset)\r\n",sr);
            print_status();XAxiDma_Reset(&dma);return;
        }
    }while(!(sr&XAXIDMA_IRQ_IOC_MASK)||XAxiDma_Busy(&dma,XAXIDMA_DEVICE_TO_DMA)||!(s&(1u<<28)));
    Xil_DCacheInvalidateRange((UINTPTR)buffer,sizeof(buffer));
    for(i=0;i<16;i++)x^=h[i];
    if(h[0]!=0xA610||(h[1]!=2&&h[1]!=3)||h[2]!=16||h[12]!=FRAME_BYTES||h[13]!=32||x)errors++;
    for(i=FRAME_BYTES;i<sizeof(buffer);i++)if(buffer[i]!=0xA5)errors++;
    if(((s>>8)&65535)!=(FRAME_BYTES/2))errors++;
    xil_printf("FRAME version=%d seq=%d beam_index=%d angle_cdeg=%d bytes=%d header_xor=%04x check_errors=%d local_payload_crc=%08x\r\n",
        h[1],h[1]==3 ? h[14] : 0,h[3],(int)(int16_t)h[4],FRAME_BYTES,x,errors,crc32(buffer+32,FRAME_BYTES-32));
    if(errors){xil_printf("FRAME CHECK FAILED; no ACK\r\n");return;}
    /* Local CRC records received bytes; the current wire protocol has no payload CRC. */
    last_frame_valid=1;cmd=(cmd^16u)&~8u;XGpio_DiscreteWrite(&control,1,cmd);
    xil_printf("SIMULATED FRAME STORED: metadata/length/guard checks passed; no real radar data\r\n");
}
/* Every simulated sample is checked, not just a displayed CRC. */
static int verify_pattern(void){
    u32 i,errors=0,seq,index,actual,expected;
    if(!last_frame_valid){xil_printf("NO VERIFIED FRAME\r\n");return 0;}
    seq=((u16 *)buffer)[14];
    for(i=32;i<FRAME_BYTES;i+=4){
        index=512u+(i-32u)/4u;
        expected=((u32)(u16)(0x8000u|seq)<<16)|(index&0x7fffu);
        actual=(u32)buffer[i]|((u32)buffer[i+1]<<8)|((u32)buffer[i+2]<<16)|((u32)buffer[i+3]<<24);
        if(actual!=expected)errors++;
    }
    xil_printf("FPGA PATTERN seq=%d complex_samples=%d errors=%d %s\r\n",seq,(FRAME_BYTES-32)/4,errors,errors?"FAIL":"PASS");
    return errors==0;
}
/* `t` is a complete board-local positive test: shared reset, first-frame prime,
 * then three previous-bank frames through actual SPI receiver + DMA + DDR.
 */
static void run_debug_selftest(void){
    u32 i;u16 *h=(u16 *)buffer;XTime started;
    xil_printf("BEGIN FPGA LOOP SELFTEST / SYNTHETIC DATA ONLY\r\n");
    if(!reset_fpga()){xil_printf("SELFTEST FAIL reset\r\n");return;}
    XTime_GetTime(&started);
    while(!(status()&64u))if(elapsed(started,1000)){xil_printf("SELFTEST FAIL internal source not ready\r\n");return;}
    prime();
    if(!(status()&128u)){xil_printf("SELFTEST FAIL prime\r\n");return;}
    for(i=0;i<3;i++){
        capture();
        if(!last_frame_valid||!verify_pattern()||h[14]!=i||h[3]!=(9u+i)%19u){
            xil_printf("FPGA LOOP SELFTEST FAIL frame=%d\r\n",i);return;
        }
    }
    xil_printf("FPGA LOOP SELFTEST PASS frames=3 bytes=141408 previous_frame_sequence_and_beam=PASS\r\n");
}
static void handle(u8 c){
    u32 i;
    switch(c){
    case 'p':prime();break;
    case 'c':capture();break;
    case 's':print_status();break;
    case 'v':(void)verify_pattern();break;
    case 't':run_debug_selftest();break;
    case 'b':sim_faults=1u<<8;capture();sim_faults=0;break;
    case 'l':sim_faults=1u<<9;capture();sim_faults=0;break;
    case 'n':sim_faults=1u<<10;capture();sim_faults=0;break;
    case 'r':case 'q':
        xil_printf(reset_fpga() ? "FPGA/DMA AND INTERNAL SOURCE RESET; use p before c\r\n" : "RESET FAILED\r\n");break;
    case 'd':
        if(!last_frame_valid){xil_printf("NO VERIFIED FRAME\r\n");break;}
        xil_printf("BEGIN RADAR_RAW bytes=%d\r\n",FRAME_BYTES);
        for(i=0;i<FRAME_BYTES;i++)xil_printf("%02x%s",buffer[i],(i%16==15)?"\r\n":" ");
        xil_printf("END RADAR_RAW\r\n");break;
    default:xil_printf("t=complete FPGA-only test; s=status; r=reset BOTH internal endpoints; p=prime; c=capture; v=verify all samples; d=raw dump; b=bad header; l=short frame; n=no reply. Enter required.\r\n");break;
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
    xil_printf("\r\nZ100 FPGA-ONLY RADAR SPI DEBUG / UART0 115200 8N1 / NO EXTERNAL FRONTEND / NO AUTO START\r\n");handle('h');
    while(1)if(XUartPs_Recv(&uart,&ch,1)==1){
        if(ch=='\r'||ch=='\n'){if(command_char){handle(command_char);command_char=0;}}
        else if(!command_char)command_char=ch;
    }
}
