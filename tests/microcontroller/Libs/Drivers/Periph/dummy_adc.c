#include "dummy_adc.h"

/**********************************************************************************************/

static uint8_t Dummy_PeriphAdc_Init(void) {}
static uint32_t Dummy_PeriphAdc_Read(void) {}

/**********************************************************************************************/

const PeriphAdcInterface dummyAdcDriver = {
		.init = Dummy_PeriphAdc_Init,
		.read = Dummy_PeriphAdc_Read,
};