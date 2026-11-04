/* PS SPI0 slave test source. PL clocks MISO via internal EMIO.
 * Fixed 8-byte burst, mode 2, 16MHz SCLK and 1MHz CS cadence.
 * Prepare TX FIFO before START; DMA writes two words into DDR.
 * FFT is intentionally disabled for the 8-byte debug payload.
 */

#include "xparameters.h"
#include "xaxidma.h"
#include "xgpio.h"
#include "xuartps.h"
#include "xspips.h"
#include "xspips_hw.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xtime_l.h"
#include "sleep.h"
#include <stdio.h>
#include <string.h>
#define FS 1000000U
#define MAX_N 4096U

static XAxiDma dma;
static XGpio control, config;
static XUartPs uart;

static u8 buffer[MAX_N + 64] __attribute__((aligned(64)));
static u32 cmd, job;
static int failed;

static XSpiPs source_spi;
static u8 spi_source[MAX_N];
static u32 spi_queued, spi_received, spi_length;
static int spi_fault;
static const signed char sine_lut[256] = {0,3,6,9,12,16,19,22,25,28,31,34,37,40,43,46,49,51,54,57,60,63,65,68,71,73,76,78,81,83,85,88,90,92,94,96,98,100,102,104,106,107,109,111,112,113,115,116,117,118,120,121,122,122,123,124,125,125,126,126,126,127,127,127,127,127,127,127,126,126,126,125,125,124,123,122,122,121,120,118,117,116,115,113,112,111,109,107,106,104,102,100,98,96,94,92,90,88,85,83,81,78,76,73,71,68,65,63,60,57,54,51,49,46,43,40,37,34,31,28,25,22,19,16,12,9,6,3,0,-3,-6,-9,-12,-16,-19,-22,-25,-28,-31,-34,-37,-40,-43,-46,-49,-51,-54,-57,-60,-63,-65,-68,-71,-73,-76,-78,-81,-83,-85,-88,-90,-92,-94,-96,-98,-100,-102,-104,-106,-107,-109,-111,-112,-113,-115,-116,-117,-118,-120,-121,-122,-122,-123,-124,-125,-125,-126,-126,-126,-127,-127,-127,-127,-127,-127,-127,-126,-126,-126,-125,-125,-124,-123,-122,-122,-121,-120,-118,-117,-116,-115,-113,-112,-111,-109,-107,-106,-104,-102,-100,-98,-96,-94,-92,-90,-88,-85,-83,-81,-78,-76,-73,-71,-68,-65,-63,-60,-57,-54,-51,-49,-46,-43,-40,-37,-34,-31,-28,-25,-22,-19,-16,-12,-9,-6,-3};
static int prepare_spi(int ramp, u32 n, u32 k, u32 amplitude) {
    XSpiPs_Config *sc = XSpiPs_LookupConfig(XPAR_XSPIPS_0_DEVICE_ID);
    u32 phase = 0, i;
    if (!sc || XSpiPs_CfgInitialize(&source_spi, sc, sc->BaseAddress) != XST_SUCCESS)
        return XST_FAILURE;
    XSpiPs_Reset(&source_spi);
    
    if (XSpiPs_SetOptions(&source_spi, XSPIPS_CLK_ACTIVE_LOW_OPTION) != XST_SUCCESS) return XST_FAILURE;
    XSpiPs_WriteReg(sc->BaseAddress, XSPIPS_IDR_OFFSET, 0x7FU);
    XSpiPs_WriteReg(sc->BaseAddress, XSPIPS_RXWR_OFFSET, 1);
    for (i = 0; i < n; ++i) {
        spi_source[i] = ramp ? (u8)i : (u8)(128 + sine_lut[phase >> 24] * (int)amplitude / 127);
        phase += k;
    }
    spi_queued = spi_received = 0; spi_length = n; spi_fault = 0;
    XSpiPs_Enable(&source_spi);
    while (spi_queued < n && !(XSpiPs_ReadReg(sc->BaseAddress, XSPIPS_SR_OFFSET) & XSPIPS_IXR_TXFULL_MASK))
        XSpiPs_WriteReg(sc->BaseAddress, XSPIPS_TXD_OFFSET, spi_source[spi_queued++]);
    XSpiPs_WriteReg(sc->BaseAddress, XSPIPS_SR_OFFSET, XSPIPS_IXR_WR_TO_CLR_MASK);
    return XST_SUCCESS;
}
static void service_spi(void) {
    u32 base = source_spi.Config.BaseAddress;
    u32 sr = XSpiPs_ReadReg(base, XSPIPS_SR_OFFSET);
    if (sr & (XSPIPS_IXR_TXUF_MASK | XSPIPS_IXR_RXOVR_MASK | XSPIPS_IXR_MODF_MASK)) spi_fault = 1;
    while (XSpiPs_ReadReg(base, XSPIPS_SR_OFFSET) & XSPIPS_IXR_RXNEMPTY_MASK) {
        u8 byte = (u8)XSpiPs_ReadReg(base, XSPIPS_RXD_OFFSET);
        if (byte != 0xA5 || spi_received >= spi_length) spi_fault = 1;
        spi_received++;
    }
    while (spi_queued < spi_length && !(XSpiPs_ReadReg(base, XSPIPS_SR_OFFSET) & XSPIPS_IXR_TXFULL_MASK))
        XSpiPs_WriteReg(base, XSPIPS_TXD_OFFSET, spi_source[spi_queued++]);
}

