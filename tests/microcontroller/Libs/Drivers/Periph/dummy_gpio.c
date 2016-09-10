#include "dummy_gpio.h"

/**********************************************************************************************/

static uint8_t Dummy_PeriphGpio_Init(void) {}
static uint8_t Dummy_PeriphGpio_Write(uint8_t pinState) {}

/**********************************************************************************************/

const PeriphGpioInterface dummyGpioDriver = {
		.init = Dummy_PeriphGpio_Init,
		.write = Dummy_PeriphGpio_Write,
};