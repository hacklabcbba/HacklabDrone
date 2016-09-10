#include "stm32_gpio.h"

#if defined(STM32F446xx)
#include "stm32f4xx.h"
#endif

/**********************************************************************************************/

typedef struct
{
	uint32_t RCCPeriph;
	GPIO_TypeDef *GPIOx;
	uint16_t Pin;
} gpio_config_t;

typedef gpio_config_t gpio_configTable_t;

/**********************************************************************************************/

#if defined(STM32F446xx)
static const gpio_configTable_t GPIO_ConfigTable[] = {
		{RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_5},
		{RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_12},
		{RCC_AHB1Periph_GPIOB, GPIOB, GPIO_Pin_4},
		{RCC_AHB1Periph_GPIOB, GPIOB, GPIO_Pin_5},
};
#endif

/**********************************************************************************************/

static void GPIO_Config(gpio_config_t Gpio)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	RCC_AHB1PeriphClockCmd(Gpio.RCCPeriph, ENABLE);

	/* Configure GPIO pin */
	GPIO_InitStruct.GPIO_Pin = Gpio.Pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(Gpio.GPIOx, &GPIO_InitStruct);

	/* Configure GPIO pin Output Level */
	GPIO_WriteBit(Gpio.GPIOx, Gpio.Pin, Bit_RESET);
}

/**********************************************************************************************/

static uint8_t stm32Gpio0_Init(void)
{
	GPIO_Config(GPIO_ConfigTable[0]);
}

static uint8_t stm32Gpio1_Init(void)
{
	GPIO_Config(GPIO_ConfigTable[1]);
}

static uint8_t stm32Gpio2_Init(void)
{
	GPIO_Config(GPIO_ConfigTable[2]);
}

static uint8_t stm32Gpio3_Init(void)
{
	GPIO_Config(GPIO_ConfigTable[3]);
}

static uint8_t stm32Gpio0_Write(uint8_t pinState)
{
	GPIO_WriteBit(GPIO_ConfigTable[0].GPIOx, GPIO_ConfigTable[0].Pin, pinState);
}

static uint8_t stm32Gpio1_Write(uint8_t pinState)
{
	GPIO_WriteBit(GPIO_ConfigTable[1].GPIOx, GPIO_ConfigTable[1].Pin, pinState);
}

static uint8_t stm32Gpio2_Write(uint8_t pinState)
{
	GPIO_WriteBit(GPIO_ConfigTable[2].GPIOx, GPIO_ConfigTable[2].Pin, pinState);
}

static uint8_t stm32Gpio3_Write(uint8_t pinState)
{
	GPIO_WriteBit(GPIO_ConfigTable[3].GPIOx, GPIO_ConfigTable[3].Pin, pinState);
}

/**********************************************************************************************/

const PeriphGpioInterface stm32Gpio0Interface = {
		.init = stm32Gpio0_Init,
		.write = stm32Gpio0_Write,
};

const PeriphGpioInterface stm32Gpio1Interface = {
		.init = stm32Gpio1_Init,
		.write = stm32Gpio1_Write,
};

const PeriphGpioInterface stm32Gpio2Interface = {
		.init = stm32Gpio2_Init,
		.write = stm32Gpio2_Write,
};

const PeriphGpioInterface stm32Gpio3Interface = {
		.init = stm32Gpio3_Init,
		.write = stm32Gpio3_Write,
};