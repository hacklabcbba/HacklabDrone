#ifndef __HAL_INTERFACE_ADC_H
#define __HAL_INTERFACE_ADC_H

#include "stdint.h"

/* Adc typedef declaration */
typedef struct PeriphAdcInterface PeriphAdcInterface;

/* Adc typedef members declaration */
typedef uint8_t (*PeriphAdcInit)(void);
typedef uint32_t (*PeriphAdcRead)(void);

/* Adc typedef definition */
struct PeriphAdcInterface
{
	const PeriphAdcInit init;
	const PeriphAdcRead read;
};

#endif