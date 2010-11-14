#include "Vec4.h"
#include "Vec3.h"

#include <math.h>

xprVec4 xprVec4_(float x, float y, float z, float w)
{
	xprVec4 _out = {x, y, z, w};
	return _out;
}

xprVec4 xprVec4_FromVec3(const xprVec3* const xyz, float w)
{
	xprVec4 _out = {xyz->x, xyz->y, xyz->z, w};
	return _out;
}

static const xprVec4 _xprVec4_c0000 = {0, 0, 0, 0};
static const xprVec4 _xprVec4_c1000 = {1, 0, 0, 0};
static const xprVec4 _xprVec4_c0100 = {0, 1, 0, 0};
static const xprVec4 _xprVec4_c0010 = {0, 0, 1, 0};
static const xprVec4 _xprVec4_c0001 = {0, 0, 0, 1};

const xprVec4* const xprVec4_c0000()
{
	return &_xprVec4_c0000;
}

const xprVec4* const xprVec4_c1000()
{
	return &_xprVec4_c1000;
}

const xprVec4* const xprVec4_c0100()
{
	return &_xprVec4_c0100;
}

const xprVec4* const xprVec4_c0010()
{
	return &_xprVec4_c0010;
}

const xprVec4* const xprVec4_c0001()
{
	return &_xprVec4_c0001;
}

void xprVec4_set(xprVec4* _out, float x, float y, float z, float w)
{
	if(nullptr == _out)
		return;

	_out->x = x;
	_out->y = y;
	_out->z = z;
	_out->w = w;
}

xprBool xprVec4_isEquals(const xprVec4* const a, const xprVec4* const b, float epsilon)
{
	float ex = a->x - b->x;
	float ey = a->y - b->y;
	float ez = a->z - b->z;
	float ew = a->w - b->w;

	if(ex * ex + ey * ey + ez * ez + ew * ew < epsilon)
		return xprTrue;

	return xprFalse;
}

xprVec4* xprVec4_add(xprVec4* _out, const xprVec4* const a, const xprVec4* const b)
{
	_out->x = a->x + b->x;
	_out->y = a->y + b->y;
	_out->z = a->z + b->z;
	_out->w = a->w + b->w;
	return _out;
}

xprVec4* xprVec4_sub(xprVec4* _out, const xprVec4* const a, const xprVec4* const b)
{
	_out->x = a->x - b->x;
	_out->y = a->y - b->y;
	_out->z = a->z - b->z;
	_out->w = a->w - b->w;
	return _out;
}

xprVec4* xprVec4_mult(xprVec4* _out, const xprVec4* const a, const xprVec4* const b)
{
	_out->x = a->x * b->x;
	_out->y = a->y * b->y;
	_out->z = a->z * b->z;
	_out->w = a->w * b->w;
	return _out;
}

xprVec4* xprVec4_multS(xprVec4* _out, const xprVec4* const a, float b)
{
	_out->x = a->x * b;
	_out->y = a->y * b;
	_out->z = a->z * b;
	_out->w = a->w * b;
	return _out;
}

float xprVec4_dot(const xprVec4* const a, const xprVec4* const b)
{
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z) + (a->w * b->w);
}

float xprVec4_sqLength(const xprVec4* const a)
{
	return xprVec4_dot(a, a);
}

float xprVec4_length(const xprVec4* const a)
{
	return sqrtf(xprVec4_sqLength(a));
}

float xprVec4_distance(const xprVec4* const a, const xprVec4* const b)
{
	xprVec4 diff;
	xprVec4_sub(&diff, a, b);
	return xprVec4_length(&diff);
}

float xprVec4_normalize(xprVec4* a)
{
	float len = xprVec4_length(a);
	
	if(len > 1e-5f || len < -1e-5f)
	{
		float inv_len = 1 / len;

		a->x *= inv_len;
		a->y *= inv_len;
		a->z *= inv_len;
		a->w *= inv_len;
	}

	return len;
}

xprVec4 xprVec4_normalizedCopy(const xprVec4* const a)
{
	xprVec4 _out = *a;
	xprVec4_normalize(&_out);
	return _out;
}
