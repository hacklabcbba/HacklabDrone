#ifndef __HAL_INTERFACE_SPI_H
#define __HAL_INTERFACE_SPI_H

#include "stdint.h"
#include "stddef.h"

/* Spi typedef declaration */
typedef struct PeriphSpiInterface PeriphSpiInterface;

/* Spi typedef members declaration */
typedef uint8_t (*PeriphSpiInit)(void);
typedef uint8_t (*PeriphSpiSetMode)(uint32_t mode);
typedef uint8_t (*PeriphSpiSetBitrate)(uint32_t bitrate);
typedef uint8_t (*PeriphSpiTransaction)(uint8_t data);

/* Spi typedef definition */
struct PeriphSpiInterface
{
	const PeriphSpiInit init;
	const PeriphSpiSetMode setMode;
	const PeriphSpiSetBitrate setBitrate;
	const PeriphSpiTransaction transaction;
};

#endif