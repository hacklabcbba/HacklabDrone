#include "dummy_spi.h"

/**********************************************************************************************/

static uint8_t Dummy_PeriphSpi_Init(void) {}
static uint8_t Dummy_PeriphSpi_Transaction(uint8_t data) {}

/**********************************************************************************************/

const PeriphSpiInterface dummySpiDriver = {
		.init = Dummy_PeriphSpi_Init,
		.transaction = Dummy_PeriphSpi_Transaction,
};