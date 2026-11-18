/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    pmu_bsp.h
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file provides a PMU universal function interface, making it convenient
 *          for users to directly call it
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __PMU_BSP_H
#define __PMU_BSP_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "adt6101P_pmu.h"
#include "adt6101P_rx.h"


/** @addtogroup BSP
 *  @{
 */

/** @addtogroup PMU_BSP
 *  @{
 */


/* Exported types ------------------------------------------------------------*/
/** @defgroup PMU_Exported_Macros
  * @{
  */
/**
  * @brief  Set analog RF and IF parameters
  */
typedef struct
{
    stateTypedef rx0En;     /*!< Select whether to enable or disable the RX0 channel */
    stateTypedef rx1En;     /*!< Select whether to enable or disable the RX1 channel */
    stateTypedef tx0En;     /*!< Select whether to enable or disable the TX0 channel */
    stateTypedef tx1En;     /*!< Select whether to enable or disable the TX1 channel */
    stateTypedef txMimoEn;  /*!< Select whether to enable or disable the TD MIMO */
    uint8 PaGain0;          /*!< Select the PA0 gain of the TX0 channel */
    uint8 PaGain1;          /*!< Select the PA1 gain of the TX1 channel */

    stateTypedef tiaHpfEn;          /*!< Select TIA HPF enable or disable state */
    stateTypedef rcHpfEn;           /*!< Select RC HPF enable or disable state */
    stateTypedef pgaHpfEn;          /*!< Select PGA HPF enable or disable state */
    tiaDiffGainTypedef tiaGain0;    /*!< Select the gear for RX0 channel TIA gain */
    tiaDiffGainTypedef tiaGain1;    /*!< Select the gear for RX1 channel TIA gain */
    pgaDiffGainTypedef pgaGain0;    /*!< Select the gear for RX0 channel PGA gain */
    pgaDiffGainTypedef pgaGain1;    /*!< Select the gear for RX1 channel PGA gain */
    tiaHpfTypedef tiaHpf;           /*!< Select the HPF gear for RX channel TIA */
    rcHpfTyepdef rcHpf;             /*!< Select the HPF gear for RX channel RC */
    pgaHpfTypedef pgaHpf;           /*!< Select the HPF gear for RX channel PGA */
    pgaLpfTypedef pgaLpf;           /*!< Select the LPF gear for RX channel PGA */
} AnalogDataStrTypedef;
/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup PMU_Exported_Functions
  * @{
  */
void adt6101PPowerSw(const powerStateTypedef onOroff);

void adt6101PGpioHz(void);


/**
  * @}
  */
/**
  * @}
  */
/**
  * @}
  */

#endif
