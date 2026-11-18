/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 中; basis recorded in AUTHORS.json.
 */
#ifndef ADT2011_GPIO_BUS_H
#define ADT2011_GPIO_BUS_H

#include "adt2011_driver.h"

/* Construct the ADT6101P-board implementation of the ADT2011 bus port. */
void Adt2011GpioBus_Create(Adt2011BusTypedef *bus);

#endif /* ADT2011_GPIO_BUS_H */
