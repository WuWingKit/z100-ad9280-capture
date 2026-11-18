/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 * Andar chip register/protocol content; project adapter author confirmed by owner.
 */
#include "adt2011_driver.h"

static Adt2011StatusTypedef writeRegister(
    Adt2011DriverTypedef *driver,
    uint16_t addr,
    uint8_t data,
    Adt2011ChipTypedef chip);
static Adt2011StatusTypedef readRegister(
    Adt2011DriverTypedef *driver,
    uint16_t addr,
    uint8_t *data,
    Adt2011ChipTypedef chip);

/**
 * @brief  初始化 ADT2011（GPIO 方向/上下拉、复位、寄存器默认配置）。
 * @note   本函数会对两片芯片（id0/id1）写入寄存器表，并进行必要的 GPIO 初始化。
 */
Adt2011StatusTypedef Adt2011Driver_Initialize(
    Adt2011DriverTypedef *driver,
    const Adt2011BusTypedef *bus)
{
    if((driver == (Adt2011DriverTypedef *)0) ||
       (bus == (const Adt2011BusTypedef *)0) ||
       (bus->initialize == 0) ||
       (bus->reset == 0) ||
       (bus->transfer == 0))
    {
        return ADT2011_STATUS_BAD_PARAM;
    }
    driver->bus = bus;
    driver->initialized = 0u;
    driver->rfEnabled = 0u;
    driver->beamPhaseValid[ADT2011_CHIP_TX] = 0u;
    driver->beamPhaseValid[ADT2011_CHIP_RX] = 0u;
    if((bus->initialize(bus->context) == 0u) ||
       (bus->reset(bus->context) == 0u))
    {
        return ADT2011_STATUS_BUS_ERROR;
    }

	const uint8_t regTxId0[256] =   // each line number is 16
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0x00
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
		0, 0, 255, 0, 0, 0x3e, 0, 0, 0, 0, 0, 0x00, 0, 0xff, 0xff, 0x0e, //
		0, 0, 0, 3, 0x80, 0, 0, 0, 0, 0, 0, 0, 0xfe, 0x37, 0x80, 10, //
		11, 11, 0x0a, 15, 13, 9, 10, 10, 10, 10, 10, 13, 13, 0x0f, 15, 13, //
		8, 10, 7, 0x0A, 0x0A, 10, 10, 10, 11, 15, 15, 10, 14, 15, 15, 10, //
		10, 10, 10, 0x0f, 15, 0x0A, 12, 6, 10, 8, 7, 10, 0x0A, 10, 10, 8, //
		8, 10, 10, 10, 10, 0, 0, 8, 10, 10, 10, 10, 10, 10, 10, 10, //
		10, 10, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0, //0x08
		0, 0, 0, 0, 0, 0, 0, 0x0d, 0x88, 0, 3, 0x11, 2, 0, 0, 0x85, //
		0, 0x00, 0xe6, 0x20, 0x6f, 0xbf, 0x5d, 4, 0x20, 0xc8, 4, 0, 0, 0, 3, 16, //
		1, 0x30, 9, 4, 4, 8, 8, 8, 10, 8, 8, 1, 4, 0, 0, 0, //
		8, 8, 8, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0x0f, 0x10, 0xe3, //
		8, 8, 8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, //
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 //0x0F
	};

    for (int i = 0x00; i < 0x100; i++)
    {
        if(writeRegister(driver,
                         (uint16_t)i,
                         regTxId0[i],
                         ADT2011_CHIP_TX) != ADT2011_STATUS_OK)
        {
            return ADT2011_STATUS_BUS_ERROR;
        }
    }

    for (int i = 0x3f; i <= 0x6e; i++)
    {
        if(writeRegister(driver,
                         (uint16_t)i,
                         10u,
                         ADT2011_CHIP_TX) != ADT2011_STATUS_OK)
        {
            return ADT2011_STATUS_BUS_ERROR;
        }
    }

	const uint8_t regRxId1[256] =    // each line number is 16
	{
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0x00
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0x2f, 0, 0, 0, 0, 0, 0x40, 2, 0, 0, 2, //
        0, 0, 0, 2, 0x80, 0, 0, 0, 0, 0, 0, 0, 0xfe, 0x37, 0x80, 12, //
        10, 15, 15, 12, 10, 10, 10, 10, 10, 10, 10, 15, 15, 10, 10, 13, //
        10, 10, 10, 10, 10, 10, 10, 10, 10, 15, 15, 13, 10, 15, 15, 10, //
        10, 10, 10, 10, 10, 10, 10, 13, 10, 10, 10, 10, 10, 10, 10, 8, //
        8, 10, 10, 10, 10, 0, 0, 8, 10, 10, 10, 10, 10, 10, 10, 10, //
        10, 10, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0, //0x08
        0, 0, 0, 0, 0, 0, 0, 0x0b, 0x88, 0, 0, 0x11, 2, 0, 4, 0xca, //
        0, 0x3d, 0xe6, 0x21, 0x6f, 0xbf, 0xdd, 4, 0x20, 0xc8, 4, 0, 0, 0, 3, 16, //
        1, 0x30, 8, 0, 4, 8, 8, 8, 10, 8, 8, 1, 4, 0, 0, 0, //
        8, 8, 8, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0x0f, 0x10, 0xe3, //
        8, 8, 8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 //0x0F
	};

    for (int i = 0x00; i < 0x100; i++)
    {
        if(writeRegister(driver,
                         (uint16_t)i,
                         regRxId1[i],
                         ADT2011_CHIP_RX) != ADT2011_STATUS_OK)
        {
            return ADT2011_STATUS_BUS_ERROR;
        }
    }
    
    for (int i = 0x3f; i <= 0x6e; i++)
    {
        if(writeRegister(driver,
                         (uint16_t)i,
                         10u,
                         ADT2011_CHIP_RX) != ADT2011_STATUS_OK)
        {
            return ADT2011_STATUS_BUS_ERROR;
        }
    }
    if((writeRegister(driver, 0x9au, 0x03u, ADT2011_CHIP_RX) != ADT2011_STATUS_OK) ||
       (writeRegister(driver, 0x33u, 0x02u, ADT2011_CHIP_RX) != ADT2011_STATUS_OK) ||
       (writeRegister(driver, 0x98u, 0x88u, ADT2011_CHIP_RX) != ADT2011_STATUS_OK) ||
       (writeRegister(driver, 0xa4u, 0x11u, ADT2011_CHIP_RX) != ADT2011_STATUS_OK) ||
       (writeRegister(driver, 0x9fu, 0xAAu, ADT2011_CHIP_RX) != ADT2011_STATUS_OK) ||
       (writeRegister(driver, 0x2cu, 0x01u, ADT2011_CHIP_RX) != ADT2011_STATUS_OK))
    {
        return ADT2011_STATUS_BUS_ERROR;
    }
    uint8_t data = 0u;
    if((readRegister(driver, 0x9fu, &data, ADT2011_CHIP_RX) != ADT2011_STATUS_OK) ||
       (readRegister(driver, 0x2cu, &data, ADT2011_CHIP_RX) != ADT2011_STATUS_OK))
    {
        return ADT2011_STATUS_BUS_ERROR;
    }
    driver->initialized = 1u;
    return Adt2011Driver_SetRfEnabled(driver, 0u);
}

