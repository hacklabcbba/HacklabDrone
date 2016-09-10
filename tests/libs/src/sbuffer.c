#include "sbuffer.h"
#include <stdlib.h>
#include <string.h>

/**********************************************************************************************/

#define CBUFFER_SIZE (sizeof(struct _sbuffer_t))

/**********************************************************************************************/

struct _sbuffer_t
{
	size_t Idx;
	size_t Num;
	size_t Size;
	void *Buffer;
};

/**********************************************************************************************/

sbuffer_t SBuffer_Create(size_t Num, size_t Size)
{
	sbuffer_t SBuffer = SBuffer_Malloc(CBUFFER_SIZE);
	if(SBuffer == NULL)
		return NULL;

	SBuffer->Buffer = SBuffer_Malloc(Num * Size);
	if(SBuffer->Buffer == NULL)
	{
		SBuffer_Free(SBuffer);
		return NULL;
	}

	SBuffer->Num = Num;
	SBuffer->Size = Size;
	SBuffer->Idx = 0;

	return SBuffer;
}

void SBuffer_Destroy(sbuffer_t SBuffer)
{
	if(SBuffer != NULL)
	{
		if(SBuffer->Buffer != NULL)
		{
			SBuffer_Free(SBuffer->Buffer);
		}
		SBuffer_Free(SBuffer);
	}
}

/**********************************************************************************************/

void *SBuffer_Next(sbuffer_t SBuffer)
{
	if((SBuffer->Buffer != NULL) && (SBuffer->Num > 0) && (SBuffer->Size > 0))
	{
		SBuffer->Idx = (SBuffer->Idx + 1) % SBuffer->Num;
		return SBuffer_GetMem(SBuffer);
	}
	else
	{
		SBuffer->Idx = 0;
		return NULL;
	}
}

inline void *SBuffer_GetMem(sbuffer_t SBuffer)
{
	if(SBuffer->Buffer != NULL)
		return (void *)((uint8_t(*)[SBuffer->Size])SBuffer->Buffer)[SBuffer->Idx];
	else
		return NULL;
}

void *SBuffer_SetMem(sbuffer_t SBuffer, void *Data)
{
	if(SBuffer->Buffer != NULL)
	{
		void *pBuffer = SBuffer_GetMem(SBuffer);
		memcpy(pBuffer, Data, SBuffer->Size);
		return pBuffer;
	}
	else
		return NULL;
}

/**********************************************************************************************/

inline size_t SBuffer_GetSize(sbuffer_t SBuffer)
{
	return SBuffer->Size;
}

inline size_t SBuffer_GetNum(sbuffer_t SBuffer)
{
	return SBuffer->Num;
}

/**********************************************************************************************/

__attribute__((weak)) void *SBuffer_Malloc(size_t Size)
{
	return malloc(Size);
}

__attribute__((weak)) void SBuffer_Free(void *Memory)
{
	free(Memory);
}