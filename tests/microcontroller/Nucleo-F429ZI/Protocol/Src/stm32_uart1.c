#include "stm32_uart1.h"

#if defined(STM32F429xx)
#include "stm32f4xx.h"
#endif

/**********************************************************************************************/

typedef struct
{
	uint32_t RCCPeriph;
	USART_TypeDef *UARTx;
} uart_config_t;

typedef struct
{
	uint32_t RCCPeriph;
	GPIO_TypeDef *GPIOx;
	uint16_t Pin;
	uint8_t PinSource;
	uint8_t AltFuncion;
} gpio_config_t;

typedef struct
{
	uint32_t RCCPeriph;
	DMA_Stream_TypeDef *DMAx_Stream;
	uint32_t Channel;
	uint32_t Flag;
} dma_config_t;

typedef struct
{
	uart_config_t Uart;
	gpio_config_t GpioTx;
	gpio_config_t GpioRx;
	dma_config_t DmaTx;
	dma_config_t DmaRx;
} uart_configTable_t;

/**********************************************************************************************/

#if defined(STM32F429xx)
static const uart_configTable_t UART_ConfigTable[] = {
		{
				{RCC_APB2Periph_USART6, USART6},
				{RCC_AHB1Periph_GPIOG, GPIOG, GPIO_Pin_14, GPIO_PinSource14, GPIO_AF_USART6},
				{RCC_AHB1Periph_GPIOG, GPIOG, GPIO_Pin_9, GPIO_PinSource9, GPIO_AF_USART6},
				{RCC_AHB1Periph_DMA2, DMA2_Stream7, DMA_Channel_5, DMA_FLAG_TCIF7},
				{RCC_AHB1Periph_DMA2, DMA2_Stream2, DMA_Channel_5, DMA_FLAG_TCIF2},
		},
};
#endif

/**********************************************************************************************/

static void UART_GPIOConfig(gpio_config_t GpioTx, gpio_config_t GpioRx)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO clocks enable */
	RCC_AHB1PeriphClockCmd(GpioTx.RCCPeriph, ENABLE);
	RCC_AHB1PeriphClockCmd(GpioRx.RCCPeriph, ENABLE);

	/* GPIO Configuration */
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_InitStruct.GPIO_Pin = GpioTx.Pin;
	GPIO_Init(GpioTx.GPIOx, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = GpioRx.Pin;
	GPIO_Init(GpioRx.GPIOx, &GPIO_InitStruct);

	/* Connect USART pins to AF */
	GPIO_PinAFConfig(GpioTx.GPIOx, GpioTx.PinSource, GpioTx.AltFuncion);
	GPIO_PinAFConfig(GpioRx.GPIOx, GpioRx.PinSource, GpioRx.AltFuncion);
}

static void UART_Config(uart_config_t Uart)
{
	USART_InitTypeDef USART_InitStruct;

	/* Enable peripheral clock for USART */
	RCC_APB2PeriphClockCmd(Uart.RCCPeriph, ENABLE);

	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(Uart.UARTx, &USART_InitStruct);

	/* Enable USART */
	USART_Cmd(Uart.UARTx, ENABLE);
}

static void UART_DMAConfig(uart_config_t Uart, dma_config_t DmaTx, dma_config_t DmaRx)
{
	DMA_InitTypeDef DMA_InitStruct;

	/* DMA clock enable (to be used with USART) */
	RCC_AHB1PeriphClockCmd(DmaTx.RCCPeriph, ENABLE);
	RCC_AHB1PeriphClockCmd(DmaRx.RCCPeriph, ENABLE);

	/* DMA configuration */
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&Uart.UARTx->DR;
	DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)0x00;
	DMA_InitStruct.DMA_BufferSize = 0;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_Priority = DMA_Priority_Low;
	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	DMA_DeInit(DmaTx.DMAx_Stream);
	DMA_InitStruct.DMA_Channel = DmaTx.Channel;
	DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
	DMA_Init(DmaTx.DMAx_Stream, &DMA_InitStruct);

	DMA_DeInit(DmaRx.DMAx_Stream);
	DMA_InitStruct.DMA_Channel = DmaRx.Channel;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	DMA_Init(DmaRx.DMAx_Stream, &DMA_InitStruct);

	/* Enable the USART DMA request */
	USART_DMACmd(Uart.UARTx, USART_DMAReq_Tx, ENABLE);
	USART_DMACmd(Uart.UARTx, USART_DMAReq_Rx, ENABLE);

	/* Enable DMA */
	DMA_Cmd(DmaTx.DMAx_Stream, ENABLE);
	DMA_Cmd(DmaRx.DMAx_Stream, ENABLE);
}

