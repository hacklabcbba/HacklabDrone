#include "stm32_gpioLeds.h"
#include "mxconstants.h"

#ifdef STM32F746xx
#include "stm32f7xx.h"
#elif STM32F429xx
#include "stm32f4xx.h"
#endif

void __GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
  /* Check the parameters */
  assert_param(IS_GPIO_PIN(GPIO_Pin));
  assert_param(IS_GPIO_PIN_ACTION(PinState));

  if(PinState != GPIO_PIN_RESET)
  {
    GPIOx->BSRR = GPIO_Pin;
  }
  else
  {
    GPIOx->BSRR = (uint32_t)GPIO_Pin << 16U;
  }
}

/**********************************************************************************************/

uint8_t __stm32Led1_Init(void);
uint8_t __stm32Led1_Write(uint8_t pinState);

uint8_t __stm32Led2_Init(void);
uint8_t __stm32Led2_Write(uint8_t pinState);

uint8_t __stm32Led3_Init(void);
uint8_t __stm32Led3_Write(uint8_t pinState);

const PeriphGpioInterface stm32Led1Interface = {
		.init = __stm32Led1_Init,
		.write = __stm32Led1_Write};

const PeriphGpioInterface stm32Led2Interface = {
		.init = __stm32Led2_Init,
		.write = __stm32Led2_Write};

const PeriphGpioInterface stm32Led3Interface = {
		.init = __stm32Led3_Init,
		.write = __stm32Led3_Write};

/**********************************************************************************************/

uint8_t __stm32Led1_Init(void)
{}

uint8_t __stm32Led1_Write(uint8_t pinState)
{
	__GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, pinState);
}

uint8_t __stm32Led2_Init(void)
{}

uint8_t __stm32Led2_Write(uint8_t pinState)
{
	__GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, pinState);
}

uint8_t __stm32Led3_Init(void)
{}

uint8_t __stm32Led3_Write(uint8_t pinState)
{
	__GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, pinState);
}