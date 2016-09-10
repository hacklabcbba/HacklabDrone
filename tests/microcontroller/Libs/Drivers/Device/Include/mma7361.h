#include "hal.h"

/* MMA7361 struct */
typedef struct
{
	PeriphAdcInterface *interfaceAdcX;
	PeriphAdcInterface *interfaceAdcY;
	PeriphAdcInterface *interfaceAdcZ;
} MMA7361_DeviceTypedef;

void MMA7361_Init(MMA7361_DeviceTypedef *device);

void MMA7361_Read(MMA7361_DeviceTypedef *device, uint32_t *data);
