#ifndef __EXAMPLE_CLOTH_H__
#define __EXAMPLE_CLOTH_H__

#include "../lib/xprender/Vec3.h"

typedef struct xprBuffer xprBuffer;

typedef struct ClothConstraint
{
	size_t pIdx[2];
	float restDistance;
} ClothConstraint;

typedef struct Cloth
{
	size_t segmentCount;
	xprVec3* p;		// current positions
	xprVec3* p2;	// last positions
	xprVec3* a;		// accelerations
	xprVec3 g;		// gravity
	xprBool* fixed;		// fixed
	xprVec3* fixPos;	// fix positions
	float timeStep;	// system time step
	float dumping;	// dumping due to air resistence, 0-1, 0 = no dumping

	ClothConstraint* constraints;
	size_t constraintCount;

	xprBuffer* vertexBuffer;
	xprBuffer* indexBuffer;

} Cloth;

Cloth* Cloth_new(float width, float height, size_t segmentCount);

void Cloth_free(Cloth* self);

void Cloth_addForceToAll(Cloth* self, const xprVec3* const force);

void Cloth_timeStep(Cloth* self);

void Cloth_draw(Cloth* self);

#endif	// __EXAMPLE_CLOTH_H__
