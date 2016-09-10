#ifndef __HAL_INTERFACE_RCOUT_H
#define __HAL_INTERFACE_RCOUT_H

#include "stdint.h"
#include "stddef.h"

/* RCOut typedef declaration */
typedef struct PeriphRCOutInterface PeriphRCOutInterface;

/* RCOut typedef members declaration */
typedef uint8_t (*PeriphRCOutInit)(void);
typedef uint8_t (*PeriphRCOutSetPeriod)(uint32_t period);
typedef uint8_t (*PeriphRCOutWrite)(uint32_t channel, uint32_t pulse);

/* RCOut typedef definition */
struct PeriphRCOutInterface
{
	const PeriphRCOutInit init;
	const PeriphRCOutSetPeriod setPeriod;
	const PeriphRCOutWrite write;
};

#endif