/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#include "signal_porcess.h"

#include "adt6101P.h"
#include "adt6101P_adc.h"
#include "adt6101P_dma.h"
#include "adt6101P_system.h"
#include "dma_bsp.h"

/* f1_0 keeps only the ADC DMA front-end path; all FFT/ABS/phase processing is removed. */
typedef struct
{
    volatile uint8 active;
    adcSampleSelTypedef adcSel;
    uint16 dma1TransLen;
    uint16 dma2TransLen;
} AppAdcAsyncTransferTypedef;

static AppAdcAsyncTransferTypedef s_adcAsyncTransfer;

#define APP_ADC_DMA_WAIT_MIN_LOOPS          (100000UL)
#define APP_ADC_DMA_WAIT_LOOPS_PER_SAMPLE   (1024UL)

static uint8 app_wait_dma_transfer(dmaSelTypedef dmaId, uint16 transLen)
{
    uint32 timeoutLoops = APP_ADC_DMA_WAIT_MIN_LOOPS +
                          ((uint32)transLen * APP_ADC_DMA_WAIT_LOOPS_PER_SAMPLE);
    return dmaWaitForTransfer(dmaId, transLen, timeoutLoops);
}

static uint8 app_wait_dual_dma_transfer(uint16 dma1TransLen, uint16 dma2TransLen)
{
    uint16 transLen = (dma1TransLen > dma2TransLen) ? dma1TransLen : dma2TransLen;
    uint32 timeoutLoops = APP_ADC_DMA_WAIT_MIN_LOOPS +
                          ((uint32)transLen * APP_ADC_DMA_WAIT_LOOPS_PER_SAMPLE);

    while(timeoutLoops > 0u)
    {
        if((getDmaTransferCount(DMA1) >= dma1TransLen) &&
           (getDmaTransferCount(DMA2) >= dma2TransLen))
        {
            return 1u;
        }
        timeoutLoops--;
    }
    return 0u;
}

static void app_adc_async_disable_channels(void)
{
    if((s_adcAsyncTransfer.adcSel == SAMPLE_ADC0) ||
       (s_adcAsyncTransfer.adcSel == SAMPLE_ADC01))
    {
        dmaIntConfig(DMA1, DMA_4_4_DONE, DISABLE);
        setDmaDisable(DMA1);
    }
    if((s_adcAsyncTransfer.adcSel == SAMPLE_ADC1) ||
       (s_adcAsyncTransfer.adcSel == SAMPLE_ADC01))
    {
        dmaIntConfig(DMA2, DMA_4_4_DONE, DISABLE);
        setDmaDisable(DMA2);
    }
}

uint8 adcSampleStartAsync(const adcDspStrTypedef *adcDspStr)
{
    adcSampleSelTypedef adcSel;
    uint32 dmaTransferCount;
    dmaConfigStrTypedef dma1ConfigStr;
    dmaConfigStrTypedef dma2ConfigStr;

    if((adcDspStr == (const adcDspStrTypedef *)0) || (s_adcAsyncTransfer.active != 0u))
    {
        return 0u;
    }

    adcSel = adcDspStr->adcSel;
    dmaTransferCount = (uint32)adcDspStr->samplePoint * (uint32)adcDspStr->scanNum;
    if((dmaTransferCount == 0u) || (dmaTransferCount > 0xffffu))
    {
        return 0u;
    }

    dma1ConfigStr.dmaId = DMA1;
    dma1ConfigStr.srcAddr = adcDspStr->adc0SrcAddr;
    dma1ConfigStr.dstAddr = adcDspStr->adc0DstAddr;
    dma1ConfigStr.srcStep = adcDspStr->dmaSrcStep;
    dma1ConfigStr.dstStep = adcDspStr->dmaDstStep;
    dma1ConfigStr.transLen = (uint16)dmaTransferCount;
    dma1ConfigStr.periSelect = PERI_SELECT_ADC0;
    dma1ConfigStr.memTomemOrperi = PERITOMEM;

    dma2ConfigStr.dmaId = DMA2;
    dma2ConfigStr.srcAddr = adcDspStr->adc1SrcAddr;
    dma2ConfigStr.dstAddr = adcDspStr->adc1DstAddr;
    dma2ConfigStr.srcStep = adcDspStr->dmaSrcStep;
    dma2ConfigStr.dstStep = adcDspStr->dmaDstStep;
    dma2ConfigStr.transLen = (uint16)dmaTransferCount;
    dma2ConfigStr.periSelect = PERI_SELECT_ADC1;
    dma2ConfigStr.memTomemOrperi = PERITOMEM;

    s_adcAsyncTransfer.adcSel = adcSel;
    s_adcAsyncTransfer.dma1TransLen = dma1ConfigStr.transLen;
    s_adcAsyncTransfer.dma2TransLen = dma2ConfigStr.transLen;

    if(adcSel == SAMPLE_ADC0)
    {
        dmaConfig(&dma1ConfigStr);
        dmaIntConfig(DMA1, DMA_4_4_DONE, DISABLE);
    }
    else if(adcSel == SAMPLE_ADC1)
    {
        dmaConfig(&dma2ConfigStr);
        dmaIntConfig(DMA2, DMA_4_4_DONE, DISABLE);
    }
    else if(adcSel == SAMPLE_ADC01)
    {
        dmaConfig(&dma1ConfigStr);
        dmaConfig(&dma2ConfigStr);
        dmaIntConfig(DMA1, DMA_4_4_DONE, DISABLE);
        dmaIntConfig(DMA2, DMA_4_4_DONE, DISABLE);
    }
    else
    {
        return 0u;
    }

    /* ADC completion is intentionally polled from the main loop; no DMAA ISR. */
    setIntHandlerEn(INT_DMAA, DISABLEINT);
    s_adcAsyncTransfer.active = 1u;
    adcFmcwStart();
    return 1u;
}

uint8 adcSampleWait(void)
{
    uint8 ok = 0u;

    if(s_adcAsyncTransfer.active == 0u)
    {
        return 0u;
    }
    if(s_adcAsyncTransfer.adcSel == SAMPLE_ADC0)
    {
        ok = app_wait_dma_transfer(DMA1, s_adcAsyncTransfer.dma1TransLen);
    }
    else if(s_adcAsyncTransfer.adcSel == SAMPLE_ADC1)
    {
        ok = app_wait_dma_transfer(DMA2, s_adcAsyncTransfer.dma2TransLen);
    }
    else if(s_adcAsyncTransfer.adcSel == SAMPLE_ADC01)
    {
        ok = app_wait_dual_dma_transfer(s_adcAsyncTransfer.dma1TransLen,
                                        s_adcAsyncTransfer.dma2TransLen);
    }

    app_adc_async_disable_channels();
#ifdef CASCADE_MASTER
    RFC->fmcw_fmcw_sync &= ~fmcw_fmcw_sync_sync_out_bit;
#endif
    s_adcAsyncTransfer.active = 0u;
    return ok;
}
