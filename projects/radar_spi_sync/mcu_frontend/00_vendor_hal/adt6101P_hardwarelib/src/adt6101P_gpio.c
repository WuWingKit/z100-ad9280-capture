/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_gpio.c
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-18
 * @brief   This file contains all the functions implemented by the GPIO peripheral
 *          register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_gpio.h"
#include "gpio_ctype_map.h"


/* Private functions --------------------------------------------------------*/
/**
 * @brief   get Gpio Interrupt State
 * @param   None.
 * @return  Interrupt State: gpio0 ~ gpio30.
 */
uint32 getGpioInterruptState(void)
{
    return GPIO->gpio_int_state;
}

/**
 * @brief   clear Gpio Interrupt
 * @param   gpioPin: gpio0 ~ gpio30.
 * @return  Interrupt State.
 */
void clearGpioInterrupt(uint32 gpioPin)
{
    SET_BIT(GPIO->gpio_int_clear, gpioPin);
}

/**
 * @brief   set the interrupt enable for gpio.
 * @param   gpioPin: gpio0 ~ gpio30.
 * @param   enable: 1: enable.  0: disable.
 * @return  None.
 */
void setGpioInterruptEnable(uint32 gpioPin, stateTypedef enable)
{
    if(enable == ENABLE)
    {
        SET_BIT(GPIO->gpio_int_en, gpioPin);
    }
    else
    {
        CLEAR_BIT(GPIO->gpio_int_en, gpioPin);
    }
}

/**
 * @brief   get the interrupt enable for gpio.
 * @param   None.
 * @return  Interrupt enable: gpio0 ~ gpio30.
 */
uint32 getGpioInterruptEnable(void)
{
    return GPIO->gpio_int_en;
}

/**
 * @brief   set Gpio Interrupt Trigger.
 * @param   gpioPin: gpio0 ~ gpio30.
 * @param   trigger: 0: level mode: high level.
                     3: edge mode: posedge.
                     4: edge mode: negedge.
 * @return  None.
 */
void setGpioInterruptTrigger(uint32 gpioPin, triggerTypedef trigger)
{
    switch(trigger)
    {
        case LEVEL_MODE_HIGH_LEVEL:
        {
            CLEAR_BIT(GPIO->gpio_int_edge_nlevel, gpioPin);
            SET_BIT(GPIO->gpio_int_pol, gpioPin);
            break;
        }
        case EDGE_MODE_POSEDGE:
        {
            SET_BIT(GPIO->gpio_int_edge_nlevel, gpioPin);
            SET_BIT(GPIO->gpio_int_pol, gpioPin);
            break;
        }
        case EDGE_MODE_NEGEDGE:
        {
            SET_BIT(GPIO->gpio_int_edge_nlevel, gpioPin);
            CLEAR_BIT(GPIO->gpio_int_pol, gpioPin);
            break;
        }
        default:
        {
            break;
        }
    }
}

/**
 * @brief   get Gpio Int Edge Nlevel.
 * @param   None.
 * @return  Int Edge Nlevel: gpio0 ~ gpio30.
 */
uint32 getGpioIntEdgeNlevel(void)
{
    return GPIO->gpio_int_edge_nlevel;
}

/**
 * @brief   get Gpio Int Pol.
 * @param   None.
 * @return  Int Pol: gpio0 ~ gpio30.
 */
uint32 getGpioIntPol(void)
{
    return GPIO->gpio_int_pol;
}

/**
 * @brief   set Gpio Wake Pol.
 * @param   gpioPin: gpio0 ~ gpio30.
 * @param   mode: 1: low level to wake up.  0: high level to wake up.
 * @return  None.
 */
void setGpioWakePol(uint32 gpioPin, gpioWakeUpTypedef wakeUpPol)
{
    if(wakeUpPol == LOW_LEVEL)
    {
        SET_BIT(GPIO->gpio_wake_pol, gpioPin);
    }
    else
    {
        CLEAR_BIT(GPIO->gpio_wake_pol, gpioPin);
    }
}

/**
 * @brief   get Gpio Wake Pol.
 * @param   None.
 * @return  Wake Pol: gpio0 ~ gpio30.
 */
uint32 getGpioWakePol(void)
{
    return GPIO->gpio_wake_pol;
}

/**
 * @brief   set Gpio Wake Enable.
 * @param   gpioPin: gpio0 ~ gpio30.
 * @param   enable: 1: enable.  0: disable.
 */
