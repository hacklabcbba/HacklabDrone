#ifndef __HAL_INTERFACE_UART_H
#define __HAL_INTERFACE_UART_H

#include "stdint.h"
#include "stddef.h"

/* Uart typedef declaration */
typedef struct PeriphUartInterface PeriphUartInterface;

/* Uart typedef members declaration */
typedef uint8_t (*PeriphUartInit)(void);
typedef uint8_t (*PeriphUartSetMode)(uint32_t mode);
typedef uint8_t (*PeriphUartSetBaudtrate)(uint32_t baudrate);
typedef size_t (*PeriphUartWrite)(uint8_t* data, size_t size);
typedef size_t (*PeriphUartRead)(uint8_t* data, size_t size);
typedef size_t (*PeriphUartWriteDMA)(uint8_t *data, size_t size);
typedef size_t (*PeriphUartReadDMA)(uint8_t *data, size_t size);
typedef size_t (*PeriphUartWriteStatus)(void);
typedef size_t (*PeriphUartReadStatus)(void);

/* Uart typedef definition */
struct PeriphUartInterface
{
	const PeriphUartInit init;
	const PeriphUartSetMode setMode;
	const PeriphUartSetBaudtrate setBaudtrate;
	const PeriphUartWrite write;
	const PeriphUartRead read;
	const PeriphUartWriteDMA writeDMA;
	const PeriphUartReadDMA readDMA;
	const PeriphUartWriteStatus writeStatus;
	const PeriphUartReadStatus readStatus;
};

#endif