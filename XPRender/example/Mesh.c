#include "Mesh.h"

#include "../lib/xprender/Buffer.GL3.h"
#include "../lib/xprender/Shader.GL3.h"
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
	self->impl->indexBuffer = XprBuffer_alloc();
	XprBuffer_init(self->impl->indexBuffer, XprBufferType_Index, self->index.sizeInBytes, nullptr);

	self->impl->vertexBuffer = XprBuffer_alloc();
	XprBuffer_init(self->impl->vertexBuffer, XprBufferType_Vertex, self->vertex.sizeInBytes, nullptr);
	
	self->impl->normalBuffer = XprBuffer_alloc();
	XprBuffer_init(self->impl->normalBuffer, XprBufferType_Vertex, self->normal.sizeInBytes, nullptr);

	self->impl->colorBuffer = XprBuffer_alloc();
	XprBuffer_init(self->impl->colorBuffer, XprBufferType_Vertex, self->color.sizeInBytes, nullptr);

	for(i=0; i<MeshTrait_MaxTexcoord; ++i) {
		self->impl->tcBuffer[i] = XprBuffer_alloc();
		XprBuffer_init(self->impl->tcBuffer[i], XprBufferType_Vertex, self->texcoord[i].sizeInBytes, nullptr);
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
			XprBuffer_free(self->impl->tcBuffer[i]);
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
		XprBuffer_update(self->impl->tcBuffer[i], 0, self->texcoord[i].sizeInBytes, self->texcoord[i].buffer);
	}	

#undef commit
}

void Mesh_preRender(Mesh* self, struct XprGpuProgram* program)
{
	int vertLoc = glGetAttribLocation(program->impl->glName, self->vertex.shaderName);
	int normLoc = glGetAttribLocation(program->impl->glName, self->normal.shaderName);
	int colorLoc = glGetAttribLocation(program->impl->glName, self->color.shaderName);
	int uvLoc[MeshTrait_MaxTexcoord];
	int i;
	
	for(i=0; i<MeshTrait_MaxTexcoord; ++i)
		uvLoc[i] = glGetAttribLocation(program->impl->glName, self->texcoord[i].shaderName);

	glBindVertexArray(self->impl->ia);

	if(vertLoc >= 0) {
		glBindBuffer(GL_ARRAY_BUFFER, self->impl->vertexBuffer->impl->glName);
		glVertexAttribPointer(vertLoc, 3, GL_FLOAT, GL_FALSE, sizeof(XprVec3), 0);
		glEnableVertexAttribArray(vertLoc);
	}

	if(normLoc >= 0) {
		glBindBuffer(GL_ARRAY_BUFFER, self->impl->normalBuffer->impl->glName);
		glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, sizeof(XprVec3), 0);
		glEnableVertexAttribArray(normLoc);
	}
	
	if(colorLoc >= 0) {
		glBindBuffer(GL_ARRAY_BUFFER, self->impl->colorBuffer->impl->glName);
		glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, sizeof(XprVec4), 0);
		glEnableVertexAttribArray(colorLoc);
	}

	for(i=0; i<MeshTrait_MaxTexcoord; ++i) {
		if(uvLoc[i] >= 0) {
			glBindBuffer(GL_ARRAY_BUFFER, self->impl->tcBuffer[i]->impl->glName);
			glVertexAttribPointer(uvLoc[i], 2, GL_FLOAT, GL_FALSE, sizeof(XprVec2), 0);
			glEnableVertexAttribArray(uvLoc[i]);
		}
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->impl->indexBuffer->impl->glName);
}

void Mesh_render(Mesh* self)
{
	glDrawElements(GL_TRIANGLES, self->indexCount, GL_UNSIGNED_SHORT, 0);
}

void Mesh_renderPatches(Mesh* self, size_t vertexPrePatch)
{
	if((self->indexCount % vertexPrePatch) != 0)
		return;

	glPatchParameteri(GL_PATCH_VERTICES, vertexPrePatch);
	glDrawElements(GL_PATCHES, self->indexCount, GL_UNSIGNED_SHORT, 0);
}

void Mesh_renderPoints(Mesh* self)
{
	glDrawArrays(GL_POINTS, 0, self->vertexCount);
}

