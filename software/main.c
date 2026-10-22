/* Z100 / Z035 bare-metal capture: ADC -> FIFO -> DMA -> PS DDR -> UART0.
 * c: capture physical ADC; t: 0..99 counter self-test; s: status.
 * Failure stops new captures: reset/relaunch to flush partial frames safely.
 */
#include "xparameters.h"
#include "xaxidma.h"
#include "xgpio.h"
#include "xuartps.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xtime_l.h"
#include "sleep.h"
#include <string.h>

#define CAPTURE_BYTES 100U
#define BUFFER_BYTES 128U
#define TIMEOUT_SECONDS 2U
static XAxiDma dma;
static XGpio gpio;
static XUartPs uart;
static u8 rx_buffer[BUFFER_BYTES] __attribute__((aligned(64)));
static u32 command;
static int failed;

static int timed_out(XTime start) {
    XTime now;
    XTime_GetTime(&now);
    return (now - start) > (XTime)COUNTS_PER_SECOND * TIMEOUT_SECONDS;
}

static void print_status(void) {
    u32 st = XGpio_DiscreteRead(&gpio, 2);
    xil_printf("STATUS gpio=0x%08x ready=%d busy=%d done=%d overflow=%d count=%d DMA_SR=0x%08x\r\n",
        st, (st >> 6) & 1, st & 1, (st >> 2) & 1, (st >> 1) & 1,
        (st >> 8) & 127, XAxiDma_ReadReg(dma.RegBase, XAXIDMA_RX_OFFSET + XAXIDMA_SR_OFFSET));
}

static int capture(int test_mode) {
    XTime start;
    u32 st, dma_status;
    unsigned i, sum = 0, minimum = 255, maximum = 0, errors = 0;
    if (failed) {
        xil_printf("ERROR: previous transfer failed; reset/relaunch before retry.\r\n");
        return XST_FAILURE;
    }
    st = XGpio_DiscreteRead(&gpio, 2);
    if (!(st & 0x40U) || (st & 1U)) {
        xil_printf("NOT_READY: check ADC clock or wait for current capture.\r\n");
        print_status();
        return XST_FAILURE;
    }
    memset(rx_buffer, 0xA5, sizeof(rx_buffer));
    Xil_DCacheFlushRange((UINTPTR)rx_buffer, sizeof(rx_buffer));
    XAxiDma_IntrAckIrq(&dma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
    if (XAxiDma_SimpleTransfer(&dma, (UINTPTR)rx_buffer, CAPTURE_BYTES,
                              XAXIDMA_DEVICE_TO_DMA) != XST_SUCCESS) goto fail;

    // Set mode and settle before toggling the request across clock domains.
    command = (command & 1U) | (test_mode ? 2U : 0U);
    XGpio_DiscreteWrite(&gpio, 1, command);
    usleep(1000);
    command ^= 1U;
    XGpio_DiscreteWrite(&gpio, 1, command);
    XTime_GetTime(&start);
    do {
        st = XGpio_DiscreteRead(&gpio, 2);
        dma_status = XAxiDma_ReadReg(dma.RegBase, XAXIDMA_RX_OFFSET + XAXIDMA_SR_OFFSET);
        if ((st & 0x12U) || (dma_status & XAXIDMA_ERR_ALL_MASK)) goto fail;
        if (timed_out(start)) goto fail;
    } while ((((st >> 3) & 1U) != (command & 1U)) || !(st & 4U) ||
             !(dma_status & XAXIDMA_IRQ_IOC_MASK) ||
             XAxiDma_Busy(&dma, XAXIDMA_DEVICE_TO_DMA));
    usleep(10); // sample_count is stable after done; allow status settling
    st = XGpio_DiscreteRead(&gpio, 2);
    if (((st >> 8) & 127U) != CAPTURE_BYTES) goto fail;
    if (st & 0x80U) xil_printf("WARNING: ADC OTR asserted during this frame.\r\n");
    Xil_DCacheInvalidateRange((UINTPTR)rx_buffer, sizeof(rx_buffer));
    xil_printf("BEGIN ADC100 mode=%s bytes=100\r\n", test_mode ? "RAMP" : "REAL");
    for (i = 0; i < CAPTURE_BYTES; ++i) {
        xil_printf("%02x%s", rx_buffer[i], ((i % 20U) == 19U) ? "\r\n" : " ");
        sum += rx_buffer[i];
        if (rx_buffer[i] < minimum) minimum = rx_buffer[i];
        if (rx_buffer[i] > maximum) maximum = rx_buffer[i];
        if (test_mode && rx_buffer[i] != i) ++errors;
    }
    for (i = CAPTURE_BYTES; i < BUFFER_BYTES; ++i)
        if (rx_buffer[i] != 0xA5U) ++errors;
    xil_printf("END ADC100 sum=%d min=%d max=%d errors=%d\r\n",
               sum, minimum, maximum, errors);
    if (errors) goto fail;
    xil_printf("%s\r\n", test_mode ? "SELFTEST PASS" : "CAPTURE OK (signal quality not assessed)");
    return XST_SUCCESS;
fail:
    failed = 1;
    xil_printf("CAPTURE FAILED: stopping retries to preserve frame alignment.\r\n");
    print_status();
    return XST_FAILURE;
}

int main(void) {
    XAxiDma_Config *dc;
    XUartPs_Config *uc;
    XTime start;
    u8 ch;
    uc = XUartPs_LookupConfig(XPAR_XUARTPS_0_DEVICE_ID);
    if (!uc || XUartPs_CfgInitialize(&uart, uc, uc->BaseAddress) != XST_SUCCESS) return 1;
    XUartPs_SetBaudRate(&uart, 115200);
    XUartPs_SetOperMode(&uart, XUARTPS_OPER_MODE_NORMAL);
    xil_printf("\r\nZ100 ADC100 / DMA S2MM / UART0 115200 8N1\r\n");
    if (XGpio_Initialize(&gpio, XPAR_AXI_GPIO_0_DEVICE_ID) != XST_SUCCESS) return 2;
    XGpio_SetDataDirection(&gpio, 1, 0U);
    XGpio_SetDataDirection(&gpio, 2, 0xFFFFFFFFU);
    command = 0;
    XGpio_DiscreteWrite(&gpio, 1, command);
    dc = XAxiDma_LookupConfig(XPAR_AXIDMA_0_DEVICE_ID);
    if (!dc || XAxiDma_CfgInitialize(&dma, dc) != XST_SUCCESS || XAxiDma_HasSg(&dma)) return 3;
    XAxiDma_Reset(&dma);
    XTime_GetTime(&start);
    while (!XAxiDma_ResetIsDone(&dma)) if (timed_out(start)) return 4;
    XAxiDma_IntrDisable(&dma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
    usleep(100000); // FIFO clocks must be running during reset recovery
    print_status();
    xil_printf("Commands: t=self-test, c=real ADC capture, s=status. Send one character.\r\n");
    while (1) {
        if (XUartPs_Recv(&uart, &ch, 1) == 1U) {
            if (ch == 'c' || ch == 'C') capture(0);
            else if (ch == 't' || ch == 'T') capture(1);
            else if (ch == 's' || ch == 'S') print_status();
        }
    }
}
