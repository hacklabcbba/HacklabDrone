#include "stm32_rcout.h"

#if defined(STM32F429xx)
#include "stm32f4xx.h"
#endif

/**********************************************************************************************/

typedef struct
{
	uint32_t RCCPeriph;
	TIM_TypeDef *TIMx;
} timer_config_t;

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
	timer_config_t Timer;
	gpio_config_t Gpio1;
	gpio_config_t Gpio2;
	gpio_config_t Gpio3;
	gpio_config_t Gpio4;
} rcout_configTable_t;

/**********************************************************************************************/

#if defined(STM32F429xx)
static const rcout_configTable_t RCOUT_ConfigTable[] = {
		{
				{RCC_APB2Periph_TIM8, TIM8},
				{RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_6, GPIO_PinSource6, GPIO_AF_TIM8},
				{RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_7, GPIO_PinSource7, GPIO_AF_TIM8},
				{RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_8, GPIO_PinSource8, GPIO_AF_TIM8},
				{RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_9, GPIO_PinSource9, GPIO_AF_TIM8},
		},
		{
				{RCC_APB2Periph_TIM1, TIM1},
				{RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_9, GPIO_PinSource9, GPIO_AF_TIM1},
				{RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_11, GPIO_PinSource11, GPIO_AF_TIM1},
				{RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_13, GPIO_PinSource13, GPIO_AF_TIM1},
				{RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_14, GPIO_PinSource14, GPIO_AF_TIM1},
		},
};
#endif

/**********************************************************************************************/

inline static void RCOUT_GPIOConfig(gpio_config_t Gpio)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Enable GPIO clocks */
	RCC_AHB1PeriphClockCmd(Gpio.RCCPeriph, ENABLE);

	/* Configure GPIO pin */
	GPIO_InitStruct.GPIO_Pin = Gpio.Pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(Gpio.GPIOx, &GPIO_InitStruct);

	/* Connect TIM pins to AF */
	GPIO_PinAFConfig(Gpio.GPIOx, Gpio.PinSource, Gpio.AltFuncion);
}

inline static void RCOUT_TIMConfig(timer_config_t Timer)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;

	/* Timer Clock Enable */
	RCC_APB2PeriphClockCmd(Timer.RCCPeriph, ENABLE);

	/* Timer base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStruct);
	TIM_TimeBaseStruct.TIM_Period = 20000 - 1;
	TIM_TimeBaseStruct.TIM_Prescaler = 180 - 1; // 180 MHz Clock down to 1MHz
	TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(Timer.TIMx, &TIM_TimeBaseStruct);

	/* Timer Auto-reload always */
	TIM_ARRPreloadConfig(Timer.TIMx, ENABLE);

	/* Timer Channels initialization */
	TIM_OCStructInit(&TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_Pulse = 0;
	TIM_OC1Init(Timer.TIMx, &TIM_OCInitStruct);
	TIM_OC2Init(Timer.TIMx, &TIM_OCInitStruct);
	TIM_OC3Init(Timer.TIMx, &TIM_OCInitStruct);
	TIM_OC4Init(Timer.TIMx, &TIM_OCInitStruct);

	/* Timer CCRs auto-reload always */
	TIM_OC1PreloadConfig(Timer.TIMx, TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(Timer.TIMx, TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(Timer.TIMx, TIM_OCPreload_Enable);
	TIM_OC4PreloadConfig(Timer.TIMx, TIM_OCPreload_Enable);

	/* Enable Timer Main Output */
	TIM_CtrlPWMOutputs(Timer.TIMx, ENABLE);

	/* Enable Timer*/
	TIM_Cmd(Timer.TIMx, ENABLE);
}

/**********************************************************************************************/

static uint8_t stm32RCOut_Init(void)
{
	RCOUT_GPIOConfig(RCOUT_ConfigTable[0].Gpio1);
	RCOUT_GPIOConfig(RCOUT_ConfigTable[0].Gpio2);
	RCOUT_GPIOConfig(RCOUT_ConfigTable[0].Gpio3);
	RCOUT_GPIOConfig(RCOUT_ConfigTable[0].Gpio4);
	RCOUT_TIMConfig(RCOUT_ConfigTable[0].Timer);

	RCOUT_GPIOConfig(RCOUT_ConfigTable[1].Gpio1);
	RCOUT_GPIOConfig(RCOUT_ConfigTable[1].Gpio2);
	RCOUT_GPIOConfig(RCOUT_ConfigTable[1].Gpio3);
	RCOUT_GPIOConfig(RCOUT_ConfigTable[1].Gpio4);
	RCOUT_TIMConfig(RCOUT_ConfigTable[1].Timer);
}

static uint8_t stm32RCOut_Write(uint32_t channel, uint32_t pulse)
{
	switch(channel)
	{
	case 0:
		TIM_SetCompare1(RCOUT_ConfigTable[0].Timer.TIMx, pulse);
		break;
	case 1:
		TIM_SetCompare2(RCOUT_ConfigTable[0].Timer.TIMx, pulse);
		break;
	case 2:
		TIM_SetCompare3(RCOUT_ConfigTable[0].Timer.TIMx, pulse);
		break;
	case 3:
		TIM_SetCompare4(RCOUT_ConfigTable[0].Timer.TIMx, pulse);
		break;
	case 4:
		TIM_SetCompare1(RCOUT_ConfigTable[1].Timer.TIMx, pulse);
		break;
	case 5:
		TIM_SetCompare2(RCOUT_ConfigTable[1].Timer.TIMx, pulse);
		break;
	case 6:
		TIM_SetCompare3(RCOUT_ConfigTable[1].Timer.TIMx, pulse);
		break;
	case 7:
		TIM_SetCompare4(RCOUT_ConfigTable[1].Timer.TIMx, pulse);
		break;
	}
}

/**********************************************************************************************/

const PeriphRCOutInterface stm32RCOutInterface = {
		.init = stm32RCOut_Init,
		.write = stm32RCOut_Write,
};