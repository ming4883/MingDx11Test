#include "Vec4.h"
#include "Vec3.h"

#include <math.h>

XprVec4 XprVec4_(float x, float y, float z, float w)
{
	XprVec4 _out;
	_out.x = x;
	_out.y = y;
	_out.z = z;
	_out.w = w;
	return _out;
}

XprVec4 XprVec4_fromVec3(const XprVec3* xyz, float w)
{
	XprVec4 _out;
	_out.x = xyz->x;
	_out.y = xyz->y;
	_out.z = xyz->z;
	_out.w = w;
	return _out;
}

static const XprVec4 _XprVec4_c0000 = {0, 0, 0, 0};
static const XprVec4 _XprVec4_c1000 = {1, 0, 0, 0};
static const XprVec4 _XprVec4_c0100 = {0, 1, 0, 0};
static const XprVec4 _XprVec4_c0010 = {0, 0, 1, 0};
static const XprVec4 _XprVec4_c0001 = {0, 0, 0, 1};

const XprVec4* XprVec4_c0000()
{
	return &_XprVec4_c0000;
}

const XprVec4* XprVec4_c1000()
{
	return &_XprVec4_c1000;
}

const XprVec4* XprVec4_c0100()
{
	return &_XprVec4_c0100;
}

const XprVec4* XprVec4_c0010()
{
	return &_XprVec4_c0010;
}

const XprVec4* XprVec4_c0001()
{
	return &_XprVec4_c0001;
}

void XprVec4_set(XprVec4* _out, float x, float y, float z, float w)
{
	if(nullptr == _out)
		return;

	_out->x = x;
	_out->y = y;
	_out->z = z;
	_out->w = w;
}

XprBool XprVec4_isEquals(const XprVec4* a, const XprVec4* b, float epsilon)
{
	float ex = a->x - b->x;
	float ey = a->y - b->y;
	float ez = a->z - b->z;
	float ew = a->w - b->w;

	if(ex * ex + ey * ey + ez * ez + ew * ew < epsilon)
		return XprTrue;

	return XprFalse;
}

XprVec4* XprVec4_add(XprVec4* _out, const XprVec4* a, const XprVec4* b)
{
	_out->x = a->x + b->x;
	_out->y = a->y + b->y;
	_out->z = a->z + b->z;
	_out->w = a->w + b->w;
	return _out;
}

XprVec4* XprVec4_sub(XprVec4* _out, const XprVec4* a, const XprVec4* b)
{
	_out->x = a->x - b->x;
	_out->y = a->y - b->y;
	_out->z = a->z - b->z;
	_out->w = a->w - b->w;
	return _out;
}

XprVec4* XprVec4_mult(XprVec4* _out, const XprVec4* a, const XprVec4* b)
{
	_out->x = a->x * b->x;
	_out->y = a->y * b->y;
	_out->z = a->z * b->z;
	_out->w = a->w * b->w;
	return _out;
}

XprVec4* XprVec4_multS(XprVec4* _out, const XprVec4* a, float b)
{
	_out->x = a->x * b;
	_out->y = a->y * b;
	_out->z = a->z * b;
	_out->w = a->w * b;
	return _out;
}

float XprVec4_dot(const XprVec4* a, const XprVec4* b)
{
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z) + (a->w * b->w);
}

float XprVec4_sqLength(const XprVec4* a)
{
	return XprVec4_dot(a, a);
}

float XprVec4_length(const XprVec4* a)
{
	return sqrtf(XprVec4_sqLength(a));
}

float XprVec4_distance(const XprVec4* a, const XprVec4* b)
{
	XprVec4 diff;
	XprVec4_sub(&diff, a, b);
	return XprVec4_length(&diff);
}

float XprVec4_normalize(XprVec4* a)
{
	float len = XprVec4_length(a);
	
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

XprVec4 XprVec4_normalizedCopy(const XprVec4* a)
{
	XprVec4 _out = *a;
	XprVec4_normalize(&_out);
	return _out;
}