void setGpioWakeEnable(uint32 gpioPin, stateTypedef enable)
{
    if(enable == ENABLE)
    {
        SET_BIT(GPIO->gpio_wake_en, gpioPin);
    }
    else
    {
        CLEAR_BIT(GPIO->gpio_wake_en, gpioPin);
    }
}

/**
 * @brief   get Gpio Wake Enable.
 * @param   None.
 * @return  Wake Enable: gpio0 ~ gpio30.
 */
uint32 getGpioWakeEnable(void)
{
    return GPIO->gpio_wake_en;
}

/**
 * @brief   clear Gpio Output.
 * @param   gpioPin: gpio0 ~ gpio30.
 * @return  None.
 */
void clearGpioOutput(uint32 gpioPin)
{
    CLEAR_BIT(GPIO->gpio_out_clear, gpioPin);
}

/**
 * @brief   get Gpio Output State.
 * @param   None.
 * @return  Output State: gpio0 ~ gpio30.
 */
uint32 getGpioOutputState(void)
{
    return GPIO->gpio_out_state;
}

/**
 * @brief   set Gpio Config Pid.
 * @param   gpioPinCfg: gpio0 ~ gpio30: 0~30.
 * @param   pid: function of port.
 * @return  None.
 */
void setGpioConfigPid(gpioConfigPinTypedef gpioCfgPin, gpioPidTypedef pid)
{
    SET_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), \
              gpio_config_pid_shift, gpio_config_pid_mask,  \
              pid);
}

/**
 * @brief   get Gpio Config Pid
 * @param   gpioPinCfg: gpio0 ~ gpio30: 0~30.
 * @return  Gpio Config Pid.
 */
uint32 getGpioConfigPid(gpioConfigPinTypedef gpioCfgPin)
{
    return  GET_BITS_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), \
                           gpio_config_pid_shift, gpio_config_pid_mask);
}

/**
 * @brief   set Gpio Config Mode.
 * @param   gpioCfgPin: gpio0 ~ gpio30: 0~30.
 * @param   mode: GPIO set output or input or high inpendence
 *                     0:output
 *                     1:input
 *                     2:high inpendence
 *                     3:output & input
 * @return  None.
 */
void setGpioConfigMode(gpioConfigPinTypedef gpioCfgPin, gpioModeTypedef mode)
{
    switch(mode)
    {
        case GPIO_MODE_OUTPUT:
        {
            CLEAR_BIT(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_oen_bit | gpio_config_ie_bit);
            break;
        }
        case GPIO_MODE_INPUT:
        {
            SET_BIT(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_oen_bit | gpio_config_ie_bit);
            break;
        }
        case GPIO_MODE_HIGH_INPENDECE:
        {
            SET_BIT(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_oen_bit);
            CLEAR_BIT(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_ie_bit);
            break;
        }
        case GPIO_MODE_OUTPUT_AND_INPUT:
        {
            CLEAR_BIT(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_oen_bit);
            SET_BIT(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_ie_bit);
            break;
        }
        default:
        {
            break;
        }
    }
}

/**
 * @brief   get Gpio Config Mode
 * @param   gpioPinCfg: gpio0 ~ gpio30: 0~30.
 * @return  Gpio Config Mode.
 */
gpioModeTypedef getGpioConfigMode(gpioConfigPinTypedef gpioCfgPin)
{
    gpioModeTypedef mode;

    if(GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_oen_shift) == 0x0 && \
            GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_ie_shift) == 0x0)
    {
        mode = GPIO_MODE_OUTPUT;
    }
    else if(GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_oen_shift) == 0x1 && \
            GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_ie_shift) == 0x1)
    {
        mode = GPIO_MODE_INPUT;
    }
    else if(GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_oen_shift) == 0x1 && \
            GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_ie_shift) == 0x0)
    {
        mode = GPIO_MODE_HIGH_INPENDECE;
    }
    else if(GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_oen_shift) == 0x0 && \
            GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_ie_shift) == 0x1)
    {
        mode = GPIO_MODE_OUTPUT_AND_INPUT;
    }

    return mode;
}

/**
 * @brief   set Gpio Config Pull.
 * @param   gpioCfgPin: gpio0 ~ gpio30: 0~30.
 * @param   gpioPull: 0: no pull.  1: pull up. 2: pull down.
 * @return  None.
 */
