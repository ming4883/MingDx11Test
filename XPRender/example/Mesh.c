#include "Mesh.h"

#include "../lib/xprender/Buffer.GL3.h"
#include "../lib/xprender/Shader.GL3.h"
#include "../lib/xprender/Vec3.h"
#include "../lib/xprender/Vec4.h"

#include <math.h>

typedef struct MeshImpl {
	struct XprBuffer* indexBuffer;
	struct XprBuffer* vertexBuffer;
	struct XprBuffer* normalBuffer;
	struct XprBuffer* colorBuffer;
	struct XprBuffer* texcoordBuffer[MeshTrait_MaxTexcoord];
	int ia;
} MeshImpl;

Mesh* Mesh_alloc()
{
	Mesh* self;
	XprAllocWithImpl(self, Mesh, MeshImpl);

	return self;
}

void Mesh_init(Mesh* self, size_t vertexCount, size_t indexCount)
{
	size_t i;

	self->vertexCount = vertexCount;
	self->indexCount = indexCount;

	self->index.sizeInBytes = sizeof(unsigned short) * self->indexCount;
	self->index.buffer = malloc(self->index.sizeInBytes);

	self->vertex.sizeInBytes = sizeof(XprVec3) * self->vertexCount;
	self->vertex.buffer = malloc(self->vertex.sizeInBytes);

	self->normal.sizeInBytes = sizeof(XprVec3) * self->vertexCount;
	self->normal.buffer = malloc(self->normal.sizeInBytes);

	self->color.sizeInBytes = sizeof(XprVec4) * self->vertexCount;
	self->color.buffer = malloc(self->color.sizeInBytes);

	for(i=0; i<MeshTrait_MaxTexcoord; ++i) {
		self->texcoord[i].sizeInBytes = sizeof(float) * 2 * self->vertexCount;
		self->texcoord[i].buffer = malloc(self->texcoord[i].sizeInBytes);
	}

	// hardware buffer
	self->impl->indexBuffer = XprBuffer_alloc();
	XprBuffer_init(self->impl->indexBuffer, XprBufferType_Index, self->index.sizeInBytes, nullptr);

	self->impl->vertexBuffer = XprBuffer_alloc();
	XprBuffer_init(self->impl->vertexBuffer, XprBufferType_Vertex, self->vertex.sizeInBytes, nullptr);
	
	self->impl->normalBuffer = XprBuffer_alloc();
	XprBuffer_init(self->impl->normalBuffer, XprBufferType_Vertex, self->normal.sizeInBytes, nullptr);

	self->impl->colorBuffer = XprBuffer_alloc();
	XprBuffer_init(self->impl->colorBuffer, XprBufferType_Vertex, self->color.sizeInBytes, nullptr);

	for(i=0; i<MeshTrait_MaxTexcoord; ++i) {
		self->impl->texcoordBuffer[i] = XprBuffer_alloc();
		XprBuffer_init(self->impl->texcoordBuffer[i], XprBufferType_Vertex, self->texcoord[i].sizeInBytes, nullptr);
	}	
	
	glGenVertexArrays(1, &self->impl->ia);

	self->flags |= MeshFlag_Inited;
}

