#include "Vec2.h"

#include <math.h>

XprVec2 XprVec2_(float x, float y)
{
	XprVec2 _out;
	_out.x = x;
	_out.y = y;
	return _out;
}

static const XprVec2 _XprVec2_c00 = {0, 0};
static const XprVec2 _XprVec2_c10 = {1, 0};
static const XprVec2 _XprVec2_c01 = {0, 1};

const XprVec2* XprVec2_c00()
{
	return &_XprVec2_c00;
}

const XprVec2* XprVec2_c10()
{
	return &_XprVec2_c10;
}

const XprVec2* XprVec2_c01()
{
	return &_XprVec2_c01;
}


void XprVec2_set(XprVec2* _out, float x, float y)
{
	if(nullptr == _out)
		return;

	_out->x = x;
	_out->y = y;
}

XprBool XprVec2_isEquals(const XprVec2* a, const XprVec2* b, float epsilon)
{
	float ex = a->x - b->x;
	float ey = a->y - b->y;

	if(ex * ex + ey * ey < epsilon)
		return XprTrue;

	return XprFalse;
}

XprVec2* XprVec2_add(XprVec2* _out, const XprVec2* a, const XprVec2* b)
{
	_out->x = a->x + b->x;
	_out->y = a->y + b->y;
	return _out;
}

XprVec2* XprVec2_sub(XprVec2* _out, const XprVec2* a, const XprVec2* b)
{
	_out->x = a->x - b->x;
	_out->y = a->y - b->y;
	return _out;
}

XprVec2* XprVec2_mult(XprVec2* _out, const XprVec2* a, const XprVec2* b)
{
	_out->x = a->x * b->x;
	_out->y = a->y * b->y;
	return _out;
}

XprVec2* XprVec2_multS(XprVec2* _out, const XprVec2* a, float b)
{
	_out->x = a->x * b;
	_out->y = a->y * b;
	return _out;
}

float XprVec2_dot(const XprVec2* a, const XprVec2* b)
{
	return (a->x * b->x) + (a->y * b->y);
}

float XprVec2_sqLength(const XprVec2* a)
{
	return XprVec2_dot(a, a);
}

float XprVec2_length(const XprVec2* a)
{
	return sqrtf(XprVec2_sqLength(a));
}

float XprVec2_distance(const XprVec2* a, const XprVec2* b)
{
	XprVec2 diff;
	XprVec2_sub(&diff, a, b);
	return XprVec2_length(&diff);
}

float XprVec2_normalize(XprVec2* a)
{
	float len = XprVec2_length(a);
	
	if(len > 1e-5f || len < -1e-5f)
	{
		float inv_len = 1 / len;

		a->x *= inv_len;
		a->y *= inv_len;
	}

	return len;
}

XprVec2 XprVec2_normalizedCopy(const XprVec2* a)
{
	XprVec2 _out = *a;
	XprVec2_normalize(&_out);
	return _out;
}
