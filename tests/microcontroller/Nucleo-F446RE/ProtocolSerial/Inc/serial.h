#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_dma.h"
#include <stddef.h>

void Serial_Init(void);
void Serial_ChangeBaudRate(uint32_t baudrate);

void Serial_Send(uint8_t* buffer, size_t size);
void Serial_DMA_Send(uint8_t* buffer, size_t size);

void Serial_DMA_RecvMem(uint8_t* buffer, size_t size);

size_t Serial_Send_Status(void);
size_t Serial_Recv_Status(void);