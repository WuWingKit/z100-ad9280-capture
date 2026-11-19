/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_can.c
 * @original_author  Dingzhen(zding@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-03-11
 * @brief   This file references the can HAL library and combines it into a
 *          universal interface function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_can.h"
#include "adt6101P_type_define.h"
#include "gpio_ctype_map.h"

#pragma GCC push_options
#pragma GCC optimize("O0")
/*----------------------------- variable -------------------------------------*/
CanMsgInfoStrTypedef        g_sendCanMsgInfo;
 

#if FRAME_LOWPOWER //just of human_detection_chuhang, other project set HIGH_CLOCK

///< Baurate sample setting list: Source clock = 50MHz, sample point = 70%, or user define
static const CanBaudrateCfgStrTypedef s_commonBaudrate[CAN_BAUDRATE_NUM] =
{
    {0x00, 0x21, 0x0E, 0x03,   18,    4, 0x03,  18,    4, 0x03},  //AC-FD-XL,1MHz-2MHz-2MHz
    {0x01, 0x21, 0x0E, 0x03, 0x21, 0x0E, 0x03,  18,    4, 0x03},  //AC-FD-XL,500K-2MHz-2MHz
    {0x03, 0x21, 0x0E, 0x03,   18,    4, 0x03,  18,    4, 0x03},  //AC-FD-XL,250K-2MHz-2MHz
    {0x07, 0x21, 0x0E, 0x03,   18,    4, 0x03,  18,    4, 0x03},  //AC-FD-XL,125K-2MHz-2MHz
};
#else
///< Baurate sample setting list: Source clock = 100MHz, sample point = 70%, or user define
static const CanBaudrateCfgStrTypedef s_commonBaudrate[CAN_BAUDRATE_NUM] =
{
    {0x00, 0x45, 0x1C, 0x03,   18,    4, 0x03,  18,    4, 0x03},  //AC-FD-XL,1MHz-2MHz-2MHz
    {0x01, 0x45, 0x1C, 0x03,   18,    4, 0x03,  18,    4, 0x03},  //AC-FD-XL,500K-2MHz-2MHz
    {0x03, 0x45, 0x1C, 0x03,   18,    4, 0x03,  18,    4, 0x03},  //AC-FD-XL,250K-2MHz-2MHz
    {0x07, 0x45, 0x1C, 0x03,   18,    4, 0x03,  18,    4, 0x03},  //AC-FD-XL,125K-2MHz-2MHz
};
#endif

/*----------------------------- function -------------------------------------*/
static void canSetBitrate(const CanBaudrateCfgStrTypedef *config, CanTypedef canType);


/**
 * @brief can io config
 * @param can_mux :choose which pair of pads to use
 *                0: gpio00, gpio01
 *                1: gpio07, gpio08
 *                2: gpio10, gpio11
 *                3: gpio12, gpio13
 *                4: gpio14, gpio15
 *                5: gpio16, gpio17
 *                6: gpio18, gpio19
 *                7: gpio23, gpio24
 * @return None
 * @note
 */
void canCfgPad(CanGpioResetTypedef num)
{
    switch(num)
    {
        case PIN00_H_PIN01_L:
        {
            setGpioConfigPid(GPIO_CFG_P0, PID_CAN0_TX);
            setGpioConfigPid(GPIO_CFG_P1, PID_CAN0_RX);
            break;
        }
        case PIN07_H_PIN08_L:
        {
            setGpioConfigPid(GPIO_CFG_P7, PID_CAN0_TX);
            setGpioConfigPid(GPIO_CFG_P8, PID_CAN0_RX);
            break;
        }
        case PIN10_H_PIN11_L:
        {
            setGpioConfigPid(GPIO_CFG_P10, PID_CAN0_TX);
            setGpioConfigPid(GPIO_CFG_P11, PID_CAN0_RX);
            break;
        }
        case PIN12_H_PIN13_L:
        {
            setGpioConfigPid(GPIO_CFG_P12, PID_CAN0_TX);
            setGpioConfigPid(GPIO_CFG_P13, PID_CAN0_RX);
            break;
        }
        case PIN14_H_PIN15_L:
        {
            setGpioConfigPid(GPIO_CFG_P14, PID_CAN0_TX);
            setGpioConfigPid(GPIO_CFG_P15, PID_CAN0_RX);
            break;
        }
        case PIN16_H_PIN17_L:
        {
            setGpioConfigPid(GPIO_CFG_P16, PID_CAN0_TX);
            setGpioConfigPid(GPIO_CFG_P17, PID_CAN0_RX);
            break;
        }
        case PIN18_H_PIN19_L:
        {
            setGpioConfigPid(GPIO_CFG_P18, PID_CAN0_TX);
            setGpioConfigPid(GPIO_CFG_P19, PID_CAN0_RX);
            break;
        }
        case PIN23_H_PIN24_L:
        {
            setGpioConfigPid(GPIO_CFG_P23, PID_CAN0_TX);
            setGpioConfigPid(GPIO_CFG_P24, PID_CAN0_RX);
            break;
        }
    }
    //enable CAN's clock and release reset. 
    setSoftwareReset(ENABLE, CAN0_HOST_RST);
    setSoftwareReset(ENABLE, CAN0_CAN_RST);
    setSoftwareReset(ENABLE, CAN0_TIMER_RST);
    
    setClkGateEn(ENABLE, CAN0_HOST_CLK_EN);
    setClkGateEn(ENABLE, CAN0_CAN_CLK_EN);
    setClkGateEn(ENABLE, CAN0_TIMER_CLK_EN);
}


/**
 * @brief  Wait for transmission to complete.
 * @return Waiting status.
 *          1- Transfer complete
 */
int32 waitTransmitDone(void)
{
    // i don't know why tstat1 and tstat2 isn't working at fpga platform.
    /*
    uint32 tStatus1, tStatus2;
    do
    {
        tStatus1 = CAN->BIT.tstat1_b;
        tStatus2 = CAN->BIT.tstat2_b;
    }while(1 != ((tStatus1==IDLE)&(tStatus2==TRANSMITTED)));
    */

    uint32 i=0;
    do
    {
        if (CAN->BIT.tpif)
        {
            CAN->BIT.tpif = 1;
            break;
        }
    } while (i++ < MAX_CAN_WAIT_TIMES);

    return 1;
}


/**
 * @brief  Wait for transmission to be idle.
 * @param  type
 *          TRANSMIT_PRIMARY- Transmit promary mode
 *          TRANSMIT_SECONDARY- Transmit secondary mode
 * @return Waiting Status.
 *          1- Already to be idle
 */
int32 canWaitTransmissionIdle(TransmitTypedef type)
{
    uint32 i = 0;
    int32 ret = 0;

    if (TRANSMIT_PRIMARY == type)
    {
        do
        {
            if (CAN->BIT.tpe == 0)    // Transmission is successful
            {
                break;
            }
        } while (i++ < MAX_CAN_WAIT_TIMES);
    }
    else
    {
        do
        {
            if (CAN->BIT.tsstat != 3)    // Transmission buffers are not full
            {
                break;
            }
        } while (i++ < MAX_CAN_WAIT_TIMES);
    }

    if (i >= MAX_CAN_WAIT_TIMES)
    {
        //DEBUGMSG(DEBUG_ZONE_ERROR, ("[CAN%d]WaitTransmissionIdle timeout\r\n", GetCanIndex(CANx)));
        ret = 1;
    }

    return ret;
}


/**
 * @brief  Set CAN filters
 * @param   index    Set the number of filters, no more than 15.
 * @param   enable
 *              1- Enabled
 *              0- Disabled
 * @param   LLC frame.
 * @return  Get filter setting status.
 *          0- Successfully
 *          1- Failed
 */
int32 CAN_SetFilter(CanFilterCtrlStrTypeDef *CanFilterCfg)
{
    int32 ret = 0;  
    if (CanFilterCfg->enable)
    {
        CAN->REG.acfctrl  = CanFilterCfg->index;
        if(CanFilterCfg->code_fdf)
        {
            if(CanFilterCfg->code_ide)
            {
                CAN->REG.acfm0 = 0xffffffff & ~(CanFilterCfg->mask_id);//0xe0000000
                CAN->REG.acfc0 = CanFilterCfg->code_id;
            }
            else{
                CAN->REG.acfm0 = (0xffffffff & ~(CanFilterCfg->mask_id << 18));//0xe003ffff
                CAN->REG.acfc0 = CanFilterCfg->code_id << 18;
            }
        }
        else
        {   
            if(CanFilterCfg->code_ide){
                CAN->REG.acfm0 = 0xffffffff & ~(CanFilterCfg->mask_id);
                CAN->REG.acfc0 = CanFilterCfg->code_id;
            }else
            {
                CAN->REG.acfm0 = (0xffffffff & ~(CanFilterCfg->mask_id << 18));//CAN class modefy this
                CAN->REG.acfc0 = CanFilterCfg->code_id << 18;
            }
        }
            CAN->REG.acfm1 = 0xffffffff;
            CAN->REG.acfm2 = 0xffffffff;
            CAN->REG.acfm3 = 0xffffffff;
            CAN->REG.acfc1 = 0;
            CAN->REG.acfc2 = 0;
            CAN->REG.acfc3 = 0;

            CAN->REG.acfen |= (1 << CanFilterCfg->index); //don't work 
    }
    else
    {
        CAN->BIT.acfen_b &= ~(uint32)(1 << CanFilterCfg->index);  //don't work
    }

    CAN->BIT.afwl = CAN_RECEIVE_FIFO_COUNT - 1;
    
    return ret;
}



/**
 * @brief   Can set baud rate.
 * @param   config    Can configuration.
 * @param   canType
 *              CAN_CLASSIC
 *              CAN_FD
 */
static void canSetBitrate(const CanBaudrateCfgStrTypedef *config, CanTypedef canType)
{
    CAN->BIT.reset = 1;
    if (CAN_CLASSIC == canType)
    {
      CAN->BIT.ac_seg1 = config->ac_sSeg1;
      CAN->BIT.ac_seg2 = config->ac_sSeg2;
      CAN->BIT.ac_sjw  = config->ac_sSjw;
    }
    else if(CAN_FD == canType)
    {
      CAN->BIT.ac_seg1 = config->ac_sSeg1;
      CAN->BIT.ac_seg2 = config->ac_sSeg2;
      CAN->BIT.ac_sjw  = config->ac_sSjw;
  
      CAN->BIT.fd_seg1 = config->fd_sSeg1;
      CAN->BIT.fd_seg2 = config->fd_sSeg2;
      CAN->BIT.fd_sjw  = config->fd_sSjw;
    }
    CAN->BIT.presc = config->sPresc;
}


/**
 * @brief   Get baud rate configuration.
 * @param   type
 *              CAN_BAUDRATE_1M_2M,
 *              CAN_BAUDRATE_500K_1M,
 *              CAN_BAUDRATE_250K_1M,
 *              CAN_BAUDRATE_125K_1M,
 *              CAN_BAUDRATE_100K_1M
 * @return  config: Baud rate configuration.
 */
const CanBaudrateCfgStrTypedef *getCommonBaudrate(CanBaudrateTypedef type)
{
    const CanBaudrateCfgStrTypedef *config = 0;

    if (type < CAN_BAUDRATE_NUM)
    {
        config = &s_commonBaudrate[type];
    }
    
    return config;
}


/**
 * @brief   Wait for transmission to be idle.
 * @param   config  Can configuration structure type.
 * @param   baudrate Can baudrate configuration structure type.
 * @param   canTpye
 *              CAN_CLASSIC- 
 *              CAN_FD- 
 * @return  None.
 */
void canInitialize(CanCfgStrTypedef *config, const CanBaudrateCfgStrTypedef *baudrate, CanTypedef canTpye)
{
    if(config != NULL)
    {
        //set baudrate
        canSetBitrate(baudrate, canTpye);

        //set fileter(user define)
        for (int i = 0; i < CAN_MAX_FILTER_NUM; i++)
        {
            CAN_SetFilter(&config->filterList[i]);
        }


        CAN->BIT.reset = 0;
        CAN->BIT.afwl = CAN_RECEIVE_FIFO_COUNT - 1;

        // Set work mode
        if (config->canMode == (uint32)CAN_MODE_MONITOR)
        {
            CAN->BIT.lom = 1;
        }
        else if (config->canMode == (uint32)CAN_MODE_LOOPBACK_INTERNAL)
        {
            CAN->BIT.lbmi = 1;
        }
        else if (config->canMode == (uint32)CAN_MODE_LOOPBACK_EXTERNAL)
        {
            CAN->BIT.lbme = 1;
        }
        else
        {
            
        }
        //CAN->REG.inte   = 1<<3;
        CAN->BIT.tsmode = 1;
        CAN->BIT.rom    = config->rom;
        CAN->BIT.rrel   = 1;    
        CAN->BIT.retlim = 0; //retlim(2:0) :000-1 attempt(no retranmsission);111-unlimited
        CAN->BIT.realim = 0; //realim(2:0) :000-1 attempt(no re-arbitration);111-unlimited

        CAN->BIT.tpie   = 1; //Transmission Primary Interrupt Enable  0 – Disabled, 1 – Enabled
        CAN->BIT.tsie   = 1; //transmission second interrupt enable.
        CAN->BIT.beie   = 1; //bus off interrupt enable.
        CAN->BIT.eie    = 1; //error interrupt enable.
        CAN->BIT.rie    = 1; //receive interrupt enable.

        #ifdef TTCAN
        // enable ttcan mode
        ttcanEnable(1); 
        #endif

    }
}


/**
 * @brief   System reset.
 * @param   None.
 * @return  1- Successfully.
 */
int32 canUninitialize(void)
{
    CAN->BIT.acfen_b = 0;
    CAN->BIT.reset = 1;
    CAN->BIT.acfen_b = 0;
    return 1;
}

/**
 * @brief   read bus off state.
 * @param   None.
 * @return  0-bus on; 1-bus off
 */
uint32 readBusOff(void)
{
    return ( CAN->BIT.eif ||  CAN->BIT.beif);
}

/**
 * @brief  Check whether message are transmission.
 * @return Receive buffer.
 *          0- no transmission.
 *          1- transmission has been successfully completed.
 */
int32 canIsMsgInTransmissionBuf(void)
{
    return (CAN->BIT.tpif)||(CAN->BIT.tsif);
}

/**
 * @brief  Check whether message are recived.
 * @return Receive buffer.
 *          0- no recived.
 *          1- received has been successfully completed.
 */
int32 canMsgIsReceive(void)
{
    return CAN->BIT.rif;
}

/**
 * @brief   Set the message information.
 * @param   info    Can message information structure type.
 * @param   type
 *              TRANSMIT_PRIMARY- Transmit promary mode
 *              TRANSMIT_SECONDARY- Transmit secondary mode
 * @param   data_len:   Length of data.
 */
void canSetMsgInfo(const CanMsgInfoStrTypedef *info, TransmitTypedef type,  uint32 data_len)
{

    if (TRANSMIT_PRIMARY == type)
    {
        CAN->BIT.tbsel = 0;
    }
    else
    {
        CAN->BIT.tbsel = 1;
    }


    
    if(info->ide)
    {
        CAN->tbuf.id = info->id;  // info->IDE ? info->ID : (info->ID & 0x7FF);
    }
    else
    {
        CAN->tbuf.id = info->id << 18;
    }

    if ((data_len > 8) & (data_len <= 12))
    {
        CAN->tbuf.dlc = 9;
    }
    else if((data_len > 12) & (data_len <= 16))
    {
        CAN->tbuf.dlc = 10;
    }
    else if((data_len > 16) & (data_len <= 20))
    {
        CAN->tbuf.dlc = 11;
    }
    else if((data_len > 20) & (data_len <= 24))
    {
        CAN->tbuf.dlc = 12;
    }
    else if((data_len > 24) & (data_len <= 32))
    {
        CAN->tbuf.dlc = 13;
    }
    else if((data_len > 32) & (data_len <= 48))
    {
        CAN->tbuf.dlc = 14;
    }
    else if((data_len > 48) & (data_len <= 64))
    {
        CAN->tbuf.dlc = 15;
    }
    else if(data_len > 64)
    {
        CAN->tbuf.dlc = 16;
    }
    else
    {
        CAN->tbuf.dlc = data_len;
    }

    CAN->tbuf.ide    = info->ide;
    CAN->tbuf.fdf    = info->fdf;
    CAN->tbuf.brs    = info->brs;
    CAN->tbuf.xlf    = info->xlf;
    CAN->tbuf.rmf    = info->rmf;
    CAN->tbuf.sec    = info->sec;
    CAN->tbuf.koer   = info->koer;
    CAN->tbuf.esi    = info->esi;
    CAN->tbuf.lbf    = info->lbf;
    CAN->tbuf.sdt    = info->sdt;
    CAN->tbuf.vcid   = info->vcid;
    CAN->tbuf.handle = info->handle;
    CAN->tbuf.af     = info->af;

    if ((info->rmf == 0) && (info->dlc))
    {
        for(int i = 0; i < (data_len/4); i++)
        {
          CAN->tbuf.data[i] = *(uint32 *)(&info->data[i*4]);
        }
    }
    if (TRANSMIT_SECONDARY == type)
    {
        CAN->BIT.tsnext = 1;
    }
}


/**
 * @brief   Can sends messages.
 * @param   info    Can message information structure type.
 * @param   type
 *              TRANSMIT_PRIMARY- Transmit promary mode
 *              TRANSMIT_SECONDARY- Transmit secondary mode
 * @param   data_len:   Length of data.
 * @return  0- Transfer incomplete.
 *          -1- Timeout.
 *          -2- Start transfer failed.
 *          -3- Transfer complete.
 *          
 */
int32 canMessageSend(const CanMsgInfoStrTypedef *info, TransmitTypedef type, uint32 data_len)
{
    int32 ret = 0;

    if (!canWaitTransmissionIdle(type))//!canWaitTransmissionIdle(type)
    {
        canSetMsgInfo(info, type, data_len);

        if (canStartTransmission(type, 1))
        {
            ret = -2;
        }
        else
        {
            if (waitTransmitDone())
            {
                ret = -3;
            }
        }
    }
    else  // If timeout, return error or abort transmission
    {
        ret = -1;
    }

    return ret;
}


/**
 * @brief   Can read messages.
 * @param   info    Can message information structure type.
 */
void canMessageRead(CanMsgInfoStrTypedef *info)
{
    volatile int32 ret = 0;
    uint32 recvDataLen;
    if (!canIsMsgInReceiveBuf())// if DEBUG set canIsMsgInReceiveBuf()
    {
        ret = 1;
    }
    else
    {
        info->id     = CAN->rbuf.id;
        info->dlc    = CAN->rbuf.dlc;
        info->ide    = CAN->rbuf.ide;
        info->fdf    = CAN->rbuf.fdf;
        info->brs    = CAN->rbuf.brs;
        info->xlf    = CAN->rbuf.xlf;
        info->rmf    = CAN->rbuf.rmf;
        info->sec    = CAN->rbuf.sec;
        info->koer   = CAN->rbuf.koer;
        info->esi    = CAN->rbuf.esi;
        info->lbf    = CAN->rbuf.lbf;   
        info->sdt    = CAN->rbuf.sdt;
        info->vcid   = CAN->rbuf.vcid;
        info->handle = CAN->rbuf.handle;
        info->af     = CAN->rbuf.af;
#ifdef TTCAN
        //info->CYCLE_TIME = CANx->RBUF.CYCLE_TIME;
#endif

    switch (info->dlc)
    {
    case 9:
        recvDataLen = 12;
        break;
    case 10:
        recvDataLen = 16;
        break;
    case 11:
        recvDataLen = 20;
        break;
    case 12:
        recvDataLen = 24;
        break;
    case 13:
        recvDataLen = 32;
        break;
    case 14:
        recvDataLen = 48;
        break;
    case 15:
        recvDataLen = 64;
        break;               
    default:
        recvDataLen = 8;
        break;
    }
        
        if ((info->rmf == 0) && (info->dlc))
        {
            for(int i = 0; i < (recvDataLen / 4); i++)
            {
                *(uint32 *)(&info->data[i*4]) = CAN->rbuf.data[i];
            }
        }
        CAN->BIT.rrel = 1;
    }
}


/**
 * @brief  Reset CAN.
 * @param  reset CAN reset value.
 *          1-  Reset
 *          0-  No local reset
 */
void canReset(uint8 reset)
{
    CAN->BIT.reset = reset;
}


/**
 * @brief  Using CAN standby mode or using other GPIO.
 * @param   enable
 *              1- Enabled
 *              0- Disabled
 * @return  error code
 *              1- standby error
 *              0- no error
 * @note    If there is any transmission, can not entry standby mode.
 */
int32 canStandby(uint8 enable)
{
    int32 err = 0;

    // Using CAN standby mode or using other GPIO
    if (enable)
    {
        // If there is any transmission, can not entry standby mode
        if (CAN->BIT.tpe != 1 && CAN->BIT.tsone != 1 && CAN->BIT.tsall != 1)
        {
            CAN->BIT.stby = 1;
        }
        else
        {
            err = 1;
        }
    }
    else
    {
        CAN->BIT.stby = 0;
    }

    return err;
}


/**
 * @brief  Can start transmission.
 * @param  type
 *          TRANSMIT_PRIMARY- Transmit promary mode
 *          TRANSMIT_SECONDARY- Transmit secondary mode
 * @param  mode
 *          1- Send one message only
 *          0- Send all messages
 * @return error code
 *          0- Transfer started successfully
 *          1- Start transfer failed
 * @note    Can't send message when CAN is in standby mode(return or delay).
 */
int32 canStartTransmission(TransmitTypedef type, uint8 mode)
{
    // Can't send message when CAN is in standby mode(return or delay)
    if (CAN->BIT.stby)
    {
        return 1;
    }

    if (TRANSMIT_PRIMARY == type)
    {
        CAN->BIT.tpe = 1;
    }
    else
    {
        if (mode)
        {
            CAN->BIT.tsone = 1;    // Send one message only
        }
        else
        {
            CAN->BIT.tsall = 1;    // Send all messages
        }
    }

    return 0;
}


/**
 * @brief  Check whether can transmission is idle.
 * @param  type
 *          TRANSMIT_PRIMARY- Transmit promary mode
 *          TRANSMIT_SECONDARY- Transmit secondary mode
 * @return Detection flag bit.
 *          1- PTB or STB is empty
 *          0- PTB or STB is not empty
 */
int32 canIsTransmitIdle(TransmitTypedef type)
{
    if (TRANSMIT_PRIMARY == type)
    {
        return (CAN->BIT.tpe == 0);     // PTB is empty
    }
    else
    {
        return (CAN->BIT.tsstat == 0);  // STB is empty
    }
}


/**
 * @brief  Check whether can transmission is busy.
 * @param  type
 *          TRANSMIT_PRIMARY- Transmit promary mode
 *          TRANSMIT_SECONDARY- Transmit secondary mode
 * @return Detection flag bit.
 *          1- Transmitting is not successful or Transmission buffers are full
 */
int32 canIsTransmitBusy(TransmitTypedef type)
{
    if (TRANSMIT_PRIMARY == type)
    {
        return (CAN->BIT.tpe == 1);    // Transmitting is not successful
    }
    else
    {
        return (CAN->BIT.tsstat == 3);    // Transmission buffers are full
    }
}


/**
 * @brief  Check whether message are receive.
 * @return Receive buffer.
 *          0- empty
 *          1- > empty and < almost full
 *          2- >= almost full but not full and no overflow
 *          3- full
 */
int32 canIsMsgInReceiveBuf(void)
{
    return CAN->BIT.rstat;
}


/**
 * @brief  copy and handle the receive CAN message.
 * @param  pUserCanMsgBuf: after processing receive message  
 * @param  recvinfo:receive message
 * @return Receive buffer.
 */
int32 userCopyData(CanMsgInfoUserStrTypedef *pUserCanMsgBuf, CanMsgInfoStrTypedef *recvinfo)
{
    uint32 recvDataLen;
    if(recvinfo->fdf) //get frame id
    {
        if(recvinfo->ide) //CAN FD
        {
            pUserCanMsgBuf->id = recvinfo->id;  //FEFF ID (28:0)
        }else
        {
            pUserCanMsgBuf->id = (recvinfo->id >> 18); //FBFF ID (28:18)
        }
    }else if(recvinfo->xlf)
    {
        pUserCanMsgBuf->id = (recvinfo->id >> 18); //CAN XL  XLFF ID(28:18)
    }else
    {
        if(recvinfo->ide) //CAN class
        {
            pUserCanMsgBuf->id = recvinfo->id; //CEFF ID (28:0)

        }else{
             pUserCanMsgBuf->id = (recvinfo->id >> 18); //CBFF ID (28:18)
        }
    }
    switch (recvinfo->dlc)
    {
    case 9:
        recvDataLen = 12;
        break;
    case 10:
        recvDataLen = 16;
        break;
    case 11:
        recvDataLen = 20;
        break;
    case 12:
        recvDataLen = 24;
        break;
    case 13:
        recvDataLen = 32;
        break;
    case 14:
        recvDataLen = 48;
        break;
    case 15:
        recvDataLen = 64;
        break;               
    default:
        recvDataLen = 8;
        break;
    }
    pUserCanMsgBuf->dlc = recvinfo->dlc;
    pUserCanMsgBuf->ide = recvinfo->ide;
    pUserCanMsgBuf->fdf = recvinfo->fdf;
    pUserCanMsgBuf->xlf = recvinfo->xlf;

    memcpy(pUserCanMsgBuf->data, recvinfo->data, recvDataLen);

    return 0;
}

/**
 * @brief  Check whether message are recived.
 * @return Receive buffer.
 *          0- no recived.
 *          1- received has been successfully completed.
 */
//int32 canMsgIsReceive(void)
//{
//    return CAN->BIT.rif;
//}

/**
 * @brief  set CAN transceiver mode
 * @param  transceiver mode 
 * @return void.
 */
void setCanTransMode(CanTransTypedef mode)
{
    switch (mode)
    {
    case STANDBY:
        gpioWritePin(PIN_CAN_EN, 0);
        gpioWritePin(PIN_STB_N, 0);
        break;
    case NORMAL:
		gpioWritePin(PIN_CAN_EN, 1);
		gpioWritePin(PIN_STB_N, 1);
		break;
	case LISTEN_ONLY:
		gpioWritePin(PIN_CAN_EN, 0);
		gpioWritePin(PIN_STB_N, 1);
		break;
	case SLEEP:
		gpioWritePin(PIN_CAN_EN, 1);
        gpioWritePin(PIN_STB_N, 0);
		break;
    default:
        //standby mode
        gpioWritePin(PIN_CAN_EN, 0);
		gpioWritePin(PIN_STB_N, 0);
        break;
    }
}

#pragma GCC pop_options

#ifdef TTCAN
/**
 * @brief   Can time prescale.
 * @param   presc   Select CAN clock, base on clk.
 *                  0- clk = hclk
 *                  1- clk = hclk/2
 *                  2- clk = hclk/4
 *                  3- clk = hclk/8
 */
void ttcanTimePresc(uint8 presc)
{
    // you can change the time presc
    // 0 :50M than each count of TTS is 20ns
    PMU->pmu_clk_high_reg = presc << 22;
}


/**
 * @brief   TTEN enable.
 * @param   enable
 *          0- Disable
 *          1- TTCAN enable,timer is running.
 */
void ttcanEnable(uint8 enable)
{
    CAN->BIT.tten = enable;
}


/**
 * @brief   Can set prescale.
 * @param   presc Time Trigger PRESCaler.
 *          0- 1
 *          1- 2
 *          2- 4
 *          3- 8
 */
void ttcanSetPresc(uint8 presc)
{
    CAN->BIT.t_presc = presc;
}


/**
 * @brief   TTCAN transmit buffer mode.
 * @param   mode
 *              0- Separate PTB and STB, behavior defined by TSMODE
 *              1- Full TTCAN support
 */
void ttcanSetBufMode(uint8 mode)
{
    CAN->BIT.tttbm = mode;
}


/**
 * @brief   Time trigger interrupt.
 * @param   enable
 *              1- Time trigger interrupt enable
 */
void ttcanEnableTimeInt(uint8 enable)
{
    CAN->BIT.ttie = enable;
}


/**
 * @brief   Watch trigger interrupt.
 * @param   enable
 *              1- Watch trigger interrupt enable
 */
void ttcanEnableWatchInt(uint8 enable)
{
    CAN->BIT.wtie = enable;
}


/**
 * @brief   Trigger configuration.
 * @param   type
 *              0- Immediate trigger
 *              1- Time trigger
 *              2- Single shot transmit trigger
 *              3- Transmit start trigger
 *              4- Transmit stop trigger
 *              other- no action
 * @param   tew Transmit enable window.
 * @param   ttptr   Transmit trigger TB slot pointer.
 * @param   time    Trigger time.
 */
void ttcanSetTriggerConfig(uint8 type, uint8 tew, uint8 ttptr, uint16 time)
{
    CAN->BIT.ttype    = type;
    CAN->BIT.tew      = tew;
    CAN->BIT.ttptr    = ttptr;
    CAN->BIT.tttrig_b = time;
}


/**
 * @brief   Get trigger type.
 * @return
 *              0- Immediate trigger
 *              1- Time trigger
 *              2- Single shot transmit trigger
 *              3- Transmit start trigger
 *              4- Transmit stop trigger
 *              other- no action
 */
uint8 ttcanGetTriggerType(void)
{
    return CAN->BIT.ttype;
}


/**
 * @brief   Watch trigger time.
 * @param   time    (15:0)defines the cycle time for a watch trigger.
 */
void ttcanSetWatchTrigger(uint16 time)
{
    CAN->BIT.ttwtrig_b = time;
}


/**
 * @brief   Set pointer to a TB frame slot.
 * @param   pointer
 *                  0- Pointer to the PTB
 *                  others- Pointer to a slot in the STB
 */
void ttcanSetTbptr(uint8 pointer)
{
    CAN->BIT.tbptr = pointer;
}


/**
 * @brief   Set TB slot.
 * @param   state
 *                  1- Slot selected by TBPTR shall be marked as "filled"
 *                  0- Slot selected by TBPTR shall be marked as "empty"
 */
void ttcanSetTb(uint8 state)
{
    if (state)
    {
        CAN->BIT.tbf = 1;
    }
    else
    {
        CAN->BIT.tbe = 1;
    }
}


/**
 * @brief   Reference message REF_MSG_0 to REF_MEG_3
 * @param   refId   REFerence message IDE bit.
 * @param   refIde  REFerence message IDentifier.
 *                  1- REF_ID(28:0) is valid(base ID + identifier extension).
 *                  0- REF_ID(10:0) is valid(base ID only).
 *                  
 */
void ttcanSetRefMsg(uint32 refId, uint8 refIde)
{
    CAN->BIT.ref_msg = refId | (refIde << 31);
}


/**
 * @brief   Set time stamping POSition.
 * @param   position
 *                  0- SOF
 *                  1- EOF
 */
void ttcanSetSamplePos(uint8 position)
{
    CAN->BIT.tspos = position;
}
#endif
