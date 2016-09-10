#include "main.h"
#include "stm32_gpio.h"

/**********************************************************************************************/

void BlinkTask(void *param)
{
	PeriphGpioInterface *Led2Int, *Led3Int;
	NetInterface *interface = netGetDefaultInterface();
	volatile uint32_t Led3Val;

	/* Configure peripheral */
	xSemaphoreTake(mutex_hal, portMAX_DELAY);
	int32_t Led2Fd = Hal_RegisterInterface(HAL_PERIPH_GPIO, &stm32LD2Interface);
	int32_t Led3Fd = Hal_RegisterInterface(HAL_PERIPH_GPIO, &stm32LD3Interface);
	xSemaphoreGive(mutex_hal);

	/* Configure device */
	Led2Int = Hal_GetInterface(Led2Fd);
	Led3Int = Hal_GetInterface(Led3Fd);
	Led2Int->init();
	Led3Int->init();
	while(1)
	{
		if(interface->linkState)
			Led2Int->write(1);
		else
			Led2Int->write(0);
		Led3Val = !Led3Val;
		Led3Int->write(Led3Val);
		uint32_t taskPeriod = (linkStatus) ? 50 : 500;
		vTaskDelay(OS_MS_TO_SYSTICKS(taskPeriod));
	}
}

/**********************************************************************************************/

void SNTPTask(void *param)
{
	NetInterface *interface = netGetDefaultInterface();
	while(1)
	{
		if((remoteAddr.ipv4Addr != 0) && (remotePort != 0))
		{
			NtpTimestamp TimeStamp;
			sntpClientGetTimestamp(interface, &remoteAddr, &TimeStamp);
		}
		vTaskDelay(OS_MS_TO_SYSTICKS(5000));
	}
}