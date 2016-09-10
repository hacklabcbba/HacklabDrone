#include "hal.h"
#include "stddef.h"

struct __HalInterface
{
	struct
	{
		void *interface;
		PeriphType type;
	} fdTable[HAL_FD_MAX];
	int32_t index;
};

struct __HalInterface HalInterface = {.index = 0};

/**********************************************************************************************/

int32_t Hal_RegisterInterface(PeriphType type, const void *interfaceDriver)
{
	uint8_t isType;
	int32_t newFd = HalInterface.index;

	if((newFd < HAL_FD_MAX) && (newFd >= 0))
	{
		switch(type)
		{
		case HAL_PERIPH_DEBUG:
			isType = 1;
			break;
		case HAL_PERIPH_GPIO:
			isType = 1;
			break;
		case HAL_PERIPH_SPI:
			isType = 1;
			break;
		case HAL_PERIPH_UART:
			isType = 1;
			break;
		case HAL_PERIPH_ADC:
			isType = 1;
			break;
		case HAL_PERIPH_RCOUT:
			isType = 1;
			break;
		default:
			isType = 0;
		}
		if(isType)
		{
			HalInterface.fdTable[newFd].type = type;
			HalInterface.fdTable[newFd].interface = (void *)interfaceDriver;
			HalInterface.index = newFd + 1;
			return newFd;
		}
	}
	else
	{
		HalInterface.index = HAL_FD_MAX;
	}
	return -1;
}

inline void *Hal_GetInterface(int32_t fd)
{
	if((fd < HAL_FD_MAX) && (fd >= 0))
	{
		return HalInterface.fdTable[fd].interface;
	}
	return NULL;
}