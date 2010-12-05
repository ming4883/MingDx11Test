#ifndef __EXAMPLE_MESH_H__
#define __EXAMPLE_MESH_H__

#include "../lib/xprender/Platform.h"
#include "../lib/xprender/Vec3.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XprBuffer;

typedef struct Mesh
{
	size_t vertexCount;
	size_t indexCount;
	struct XprBuffer* vertexBuffer;
	struct XprBuffer* normalBuffer;
	struct XprBuffer* indexBuffer;

} Mesh;

Mesh* Mesh_new(size_t vertexCount, size_t indexCount);

void Mesh_free(Mesh* self);

void Mesh_draw(Mesh* self);

void Mesh_drawPoints(Mesh* self);

Mesh* Mesh_createUnitSphere(size_t segmentCount);

Mesh* Mesh_createQuad(float width, float height, const XprVec3* offset, size_t segmentCount);

#ifdef __cplusplus
}
#endif

#endif	// __EXAMPLE_CLOTH_H__
