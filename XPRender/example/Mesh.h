#ifndef __EXAMPLE_MESH_H__
#define __EXAMPLE_MESH_H__

#include "../lib/xprender/Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xprBuffer xprBuffer;

typedef struct Mesh
{
	size_t vertexCount;
	size_t indexCount;
	xprBuffer* vertexBuffer;
	xprBuffer* normalBuffer;
	xprBuffer* indexBuffer;

} Mesh;

Mesh* Mesh_new(size_t vertexCount, size_t indexCount);

void Mesh_free(Mesh* self);

void Mesh_draw(Mesh* self);

Mesh* Mesh_createUnitSphere(size_t segmentCount);

Mesh* Mesh_createQuad(float width, float height, const float offset[3], size_t segmentCount);

#ifdef __cplusplus
}
#endif

#endif	// __EXAMPLE_CLOTH_H__
