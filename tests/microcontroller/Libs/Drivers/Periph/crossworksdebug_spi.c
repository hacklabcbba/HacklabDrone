#include <cross_studio_io.h>
#include "crossworksdebug_spi.h"

/**********************************************************************************************/

static uint8_t Crossworks_PeriphSpi_Init(void)
{
	debug_printf("Crossworks_PeriphSpi_Init()\n");
}

static uint8_t Crossworks_PeriphSpi_Transaction(uint8_t data)
{
	debug_printf("Crossworks_PeriphSpi_Transaction(0x%02X)\n", data);
}

/**********************************************************************************************/

const PeriphSpiInterface crossDebugSpiDriver = {
		.init = Crossworks_PeriphSpi_Init,
		.transaction = Crossworks_PeriphSpi_Transaction,
};