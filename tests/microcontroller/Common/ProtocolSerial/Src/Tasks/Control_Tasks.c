#include "main.h"
#include "stm32_rcout.h"

/**********************************************************************************************/

void ControlTask(void *param)
{
	traceLabel traceUserEvent1 = xTraceOpenLabel("ControlEvent1");
	QueueHandle_t ControlQueue = queue_control;
	Control_t Control;
	PeriphRCOutInterface *RCOut;

	/* Configure peripheral */
	xSemaphoreTake(mutex_hal, portMAX_DELAY);
	int32_t rcoutFd = Hal_RegisterInterface(HAL_PERIPH_RCOUT, &stm32RCOutInterface);
	xSemaphoreGive(mutex_hal);

	/* Configure device */
	RCOut = Hal_GetInterface(rcoutFd);
	RCOut->init();
	while(1)
	{
		xSemaphoreTake(semap_ctrl, portMAX_DELAY);

		iscom_message_t Message;
		BaseType_t errQueue = xQueueReceive(ControlQueue, &Message, (TickType_t)0);
		if(errQueue)
		{
			memcpy(&Control, ISCOM_GetContent(&Message), ISCOM_GetContentSize(&Message));
			vTraceUserEvent(traceUserEvent1);

			RCOut->write(0, Control.data[0].elem[0]);
			RCOut->write(1, Control.data[0].elem[1]);
			RCOut->write(2, Control.data[0].elem[2]);
			RCOut->write(3, Control.data[0].elem[3]);
		}
	}
}