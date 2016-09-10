#ifndef __DEBUG_H
#define __DEBUG_H

#include <stddef.h>
#include "slink.h"
#include "iscom.h"
#include "transport_types.h"

void DEBUG_RawMessagePrint(void *buffer, size_t size);
void DEBUG_SlinkMessagePrint(slink_message_t message);
void DEBUG_IscomPackedMessagePrint(void *buffer, size_t size);

#endif
