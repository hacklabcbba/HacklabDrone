#include "main.h"
#include "stm32_adc.h"

#include "crossworksdebug_spi.h"
#include "crossworksdebug_gpio.h"
#include "dummy_gpio.h"
#include "dummy_spi.h"
#include "dummy_adc.h"

#include "hmc5983.h"
#include "mpu9250.h"
#include "mma7361.h"

#define SensorAccelTask SensorAccel2Task

/**********************************************************************************************/

void SensorGyroTask(void *param);
void SensorAccelTask(void *param);
void SensorMagnetTask(void *param);

/**********************************************************************************************/

TaskHandle_t xSensorGyroTask;
TaskHandle_t xSensorAccelTask;
TaskHandle_t xSensorMagnetTask;
SemaphoreHandle_t semap_gyro;
SemaphoreHandle_t semap_accel;
SemaphoreHandle_t semap_magnet;

/**********************************************************************************************/

void SensorManagerTask(void *param)
{
	/* Create semaphores */
	semap_gyro = xSemaphoreCreateBinary();
	semap_accel = xSemaphoreCreateBinary();
	semap_magnet = xSemaphoreCreateBinary();

	/* Test semaphores */
	configASSERT(semap_gyro != NULL);
	configASSERT(semap_accel != NULL);
	configASSERT(semap_magnet != NULL);

	/* Take semaphores */
	xSemaphoreTake(semap_gyro, 0);
	xSemaphoreTake(semap_accel, 0);
	xSemaphoreTake(semap_magnet, 0);

	/* Create tasks */
	xTaskCreate(SensorGyroTask, "SensorGyro", 500, NULL, 5, &xSensorGyroTask);
	xTaskCreate(SensorAccelTask, "SensorAccel", 500, NULL, 5, &xSensorAccelTask);
	xTaskCreate(SensorMagnetTask, "SensorMagnet", 500, NULL, 5, &xSensorMagnetTask);

	/* Configure task period */
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = OS_MS_TO_SYSTICKS(SENSOR_TIME_PERIOD);
	xLastWakeTime = xTaskGetTickCount();
	while(1)
	{
		vTaskDelayUntil(&xLastWakeTime, xPeriod);

		/* Read time */
		uint64_t Timestamp = GTIME_GetCounter();
		localTime.seconds = Timestamp / (uint32_t)1E6;
		localTime.fraction = Timestamp % (uint32_t)1E6;

		/* Semaphores control */
		xSemaphoreGive(semap_gyro);
		xSemaphoreGive(semap_accel);
		xSemaphoreGive(semap_magnet);
	}
}

/**********************************************************************************************/

void SensorGyroTask(void *param)
{
	traceLabel traceUserEvent1 = xTraceOpenLabel("GyroEvent1");

	SensorGyro_t Sensor;
	sbuffer_t SensorBuffer = circBuffer_SensorGyro;
	const size_t SensorSize = SBuffer_GetSize(SensorBuffer);
	QueueHandle_t SensorQueue = queue_sensors;
	iscom_message_t SensorMsg;

	MPU9250_DeviceTypedef mpu9250Device;
	MPU9250Gyro_InitTypeDef mpu9250GyroInitStruct;

	/* Configure peripheral */
	xSemaphoreTake(mutex_hal, portMAX_DELAY);
	int32_t spiFd = Hal_RegisterInterface(HAL_PERIPH_SPI, &dummySpiDriver);
	int32_t gpioFd = Hal_RegisterInterface(HAL_PERIPH_GPIO, &dummyGpioDriver);
	xSemaphoreGive(mutex_hal);

	/* Configure device */
	mpu9250Device.spiInterface = Hal_GetInterface(spiFd);
	mpu9250Device.gpioInterface = Hal_GetInterface(gpioFd);

	xSemaphoreTake(mutex_spi, portMAX_DELAY);
	MPU9250_Init(&mpu9250Device, NULL);
	MPU9250_GyroInit(&mpu9250Device, &mpu9250GyroInitStruct);
	xSemaphoreGive(mutex_spi);

	ISCOM_SetIdentifier(&SensorMsg, 0x01, 0x1A);
	while(1)
	{
		xSemaphoreTake(semap_gyro, portMAX_DELAY);

		/* Read time */
		uint64_t Timestamp = GTIME_GetCounter();
		Sensor.timestamp.seconds = Timestamp / (uint32_t)1E6;
		Sensor.timestamp.fraction = Timestamp % (uint32_t)1E6;

		/* Read sensor */
		xSemaphoreTake(mutex_spi, portMAX_DELAY);
		MPU9250_GyroRead(&mpu9250Device, Sensor.data[0].elem);
		xSemaphoreGive(mutex_spi);

		/* Send message */
		vTraceUserEvent(traceUserEvent1);
		void *pSensor = SBuffer_Next(SensorBuffer);
		SBuffer_SetMem(SensorBuffer, &Sensor);
		ISCOM_SetContent(&SensorMsg, pSensor, SensorSize);
		xQueueSend(SensorQueue, (void *)&SensorMsg, (TickType_t)0);
	}
}

