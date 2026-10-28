/* SDK2018.3 bare-metal. CORDIC is in PL; FFT/plotting runs on PC.
 * No auto-start; no Flash programming. Commands require Enter.
 */
/* ===== 1. BSP 驱动与基本参数 =====
 * Xilinx SDK 2018.3 裸机 C；不是 PC 程序，也不运行 FFT。
 * PS 负责配置/检查，逐点发波与采样由 PL 硬件实现。
 */
#include "xparameters.h"
#include "xaxidma.h"
#include "xgpio.h"
#include "xuartps.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xtime_l.h"
#include "sleep.h"
#include <stdio.h>
#include <string.h>
#define FS 10000000U
#define MAX_N 4096U
/* ===== 2. 驱动实例与会话状态 =====
 * control: GPIO0，通道1命令、通道2状态。
 * config: 新配置GPIO，通道1频率字K、通道2打包参数。
 */
static XAxiDma dma;
static XGpio control, config;
static XUartPs uart;
/* DMA 缓冲按 Cache line 对齐；最大4096字节，额外64字节用于越界保护。
 * cmd 保留START/ACK的上次电平，下一次触发通过异或翻转，不是发短脉冲。
 */
static u8 buffer[MAX_N + 64] __attribute__((aligned(64)));
static u32 cmd, job;
static int failed;
/* ===== 3. 公共工具 =====
 * 使用全局计时器判断2秒超时，避免永久卡在DMA或跨域确认等待中。
 */
static int timeout(XTime start) {
    XTime now;
    XTime_GetTime(&now);
    return now - start > 2U * (XTime)COUNTS_PER_SECOND;
}
/* IEEE CRC32：初值/终值异或0xffffffff，反射多项式0xedb88320。
 * 只覆盖采样字节，不覆盖文本元数据；PC与HTML使用同一算法。
 */
static u32 crc32(const u8 *p, unsigned n) {
    u32 c = 0xffffffffU;
    unsigned i, j;
    /* 逐字节更新CRC：每字节再做8次最低位移出/多项式异或。 */
    for (i = 0; i < n; i++) {
        c ^= p[i];
        for (j = 0; j < 8; j++)
            c = (c >> 1) ^ ((0U - (c & 1U)) & 0xedb88320U);
    }
    return c ^ 0xffffffffU;
}
/* 将硬件状态位译为串口文本。count 为完成后的诊断值，非实时原子快照。 */
static void status(void) {
    u32 s = XGpio_DiscreteRead(&control, 2);
    xil_printf(
        "STATUS raw=%08x busy=%d overflow=%d done=%d ack=%d reject=%d ready=%d otr=%d count=%d state=%d streamed=%d\r\n",
        s, s & 1, (s >> 1) & 1, (s >> 2) & 1, (s >> 3) & 1, (s >> 4) & 1, (s >> 6) & 1,
        (s >> 7) & 1, (s >> 8) & 8191, (s >> 21) & 15, (s >> 25) & 1);
}
/* ===== 4. 整条路径复位 =====
 * 先保持PL采集/FIFO复位，再复位DMA；两边都恢复后才能接下一帧。
 * 不要仅清一个完成标志后继续使用可能残留半帧的数据。
 */
