#include "mpu9250_api.h"
#include "hal.h"

/* MPU9250 structs */
typedef struct
{
	uint8_t Clock_Source;											/* Clock source */
	uint8_t Sample_Rate_Div;									/* Sample Rate Divider */
} MPU9250_InitTypeDef;

typedef struct
{
	uint8_t Enable_Interrupt;									/* Interrupts enabled */
	uint8_t Logic_Level;											/* Logic level for INT pin */
	uint8_t Enable_Latch_Int;									/* INT pin level held or pulse */
	uint8_t Status_Clear;											/* Interrupt status is cleared with any read operation*/
} MPU9250Int_InitTypeDef;

typedef struct
{
	uint8_t Full_Scale;												/* Full Scale selection */
	uint8_t Enable_Axis;											/* Accelerometer Enable Axis */
	uint8_t DLPF_Config;											/* Configure DLPF */
} MPU9250Accel_InitTypeDef;

typedef struct
{
	uint8_t Full_Scale;												/* Full Scale selection */
	uint8_t Enable_Axis;											/* Accelerometer Enable Axis */
	uint8_t DLPF_Config;											/* Configure DLPF */
} MPU9250Gyro_InitTypeDef;

typedef struct
{
	uint8_t Full_Resolution;									/* Full Resolution selection */
	uint8_t Working_Mode;											/* Operating mode */
} MPU9250Magnet_InitTypeDef;

typedef struct
{
	PeriphSpiInterface *spiInterface;
	PeriphGpioInterface *gpioInterface;
} MPU9250_DeviceTypedef;

void MPU9250_Read(MPU9250_DeviceTypedef *device, uint8_t ReadAddr, uint8_t* pBuffer, uint16_t NumByteToRead);
uint8_t MPU9250_Write(MPU9250_DeviceTypedef *device, uint8_t WriteAddr, uint8_t Data);

void MPU9250_Reset(MPU9250_DeviceTypedef *device);
void MPU9250_Init(MPU9250_DeviceTypedef *device, MPU9250_InitTypeDef *MPU9250_InitStruct);
void MPU9250_AccelInit(MPU9250_DeviceTypedef *device, MPU9250Accel_InitTypeDef *MPU9250Accel_InitStruct);
void MPU9250_GyroInit(MPU9250_DeviceTypedef *device, MPU9250Gyro_InitTypeDef *MPU9250Gyro_InitStruct);

void MPU9250_TempRead(MPU9250_DeviceTypedef *device, uint32_t *data);
void MPU9250_AccelRead(MPU9250_DeviceTypedef *device, uint32_t *data);
void MPU9250_GyroRead(MPU9250_DeviceTypedef *device, uint32_t *data);
void MPU9250_MagnetRead(MPU9250_DeviceTypedef *device, uint32_t *data);

void MPU9250_GetAccelGain(MPU9250_DeviceTypedef *device, float *Gain);
void MPU9250_GetGyroGain(MPU9250_DeviceTypedef *device, float *Gain);