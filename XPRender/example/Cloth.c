#include "Cloth.h"

#include "Mesh.h"
#include "Sphere.h"
#include "../lib/xprender/Buffer.h"
#include <stdio.h>

void Cloth_makeConstraint(Cloth* self, size_t x0, size_t y0, size_t x1, size_t y1)
{
	size_t idx0 = y0 * self->segmentCount + x0;
	size_t idx1 = y1 * self->segmentCount + x1;

	ClothConstraint* constraint = &self->constraints[self->constraintCount];
	constraint->pIdx[0] = idx0;
	constraint->pIdx[1] = idx1;
	constraint->restDistance = xprVec3_distance(&self->p[idx0], &self->p[idx1]);

	++self->constraintCount;
}

Cloth* Cloth_new(float width, float height, const float offset[3], size_t segmentCount)
{
	size_t r, c;
	unsigned short* mapped = nullptr;

	Cloth* self = (Cloth*)malloc(sizeof(Cloth));
	self->segmentCount = segmentCount;
	self->g = xprVec3_(0, -1, 0);
	self->timeStep = 0;
	self->damping = 0;

	self->p = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->p2 = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->a = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->fixPos = (xprVec3*)malloc(sizeof(xprVec3) * segmentCount * segmentCount);
	self->fixed = (xprBool*)malloc(sizeof(xprBool) * segmentCount * segmentCount);

	self->mesh = Mesh_new(segmentCount * segmentCount, (segmentCount-1) * (segmentCount-1) * 6);
	
	mapped = (unsigned short*)xprBuffer_map(self->mesh->indexBuffer, xprBufferMapAccess_Write);
	for(r=0; r<segmentCount-1; ++r)
	{
		for(c=0; c<segmentCount-1; ++c)
		{
			unsigned short p0 = (unsigned short)(r * segmentCount + c);
			unsigned short p1 = (unsigned short)((r+1) * segmentCount + c);
			unsigned short p2 = (unsigned short)(r * segmentCount + (c+1));
			unsigned short p3 = (unsigned short)((r+1) * segmentCount + (c+1));

			(*mapped++) = p0;
			(*mapped++) = p1;
			(*mapped++) = p2;

			(*mapped++) = p3;
			(*mapped++) = p2;
			(*mapped++) = p1;
		}
	}
	xprBuffer_unmap(self->mesh->indexBuffer);

	for(r=0; r<segmentCount; ++r)
	{
		float y = offset[2] - height * (float)r / segmentCount;
		for(c=0; c<segmentCount; ++c)
		{
			size_t i = r * segmentCount + c;
			float x = offset[0] + width * (float)c / segmentCount;
			xprVec3 p = xprVec3_(x, offset[1], y);

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

			
			if(r+2 < segmentCount)
				Cloth_makeConstraint(self, r, c, r+2, c);

			if(c+2 < segmentCount)
				Cloth_makeConstraint(self, r, c, r, c+2);

			if(r+2 < segmentCount && c+2 < segmentCount)
			{
				Cloth_makeConstraint(self, r, c, r+2, c+2);
				Cloth_makeConstraint(self, r+2, c, r, c+2);
			}
		}
	}

	return self;
}

void Cloth_free(Cloth* self)
{
	Mesh_free(self->mesh);
	free(self->p);
	free(self->p2);
	free(self->a);
	free(self->constraints);
	free(self->fixed);
	free(self->fixPos);
	free(self);
}

void Cloth_addForceToAll(Cloth* self, const float force[3])
{
	size_t i, cnt = self->segmentCount * self->segmentCount;
	for(i = 0; i < cnt; ++i)
	{
		xprVec3* a = &self->a[i];
		xprVec3_add(a, a, (xprVec3*)force);
	}
}

