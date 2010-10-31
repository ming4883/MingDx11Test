#include "Cloth.h"
#include <stdlib.h>

Cloth* Cloth_new(unsigned int segmentCount)
{
	Cloth* self = (Cloth*)malloc(sizeof(Cloth));
	self->segmentCount = segmentCount;
	self->p = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->p2 = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->a = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->g = xprVec3_(0, -10, 0);
	self->timeStep = 0;

	return self;
}

void Cloth_free(Cloth* self)
{
	free(self->p);
	free(self->p2);
	free(self->a);
	free(self);
}
