#ifndef __HAL_INTERFACE_DEBUG_H
#define __HAL_INTERFACE_DEBUG_H

#include "stdint.h"

/* Debug typedef declaration */
typedef struct PeriphDebugInterface PeriphDebugInterface;

/* Debug typedef members declaration */
typedef uint8_t (*PeriphDebugInit)(void);
typedef uint8_t (*PeriphDebugWrite)(uint8_t data);

/* Debug typedef definition */
struct PeriphDebugInterface
{
	const PeriphDebugInit init;
	const PeriphDebugWrite write;
};

#endif