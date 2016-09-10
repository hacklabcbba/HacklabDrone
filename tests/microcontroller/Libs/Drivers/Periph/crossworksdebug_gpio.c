#include <cross_studio_io.h>
#include "crossworksdebug_gpio.h"

/**********************************************************************************************/

static uint8_t Crossworks_Init(uint8_t device)
{
	debug_printf("Crossworks_PeriphGpio%d_Init()\n", device);
}

static uint8_t Crossworks_Write(uint8_t device, uint8_t pinState)
{
	debug_printf("Crossworks_PeriphGpio%d_Write(%d)\n", device, pinState);
}

/**********************************************************************************************/

static uint8_t Crossworks_PeriphGpio1_Init(void)
{
	Crossworks_Init(0x01);
}

static uint8_t Crossworks_PeriphGpio1_Write(uint8_t pinState)
{
	Crossworks_Write(0x01, pinState);
}

static uint8_t Crossworks_PeriphGpio2_Init(void)
{
	Crossworks_Init(0x02);
}

static uint8_t Crossworks_PeriphGpio2_Write(uint8_t pinState)
{
	Crossworks_Write(0x02, pinState);
}

/**********************************************************************************************/

const PeriphGpioInterface crossDebugGpio1Driver = {
		.init = Crossworks_PeriphGpio1_Init,
		.write = Crossworks_PeriphGpio1_Write,
};

const PeriphGpioInterface crossDebugGpio2Driver = {
		.init = Crossworks_PeriphGpio2_Init,
		.write = Crossworks_PeriphGpio2_Write,
};