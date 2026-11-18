/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 中; basis recorded in AUTHORS.json.
 */
#include "adt2011_gpio_bus.h"

#include "gpio_bsp.h"
#include "CMSDK_CM3.h"

#define ADT2011_RESET_PIN             GPIO_PIN_27
#define ADT2011_CLOCK_PIN             GPIO_PIN_8
#define ADT2011_MOSI_PIN              GPIO_PIN_21
#define ADT2011_MISO_PIN              GPIO_PIN_20
#define ADT2011_TX_SELECT_PIN         GPIO_PIN_30
#define ADT2011_RX_SELECT_PIN         GPIO_PIN_29
#define ADT2011_FLASH_SELECT_PIN      GPIO_PIN_4

/* Board measurement: about ten volatile iterations per microsecond. */
#define ADT2011_DELAY_LOOPS_PER_US     (10u)

static void gpio_bus_delay_us(uint32 delayUs)
{
    volatile uint32 remaining = delayUs * ADT2011_DELAY_LOOPS_PER_US;

    while(remaining > 0u)
    {
        --remaining;
    }
}

static uint8_t gpio_bus_initialize(void *context)
{
    gpioInitStrTypedef gpioConfig;

    (void)context;
    gpioConfig.Mode = GPIO_MODE_OUTPUT;
    gpioConfig.Pull = GPIO_PULL_UP;
    gpioConfig.Pin = ADT2011_CLOCK_PIN |
                     ADT2011_MOSI_PIN |
                     ADT2011_TX_SELECT_PIN |
                     ADT2011_RX_SELECT_PIN |
                     ADT2011_FLASH_SELECT_PIN;
    gpioInit(&gpioConfig);

    gpioConfig.Mode = GPIO_MODE_INPUT;
    gpioConfig.Pull = GPIO_NO_PULL;
    gpioConfig.Pin = ADT2011_MISO_PIN;
    gpioInit(&gpioConfig);

    gpioWritePin(ADT2011_CLOCK_PIN, GPIO_PIN_RESET);
    gpioWritePin(ADT2011_TX_SELECT_PIN, GPIO_PIN_SET);
    gpioWritePin(ADT2011_RX_SELECT_PIN, GPIO_PIN_SET);
    gpioWritePin(ADT2011_FLASH_SELECT_PIN, GPIO_PIN_SET);
    return 1u;
}

static uint8_t gpio_bus_reset(void *context)
{
    gpioInitStrTypedef gpioConfig;

    (void)context;
    gpioConfig.Mode = GPIO_MODE_OUTPUT;
    gpioConfig.Pull = GPIO_PULL_UP;
    gpioConfig.Pin = ADT2011_RESET_PIN;
    gpioInit(&gpioConfig);

    gpioWritePin(ADT2011_RESET_PIN, GPIO_PIN_SET);
    gpio_bus_delay_us(4000u);
    gpioWritePin(ADT2011_RESET_PIN, GPIO_PIN_RESET);
    gpio_bus_delay_us(4000u);
    gpioWritePin(ADT2011_RESET_PIN, GPIO_PIN_SET);
    return 1u;
}

static uint8_t gpio_bus_transfer_byte(uint8_t value)
{
    int8 bitIndex;
    uint8_t received = 0u;

    for(bitIndex = 7; bitIndex >= 0; --bitIndex)
    {
        gpioWritePin(ADT2011_CLOCK_PIN, GPIO_PIN_RESET);
        gpioWritePin(ADT2011_MOSI_PIN,
                     (((value >> bitIndex) & 0x01u) != 0u) ?
                     GPIO_PIN_SET : GPIO_PIN_RESET);
        gpio_bus_delay_us(1u);
        gpioWritePin(ADT2011_CLOCK_PIN, GPIO_PIN_SET);
        received = (uint8_t)((received << 1) |
                             gpioReadPin(ADT2011_MISO_PIN));
        gpio_bus_delay_us(1u);
    }
    gpioWritePin(ADT2011_CLOCK_PIN, GPIO_PIN_RESET);
    return received;
}

static uint8_t gpio_bus_transfer(void *context,
                                 Adt2011ChipTypedef chip,
                                 const uint8_t *txData,
                                 uint8_t *rxData,
                                 uint16_t length)
{
    uint16_t index;
    uint32_t primask;

    (void)context;
    if((txData == (const uint8_t *)0) ||
       (length == 0u) ||
       (chip > ADT2011_CHIP_RX))
    {
        return 0u;
    }

    /* Keep each GPIO-bus register transaction atomic. GPIO SYNC edges can
     * latch in hardware while interrupts are masked, but the ISR and ADC
     * completion handler never perform blocking phase-shifter transactions.
     */
    primask = __get_PRIMASK();
    __disable_irq();

    gpioWritePin(ADT2011_TX_SELECT_PIN,
                 (chip == ADT2011_CHIP_TX) ?
                 GPIO_PIN_RESET : GPIO_PIN_SET);
    gpioWritePin(ADT2011_RX_SELECT_PIN,
                 (chip == ADT2011_CHIP_RX) ?
                 GPIO_PIN_RESET : GPIO_PIN_SET);
    gpio_bus_delay_us(1u);

    for(index = 0u; index < length; ++index)
    {
        uint8_t received = gpio_bus_transfer_byte(txData[index]);
        if(rxData != (uint8_t *)0)
        {
            rxData[index] = received;
        }
    }

    gpio_bus_delay_us(1u);
    gpioWritePin(ADT2011_TX_SELECT_PIN, GPIO_PIN_SET);
    gpioWritePin(ADT2011_RX_SELECT_PIN, GPIO_PIN_SET);
    __set_PRIMASK(primask);
    return 1u;
}

void Adt2011GpioBus_Create(Adt2011BusTypedef *bus)
{
    if(bus == (Adt2011BusTypedef *)0)
    {
        return;
    }

    bus->context = (void *)0;
    bus->initialize = gpio_bus_initialize;
    bus->reset = gpio_bus_reset;
    bus->transfer = gpio_bus_transfer;
}
