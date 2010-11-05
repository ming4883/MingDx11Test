#ifndef __EXAMPLE_CLOTH_H__
#define __EXAMPLE_CLOTH_H__

#include "../lib/xprender/Vec3.h"

typedef struct ClothConstraint
{
	unsigned int pIdx[2];
	float restDistance;
} ClothConstraint;

typedef struct Cloth
{
	unsigned int segmentCount;
	xprVec3* p;		// current positions
	xprVec3* p2;	// last positions
	xprVec3* a;		// accelerations
	xprVec3 g;		// gravity
	float timeStep;	// system time step

	ClothConstraint* constraints;
	unsigned int constraintCount;

} Cloth;

Cloth* Cloth_new(float width, float height, unsigned int segmentCount);

void Cloth_free(Cloth* self);

void Cloth_timeStep(Cloth* self);

void Cloth_draw(Cloth* self);

#endif	// __EXAMPLE_CLOTH_H__
