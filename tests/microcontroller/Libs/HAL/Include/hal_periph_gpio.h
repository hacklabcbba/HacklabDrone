#ifndef __HAL_INTERFACE_GPIO_H
#define __HAL_INTERFACE_GPIO_H

#include "stdint.h"

/* Gpio typedef declaration */
typedef struct PeriphGpioInterface PeriphGpioInterface;

/* Gpio typedef members declaration */
typedef uint8_t (*PeriphGpioInit)(void);
typedef uint8_t (*PeriphGpioSetMode)(uint32_t pinMode);
typedef uint8_t (*PeriphGpioWrite)(uint8_t pinState);
typedef uint8_t (*PeriphGpioRead)(void);

/* Gpio typedef definition */
struct PeriphGpioInterface
{
	const PeriphGpioInit init;
	const PeriphGpioSetMode setMode;
	const PeriphGpioWrite write;
	const PeriphGpioRead read;
};

#endif