#ifndef __EXAMPLE_MESH_H__
#define __EXAMPLE_MESH_H__

#include "../lib/xprender/Platform.h"
#include "../lib/xprender/Vec3.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XprBuffer;
struct XprGpuProgram;

typedef struct MeshImpl;

enum MeshTrait
{
	MeshTrait_MaxTexcoord = 2
};

typedef struct MeshData
{
	unsigned char* buffer;
	size_t sizeInBytes;
};

enum MeshFlag
{
	MeshFlag_Inited = 1 << 0,
	MeshFlag_Dirty = 1 << 1,
};

typedef struct Mesh
{
	size_t flags;
	size_t vertexCount;
	size_t indexCount;
	struct MeshData index;
	struct MeshData vertex;
	struct MeshData normal;
	struct MeshData color;
	struct MeshData texcoord[MeshTrait_MaxTexcoord];

	struct MeshImpl* impl;
} Mesh;

Mesh* Mesh_alloc();

void Mesh_free(Mesh* self);

void Mesh_init(Mesh* self, size_t vertexCount, size_t indexCount);

void Mesh_initWithUnitSphere(Mesh* self, size_t segmentCount);

void Mesh_initWithQuad(Mesh* self, float width, float height, const XprVec3* offset, size_t segmentCount);

void Mesh_commit(Mesh* self);

void Mesh_bindInputs(Mesh* self, struct XprGpuProgram* program);

void Mesh_draw(Mesh* self);

void Mesh_drawPoints(Mesh* self);

#ifdef __cplusplus
}
#endif

#endif	// __EXAMPLE_CLOTH_H__
