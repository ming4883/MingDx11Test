#include "Mesh.h"

#include "../lib/xprender/Buffer.h"
#include "../lib/xprender/Vec3.h"
#include <GL/glew.h>
#include <math.h>

Mesh* Mesh_new(size_t vertexCount, size_t indexCount)
{
	Mesh* self = (Mesh*)malloc(sizeof(Mesh));
	self->vertexCount = vertexCount;
	self->indexCount = indexCount;
	
	self->vertexBuffer = xprBuffer_new(xprBufferType_Vertex, sizeof(xprVec3) * self->vertexCount, nullptr);
	self->normalBuffer = xprBuffer_new(xprBufferType_Vertex, sizeof(xprVec3) * self->vertexCount, nullptr);
	self->indexBuffer = xprBuffer_new(xprBufferType_Index, sizeof(unsigned short) * self->indexCount, nullptr);

	return self;
}

void Mesh_free(Mesh* self)
{
	xprBuffer_free(self->vertexBuffer);
	xprBuffer_free(self->normalBuffer);
	xprBuffer_free(self->indexBuffer);
	free(self);
}

void Mesh_draw(Mesh* self)
{
	glBindBuffer(GL_ARRAY_BUFFER, self->vertexBuffer->name);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(xprVec3), 0);

	glBindBuffer(GL_ARRAY_BUFFER, self->normalBuffer->name);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, sizeof(xprVec3), 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->indexBuffer->name);

	glDrawElements(GL_TRIANGLES, self->indexCount, GL_UNSIGNED_SHORT, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

Mesh* Mesh_createUnitSphere(size_t segmentCount)
{	
#define PI 3.14159265358979323846f

	xprVec3* pos;
	xprVec3* nor;
	unsigned short* idx;

	float theta, phi;
	int i, j, t;

	int width = segmentCount * 2;
	int height = segmentCount;

	Mesh* mesh = Mesh_new(
		(height-2)* width+2,
		(height-2)*(width-1)*2 * 3
		);

	pos = xprBuffer_map(mesh->vertexBuffer, xprBufferMapAccess_Write);
	nor = xprBuffer_map(mesh->normalBuffer, xprBufferMapAccess_Write);
	idx = xprBuffer_map(mesh->indexBuffer, xprBufferMapAccess_Write);

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
	pos[t] = xprVec3_(0, 1, 0); nor[t] = pos[t]; ++t;
	pos[t] = xprVec3_(0,-1, 0); nor[t] = pos[t]; ++t;

	for(t=0, j=0; j<height-3; ++j)
	{
		for(i=0; i<width-1; ++i)
		{
			idx[t++] = (j  )*width + i  ;
			idx[t++] = (j  )*width + i+1;
			idx[t++] = (j+1)*width + i+1;
			
			idx[t++] = (j  )*width + i  ;
			idx[t++] = (j+1)*width + i+1;
			idx[t++] = (j+1)*width + i  ;
		}
	}

	for( i=0; i<width-1; i++ )
	{
		idx[t++] = (height-2)*width;
		idx[t++] = i+1;
		idx[t++] = i;

		idx[t++] = (height-2)*width+1;
		idx[t++] = (height-3)*width + i;
		idx[t++] = (height-3)*width + i+1;
		
	}

	xprBuffer_unmap(mesh->vertexBuffer);
	xprBuffer_unmap(mesh->normalBuffer);
	xprBuffer_unmap(mesh->indexBuffer);

	return mesh;

#undef PI
}

