/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 * Andar chip register/protocol content; project adapter author confirmed by owner.
 */
#ifndef ADT2011_DRIVER_H
#define ADT2011_DRIVER_H

#include <stdint.h>

typedef enum
{
    ADT2011_STATUS_OK = 0u,
    ADT2011_STATUS_BAD_PARAM,
    ADT2011_STATUS_NOT_INITIALIZED,
    ADT2011_STATUS_BUS_ERROR
} Adt2011StatusTypedef;

typedef enum
{
    ADT2011_CHIP_TX = 0u,
    ADT2011_CHIP_RX = 1u
} Adt2011ChipTypedef;

/*
 * Hardware boundary for the ADT2011 core driver.
 *
 * The driver owns the ADT2011 register protocol.  The board layer owns GPIO,
 * chip-select polarity, reset timing and the physical serial transfer.
 */
typedef struct
{
    void *context;
    uint8_t (*initialize)(void *context);
    uint8_t (*reset)(void *context);
    uint8_t (*transfer)(void *context,
                        Adt2011ChipTypedef chip,
                        const uint8_t *txData,
                        uint8_t *rxData,
                        uint16_t length);
} Adt2011BusTypedef;

typedef struct
{
    const Adt2011BusTypedef *bus;
    uint8_t initialized;
    uint8_t rfEnabled;
    uint8_t beamPhaseValid[2];
    uint8_t beamPhaseCache[2][16];
} Adt2011DriverTypedef;

Adt2011StatusTypedef Adt2011Driver_Initialize(
    Adt2011DriverTypedef *driver,
    const Adt2011BusTypedef *bus);

/*
 * Beam programming is split deliberately:
 *  - WriteBeamConfig() updates only changed PSR shadow registers (1,3,...,31).
 *  - LoadBeam() writes register 32 and makes the prepared PSR values active.
 * SetBeam() is kept as the immediate/compatibility wrapper.
 */
Adt2011StatusTypedef Adt2011Driver_WriteBeamConfig(
    Adt2011DriverTypedef *driver,
    Adt2011ChipTypedef chip,
    const uint8_t phase[16]);

Adt2011StatusTypedef Adt2011Driver_LoadBeam(
    Adt2011DriverTypedef *driver,
    Adt2011ChipTypedef chip);

Adt2011StatusTypedef Adt2011Driver_SetBeam(
    Adt2011DriverTypedef *driver,
    Adt2011ChipTypedef chip,
    const uint8_t phase[16]);

Adt2011StatusTypedef Adt2011Driver_SetRfEnabled(
    Adt2011DriverTypedef *driver,
    uint8_t enabled);

#endif /* ADT2011_DRIVER_H */

