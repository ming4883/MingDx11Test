#ifndef __XPRENDER_BUFFER_H__
#define __XPRENDER_BUFFER_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum XprBufferType
{
	XprBufferType_Vertex,	//!< vertex buffer
	XprBufferType_Index,	//!< 16-bit index buffer
	XprBufferType_Index8,	//!< 8-bit index buffer

#if !defined(XPR_GLES_2)
	XprBufferType_Index32,	//!< 32-bit index buffer
	XprBufferType_Uniform,	//!< shader uniform buffer
#endif

} XprBufferType;

typedef enum XprBufferFlag
{
	XprBuffer_Inited = 0x0001,
	XprBuffer_Mapped = 0x0002,
} XprBufferFlag;

typedef enum XprBufferMapAccess
{
	XprBufferMapAccess_Read,
	XprBufferMapAccess_Write,
	XprBufferMapAccess_ReadWrite,
} XprBufferMapAccess;

typedef struct XprBuffer
{
	size_t flags;	// combinations of XprBufferFlag
	XprBufferType type;
	size_t sizeInBytes;

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