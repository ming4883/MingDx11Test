#include "Cloth.h"
#include <stdlib.h>
#include <stdio.h>

Cloth* Cloth_new(unsigned int segmentCount)
{
	unsigned int r, c;

	Cloth* self = (Cloth*)malloc(sizeof(Cloth));
	self->segmentCount = segmentCount;
	self->p = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->p2 = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->a = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->g = xprVec3_(0, -10, 0);
	self->timeStep = 0;

	for(r=0; r<segmentCount; ++r)
	{
		for(c=0; c<segmentCount; ++c)
		{
			unsigned int i = r*segmentCount + c;
			float x = (float)i / segmentCount;
			self->p[i].x = x;
			self->p[i].y = 0;
			self->p[i].z = 0;

			self->p2[i].x = x;
			self->p2[i].y = 0;
			self->p2[i].z = 0;

			self->a[i].x = x;
			self->a[i].y = 0;
			self->a[i].z = 0;
		}
	}

	return self;
}

void Cloth_free(Cloth* self)
{
	free(self->p);
	free(self->p2);
	free(self->a);
	free(self);
}

void Cloth_timeStep(Cloth* self)
{
}
