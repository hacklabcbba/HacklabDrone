#include "gtime.h"
#include "stm32f7xx_hal.h"

#define TIMER1 TIM2
#define TIMER2 TIM9

void TIM_SelectMasterSlaveMode(TIM_TypeDef *TIMx, uint16_t TIM_MasterSlaveMode)
{
	/* Reset the MSM Bit */
	TIMx->SMCR &= (uint16_t)~TIM_SMCR_MSM;

	/* Set or Reset the MSM Bit */
	TIMx->SMCR |= TIM_MasterSlaveMode;
}

void TIM_SelectOutputTrigger(TIM_TypeDef *TIMx, uint16_t TIM_TRGOSource)
{
	/* Reset the MMS Bits */
	TIMx->CR2 &= (uint16_t)~TIM_CR2_MMS;
	/* Select the TRGO source */
	TIMx->CR2 |= TIM_TRGOSource;
}

void TIM_SelectSlaveMode(TIM_TypeDef *TIMx, uint16_t TIM_SlaveMode)
{
	/* Reset the SMS Bits */
	TIMx->SMCR &= (uint16_t)~TIM_SMCR_SMS;

	/* Select the Slave Mode */
	TIMx->SMCR |= TIM_SlaveMode;
}

void TIM_SelectInputTrigger(TIM_TypeDef *TIMx, uint16_t TIM_InputTriggerSource)
{
	uint16_t tmpsmcr = 0;

	/* Get the TIMx SMCR register value */
	tmpsmcr = TIMx->SMCR;

	/* Reset the TS Bits */
	tmpsmcr &= (uint16_t)~TIM_SMCR_TS;

	/* Set the Input Trigger source */
	tmpsmcr |= TIM_InputTriggerSource;

	/* Write to TIMx SMCR */
	TIMx->SMCR = tmpsmcr;
}

void __TIMER1_Config(void)
{
	TIM_Base_InitTypeDef TIM_TimeBaseStructure;
	__HAL_RCC_TIM2_CLK_ENABLE();

	/* Time base configuration */
	TIM_TimeBaseStructure.Period = UINT32_MAX;
	TIM_TimeBaseStructure.Prescaler = 54 - 1;
	TIM_TimeBaseStructure.ClockDivision = 0;
	TIM_TimeBaseStructure.CounterMode = TIM_COUNTERMODE_UP;
	TIM_Base_SetConfig(TIMER1, &TIM_TimeBaseStructure);

	/* Master Mode selection */
	TIM_SelectMasterSlaveMode(TIMER1, TIM_MASTERSLAVEMODE_ENABLE);
	TIM_SelectOutputTrigger(TIMER1, TIM_TRGO_UPDATE);

	/* Reset counter */
	TIMER1->CNT = 0;
}

void __TIMER2_Config(void)
{
	TIM_Base_InitTypeDef TIM_TimeBaseStructure;
	__HAL_RCC_TIM9_CLK_ENABLE();

	/* Time base configuration */
	TIM_TimeBaseStructure.Period = UINT16_MAX;
	TIM_TimeBaseStructure.Prescaler = 0;
	TIM_TimeBaseStructure.ClockDivision = 0;
	TIM_TimeBaseStructure.CounterMode = TIM_COUNTERMODE_UP;
	TIM_Base_SetConfig(TIMER2, &TIM_TimeBaseStructure);

	/* Slave Mode selection */
	TIM_SelectSlaveMode(TIMER2, TIM_SLAVEMODE_EXTERNAL1);
	TIM_SelectInputTrigger(TIMER2, TIM_TS_ITR0);

	/* Reset counter */
	TIMER2->CNT = 0;
}

void GTIME_Init(void)
{
	/* Timer Disable */
	CLEAR_BIT(TIMER1->CR1, TIM_CR1_CEN);
	CLEAR_BIT(TIMER2->CR1, TIM_CR1_CEN);

	__TIMER2_Config();
	__TIMER1_Config();

	/* Timer Enable */
	SET_BIT(TIMER1->CR1, TIM_CR1_CEN);
	SET_BIT(TIMER2->CR1, TIM_CR1_CEN);
}

uint64_t GTIME_GetCounter(void)
{
	return (uint64_t)(((uint64_t)TIMER2->CNT << 32) | (TIMER1->CNT));
}