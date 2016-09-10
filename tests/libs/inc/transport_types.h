#ifndef __TRANSPORT_TYPES_H
#define __TRANSPORT_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**********************************************************************************************/

#pragma pack(push)
#pragma pack(1)

#define TIMESTAMP_SIZE     sizeof(timestamp_t)
#define SENSORGYRO_SIZE    sizeof(SensorGyro_t)
#define SENSORACCEL_SIZE   sizeof(SensorAccel_t)
#define SENSORMAGNET_SIZE  sizeof(SensorMagnet_t)
#define CONTROL_SIZE       sizeof(Control_t)

typedef struct __TIMESTAMP_TYPE timestamp_t;
typedef struct __ELEM3_DATA_TYPE SensorGyro_t;
typedef struct __ELEM3_DATA_TYPE SensorAccel_t;
typedef struct __ELEM3_DATA_TYPE SensorMagnet_t;
typedef struct __ELEM8_DATA_TYPE Control_t;

struct __TIMESTAMP_TYPE
{
	uint32_t seconds;
	uint32_t fraction;
};

struct __ELEM3_DATA_TYPE
{
	timestamp_t timestamp;
	struct
	{
		uint32_t elem[3];
	} data[1];
};

struct __ELEM8_DATA_TYPE
{
	timestamp_t timestamp;
	struct
	{
		uint32_t elem[8];
	} data[1];
};

#pragma pack(pop)

/**********************************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TRANSPORT_TYPES_H */