static Adt2011StatusTypedef writeRegister(
    Adt2011DriverTypedef *driver,
    uint16_t addr,
    uint8_t data,
    Adt2011ChipTypedef chip)
{
    uint8_t sendBuffer[3];
    uint8_t chipId = (uint8_t)chip;
    uint8_t addressHigh = (uint8_t)((addr >> 8) & 0x03u);

    sendBuffer[0] = (uint8_t)((chipId << 2) | addressHigh);
    sendBuffer[1] = (uint8_t)(addr & 0xFFu);
    sendBuffer[2] = data;
    return (driver->bus->transfer(driver->bus->context,
                                  chip,
                                  sendBuffer,
                                  (uint8_t *)0,
                                  3u) != 0u) ?
           ADT2011_STATUS_OK : ADT2011_STATUS_BUS_ERROR;
}

static Adt2011StatusTypedef readRegister(
    Adt2011DriverTypedef *driver,
    uint16_t addr,
    uint8_t *data,
    Adt2011ChipTypedef chip)
{
    uint8_t sendBuffer[3] = {0u, 0u, 0u};
    uint8_t receiveBuffer[3] = {0u, 0u, 0u};
    uint8_t chipId = (uint8_t)chip;
    uint8_t addressHigh = (uint8_t)((addr >> 8) & 0x03u);

    sendBuffer[0] = (uint8_t)(0x80u | (chipId << 2) | addressHigh);
    sendBuffer[1] = (uint8_t)(addr & 0xFFu);
    sendBuffer[2] = 0xFFu;
    if(driver->bus->transfer(driver->bus->context,
                             chip,
                             sendBuffer,
                             receiveBuffer,
                             3u) == 0u)
    {
        return ADT2011_STATUS_BUS_ERROR;
    }
    *data = receiveBuffer[2];
    return ADT2011_STATUS_OK;
}

/**
 * @brief 预写 16 路波束相位，只写相对上一份 PSR 配置发生变化的寄存器。
 * @note  本函数只写 PSR shadow 寄存器 1,3,...,31，不写 load。
 *        第一次配置该芯片时完整写 16 路；之后仅写差分项。
 */
