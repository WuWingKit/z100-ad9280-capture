/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 中; basis recorded in AUTHORS.json.
 */
#include "adt6101p_driver_layer.h"

/**
 * @file adt6101p_driver_layer.c
 * @brief 驱动分类封装实现
 *
 * 说明：
 * - 本文件只做“参数映射和统一命名”，不改变底层 BSP 的行为；
 * - 通过集中封装，便于后续统一加参数检查、日志与异常处理。
 */

/* ========================= 系统控制（System） ========================= */
void adtDrvSystemSetInterrupt(interruptNumTypedef interruptNum, interruptStateTypedef state)
{
    setIntHandlerEn(interruptNum, state);
}

void adtDrvSystemSwitchHighClock100M(void)
{
    switch100MHclk();
}

/* =========================== IO（GPIO） =========================== */
void adtDrvGpioInit(const adtDrvGpioConfig_t *cfg)
{
    gpioInitStrTypedef init = {0};

    if(cfg == (const adtDrvGpioConfig_t *)0)
    {
        return;
    }

    /* 将统一封装配置映射到底层 BSP 配置结构体 */
    init.Pin = cfg->pinMask;
    init.Mode = cfg->mode;
    init.Pull = cfg->pull;
    gpioInit(&init);
}

void adtDrvGpioWrite(uint32 pinMask, gpioPinStateTypedef state)
{
    gpioWritePin(pinMask, state);
}

gpioPinStateTypedef adtDrvGpioRead(uint32 pinMask)
{
    return gpioReadPin(pinMask);
}

/* ========================== 通信（UART/SPI） ========================== */
uartStatusTypedef adtDrvUartInit(const adtDrvUartConfig_t *cfg)
{
    uartInitStrTypedef init = {0};

    if(cfg == (const adtDrvUartConfig_t *)0)
    {
        return UART_ERROR;
    }

    /* 统一配置转发到底层 uartInit */
    init.uartX = cfg->instance;
    init.baudRate = cfg->baudRate;
    init.mode = cfg->mode;
    init.intEn = cfg->intEnableMask;
    init.intHandler = cfg->irqHandler;
    return uartInit(&init);
}

void adtDrvSpiInit(SPI_TypeDef *instance, const adtDrvSpiConfig_t *cfg)
{
    SpiInitStrTypedef init;

    if((instance == (SPI_TypeDef *)0) || (cfg == (const adtDrvSpiConfig_t *)0))
    {
        return;
    }

    /* SPI 通信参数映射 */
    init.SpiGaterMode = SPI_CPOL_Low_CPHA_1Edge;
    init.SpiNSS = SPI_NSS_Soft;
    init.SpiMode = cfg->mode;
    init.SpiDataSize = cfg->dataSize;
    init.SpiPrescaler = cfg->prescaler;
    init.intEn = cfg->intEnableMask;
    init.intHandler = cfg->irqHandler;
    spiInit(instance, &init);
}

/* ============================ 定时（Timer） ============================ */
void adtDrvTimerInit(const adtDrvTimerConfig_t *cfg)
{
    timerInitStrTypedef init = {0};

    if(cfg == (const adtDrvTimerConfig_t *)0)
    {
        return;
    }

    /* Timer 配置映射 */
    init.Timerx = cfg->instance;
    init.mode = cfg->mode;
    init.reload = cfg->reload;
    init.intHandler = cfg->irqHandler;
    timerInit(&init);
}

uint32 adtDrvTimerCalcReload(float periodMs)
{
    return getReload(periodMs);
}

/* =========================== 低功耗（LowPower） =========================== */
void adtDrvWakeupInit(const adtDrvWakeupConfig_t *cfg)
{
    wakeUpInitTypedef init;

    if(cfg == (const adtDrvWakeupConfig_t *)0)
    {
        return;
    }

    /* 唤醒配置映射，支持 RTC / GPIO 两种模式 */
    init.pdRamSet = cfg->ramState;
    init.sleepTime = cfg->sleepTimeMs;
    init.wakeUpMode = cfg->wakeupMode;
    init.wakeupPin = cfg->wakeupPinMask;
    init.wakeUpLevelSet = cfg->wakeupLevel;
    wakeUpInit(&init);
}
