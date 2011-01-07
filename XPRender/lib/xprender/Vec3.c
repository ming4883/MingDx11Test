#include "Vec3.h"
#include "Vec2.h"
#include <math.h>

XprVec3 XprVec3_(float x, float y, float z)
{
	XprVec3 _out;
	_out.x = x;
	_out.y = y;
	_out.z = z;
	return _out;
}

XprVec3 XprVec3_fromVec2(const struct XprVec2* xy, float z)
{
	XprVec3 _out;
	_out.x = xy->x;
	_out.y = xy->y;
	_out.z = z;
	return _out;
}

static const XprVec3 _XprVec3_c000 = {0, 0, 0};
static const XprVec3 _XprVec3_c100 = {1, 0, 0};
static const XprVec3 _XprVec3_c010 = {0, 1, 0};
static const XprVec3 _XprVec3_c001 = {0, 0, 1};

const XprVec3* XprVec3_c000()
{
	return &_XprVec3_c000;
}

const XprVec3* XprVec3_c100()
{
	return &_XprVec3_c100;
}

const XprVec3* XprVec3_c010()
{
	return &_XprVec3_c010;
}

const XprVec3* XprVec3_c001()
{
	return &_XprVec3_c001;
}

void XprVec3_set(XprVec3* _out, float x, float y, float z)
{
	if(nullptr == _out)
		return;

	_out->x = x;
	_out->y = y;
	_out->z = z;
}

XprBool XprVec3_isEquals(const XprVec3* a, const XprVec3* b, float epsilon)
{
	float ex = a->x - b->x;
	float ey = a->y - b->y;
	float ez = a->z - b->z;

	if(ex * ex + ey * ey + ez * ez < epsilon)
		return XprTrue;

	return XprFalse;
}

XprVec3* XprVec3_add(XprVec3* _out, const XprVec3* a, const XprVec3* b)
{
	_out->x = a->x + b->x;
	_out->y = a->y + b->y;
	_out->z = a->z + b->z;
	return _out;
}

XprVec3* XprVec3_sub(XprVec3* _out, const XprVec3* a, const XprVec3* b)
{
	_out->x = a->x - b->x;
	_out->y = a->y - b->y;
	_out->z = a->z - b->z;
	return _out;
}

XprVec3* XprVec3_mult(XprVec3* _out, const XprVec3* a, const XprVec3* b)
{
	_out->x = a->x * b->x;
	_out->y = a->y * b->y;
	_out->z = a->z * b->z;
	return _out;
}

XprVec3* XprVec3_multS(XprVec3* _out, const XprVec3* a, float b)
{
	_out->x = a->x * b;
	_out->y = a->y * b;
	_out->z = a->z * b;
	return _out;
}

float XprVec3_dot(const XprVec3* a, const XprVec3* b)
{
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

float XprVec3_sqLength(const XprVec3* a)
{
	return XprVec3_dot(a, a);
}

float XprVec3_length(const XprVec3* a)
{
	return sqrtf(XprVec3_sqLength(a));
}

float XprVec3_distance(const XprVec3* a, const XprVec3* b)
{
	XprVec3 diff;
	return XprVec3_length(XprVec3_sub(&diff, a, b));
}

float XprVec3_normalize(XprVec3* a)
{
	float len = XprVec3_length(a);
	
	if(len > 1e-5f || len < -1e-5f)
	{
		float inv_len = 1 / len;

		a->x *= inv_len;
		a->y *= inv_len;
		a->z *= inv_len;
	}

	return len;
}

XprVec3 XprVec3_normalizedCopy(const XprVec3* a)
{
	XprVec3 _out = *a;
	XprVec3_normalize(&_out);
	return _out;
}

XprVec3* XprVec3_cross(XprVec3* _out, const XprVec3* a, const XprVec3* b)
{
	/* reference: http://en.wikipedia.org/wiki/Cross_product#Cross_product_and_handedness */
	_out->x = a->y * b->z - a->z * b->y;
	_out->y = a->z * b->x - a->x * b->z;
	_out->z = a->x * b->y - a->y * b->x;

	return _out;
}