void Mesh_initWithUnitSphere(Mesh* self, size_t segmentCount)
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

	Mesh_init(self, (height-2)* width+2, (height-2)*(width-1)*2 * 3);

	idx = XprBuffer_map(self->impl->indexBuffer, XprBufferMapAccess_Write);
	pos = XprBuffer_map(self->impl->vertexBuffer, XprBufferMapAccess_Write);
	nor = XprBuffer_map(self->impl->normalBuffer, XprBufferMapAccess_Write);
	uv0 = XprBuffer_map(self->impl->tcBuffer[0], XprBufferMapAccess_Write);
	
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

	XprBuffer_unmap(self->impl->indexBuffer);
	XprBuffer_unmap(self->impl->vertexBuffer);
	XprBuffer_unmap(self->impl->normalBuffer);
	XprBuffer_unmap(self->impl->tcBuffer[0]);
	
#undef PI
}

void Mesh_initWithQuad(Mesh* self, float width, float height, const XprVec3* offset, size_t segmentCount)
{
	XprVec3* pos;
	XprVec3* nor;
	XprVec2* uv0;
	unsigned short* idx;

	size_t r, c;
	size_t stride = segmentCount+1;
	
	Mesh_init(self, stride * stride, (stride-1) * (stride-1) * 6);

	idx = XprBuffer_map(self->impl->indexBuffer, XprBufferMapAccess_Write);
	pos = XprBuffer_map(self->impl->vertexBuffer, XprBufferMapAccess_Write);
	nor = XprBuffer_map(self->impl->normalBuffer, XprBufferMapAccess_Write);
	uv0 = XprBuffer_map(self->impl->tcBuffer[0], XprBufferMapAccess_Write);

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
			uv0[i].x = height * (float)r / segmentCount;
			uv0[i].y = width * (float)c / segmentCount;
		}
	}
	
	XprBuffer_unmap(self->impl->indexBuffer);
	XprBuffer_unmap(self->impl->vertexBuffer);
	XprBuffer_unmap(self->impl->normalBuffer);
	XprBuffer_unmap(self->impl->tcBuffer[0]);
}

void Mesh_initWithScreenQuad(Mesh* self)
{
	XprVec3* pos;
	XprVec2* uv0;
	unsigned short* idx;

	Mesh_init(self, 4, 6);

	idx = XprBuffer_map(self->impl->indexBuffer, XprBufferMapAccess_Write);
	pos = XprBuffer_map(self->impl->vertexBuffer, XprBufferMapAccess_Write);
	uv0 = XprBuffer_map(self->impl->tcBuffer[0], XprBufferMapAccess_Write);

	(*idx++) = 0; (*idx++) = 1; (*idx++) = 2;
	(*idx++) = 3; (*idx++) = 2; (*idx++) = 1;

	pos[0] = XprVec3_(-1, 1, 0); uv0[0] = XprVec2_(0, 1);
	pos[1] = XprVec3_(-1,-1, 0); uv0[1] = XprVec2_(0, 0);
	pos[2] = XprVec3_( 1, 1, 0); uv0[2] = XprVec2_(1, 1);
	pos[3] = XprVec3_( 1,-1, 0); uv0[3] = XprVec2_(1, 0);
	
	XprBuffer_unmap(self->impl->indexBuffer);
	XprBuffer_unmap(self->impl->vertexBuffer);
	XprBuffer_unmap(self->impl->tcBuffer[0]);
}

typedef struct ObjBuffer
{
	void* data;
	size_t elemSz;
	size_t cap;
	size_t cnt;
} ObjBuffer;

typedef struct ObjFace
{
	size_t v[4];
	size_t vt[4];
	size_t vn[4];
} ObjFace;

void ObjBuffer_resize(ObjBuffer* buf)
{
	buf->data = realloc(buf->data, buf->cap * buf->elemSz);
}

void ObjBuffer_appendVec3(ObjBuffer* buf, float x, float y, float z)
{
	XprVec3* ptr;
	if(buf->cap == buf->cnt) {
		buf->cap *= 2;
		ObjBuffer_resize(buf);
	}

	ptr = (XprVec3*)buf->data;

	ptr[buf->cnt].x = x;
	ptr[buf->cnt].y = y;
	ptr[buf->cnt].z = z;

	++buf->cnt;
}

void ObjBuffer_appendFace(ObjBuffer* buf, ObjFace face)
{
	ObjFace* ptr;
	if(buf->cap == buf->cnt) {
		buf->cap *= 2;
		ObjBuffer_resize(buf);
	}

	ptr = (ObjFace*)buf->data;
	ptr[buf->cnt] = face;

	++buf->cnt;
}

