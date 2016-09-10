#include "stm32_adc.h"

#if defined(STM32F446xx)
#include "stm32f4xx.h"
#endif

/**********************************************************************************************/

typedef struct
{
	uint32_t RCCPeriph;
	ADC_TypeDef *ADCx;
	uint8_t Channel;
} adc_config_t;

typedef struct
{
	uint32_t RCCPeriph;
	GPIO_TypeDef *GPIOx;
	uint16_t Pin;
	uint8_t PinSource;
} gpio_config_t;

typedef struct
{
	adc_config_t Adc;
	gpio_config_t Gpio;
} adc_configTable_t;

/**********************************************************************************************/

#if defined(STM32F446xx)
static const adc_configTable_t ADC_ConfigTable[] = {
		{{RCC_APB2Periph_ADC1, ADC1, ADC_Channel_0}, {RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_0}},
		{{RCC_APB2Periph_ADC1, ADC1, ADC_Channel_1}, {RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_1}},
		{{RCC_APB2Periph_ADC1, ADC1, ADC_Channel_4}, {RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_4}},
		{{RCC_APB2Periph_ADC1, ADC1, ADC_Channel_8}, {RCC_AHB1Periph_GPIOB, GPIOB, GPIO_Pin_0}},
		{{RCC_APB2Periph_ADC1, ADC1, ADC_Channel_11}, {RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_1}},
		{{RCC_APB2Periph_ADC1, ADC1, ADC_Channel_10}, {RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_0}},
};
#endif

/**********************************************************************************************/

inline static void ADC_GPIOConfig(gpio_config_t Gpio)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Enable GPIO clocks */
	RCC_AHB1PeriphClockCmd(Gpio.RCCPeriph, ENABLE);

	/* Configure GPIO Pin as analog input */
	GPIO_InitStruct.GPIO_Pin = Gpio.Pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(Gpio.GPIOx, &GPIO_InitStruct);
}

inline static void ADC_Config(adc_config_t Adc)
{
	ADC_InitTypeDef ADC_InitStruct;
	ADC_CommonInitTypeDef ADC_CommonInitStruct;

	/* Enable ADC clocks */
	RCC_APB2PeriphClockCmd(Adc.RCCPeriph, ENABLE);

	/* ADC Common Init */
	ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_10Cycles;
	ADC_CommonInit(&ADC_CommonInitStruct);

	/* ADC Init */
	ADC_DeInit();
	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStruct.ADC_ScanConvMode = DISABLE;
	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStruct.ADC_ExternalTrigConvEdge = 0;
	ADC_InitStruct.ADC_ExternalTrigConv = 0;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_NbrOfConversion = 1;
	ADC_Init(Adc.ADCx, &ADC_InitStruct);

	/* Enable ADC */
	ADC_Cmd(Adc.ADCx, ENABLE);
}

inline static uint32_t Adc_Read(adc_config_t Adc)
{
	ADC_RegularChannelConfig(Adc.ADCx, Adc.Channel, 1, ADC_SampleTime_112Cycles);
	ADC_SoftwareStartConv(Adc.ADCx);
	while(ADC_GetFlagStatus(Adc.ADCx, ADC_FLAG_EOC) == RESET)
		;
	return ADC_GetConversionValue(Adc.ADCx);
}

/**********************************************************************************************/

static uint8_t stm32Adc0_Init(void)
{
	ADC_Config(ADC_ConfigTable[0].Adc);
	ADC_GPIOConfig(ADC_ConfigTable[0].Gpio);
}

static uint8_t stm32Adc1_Init(void)
{
	ADC_Config(ADC_ConfigTable[1].Adc);
	ADC_GPIOConfig(ADC_ConfigTable[1].Gpio);
}

static uint8_t stm32Adc2_Init(void)
{
	ADC_Config(ADC_ConfigTable[2].Adc);
	ADC_GPIOConfig(ADC_ConfigTable[2].Gpio);
}

static uint8_t stm32Adc3_Init(void)
{
	ADC_Config(ADC_ConfigTable[3].Adc);
	ADC_GPIOConfig(ADC_ConfigTable[3].Gpio);
}

static uint8_t stm32Adc4_Init(void)
{
	ADC_Config(ADC_ConfigTable[4].Adc);
	ADC_GPIOConfig(ADC_ConfigTable[4].Gpio);
}

static uint8_t stm32Adc5_Init(void)
{
	ADC_Config(ADC_ConfigTable[5].Adc);
	ADC_GPIOConfig(ADC_ConfigTable[5].Gpio);
}

static uint32_t stm32Adc0_Read(void)
{
	Adc_Read(ADC_ConfigTable[0].Adc);
}

static uint32_t stm32Adc1_Read(void)
{
	Adc_Read(ADC_ConfigTable[1].Adc);
}

static uint32_t stm32Adc2_Read(void)
{
	Adc_Read(ADC_ConfigTable[2].Adc);
}

static uint32_t stm32Adc3_Read(void)
{
	Adc_Read(ADC_ConfigTable[3].Adc);
}

static uint32_t stm32Adc4_Read(void)
{
	Adc_Read(ADC_ConfigTable[4].Adc);
}

static uint32_t stm32Adc5_Read(void)
{
	Adc_Read(ADC_ConfigTable[5].Adc);
}

/**********************************************************************************************/

const PeriphAdcInterface stm32Adc0Interface = {
		.init = stm32Adc0_Init,
		.read = stm32Adc0_Read,
};

const PeriphAdcInterface stm32Adc1Interface = {
		.init = stm32Adc1_Init,
		.read = stm32Adc1_Read,
};

const PeriphAdcInterface stm32Adc2Interface = {
		.init = stm32Adc2_Init,
		.read = stm32Adc2_Read,
};

const PeriphAdcInterface stm32Adc3Interface = {
		.init = stm32Adc3_Init,
		.read = stm32Adc3_Read,
};

const PeriphAdcInterface stm32Adc4Interface = {
		.init = stm32Adc4_Init,
		.read = stm32Adc4_Read,
};

const PeriphAdcInterface stm32Adc5Interface = {
		.init = stm32Adc5_Init,
		.read = stm32Adc5_Read,
};