#include "Vec4.h"
#include "Vec3.h"

#include <math.h>

xprVec4 xprVec4_(float x, float y, float z, float w)
{
	xprVec4 _out = {x, y, z, w};
	return _out;
}

xprVec4 xprVec4_FromVec3(const xprVec3* xyz, float w)
{
	xprVec4 _out = {xyz->x, xyz->y, xyz->z, w};
	return _out;
}

static const xprVec4 _xprVec4_c0000 = {0, 0, 0, 0};
static const xprVec4 _xprVec4_c1000 = {1, 0, 0, 0};
static const xprVec4 _xprVec4_c0100 = {0, 1, 0, 0};
static const xprVec4 _xprVec4_c0010 = {0, 0, 1, 0};
static const xprVec4 _xprVec4_c0001 = {0, 0, 0, 1};

const xprVec4* xprVec4_c0000()
{
	return &_xprVec4_c0000;
}

const xprVec4* xprVec4_c1000()
{
	return &_xprVec4_c1000;
}

const xprVec4* xprVec4_c0100()
{
	return &_xprVec4_c0100;
}

const xprVec4* xprVec4_c0010()
{
	return &_xprVec4_c0010;
}

const xprVec4* xprVec4_c0001()
{
	return &_xprVec4_c0001;
}

void xprVec4_Set(xprVec4* _out, float x, float y, float z, float w)
{
	if(nullptr == _out)
		return;

	_out->x = x;
	_out->y = y;
	_out->z = z;
	_out->w = w;
}

xprBool xprVec4_isEquals(const xprVec4* a, const xprVec4* b, float epsilon)
{
	float ex = a->x - b->x;
	float ey = a->y - b->y;
	float ez = a->z - b->z;
	float ew = a->w - b->w;

	if(ex * ex + ey * ey + ez * ez + ew * ew < epsilon)
		return xprTrue;

	return xprFalse;
}

xprVec4 xprVec4_Add(const xprVec4* a, const xprVec4* b)
{
	xprVec4 _out;
	_out.x = a->x + b->x;
	_out.y = a->y + b->y;
	_out.z = a->z + b->z;
	_out.w = a->w + b->w;
	return _out;
}

void xprVec4_AddTo(xprVec4* _out, const xprVec4* with)
{
	_out->x += with->x;
	_out->y += with->y;
	_out->z += with->z;
	_out->w += with->w;
}

xprVec4 xprVec4_Sub(const xprVec4* a, const xprVec4* b)
{
	xprVec4 _out;
	_out.x = a->x - b->x;
	_out.y = a->y - b->y;
	_out.z = a->z - b->z;
	_out.w = a->w - b->w;
	return _out;
}

void xprVec4_SubTo(xprVec4* _out, const xprVec4* with)
{
	_out->x -= with->x;
	_out->y -= with->y;
	_out->z -= with->z;
	_out->w -= with->w;
}

xprVec4 xprVec4_Mult(const xprVec4* a, const xprVec4* b)
{
	xprVec4 _out;
	_out.x = a->x * b->x;
	_out.y = a->y * b->y;
	_out.z = a->z * b->z;
	_out.w = a->w * b->w;
	return _out;
}

void xprVec4_MultTo(xprVec4* _out, const xprVec4* with)
{
	_out->x *= with->x;
	_out->y *= with->y;
	_out->z *= with->z;
	_out->w *= with->w;
}

xprVec4 xprVec4_MultS(const xprVec4* a, float b)
{
	xprVec4 _out;
	_out.x = a->x * b;
	_out.y = a->y * b;
	_out.z = a->z * b;
	_out.w = a->w * b;
	return _out;
}

void xprVec4_MultSTo(xprVec4* _out, float with)
{
	_out->x *= with;
	_out->y *= with;
	_out->z *= with;
	_out->w *= with;
}

float xprVec4_Dot(const xprVec4* a, const xprVec4* b)
{
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z) + (a->w * b->w);
}

float xprVec4_SqLength(const xprVec4* a)
{
	return xprVec4_Dot(a, a);
}

float xprVec4_Length(const xprVec4* a)
{
	return sqrtf(xprVec4_SqLength(a));
}

float xprVec4_Normalize(xprVec4* a)
{
	float len = xprVec4_Length(a);
	
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

xprVec4 xprVec4_NormalizedCopy(const xprVec4* a)
{
	xprVec4 _out = *a;
	xprVec4_Normalize(&_out);
	return _out;
}
