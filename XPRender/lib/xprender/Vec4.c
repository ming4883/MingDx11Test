#include "Vec4.h"
#include "Vec3.h"

#include <math.h>

XprVec4 xprVec4(float x, float y, float z, float w)
{
	XprVec4 _out;
	_out.x = x;
	_out.y = y;
	_out.z = z;
	_out.w = w;
	return _out;
}

XprVec4 xprVec4FromVec3(const XprVec3* xyz, float w)
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

void xprVec4Set(XprVec4* _out, float x, float y, float z, float w)
{
	if(nullptr == _out)
		return;

	_out->x = x;
	_out->y = y;
	_out->z = z;
	_out->w = w;
}

XprBool xprVec4IsEqual(const XprVec4* a, const XprVec4* b, float epsilon)
{
	float ex = a->x - b->x;
	float ey = a->y - b->y;
	float ez = a->z - b->z;
	float ew = a->w - b->w;

	if(ex * ex + ey * ey + ez * ez + ew * ew < epsilon)
		return XprTrue;

	return XprFalse;
}

XprVec4* xprVec4Add(XprVec4* _out, const XprVec4* a, const XprVec4* b)
{
	_out->x = a->x + b->x;
	_out->y = a->y + b->y;
	_out->z = a->z + b->z;
	_out->w = a->w + b->w;
	return _out;
}

XprVec4* xprVec4Sub(XprVec4* _out, const XprVec4* a, const XprVec4* b)
{
	_out->x = a->x - b->x;
	_out->y = a->y - b->y;
	_out->z = a->z - b->z;
	_out->w = a->w - b->w;
	return _out;
}

XprVec4* xprVec4Mult(XprVec4* _out, const XprVec4* a, const XprVec4* b)
{
	_out->x = a->x * b->x;
	_out->y = a->y * b->y;
	_out->z = a->z * b->z;
	_out->w = a->w * b->w;
	return _out;
}

XprVec4* xprVec4MultS(XprVec4* _out, const XprVec4* a, float b)
{
	_out->x = a->x * b;
	_out->y = a->y * b;
	_out->z = a->z * b;
	_out->w = a->w * b;
	return _out;
}

float xprVec4Dot(const XprVec4* a, const XprVec4* b)
{
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z) + (a->w * b->w);
}

float xprVec4SqLength(const XprVec4* a)
{
	return xprVec4Dot(a, a);
}

float xprVec4Length(const XprVec4* a)
{
	return sqrtf(xprVec4SqLength(a));
}

float xprVec4Distance(const XprVec4* a, const XprVec4* b)
{
	XprVec4 diff;
	xprVec4Sub(&diff, a, b);
	return xprVec4Length(&diff);
}

float xprVec4Normalize(XprVec4* a)
{
	float len = xprVec4Length(a);
	
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

XprVec4 xprVec4NormalizedCopy(const XprVec4* a)
{
	XprVec4 _out = *a;
	xprVec4Normalize(&_out);
	return _out;
}
