#ifndef __EXAMPLE_MESH_H__
#define __EXAMPLE_MESH_H__

#include "Stream.h"
#include "../lib/xprender/Platform.h"
#include "../lib/xprender/Vec3.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XprBuffer;
struct XprGpuProgram;

enum MeshTrait
{
	MeshTrait_MaxTexcoord = 2
};

typedef struct MeshData
{
	unsigned char* buffer;
	size_t sizeInBytes;
	char shaderName[16];
} MeshData;

enum MeshFlag
{
	MeshFlag_Inited = 1 << 0,
	MeshFlag_Dirty = 1 << 1,
};

struct MeshImpl;

typedef struct Mesh
{
	size_t flags;
	size_t vertexCount;
	size_t indexCount;
	size_t vertexPerPatch;
	struct MeshData index;
	struct MeshData vertex;
	struct MeshData normal;
	struct MeshData color;
	struct MeshData texcoord[MeshTrait_MaxTexcoord];

	struct MeshImpl* impl;
} Mesh;

Mesh* meshAlloc();

void meshFree(Mesh* self);

void meshInit(Mesh* self, size_t vertexCount, size_t indexCount);

void meshInitWithUnitSphere(Mesh* self, size_t segmentCount);

void meshInitWithQuad(Mesh* self, float width, float height, const XprVec3* offset, size_t segmentCount);

void meshInitWithScreenQuad(Mesh* self);

XprBool meshInitWithObjFile(Mesh* self, const char* path, InputStream* stream);

void meshCommit(Mesh* self);

void meshPreRender(Mesh* self, struct XprGpuProgram* program);

void meshRenderTriangles(Mesh* self);

void meshRenderPatches(Mesh* self);

void meshRenderPoints(Mesh* self);

#ifdef __cplusplus
}
#endif

#endif	// __EXAMPLE_CLOTH_H__
