/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 中; basis recorded in AUTHORS.json.
 */
#include "adt6101P_init.h"

#include "adt6101P.h"
#include "adt6101P_inthandler.h"
#include "adt6101p_driver_layer.h"
#include "project_config.h"
#include "gpio_bsp.h"
#include "adt6101P_spi.h"
#include "adt6101P_uart.h"

MMwMsgMCBStrTypedef g_MMWMsgMCB;

void adt6101PSystemInit(void)
{
    adtDrvSystemSetInterrupt(INT_DUALTIMER, ENABLEINT);
    adtDrvSystemSetInterrupt(INT_PMU, ENABLEINT);
}

void adt6101PValInit(void)
{
    g_MMWMsgMCB.sysHighClk = SYS_HIGH_CLK_100M;

    g_MMWMsgMCB.rfStr.rx0En = RX0_EN;
    g_MMWMsgMCB.rfStr.rx1En = RX1_EN;
    g_MMWMsgMCB.rfStr.tx0En = DISABLE;
    g_MMWMsgMCB.rfStr.tx1En = DISABLE;
    g_MMWMsgMCB.rfStr.txMimoEn = DISABLE;
    g_MMWMsgMCB.rfStr.PaGain0 = 0;
    g_MMWMsgMCB.rfStr.PaGain1 = 0;
    g_MMWMsgMCB.rfStr.tiaHpfEn = TIA_HPF_EN;
    g_MMWMsgMCB.rfStr.rcHpfEn = RC_HPF_EN;
    g_MMWMsgMCB.rfStr.pgaHpfEn = PGA_HPF_EN;
    g_MMWMsgMCB.rfStr.tiaGain0 = TIA0_GAIN;
    g_MMWMsgMCB.rfStr.pgaGain0 = PGA0_GAIN;
    g_MMWMsgMCB.rfStr.tiaGain1 = TIA1_GAIN;
    g_MMWMsgMCB.rfStr.pgaGain1 = PGA1_GAIN;
    g_MMWMsgMCB.rfStr.tiaHpf = TIA_HPF;
    g_MMWMsgMCB.rfStr.rcHpf = RC_HPF;
    g_MMWMsgMCB.rfStr.pgaHpf = PGA_HPF;
    g_MMWMsgMCB.rfStr.pgaLpf = PGA_LPF;

    g_MMWMsgMCB.adcStr.rx0En = RX0_EN;
    g_MMWMsgMCB.adcStr.rx1En = RX1_EN;
    g_MMWMsgMCB.adcStr.dsModStr.intervalNum = ADC_SAMPLE_INTERVAL_NUM;
    g_MMWMsgMCB.adcStr.dsModStr.endNum = ADC_SAMPLE_END_NUM;
    g_MMWMsgMCB.adcStr.dsModStr.adcSampleNum = PROJECT_ADC_SAMPLE_LENGTH;
    g_MMWMsgMCB.adcStr.dsModStr.scanNum = (uint16)PROJECT_ADC_CAPTURE_CHIRP_COUNT;
    g_MMWMsgMCB.adcStr.firBypass = FIR_EN;
    g_MMWMsgMCB.adcStr.downSample = DOWN_SAMPLE;
    g_MMWMsgMCB.adcStr.adcClk = ADC_SAMPLE_RATE;
    g_MMWMsgMCB.adcStr.adcDmaSel = FIFO0_CH0QN_CH0IN_FIFO1_CH1QN_CH1IN;
    g_MMWMsgMCB.adcStr.wnMode = ADC_WN_MODE;
    g_MMWMsgMCB.adcStr.wnLen = (uint16)PROJECT_ADC_SAMPLE_LENGTH;
    g_MMWMsgMCB.adcStr.adcWnEn = ADC_WN_EN;
    g_MMWMsgMCB.adcStr.fftWnEn = FFT_WN_EN;

    g_MMWMsgMCB.adcDspStr.stableTime = CHIRP_T1 + CHIRP_T2;
    g_MMWMsgMCB.adcDspStr.adcSel = ADC_SAMPLE_SEL;
    g_MMWMsgMCB.adcDspStr.samplePoint = (uint16)PROJECT_ADC_SAMPLE_LENGTH;
    g_MMWMsgMCB.adcDspStr.scanNum = (uint16)PROJECT_ADC_CAPTURE_CHIRP_COUNT;
    g_MMWMsgMCB.adcDspStr.dmaSrcStep = 0u;
    g_MMWMsgMCB.adcDspStr.dmaDstStep = 4u;
    g_MMWMsgMCB.adcDspStr.adc0SrcAddr = ADC0_FIFO;
    g_MMWMsgMCB.adcDspStr.adc1SrcAddr = ADC1_FIFO;
    g_MMWMsgMCB.adcDspStr.adc0DstAddr = PROJECT_ADC_BUFFER_A_ADDR;
    g_MMWMsgMCB.adcDspStr.adc1DstAddr = 0UL;
}

void adt6101PModuleInit(void)
{

    adtDrvGpioConfig_t gpioCfg = {0};

    adtDrvTimerConfig_t timerCfg = {0};

    adt6101PPowerSw(POWER_ON);
    adt6101PDigiInit();
    adt6101PRfInit(&g_MMWMsgMCB.rfStr, g_MMWMsgMCB.adcDspStr.scanNum);
    adt6101PAnalog(&g_MMWMsgMCB.rfStr);
    adt6101PAdcSetting(&g_MMWMsgMCB.adcStr);
    adt6101PAdcTest();

    if(g_MMWMsgMCB.sysHighClk == SYS_HIGH_CLK_100M)
    {
        adtDrvSystemSwitchHighClock100M();
    }

    /* UART0 stays disabled: P0 is SPI CS and P1 is external SYNC input. */
    setIntHandlerEn(INT_UART0, DISABLEINT);
    setUartCtrl(UART0, 0u);
    disableSpiInterrupt(SPI0);
    setSpiTransmitDmaEnable(SPI0, 0u);
    setSpiReceiveDmaEnable(SPI0, 0u);
    setSpiSsp(SPI0, 0u);
    setIntHandlerEn(INT_SPI0, DISABLEINT);

    gpioWritePin(GPIO_PIN_0, GPIO_PIN_SET);
    gpioWritePin(GPIO_PIN_2, GPIO_PIN_RESET);
    gpioWritePin(GPIO_PIN_4, GPIO_PIN_SET);
    gpioCfg.mode = GPIO_MODE_OUTPUT;
    gpioCfg.pull = GPIO_PULL_DOWN;
    gpioCfg.pinMask = GPIO_PIN_0 | GPIO_PIN_2;
    adtDrvGpioInit(&gpioCfg);
    gpioCfg.pull = GPIO_PULL_UP;
    gpioCfg.pinMask = GPIO_PIN_4;
    adtDrvGpioInit(&gpioCfg);
    gpioCfg.mode = GPIO_MODE_INPUT;
    gpioCfg.pull = GPIO_PULL_DOWN;
    gpioCfg.pinMask = GPIO_PIN_1;
    adtDrvGpioInit(&gpioCfg);
    /* Retain vendor Timer0 setup used by the radar frame/chirp environment. */
    timerCfg.instance = Timer0;
    timerCfg.mode = DEFUALT_MODE;
    timerCfg.reload = adtDrvTimerCalcReload(FRAME_CONCTRL);
    timerCfg.irqHandler = timer0InterruptHandler;
    adtDrvTimerInit(&timerCfg);
}

void ModuleResetClock(void)
{
    /* f1_0 does not enter a system-sleep path; runtime radio low-power does not remux IO. */
}
