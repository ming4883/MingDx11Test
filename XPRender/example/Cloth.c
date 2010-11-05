#include "Cloth.h"
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>

void Cloth_makeConstraint(Cloth* self, unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1)
{
	unsigned int idx0 = y0 * self->segmentCount + x0;
	unsigned int idx1 = y1 * self->segmentCount + x1;

	ClothConstraint* constraint = &self->constraints[self->constraintCount];
	constraint->pIdx[0] = idx0;
	constraint->pIdx[1] = idx1;
	constraint->restDistance = xprVec3_Distance(&self->p[idx0], &self->p[idx1]);

	++self->constraintCount;
}

Cloth* Cloth_new(float width, float height, unsigned int segmentCount)
{
	unsigned int r, c;

	Cloth* self = (Cloth*)malloc(sizeof(Cloth));
	self->segmentCount = segmentCount;
	self->p = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->p2 = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->a = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->g = xprVec3_(0, -0.1f, 0);
	self->timeStep = 0;

	for(r=0; r<segmentCount; ++r)
	{
		float y = height * (float)r / segmentCount;
		for(c=0; c<segmentCount; ++c)
		{
			unsigned int i = r * segmentCount + c;
			float x = width * (float)c / segmentCount;
			self->p[i].x = x;
			self->p[i].y = y;
			self->p[i].z = 0;

			self->p2[i].x = x;
			self->p2[i].y = y;
			self->p2[i].z = 0;

			self->a[i].x = 0;
			self->a[i].y = 0;
			self->a[i].z = 0;
		}
	}

	// setup constraints
	self->constraints = (ClothConstraint*)malloc(sizeof(ClothConstraint) * (self->segmentCount-1) * (self->segmentCount-1) * 4);
	self->constraintCount = 0;

	for(r=0; r<segmentCount-1; ++r)
	{
		for(c=0; c<segmentCount-1; ++c)
		{
			Cloth_makeConstraint(self, r, c, r+1, c);
			Cloth_makeConstraint(self, r, c, r, c+1);
			Cloth_makeConstraint(self, r, c, r+1, c+1);
			Cloth_makeConstraint(self, r+1, c, r, c+1);
		}
	}

	return self;
}

void Cloth_free(Cloth* self)
{
	free(self->p);
	free(self->p2);
	free(self->a);
	free(self->constraints);
	free(self);
}

void Cloth_timeStep(Cloth* self)
{
	unsigned int i, iter;
	unsigned int cnt = self->segmentCount * self->segmentCount;

	float t2 = self->timeStep * self->timeStep;

	// add force
	xprVec3 force = xprVec3_MultS(&self->g, self->timeStep);
	for(i=0; i<cnt; ++i)
	{
		xprVec3* a = &self->a[i];
		*a = force;
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
	for(iter = 0; iter < 1; ++iter)
	{
		for(i=0; i<self->constraintCount; ++i)
		{
			ClothConstraint* c = &self->constraints[i];
			xprVec3* x1 = &self->p[c->pIdx[0]];
			xprVec3* x2 = &self->p[c->pIdx[1]];
			xprVec3 delta = xprVec3_Sub(x2, x1);
			float sqRestDistance = c->restDistance * c->restDistance;
			float scale = sqRestDistance / (xprVec3_Dot(&delta, &delta) + sqRestDistance) - 0.5f;
			delta = xprVec3_MultS(&delta, scale);
			xprVec3_SubTo(x1, &delta);
			xprVec3_AddTo(x2, &delta);
		}

		//for(i=0; i<cnt; ++i)
		//{
		//	xprVec3* x = &self->p[i];
		//	if(x->y < -1)
		//		x->y = -1;
		//}

		for(i=0; i<self->segmentCount; ++i)
		{
			//self->p[i].x = 0;
			self->p[i].y = 0;
			//self->p[i].z = 0;
		}
	}
}

void Cloth_draw(Cloth* self)
{
	unsigned int i;
	unsigned int cnt = self->segmentCount * self->segmentCount;

	
	glColor3f(1, 0, 0);
	glBegin(GL_LINES);
	for(i=0; i<self->constraintCount; ++i)
	{
		xprVec3* x0 = &self->p[self->constraints[i].pIdx[0]];
		xprVec3* x1 = &self->p[self->constraints[i].pIdx[1]];
		glVertex3f(x0->x, x0->y, x0->z);
		glVertex3f(x1->x, x1->y, x1->z);
	}

	glEnd();

	glColor3f(1, 1, 1);
	glBegin(GL_POINTS);
	for(i=0; i<cnt; ++i)
	{
		xprVec3* x = &self->p[i];
		glVertex3f(x->x, x->y, x->z);
	}

	glEnd();
}