/**********************************************************************************************/

static uint8_t stm32Uart_Init(void)
{
	UART_GPIOConfig(UART_ConfigTable[0].GpioTx, UART_ConfigTable[0].GpioRx);
	UART_Config(UART_ConfigTable[0].Uart);
	UART_DMAConfig(UART_ConfigTable[0].Uart, UART_ConfigTable[0].DmaTx, UART_ConfigTable[0].DmaRx);
	return 0;
}

static uint8_t stm32Uart_SetBaudtrate(uint32_t baudrate)
{
	USART_InitTypeDef USART_InitStruct;

	if(baudrate > 5625000)
		USART_OverSampling8Cmd(UART_ConfigTable[0].Uart.UARTx, ENABLE);
	else
		USART_OverSampling8Cmd(UART_ConfigTable[0].Uart.UARTx, DISABLE);

	USART_InitStruct.USART_BaudRate = baudrate;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(UART_ConfigTable[0].Uart.UARTx, &USART_InitStruct);

	/* Enable USART */
	USART_Cmd(UART_ConfigTable[0].Uart.UARTx, ENABLE);
	return 0;
}

static size_t stm32Uart_Write(uint8_t *data, size_t size)
{
	for(size_t i = 0; i < size; i++)
	{
		USART_SendData(UART_ConfigTable[0].Uart.UARTx, data[i]);
		while(USART_GetFlagStatus(UART_ConfigTable[0].Uart.UARTx, USART_FLAG_TC) == RESET)
			;
	}
	return 0;
}

static size_t stm32Uart_Read(uint8_t *data, size_t size)
{
	for(size_t i = 0; i < size; i++)
	{
		data[i] = USART_ReceiveData(UART_ConfigTable[0].Uart.UARTx);
	}
	return 0;
}

static size_t stm32Uart_WriteDMA(uint8_t *data, size_t size)
{
	/* Disable the USART Tx DMA request */
	USART_DMACmd(UART_ConfigTable[0].Uart.UARTx, USART_DMAReq_Tx, DISABLE);

	/* Disable DMA */
	DMA_Cmd(UART_ConfigTable[0].DmaTx.DMAx_Stream, DISABLE);

	/* Clear DMA flags */
	DMA_ClearFlag(UART_ConfigTable[0].DmaTx.DMAx_Stream, UART_ConfigTable[0].DmaTx.Flag);

	/* Change remaining bytes register */
	UART_ConfigTable[0].DmaTx.DMAx_Stream->NDTR = size;

	/* Change memory address register */
	UART_ConfigTable[0].DmaTx.DMAx_Stream->M0AR = (uint32_t)data;

	/* Enable DMA */
	DMA_Cmd(UART_ConfigTable[0].DmaTx.DMAx_Stream, ENABLE);

	/* Enable the USART Tx DMA request */
	USART_DMACmd(UART_ConfigTable[0].Uart.UARTx, USART_DMAReq_Tx, ENABLE);

	/* Clear DMA flags */
	DMA_ClearFlag(UART_ConfigTable[0].DmaTx.DMAx_Stream, UART_ConfigTable[0].DmaTx.Flag);
	return 0;
}

static size_t stm32Uart_ReadDMA(uint8_t *data, size_t size)
{
	/* Disable DMA */
	DMA_Cmd(UART_ConfigTable[0].DmaRx.DMAx_Stream, DISABLE);

	/* Change remaining bytes register */
	UART_ConfigTable[0].DmaRx.DMAx_Stream->NDTR = size;

	/* Change memory address register */
	UART_ConfigTable[0].DmaRx.DMAx_Stream->M0AR = (uint32_t)data;

	/* Enable DMA */
	DMA_Cmd(UART_ConfigTable[0].DmaRx.DMAx_Stream, ENABLE);
	return 0;
}

static size_t stm32Uart_WriteStatus(void)
{
	return DMA_GetCurrDataCounter(UART_ConfigTable[0].DmaTx.DMAx_Stream);
}

static size_t stm32Uart_ReadStatus(void)
{
	return DMA_GetCurrDataCounter(UART_ConfigTable[0].DmaRx.DMAx_Stream);
}

/**********************************************************************************************/

const PeriphUartInterface stm32Uart1Interface = {
		.init = stm32Uart_Init,
		.setBaudtrate = stm32Uart_SetBaudtrate,
		.write = stm32Uart_Write,
		.read = stm32Uart_Read,
		.writeDMA = stm32Uart_WriteDMA,
		.readDMA = stm32Uart_ReadDMA,
		.writeStatus = stm32Uart_WriteStatus,
		.readStatus = stm32Uart_ReadStatus,
};