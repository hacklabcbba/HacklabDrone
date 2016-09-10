#include "gtime.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"

#define TIMER1 TIM2
#define TIMER2 TIM9
#define TIMER1_RCC_APBPeriph RCC_APB1Periph_TIM2
#define TIMER2_RCC_APBPeriph RCC_APB2Periph_TIM9

static void TIMER1_Config(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	RCC_ClocksTypeDef RCC_Clocks;
	uint32_t PrescalerValue;

	/* Enable TIM clock */
	RCC_APB1PeriphClockCmd(TIMER1_RCC_APBPeriph, ENABLE);

	/* Get clock configuration */
	RCC_GetClocksFreq(&RCC_Clocks);

	/* Compute the prescaler value to have TIM counter clock equal to 1MHz */
	PrescalerValue = (uint32_t)(((2 * RCC_Clocks.PCLK1_Frequency) / 1000000) - 1);

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = UINT32_MAX;
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIMER1, &TIM_TimeBaseStructure);

	/* Master Mode selection */
	TIM_SelectMasterSlaveMode(TIMER1, TIM_MasterSlaveMode_Enable);
	TIM_SelectOutputTrigger(TIMER1, TIM_TRGOSource_Update);

	/* Reset counter */
	TIM_SetCounter(TIMER1, 0);
}

static void TIMER2_Config(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	/* Enable TIM clock */
	RCC_APB2PeriphClockCmd(TIMER2_RCC_APBPeriph, ENABLE);

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = UINT16_MAX;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIMER2, &TIM_TimeBaseStructure);

	/* Slave Mode selection */
	TIM_SelectSlaveMode(TIMER2, TIM_SlaveMode_External1);
	TIM_SelectInputTrigger(TIMER2, TIM_TS_ITR0);

	/* Reset counter */
	TIM_SetCounter(TIMER2, 0);
}

void GTIME_Init(void)
{
	/* Timer Disable */
	TIM_Cmd(TIMER2, DISABLE);
	TIM_Cmd(TIMER1, DISABLE);

	TIMER2_Config();
	TIMER1_Config();

	/* Timer Enable */
	TIM_Cmd(TIMER2, ENABLE);
	TIM_Cmd(TIMER1, ENABLE);
}

uint64_t GTIME_GetCounter(void)
{
	return (uint64_t)(((uint64_t)TIMER2->CNT << 32) | (TIMER1->CNT));
}