/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************
 * @file    adt6101P_type_define.h
 * @original_author  WuHao(hwu@andartechs.com.cn)
 * @version 1.0.0
 * @date    2020-11-24
 * @brief   This is a header file that defines data types and memory mapping.
 *
 *                 Copyright (c) 2020, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************
 */
#ifndef __ADT6101P_TYPE_DEFINE_H
#define __ADT6101P_TYPE_DEFINE_H




/*------------------------------ define -------------------------------------*/
/* exact-width signed integer types */
typedef   signed            char int8;
typedef   signed short      int int16;
typedef   signed            int int32;
typedef   long              long int64;
/* exact-width unsigned integer types */
typedef unsigned            char uint8;
typedef unsigned short      int uint16;
typedef unsigned            int uint32;
typedef unsigned long long  int uint64;

/*! \def SET_BIT(reg, bit)
    Set the value of the register's specific location.
*/
#define SET_BIT(reg, bit)           ((reg) |= (bit))

/*! \def CLEAR_BIT(reg, bit)
    Clear the value of the register's specific location.
*/
#define CLEAR_BIT(reg, bit)         ((reg) &= ~(bit))

/*! \def CLEAR_VALUE(reg)
    Clear the value of the register.
*/
#define CLEAR_VALUE(reg)            ((reg) = 0x0)

/*! \def SET_VALUE(reg, shift, mask, value)
    Clear and set the value of the specified range in the register.
*/
#define SET_VALUE(reg, shift, mask, value) \
        ((reg) = ((reg) & ~((uint32)(mask) << (shift))) | (((uint32)(value) & (mask)) << (shift)))


/*! \def GET_BIT_VALUE(reg, shift)
    Get the value of the register's specific location.
*/
#define GET_BIT_VALUE(reg, shift)     (((reg) >> (shift)) & 0x01)

/*! \def GET_BITS_VALUE(reg, shift, mask)
    Get  the value of the specified range in the register.
*/
#define GET_BITS_VALUE(reg, shift, mask) \
        (((reg) >> (shift)) & (mask))

/*! \def WRITE_REG(reg, val)
    write the value of the register.
*/
#define WRITE_REG(reg, val)   ((reg) = (val))

/*! \def READ_REG(reg)
    read the value of the register.
*/
#define READ_REG(reg)         ((reg))

/*! \def MODIFY_REG(reg, clearMask, setMask)
    modify the value of the register.
*/
#define MODIFY_REG(reg, clearMask, setMask)  \
    WRITE_REG((reg), (((READ_REG(reg)) & (~(clearMask))) | (setMask)))

/* define NVIC */
#define soc_NVIC          ((volatile uint32 *)0xE000E100)
#define soc_SETENA        0x00
#define soc_CLRENA        0x20
#define soc_SETPEND       0x80
#define soc_CLRPEND       0xa0

/* define System control block. */
#define soc_SCB           ((volatile uint32 *)0xE000E010)
#define soc_VTOR          ((volatile uint32 *)0xE000ED08)
#define soc_SCR           ((volatile uint32 *)0xE000ED10)
#define soc_AIRCR         ((volatile uint32 *)0xE000ED0C)

#define     __IO    volatile

/*-------------------Memory Mapping-----------------------*/
#define AHB_ROM           ((volatile uint32 *)0x00000000) //ROM     32KB  0x00000000~0x00007fff
#define AHB_RAMI          ((volatile uint32 *)0x00008000) //RAMI    32KB  0x00008000~0x0000ffff
#define AHB_RAMD          ((volatile uint32 *)0x20000000) //RAMD    32KB  0x20000000~0x20007fff

#define TIMER0_REGS       ((volatile uint32 *)0x40000000)   //Timer0's base address.
#define TIMER1_REGS       ((volatile uint32 *)0x40001000)   //Timer1's base address.
#define TIMER2_REGS       ((volatile uint32 *)0x40012000)   //Timer2's base address.
#define DUALTIMER0_REGS   ((volatile uint32 *)0x40002000)   //DualTimer0's base address.

#define RFC_REG	          ((volatile uint32 *)0x40003000)

