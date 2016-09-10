#include "debug_driver.h"

uint8_t Debug_Init(DeviceDebug *device)
{
	device->debugInterface->init();
}

uint8_t Debug_Write(DeviceDebug *device, uint8_t data)
{
	device->debugInterface->write(0xD0);
	device->debugInterface->write(data);
}
