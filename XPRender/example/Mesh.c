#include "Mesh.h"

#include "../lib/xprender/Buffer.h"
#include "../lib/xprender/Shader.h"
#include "../lib/xprender/Vec2.h"
#include "../lib/xprender/Vec3.h"
#include "../lib/xprender/Vec4.h"

#include <math.h>
#include <stdio.h>

typedef struct MeshImpl {
	struct XprBuffer* indexBuffer;
	struct XprBuffer* vertexBuffer;
	struct XprBuffer* normalBuffer;
	struct XprBuffer* colorBuffer;
	struct XprBuffer* tcBuffer[MeshTrait_MaxTexcoord];
	int ia;
} MeshImpl;

Mesh* meshAlloc()
{
	Mesh* self;
	XprAllocWithImpl(self, Mesh, MeshImpl);

	return self;
}

void meshInit(Mesh* self, size_t vertexCount, size_t indexCount)
{
	size_t i;

	self->vertexCount = vertexCount;
	self->indexCount = indexCount;

	self->index.sizeInBytes = sizeof(unsigned short) * self->indexCount;
	self->index.buffer = malloc(self->index.sizeInBytes);
	strcpy(self->index.shaderName, "");

	self->vertex.sizeInBytes = sizeof(XprVec3) * self->vertexCount;
	self->vertex.buffer = malloc(self->vertex.sizeInBytes);
	strcpy(self->vertex.shaderName, "i_vertex");

	self->normal.sizeInBytes = sizeof(XprVec3) * self->vertexCount;
	self->normal.buffer = malloc(self->normal.sizeInBytes);
	strcpy(self->normal.shaderName, "i_normal");

	self->color.sizeInBytes = sizeof(XprVec4) * self->vertexCount;
	self->color.buffer = malloc(self->color.sizeInBytes);
	strcpy(self->color.shaderName, "i_color");

	for(i=0; i<MeshTrait_MaxTexcoord; ++i) {
		self->texcoord[i].sizeInBytes = sizeof(XprVec2) * self->vertexCount;
		self->texcoord[i].buffer = malloc(self->texcoord[i].sizeInBytes);
		sprintf(self->texcoord[i].shaderName, "i_texcoord%d", i);
	}

	// hardware buffer
	self->impl->indexBuffer = xprBufferAlloc();
	xprBufferInit(self->impl->indexBuffer, XprBufferType_Index, self->index.sizeInBytes, nullptr);

	self->impl->vertexBuffer = xprBufferAlloc();
	xprBufferInit(self->impl->vertexBuffer, XprBufferType_Vertex, self->vertex.sizeInBytes, nullptr);
	
	self->impl->normalBuffer = xprBufferAlloc();
	xprBufferInit(self->impl->normalBuffer, XprBufferType_Vertex, self->normal.sizeInBytes, nullptr);

	self->impl->colorBuffer = xprBufferAlloc();
	xprBufferInit(self->impl->colorBuffer, XprBufferType_Vertex, self->color.sizeInBytes, nullptr);

	for(i=0; i<MeshTrait_MaxTexcoord; ++i) {
		self->impl->tcBuffer[i] = xprBufferAlloc();
		xprBufferInit(self->impl->tcBuffer[i], XprBufferType_Vertex, self->texcoord[i].sizeInBytes, nullptr);
	}	
	
	self->flags |= MeshFlag_Inited;
}

void meshFree(Mesh* self)
{
	size_t i;

	if(self->flags & MeshFlag_Inited) {

		xprBufferFree(self->impl->indexBuffer);
		xprBufferFree(self->impl->vertexBuffer);
		xprBufferFree(self->impl->normalBuffer);
		xprBufferFree(self->impl->colorBuffer);

		for(i=0; i<MeshTrait_MaxTexcoord; ++i) {
			xprBufferFree(self->impl->tcBuffer[i]);
		}

		free(self->index.buffer);
		free(self->vertex.buffer);
		free(self->normal.buffer);
		free(self->color.buffer);

		for(i=0; i<MeshTrait_MaxTexcoord; ++i) {
			free(self->texcoord[i].buffer);
		}
	}

	free(self);
}