#define UART0_REGS        ((volatile uint32 *)0x40004000)   //UART0's base address.
#define UART1_REGS        ((volatile uint32 *)0x40005000)   //UART1's base address.
#define UART2_REGS        ((volatile uint32 *)0x40006000)   //UART2's base address.
#define TRNG_REGS         ((volatile uint32 *)0x40007000)   //Trng's base address.
#define WATCHDOG_REGS     ((volatile uint32 *)0x40008000)   //Watchdog's base address.
#define I2C0_REGS         ((volatile uint32 *)0x40009000)   // I2C0 
#define I2C1_REGS         ((volatile uint32 *)0x4000a000)   // I2C1 
#define TEST_SLAVE_REGS   ((volatile uint32 *)0x4000b000)   //test slave's base address, for validation purpose.
#define PMU_REGS          ((volatile uint32 *)0x4000c000)
#define RTC_REGS          ((volatile uint32 *)0x4000d000)
#define SPI0_REGS         ((volatile uint32 *)0x4000e000)
#define SPI1_REGS         ((volatile uint32 *)0x4000f000)
#define CAN_REGS          ((volatile uint32 *)0x40010000)
#define GPIO_REGS         ((volatile uint32 *)0x40020000)
#define DMA_REGS          ((volatile uint32 *)0x40021000)
#define DSP_REGS          ((volatile uint32 *)0x40022000)

#define RTC                ((RTC_TypeDef       *) RTC_REGS)
#define PMU                ((PMU_TypeDef       *) PMU_REGS)
#define WATCHDOG           ((WATCHDOG_TypeDef  *) WATCHDOG_REGS)
#define DMA                ((DMA_TypeDef       *) DMA_REGS)
#define SPI0               ((SPI_TypeDef       *) SPI0_REGS)
#define SPI1               ((SPI_TypeDef       *) SPI1_REGS)
#define GPIO               ((GPIO_TypeDef      *) (GPIO_REGS))
#define I2C0               ((I2C_TypeDef       *) I2C0_REGS)
#define I2C1               ((I2C_TypeDef       *) I2C1_REGS)
#define UART0              ((UartTypeDef       *) UART0_REGS)
#define UART1              ((UartTypeDef       *) UART1_REGS)
#define UART2              ((UartTypeDef       *) UART2_REGS)
#define TRNG               ((TrngTypeDef       *) TRNG_REGS)
#define Timer0             ((TimerTypeDef      *) TIMER0_REGS)
#define Timer1             ((TimerTypeDef      *) TIMER1_REGS)
#define Timer2             ((TimerTypeDef      *) TIMER2_REGS)
#define Dualtimer          ((Dualtimer_TypeDef *) DUALTIMER0_REGS)
#define DSP                ((DSP_TypeDef       *) DSP_REGS)
#define FMCW               ((FMCW_TypeDef      *) FMCW_REG)
#define ATN_PSR            ((ATN_PSR_TypeDef   *) ATN_PSR_REG)
#define PDADC_RF           ((PDADC_RF_TypeDef  *) PDADC_RF_REG)
#define PLLI_PLLN          ((PLLI_PLLN_TypeDef *) PLLI_PLLN_REG)
#define LDO                ((LDO_TypeDef       *) LDO_REG)
#define RFC                ((RFC_TypeDef       *) RFC_REG)
#define VGA                ((VGA_TypeDef       *) VGA_REG)
#define CAN                ((CanStrTypeDef       *) CAN_REGS)

typedef enum
{
    DISABLE = 0,
    ENABLE  = 1
} stateTypedef;

typedef enum
{
    RESET = 0,
    SET   = 1
} bitStateTypedef;

typedef enum
{
    BIT_0 = 0,
    BIT_1,
    BIT_2,
    BIT_3,
    BIT_4,
    BIT_5,
    BIT_6,
    BIT_7,
    BIT_8,
    BIT_9,
    BIT_10,
    BIT_11,
    BIT_12,
    BIT_13,
    BIT_14,
    BIT_15,
    BIT_16,
    BIT_17,
    BIT_18,
    BIT_19,
    BIT_20,
    BIT_21,
    BIT_22,
    BIT_23,
    BIT_24,
    BIT_25,
    BIT_26,
    BIT_27,
    BIT_28,
    BIT_29,
    BIT_30,
    BIT_31,
    BIT_32
} bitTypedef;

#endif