void setGpioConfigPull(gpioConfigPinTypedef gpioCfgPin, gpioPullTypedef gpioPull)
{
    switch(gpioPull)
    {
        case GPIO_NO_PULL:
        {
            CLEAR_BIT(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_pd_bit | gpio_config_pu_bit);
            break;
        }
        case GPIO_PULL_UP:
        {
            CLEAR_BIT(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_pd_bit);
            SET_BIT(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_pu_bit);
            break;
        }
        case GPIO_PULL_DOWN:
        {
            SET_BIT(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_pd_bit);
            CLEAR_BIT(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_pu_bit);
            break;
        }
        default:
        {
            break;
        }
    }
}

/**
 * @brief   get Gpio Config Pull
 * @param   gpioPinCfg: gpio0 ~ gpio30: 0~30.
 * @return  Gpio Config Pull.
 */
gpioPullTypedef getGpioConfigPull(gpioConfigPinTypedef gpioCfgPin)
{
    gpioPullTypedef gpioPull;

    if(GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_pd_shift) == 0x0 && \
            GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_pu_shift) == 0x0)
    {
        gpioPull = GPIO_NO_PULL;
    }
    else if(GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_pd_shift) == 0x0 && \
            GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_pu_shift) == 0x1)
    {
        gpioPull = GPIO_PULL_UP;
    }
    else if(GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_pd_shift) == 0x1 && \
            GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_pu_shift) == 0x0)
    {
        gpioPull = GPIO_PULL_DOWN;
    }

    return gpioPull;
}

/**
 * @brief   set Gpio Config Sr.
 * @param   gpioCfgPin: gpio0 ~ gpio30: 0~30.
 * @param   sr: 0: slow (half frequency). 1:  fast.
 * @return  None.
 */
void setGpioConfigSr(gpioConfigPinTypedef gpioCfgPin, srTypedef sr)
{
    if(sr == FAST)
    {
        SET_BIT(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_sr_bit);
    }
    else
    {
        CLEAR_BIT(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_sr_bit);
    }
}

/**
 * @brief   get Gpio Config Sr
 * @param   gpioPinCfg: gpio0 ~ gpio30: 0~30.
 * @return  Gpio Config Sr.
 */
srTypedef getGpioConfigSr(gpioConfigPinTypedef gpioCfgPin)
{
    srTypedef sr;

    if(GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_sr_shift) == 0x0)
    {
        sr = SLOW;
    }
    else
    {
        sr = FAST;
    }

    return sr;
}

/**
 * @brief   set Gpio Config Drive.
 * @param   gpioCfgPin: gpio0 ~ gpio30: 0~30.
 * @param   drive: 0: 2 mA, 1: 4 mA, 2: 8 mA 3: 12 mA
 * @return  None.
 */
void setGpioConfigDrive(gpioConfigPinTypedef gpioCfgPin, gpioDriveTypedef drive)
{
    SET_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), \
              gpio_config_e_shift, gpio_config_e_mask,      \
              drive);
}

/**
 * @brief   get Gpio Config Drive.
 * @param   gpioCfgPin: gpio0 ~ gpio30: 0~30.
 * @return  Gpio Config drive:drive: 0: 2 mA, 1: 4 mA, 2: 8 mA 3: 12 mA.
 */
gpioDriveTypedef getGpioConfigDrive(gpioConfigPinTypedef gpioCfgPin)
{
    return (gpioDriveTypedef) GET_BITS_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), \
            gpio_config_e_shift, gpio_config_e_mask);
}

/**
 * @brief   set Gpio Config Smt.
 * @param   gpioCfgPin: gpio0 ~ gpio30: 0~30.
 * @param   smt: 0: no hysteresis. 1:  Schmitt trigger.
 * @return  None.
 */
void setGpioConfigSmt(gpioConfigPinTypedef gpioCfgPin, smtTypedef smt)
{
    if(smt == SCHMITT_TRIGGER)
    {
        SET_BIT(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_smt_bit);
    }
    else
    {
        CLEAR_BIT(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_smt_bit);
    }
}

/**
 * @brief   get Gpio Config Smt
 * @param   gpioPinCfg: gpio0 ~ gpio30: 0~30.
 * @return  Gpio Config Smt.
 */
smtTypedef getGpioConfigSmt(gpioConfigPinTypedef gpioCfgPin)
{
    smtTypedef smt;

    if(GET_BIT_VALUE(*((uint32 *)(GPIO_CONFIG_REGS + gpioCfgPin)), gpio_config_smt_shift) == 0x0)
    {
        smt = NO_HYSTERESIS;
    }
    else
    {
        smt = NO_HYSTERESIS;
    }

    return smt;
}
