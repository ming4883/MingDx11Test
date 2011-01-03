#include "Mesh.h"

#include "../lib/xprender/Buffer.h"
#include "../lib/xprender/Vec3.h"
#include "../lib/xprender/Shader.h"
#include <GL/glew.h>
#include <math.h>

Mesh* Mesh_new(size_t vertexCount, size_t indexCount)
{
	Mesh* self = (Mesh*)malloc(sizeof(Mesh));
	self->vertexCount = vertexCount;
	self->indexCount = indexCount;
	
	self->vertexBuffer = XprBuffer_alloc();
	XprBuffer_init(self->vertexBuffer, XprBufferType_Vertex, sizeof(XprVec3) * self->vertexCount, nullptr);
	
	self->normalBuffer = XprBuffer_alloc();
	XprBuffer_init(self->normalBuffer, XprBufferType_Vertex, sizeof(XprVec3) * self->vertexCount, nullptr);
	
	self->indexBuffer = XprBuffer_alloc();
	XprBuffer_init(self->indexBuffer, XprBufferType_Index, sizeof(unsigned short) * self->indexCount, nullptr);

	glGenVertexArrays(1, &self->ia);

	return self;
}

void Mesh_free(Mesh* self)
{
	glDeleteVertexArrays(1, &self->ia);
	XprBuffer_free(self->vertexBuffer);
	XprBuffer_free(self->normalBuffer);
	XprBuffer_free(self->indexBuffer);
	free(self);
}

void Mesh_bindInputs(Mesh* self, struct XprGpuProgram* pipeline)
{
	int vertLoc = glGetAttribLocation(pipeline->name, "i_vertex");
	int normLoc = glGetAttribLocation(pipeline->name, "i_normal");

	glBindVertexArray(self->ia);

	glBindBuffer(GL_ARRAY_BUFFER, self->vertexBuffer->name);
	glVertexAttribPointer(vertLoc, 3, GL_FLOAT, GL_FALSE, sizeof(XprVec3), 0);
	glEnableVertexAttribArray(vertLoc);

	glBindBuffer(GL_ARRAY_BUFFER, self->normalBuffer->name);
	glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, sizeof(XprVec3), 0);
	glEnableVertexAttribArray(normLoc);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->indexBuffer->name);
}

void Mesh_draw(Mesh* self)
{
	glDrawElements(GL_TRIANGLES, self->indexCount, GL_UNSIGNED_SHORT, 0);
}

void Mesh_drawPoints(Mesh* self)
{
	glDrawArrays(GL_POINTS, 0, self->vertexCount);
}

Mesh* Mesh_createUnitSphere(size_t segmentCount)
{	
#define PI 3.14159265358979323846f

	XprVec3* pos;
	XprVec3* nor;
	unsigned short* idx;

	float theta, phi;
	int i, j, t;

	int width = segmentCount * 2;
	int height = segmentCount;

	Mesh* mesh = Mesh_new(
		(height-2)* width+2,
		(height-2)*(width-1)*2 * 3
		);

	pos = XprBuffer_map(mesh->vertexBuffer, XprBufferMapAccess_Write);
	nor = XprBuffer_map(mesh->normalBuffer, XprBufferMapAccess_Write);
	idx = XprBuffer_map(mesh->indexBuffer, XprBufferMapAccess_Write);

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

	XprBuffer_unmap(mesh->vertexBuffer);
	XprBuffer_unmap(mesh->normalBuffer);
	XprBuffer_unmap(mesh->indexBuffer);

	return mesh;

#undef PI
}

Mesh* Mesh_createQuad(float width, float height, const XprVec3* offset, size_t segmentCount)
{
	XprVec3* pos;
	XprVec3* nor;
	unsigned short* idx;

	size_t r, c;
	size_t stride = segmentCount+1;
	
	Mesh* mesh = Mesh_new(
		stride * stride,
		(stride-1) * (stride-1) * 6
		);

	pos = XprBuffer_map(mesh->vertexBuffer, XprBufferMapAccess_Write);
	nor = XprBuffer_map(mesh->normalBuffer, XprBufferMapAccess_Write);
	idx = XprBuffer_map(mesh->indexBuffer, XprBufferMapAccess_Write);

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
	
	XprBuffer_unmap(mesh->vertexBuffer);
	XprBuffer_unmap(mesh->normalBuffer);
	XprBuffer_unmap(mesh->indexBuffer);

	return mesh;
}
