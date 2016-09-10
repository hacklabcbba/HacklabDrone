#ifndef __GLOBAL_VAR
#define __GLOBAL_VAR

extern TaskHandle_t xSensorManagerTask;
extern TaskHandle_t xCOMMManagerTask;
extern TaskHandle_t xControlTask;
extern QueueHandle_t queue_sensors;
extern QueueHandle_t queue_control;
extern SemaphoreHandle_t mutex_hal;
extern SemaphoreHandle_t mutex_spi;
extern SemaphoreHandle_t semap_ctrl;
extern slink_message_t SLink_MessageTX;
extern slink_message_t SLink_MessageRX;
extern sbuffer_t circBuffer_Control;
extern sbuffer_t circBuffer_SensorGyro;
extern sbuffer_t circBuffer_SensorAccel;
extern sbuffer_t circBuffer_SensorMagnet;
extern timestamp_t localTime;
extern timestamp_t remoteTime;
extern volatile uint32_t linkStatus;

#endif