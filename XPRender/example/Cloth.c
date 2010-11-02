#include "Cloth.h"
#include <GL/glut.h>
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
	self->g = xprVec3_(0, -1, 0);
	self->timeStep = 0;

	for(r=0; r<segmentCount; ++r)
	{
		float z = (float)r / segmentCount;
		for(c=0; c<segmentCount; ++c)
		{
			unsigned int i = r * segmentCount + c;
			float x = (float)c / segmentCount;
			self->p[i].x = x;
			self->p[i].y = 0;
			self->p[i].z = z;

			self->p2[i].x = x;
			self->p2[i].y = 0;
			self->p2[i].z = z;

			self->a[i].x = 0;
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
	unsigned int i;
	unsigned int cnt = self->segmentCount * self->segmentCount;

	float t2 = self->timeStep * self->timeStep;

	// add force
	for(i=0; i<cnt; ++i)
	{
		xprVec3* a = &self->a[i];
		*a = self->g;
	}

	// verlet integration
	for(i=0; i<cnt; ++i)
	{
		xprVec3* x = &self->p[i];
		xprVec3* oldx = &self->p2[i];
		xprVec3* a = &self->a[i];

		xprVec3 tmp = *x;
		xprVec3 dx = xprVec3_Sub(x, oldx);
		xprVec3 da = xprVec3_MultS(a, t2);
		dx = xprVec3_Add(&dx, &da);

		xprVec3_AddTo(x, &dx);

		*oldx = tmp;
	}

	// constraints
	for(i=0; i<cnt; ++i)
	{
		xprVec3* x = &self->p[i];
		if(x->y < -1)
			x->y = -1;
	}
}

void Cloth_draw(Cloth* self)
{
	unsigned int i;
	unsigned int cnt = self->segmentCount * self->segmentCount;

	glBegin(GL_POINTS);
	for(i=0; i<cnt; ++i)
	{
		xprVec3* x = &self->p[i];
		glVertex3f(x->x, x->y, x->z);
	}

	glEnd();
}

