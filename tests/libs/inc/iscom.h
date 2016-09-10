#ifndef __ISCOM_H
#define __ISCOM_H

#include "iscom_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**********************************************************************************************/

void ISCOM_DeInit(iscom_message_t *Message);

void ISCOM_SetContent(iscom_message_t *Message, void *Content, size_t ContentSize);
void ISCOM_SetIdentifier(iscom_message_t *Message, uint8_t Primary, uint8_t Secundary);
void *ISCOM_GetContent(iscom_message_t *Message);
void ISCOM_GetIdentifier(iscom_message_t *Message, uint8_t *Primary, uint8_t *Secundary);
size_t ISCOM_GetContentSize(iscom_message_t *Message);
size_t ISCOM_GetPackedSize(iscom_message_t *Message);

size_t ISCOM_Pack(iscom_message_t *Message, void *Buffer);
size_t ISCOM_UnPack(iscom_message_t *Message, void *Buffer);

/**********************************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SBUFFER_H */