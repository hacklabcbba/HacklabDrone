#include "mpu9250.h"

void MPU9250_Read(MPU9250_DeviceTypedef *device, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t NumByteToRead)
{
	device->gpioInterface->write(0x01);

	/* Send the Address of the indexed register */
	device->spiInterface->transaction(MPU9050_READ_FLAG | ReadAddr);

	/* Receive the data that will be read from the device (MSB First) */
	while(NumByteToRead > 0x00)
	{
		/* Send dummy byte (0x00) to generate the SPI clock */
		*pBuffer = device->spiInterface->transaction(0x00);
		NumByteToRead--;
		pBuffer++;
	}

	device->gpioInterface->write(0x00);
}

uint8_t MPU9250_Write(MPU9250_DeviceTypedef *device, uint8_t WriteAddr, uint8_t Data)
{
	uint8_t DataOut;

	device->gpioInterface->write(0x01);

	/* Send the Address of the indexed register */
	device->spiInterface->transaction(WriteAddr);

	/* Send the data that will be written into the device (MSB First) */
	DataOut = device->spiInterface->transaction(Data);

	device->gpioInterface->write(0x00);

	return DataOut;
}

/**********************************************************************************************/

void MPU9250_Reset(MPU9250_DeviceTypedef *device)
{
	MPU9250_Write(device, MPU9250_PWR_MGMT_1, MPU9250_H_RESET);
}

void MPU9250_Init(MPU9250_DeviceTypedef *device, MPU9250_InitTypeDef *MPU9250_InitStruct)
{
	/* Initialize interface */
	device->spiInterface->init();
	device->gpioInterface->init();
}

void MPU9250_AccelInit(MPU9250_DeviceTypedef *device, MPU9250Accel_InitTypeDef *MPU9250Accel_InitStruct)
{
	uint8_t regPwrMgmt2 = 0x00;
	uint8_t regAccelConf = 0x00, regAccelConf2 = 0x00;
	uint8_t tmp;

	/* Configure MEMS: Enable axis */
	MPU9250_Read(device, MPU9250_PWR_MGMT_2, &regPwrMgmt2, 1);
	tmp = regPwrMgmt2 & 0x07;
	regPwrMgmt2 = ((~MPU9250Accel_InitStruct->Enable_Axis) & 0x38) | tmp;

	/* Configure MEMS: full Scale */
	regAccelConf |= (uint8_t)(MPU9250Accel_InitStruct->Full_Scale);

	/* Configure MEMS: temp, average samples and Data rate */
	regAccelConf2 |= (uint8_t)(MPU9250Accel_InitStruct->DLPF_Config);

	/* Write value to MEMS PWR_MGMT_2 register */
	/* Write value to MEMS ACCEL_CONFIG register */
	/* Write value to MEMS ACCEL_CONFIG2 register */
	MPU9250_Write(device, MPU9250_PWR_MGMT_2, regPwrMgmt2);
	MPU9250_Write(device, MPU9250_ACCEL_CONFIG, regAccelConf);
	MPU9250_Write(device, MPU9250_ACCEL_CONFIG2, regAccelConf2);
}

void MPU9250_GyroInit(MPU9250_DeviceTypedef *device, MPU9250Gyro_InitTypeDef *MPU9250Gyro_InitStruct)
{
	uint8_t regPwrMgmt2 = 0x00;
	uint8_t regConfig = 0x00;
	uint8_t regGyroConf = 0x00;
	uint8_t tmp;

	/* Configure MEMS: Enable axis */
	MPU9250_Read(device, MPU9250_PWR_MGMT_2, &regPwrMgmt2, 1);
	tmp = regPwrMgmt2 & 0x38;
	regPwrMgmt2 = ((~MPU9250Gyro_InitStruct->Enable_Axis) & 0x07) | tmp;

	/* Configure MEMS: Fifo, DLPF */
	regConfig |= (uint8_t)(MPU9250Gyro_InitStruct->DLPF_Config);

	/* Configure MEMS: full Scale */
	regGyroConf |= (uint8_t)(MPU9250Gyro_InitStruct->Full_Scale);

	/* Write value to MEMS PWR_MGMT_2 register */
	/* Write value to MEMS CONFIG register */
	/* Write value to MEMS GYRO_CONFIG register */
	MPU9250_Write(device, MPU9250_PWR_MGMT_2, regPwrMgmt2);
	MPU9250_Write(device, MPU9250_CONFIG, regConfig);
	MPU9250_Write(device, MPU9250_GYRO_CONFIG, regGyroConf);
}

/**********************************************************************************************/