void Mesh_free(Mesh* self)
{
	size_t i;

	if(self->flags & MeshFlag_Inited) {
		glDeleteVertexArrays(1, &self->impl->ia);
		XprBuffer_free(self->impl->indexBuffer);
		XprBuffer_free(self->impl->vertexBuffer);
		XprBuffer_free(self->impl->normalBuffer);
		XprBuffer_free(self->impl->colorBuffer);

		for(i=0; i<MeshTrait_MaxTexcoord; ++i) {
			XprBuffer_free(self->impl->texcoordBuffer[i]);
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

void Mesh_commit(Mesh* self)
{
	size_t i;

#define commit(x) {\
	XprBuffer_update(self->impl->x##Buffer, 0, self->x.sizeInBytes, self->x.buffer);\
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
		XprBuffer_update(self->impl->texcoordBuffer[i], 0, self->texcoord[i].sizeInBytes, self->texcoord[i].buffer);
	}	

#undef commit
}

void Mesh_bindInputs(Mesh* self, struct XprGpuProgram* program)
{
	int vertLoc = glGetAttribLocation(program->impl->glName, "i_vertex");
	int normLoc = glGetAttribLocation(program->impl->glName, "i_normal");

	glBindVertexArray(self->impl->ia);

	glBindBuffer(GL_ARRAY_BUFFER, self->impl->vertexBuffer->impl->glName);
	glVertexAttribPointer(vertLoc, 3, GL_FLOAT, GL_FALSE, sizeof(XprVec3), 0);
	glEnableVertexAttribArray(vertLoc);

	glBindBuffer(GL_ARRAY_BUFFER, self->impl->normalBuffer->impl->glName);
	glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, sizeof(XprVec3), 0);
	glEnableVertexAttribArray(normLoc);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->impl->indexBuffer->impl->glName);
}

void Mesh_draw(Mesh* self)
{
	glDrawElements(GL_TRIANGLES, self->indexCount, GL_UNSIGNED_SHORT, 0);
}

void Mesh_drawPoints(Mesh* self)
{
	glDrawArrays(GL_POINTS, 0, self->vertexCount);
}

void Mesh_initWithUnitSphere(Mesh* self, size_t segmentCount)
{	
#define PI 3.14159265358979323846f

	XprVec3* pos;
	XprVec3* nor;
	unsigned short* idx;

	float theta, phi;
	int i, j, t;

	int width = segmentCount * 2;
	int height = segmentCount;

	Mesh_init(self, (height-2)* width+2, (height-2)*(width-1)*2 * 3);

	pos = XprBuffer_map(self->impl->vertexBuffer, XprBufferMapAccess_Write);
	nor = XprBuffer_map(self->impl->normalBuffer, XprBufferMapAccess_Write);
	idx = XprBuffer_map(self->impl->indexBuffer, XprBufferMapAccess_Write);

	for(t=0, j=1; j<height-1; ++j)
	{
		for(i=0; i<width; ++i)
		{
			theta = (float)j/(height-1) * PI;
			phi   = (float)i/(width-1 ) * PI*2;
			pos[t].x =  sinf(theta) * cosf(phi);
			pos[t].y =  cosf(theta);
			pos[t].z = -sinf(theta) * sinf(phi);
			nor[t] = pos[t];
			++t;
		}
	}
	pos[t] = XprVec3_(0, 1, 0); nor[t] = pos[t]; ++t;
	pos[t] = XprVec3_(0,-1, 0); nor[t] = pos[t]; ++t;

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

	XprBuffer_unmap(self->impl->vertexBuffer);
	XprBuffer_unmap(self->impl->normalBuffer);
	XprBuffer_unmap(self->impl->indexBuffer);

#undef PI
}

void Mesh_initWithQuad(Mesh* self, float width, float height, const XprVec3* offset, size_t segmentCount)
{
	XprVec3* pos;
	XprVec3* nor;
	unsigned short* idx;

	size_t r, c;
	size_t stride = segmentCount+1;
	
	Mesh_init(self, stride * stride, (stride-1) * (stride-1) * 6);

	pos = XprBuffer_map(self->impl->vertexBuffer, XprBufferMapAccess_Write);
	nor = XprBuffer_map(self->impl->normalBuffer, XprBufferMapAccess_Write);
	idx = XprBuffer_map(self->impl->indexBuffer, XprBufferMapAccess_Write);

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
			pos[i] = XprVec3_(x, y, offset->v[2]);
			nor[i] = XprVec3_(0, 0, 1);
		}
	}
	
	XprBuffer_unmap(self->impl->vertexBuffer);
	XprBuffer_unmap(self->impl->normalBuffer);
	XprBuffer_unmap(self->impl->indexBuffer);
}
