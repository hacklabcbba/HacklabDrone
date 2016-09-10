#include "hmc5983.h"

void HMC5983_Read(HMC5983_DeviceTypedef *device, uint8_t ReadAddr, uint8_t *pBuffer, uint16_t NumByteToRead)
{
	if(NumByteToRead > 1)
		ReadAddr = ReadAddr | HMC5983_MULTI_FLAG;

	device->gpioInterface->write(0x01);
	/* Send the Address of the indexed register */
	device->spiInterface->transaction(HMC5983_READ_FLAG | ReadAddr);

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

uint8_t HMC5983_Write(HMC5983_DeviceTypedef *device, uint8_t WriteAddr, uint8_t Data)
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

void HMC5983_Init(HMC5983_DeviceTypedef *device, HMC5983_InitTypeDef *HMC5983_InitStruct)
{
	/* Initialize interface */
	device->spiInterface->init();
	device->gpioInterface->init();

	uint8_t confRegA = 0x00, confRegB = 0x00, confMode = 0x00;

	/* Configure MEMS: temp, average samples and Data rate */
	confRegA |= (uint8_t)(HMC5983_InitStruct->Temperature_Sensor | HMC5983_InitStruct->Average_Sample | HMC5983_InitStruct->Output_DataRate);

	/* Configure MEMS: full Scale */
	confRegB |= (uint8_t)(HMC5983_InitStruct->Full_Scale);

	/* Configure MEMS: working mode */
	confMode |= (uint8_t)(HMC5983_InitStruct->Working_Mode);

	/* Write value to MEMS CRA register */
	/* Write value to MEMS CRB register */
	/* Write value to MEMS CRB register */
	HMC5983_Write(device, HMC5983_CRA, confRegA);
	HMC5983_Write(device, HMC5983_CRB, confRegB);
	HMC5983_Write(device, HMC5983_MR, confMode);
}

/**********************************************************************************************/

void HMC5983_MagnetRead(HMC5983_DeviceTypedef *device, uint32_t *data)
{
	uint8_t buffer[6] = {0};
	int16_t RawData[3] = {0};
	uint8_t i;

	/* Read the register content */
	HMC5983_Read(device, HMC5983_DXRA, buffer, 6);

	for(i = 0; i < 3; i++)
	{
		RawData[i] = (int16_t)(((uint16_t)buffer[2 * i] << 8) | buffer[2 * i + 1]);
	}

	data[0] = (uint32_t)RawData[0];
	data[1] = (uint32_t)RawData[2];
	data[2] = (uint32_t)RawData[1];
}

void HMC5983_TempRead(HMC5983_DeviceTypedef *device, uint32_t *data)
{
	uint8_t buffer[2] = {0};
	int16_t RawData[1] = {0};

	/* Read the register content */
	HMC5983_Read(device, HMC5983_TEMPH, buffer, 2);

	RawData[0] = (int16_t)(((uint16_t)buffer[0] << 8) | buffer[1]);
	data[0] = (uint32_t)RawData[0];
}

/**********************************************************************************************/

uint8_t HMC5983_GetDataStatus(HMC5983_DeviceTypedef *device)
{
	uint8_t tmpreg;

	/* Read STATUS_REG register */
	HMC5983_Read(device, HMC5983_SR, &tmpreg, 1);

	return tmpreg;
}

void HMC5983_GetMagnetGain(HMC5983_DeviceTypedef *device, float *Gain)
{
	uint8_t confRegB;
	float SensitivityXYZ;

	/* Read value to MEMS CRB register */
	HMC5983_Read(device, HMC5983_CRB, &confRegB, 1);

	switch(confRegB & 0xE0)
	{
	case HMC5983_FS_0_88_GA:
	{
		SensitivityXYZ = (float)1.0f / HMC5983_SENSITIVITY_0_88Ga;
	}
	break;
	case HMC5983_FS_1_3_GA:
	{
		SensitivityXYZ = (float)1.0f / HMC5983_SENSITIVITY_1_3Ga;
	}
	break;
	case HMC5983_FS_1_9_GA:
	{
		SensitivityXYZ = (float)1.0f / HMC5983_SENSITIVITY_1_9Ga;
	}
	break;
	case HMC5983_FS_2_5_GA:
	{
		SensitivityXYZ = (float)1.0f / HMC5983_SENSITIVITY_2_5Ga;
	}
	break;
	case HMC5983_FS_4_0_GA:
	{
		SensitivityXYZ = (float)1.0f / HMC5983_SENSITIVITY_4_0Ga;
	}
	break;
	case HMC5983_FS_4_7_GA:
	{
		SensitivityXYZ = (float)1.0f / HMC5983_SENSITIVITY_4_7Ga;
	}
	break;
	case HMC5983_FS_5_6_GA:
	{
		SensitivityXYZ = (float)1.0f / HMC5983_SENSITIVITY_5_6Ga;
	}
	break;
	case HMC5983_FS_8_1_GA:
	{
		SensitivityXYZ = (float)1.0f / HMC5983_SENSITIVITY_8_1Ga;
	}
	break;
	}

	*Gain = SensitivityXYZ * 1000.0f;
}