/**********************************************************************************************/

void SensorAccel1Task(void *param)
{
	traceLabel traceUserEvent1 = xTraceOpenLabel("AccelEvent1");

	SensorAccel_t Sensor;
	sbuffer_t SensorBuffer = circBuffer_SensorAccel;
	const size_t SensorSize = SBuffer_GetSize(SensorBuffer);
	QueueHandle_t SensorQueue = queue_sensors;
	iscom_message_t SensorMsg;

	MPU9250_DeviceTypedef mpu9250Device;
	MPU9250Accel_InitTypeDef mpu9250AccelInitStruct;

	/* Configure peripheral */
	xSemaphoreTake(mutex_hal, portMAX_DELAY);
	int32_t spiFd = Hal_RegisterInterface(HAL_PERIPH_SPI, &dummySpiDriver);
	int32_t gpioFd = Hal_RegisterInterface(HAL_PERIPH_GPIO, &dummyGpioDriver);
	xSemaphoreGive(mutex_hal);

	/* Configure device */
	mpu9250Device.spiInterface = Hal_GetInterface(spiFd);
	mpu9250Device.gpioInterface = Hal_GetInterface(gpioFd);

	xSemaphoreTake(mutex_spi, portMAX_DELAY);
	MPU9250_Init(&mpu9250Device, NULL);
	MPU9250_AccelInit(&mpu9250Device, &mpu9250AccelInitStruct);
	xSemaphoreGive(mutex_spi);

	ISCOM_SetIdentifier(&SensorMsg, 0x01, 0x1B);
	while(1)
	{
		xSemaphoreTake(semap_accel, portMAX_DELAY);

		/* Read time */
		uint64_t Timestamp = GTIME_GetCounter();
		Sensor.timestamp.seconds = Timestamp / (uint32_t)1E6;
		Sensor.timestamp.fraction = Timestamp % (uint32_t)1E6;

		/* Read sensor */
		xSemaphoreTake(mutex_spi, portMAX_DELAY);
		MPU9250_AccelRead(&mpu9250Device, Sensor.data[0].elem);
		xSemaphoreGive(mutex_spi);

		/* Send message */
		for(uint32_t i = 0; i < 8; i++)
		{
			vTraceUserEvent(traceUserEvent1);
			void *pSensor = SBuffer_Next(SensorBuffer);
			SBuffer_SetMem(SensorBuffer, &Sensor);
			ISCOM_SetContent(&SensorMsg, pSensor, SensorSize);
			xQueueSend(SensorQueue, (void *)&SensorMsg, (TickType_t)0);
		}
	}
}