void Mesh_initWithObjFile(Mesh* self, const char* path)
{
	FILE* fp;
	char readbuf[512];
	char* token;
	const char* whitespace = " \t\n\r";
	ObjBuffer vbuf  = {nullptr, sizeof(XprVec3), 128, 0};
	ObjBuffer vtbuf = {nullptr, sizeof(XprVec3), 128, 0};
	ObjBuffer vnbuf = {nullptr, sizeof(XprVec3), 128, 0};
	ObjBuffer fbuf = {nullptr, sizeof(ObjFace), 128, 0};
	size_t vpf = 0;

	if(nullptr == (fp = fopen(path, "r"))) {
		return;
	}

	ObjBuffer_resize(&vbuf);
	ObjBuffer_resize(&vtbuf);
	ObjBuffer_resize(&vnbuf);
	ObjBuffer_resize(&fbuf);

	while( fgets(readbuf, 512, fp) ) {
		if('#' == readbuf[0])
			continue;

		token = strtok(readbuf, whitespace);

		if(nullptr == token) {
			continue;
		}
		else if(0 == strcmp(token, "v")) {
			float x = (float)atof(strtok(nullptr, whitespace));
			float y = (float)atof(strtok(nullptr, whitespace));
			float z = (float)atof(strtok(nullptr, whitespace));
			ObjBuffer_appendVec3(&vbuf, x, y, z);
		}
		else if(0 == strcmp(token, "vt")) {
			float x = (float)atof(strtok(nullptr, whitespace));
			float y = (float)atof(strtok(nullptr, whitespace));
			float z = 0;
			token = strtok(nullptr, whitespace);
			if(nullptr != token)
				z = (float)atof(token);
			ObjBuffer_appendVec3(&vtbuf, x, y, z);
		}
		else if(0 == strcmp(token, "vn")) {
			float x = (float)atof(strtok(nullptr, whitespace));
			float y = (float)atof(strtok(nullptr, whitespace));
			float z = (float)atof(strtok(nullptr, whitespace));
			ObjBuffer_appendVec3(&vnbuf, x, y, z);
		}
		else if(0 == strcmp(token, "f")) {
			char vi[16];
			char vti[16];
			char vni[16];
			ObjFace f;
			size_t vcnt = 0;
			memset(&f, 0, sizeof(f));			

			while(nullptr != (token = strtok(nullptr, whitespace)) && vcnt <= 4) {
				sscanf(token, "%[^/]/%[^/]/%[^/]", vi, vti, vni);
				f.v[vcnt] = atoi(vi)-1;
				f.vt[vcnt] = atoi(vti)-1;
				f.vn[vcnt] = atoi(vni)-1;
				++vcnt;
			}
			ObjBuffer_appendFace(&fbuf, f);
			vpf = vcnt > vpf ? vcnt : vpf;

		}
	}

	// flatten vertices
	Mesh_init(self, fbuf.cnt * vpf, fbuf.cnt * vpf);

	{
		size_t i=0, j=0, vcnt = 0;
		XprVec3* v = (XprVec3*)self->vertex.buffer;
		XprVec3* n = (XprVec3*)self->normal.buffer;
		XprVec2* t = (XprVec2*)self->texcoord[0].buffer;
		unsigned short* id = (unsigned short*)self->index.buffer;

		for(i = 0; i < fbuf.cnt; ++i) {
			ObjFace* f = &((ObjFace*)fbuf.data)[i];
			for(j = 0; j < vpf; ++j) {
				*v = ((XprVec3*)vbuf.data)[f->v[j]];

				if(vnbuf.cnt) {
					*n = ((XprVec3*)vnbuf.data)[f->vn[j]];
				}

				if(vtbuf.cnt) {
					t->x = ((XprVec3*)vtbuf.data)[f->vt[j]].x;
					t->y = ((XprVec3*)vtbuf.data)[f->vt[j]].y;
				}

				*id = (unsigned short)vcnt;

				++v;
				++n;
				++t;
				++id;
				++vcnt;
			}
		}
	}

	Mesh_commit(self);

	// clean up
	vbuf.cap = 0;
	vtbuf.cap = 0;
	vnbuf.cap = 0;
	fbuf.cap = 0;

	ObjBuffer_resize(&vbuf);
	ObjBuffer_resize(&vtbuf);
	ObjBuffer_resize(&vnbuf);
	ObjBuffer_resize(&fbuf);

	fclose(fp);
}