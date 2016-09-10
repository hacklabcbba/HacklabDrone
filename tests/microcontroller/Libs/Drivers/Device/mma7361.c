#include "mma7361.h"

void MMA7361_Init(MMA7361_DeviceTypedef *device)
{
	device->interfaceAdcX->init();
	device->interfaceAdcY->init();
	device->interfaceAdcZ->init();
}

/**********************************************************************************************/

void MMA7361_Read(MMA7361_DeviceTypedef *device, uint32_t *data)
{
	data[0] = device->interfaceAdcX->read();
	data[1] = device->interfaceAdcY->read();
	data[2] = device->interfaceAdcZ->read();
}