#ifndef __XPRENDER_BUFFER_H__
#define __XPRENDER_BUFFER_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum xprBufferType
{
	xprBufferType_Vertex,
	xprBufferType_Index,
	xprBufferType_Uniform,
} xprBufferType;

typedef enum xprBufferFlag
{
	xprBufferFlag_Mapped = 1 << 1,
} xprBufferFlag;

typedef enum xprBufferMapAccess
{
	xprBufferMapAccess_Read,
	xprBufferMapAccess_Write,
	xprBufferMapAccess_ReadWrite,
} xprBufferMapAccess;

typedef struct xprBuffer
{
	int name;
	xprBufferType type;
	size_t sizeInBytes;
	size_t flags;	// combinations of xprBufferFlag

} xprBuffer;

xprBuffer* xprBuffer_new(xprBufferType type, size_t sizeInBytes, void* initialData);

void xprBuffer_free(xprBuffer* self);

void xprBuffer_update(xprBuffer* self, size_t offsetInBytes, size_t sizeInBytes, void* data);

void* xprBuffer_map(xprBuffer* self, xprBufferMapAccess usage);

void xprBuffer_unmap(xprBuffer* self);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_BUFFER_H__