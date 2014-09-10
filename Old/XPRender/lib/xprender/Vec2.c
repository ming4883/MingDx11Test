#include "Vec2.h"

#include <math.h>

XprVec2 xprVec2(float x, float y)
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


void xprVec2_set(XprVec2* _out, float x, float y)
{
	if(nullptr == _out)
		return;

	_out->x = x;
	_out->y = y;
}

XprBool xprVec2IsEqual(const XprVec2* a, const XprVec2* b, float epsilon)
{
	float ex = a->x - b->x;
	float ey = a->y - b->y;

	if(ex * ex + ey * ey < epsilon)
		return XprTrue;

	return XprFalse;
}

XprVec2* xprVec2Add(XprVec2* _out, const XprVec2* a, const XprVec2* b)
{
	_out->x = a->x + b->x;
	_out->y = a->y + b->y;
	return _out;
}

XprVec2* xprVec2Sub(XprVec2* _out, const XprVec2* a, const XprVec2* b)
{
	_out->x = a->x - b->x;
	_out->y = a->y - b->y;
	return _out;
}

XprVec2* xprVec2Mult(XprVec2* _out, const XprVec2* a, const XprVec2* b)
{
	_out->x = a->x * b->x;
	_out->y = a->y * b->y;
	return _out;
}

XprVec2* xprVec2MultS(XprVec2* _out, const XprVec2* a, float b)
{
	_out->x = a->x * b;
	_out->y = a->y * b;
	return _out;
}

float xprVec2Dot(const XprVec2* a, const XprVec2* b)
{
	return (a->x * b->x) + (a->y * b->y);
}

float xprVec2SqLength(const XprVec2* a)
{
	return xprVec2Dot(a, a);
}

float xprVec2Length(const XprVec2* a)
{
	return sqrtf(xprVec2SqLength(a));
}

float xprVec2Distance(const XprVec2* a, const XprVec2* b)
{
	XprVec2 diff;
	xprVec2Sub(&diff, a, b);
	return xprVec2Length(&diff);
}

float xprVec2Normalize(XprVec2* a)
{
	float len = xprVec2Length(a);
	
	if(len > 1e-5f || len < -1e-5f)
	{
		float inv_len = 1 / len;

		a->x *= inv_len;
		a->y *= inv_len;
	}

	return len;
}

XprVec2 xprVec2NormalizedCopy(const XprVec2* a)
{
	XprVec2 _out = *a;
	xprVec2Normalize(&_out);
	return _out;
}
