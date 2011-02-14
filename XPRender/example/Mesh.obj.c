#include "Mesh.h"

#include "../lib/xprender/Vec2.h"
#include "../lib/xprender/Vec3.h"
#include "../lib/xprender/Vec4.h"

#include <math.h>
#include <stdio.h>

typedef struct ObjBuffer
{
	char* data;
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

char* ObjBuffer_append(ObjBuffer* buf)
{
	char* ptr;

	if(buf->cap == buf->cnt) {
		buf->cap *= 2;
		ObjBuffer_resize(buf);
	}

	ptr = buf->data + (buf->cnt * buf->elemSz);
	
	++buf->cnt;

	return ptr;
}

const char* Obj_Whitespace = " \t\n\r";

void Obj_ReadVec3(XprVec3* ret)
{
	char* token;
	ret->x = 0;
	ret->y = 0;
	ret->z = 0;

	token = strtok(nullptr, Obj_Whitespace);
	if(nullptr != token)
		ret->x = (float)atof(token);

	token = strtok(nullptr, Obj_Whitespace);
	if(nullptr != token)
		ret->y = (float)atof(token);

	token = strtok(nullptr, Obj_Whitespace);
	if(nullptr != token)
		ret->z = (float)atof(token);
}

size_t Obj_ReadFace(ObjFace* face)
{
	char* token;
	char vi[16] = {0};
	char vti[16] = {0};
	char vni[16] = {0};
	size_t vcnt = 0;
	memset(face, 0, sizeof(ObjFace));

	while(nullptr != (token = strtok(nullptr, Obj_Whitespace)) && vcnt <= 4) {
		if(strstr(token, "//")) {
			sscanf(token, "%[^/]//%[^/]", vi, vni);
			if(strlen(vi) > 0) face->v[vcnt] = atoi(vi)-1;
			if(strlen(vni) > 0) face->vn[vcnt] = atoi(vni)-1;
		}
		else {
			sscanf(token, "%[^/]/%[^/]/%[^/]", vi, vti, vni);
			if(strlen(vi) > 0) face->v[vcnt] = atoi(vi)-1;
			if(strlen(vti) > 0) face->vt[vcnt] = atoi(vti)-1;
			if(strlen(vni) > 0) face->vn[vcnt] = atoi(vni)-1;
		}
		++vcnt;
	}

	return vcnt;
}

XprBool Mesh_initWithObjFile(Mesh* self, const char* path, InputStream* stream)
{
	void* fp;
	char readbuf[512];
	char* token;
	
	ObjBuffer vbuf  = {nullptr, sizeof(XprVec3), 128, 0};
	ObjBuffer vtbuf = {nullptr, sizeof(XprVec3), 128, 0};
	ObjBuffer vnbuf = {nullptr, sizeof(XprVec3), 128, 0};
	ObjBuffer fbuf  = {nullptr, sizeof(ObjFace), 128, 0};
	size_t vpf = 0;

	if(nullptr == (fp = stream->open(path))) {
		XprDbgStr("obj file %s not found", path);
		return XprFalse;
	}

	ObjBuffer_resize(&vbuf);
	ObjBuffer_resize(&vtbuf);
	ObjBuffer_resize(&vnbuf);
	ObjBuffer_resize(&fbuf);

	//while( fgets(readbuf, 512, fp) ) {
	while( Stream_gets(stream, readbuf, 512, fp) ) {
		if('#' == readbuf[0])
			continue;

		token = strtok(readbuf, Obj_Whitespace);

		if(nullptr == token) {
			continue;
		}
		else if(0 == strcmp(token, "v")) {
			Obj_ReadVec3((XprVec3*)ObjBuffer_append(&vbuf));
		}
		else if(0 == strcmp(token, "vt")) {
			Obj_ReadVec3((XprVec3*)ObjBuffer_append(&vtbuf));
		}
		else if(0 == strcmp(token, "vn")) {
			Obj_ReadVec3((XprVec3*)ObjBuffer_append(&vnbuf));
		}
		else if(0 == strcmp(token, "f")) {
			size_t vcnt = Obj_ReadFace((ObjFace*)ObjBuffer_append(&fbuf));
			vpf = vcnt > vpf ? vcnt : vpf;
		}
	}
	
	stream->close(fp);

	// flatten vertices
	Mesh_init(self, fbuf.cnt * vpf, fbuf.cnt * vpf);
	self->vertexPerPatch = vpf;

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

				++v; ++n; ++t; ++id;
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

	return XprTrue;
}