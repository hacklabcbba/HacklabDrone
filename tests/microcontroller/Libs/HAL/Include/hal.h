#ifndef __HAL_MANAGER_H
#define __HAL_MANAGER_H

#include "stdint.h"
#include "hal_config.h"

#include "hal_periph_debug.h"
#include "hal_periph_gpio.h"
#include "hal_periph_spi.h"
#include "hal_periph_uart.h"
#include "hal_periph_adc.h"
#include "hal_periph_rcout.h"

#ifndef HAL_FD_MAX
#define HAL_FD_MAX 20
#endif

/**********************************************************************************************/

typedef enum
{
	HAL_PERIPH_DEBUG = 0x01,
	HAL_PERIPH_GPIO,
	HAL_PERIPH_SPI,
	HAL_PERIPH_UART,
	HAL_PERIPH_I2C,
	HAL_PERIPH_ADC,
	HAL_PERIPH_RCOUT,
	HAL_PERIPH_RCIN,
} PeriphType;

int32_t Hal_RegisterInterface(PeriphType type, const void *interfaceDriver);
void *Hal_GetInterface(int32_t fd);

#endif