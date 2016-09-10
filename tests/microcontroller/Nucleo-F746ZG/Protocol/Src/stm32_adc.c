#include "stm32_adc.h"
#include "mxconstants.h"

#ifdef STM32F746xx
#include "stm32f7xx.h"
#include "stm32f7xx_hal_adc.h"
#include "stm32f7xx_hal_gpio.h"
#include "stm32f7xx_hal_rcc.h"
#elif STM32F429xx
#include "stm32f4xx.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#endif

/**********************************************************************************************/

#define ADC_CH1 ADC_CHANNEL_10
#define ADC_CH2 ADC_CHANNEL_11
#define ADC_CH3 ADC_CHANNEL_8

/**********************************************************************************************/

void Analog_Init(void)
{
//	ADC_InitTypeDef ADC_InitStructure;
//	ADC_CommonInitTypeDef ADC_CommonInitStructure;
//	GPIO_InitTypeDef GPIO_InitStructure;
//
//	/* Enable ADC1 and GPIO clocks ****************************************/
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
//
//	/* Configure ADC1 Channel8 pin as analog input ******************************/
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//
//	/* Configure ADC1 Channel11-12 pin as analog input ******************************/
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//	GPIO_Init(GPIOC, &GPIO_InitStructure);
//
//	/* ADC Common Init **********************************************************/
//	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
//	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
//	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
//	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_10Cycles;
//	ADC_CommonInit(&ADC_CommonInitStructure);
//
//	/* ADC1 Init ****************************************************************/
//	ADC_DeInit();
//	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
//	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
//	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
//	ADC_InitStructure.ADC_ExternalTrigConvEdge = 0;
//	ADC_InitStructure.ADC_ExternalTrigConv = 0;
//	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//	ADC_InitStructure.ADC_NbrOfConversion = 1;
//	ADC_Init(ADC1, &ADC_InitStructure);
//
//	/* Enable ADC1 */
//	ADC_Cmd(ADC1, ENABLE);
}

/**********************************************************************************************/

uint8_t __stm32Adc1_Init(void);
uint32_t __stm32Adc1_Read(void);

uint8_t __stm32Adc2_Init(void);
uint32_t __stm32Adc2_Read(void);

uint8_t __stm32Adc3_Init(void);
uint32_t __stm32Adc3_Read(void);

/**********************************************************************************************/

const PeriphAdcInterface stm32Adc1Interface = {
		.init = __stm32Adc1_Init,
		.read = __stm32Adc1_Read,
};

const PeriphAdcInterface stm32Adc2Interface = {
		.init = __stm32Adc2_Init,
		.read = __stm32Adc2_Read,
};

const PeriphAdcInterface stm32Adc3Interface = {
		.init = __stm32Adc3_Init,
		.read = __stm32Adc3_Read,
};

/**********************************************************************************************/

uint8_t __stm32Adc1_Init(void)
{
	Analog_Init();
}

uint32_t __stm32Adc1_Read(void)
{
//	ADC_RegularChannelConfig(ADC1, ADC_CH1, 1, ADC_SAMPLETIME_112CYCLES);
//	ADC_SoftwareStartConv(ADC1);
//	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
//		;
//	return ADC_GetConversionValue(ADC1);
}

uint8_t __stm32Adc2_Init(void)
{
	Analog_Init();
}

uint32_t __stm32Adc2_Read(void)
{
//	ADC_RegularChannelConfig(ADC1, ADC_CH2, 1, ADC_SAMPLETIME_112CYCLES);
//	ADC_SoftwareStartConv(ADC1);
//	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
//		;
//	return ADC_GetConversionValue(ADC1);
}

uint8_t __stm32Adc3_Init(void)
{
	Analog_Init();
}

uint32_t __stm32Adc3_Read(void)
{
//	ADC_RegularChannelConfig(ADC1, ADC_CH3, 1, ADC_SAMPLETIME_112CYCLES);
//	ADC_SoftwareStartConv(ADC1);
//	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
//		;
//	return ADC_GetConversionValue(ADC1);
}