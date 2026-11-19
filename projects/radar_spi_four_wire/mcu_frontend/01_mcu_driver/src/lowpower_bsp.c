/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    lowpower_bsp.c
 * @original_author  DingZhen(zding@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-29
 * @brief   This file references the Lowpower HAL library and combines it into a
 *          universal interface function.
 * @brief
 *            Process sample:
 *                - Scan -> lowPowerRadioGotoActive(RF off, Clock 50/125MHz) -> Signal processing ->
 *                (goto sleep/goto idle -> Wakeup) -> lowPowerActiveGotoRadio(RF on) -> Scan......
 *
 *            The process includes the following steps:
 *                - Scan( @ref adcSampleStart):
 *                    - Sweep the frequency and collect data.
 *                - LowPowerRadioGotoActive( @ref lowPowerRadioGotoActive):
 *                    - RF Module Off: Turn off the RF after the frequency sweep is completed
 *                    (part of the RF can be turned off according to the choice, divided into Fast and Slow parts
 *                    Slow mode is based on Faste mode,more RF is closed,spend more than 550 us).
 *                    - Adjusts System Clock: Operates at reduced clock speeds (50MHz or 125MHz), further lowers
 *                    the clock speed to minimize power use during the transition to data processing.
 *                - Signal Processing:
 *                    - Performed with the RF module off, focusing on energy saving.
 *                - Sleep/Idle Decision:
 *                    - Sleep Mode( @ref lowPowerActiveGotoSleep): Only the RTC clock is retained, most of the clock is turned off.
 *                    Establishes a wake-up trigger, keeping the device in low power mode until necessary.
 *                    - Idle Mode( @ref lowPowerActiveGotoIdle): Slightly lowers energy consumption, with fewer clocks turned off compared to sleep mode.
 *                - Wakeup( @ref wakeUpInit):
 *                    - Reactivates from low-power mode under certain conditions, with two wake-up modes available: RTC and GPIO interrupt.
 *                - LowPowerActiveGotoRadio( @ref lowPowerActiveGotoRadio):
 *                    - Reactivates the RF before sweeping.
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "lowpower_bsp.h"
#include <stddef.h>
#include <string.h>

/* Exported variables ---------------------------------------------------------*/
static wakeUpInitTypedef s_sleepWakeUpInit;

/* Exported functions --------------------------------------------------------*/
static void lowPowerSwitchPlln(int32 pllnOn);
static void lowPowerSwitchFastRF(const AnalogDataStrTypedef *pDataStr);
static void lowPowerSwitchSlow(const AnalogDataStrTypedef *pDataStr, const sysHighClkTypedef targetHighClock);

/**
 * @brief   Turn on RF and switch MCU from Active mode to Radio mode.
 *          Please notice that mcuGotoRadar function is only used to switch MCU from Active mode to Radio mode.
 *          And this function does not support to switch MCU from others mode, such as Idle or Sleep mode, to Radio mode.
 * @param   pDataStr: analog RF and IF structure, see also @ref AnalogDataStrTypedef.
 */
void lowPowerActiveGotoRadio(uint8 lowpowerEn, lowpowerFrameModeTypedef lowpowerFrameMode,
                             const AnalogDataStrTypedef *pDataStr, const sysHighClkTypedef targetHighClock)
{
    sysHighClkTypedef highClock = getSysHighClk();

    if(lowpowerEn == ENABLE)
    {
        //To modify the ADC phase deviation caused by some factors
        //Specific problems can be referenced：JIRA:USKH-33
        setSoftwareReset(DISABLE,CTRL_RST_IFADC);
        setSoftwareReset(DISABLE,CTRL_RST_FIR);
        setSoftwareReset(ENABLE,CTRL_RST_IFADC);
        setSoftwareReset(ENABLE,CTRL_RST_FIR);
        
        if(lowpowerFrameMode == LOWPOWER_SLOW)
        {
            /* power on RF. */
            lowPowerSwitchSlow(pDataStr, highClock);
            if(highClock != targetHighClock)
            {
                if(targetHighClock == SYS_HIGH_CLK_100M)
                {
                    switch100MHclk();
                }
                else if(targetHighClock == SYS_HIGH_CLK_50M)
                {
                    switch50MHclk();
                }
            }

        }
        if((lowpowerFrameMode == LOWPOWER_FAST) || \
                (lowpowerFrameMode == LOWPOWER_SLOW))
        {
            lowPowerSwitchPlln(ENABLE);
            lowPowerSwitchFastRF(pDataStr);
        }
    }

}


/**
 * @brief   Turn off RF and switch MCU from Radio mode to Active mode.
 *          Please notice that mcuGotoActive function is only used to switch MCU from Radio mode to Active mode.
 *          And this function does not support to switch MCU from others mode, such as Idle or Sleep mode, to Active mode.
 * @param   pDataStr: analog RF and IF structure, see also @ref AnalogDataStrTypedef.
 * @param   targetHighClock: Select system clock frequency, see also @ref sysHighClkTypedef.
 */
void lowPowerRadioGotoActive(uint8 lowpowerEn, lowpowerFrameModeTypedef lowpowerFrameMode,
                             const AnalogDataStrTypedef *pDataStr, const sysHighClkTypedef targetHighClock)
{
    sysHighClkTypedef currentHighClock = getSysHighClk();

    AnalogDataStrTypedef tmpData;
    memcpy(&tmpData, pDataStr, sizeof(AnalogDataStrTypedef));
    tmpData.rx0En = DISABLE;
    tmpData.rx1En = DISABLE;
    tmpData.tx0En = DISABLE;
    tmpData.tx1En = DISABLE;

    if(lowpowerEn == ENABLE)
    {
        if((lowpowerFrameMode == LOWPOWER_FAST) || \
                (lowpowerFrameMode == LOWPOWER_SLOW))
        {
            lowPowerSwitchPlln(DISABLE);
            lowPowerSwitchFastRF(&tmpData);
            delayUs(100);
        }

        if(lowpowerFrameMode == LOWPOWER_SLOW)
        {
            /* clock switch. */
            if((currentHighClock == SYS_HIGH_CLK_100M) & (targetHighClock == SYS_HIGH_CLK_50M))
            {
                switch50MHclk();
            }

            // if((currentHighClock == HIGH_CLOCK_100M) & (targetHighClock == HIGH_CLOCK_100M)), just lowPowerSwitchSlow to target.
            // if((currentHighClock == HIGH_CLOCK_50M)  & (targetHighClock == HIGH_CLOCK_50M)),  just lowPowerSwitchSlow to target.
            // if((currentHighClock == HIGH_CLOCK_50M)  & (targetHighClock == HIGH_CLOCK_100M)), currently, lowPowerSwitchSlow function does not support this case.
            lowPowerSwitchSlow(&tmpData, targetHighClock);
        }
    }
}

/**
 * @brief   adt6101P mcu go to idle mode.
 * @param   None.
 * @return  None.
 */
void lowPowerActiveGotoIdle(void)
{
    uint32 regTemp2;
    uint8 tmp50MBias;
    uint8 tmp50MCap;

    tmp50MBias  = getXo50MBias();
    tmp50MCap   = getXo50MCap();
    regTemp2    = getPlliDaca();

//    setIntHandlerEn(INT_DUALTIMER, ENABLEINT);   /* for delayUs function. */
    setIntHandlerEn(INT_RTC, ENABLEINT);           /* enable RTC interrupt, to wake up MCU from IDLE mode. */
    /* decrease hclk. */
    setAHBClock(CLK_HIGH_DIV_64);

    /* close plli and ldo12_hp. */
    setPlliDaca(0x0);
    setManualCtrlToLdoAndClk(ENABLE);
    setLdo12PowerDown(POWER_DOWN);

    setXo50MBiasAndCap(0x88, 0x8);

    /* delay about 100us.  Currently, hclk=1/64 clk_high, so 3 means 3*64 us. */
    delayUs(3);

    /* idle */
    soc_SCR[0x0] = 0x00;
    __asm("WFI");

    /* recovery plli and ldo12_hp. */
    setLdo12PowerDown(POWER_ON);
    setPlliDaca(regTemp2);

    setXo50MBiasAndCap(tmp50MBias, tmp50MCap);
    /* delay about 100us.  Currently, hclk=1/64 clk_high, so 3 means 3*64 us. */
    delayUs(3);

    /* recovery hclk */
    setAHBClock(CLK_HIGH);

    return;
}


/**
 * @brief   adt6101P mcu go to sleep.
 * @param   None.
 * @return  None.
 */
void lowPowerActiveGotoSleep(void)
{
    uint32 regVal = 0;
    sysHighClkTypedef highClock = getSysHighClk();
    
    if((s_sleepWakeUpInit.wakeUpMode == WAKEUP_MODE_RTC) || \
            ((s_sleepWakeUpInit.wakeUpMode == WAKEUP_MODE_RTC_AND_GPIO)))\
    {
        rtcTimerStart((uint32)(s_sleepWakeUpInit.sleepTime));
    }
    if((s_sleepWakeUpInit.wakeUpMode == WAKEUP_MODE_GPIO)
            || ((s_sleepWakeUpInit.wakeUpMode == WAKEUP_MODE_RTC_AND_GPIO)))
    {
        pmuGpioWakeup(s_sleepWakeUpInit.wakeupPin, s_sleepWakeUpInit.wakeUpLevelSet);
    }

    /* before going to sleep, MUST switch clock from 100mhz to 50mhz since we do not support to sleep with 100mhz. */
    if(highClock == SYS_HIGH_CLK_100M)
    {
        switch50MHclk();

        /* Firstly, disable PLLI */
        pllLDOEnable(0, 0);

        /* If target high clock is 50mhz, here is some optimization for active mode. */
        /* optimization for active mode. */
        setXo50MBiasAndCap(0x88, 0x8);
        setPlliDaca(0x0);
        setPlliDacb(0x0);

        /* when 50Mhz, close ldo12_hp.  ldo12_lp could support less than 12mA. */
        setLdo12PowerDown(POWER_DOWN);
        delayUs(100);
        
        gpioWritePin(GPIO_PIN_22,GPIO_PIN_RESET);       
    }
    
    /* manu_ctrl_en = 0*/
    setManualCtrlToLdoAndClk(DISABLE);
    delayUs(70);// pending effectiveness
    
    /* MUST check RTC's counter, making sure it will not wake up in 3 cycles. */
    /* RTC timer runs at 32KHz and is asynchronous with a main frequency clock of
        50MHz. It needs to be read twice and compared. If the value read twice are
        consistent, it is correct. If the values read twice are inconsistent, it
        should be read again. */
    if((s_sleepWakeUpInit.wakeUpMode == WAKEUP_MODE_RTC) || \
            ((s_sleepWakeUpInit.wakeUpMode == WAKEUP_MODE_RTC_AND_GPIO)))\
    {
        regVal = getRtcData();
        delayUs(1);
        if(regVal != getRtcData())
        {
            delayUs(1);
            regVal = getRtcData();
        }

        if(regVal < getRtcMatch() + 3)
        {
            /* MUST make sure PMU's state is idle before going to sleep. */
            regVal = PMU->pmu_pmu_state;
            regVal = regVal & (pmu_pmu_state_cur_state_mask << pmu_pmu_state_cur_state_shift);
            while(regVal != 0x00)
            {
                regVal = PMU->pmu_pmu_state;
                regVal = regVal & (pmu_pmu_state_cur_state_mask << pmu_pmu_state_cur_state_shift);
            }
        }
    }

    if((s_sleepWakeUpInit.wakeUpMode == WAKEUP_MODE_GPIO) ||
            (s_sleepWakeUpInit.wakeUpMode == WAKEUP_MODE_RTC_AND_GPIO))
    {
        /* MUST make sure PMU's state is idle before going to sleep. */
        regVal = PMU->pmu_pmu_state;
        regVal = regVal & (pmu_pmu_state_cur_state_mask << pmu_pmu_state_cur_state_shift);
        while(regVal != 0x00)
        {
            regVal = PMU->pmu_pmu_state;
            regVal = regVal & (pmu_pmu_state_cur_state_mask << pmu_pmu_state_cur_state_shift);
        }
    }

    soc_SCR[0x0] = 0x04;
    __asm("WFI");
    
    /* manu_ctrl_en = 1*/
    setManualCtrlToLdoAndClk(ENABLE);
}


/**
 * @brief   adt6101P setting for entering sleep mode.
 * @param   ramOn: Select RAM on or off status, see also @ref ramStateTypedef.
 * @return  None.
 */
void mcuSleepConfig(ramStateTypedef ramOn)
{
    /* ramOn = 0, sleep with PD_RAM off.
    ramOn = 1, sleep with PD_RAM on, keep RAM powered on. */

    /* set por_test_sel = 1 avoid leakage, from Yuan. */
    setPorTest(POR_INPUT); 

    /* disable debugger's wake up function. */
    clearAllWakeUpEn();
    setIoWakeUpEn(ENABLE);
    setRtcTimerWakeUpEn(ENABLE);

    #if ANALOG_IF_OUTPUT
    #else
    /* must output 1, ant then set as output. so there is no glitch of P29's high level output. */
    gpioInitStrTypedef  gpio29InitStruct;
    gpio29InitStruct.Mode  = GPIO_MODE_OUTPUT;
    gpio29InitStruct.Pull  = GPIO_PULL_UP;
    gpio29InitStruct.Pin   = GPIO_PIN_29;
    gpioInit(&gpio29InitStruct);
    gpioWritePin(GPIO_PIN_29, GPIO_PIN_SET);
    #endif

    /* must output 1, ant then set as output. so there is no glitch of P29's high level output. */
    gpioInitStrTypedef  gpio22InitStruct;
    gpio22InitStruct.Mode  = GPIO_MODE_OUTPUT;
    gpio22InitStruct.Pull  = GPIO_PULL_UP;
    gpio22InitStruct.Pin   = GPIO_PIN_22;
    gpioInit(&gpio22InitStruct);
    gpioWritePin(GPIO_PIN_22, GPIO_PIN_SET);


    /* the time of IO_Wakeup to MCU_running = (powering_up_time+4)*T_32k.  0x99 means (153+4)*32us = 5ms appro. */
    setPoweringDownTime(0x0001);
    setPoweringUpTime(0x0099);

    setSlp(0x0, 0x0, 0x0);
    
    setLdoLpVol(LDO12_VOL_0_DEC_8);

    if(ramOn)
    {
        setSlpPowerCtrl(0x1F);
    }
    else
    {
        setSlpPowerCtrl(0x07);
    }
}

/**
 * @brief   setting gpio for mcu weekup.
 * @param   pinNumber: gpio number id, 0~30.
 * @param   pinPolarity: wake up edge of gpio.
 *                      0: high level to wake up
 *                      1: low level to wake up.
 * @return  None.
 */
void pmuGpioWakeup(uint32 pinNumber, uint32 pinPolarity)
{
    // set as input
    gpioInitStrTypedef  gpio4InitStruct;
    gpio4InitStruct.Mode  = GPIO_MODE_INPUT;
    gpio4InitStruct.Pull  = GPIO_NO_PULL;
    gpio4InitStruct.Pin   = pinNumber;
    gpioInit(&gpio4InitStruct);
    
    // wake up enable.
    setGpioWakeEnable(pinNumber, ENABLE);
    // set polarity.
    setGpioWakePol(pinNumber, (gpioWakeUpTypedef)pinPolarity);
}

/**
 * @brief   setting for mcu wakeup.
 * @param   wakeUpInitTypedef: wakeup init.
 * @return  None.
 */
void wakeUpInit(wakeUpInitTypedef *wakeuInit)
{
    s_sleepWakeUpInit.wakeUpMode = wakeuInit->wakeUpMode;
    s_sleepWakeUpInit.sleepTime = wakeuInit->sleepTime;
    s_sleepWakeUpInit.wakeupPin = wakeuInit->wakeupPin;
    s_sleepWakeUpInit.wakeUpLevelSet = wakeuInit->wakeUpLevelSet;
    
    mcuSleepConfig(wakeuInit->pdRamSet);
}

/**
 * @brief   adt6101P low power mode, open or close plli or plln.
 *           ________                                           ________
 *   |      |        |                                         |        |
 *   |      |        |                                         |        |
 *   |     	|        |                                         |        |
 *   |     	|        |                                         |        |
 *   |     	|        |_________                                |        |_________
 *   |      |        |         |                               |        |         |
 *   |      |        |         |                               |        |         |
_____|______|________|_________|_______________________________|________|_________|________
 *   |         radio    active                  sleep
 *   |
 *   |
 * @param   pllnOn :  open or close plln vco.
 * @return  None.
 */
static void lowPowerSwitchPlln(int32 pllnOn)
{
    //Enable VCOEn bit first and wait 10us
    pllVcoEnable(pllnOn);
    if(pllnOn==1)
    {
        delayUs(10); //wait for plli&plln settle
    }

    //Enable bufpll and bufVCO bit second and wait 1us
    pllBufEnable(pllnOn);
    if(pllnOn==1)
    {
        delayUs(1); //wait for plli&plln settle
    }

    //Enable MMDEn bit last and wait 1us
    pllMMDEnable(pllnOn);
    if(pllnOn==1)
    {
        delayUs(1); //wait for plli&plln settle
    }

    #if 1
    if(pllnOn==1)
    {
        delayUs(50); //wait for plli&plln settle
    }
    #endif
}


/**
 * @brief   adt6101P in the low power consumption mode, the module that can not be
                    switched in a short time needs a long stable time.
 * @param   pDataStr: analog RF and IF structure, see also @ref AnalogDataStrTypedef.
 * @return  None.
 */
static void lowPowerSwitchFastRF(const AnalogDataStrTypedef *pDataStr)
{
    stateTypedef rx0En = pDataStr->rx0En;
    stateTypedef rx1En = pDataStr->rx1En;
    stateTypedef tx0En = pDataStr->tx0En;
    stateTypedef tx1En = pDataStr->tx1En;
    pgaHpfTypedef pgaHpf = pDataStr->pgaHpf;

    int32 dacPower = 0;
    //int32 biasRx0 = rx0En * 8;
    //int32 biasRx1 = rx1En * 8;

    int32 biasRx00 = rx0En * 8;
    int32 biasRx01 = rx0En * 7;
    int32 biasRx10 = rx1En * 8;
    int32 biasRx11 = rx1En * 7;

    uint8 anyRxTx = (rx0En | rx1En | tx0En | tx1En);
    int32 biasAnyRxTx = anyRxTx * 8;

    uint8 buffer20gEn = (pDataStr->rx0En  | \
                         pDataStr->rx1En | \
                         pDataStr->tx0En | \
                         pDataStr->tx1En);
    uint8 doublerEn = (pDataStr->rx0En    | \
                       pDataStr->rx1En | \
                       pDataStr->tx0En | \
                       pDataStr->tx1En);

    if(anyRxTx == 1)
    {
        dacPower = POWER_ON;
    }
    else
    {
        dacPower = POWER_DOWN;
    }
    dacEnBias1Pd((stateTypedef)anyRxTx, dacPower); /* all bias except for PA&LNA */
    /* bias2_en: enable signal for ztc bias for BB */
    baseBandBiasSetting((stateTypedef)(rx0En | rx1En));
    /*turn on tia pga */
    /*rfTiaEnable(rx0En, rx1En);
    rfTiaDacBias(biasRx0, biasRx1);
    rfVgaEnable(rx0En, rx1En);
    rfVgaDacBias(biasRx0, biasRx1);*/
    pgaHpfConfig(rx0En, rx1En, pgaHpf, pgaHpf);
    /* turn on adc, gpadc */
    adcEnable((stateTypedef)(rx0En | rx1En), rx1En);
    adcBiasSetting(biasRx01, biasRx01, biasRx01, biasRx00, biasRx00, biasRx00, biasRx11, \
                    biasRx11,biasRx11, biasRx10, biasRx10, biasRx10);
    /* RF RX */
    rxLnaEnable(rx0En, rx1En);
    mixerEnable(rx0En, rx1En);

    /* enable 20G and doubler */
    rfbuff20G((stateTypedef)buffer20gEn, (stateTypedef)doublerEn);
    /* enable buffer1to4 */
    //rfBuff60GEnable((stateTypedef)anyRxTx, tx0En, tx1En, rx0En, rx1En);
    /* enable bias for 20G(fmcw buffer), doubler and buffer1to4 */
    bias20GTo60G(0, 0, 0, biasAnyRxTx, biasAnyRxTx, 0, 0, 0);
    //------------------------ TX ------------------ //
    txPaEnable(tx0En, tx1En);
}


/**
 * @brief   adt6101P in the low power consumption mode, the module that can be
                    switched in a short time needs a short stable time.
 * @param   pDataStr: analog RF and IF structure, see also @ref AnalogDataStrTypedef.
 * @param   targetHighClock: Select system clock frequency, see also @ref sysHighClkTypedef.
 * @return  None.
 */
static void lowPowerSwitchSlow(const AnalogDataStrTypedef *pDataStr, const sysHighClkTypedef targetHighClock)
{
    stateTypedef rx0En = pDataStr->rx0En;
    stateTypedef rx1En = pDataStr->rx1En;
    stateTypedef tx0En = pDataStr->tx0En;
    stateTypedef tx1En = pDataStr->tx1En;

    int32 anyRxTx = rx0En | rx1En | tx0En | tx1En;
    gpioInitStrTypedef  gpio22InitStruct;

    sysHighClkTypedef currentHighClock = getSysHighClk();

    /* If turn off all RF. */
    if(anyRxTx == 0)
    {
        /* Step 1: check clock: currentHighClock == targetHighClock. */
        while(currentHighClock !=
                targetHighClock); /* make sure you have switched clock to targetHighClock before turning off RF. */

        /* Step 2: Turn off PLLN and BB's LDOs */
        /*  switch PLLN's LDO from 2.5v to 1.8v */
        pllVcoConfig(0, PLLN_VCO_LDO_1P8);
        pllPllConfig(1, PLLN_PLL_LDO_1P8, 1);  // from 2P5 to 1P8, decrease 3.2mA. 
        /* turn off BB's LDO. */
        baseband18LDO(0, BB_LDO_1P8, 0, BB_LDO_1P8);
        dacEnBias1Pd(DISABLE, POWER_DOWN); // 0.4mA

        //* Step 3: If target high clock is 50mhz, turn off PLLI and VDD_RF.  Else, keep PLLI and VDD_RF as it's previous status */
        if(targetHighClock == SYS_HIGH_CLK_50M)
        {
            /* Firstly, disable PLLI */
            setPlliDaca(0x0);
            setPlliDacb(0x0);
            pllLDOEnable(0, 0);
            
            /* Then, set P22=0 to turn off VDD_RF. */
            gpio22InitStruct.Mode  = GPIO_MODE_OUTPUT;
            gpio22InitStruct.Pull  = GPIO_PULL_DOWN;
            gpio22InitStruct.Pin   = GPIO_PIN_22;
            gpioInit(&gpio22InitStruct);
            gpioWritePin(GPIO_PIN_22,
                         GPIO_PIN_RESET); /* must output 1, ant then set as output. so there is no glitch of P29's high level output. */

        }
        else 
        {
            /* optimization for active mode when targetHighClock = SYS_HIGH_CLK_100M . */
            setPlliDaca(0x0);   // close all other bias except clock doubler. 
            setPlliDacb(0x040); // close all other bias except clock doubler. 
            
            /* Firstly, disable PLLI's VCO, but keep PLLI's clock doubler working. */
            pllLDOEnable(0, 1);            
        }
        
        /* As a GPIO, P29 is conflict with debugMode, which would enable analog IF on P29.
        Add this to being compatible with some old version of PCB. */
        #if ANALOG_IF_OUTPUT
        #else
        gpioWritePin(GPIO_PIN_29, GPIO_PIN_RESET);
        #endif

        /* Step 4: If target high clock is 50mhz, here is some optimization for active mode. */
        if(targetHighClock == SYS_HIGH_CLK_50M)
        {
            /* optimization for active mode. */
            setXo50MBiasAndCap(0x88, 0x8);
            //PMU->pmu_clk_gate_reg = 0x0; /* close all submodule's clock. */

            /* when 50Mhz, close ldo12_hp.  ldo12_lp could support less than 12mA. */
            setLdo12PowerDown(POWER_DOWN);
            delayUs(100);
        }

    }
    else
    {
        /* Step 1: turn on VDD_RF by setting P22=1. */
        gpio22InitStruct.Mode  = GPIO_MODE_OUTPUT;
        gpio22InitStruct.Pull  = GPIO_PULL_DOWN;
        gpio22InitStruct.Pin   = GPIO_PIN_22;
        gpioInit(&gpio22InitStruct);
        gpioWritePin(GPIO_PIN_22,
                     GPIO_PIN_SET); /* must output 1, ant then set as output. so there is no glitch of P29's high level output. */

        /* As a GPIO, P29 is conflict with debugMode, which would enable analog IF on P29.
        Add this to being compatible with some old version of PCB. */
        #if ANALOG_IF_OUTPUT
        #else
        gpioWritePin(GPIO_PIN_29, GPIO_PIN_SET);
        #endif

        /* Step 2: turn on PLLI, PLLN and BB's LDOs and bias settings. */
        delayUs(100);
        pllLDOEnable(ENABLE, ENABLE);
        delayUs(50);  /* 100us for 10uF cap on board */
        pllVcoConfig(1, PLLN_VCO_LDO_1P8);
        delayUs(50);
        pllPllConfig(1, PLLN_PLL_LDO_2P5, 1);
        delayUs(50);
        dacEnBias1Pd(ENABLE, POWER_ON); // 0.4mA
        delayUs(50);
        baseband18LDO(rx0En, BB_LDO_1P8, 0, BB_LDO_1P8);
        delayUs(50);
        baseband18LDO(rx0En, BB_LDO_1P8, rx1En, BB_LDO_1P8);
        delayUs(50);

        /* Step 3: Recovery settings for radio mode. */
        /* recovery for radio mode. */
        setXo50MBiasAndCap(0x8F, 0x8);
        plliDacSetting();
        setLdo12PowerDown(POWER_ON); /* when 50Mhz, close ldo12_hp. */
        delayUs(100);
    }
}

/**
 * @brief   next Chirp Disable. 
 * @param   None.
 * @return  None.
 */
void nextChirpDisable(void)
{
    AnalogDataStrTypedef tmpData;
    tmpData.rx0En = DISABLE;
    tmpData.rx1En = DISABLE;
    tmpData.tx0En = DISABLE;
    tmpData.tx1En = DISABLE;

    lowPowerSwitchFastRF(&tmpData);
}

/**
 * @brief   next Chirp Enable. 
 * @param   None.
 * @return  None.
 */
void nextChirpEnable(const AnalogDataStrTypedef *pDataStr)
{
    AnalogDataStrTypedef tmpData;

    if(pDataStr == NULL)
    {
        return;
    }
    memcpy(&tmpData, pDataStr, sizeof(AnalogDataStrTypedef));
    lowPowerSwitchFastRF(&tmpData);
}

#if POWER_TEST
/**
 * @brief   Current testing in the ACTIVE mode.
 * @return  None.
 * @note    50MHz: 9.6mA. Must be placed on the first line of the main function execution.
 */
void pmuActiveTest(void)
{
    /* MUST enable all of clock gating cells. Otherwise, the sleep will fail. */
    for(uint8 i = (uint8)CLK_GATING_PMU_APB; i < (uint8)CLK_GATING_ROM; ++i)
    {
        setExtClkEn(DISABLE, (extClkTypedef)i);
    }

    setIntHandlerEn(INT_UART1, ENABLEINT);  //disable uart1.

    for(int i = 0; i < 31; i++)
    {
        *((uint32 *)(GPIO_REGS + i + 12)) = 0x50;
    }
    *((uint32 *)(GPIO_REGS + 4 + 12)) = 0x90; //p04 pullup + high-z.
    //*((uint32*)(GPIO_REGS + 22 + 12)) = 0x50;  //p22 pulldown + high-z.
    //*((uint32*)(GPIO_REGS + 29 + 12)) = 0x50;  //p29 pulldown + high-z.

    setXo50MBiasAndCap(0x88, 0x8);
    setPlliDaca(0x0);
    for(uint8 i = (uint8)CTRL_CLK_I2C1_EN; i < (uint8)CTRL_CLK_DSM_TEST_EN; ++i)
    {
        setClkGateEn(DISABLE, (ctrlClkTypedef)i);
    }
    setManualCtrlToLdoAndClk(ENABLE);
    setLdo12PowerDown(POWER_DOWN);

    if(1)
    {
        while(1);
    }
    else
    {
        gpioInitStrTypedef  gpio21InitStruct;
        gpio21InitStruct.Mode  = GPIO_MODE_OUTPUT;
        gpio21InitStruct.Pull  = GPIO_PULL_UP;
        gpio21InitStruct.Pin   = GPIO_PIN_21;
        gpioInit(&gpio21InitStruct);

        /* test stability for the switch. */
        while(1)
        {
            for(int i = 0; i < 10000; i++);
            /* must output 1, ant then set as output. so there is no glitch of P29's high level output. */
            gpioWritePin(GPIO_PIN_21, GPIO_PIN_RESET);

            setXo50MBiasAndCap(0xF8, 0x8);
            setPlliDaca(0x14aaa8);
            setLdo12PowerDown(POWER_ON);
            for(int i = 0; i < 10000; i++);
            gpioWritePin(GPIO_PIN_21, GPIO_PIN_SET);

            setXo50MBiasAndCap(0x88, 0x8);
            setPlliDaca(0x0);
            setLdo12PowerDown(POWER_DOWN);

        }
    }
}


/**
 * @brief   Current testing in the IDLE mode.
 * @return  None.
 * @note    50MHz: 5.5mA. Must be placed on the first line of the main function execution.
 */
void pmuIdleTest(void)
{
    /* MUST enable all of clock gating cells. Otherwise, the sleep will fail. */
    for(uint8 i = (uint8)CLK_GATING_PMU_APB; i < (uint8)CLK_GATING_ROM; ++i)
    {
        setExtClkEn(DISABLE, (extClkTypedef)i);
    }

    setIntHandlerEn(INT_UART1, ENABLEINT);  /* disable uart1. */

    for(int i = 0; i < 31; i++)
    {
        *((uint32 *)(GPIO_REGS + i + 12)) = 0x50;
    }
    *((uint32 *)(GPIO_REGS + 4 + 12)) = 0x90; /* p04 pullup + high-z. */
    //*((uint32*)(GPIO_REGS + 22 + 12)) = 0x50;  /* p22 pulldown + high-z. */
    //*((uint32*)(GPIO_REGS + 29 + 12)) = 0x50;  /* p29 pulldown + high-z. */

    lowPowerActiveGotoIdle();
}


/**
 * @brief   Current testing in the SLEEP mode.
 * @return  None.
 * @note    50MHz: 100uA. Must be placed on the first line of the main function execution.
 */
void pmuSleepTest(void)
{
    adt6101PSystemInit();

    /* sleep configuation. */
    mcuSleepConfig(PD_RAM_OFF);

    /* set all GPIOs as pulldown + high-z, except P04 and P11 which are pulled up by PCB. */
    for(int i = 0; i < 31; i++)
    {
        *((uint32 *)(GPIO_REGS + i + 12)) = 0x50;
    }
    *((uint32 *)(GPIO_REGS + 4 + 12))    = 0x90; /* p04 pullup + high-z. */
    *((uint32 *)(GPIO_REGS + 11 + 12))   = 0x90; /* p11 pullup + high-z. */

    /* goto sleep without wakeup. */
    lowPowerActiveGotoSleep();
    while(1);
}
#endif
