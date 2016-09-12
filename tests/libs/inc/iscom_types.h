#ifndef __ISCOM_TYPES_H
#define __ISCOM_TYPES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**********************************************************************************************/

#define ISCOM_HEADER_SIZE  (sizeof(iscom_header_t))
#define ISCOM_MESSAGE_SIZE (sizeof(iscom_message_t))

/**********************************************************************************************/

typedef struct
{
	uint16_t Size;
	struct
	{
		uint8_t Primary;
		uint8_t Secundary;
	} Identifier;
} iscom_header_t;

typedef struct
{
	iscom_header_t Header;
	void *Content;
} iscom_message_t;

/**********************************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SBUFFER_H */