void MPU9250_TempRead(MPU9250_DeviceTypedef *device, uint32_t *data)
{
	uint8_t buffer[2] = {0};
	int16_t RawData[1] = {0};

	/* Read the register content */
	MPU9250_Read(device, MPU9250_TEMP_OUT_H, buffer, 2);

	RawData[0] = (int16_t)(((uint16_t)buffer[0] << 8) | buffer[1]);
	data[0] = (uint32_t)RawData[0];
}

void MPU9250_AccelRead(MPU9250_DeviceTypedef *device, uint32_t *data)
{
	uint8_t buffer[6] = {0};
	int16_t RawData[3] = {0};
	uint8_t i;

	/* Read the register content */
	MPU9250_Read(device, MPU9250_ACCEL_XOUT_H, buffer, 6);

	for(i = 0; i < 3; i++)
	{
		RawData[i] = (int16_t)(((uint16_t)buffer[2 * i] << 8) | buffer[2 * i + 1]);
		data[i] = (uint32_t)RawData[i];
	}
}

void MPU9250_GyroRead(MPU9250_DeviceTypedef *device, uint32_t *data)
{
	uint8_t buffer[6] = {0};
	int16_t RawData[3] = {0};
	uint8_t i;

	/* Read the register content */
	MPU9250_Read(device, MPU9250_GYRO_XOUT_H, buffer, 6);

	for(i = 0; i < 3; i++)
	{
		RawData[i] = (int16_t)(((uint16_t)buffer[2 * i] << 8) | buffer[2 * i + 1]);
		data[i] = (uint32_t)RawData[i];
	}
}

void MPU9250_MagnetRead(MPU9250_DeviceTypedef *device, uint32_t *data)
{
	uint8_t buffer[6] = {0};
	int16_t RawData[3] = {0};
	uint8_t i;

	/* Read the register content */
	MPU9250_Read(device, MPU9250_EXT_SENS_DATA_00, buffer, 6);

	for(i = 0; i < 3; i++)
	{
		RawData[i] = (int16_t)(((uint16_t)buffer[2 * i + 1] << 8) | buffer[2 * i]);
		data[i] = (uint32_t)RawData[i];
	}
}

/**********************************************************************************************/

void MPU9250_GetAccelGain(MPU9250_DeviceTypedef *device, float *Gain)
{
	uint8_t regAccelConf;
	float SensitivityXYZ;

	/* Read value from MEMS ACCEL_CONFIG register */
	MPU9250_Read(device, MPU9250_ACCEL_CONFIG, &regAccelConf, 1);

	switch(regAccelConf & 0x18)
	{
	case MPU9250_ACCEL_FS_2G:
	{
		SensitivityXYZ = (float)1.0f / MPU9250_ACCEL_SENSITIVITY_2G;
	}
	break;
	case MPU9250_ACCEL_FS_4G:
	{
		SensitivityXYZ = (float)1.0f / MPU9250_ACCEL_SENSITIVITY_4G;
	}
	break;
	case MPU9250_ACCEL_FS_8G:
	{
		SensitivityXYZ = (float)1.0f / MPU9250_ACCEL_SENSITIVITY_8G;
	}
	break;
	case MPU9250_ACCEL_FS_16G:
	{
		SensitivityXYZ = (float)1.0f / MPU9250_ACCEL_SENSITIVITY_16G;
	}
	break;
	}

	*Gain = SensitivityXYZ * 9.81f;
}

void MPU9250_GetGyroGain(MPU9250_DeviceTypedef *device, float *Gain)
{
	uint8_t regGyroConf;
	float SensitivityXYZ;

	/* Read value from MEMS GYRO_CONFIG register */
	MPU9250_Read(device, MPU9250_GYRO_CONFIG, &regGyroConf, 1);

	switch(regGyroConf & 0x18)
	{
	case MPU9250_GYRO_FS_250:
	{
		SensitivityXYZ = (float)1.0f / MPU9250_GYRO_SENSITIVITY_250;
	}
	break;
	case MPU9250_GYRO_FS_500:
	{
		SensitivityXYZ = (float)1.0f / MPU9250_GYRO_SENSITIVITY_500;
	}
	break;
	case MPU9250_GYRO_FS_1000:
	{
		SensitivityXYZ = (float)1.0f / MPU9250_GYRO_SENSITIVITY_1000;
	}
	break;
	case MPU9250_GYRO_FS_2000:
	{
		SensitivityXYZ = (float)1.0f / MPU9250_GYRO_SENSITIVITY_2000;
	}
	break;
	}

	*Gain = SensitivityXYZ;
}