static int timeout(XTime start) {
    XTime now;
    XTime_GetTime(&now);
    return now - start > 2U * (XTime)COUNTS_PER_SECOND;
}

static u32 crc32(const u8 *p, unsigned n) {
    u32 c = 0xffffffffU;
    unsigned i, j;
    
    for (i = 0; i < n; i++) {
        c ^= p[i];
        for (j = 0; j < 8; j++)
            c = (c >> 1) ^ ((0U - (c & 1U)) & 0xedb88320U);
    }
    return c ^ 0xffffffffU;
}

static void status(void) {
    u32 s = XGpio_DiscreteRead(&control, 2);
    xil_printf(
        "STATUS raw=%08x busy=%d overflow=%d done=%d ack=%d reject=%d ready=%d otr=%d count=%d state=%d streamed=%d\r\n",
        s, s & 1, (s >> 1) & 1, (s >> 2) & 1, (s >> 3) & 1, (s >> 4) & 1, (s >> 6) & 1,
        (s >> 7) & 1, (s >> 8) & 8191, (s >> 21) & 15, (s >> 25) & 1);
}

static int reset_path(void) {
    XTime t;
    XGpio_DiscreteWrite(&control, 1, 4);
    if (source_spi.IsReady) XSpiPs_Disable(&source_spi);
    usleep(1000);
    cmd = 0;
    XAxiDma_Reset(&dma);
    XTime_GetTime(&t);
    while (!XAxiDma_ResetIsDone(&dma))
        if (timeout(t))
            return XST_FAILURE;
    XAxiDma_IntrDisable(&dma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
    XGpio_DiscreteWrite(&control, 1, 0);
    usleep(100000);
    failed = 0;
    return XST_SUCCESS;
}

static int measure(int ramp, u32 freq, u32 n, u32 delay, u32 amp) {
    XTime t;
    u8 key;
    u32 st, sr, k, params, sum = 0, min = 255, max = 0, errors = 0, i;
    
    if (failed) {
        xil_printf("ERROR reset required: send r then Enter\r\n");
        return 1;
    }
    if (freq < 1000 || freq > 400000 || n != 8 || delay > 255 ||
        amp > 96) {
        xil_printf(
            "ERROR parameters: Hz 1000..400000, N 8 only, delay 0..255, amp 0..96\r\n");
        return 1;
    }
    st = XGpio_DiscreteRead(&control, 2);
    if (!(st & 64) || (st & 1)) {
        xil_printf("ERROR not idle/ready\r\n");
        status();
        return 1;
    }
    
    k = (u32)((((u64)freq << 32) + FS / 2) / FS);
    params = n | (delay << 13) | (amp << 21);
    XGpio_DiscreteWrite(&config, 1, k);
    XGpio_DiscreteWrite(&config, 2, params);
    usleep(1000); 
    
    memset(buffer, 0xa5, sizeof(buffer));
    Xil_DCacheFlushRange((UINTPTR)buffer, sizeof(buffer));
    XAxiDma_IntrAckIrq(&dma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
    if (XAxiDma_SimpleTransfer(&dma, (UINTPTR)buffer, n, XAXIDMA_DEVICE_TO_DMA) != XST_SUCCESS)
        goto fail;
    
    if (prepare_spi(ramp, n, k, amp) != XST_SUCCESS) goto fail;
    cmd = (cmd & 17U) | (ramp ? 2U : 0U) | 8U;
    cmd ^= 1;
    XGpio_DiscreteWrite(&control, 1, cmd);
    XTime_GetTime(&t);
    
    do {
        service_spi();
        if (spi_fault) goto fail;
        if (XUartPs_Recv(&uart, &key, 1) == 1 && key == 'q') {
            if (reset_path() != XST_SUCCESS)
                goto fail;
            xil_printf("MEASUREMENT ABORTED; SPI path reset, CS idle high\r\n");
            return 1;
        }
        st = XGpio_DiscreteRead(&control, 2);
        sr = XAxiDma_ReadReg(dma.RegBase, XAXIDMA_RX_OFFSET + XAXIDMA_SR_OFFSET);
        
        if ((st & 18U) || (sr & XAXIDMA_ERR_ALL_MASK) || timeout(t))
            goto fail;
    } while (((st >> 3) & 1) != (cmd & 1) || !(st & 4) || !(sr & XAXIDMA_IRQ_IOC_MASK) ||
             XAxiDma_Busy(&dma, XAXIDMA_DEVICE_TO_DMA) || spi_received != n);
    
    usleep(10);
    st = XGpio_DiscreteRead(&control, 2);
    if (((st >> 8) & 8191) != n || !(st & (1U << 25)))
        goto fail;
    Xil_DCacheInvalidateRange((UINTPTR)buffer, sizeof(buffer));
    
    for (i = 0; i < n; i++) {
        if (buffer[i] != spi_source[i]) errors++;
        sum += buffer[i];
        if (buffer[i] < min)
            min = buffer[i];
        if (buffer[i] > max)
            max = buffer[i];
        if (ramp && buffer[i] != (u8)i)
            errors++;
    }
    for (i = n; i < sizeof(buffer); i++)
        if (buffer[i] != 0xa5)
            errors++;

    cmd ^= 16;
    cmd &= ~8U;
    XGpio_DiscreteWrite(&control, 1, cmd);
    XTime_GetTime(&t);
    while (XGpio_DiscreteRead(&control, 2) & 1)
        if (timeout(t))
            goto fail;
    
    xil_printf("BEGIN MEAS job=%d mode=%s samples=%d fs=%d freq=%d k=%d delay=%d amp=%d otr=%d\r\n",
               ++job, ramp ? "RAMP" : "SPI_SIM", n, FS, freq, k, delay, amp, (st >> 7) & 1);
    for (i = 0; i < n; i++)
        xil_printf("%02x%s", buffer[i], (i % 16 == 15) ? "\r\n" : " ");
    xil_printf("END MEAS sum=%d min=%d max=%d errors=%d crc=%08x\r\n", sum, min, max, errors,
               crc32(buffer, n));
    if (errors)
        goto fail;
    xil_printf("%s\r\n", ramp ? "SELFTEST PASS" : "MEASUREMENT OK (PS SPI simulated source)");
    return 0;
    
fail:
    failed = 1;
    xil_printf("MEASUREMENT FAILED: preserve log, send r to reset entire path\r\n");
    status();
    return 1;
}

static void handle(char *line) {
    unsigned f = 100000, n = 8, d = 3, a = 24;
    if (line[0] == 'm' || line[0] == 'c' || line[0] == 't') {
        if (line[1] && sscanf(line + 1, "%u %u %u %u", &f, &n, &d, &a) != 4) {
            xil_printf("ERROR use m Hz N delay amp\r\n");
            return;
        }
        measure(line[0] == 't', f, n, d, a);
    } else if (line[0] == 'r') {
        xil_printf(reset_path() == XST_SUCCESS ? "RESET OK\r\n" : "RESET FAILED\r\n");
    } else if (line[0] == 'q') {
        xil_printf(reset_path() == XST_SUCCESS ? "STOPPED / RESET OK\r\n" : "RESET FAILED\r\n");
    } else if (line[0] == 's')
        status();
    else
        xil_printf(
            "m Hz N delay amp = real; t [Hz N delay amp] = ramp; s=status; r=reset; q=stop/reset (q alone also aborts capture). Enter required.\r\n");
}

int main(void) {
    XAxiDma_Config *dc;
    XUartPs_Config *uc;
    u8 ch;
    char line[96];
    unsigned used = 0;
    uc = XUartPs_LookupConfig(XPAR_XUARTPS_0_DEVICE_ID);
    if (!uc || XUartPs_CfgInitialize(&uart, uc, uc->BaseAddress) != XST_SUCCESS)
        return 1;
    XUartPs_SetBaudRate(&uart, 115200);
    XUartPs_SetOperMode(&uart, XUARTPS_OPER_MODE_NORMAL);
    if (XGpio_Initialize(&control, XPAR_AXI_GPIO_0_DEVICE_ID) != XST_SUCCESS)
        return 2;
    if (XGpio_Initialize(&config, XPAR_AXI_GPIO_CONFIG_DEVICE_ID) != XST_SUCCESS)
        return 3;
    XGpio_SetDataDirection(&control, 1, 0);
    XGpio_SetDataDirection(&control, 2, 0xffffffffU);
    XGpio_SetDataDirection(&config, 1, 0);
    XGpio_SetDataDirection(&config, 2, 0);
    dc = XAxiDma_LookupConfig(XPAR_AXIDMA_0_DEVICE_ID);
    if (!dc || XAxiDma_CfgInitialize(&dma, dc) != XST_SUCCESS || XAxiDma_HasSg(&dma))
        return 4;
    if (reset_path() != XST_SUCCESS)
        return 5;
    xil_printf("\r\nZ100 PS-SPI-SLAVE LOOP v1 / UART0 115200 8N1 / NO AUTO START\r\n");
    handle("h");
    status();
    
    while (1)
        if (XUartPs_Recv(&uart, &ch, 1) == 1) {
            if (ch == '\r' || ch == '\n') {
                if (used) {
                    line[used] = 0;
                    handle(line);
                    used = 0;
                }
            } else if (used < sizeof(line) - 1)
                line[used++] = ch;
            else {
                used = 0;
                xil_printf("ERROR command too long\r\n");
            }
        }
}
