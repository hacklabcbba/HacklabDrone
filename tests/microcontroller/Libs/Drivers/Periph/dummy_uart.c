#include "dummy_uart.h"

/**********************************************************************************************/

static uint8_t Dummy_PeriphUart_Init(void) {}
static uint8_t Dummy_PeriphUart_SetBaudtrate(uint32_t baudrate) {}
static size_t Dummy_PeriphUart_Write(uint8_t *data, size_t size) {}
static size_t Dummy_PeriphUart_Read(uint8_t *data, size_t size) {}
static size_t Dummy_PeriphUart_WriteDMA(uint8_t *data, size_t size) {}
static size_t Dummy_PeriphUart_ReadDMA(uint8_t *data, size_t size) {}
static size_t Dummy_PeriphUart_WriteStatus(void) {}
static size_t Dummy_PeriphUart_ReadStatus(void) {}

/**********************************************************************************************/

const PeriphUartInterface Dummy_PeriphUart1Interface = {
		.init = Dummy_PeriphUart_Init,
		.setBaudtrate = Dummy_PeriphUart_SetBaudtrate,
		.write = Dummy_PeriphUart_Write,
		.read = Dummy_PeriphUart_Read,
		.writeDMA = Dummy_PeriphUart_WriteDMA,
		.readDMA = Dummy_PeriphUart_ReadDMA,
		.writeStatus = Dummy_PeriphUart_WriteStatus,
		.readStatus = Dummy_PeriphUart_ReadStatus,
};