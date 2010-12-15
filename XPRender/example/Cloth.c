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
	constraint->restDistance = XprVec3_distance(&self->p[idx0], &self->p[idx1]);

	++self->constraintCount;
}

Cloth* Cloth_new(float width, float height, const XprVec3* offset, size_t segmentCount)
{
	size_t r, c;
	unsigned short* mapped = nullptr;

	Cloth* self = (Cloth*)malloc(sizeof(Cloth));
	self->segmentCount = segmentCount;
	self->g = XprVec3_(0, -1, 0);
	self->timeStep = 0;
	self->damping = 0;

	self->p = (XprVec3*)malloc(sizeof(XprVec3) * segmentCount * segmentCount);
	self->p2 = (XprVec3*)malloc(sizeof(XprVec3) * segmentCount * segmentCount);
	self->a = (XprVec3*)malloc(sizeof(XprVec3) * segmentCount * segmentCount);
	self->fixPos = (XprVec3*)malloc(sizeof(XprVec3) * segmentCount * segmentCount);
	self->fixed = (XprBool*)malloc(sizeof(XprBool) * segmentCount * segmentCount);

	self->mesh = Mesh_new(segmentCount * segmentCount, (segmentCount-1) * (segmentCount-1) * 6);
	
	mapped = (unsigned short*)XprBuffer_map(self->mesh->indexBuffer, XprBufferMapAccess_Write);
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
	XprBuffer_unmap(self->mesh->indexBuffer);

	for(r=0; r<segmentCount; ++r)
	{
		float y = offset->v[2] - height * (float)r / segmentCount;
		for(c=0; c<segmentCount; ++c)
		{
			size_t i = r * segmentCount + c;
			float x = offset->v[0] + width * (float)c / segmentCount;
			XprVec3 p = XprVec3_(x, offset->v[1], y);

			self->p[i] = p;
			self->p2[i] = p;
			self->fixPos[i] = p;
			self->fixed[i] = XprFalse;

			self->a[i].x = 0;
			self->a[i].y = 0;
			self->a[i].z = 0;
		}
	}

	self->fixed[0] = XprTrue;
	self->fixed[segmentCount-1] = XprTrue;

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

void Cloth_addForceToAll(Cloth* self, const XprVec3* force)
{
	size_t i, cnt = self->segmentCount * self->segmentCount;
	for(i = 0; i < cnt; ++i)
	{
		XprVec3* a = &self->a[i];
		XprVec3_add(a, a, (XprVec3*)force);
	}
}

void Cloth_updateMesh(Cloth* self)
{
	XprBuffer_update(self->mesh->vertexBuffer, 0, self->mesh->vertexBuffer->sizeInBytes, self->p);

	{
		size_t r, c;

		XprVec3* normals = (XprVec3*)XprBuffer_map(self->mesh->normalBuffer, XprBufferMapAccess_Write);

		for(r = 0; r < self->segmentCount; ++r)
		{
			for(c = 0; c < self->segmentCount; ++c)
			{
				XprVec3 n = *XprVec3_c000();
				int cnt = 0;

				XprVec3* p1, *p2, *p3;
				XprVec3 v1, v2, n2;
				p1 = &self->p[r * self->segmentCount + c];

				if(r>0 && c>0)
				{
					p2 = &self->p[(r) * self->segmentCount + (c-1)];
					p3 = &self->p[(r-1) * self->segmentCount + c];
					
					XprVec3_sub(&v1, p2, p1);
					XprVec3_sub(&v2, p3, p1);
					XprVec3_cross(&n2, &v1, &v2);
					XprVec3_normalize(&n2);
					XprVec3_add(&n, &n, &n2);
					++cnt;
				}

				if(r>0 && c<(self->segmentCount-1))
				{
					p2 = &self->p[(r-1) * self->segmentCount + c];
					p3 = &self->p[(r) * self->segmentCount + (c+1)];
					
					XprVec3_sub(&v1, p2, p1);
					XprVec3_sub(&v2, p3, p1);
					XprVec3_cross(&n2, &v1, &v2);
					XprVec3_normalize(&n2);
					XprVec3_add(&n, &n, &n2);
					++cnt;
				}

				if(r<(self->segmentCount-1) && c<(self->segmentCount-1))
				{
					p2 = &self->p[(r) * self->segmentCount + (c+1)];
					p3 = &self->p[(r+1) * self->segmentCount + c];
					
					XprVec3_sub(&v1, p2, p1);
					XprVec3_sub(&v2, p3, p1);
					XprVec3_cross(&n2, &v1, &v2);
					XprVec3_normalize(&n2);
					XprVec3_add(&n, &n, &n2);
					++cnt;
				}

				if(r<(self->segmentCount-1) && c>0)
				{
					p2 = &self->p[(r+1) * self->segmentCount + c];
					p3 = &self->p[(r) * self->segmentCount + (c-1)];
					
					XprVec3_sub(&v1, p2, p1);
					XprVec3_sub(&v2, p3, p1);
					XprVec3_cross(&n2, &v1, &v2);
					XprVec3_normalize(&n2);
					XprVec3_add(&n, &n, &n2);
					++cnt;
				}
			
				if(cnt > 0)
					XprVec3_normalize(&n);
				n.x *= -1;
				n.y *= -1;
				n.z *= -1;
				(*normals++) = n;
			}
		}

		XprBuffer_unmap(self->mesh->normalBuffer);
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
		XprVec3* x = &self->p[i];
		XprVec3* oldx = &self->p2[i];
		XprVec3* a = &self->a[i];

		XprVec3 tmp = *x;
		XprVec3 dx;
		XprVec3 da;
		XprVec3_multS(&da, a, t2);
		XprVec3_sub(&dx, x, oldx);
		XprVec3_multS(&dx, &dx, 1-self->damping);
		XprVec3_add(&dx, &dx, &da);

		XprVec3_add(x, x, &dx);

		*a = *XprVec3_c000();
		*oldx = tmp;
	}
}

void Cloth_satisfyConstraints(Cloth* self)
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;

	for(i = 0; i < cnt; ++i)
	{
		if(XprTrue == self->fixed[i])
			self->p[i] = self->fixPos[i];
	}

	for(i=0; i<self->constraintCount; ++i)
	{
		ClothConstraint* c = &self->constraints[i];
		XprVec3* x1 = &self->p[c->pIdx[0]];
		XprVec3* x2 = &self->p[c->pIdx[1]];
		XprVec3 delta;
		float scale;
		XprVec3_sub(&delta, x2, x1);
		
		scale = (1 - c->restDistance / XprVec3_length(&delta)) * 0.5f;
		XprVec3_multS(&delta, &delta, scale);

		XprVec3_add(x1, x1, &delta);
		XprVec3_sub(x2, x2, &delta);
	}
}

const float collisionEpsilon = 1e-1f;

void Cloth_collideWithSphere(Cloth* self, const Sphere* sphere)
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;

	for(i = 0; i < cnt; ++i)
	{
		XprVec3* x = &self->p[i];
		XprVec3 d;
		float l;

		XprVec3_sub(&d, x, &sphere->center);
		l = XprVec3_length(&d);

		if(l < sphere->radius)
		{
			//XprVec3_normalize(&d);
			XprVec3_multS(&d, &d, (sphere->radius - (l - collisionEpsilon)) / (l - collisionEpsilon));
			XprVec3_add(x, x, &d);
		}
	}
}

void Cloth_collideWithPlane(Cloth* self, const XprVec3* normal, const XprVec3* point)
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;
	float d = -XprVec3_dot(normal, point);

	for(i = 0; i < cnt; ++i)
	{
		XprVec3* x = &self->p[i];
		float l = XprVec3_dot((XprVec3*)normal, x) + d;
		if(l < collisionEpsilon)
		{
			XprVec3 dx;
			XprVec3_multS(&dx, normal, -(l - collisionEpsilon));
			XprVec3_add(x, x, &dx);
		}
	}
}