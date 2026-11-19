/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_can.h
 * @original_author  DingZhen(zding@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-03-11
 * @brief   This file provides a CAN universal peripheral function interface, making it convenient
 *          for users to directly call it
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_CAN_H
#define __ADT6101P_CAN_H

/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "adt6101P_gpio.h"
#include "adt6101P_pmu.h"
#include "gpio_bsp.h"
#include "memscheduling.h"
#include <arm_math.h>

/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup CAN
 *  @{
 */

/** @addtogroup CAN_Private_Macros
  * @{
  */
/* Private define ------------------------------------------------------------*/
#define CAN_MAX_FILTER_NUM            16
#define CAN_RECEIVE_FIFO_COUNT        3             /* CAN receive fifo count */
#define CAN_TRANSMIT_FIFO_COUNT       3             /* CAN transmit fifo count */
#define CAN_BUF_LENGTH                (2048 / 4)    /* CAN buffer length */
#define MAX_CAN_WAIT_TIMES            100000        /* default wait times, or user define */
#define CAN_BAUDRATE_NUM              4

#define PIN_STB_N  			          (26)
#define PIN_CAN_EN 			          (25)

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
    IDLE = 0,
    ONGOING,
    LOST_ARBITRATION,
    TRANSMITTED,
    ABORTED,
    DISTURBED,
    REJECTED
}CanTxStatusTypedef;

typedef enum 
{
	STANDBY = 0,
	NORMAL,
	LISTEN_ONLY,
	SLEEP,
}CanTransTypedef;

typedef enum
{
    PIN00_H_PIN01_L = 0,
    PIN07_H_PIN08_L,
    PIN10_H_PIN11_L,
    PIN12_H_PIN13_L,
    PIN14_H_PIN15_L,
    PIN16_H_PIN17_L,
    PIN18_H_PIN19_L,
    PIN23_H_PIN24_L
}CanGpioResetTypedef;

typedef enum
{
    CAN_SEND_MSG_FILED = 0,
    CAN_SEND_MSG_SUCCS
}CanTxStsTypedef;

typedef enum 
{
    CAN_CLASSIC = 0,
    CAN_FD
}CanTypedef;

typedef enum
{
    CAN_MODE_INITIALIZATION,        /* Initialization mode */
    CAN_MODE_NORMAL,                /* Normal mode */
    CAN_MODE_RESTRICTED,            /* Restricted mode */
    CAN_MODE_MONITOR,               /* Listen only mode */
    CAN_MODE_LOOPBACK_INTERNAL,     /* Loopback internal mode */
    CAN_MODE_LOOPBACK_EXTERNAL      /* Loopback external mode */
}CanModeTypedef;

typedef enum
{
    TRANSMIT_PRIMARY   = 0,         /* Transmit primary mode */
    TRANSMIT_SECONDARY              /* Transmit secondary mode */
}TransmitTypedef;

typedef enum
{
    CAN_BAUDRATE_1M_2M   = 0,
    CAN_BAUDRATE_500K_1M,
    CAN_BAUDRATE_250K_1M,
    CAN_BAUDRATE_125K_1M,
    CAN_BAUDRATE_100K_1M
} CanBaudrateTypedef;

typedef volatile struct
{
    uint32 id:                    29; /* frame Identifier */
    uint32 :                      2;  /* Reserved */
    uint32 ttsen:                 1;  /* Transmit Time-Stamp enable */
    uint32 dlc:                   11; /* data length code */
    uint32 :                      5;  /* Reserved */
    uint32 ide:                   1;  /* Identifier extension */
    uint32 fdf:                   1;  /* can FD frame Format */
    uint32 brs:                   1;  /* CAN FD Bit Rate Switch Enable */
    uint32 xlf:                   1;  /* can XL frame Format */
    uint32 rmf:                   1;  /* remote Frame */
    uint32 sec:                   1;  /* Simple or Extended Contend+4 */
    uint32 :                      2;  /* Reserved */
    uint32 koer:                  3;  /* Kind of error */
    uint32 esi:                   1;  /* Error State Indicator */
    uint32 lbf:                   1;  /* Loop-Back Frame */
    uint32 :                      3;  /* Reserved */
    uint32 sdt:                   8;  /* Service Data Uint Type */
    uint32 vcid:                  8;  /* Virtual CAN Network ID */
    uint32 :                      8;  /* Reserved */
    uint32 handle:                8;  /* Handle for frame identifiation */
    uint32 af;                        /* Acceptance Field */
    uint32 rts1:                  32; /* Reception Time Stamp for CiA 603 time-stamping */
    uint32 rts2:                  32; /* Reception Time Stamp for CiA 603 time-stamping */
    uint32 cycle_time:            16; /* Cycle_time(time stamp for TTCAN) */
    uint32 :                      16; /* Reserved */
    uint32 data[CAN_BUF_LENGTH];      /* Buffer data */
}CanLLcStrTypedef;

#pragma anon_unions   // make REG and BIT structure is different but the address is the same
typedef volatile struct
{       
    union {
        struct {
        uint32 version:           16; /* Version of CAN */
        uint32 nodecfg:           8;  /* Can note configuration */
        uint32 tscfg:             8;  /* CiA 603 Time-Stamping configuration */
        uint32 ac_time;               /* Arbitration and Classic (Nominal) Bit Timing */
        uint32 fd_time;               /* CAN FD Bit Timing */
        uint32 xl_time;               /* CAN XL Bit Timing */
        uint32 presc:             8;  /* prescaler */
        uint32 fd_tdc:            8;  /*  */
        uint32 xl_tdc:            8;  /*  */
        uint32 relim:             8;  /* Retransmission and Re-arbitration Limit */
        uint32 intf;                  /* interrupt flag */
        uint32 inte;                  /* interrupt enable */
        uint32 tstat1:            16; /* transmit Status 1 */
        uint32 tstat2:            16; /* transmit Status 2 */
        uint32 tts0;                  /* transmission Time stamp */
        uint32 tts1;                  /* transmission Time stamp */
        uint32 cfg_stat:          8;  /* configuration and Status Register */
        uint32 tcmd:              8;  /* transmission command  */
        uint32 tctrl:             8;  /* transmission control  */
        uint32 rctrl:             8;  /* receive control */
        uint32 limit:             8;  /* warning limit */
        uint32 ealcap:            8;  /* error and arbitration lost capture */
        uint32 recnt:             8;  /* receive error count */
        uint32 tecnt:             8;  /* transmit error count */
        uint32 ref_msg;               /* reference message */
        uint32 trig_cfg:          16; /* trigger configuration */
        uint32 tbslot:            8;  /* TB slot pointer */
        uint32 ttcfg:             8;  /* time trigger configuration */
        uint32 tttrig:            16; /* trigger time */
        uint32 ttwtrig:           16; /* watch trigger */
        uint32 scfg:              8;  /* safety configuration */
        uint32 :                  16; /* Reserved */
        uint32 mem_stat:          8;  /* Memory Status */
        uint32 mem_es;                /* Memory Error Simulation */
        uint32 acfctrl:           8;  /* acceptance filter control */
        uint32 :                  8;  /* Reserved */
        uint32 acfen:            16;  /* acceptance filter enable */
        uint32 acfc0;     /* acceptance filter code 0 */
        uint32 acfc1;     /* acceptance filter code 1 */
        uint32 acfc2;     /* acceptance filter code 2 */
        uint32 acfc3;     /* acceptance filter code 3 */
        uint32 acfm0;     /* acceptance filter mask 0 */
        uint32 acfm1;     /* acceptance filter mask 1 */
        uint32 acfm2;     /* acceptance filter mask 2 */
        uint32 acfm3;     /* acceptance filter mask 3 */
        } REG;

        struct {
        uint32 version:           16;   /* Version of CAN */

        uint32 ces:                 1;  /* CAN Error Signaling */
        uint32 rop:                 1;  /* Restricted Operation */
        uint32 tmse:                1;  /* CAN XL transceiver mode switch enable */
        uint32 :                    5;  /* Reserved */

        uint32 tsen:                1;  /* time-stamping enable */
        uint32 tspos:               1;  /* time_stamping Position */
        uint32 :                    6;  /* Reserved */

        uint32 ac_seg1:             9;  /* bit timming segment 1 */
        uint32 :                    7;  /* Reserved */
        uint32 ac_seg2:             7;  /* bit timming segment 2 */
        uint32 :                    1;  /* Reserved */
        uint32 ac_sjw:              7;  /* synchronization jump width */
        uint32 :                    1;  /* Reserved */

        uint32 fd_seg1:             8;  /* bit timming segment 1 */
        uint32 :                    8;  /* Reserved */
        uint32 fd_seg2:             7;  /* bit timming segment 2 */
        uint32 :                    1;  /* Reserved */
        uint32 fd_sjw:              7;  /* synchronization jump width */
        uint32 :                    1;  /* Reserved */
        
        uint32 xl_seg1:             8;  /* bit timming segment 1 */
        uint32 :                    8;  /* Reserved */
        uint32 xl_seg2:             7;  /* bit timming segment 2 */
        uint32 :                    1;  /* Reserved */
        uint32 xl_sjw:              7;  /* synchronization jump width */
        uint32 :                    1;  /* Reserved */
        
        uint32 presc:               5;  /* prescaler */
        uint32 :                    3;  /* Reserved */

        uint32 fd_ssp_off:          8;

        uint32 xl_ssp_off:          8;

        uint32 realim:              3;  /* arbitration Limit */
        uint32 :                    1;  /* Reserved */
        uint32 retlim:              3;  /* Retransmission Limit */
        uint32 :                    1;  /* Reserved */
        
        uint32 aif:                 1;  /* abort interrupt flag */
        uint32 eif:                 1;  /* error interrupt flag */
        uint32 tsif:                1;  /* transmission secondary interrupt flag */
        uint32 tpif:                1;  /* transmission primary interrupt flag */
        uint32 rafif:               1;  /* RB almost full interrupt flag */
        uint32 rfif:                1;  /* RB full interrupt flag */
        uint32 roif:                1;  /* RB overflow interrupt flag */
        uint32 rif:                 1;  /* receive interrupt flag */
        uint32 beif:                1;  /* bus error interrupt flag */
        uint32 alif:                1;  /* arbitration lost interrupt flag */
        uint32 epif:                1;  /* error passive interrupt flag */
        uint32 ttif:                1;  /* time trigger error interrupt */
        uint32 teif:                1;  /* trigger error interrupt flag */
        uint32 wtif:                1;  /* watch trigger interrupt flag */
        uint32 mdwif:               1;  /* data warning interrupt flag */
        uint32 mdeif:               1;  /* data error interrupt flag */
        uint32 maeif:               1;  /* address error interrupt flag */
        uint32 seif:                1;  /* safety error interrupt flag */
        uint32 swif:                1;  /* safety warning interrupt flag */
        uint32 :                    11; /* Reserved */
        uint32 epass:               1;  /* error passive interrupt flag */
        uint32 ewarn:               1;  /* error warning interrupt flag */

        uint32 :                    1;  /* Reserved */
        uint32 eie:                 1;  /* error interrupt flag */
        uint32 tsie:                1;  /* transmission secondary interrupt flag */
        uint32 tpie:                1;  /* transmission primary interrupt flag */
        uint32 rafie:               1;  /* RB almost full interrupt flag */
        uint32 rfie:                1;  /* RB full interrupt flag */
        uint32 roie:                1;  /* RB overflow interrupt flag */
        uint32 rie:                 1;  /* receive interrupt flag */
        uint32 beie:                1;  /* bus error interrupt flag */
        uint32 alie:                1;  /* arbitration lost interrupt flag */
        uint32 epie:                1;  /* error passive interrupt flag */
        uint32 ttie:                1;  /* time trigger error interrupt */
        uint32 :                    1;  /* Reserved */
        uint32 wtie:                1;  /* watch trigger interrupt flag */
        uint32 mdwie:               1;  /* data warning interrupt flag */
        uint32 :                    3;  /* Reserved */
        uint32 swie:                1;  /* safety warning interrupt flag */
        uint32 :                    13; /* Reserved */

        uint32 handle1:             8;  /* handle for frame identification */
        uint32 tstat1_b:            3;  /* transmit Status */
        uint32 :                    5;  /* Reserved */
    
        uint32 handle2:             8;  /* handle for frame identification */
        uint32 tstat2_b:            3;  /* transmit Status */
        uint32 :                    5;  /* Reserved */

        uint32 tts0_b;                  /* transmission Time stamp */
        uint32 tts1_b;                  /* transmission Time stamp */

        uint32 busoff:              1;  /* buss off */
        uint32 :                    4;  /* Reserved */
        uint32 lbmi:                1;  /* loop back mode, internal */
        uint32 lbme:                1;  /* loop back mode, external */
        uint32 reset:               1;  /* reset request bit */

        uint32 tsa:                 1;  /* transmit secondary abort */
        uint32 tsall:               1;  /* transmit secondary all frame */
        uint32 tsone:               1;  /* transmit secondary one frame */
        uint32 tpa:                 1;  /* transmit primary abort */
        uint32 tpe:                 1;  /* transmit primary enable */
        uint32 stby:                1;  /* transceiver standby mode */
        uint32 lom:                 1;  /* listen only mode */
        uint32 tbsel:               1;  /* transmit buffer select */

        uint32 tsstat:              2;  /* transmission secondary status bit */
        uint32 tsff:                1;  /* transmission secondary buffer full flag */
        uint32 :                    1;  /* Reserved */
        uint32 tttbm:               1;  /* TTCAN transmit Buffer mode */
        uint32 tsmode:              1;  /* transmit buffer secondary operation mode */
        uint32 tsnext:              1;  /* transmit buffer secondary next */
        uint32 fd_iso:              1;  /* CAN FD ISO mode */

        uint32 rstat:               2;  /* receive buffer status */
        uint32 :                    1;  /* Reserved */
        uint32 rball:               1;  /* receive buffer stores all data frame */
        uint32 rrel:                1;  /* receive buffer release */
        uint32 rov:                 1;  /* receive overflow */
        uint32 rom:                 1;  /* receive overflow mode */
        uint32 sack:                1;  /* self-acknowledge */

        uint32 ewl:                 4;  /* programmable error warning limit */
        uint32 afwl:                4;  /* receive buffer almost full warning limit */
        
        uint32 alc:                 5;  /* arbitration lost capture */
        uint32 koer:                3;  /* kind of error */

        uint32 recnt_b:             8;  /* receive error count */
        uint32 tecnt_b:             8;  /* transmit error count */

        uint32 ref_msg:             29; /* reference message */
        uint32 :                    2;  /* Reserved */
        uint32 ref_ide:             1;  /* reference message ide bit */

        uint32 ttptr:               6;  /* transmit trigger TB slot pointer */
        uint32 :                    2;  /* Reserved */
        uint32 ttype:               3;  /* trigger type */
        uint32 :                    1;  /* Reserved */
        uint32 tew:                 4;  /* transmit enable window */

        uint32 tbptr:               6;  /* pointer to a TB frame slot */
        uint32 tbf:                 1;  /* set TB slot to "Filled" */
        uint32 tbe:                 1;  /* set TB slot to "Empty" */

        uint32 tten:                1;  /* time trigger enable */
        uint32 t_presc:             2;  /* TTCAN Time prescaler */
        uint32 :                    5;  /* Reserved */

        uint32 tttrig_b:            16; /* trigger time */
        uint32 ttwtrig_b:           16; /* watch trigger */

        uint32 xmren:               1;  /* DMR/TMR enable */
        uint32:                     3;  /* Reserved */
        uint32 fstim:               3;  /* fault stimulation */
        uint32 :                    1;  /* Reserved */

        uint32 :                    16; /* Reserved */

        uint32 acfa:                1;  /* acceptance Filter accept */
        uint32 txs:                 1;  /* transmission stop */
        uint32 txb:                 1;  /* transmission block */
        uint32 heloc:               2;  /* host side memory error location */
        uint32 :                    2;  /* Reserved */
        uint32 mpen:                1;  /* Memory protection enable */

        uint32 mebp1:               6;  /* memory error : bit position 1 */
        uint32 me1ee:               1;  /* memory error : 1st error enable */
        uint32 meaee:               1;  /* memory error : address error enable */
        uint32 mebp2:               6;  /* memory error : bit position 2 */
        uint32 me2ee:               1;  /* memory error : 2nd error enabl */
        uint32 :                    1;  /* Reserved */
        uint32 meeec:               4;  /* memory error : error enable counter */
        uint32 menec:               4;  /* memory error : no error counter */
        uint32 mel:                 2;  /* memory error location */
        uint32 mes:                 1;  /* memory error side */
        uint32 :                    5;  /* Reserved */

        uint32 acfadr:              4;  /* acceptance filter address */
        uint32 :                    4;  /* Reserved */

        uint32 :                    8;  /* Reserved */
        uint32 acfen_b:             16; /* acceptance filter enable */
        uint32 acfc0_b;                 /* acceptance filter code 0 */
        uint32 acfc1_b;                 /* acceptance filter code 1 */
        uint32 acfc2_b;                 /* acceptance filter code 2 */
        uint32 acfc3_b;                 /* acceptance filter code 3 */
        uint32 acfm0_b;                 /* acceptance filter mask 0 */
        uint32 acfm1_b;                 /* acceptance filter mask 1 */
        uint32 acfm2_b;                 /* acceptance filter mask 2 */
        uint32 acfm3_b;                 /* acceptance filter mask 3 */
        } BIT;
    };
    uint32 test_address1;
    uint32 RESERVED1[1];
    CanLLcStrTypedef  rbuf;             /* receive buffer */
    uint32 test_address;
    CanLLcStrTypedef  tbuf;             /* transmit buffer */
    uint32 pwm_cfg;                     /* pwm encoding configuration */
}CanStrTypeDef;

typedef struct
{
    uint8 index:                    8;  /* filer index */
    uint8 enable:                   1;  /* enable filer inder */

    uint32 code_id:                 29; /* frame Identifier */
    uint32 code_dlc:                16; /* data length code */
    uint32 code_ide:                1;  /* Identifier extension */
    uint32 code_fdf:                1;  /* can FD frame Format */
    uint32 code_brs:                1;  /* CAN FD Bit Rate Switch Enable */
    uint32 code_xlf:                1;  /* can XL frame Format */
    uint32 code_rmf:                1;  /* remote Frame */
    uint32 code_sec:                1;  /* Simple or Extended Contend+4 */
    uint32 code_esi:                1;  /* Error State Indicator */
    uint32 code_lbf:                1;  /* Loop-Back Frame */
    uint32 code_sdt:                8;  /* Service Data Uint Type */
    uint32 code_vcid:               8;  /* Virtual CAN Network ID */
    uint32 code_af;                     /* Acceptance Field */
    
    uint32 mask_id:                 29; /* frame Identifier */
    uint32 mask_dlc:                16; /* data length code */
    uint32 mask_ide:                1;  /* Identifier extension */
    uint32 mask_fdf:                1;  /* can FD frame Format */
    uint32 mask_brs:                1;  /* CAN FD Bit Rate Switch Enable */
    uint32 mask_xlf:                1;  /* can XL frame Format */
    uint32 mask_rmf:                1;  /* remote Frame */
    uint32 mask_sec:                1;  /* Simple or Extended Contend+4 */
    uint32 mask_esi:                1;  /* Error State Indicator */
    uint32 mask_lbf:                1;  /* Loop-Back Frame */
    uint32 mask_sdt:                8;  /* Service Data Uint Type */
    uint32 mask_vcid:               8;  /* Virtual CAN Network ID */
    uint32 mask_af;                     /* Acceptance Fiel */
}CanFilterCtrlStrTypeDef;

typedef struct
{
    uint8 canMode;                                          /* more details see CanModeTypedef */
    uint8 rom;                                              /* receive overflow mode: 
                                                             * 0-the new frame will not be stored
                                                             * 1-the oldest frame will be overwrite */
    CanFilterCtrlStrTypeDef filterList[CAN_MAX_FILTER_NUM]; /* CAN Filter*/
}CanCfgStrTypedef;




typedef struct _Can_Msg_Info
{
    uint32 id;
    uint32 dlc;
    uint8  ide;
    uint8  fdf;
    uint8  brs;
    uint8  xlf;
    uint8  rmf;
    uint8  sec;
    uint8  koer;
    uint8  esi;
    uint8  lbf;
    uint8  sdt;
    uint8  vcid;
    uint8  handle;
    uint32 af;
    uint8  data[64];
}CanMsgInfoStrTypedef;

typedef struct 
{
    uint32 id;
    uint32 dlc;
    uint8  ide;
    uint8  fdf;
    uint8  xlf;
    uint8  rmf;
    uint8  data[2048];
    uint16 timestamp;
} CanMsgInfoUserStrTypedef;

/* Bautrate = HIGH_CLOCK / ((sPresc+1)*(sSeg1+2+sSeg2+1))
    The max baudrate is 1Mbit/s from can2.0B defines data */
typedef struct
{
    uint8 sPresc;
    uint8 ac_sSeg1;
    uint8 ac_sSeg2;
    uint8 ac_sSjw;
    uint8 fd_sSeg1;
    uint8 fd_sSeg2;
    uint8 fd_sSjw;
    uint8 xlsSeg1;
    uint8 xlsSeg2;
    uint8 xlsSjw;
    uint8 pwmo;
    uint8 pwms;
    uint8 pwml;
}CanBaudrateCfgStrTypedef;
/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/
/** @addtogroup CAN_Private_Functions
  * @{
  */
void canReset(uint8 reset);

void canCfgPad(CanGpioResetTypedef num);

int32 canStandby(uint8 enable);

int32 canMsgIsReceive(void);

int32 canStartTransmission(TransmitTypedef type, uint8 mode);

int32 canIsTransmitIdle(TransmitTypedef type);

int32 canIsTransmitBusy(TransmitTypedef type);

int32 canIsMsgInReceiveBuf(void);

int32 waitTransmitDone(void);

int32 canWaitTransmissionIdle(TransmitTypedef type);

int32 CAN_SetFilter(CanFilterCtrlStrTypeDef *CanFilterCfg);

void canInitialize(CanCfgStrTypedef *config, const CanBaudrateCfgStrTypedef *baudrate, CanTypedef canTpye);

int32 canUninitialize(void);

uint32 readBusOff(void);

int32 canIsMsgInTransmissionBuf(void);

int32 canMsgIsReceive(void);

void canSetMsgInfo(const CanMsgInfoStrTypedef *info, TransmitTypedef type, uint32 data_len);

int32 canMessageSend(const CanMsgInfoStrTypedef *info, TransmitTypedef type, uint32 data_len);

void canMessageRead(CanMsgInfoStrTypedef *info);

const CanBaudrateCfgStrTypedef *getCommonBaudrate(CanBaudrateTypedef type);

int32 userCopyData(CanMsgInfoUserStrTypedef *pUserCanMsgBuf, CanMsgInfoStrTypedef *recvinfo);

void setCanTransMode(CanTransTypedef mode);

#ifdef TTCAN
void ttcanTimePresc(uint8 presc);

void ttcanEnable(uint8 enable);

void ttcanSetPresc(uint8 presc);

void ttcanSetBufMode(uint8 mode);

void ttcanEnableTimeInt(uint8 enable);

void ttcanEnableWatchInt(uint8 enable);

void ttcanSetTriggerConfig(uint8 type, uint8 tew, uint8 ttptr, uint16 time);

uint8 ttcanGetTriggerType(void);

void ttcanSetWatchTrigger(uint16 time);

void ttcanSetTbptr(uint8 pointer);

void ttcanSetTb(uint8 state);

void ttcanSetRefMsg(uint32 refId, uint8 refIde);

void ttcanSetSamplePos(uint8 position);

#endif // TTCAN


/**
  * @}
  */
/**
  * @}
  */
/**
  * @}
  */

#endif /* __CAN_BSP_H */