void SensorAccel2Task(void *param)
{
	traceLabel traceUserEvent1 = xTraceOpenLabel("AccelEvent1");

	SensorAccel_t Sensor;
	sbuffer_t SensorBuffer = circBuffer_SensorAccel;
	const size_t SensorSize = SBuffer_GetSize(SensorBuffer);
	QueueHandle_t SensorQueue = queue_sensors;
	iscom_message_t SensorMsg;

	MMA7361_DeviceTypedef mma7361Device;

	/* Configure peripheral */
	xSemaphoreTake(mutex_hal, portMAX_DELAY);
	int32_t adcFd1 = Hal_RegisterInterface(HAL_PERIPH_ADC, &stm32Adc5Interface);
	int32_t adcFd2 = Hal_RegisterInterface(HAL_PERIPH_ADC, &stm32Adc4Interface);
	int32_t adcFd3 = Hal_RegisterInterface(HAL_PERIPH_ADC, &stm32Adc3Interface);
	xSemaphoreGive(mutex_hal);

	/* Configure device */
	mma7361Device.interfaceAdcX = Hal_GetInterface(adcFd1);
	mma7361Device.interfaceAdcY = Hal_GetInterface(adcFd2);
	mma7361Device.interfaceAdcZ = Hal_GetInterface(adcFd3);
	MMA7361_Init(&mma7361Device);

	ISCOM_SetIdentifier(&SensorMsg, 0x01, 0x1B);
	while(1)
	{
		xSemaphoreTake(semap_accel, portMAX_DELAY);

		/* Read time */
		uint64_t Timestamp = GTIME_GetCounter();
		Sensor.timestamp.seconds = Timestamp / (uint32_t)1E6;
		Sensor.timestamp.fraction = Timestamp % (uint32_t)1E6;

		/* Read sensor */
		MMA7361_Read(&mma7361Device, Sensor.data[0].elem);

		/* Send message */
		vTraceUserEvent(traceUserEvent1);
		void *pSensor = SBuffer_Next(SensorBuffer);
		SBuffer_SetMem(SensorBuffer, &Sensor);
		ISCOM_SetContent(&SensorMsg, pSensor, SensorSize);
		xQueueSend(SensorQueue, (void *)&SensorMsg, (TickType_t)0);
	}
}

/**********************************************************************************************/

void SensorMagnetTask(void *param)
{
	traceLabel traceUserEvent1 = xTraceOpenLabel("MagnetEvent1");

	SensorMagnet_t Sensor;
	sbuffer_t SensorBuffer = circBuffer_SensorMagnet;
	const size_t SensorSize = SBuffer_GetSize(SensorBuffer);
	QueueHandle_t SensorQueue = queue_sensors;
	iscom_message_t SensorMsg;

	HMC5983_DeviceTypedef hmc5983Device;
	HMC5983_InitTypeDef hmc5983InitStruct;

	/* Configure peripheral */
	xSemaphoreTake(mutex_hal, portMAX_DELAY);
	int32_t spiFd = Hal_RegisterInterface(HAL_PERIPH_SPI, &dummySpiDriver);
	int32_t gpioFd = Hal_RegisterInterface(HAL_PERIPH_GPIO, &dummyGpioDriver);
	xSemaphoreGive(mutex_hal);

	/* Configure device */
	hmc5983Device.spiInterface = Hal_GetInterface(spiFd);
	hmc5983Device.gpioInterface = Hal_GetInterface(gpioFd);

	xSemaphoreTake(mutex_spi, portMAX_DELAY);
	HMC5983_Init(&hmc5983Device, &hmc5983InitStruct);
	xSemaphoreGive(mutex_spi);

	ISCOM_SetIdentifier(&SensorMsg, 0x01, 0x1C);
	while(1)
	{
		xSemaphoreTake(semap_magnet, portMAX_DELAY);

		/* Read time */
		uint64_t Timestamp = GTIME_GetCounter();
		Sensor.timestamp.seconds = Timestamp / (uint32_t)1E6;
		Sensor.timestamp.fraction = Timestamp % (uint32_t)1E6;

		/* Read sensor */
		xSemaphoreTake(mutex_spi, portMAX_DELAY);
		HMC5983_MagnetRead(&hmc5983Device, Sensor.data[0].elem);
		xSemaphoreGive(mutex_spi);

		/* Send message */
		vTraceUserEvent(traceUserEvent1);
		void *pSensor = SBuffer_Next(SensorBuffer);
		SBuffer_SetMem(SensorBuffer, &Sensor);
		ISCOM_SetContent(&SensorMsg, pSensor, SensorSize);
		xQueueSend(SensorQueue, (void *)&SensorMsg, (TickType_t)0);
	}
}

/**********************************************************************************************/