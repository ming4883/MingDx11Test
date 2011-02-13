#include "Cloth.h"

#include "Mesh.h"
#include "Sphere.h"

#include "../lib/xprender/Vec2.h"

#include <stdio.h>

void Cloth_makeConstraint(Cloth* self, size_t x0, size_t y0, size_t x1, size_t y1)
{
	size_t idx0 = y0 * self->segmentCount + x0;
	size_t idx1 = y1 * self->segmentCount + x1;

	ClothConstraint* constraint = &self->constraints[self->constraintCount];
	constraint->pIdx[0] = idx0;
	constraint->pIdx[1] = idx1;
	constraint->restDistance = xprVec3Distance(&self->p[idx0], &self->p[idx1]);

	++self->constraintCount;
}

Cloth* Cloth_new(float width, float height, const XprVec3* offset, size_t segmentCount)
{
	size_t r, c;
	unsigned short* mapped = nullptr;
	XprVec2* uv = nullptr;

	Cloth* self = (Cloth*)malloc(sizeof(Cloth));
	self->segmentCount = segmentCount;
	self->g = xprVec3(0, -1, 0);
	self->timeStep = 0;
	self->damping = 0;

	self->p = (XprVec3*)malloc(sizeof(XprVec3) * segmentCount * segmentCount);
	self->p2 = (XprVec3*)malloc(sizeof(XprVec3) * segmentCount * segmentCount);
	self->a = (XprVec3*)malloc(sizeof(XprVec3) * segmentCount * segmentCount);
	self->fixPos = (XprVec3*)malloc(sizeof(XprVec3) * segmentCount * segmentCount);
	self->fixed = (XprBool*)malloc(sizeof(XprBool) * segmentCount * segmentCount);

	self->mesh = Mesh_alloc();
	Mesh_init(self->mesh, segmentCount * segmentCount, (segmentCount-1) * (segmentCount-1) * 6);
	
	mapped = (unsigned short*)self->mesh->index.buffer;
	uv = (XprVec2*)self->mesh->texcoord[0].buffer;

	for(r=0; r<segmentCount-1; ++r) {
		for(c=0; c<segmentCount-1; ++c) {
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

	for(r=0; r<segmentCount; ++r) {
		float y = offset->v[2] - height * (float)r / segmentCount;
		for(c=0; c<segmentCount; ++c) {
			size_t i = r * segmentCount + c;
			float x = offset->v[0] + width * (float)c / segmentCount;
			XprVec3 p = xprVec3(x, offset->v[1], y);

			self->p[i] = p;
			self->p2[i] = p;
			self->fixPos[i] = p;
			self->fixed[i] = XprFalse;

			self->a[i].x = 0;
			self->a[i].y = 0;
			self->a[i].z = 0;

			uv[i].x = width  * (float)c / segmentCount;
			uv[i].y = height * (float)r / segmentCount;
		}
	}

	self->fixed[0] = XprTrue;
	self->fixed[segmentCount-1] = XprTrue;

	// setup constraints
	self->constraints = (ClothConstraint*)malloc(sizeof(ClothConstraint) * self->segmentCount * self->segmentCount * 8);
	self->constraintCount = 0;

	for(r=0; r<segmentCount; ++r) {
		for(c=0; c<segmentCount; ++c) {
			if(r+1 < segmentCount)
				Cloth_makeConstraint(self, r, c, r+1, c);

			if(c+1 < segmentCount)
				Cloth_makeConstraint(self, r, c, r, c+1);

			if(r+1 < segmentCount && c+1 < segmentCount) {
				Cloth_makeConstraint(self, r, c, r+1, c+1);
				Cloth_makeConstraint(self, r+1, c, r, c+1);
			}
			
			if(r+2 < segmentCount)
				Cloth_makeConstraint(self, r, c, r+2, c);

			if(c+2 < segmentCount)
				Cloth_makeConstraint(self, r, c, r, c+2);

			if(r+2 < segmentCount && c+2 < segmentCount) {
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
		xprVec3Add(a, a, (XprVec3*)force);
	}
}

void Cloth_updateMesh(Cloth* self)
{
	size_t r, c;

	XprVec3* normals = (XprVec3*)self->mesh->normal.buffer;

	for(r = 0; r < self->segmentCount; ++r) {
		for(c = 0; c < self->segmentCount; ++c) {
			XprVec3 n = *XprVec3_c000();
			int cnt = 0;

			XprVec3* p1, *p2, *p3;
			XprVec3 v1, v2, n2;
			p1 = &self->p[r * self->segmentCount + c];

			if(r>0 && c>0) {
				p2 = &self->p[(r) * self->segmentCount + (c-1)];
				p3 = &self->p[(r-1) * self->segmentCount + c];
				
				xprVec3Sub(&v1, p2, p1);
				xprVec3Sub(&v2, p3, p1);
				xprVec3Cross(&n2, &v1, &v2);
				xprVec3Normalize(&n2);
				xprVec3Add(&n, &n, &n2);
				++cnt;
			}

			if(r>0 && c<(self->segmentCount-1)) {
				p2 = &self->p[(r-1) * self->segmentCount + c];
				p3 = &self->p[(r) * self->segmentCount + (c+1)];
				
				xprVec3Sub(&v1, p2, p1);
				xprVec3Sub(&v2, p3, p1);
				xprVec3Cross(&n2, &v1, &v2);
				xprVec3Normalize(&n2);
				xprVec3Add(&n, &n, &n2);
				++cnt;
			}

			if(r<(self->segmentCount-1) && c<(self->segmentCount-1)) {
				p2 = &self->p[(r) * self->segmentCount + (c+1)];
				p3 = &self->p[(r+1) * self->segmentCount + c];
				
				xprVec3Sub(&v1, p2, p1);
				xprVec3Sub(&v2, p3, p1);
				xprVec3Cross(&n2, &v1, &v2);
				xprVec3Normalize(&n2);
				xprVec3Add(&n, &n, &n2);
				++cnt;
			}

			if(r<(self->segmentCount-1) && c>0) {
				p2 = &self->p[(r+1) * self->segmentCount + c];
				p3 = &self->p[(r) * self->segmentCount + (c-1)];
				
				xprVec3Sub(&v1, p2, p1);
				xprVec3Sub(&v2, p3, p1);
				xprVec3Cross(&n2, &v1, &v2);
				xprVec3Normalize(&n2);
				xprVec3Add(&n, &n, &n2);
				++cnt;
			}
		
			if(cnt > 0)
				xprVec3Normalize(&n);
			n.x *= -1;
			n.y *= -1;
			n.z *= -1;
			(*normals++) = n;
		}
	}
	
	memcpy(self->mesh->vertex.buffer, self->p, self->mesh->vertex.sizeInBytes);

	Mesh_commit(self->mesh);
}

void Cloth_verletIntegration(Cloth* self)
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;

	float t2 = self->timeStep * self->timeStep;

	// Verlet Integration
	for(i = 0; i < cnt; ++i) {
		XprVec3* x = &self->p[i];
		XprVec3* oldx = &self->p2[i];
		XprVec3* a = &self->a[i];

		XprVec3 tmp = *x;
		XprVec3 dx;
		XprVec3 da;
		xprVec3MultS(&da, a, t2);
		xprVec3Sub(&dx, x, oldx);
		xprVec3MultS(&dx, &dx, 1-self->damping);
		xprVec3Add(&dx, &dx, &da);

		xprVec3Add(x, x, &dx);

		*a = *XprVec3_c000();
		*oldx = tmp;
	}
}

void Cloth_satisfyConstraints(Cloth* self)
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;

	for(i = 0; i < cnt; ++i) {
		if(XprTrue == self->fixed[i])
			self->p[i] = self->fixPos[i];
	}

	for(i=0; i<self->constraintCount; ++i) {
		ClothConstraint* c = &self->constraints[i];
		XprVec3* x1 = &self->p[c->pIdx[0]];
		XprVec3* x2 = &self->p[c->pIdx[1]];
		XprVec3 delta;
		float scale;
		xprVec3Sub(&delta, x2, x1);
		
		scale = (1 - c->restDistance / xprVec3Length(&delta)) * 0.5f;
		xprVec3MultS(&delta, &delta, scale);

		xprVec3Add(x1, x1, &delta);
		xprVec3Sub(x2, x2, &delta);
	}
}

const float collisionEpsilon = 1e-1f;

void Cloth_collideWithSphere(Cloth* self, const Sphere* sphere)
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;

	for(i = 0; i < cnt; ++i) {
		XprVec3* x = &self->p[i];
		XprVec3 d;
		float l;

		xprVec3Sub(&d, x, &sphere->center);
		l = xprVec3Length(&d);

		if(l < sphere->radius) {
			xprVec3MultS(&d, &d, (sphere->radius - (l - collisionEpsilon)) / (l - collisionEpsilon));
			xprVec3Add(x, x, &d);
		}
	}
}

void Cloth_collideWithPlane(Cloth* self, const XprVec3* normal, const XprVec3* point)
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;
	float d = -xprVec3Dot(normal, point);

	for(i = 0; i < cnt; ++i) {
		XprVec3* x = &self->p[i];
		float l = xprVec3Dot((XprVec3*)normal, x) + d;
		if(l < collisionEpsilon) {
			XprVec3 dx;
			xprVec3MultS(&dx, normal, -(l - collisionEpsilon));
			xprVec3Add(x, x, &dx);
		}
	}
}