void meshCommit(Mesh* self)
{
	size_t i;

#define commit(x) {\
	xprBufferUpdate(self->impl->x##Buffer, 0, self->x.sizeInBytes, self->x.buffer);\
	}
	if(nullptr == self)
		return;

	if(0 == (self->flags & MeshFlag_Inited))
		return;

	commit(index);
	commit(vertex);
	commit(normal);
	commit(color);
	for(i=0; i<MeshTrait_MaxTexcoord; ++i) {
		xprBufferUpdate(self->impl->tcBuffer[i], 0, self->texcoord[i].sizeInBytes, self->texcoord[i].buffer);
	}	

#undef commit
}

void meshPreRender(Mesh* self, struct XprGpuProgram* program)
{
	XprGpuProgramInput inputs[] = {
		{self->impl->indexBuffer},
		{self->impl->vertexBuffer, self->vertex.shaderName, 0, XprGpuFormat_FloatR32G32B32},
		{self->impl->normalBuffer, self->normal.shaderName, 0, XprGpuFormat_FloatR32G32B32},
		{self->impl->colorBuffer, self->color.shaderName, 0, XprGpuFormat_FloatR32G32B32A32},
		{self->impl->tcBuffer[0], self->texcoord[0].shaderName, 0, XprGpuFormat_FloatR32G32},
		{self->impl->tcBuffer[1], self->texcoord[1].shaderName, 0, XprGpuFormat_FloatR32G32},
	};

	xprGpuProgramBindInput(program, inputs, XprCountOf(inputs));
}

void meshRenderTriangles(Mesh* self)
{
	xprGpuDrawTriangleIndexed(0, self->indexCount, 0, self->vertexCount-1, 0);
}

void meshRenderPatches(Mesh* self)
{
	if((self->indexCount % self->vertexPerPatch) != 0)
		return;

	xprGpuDrawPatchIndexed(0, self->indexCount, 0, self->vertexCount-1, 0, self->vertexPerPatch);
}

void meshRenderPoints(Mesh* self)
{
	xprGpuDrawPoint(0, self->vertexCount);
}

void meshInitWithUnitSphere(Mesh* self, size_t segmentCount)
{	
#define PI 3.14159265358979323846f

	XprVec3* pos;
	XprVec3* nor;
	XprVec2* uv0;
	
	unsigned short* idx;

	float theta, phi;
	int i, j, t;

	int width = segmentCount * 2;
	int height = segmentCount;

	meshInit(self, (height-2)* width+2, (height-2)*(width-1)*2 * 3);
	self->vertexPerPatch = 3;

	idx = xprBufferMap(self->impl->indexBuffer, XprBufferMapAccess_Write);
	pos = xprBufferMap(self->impl->vertexBuffer, XprBufferMapAccess_Write);
	nor = xprBufferMap(self->impl->normalBuffer, XprBufferMapAccess_Write);
	uv0 = xprBufferMap(self->impl->tcBuffer[0], XprBufferMapAccess_Write);
	
	for(t=0, j=1; j<height-1; ++j)
	{
		for(i=0; i<width; ++i)
		{
			uv0[t].x = (float)i/(width-1);
			uv0[t].y = (float)j/(height-1);

			theta = uv0[t].y * PI;
			phi   = uv0[t].x * PI*2;
			pos[t].x =  sinf(theta) * cosf(phi);
			pos[t].y =  cosf(theta);
			pos[t].z = -sinf(theta) * sinf(phi);
			nor[t] = pos[t];
			++t;
		}
	}
	pos[t] = xprVec3(0, 1, 0); nor[t] = pos[t]; ++t;
	pos[t] = xprVec3(0,-1, 0); nor[t] = pos[t]; ++t;

	for(t=0, j=0; j<height-3; ++j)
	{
		for(i=0; i<width-1; ++i)
		{
			idx[t++] = (j  )*width + i  ;
			idx[t++] = (j+1)*width + i+1;
			idx[t++] = (j  )*width + i+1;
			
			idx[t++] = (j  )*width + i  ;
			idx[t++] = (j+1)*width + i  ;
			idx[t++] = (j+1)*width + i+1;
		}
	}

	for( i=0; i<width-1; i++ )
	{
		idx[t++] = (height-2)*width;
		idx[t++] = i;
		idx[t++] = i+1;

		idx[t++] = (height-2)*width+1;
		idx[t++] = (height-3)*width + i+1;
		idx[t++] = (height-3)*width + i;
		
	}

	xprBufferUnmap(self->impl->indexBuffer);
	xprBufferUnmap(self->impl->vertexBuffer);
	xprBufferUnmap(self->impl->normalBuffer);
	xprBufferUnmap(self->impl->tcBuffer[0]);
	
#undef PI
}

