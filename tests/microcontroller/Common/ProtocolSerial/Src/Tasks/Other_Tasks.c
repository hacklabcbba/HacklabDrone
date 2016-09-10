#include "main.h"
#include "stm32_gpio.h"

/**********************************************************************************************/

void BlinkTask(void *param)
{
	PeriphGpioInterface *LedInt;
	volatile uint32_t LedVal;

	/* Configure peripheral */
	xSemaphoreTake(mutex_hal, portMAX_DELAY);
	int32_t Led2Fd = Hal_RegisterInterface(HAL_PERIPH_GPIO, &stm32LD2Interface);
	xSemaphoreGive(mutex_hal);

	/* Configure device */
	LedInt = Hal_GetInterface(Led2Fd);
	LedInt->init();
	while(1)
	{
		LedVal = !LedVal;
		LedInt->write(LedVal);
		uint32_t taskPeriod = (linkStatus) ? 50 : 500;
		vTaskDelay(OS_MS_TO_SYSTICKS(taskPeriod));
	}
}