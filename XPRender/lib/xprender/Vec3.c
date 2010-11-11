#include "Vec3.h"

#include <math.h>

xprVec3 xprVec3_(float x, float y, float z)
{
	xprVec3 _out = {x, y, z};
	return _out;
}

static const xprVec3 _xprVec3_c000 = {0, 0, 0};
static const xprVec3 _xprVec3_c100 = {1, 0, 0};
static const xprVec3 _xprVec3_c010 = {0, 1, 0};
static const xprVec3 _xprVec3_c001 = {0, 0, 1};

const xprVec3* const xprVec3_c000()
{
	return &_xprVec3_c000;
}

const xprVec3* const xprVec3_c100()
{
	return &_xprVec3_c100;
}

const xprVec3* const xprVec3_c010()
{
	return &_xprVec3_c010;
}

const xprVec3* const xprVec3_c001()
{
	return &_xprVec3_c001;
}

void xprVec3_Set(xprVec3* _out, float x, float y, float z)
{
	if(nullptr == _out)
		return;

	_out->x = x;
	_out->y = y;
	_out->z = z;
}

xprBool xprVec3_isEquals(const xprVec3* const a, const xprVec3* const b, float epsilon)
{
	float ex = a->x - b->x;
	float ey = a->y - b->y;
	float ez = a->z - b->z;

	if(ex * ex + ey * ey + ez * ez < epsilon)
		return xprTrue;

	return xprFalse;
}

xprVec3* xprVec3_Add(xprVec3* _out, const xprVec3* const a, const xprVec3* const b)
{
	_out->x = a->x + b->x;
	_out->y = a->y + b->y;
	_out->z = a->z + b->z;
	return _out;
}

xprVec3* xprVec3_Sub(xprVec3* _out, const xprVec3* const a, const xprVec3* const b)
{
	_out->x = a->x - b->x;
	_out->y = a->y - b->y;
	_out->z = a->z - b->z;
	return _out;
}

xprVec3* xprVec3_Mult(xprVec3* _out, const xprVec3* const a, const xprVec3* const b)
{
	_out->x = a->x * b->x;
	_out->y = a->y * b->y;
	_out->z = a->z * b->z;
	return _out;
}

xprVec3* xprVec3_MultS(xprVec3* _out, const xprVec3* const a, float b)
{
	_out->x = a->x * b;
	_out->y = a->y * b;
	_out->z = a->z * b;
	return _out;
}

float xprVec3_Dot(const xprVec3* const a, const xprVec3* const b)
{
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

float xprVec3_SqLength(const xprVec3* const a)
{
	return xprVec3_Dot(a, a);
}

float xprVec3_Length(const xprVec3* const a)
{
	return sqrtf(xprVec3_SqLength(a));
}

float xprVec3_Distance(const xprVec3* const a, const xprVec3* const b)
{
	xprVec3 diff;
	return xprVec3_Length(xprVec3_Sub(&diff, a, b));
}

float xprVec3_Normalize(xprVec3* a)
{
	float len = xprVec3_Length(a);
	
	if(len > 1e-5f || len < -1e-5f)
	{
		float inv_len = 1 / len;

		a->x *= inv_len;
		a->y *= inv_len;
		a->z *= inv_len;
	}

	return len;
}

xprVec3 xprVec3_NormalizedCopy(const xprVec3* const a)
{
	xprVec3 _out = *a;
	xprVec3_Normalize(&_out);
	return _out;
}

xprVec3 xprVec3_Cross(const xprVec3* const a, const xprVec3* const b)
{
	/* reference: http://en.wikipedia.org/wiki/Cross_product#Cross_product_and_handedness */
	xprVec3 _out;
	_out.x = a->y * b->z - a->z * b->y;
	_out.y = a->z * b->x - a->x * b->z;
	_out.z = a->x * b->y - a->y * b->x;

	return _out;
}
