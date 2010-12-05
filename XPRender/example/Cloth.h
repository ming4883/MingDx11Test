#ifndef __EXAMPLE_CLOTH_H__
#define __EXAMPLE_CLOTH_H__

#include "../lib/xprender/Vec3.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Mesh;
struct Sphere;

typedef struct ClothConstraint
{
	size_t pIdx[2];
	float restDistance;
} ClothConstraint;

typedef struct Cloth
{
	size_t segmentCount;
	XprVec3* p;		// current positions
	XprVec3* p2;	// last positions
	XprVec3* a;		// accelerations
	XprVec3 g;		// gravity
	xprBool* fixed;		// fixed
	XprVec3* fixPos;	// fix positions
	float timeStep;	// system time step
	float damping;	// dumping due to air resistence, 0-1, 0 = no dumping

	ClothConstraint* constraints;
	size_t constraintCount;

	struct Mesh* mesh;

} Cloth;

Cloth* Cloth_new(float width, float height, const XprVec3* offset, size_t segmentCount);

void Cloth_free(Cloth* self);

void Cloth_updateMesh(Cloth* self);

void Cloth_addForceToAll(Cloth* self, const XprVec3* force);

void Cloth_verletIntegration(Cloth* self);

void Cloth_satisfyConstraints(Cloth* self);

void Cloth_collideWithSphere(Cloth* self, const struct Sphere* sphere);

void Cloth_collideWithPlane(Cloth* self, const XprVec3* normal, const XprVec3* point);

#ifdef __cplusplus
}
#endif

#endif	// __EXAMPLE_CLOTH_H__