Adt2011StatusTypedef Adt2011Driver_WriteBeamConfig(
    Adt2011DriverTypedef *driver,
    Adt2011ChipTypedef chip,
    const uint8_t phase[16])
{
    uint8_t channel;

    if((driver == (Adt2011DriverTypedef *)0) ||
       (phase == (const uint8_t *)0) ||
       (chip > ADT2011_CHIP_RX))
    {
        return ADT2011_STATUS_BAD_PARAM;
    }
    if(driver->initialized == 0u)
    {
        return ADT2011_STATUS_NOT_INITIALIZED;
    }

    for(channel = 0u; channel < 16u; ++channel)
    {
        const uint8_t value = (uint8_t)(phase[channel] & 0x3Fu);
        if((driver->beamPhaseValid[chip] == 0u) ||
           (driver->beamPhaseCache[chip][channel] != value))
        {
            if(writeRegister(driver,
                             (uint16_t)(channel * 2u + 1u),
                             value,
                             chip) != ADT2011_STATUS_OK)
            {
                driver->beamPhaseValid[chip] = 0u;
                return ADT2011_STATUS_BUS_ERROR;
            }
            driver->beamPhaseCache[chip][channel] = value;
        }
    }

    driver->beamPhaseValid[chip] = 1u;
    return ADT2011_STATUS_OK;
}

/**
 * @brief 写 ADT2011 load 寄存器，将已经预写的 PSR shadow 配置切换为当前波束。
 * @note  load 与 PSR 写寄存器故意分离，供 ADC DMA 采集期间预写下一波束。
 */
Adt2011StatusTypedef Adt2011Driver_LoadBeam(
    Adt2011DriverTypedef *driver,
    Adt2011ChipTypedef chip)
{
    if((driver == (Adt2011DriverTypedef *)0) ||
       (chip > ADT2011_CHIP_RX))
    {
        return ADT2011_STATUS_BAD_PARAM;
    }
    if(driver->initialized == 0u)
    {
        return ADT2011_STATUS_NOT_INITIALIZED;
    }
    if(driver->beamPhaseValid[chip] == 0u)
    {
        return ADT2011_STATUS_BAD_PARAM;
    }

    if(writeRegister(driver, 32u, 1u, chip) != ADT2011_STATUS_OK)
    {
        /* 总线异常后无法确认 shadow/active 状态，下一次强制完整重写。 */
        driver->beamPhaseValid[chip] = 0u;
        return ADT2011_STATUS_BUS_ERROR;
    }
    return ADT2011_STATUS_OK;
}

/**
 * @brief 立即配置并加载一个波束。
 * @note  用于初始化和诊断采集；默认跟踪使用 WriteBeamConfig()+LoadBeam() 流水。
 */
Adt2011StatusTypedef Adt2011Driver_SetBeam(
    Adt2011DriverTypedef *driver,
    Adt2011ChipTypedef chip,
    const uint8_t phase[16])
{
    Adt2011StatusTypedef status =
        Adt2011Driver_WriteBeamConfig(driver, chip, phase);

    if(status != ADT2011_STATUS_OK)
    {
        return status;
    }
    return Adt2011Driver_LoadBeam(driver, chip);
}

/**
 * @brief  射频通道开关（Tx/Rx 0~15）。
 * @param  on 1: 打开所有通道；0: 关闭所有通道。
 */
Adt2011StatusTypedef Adt2011Driver_SetRfEnabled(
    Adt2011DriverTypedef *driver,
    uint8_t enabled)
{
    if(driver == (Adt2011DriverTypedef *)0)
    {
        return ADT2011_STATUS_BAD_PARAM;
    }
    if(driver->initialized == 0u)
    {
        return ADT2011_STATUS_NOT_INITIALIZED;
    }

    if(enabled != 0u)
    {
        /* Receive path is ready before radiation starts. */
        if((writeRegister(driver, 0x21u, 0xFFu, ADT2011_CHIP_RX) != ADT2011_STATUS_OK) ||
           (writeRegister(driver, 0x22u, 0xFFu, ADT2011_CHIP_RX) != ADT2011_STATUS_OK) ||
           (writeRegister(driver, 0x21u, 0xFFu, ADT2011_CHIP_TX) != ADT2011_STATUS_OK) ||
           (writeRegister(driver, 0x22u, 0xFFu, ADT2011_CHIP_TX) != ADT2011_STATUS_OK))
        {
            return ADT2011_STATUS_BUS_ERROR;
        }
    }
    else
    {
        /* Stop radiation before disabling the receive path. */
        if((writeRegister(driver, 0x21u, 0x00u, ADT2011_CHIP_TX) != ADT2011_STATUS_OK) ||
           (writeRegister(driver, 0x22u, 0x00u, ADT2011_CHIP_TX) != ADT2011_STATUS_OK) ||
           (writeRegister(driver, 0x21u, 0x00u, ADT2011_CHIP_RX) != ADT2011_STATUS_OK) ||
           (writeRegister(driver, 0x22u, 0x00u, ADT2011_CHIP_RX) != ADT2011_STATUS_OK))
        {
            return ADT2011_STATUS_BUS_ERROR;
        }
    }
    driver->rfEnabled = (uint8_t)(enabled != 0u);
    return ADT2011_STATUS_OK;
}
