#include "stm32f4xx_conf.h"
#include "serial.h"

void _Serial_GPIOConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* GPIOC clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* GPIOC Configuration:  USART2 TX & RX on PA2 & PA3 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Connect USART2 pins to AF */
	// TX = PA2
	// RX = PA3
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
}

void _Serial_USARTConfig(void)
{
	USART_InitTypeDef USART_InitStructure;

	/* enable peripheral clock for USART2 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	// Enable USART2
	USART_Cmd(USART2, ENABLE);
}

void _Serial_DMA_RX_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;

  /* DMA1 clock enable (to be used with USART2) */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

	/* DMA RX */
  /* DMA1_Stream5 channel4 configuration **************************************/
	DMA_DeInit(DMA1_Stream5);
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)NULL;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;

	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream5, &DMA_InitStructure);

	/* Enable the USART Rx DMA request */
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);

	/* Enable DMA1_Stream5 */
	DMA_Cmd(DMA1_Stream5, ENABLE);
}

void _Serial_DMA_TX_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;

  /* DMA1 clock enable (to be used with USART2) */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

	/* DMA RX */
  /* DMA1_Stream6 channel4 configuration **************************************/
	DMA_DeInit(DMA1_Stream6);
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0x00;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;

	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream6, &DMA_InitStructure);

  /* Disable the USART Tx DMA request */
	USART_DMACmd(USART2, USART_DMAReq_Tx, DISABLE);

	/* Disable DMA1_Stream6 */
	DMA_Cmd(DMA1_Stream6, DISABLE);
}

void Serial_Init(void)
{
	_Serial_GPIOConfig();
	_Serial_USARTConfig();
	_Serial_DMA_TX_Config();
	_Serial_DMA_RX_Config();
}

void Serial_ChangeBaudRate(uint32_t baudrate)
{
	USART_InitTypeDef USART_InitStructure;

	if(baudrate > 2812500)
	{
		USART_OverSampling8Cmd(USART2, ENABLE);
	}
	else
	{
		USART_OverSampling8Cmd(USART2, DISABLE);
	}

	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART2, &USART_InitStructure);

	// Enable USART2
	USART_Cmd(USART2, ENABLE);
}

void Serial_Send(uint8_t* buffer, size_t size)
{
	int32_t i;
	for(i=0; i<size; i++)
	{
		USART_SendData(USART2, *buffer);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
		buffer++;
	}
}

void Serial_DMA_Send(uint8_t* buffer, size_t size)
{
  /* Disable the USART Tx DMA request */
  USART_DMACmd(USART2, USART_DMAReq_Tx, DISABLE);

	/* Disable DMA1_Stream6 */
	DMA_Cmd(DMA1_Stream6, DISABLE);

	/* Clear DMA flags */
	DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);


	/* Change remaining bytes register */
	DMA1_Stream6->NDTR = size;

	/* Change memory address register */
	DMA1_Stream6->M0AR = (uint32_t)buffer;


	/* Enable DMA1_Stream6 */
	DMA_Cmd(DMA1_Stream6, ENABLE);

  /* Enable the USART Tx DMA request */
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);

	/* Clear DMA flags */
	DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);

	/* Waiting the end of Data transfer */
	//while(DMA_GetFlagStatus(DMA1_FLAG_TCIF6) == RESET);
}

void Serial_DMA_RecvMem(uint8_t* buffer, size_t size)
{
	/* Disable DMA1 Stream5 */
	DMA_Cmd(DMA1_Stream5, DISABLE);

	/* Change remaining bytes register */
	DMA1_Stream5->NDTR = size;

	/* Change memory address register */
	DMA1_Stream5->M0AR = (uint32_t)buffer;
	
	/* Enable DMA1 Stream5 */
	DMA_Cmd(DMA1_Stream5, ENABLE);
}


size_t Serial_Send_Status(void)
{
	return DMA_GetCurrDataCounter(DMA1_Stream6);
}

size_t Serial_Recv_Status(void)
{
	return DMA_GetCurrDataCounter(DMA1_Stream5);
}