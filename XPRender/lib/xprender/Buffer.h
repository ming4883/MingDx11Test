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
	XprBuffer_Inited = 1 << 0,
	XprBuffer_Mapped = 1 << 1,
} XprBufferFlag;

typedef enum XprBufferMapAccess
{
	XprBufferMapAccess_Read,
	XprBufferMapAccess_Write,
	XprBufferMapAccess_ReadWrite,
} XprBufferMapAccess;

struct XprBufferImpl;

typedef struct XprBuffer
{
	size_t flags;	// combinations of XprBufferFlag
	XprBufferType type;
	size_t sizeInBytes;
	
	struct XprBufferImpl* impl;

} XprBuffer;

XprBuffer* xprBufferAlloc();

void xprBufferFree(XprBuffer* self);

XprBool xprBufferInit(XprBuffer* self, XprBufferType type, size_t sizeInBytes, void* initialData);

void xprBufferUpdate(XprBuffer* self, size_t offsetInBytes, size_t sizeInBytes, void* data);

void* xprBufferMap(XprBuffer* self, XprBufferMapAccess usage);

void xprBufferUnmap(XprBuffer* self);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_BUFFER_H__