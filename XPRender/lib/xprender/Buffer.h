#ifndef __XPRENDER_BUFFER_H__
#define __XPRENDER_BUFFER_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum XprBufferType
{
	XprBufferType_Vertex,
	XprBufferType_Index,
	XprBufferType_Uniform,
} XprBufferType;

typedef enum XprBufferFlag
{
	XprBufferFlag_Inited = 1 << 0,
	XprBufferFlag_Mapped = 1 << 1,
} XprBufferFlag;

typedef enum XprBufferMapAccess
{
	XprBufferMapAccess_Read,
	XprBufferMapAccess_Write,
	XprBufferMapAccess_ReadWrite,
} XprBufferMapAccess;

typedef struct XprBuffer
{
	int name;
	XprBufferType type;
	size_t sizeInBytes;
	size_t flags;	// combinations of XprBufferFlag

} XprBuffer;

XprBuffer* XprBuffer_alloc();

void XprBuffer_free(XprBuffer* self);

void XprBuffer_init(XprBuffer* self, XprBufferType type, size_t sizeInBytes, void* initialData);

void XprBuffer_update(XprBuffer* self, size_t offsetInBytes, size_t sizeInBytes, void* data);

void* XprBuffer_map(XprBuffer* self, XprBufferMapAccess usage);

void XprBuffer_unmap(XprBuffer* self);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_BUFFER_H__