#include "Cloth.h"

#include "../lib/xprender/Buffer.h"
#include <GL/glew.h>
#include <stdio.h>

void Cloth_makeConstraint(Cloth* self, size_t x0, size_t y0, size_t x1, size_t y1)
{
	size_t idx0 = y0 * self->segmentCount + x0;
	size_t idx1 = y1 * self->segmentCount + x1;

	ClothConstraint* constraint = &self->constraints[self->constraintCount];
	constraint->pIdx[0] = idx0;
	constraint->pIdx[1] = idx1;
	constraint->restDistance = xprVec3_Distance(&self->p[idx0], &self->p[idx1]);

	++self->constraintCount;
}

Cloth* Cloth_new(float width, float height, size_t segmentCount)
{
	size_t r, c;

	Cloth* self = (Cloth*)malloc(sizeof(Cloth));
	self->segmentCount = segmentCount;
	self->g = xprVec3_(0, -1, 0);
	self->timeStep = 0;
	self->dumping = 0;

	self->p = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->p2 = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->a = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->fixPos = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->fixed = (xprBool*)malloc(sizeof(xprBool) * segmentCount * segmentCount);

	self->vertexBuffer = xprBuffer_new(xprBufferType_Vertex, sizeof(xprVec3) * segmentCount * segmentCount, nullptr);
	self->indexBuffer = xprBuffer_new(xprBufferType_Index, sizeof(short) * (segmentCount-1) * (segmentCount-1) * 6, nullptr);

	for(r=0; r<segmentCount; ++r)
	{
		float y = height * (float)r / segmentCount;
		for(c=0; c<segmentCount; ++c)
		{
			size_t i = r * segmentCount + c;
			float x = width * (float)c / segmentCount;
			xprVec3 p = xprVec3_(x, 0, y);

			self->p[i] = p;
			self->p2[i] = p;
			self->fixPos[i] = p;
			self->fixed[i] = xprFalse;

			self->a[i].x = 0;
			self->a[i].y = 0;
			self->a[i].z = 0;
		}
	}

	self->fixed[0] = xprTrue;
	self->fixed[segmentCount-1] = xprTrue;

	// setup constraints
	self->constraints = (ClothConstraint*)malloc(sizeof(ClothConstraint) * self->segmentCount * self->segmentCount * 8);
	self->constraintCount = 0;

	for(r=0; r<segmentCount; ++r)
	{
		for(c=0; c<segmentCount; ++c)
		{
			if(r+1 < segmentCount)
				Cloth_makeConstraint(self, r, c, r+1, c);

			if(c+1 < segmentCount)
				Cloth_makeConstraint(self, r, c, r, c+1);

			if(r+1 < segmentCount && c+1 < segmentCount)
			{
				Cloth_makeConstraint(self, r, c, r+1, c+1);
				Cloth_makeConstraint(self, r+1, c, r, c+1);
			}

			/*
			if(r+2 < segmentCount)
				Cloth_makeConstraint(self, r, c, r+2, c);

			if(c+2 < segmentCount)
				Cloth_makeConstraint(self, r, c, r, c+2);

			if(r+2 < segmentCount && c+2 < segmentCount)
			{
				Cloth_makeConstraint(self, r, c, r+2, c+2);
				Cloth_makeConstraint(self, r+2, c, r, c+2);
			}
			*/
		}
	}

	return self;
}

void Cloth_free(Cloth* self)
{
	xprBuffer_free(self->vertexBuffer);
	xprBuffer_free(self->indexBuffer);

	free(self->p);
	free(self->p2);
	free(self->a);
	free(self->constraints);
	free(self->fixed);
	free(self->fixPos);
	free(self);
}

void Cloth_addForceToAll(Cloth* self, const xprVec3* const force)
{
	size_t i, cnt = self->segmentCount * self->segmentCount;
	for(i = 0; i < cnt; ++i)
	{
		xprVec3* a = &self->a[i];
		xprVec3_AddTo(a, force);
	}
}

void Cloth_timeStep(Cloth* self)
{
	size_t i, iter;
	size_t cnt = self->segmentCount * self->segmentCount;

	float t2 = self->timeStep * self->timeStep;

	// Verlet Integration
	for(i = 0; i < cnt; ++i)
	{
		xprVec3* x = &self->p[i];
		xprVec3* oldx = &self->p2[i];
		xprVec3* a = &self->a[i];

		xprVec3 tmp = *x;
		xprVec3 dx = xprVec3_Sub(x, oldx);
		xprVec3 da = xprVec3_MultS(a, t2);
		dx = xprVec3_MultS(&dx, 1-self->dumping);
		dx = xprVec3_Add(&dx, &da);

		xprVec3_AddTo(x, &dx);

		*a = *xprVec3_c000();
		*oldx = tmp;
	}

	// performs Euler iterations on the constraints
	for(iter = 0; iter < 2; ++iter)
	{
		for(i=0; i<self->constraintCount; ++i)
		{
			ClothConstraint* c = &self->constraints[i];
			xprVec3* x1 = &self->p[c->pIdx[0]];
			xprVec3* x2 = &self->p[c->pIdx[1]];
			xprVec3 delta = xprVec3_Sub(x2, x1);
			
			float scale = (1 - c->restDistance / xprVec3_Length(&delta)) * 0.5f;
			delta = xprVec3_MultS(&delta, scale);

			xprVec3_AddTo(x1, &delta);
			xprVec3_SubTo(x2, &delta);
		}

		for(i = 0; i < cnt; ++i)
		{
			if(xprTrue == self->fixed[i])
				self->p[i] = self->fixPos[i];
		}
	}

	xprBuffer_update(self->vertexBuffer, 0, self->vertexBuffer->sizeInBytes, self->p);
}

void Cloth_draw(Cloth* self)
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;

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
	/*
	glBegin(GL_POINTS);
	for(i=0; i<cnt; ++i)
	{
		xprVec3* x = &self->p[i];
		glVertex3f(x->x, x->y, x->z);
	}
	glEnd();
	*/

	glBindBuffer(GL_ARRAY_BUFFER, self->vertexBuffer->name);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(xprVec3), 0);
	glDrawArrays(GL_POINTS, 0, cnt);
	glDisableClientState(GL_VERTEX_ARRAY);

}

