#ifndef __SBUFFER_H
#define __SBUFFER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**********************************************************************************************/

typedef struct _sbuffer_t *sbuffer_t;

/**********************************************************************************************/

sbuffer_t SBuffer_Create(size_t Num, size_t Size);
void SBuffer_Destroy(sbuffer_t SBuffer);

void *SBuffer_Next(sbuffer_t SBuffer);
void *SBuffer_GetMem(sbuffer_t SBuffer);
void *SBuffer_SetMem(sbuffer_t SBuffer, void *Data);

size_t SBuffer_GetSize(sbuffer_t SBuffer);
size_t SBuffer_GetNum(sbuffer_t SBuffer);

/**********************************************************************************************/

void *SBuffer_Malloc(size_t Size);
void SBuffer_Free(void *Memory);

/**********************************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SBUFFER_H */