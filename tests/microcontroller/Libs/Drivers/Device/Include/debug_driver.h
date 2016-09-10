#ifndef __DEBUG_DRIVER_H
#define __DEBUG_DRIVER_H

#include "stdint.h"
#include "hal.h"

typedef struct
{
	PeriphDebugInterface *debugInterface;
} DeviceDebug;

uint8_t Debug_Init(DeviceDebug *device);
uint8_t Debug_Write(DeviceDebug *device, uint8_t data);

#endif