void meshInitWithQuad(Mesh* self, float width, float height, const XprVec3* offset, size_t segmentCount)
{
	XprVec3* pos;
	XprVec3* nor;
	XprVec2* uv0;
	unsigned short* idx;

	size_t r, c;
	size_t stride = segmentCount+1;
	
	meshInit(self, stride * stride, (stride-1) * (stride-1) * 6);
	self->vertexPerPatch = 3;

	idx = xprBufferMap(self->impl->indexBuffer, XprBufferMapAccess_Write);
	pos = xprBufferMap(self->impl->vertexBuffer, XprBufferMapAccess_Write);
	nor = xprBufferMap(self->impl->normalBuffer, XprBufferMapAccess_Write);
	uv0 = xprBufferMap(self->impl->tcBuffer[0], XprBufferMapAccess_Write);

	for(r=0; r<(stride-1); ++r)
	{
		for(c=0; c<(stride-1); ++c)
		{
			
			unsigned short p0 = (unsigned short)(r * stride + (c+1));
			unsigned short p1 = (unsigned short)((r+1) * stride + (c+1));
			unsigned short p2 = (unsigned short)(r * stride + c);
			unsigned short p3 = (unsigned short)((r+1) * stride + c);

			(*idx++) = p0;
			(*idx++) = p1;
			(*idx++) = p2;

			(*idx++) = p3;
			(*idx++) = p2;
			(*idx++) = p1;
		}
	}

	for(r=0; r<stride; ++r)
	{
		float y = offset->v[1] + height * (float)r / segmentCount;

		for(c=0; c<stride; ++c)
		{
			float x = offset->v[0] + width * (float)c / segmentCount;

			size_t i = r * stride + c;
			pos[i] = xprVec3(x, y, offset->v[2]);
			nor[i] = xprVec3(0, 0, 1);
			uv0[i].x = height * (float)r / segmentCount;
			uv0[i].y = width * (float)c / segmentCount;
		}
	}
	
	xprBufferUnmap(self->impl->indexBuffer);
	xprBufferUnmap(self->impl->vertexBuffer);
	xprBufferUnmap(self->impl->normalBuffer);
	xprBufferUnmap(self->impl->tcBuffer[0]);
}

void meshInitWithScreenQuad(Mesh* self)
{
	XprVec3* pos;
	XprVec2* uv0;
	unsigned short* idx;

	meshInit(self, 4, 6);
	self->vertexPerPatch = 3;

	idx = xprBufferMap(self->impl->indexBuffer, XprBufferMapAccess_Write);
	pos = xprBufferMap(self->impl->vertexBuffer, XprBufferMapAccess_Write);
	uv0 = xprBufferMap(self->impl->tcBuffer[0], XprBufferMapAccess_Write);

	(*idx++) = 0; (*idx++) = 1; (*idx++) = 2;
	(*idx++) = 3; (*idx++) = 2; (*idx++) = 1;

	pos[0] = xprVec3(-1, 1, 0); uv0[0] = xprVec2(0, 1);
	pos[1] = xprVec3(-1,-1, 0); uv0[1] = xprVec2(0, 0);
	pos[2] = xprVec3( 1, 1, 0); uv0[2] = xprVec2(1, 1);
	pos[3] = xprVec3( 1,-1, 0); uv0[3] = xprVec2(1, 0);
	
	xprBufferUnmap(self->impl->indexBuffer);
	xprBufferUnmap(self->impl->vertexBuffer);
	xprBufferUnmap(self->impl->tcBuffer[0]);
}