static int reset_path(void) {
    XTime t;
    XGpio_DiscreteWrite(&control, 1, 4);
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
/* ===== 5. 执行一次测量任务 =====
 * 顺序：检查 → 写参数 → 准备DMA → START → 等完成 → 验证DDR → ACK → 串口输出。
 * ramp=1 使用PL内部递增码自检；ramp=0 读取真实AD并启用DA正弦输出。
 */
static int measure(int ramp, u32 freq, u32 n, u32 delay, u32 amp) {
    XTime t;
    u8 key;
    u32 st, sr, k, params, sum = 0, min = 255, max = 0, errors = 0, i;
    /* 5.1 上次失败后不允许直接再次启动，须先r复位；限制用户参数。 */
    if (failed) {
        xil_printf("ERROR reset required: send r then Enter\r\n");
        return 1;
    }
    if (freq < 1000 || freq > 1000000 || n < 256 || n > MAX_N || (n & (n - 1)) || delay > 255 ||
        amp > 96) {
        xil_printf(
            "ERROR parameters: Hz 1000..1000000, N 256/512/1024/2048/4096, delay 0..255, amp 0..96\r\n");
        return 1;
    }
    st = XGpio_DiscreteRead(&control, 2);
    if (!(st & 64) || (st & 1)) {
        xil_printf("ERROR not idle/ready\r\n");
        status();
        return 1;
    }
    /* 5.2 K=round(freq*2^32/FS)，使用64位整数防止左移/乘法溢出。
     * 参数布局：N[12:0]，delay[20:13]，amp[27:21]。
     */
    k = (u32)((((u64)freq << 32) + FS / 2) / FS);
    params = n | (delay << 13) | (amp << 21);
    XGpio_DiscreteWrite(&config, 1, k);
    XGpio_DiscreteWrite(&config, 2, params);
    usleep(1000); /* bundled parameters settle before request-toggle crossing */
    /* 5.3 填保护字节，并把CPU Cache中的缓存内容写回DDR。
     * DMA访问物理DDR，不能只依靠CPU Cache中的新值。
     */
    memset(buffer, 0xa5, sizeof(buffer));
    Xil_DCacheFlushRange((UINTPTR)buffer, sizeof(buffer));
    XAxiDma_IntrAckIrq(&dma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
    if (XAxiDma_SimpleTransfer(&dma, (UINTPTR)buffer, n, XAXIDMA_DEVICE_TO_DMA) != XST_SUCCESS)
        goto fail;
    /* 5.4 DMA提交成功后才START：17保留bit0/bit4，8设置dma_armed，异或1翻转START。 */
    cmd = (cmd & 17U) | (ramp ? 2U : 0U) | 8U;
    cmd ^= 1;
    XGpio_DiscreteWrite(&control, 1, cmd);
    XTime_GetTime(&t);
    /* 5.5 等START被确认、采集完成和DMA实际完成；三者缺一不可。
     * 轮询期间只识别q中止，其它输入不排队；请等一帧结束再发下一条命令。
     */
    do {
        if (XUartPs_Recv(&uart, &key, 1) == 1 && key == 'q') {
            if (reset_path() != XST_SUCCESS)
                goto fail;
            xil_printf("MEASUREMENT ABORTED; path reset, DAC parked\r\n");
            return 1;
        }
        st = XGpio_DiscreteRead(&control, 2);
        sr = XAxiDma_ReadReg(dma.RegBase, XAXIDMA_RX_OFFSET + XAXIDMA_SR_OFFSET);
        /* 18 = bit1 overflow | bit4 rejected；DMA自身错误和超时也中止。 */
        if ((st & 18U) || (sr & XAXIDMA_ERR_ALL_MASK) || timeout(t))
            goto fail;
    } while (((st >> 3) & 1) != (cmd & 1) || !(st & 4) || !(sr & XAXIDMA_IRQ_IOC_MASK) ||
             XAxiDma_Busy(&dma, XAXIDMA_DEVICE_TO_DMA));
    /* 5.6 完成后给跨域诊断计数稳定时间；随后使CPU重新从DDR取数据。
     * 8191是13位样本计数掩码，bit25是AXIS包尾已传输的锁存标志。
     */
    usleep(10);
    st = XGpio_DiscreteRead(&control, 2);
    if (((st >> 8) & 8191) != n || !(st & (1U << 25)))
        goto fail;
    Xil_DCacheInvalidateRange((UINTPTR)buffer, sizeof(buffer));
    /* 5.7 统计采样码，RAMP逐点核对，再检查末尾0xa5保护区。 */
    for (i = 0; i < n; i++) {
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
    /* ACK means actual DMA completion, never merely TLAST or capture_done. */
    /* 5.8 异或16翻转DMA完成ACK，清bit3允许PL恢复IDLE并将DA停回中点。 */
    cmd ^= 16;
    cmd &= ~8U;
    XGpio_DiscreteWrite(&control, 1, cmd);
    XTime_GetTime(&t);
    while (XGpio_DiscreteRead(&control, 2) & 1)
        if (timeout(t))
            goto fail;
    /* 5.9 ASCII协议：BEGIN元数据 → 每行16个十六进制字节 → END统计/CRC → 成功标志。
     * 页面必须收到完整成功帧才当作有效结果；打印发生在DMA完成和发波停止之后。
     */
    xil_printf("BEGIN MEAS job=%d mode=%s samples=%d fs=%d freq=%d k=%d delay=%d amp=%d otr=%d\r\n",
               ++job, ramp ? "RAMP" : "REAL", n, FS, freq, k, delay, amp, (st >> 7) & 1);
    for (i = 0; i < n; i++)
        xil_printf("%02x%s", buffer[i], (i % 16 == 15) ? "\r\n" : " ");
    xil_printf("END MEAS sum=%d min=%d max=%d errors=%d crc=%08x\r\n", sum, min, max, errors,
               crc32(buffer, n));
    if (errors)
        goto fail;
    xil_printf("%s\r\n", ramp ? "SELFTEST PASS" : "MEASUREMENT OK (analog phase not calibrated)");
    return 0;
    /* 统一失败出口：保留状态供排查，并锁住后续测量，直到用户显式复位。 */
fail:
    failed = 1;
    xil_printf("MEASUREMENT FAILED: preserve log, send r to reset entire path\r\n");
    status();
    return 1;
}
/* ===== 6. 串口命令解释 =====
 * m/c=真实测量，t=递增码自检，r=复位，q=停止/复位，s=状态，其它显示帮助。
 * 无参数默认100kHz/4096点/delay3/amp24；网页会显式发送较小幅度8。
 */
static void handle(char *line) {
    unsigned f = 100000, n = 4096, d = 3, a = 24;
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
/* ===== 7. 程序入口 =====
 * 初始化UART0/GPIO/DMA，复位整条路径，然后等待完整命令行，不自动开始采集。
 * SDK Debug停main后需Resume；只下载bit不会执行这里的ARM程序。
 */
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
    xil_printf("\r\nZ100 CORDIC LOOP v1 / UART0 115200 8N1 / NO AUTO START\r\n");
    handle("h");
    status();
    /* 逐字符组行，CR或LF结束一条命令；空行忽略，避免CRLF执行两次。 */
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
