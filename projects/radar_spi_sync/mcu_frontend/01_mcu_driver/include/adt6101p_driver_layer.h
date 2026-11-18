/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 中; basis recorded in AUTHORS.json.
 */
#ifndef __ADT6101P_DRIVER_LAYER_H
#define __ADT6101P_DRIVER_LAYER_H

/**
 * @file adt6101p_driver_layer.h
 * @brief ADT6101P 驱动层分类封装（应用层统一入口）
 *
 * 设计目标：
 * 1) 将分散的 BSP 驱动按“系统控制 / IO / 通信 / 定时 / 低功耗”分类；
 * 2) 在应用层使用统一命名前缀 adtDrv*，降低跨模块调用心智负担；
 * 3) 保持对现有 BSP/HAL 的兼容，不改动底层实现。
 *
 * 推荐使用方式：
 * - 应用层（app）仅包含本头文件，不再直接散落包含多个 *_bsp.h；
 * - 通过 adtDrv* 结构体 + 初始化函数做配置，后续便于参数集中管理。
 */

#include "adt6101P_type_define.h"
#include "sys_bsp.h"
#include "gpio_bsp.h"
#include "uart_bsp.h"
#include "spi_bsp.h"
#include "timer_bsp.h"
#include "lowpower_bsp.h"

/* ========================= 系统控制（System） ========================= */

/**
 * @brief 配置中断开关（封装 setIntHandlerEn）。
 * @param interruptNum  中断号。
 * @param state         ENABLEINT / DISABLEINT。
 */
void adtDrvSystemSetInterrupt(interruptNumTypedef interruptNum, interruptStateTypedef state);

/**
 * @brief 切换系统主时钟到 100MHz。
 */
void adtDrvSystemSwitchHighClock100M(void);

/* =========================== IO（GPIO） =========================== */
typedef struct
{
    uint32 pinMask;           /*!< GPIO_PIN_x 按位或组合 */
    gpioModeTypedef mode;     /*!< GPIO_MODE_OUTPUT / GPIO_MODE_INPUT 等 */
    gpioPullTypedef pull;     /*!< GPIO_PULL_UP / GPIO_PULL_DOWN / GPIO_NO_PULL */
} adtDrvGpioConfig_t;

/**
 * @brief GPIO 初始化。
 *
 * 使用示例：
 * adtDrvGpioConfig_t cfg = {GPIO_PIN_2 | GPIO_PIN_9, GPIO_MODE_OUTPUT, GPIO_PULL_UP};
 * adtDrvGpioInit(&cfg);
 */
void adtDrvGpioInit(const adtDrvGpioConfig_t *cfg);

/**
 * @brief GPIO 输出电平。
 * @param pinMask  GPIO_PIN_x 按位或组合。
 * @param state    GPIO_PIN_SET / GPIO_PIN_RESET。
 */
void adtDrvGpioWrite(uint32 pinMask, gpioPinStateTypedef state);

/**
 * @brief GPIO 读取电平。
 * @param pinMask GPIO_PIN_x（通常传入单个 pin）。
 * @return GPIO_PIN_SET / GPIO_PIN_RESET。
 */
gpioPinStateTypedef adtDrvGpioRead(uint32 pinMask);

/* ========================== 通信（UART/SPI） ========================== */
typedef struct
{
    UartTypeDef *instance;    /*!< UART0/UART1/UART2... */
    uint32 baudRate;          /*!< 波特率，如 115200 / 2000000 */
    uint32 mode;              /*!< UART_MODE_TX | UART_MODE_RX */
    uint32 intEnableMask;     /*!< UART_INT_RX 等中断使能位 */
    UART_HANDLER irqHandler;  /*!< 串口中断回调函数 */
} adtDrvUartConfig_t;

/**
 * @brief UART 初始化。
 * @param cfg UART 配置结构体指针。
 * @return UART_OK / UART_ERROR。
 *
 * 使用示例：
 * adtDrvUartConfig_t uartCfg = {0};
 * uartCfg.instance = UART1;
 * uartCfg.baudRate = 2000000;
 * uartCfg.mode = UART_MODE_TX | UART_MODE_RX;
 * uartCfg.intEnableMask = UART_INT_RX;
 * uartCfg.irqHandler = uart1InterruptHandler;
 * adtDrvUartInit(&uartCfg);
 */
uartStatusTypedef adtDrvUartInit(const adtDrvUartConfig_t *cfg);

typedef struct
{
    spiModeTypedef mode;              /*!< SPI_Master / SPI_Slave */
    spiDataSizeTypedef dataSize;      /*!< SPI_DATASIZE_8BIT / 16BIT */
    spiPrescalerTypedef prescaler;    /*!< SPI 时钟分频 */
    uint16 intEnableMask;             /*!< SPI_INTSTATUS_xxx */
    SPI_HANDLER irqHandler;           /*!< SPI 中断回调 */
} adtDrvSpiConfig_t;

/**
 * @brief SPI 初始化（默认 Master 模式常用参数）。
 * @param instance SPI0/SPI1 实例。
 * @param cfg      SPI 配置结构体指针。
 */
void adtDrvSpiInit(SPI_TypeDef *instance, const adtDrvSpiConfig_t *cfg);

/* ============================ 定时（Timer） ============================ */
typedef struct
{
    TimerTypeDef *instance;       /*!< Timer0/Timer1 */
    timerCtrlModeTypeDef mode;    /*!< DEFUALT_MODE 等 */
    uint32 reload;                /*!< 重装载值（由 adtDrvTimerCalcReload 计算） */
    TIMER_HANDLER irqHandler;     /*!< 定时器中断回调 */
} adtDrvTimerConfig_t;

/**
 * @brief 通用定时器初始化。
 * @param cfg 定时器配置结构体指针。
 */
void adtDrvTimerInit(const adtDrvTimerConfig_t *cfg);

/**
 * @brief 由时间周期计算硬件 reload 值。
 * @param periodMs 周期参数（与底层 getReload 保持一致）。
 * @return reload 寄存器值。
 */
uint32 adtDrvTimerCalcReload(float periodMs);

/* =========================== 低功耗（LowPower） =========================== */
typedef struct
{
    ramStateTypedef ramState;      /*!< PD_RAM_ON / PD_RAM_OFF */
    uint16 sleepTimeMs;            /*!< 休眠时间，单位 ms */
    wakeUpModeTypedef wakeupMode;  /*!< RTC / GPIO / RTC_AND_GPIO */
    uint32 wakeupPinMask;          /*!< GPIO 唤醒 pin mask */
    gpioWakeUpTypedef wakeupLevel; /*!< 高/低电平唤醒 */
} adtDrvWakeupConfig_t;

/**
 * @brief 休眠唤醒配置（RTC/GPIO）。
 * @param cfg 唤醒配置结构体指针。
 */
void adtDrvWakeupInit(const adtDrvWakeupConfig_t *cfg);

#endif /* __ADT6101P_DRIVER_LAYER_H */