void Cloth_updateMesh(Cloth* self)
{
	xprBuffer_update(self->mesh->vertexBuffer, 0, self->mesh->vertexBuffer->sizeInBytes, self->p);

	{
		size_t r, c;

		xprVec3* normals = (xprVec3*)xprBuffer_map(self->mesh->normalBuffer, xprBufferMapAccess_Write);

		for(r = 0; r < self->segmentCount; ++r)
		{
			for(c = 0; c < self->segmentCount; ++c)
			{
				xprVec3 n = *xprVec3_c000();
				int cnt = 0;

				xprVec3* p1, *p2, *p3;
				xprVec3 v1, v2, n2;
				p1 = &self->p[r * self->segmentCount + c];

				if(r>0 && c>0)
				{
					p2 = &self->p[(r) * self->segmentCount + (c-1)];
					p3 = &self->p[(r-1) * self->segmentCount + c];
					
					xprVec3_sub(&v1, p2, p1);
					xprVec3_sub(&v2, p3, p1);
					xprVec3_cross(&n2, &v1, &v2);
					xprVec3_normalize(&n2);
					xprVec3_add(&n, &n, &n2);
					++cnt;
				}

				if(r>0 && c<(self->segmentCount-1))
				{
					p2 = &self->p[(r-1) * self->segmentCount + c];
					p3 = &self->p[(r) * self->segmentCount + (c+1)];
					
					xprVec3_sub(&v1, p2, p1);
					xprVec3_sub(&v2, p3, p1);
					xprVec3_cross(&n2, &v1, &v2);
					xprVec3_normalize(&n2);
					xprVec3_add(&n, &n, &n2);
					++cnt;
				}

				if(r<(self->segmentCount-1) && c<(self->segmentCount-1))
				{
					p2 = &self->p[(r) * self->segmentCount + (c+1)];
					p3 = &self->p[(r+1) * self->segmentCount + c];
					
					xprVec3_sub(&v1, p2, p1);
					xprVec3_sub(&v2, p3, p1);
					xprVec3_cross(&n2, &v1, &v2);
					xprVec3_normalize(&n2);
					xprVec3_add(&n, &n, &n2);
					++cnt;
				}

				if(r<(self->segmentCount-1) && c>0)
				{
					p2 = &self->p[(r+1) * self->segmentCount + c];
					p3 = &self->p[(r) * self->segmentCount + (c-1)];
					
					xprVec3_sub(&v1, p2, p1);
					xprVec3_sub(&v2, p3, p1);
					xprVec3_cross(&n2, &v1, &v2);
					xprVec3_normalize(&n2);
					xprVec3_add(&n, &n, &n2);
					++cnt;
				}
			
				if(cnt > 0)
					xprVec3_normalize(&n);
				n.x *= -1;
				n.y *= -1;
				n.z *= -1;
				(*normals++) = n;
			}
		}

		xprBuffer_unmap(self->mesh->normalBuffer);
	}
}

void Cloth_verletIntegration(Cloth* self)
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;

	float t2 = self->timeStep * self->timeStep;

	// Verlet Integration
	for(i = 0; i < cnt; ++i)
	{
		xprVec3* x = &self->p[i];
		xprVec3* oldx = &self->p2[i];
		xprVec3* a = &self->a[i];

		xprVec3 tmp = *x;
		xprVec3 dx;
		xprVec3 da;
		xprVec3_multS(&da, a, t2);
		xprVec3_sub(&dx, x, oldx);
		xprVec3_multS(&dx, &dx, 1-self->damping);
		xprVec3_add(&dx, &dx, &da);

		xprVec3_add(x, x, &dx);

		*a = *xprVec3_c000();
		*oldx = tmp;
	}
}

void Cloth_satisfyConstraints(Cloth* self)
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;

	for(i = 0; i < cnt; ++i)
	{
		if(xprTrue == self->fixed[i])
			self->p[i] = self->fixPos[i];
	}

	for(i=0; i<self->constraintCount; ++i)
	{
		ClothConstraint* c = &self->constraints[i];
		xprVec3* x1 = &self->p[c->pIdx[0]];
		xprVec3* x2 = &self->p[c->pIdx[1]];
		xprVec3 delta;
		float scale;
		xprVec3_sub(&delta, x2, x1);
		
		scale = (1 - c->restDistance / xprVec3_length(&delta)) * 0.5f;
		xprVec3_multS(&delta, &delta, scale);

		xprVec3_add(x1, x1, &delta);
		xprVec3_sub(x2, x2, &delta);
	}
}

const float collisionEpsilon = 1e-1f;

void Cloth_collideWithSphere(Cloth* self, const Sphere* sphere)
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;

	for(i = 0; i < cnt; ++i)
	{
		xprVec3* x = &self->p[i];
		xprVec3 d;
		float l;

		xprVec3_sub(&d, x, &sphere->center);
		l = xprVec3_length(&d);

		if(l < sphere->radius)
		{
			//xprVec3_normalize(&d);
			xprVec3_multS(&d, &d, (sphere->radius - (l - collisionEpsilon)) / (l - collisionEpsilon));
			xprVec3_add(x, x, &d);
		}
	}
}

void Cloth_collideWithPlane(Cloth* self, const float normal[3], const float point[3])
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;
	float d = -xprVec3_dot((xprVec3*)normal, (xprVec3*)point);

	for(i = 0; i < cnt; ++i)
	{
		xprVec3* x = &self->p[i];
		float l = xprVec3_dot((xprVec3*)normal, x) + d;
		if(l < collisionEpsilon)
		{
			xprVec3 dx;
			xprVec3_multS(&dx, (xprVec3*)normal, -(l - collisionEpsilon));
			xprVec3_add(x, x, &dx);
		}
	}
}