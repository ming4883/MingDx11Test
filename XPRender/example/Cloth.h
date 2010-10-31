#ifndef __EXAMPLE_CLOTH_H__
#define __EXAMPLE_CLOTH_H__

#include "../lib/xprender/Vec3.h"

typedef struct Cloth
{
	unsigned int segmentCount;
	xprVec3* p;		// current positions
	xprVec3* p2;	// last positions
	xprVec3* a;		// accelerations
	xprVec3 g;		// gravity
	float timeStep;	// system time step
} Cloth;

Cloth* Cloth_new(unsigned int segmentCount);

void Cloth_free(Cloth* self);

#endif	// __EXAMPLE_CLOTH_H__
