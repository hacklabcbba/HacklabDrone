#include "hmc5983_api.h"
#include "hal.h"

/* HMC5983 struct */
typedef struct
{
	uint8_t Temperature_Sensor;								/* Temperature sensor enable/disable */
	uint8_t Average_Sample;										/* Select number of samples averaged */
	uint8_t Output_DataRate;									/* OUT data rate */
	uint8_t Working_Mode;											/* Operating mode */
	uint8_t Full_Scale;												/* Full Scale selection */
}HMC5983_InitTypeDef;

typedef struct
{
	PeriphSpiInterface *spiInterface;
	PeriphGpioInterface *gpioInterface;
} HMC5983_DeviceTypedef;

void HMC5983_Read(HMC5983_DeviceTypedef *device, uint8_t ReadAddr, uint8_t* pBuffer, uint16_t NumByteToRead);
uint8_t HMC5983_Write(HMC5983_DeviceTypedef *device, uint8_t WriteAddr, uint8_t Data);

void HMC5983_Init(HMC5983_DeviceTypedef *interfac, HMC5983_InitTypeDef *HMC5983_InitStruct);

void HMC5983_MagnetRead(HMC5983_DeviceTypedef *device, uint32_t* data);
void HMC5983_TempRead(HMC5983_DeviceTypedef *device, uint32_t* data);

uint8_t HMC5983_GetDataStatus(HMC5983_DeviceTypedef *device);
void HMC5983_GetMagnetGain(HMC5983_DeviceTypedef *device, float *Gain);