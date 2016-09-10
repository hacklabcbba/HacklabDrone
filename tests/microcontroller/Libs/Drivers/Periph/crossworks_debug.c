#include <cross_studio_io.h>
#include "crossworks_debug.h"

/**********************************************************************************************/

static uint8_t Crossworks_PeriphDebug_Init(void)
{
	debug_printf("Crossworks_PeriphDebug_Init()\n");
}

static uint8_t Crossworks_PeriphDebug_Write(uint8_t data)
{
	debug_printf("Crossworks_PeriphDebug_Write(0x%02X)\n", data);
}

/**********************************************************************************************/

const PeriphDebugInterface crossDebugDriver = {
		.init = Crossworks_PeriphDebug_Init,
		.write = Crossworks_PeriphDebug_Write,
};