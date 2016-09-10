#include "main.h"

/* Global variables ---------------------------------------------------------*/

TaskHandle_t xSensorManagerTask;
TaskHandle_t xCOMMManagerTask;
TaskHandle_t xControlTask;
QueueHandle_t queue_sensors;
QueueHandle_t queue_control;
SemaphoreHandle_t mutex_hal;
SemaphoreHandle_t mutex_spi;
SemaphoreHandle_t semap_ctrl;
slink_message_t SLink_MessageTX;
slink_message_t SLink_MessageRX;
sbuffer_t circBuffer_Control;
sbuffer_t circBuffer_SensorGyro;
sbuffer_t circBuffer_SensorAccel;
sbuffer_t circBuffer_SensorMagnet;
timestamp_t localTime;
timestamp_t remoteTime;
volatile uint32_t linkStatus;

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void SystemInitAll(void);

void SensorManagerTask(void *param);
void COMMManagerTask(void *param);
void ControlTask(void *param);
void BlinkTask(void *param);

/* Private function prototypes -----------------------------------------------*/
void main(void)
{
	SystemInitAll();

	//Initialize kernel & trace
	vTraceInitTraceData();

	//Trace recorder start
	uiTraceStart();

	//Create SLink Messages
	SLink_MessageTX = SLink_Create(SLINK_MAX_PAYLOAD_SIZE);
	SLink_MessageRX = SLink_Create(SLINK_MAX_PAYLOAD_SIZE);

	//Test SLink Messages
	configASSERT(SLink_MessageTX != NULL);
	configASSERT(SLink_MessageRX != NULL);

	//Create Circular Buffers
	circBuffer_Control = SBuffer_Create(CONTROL_BUFFER_SIZE, CONTROL_SIZE);
	circBuffer_SensorGyro = SBuffer_Create(SENSOR_BUFFER_SIZE, SENSORGYRO_SIZE);
	circBuffer_SensorAccel = SBuffer_Create(SENSOR_BUFFER_SIZE, SENSORACCEL_SIZE);
	circBuffer_SensorMagnet = SBuffer_Create(SENSOR_BUFFER_SIZE, SENSORMAGNET_SIZE);

	//Test Circular Buffers
	configASSERT(circBuffer_Control != NULL);
	configASSERT(circBuffer_SensorGyro != NULL);
	configASSERT(circBuffer_SensorAccel != NULL);
	configASSERT(circBuffer_SensorMagnet != NULL);

	//Create Queues
	queue_sensors = xQueueCreate(SENSOR_BUFFER_SIZE, ISCOM_MESSAGE_SIZE);
	queue_control = xQueueCreate(CONTROL_BUFFER_SIZE, ISCOM_MESSAGE_SIZE);

	//Test Queues
	configASSERT(queue_sensors != NULL);
	configASSERT(queue_control != NULL);

	//Create Semaphores
	semap_ctrl = xSemaphoreCreateBinary();

	//Test Semaphores
	configASSERT(semap_ctrl != NULL);
	xSemaphoreTake(semap_ctrl, 0);

	//Create Mutexes
	mutex_hal = xSemaphoreCreateMutex();
	mutex_spi = xSemaphoreCreateMutex();

	//Test Mutexes
	configASSERT(mutex_hal != NULL);
	configASSERT(mutex_spi != NULL);

	//Create Tasks
	xTaskCreate(SensorManagerTask, "SensorManager", 500, NULL, configMAX_PRIORITIES - 1, &xSensorManagerTask);
	xTaskCreate(COMMManagerTask, "COMMManager", 1000, NULL, configMAX_PRIORITIES - 2, &xCOMMManagerTask);
	xTaskCreate(ControlTask, "ControlTask", 500, NULL, configMAX_PRIORITIES - 5, &xControlTask);
	xTaskCreate(BlinkTask, "Blink", 500, NULL, tskIDLE_PRIORITY, NULL);
	vTaskStartScheduler();
	while(1)
		;
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}

#endif