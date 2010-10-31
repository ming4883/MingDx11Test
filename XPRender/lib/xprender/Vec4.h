#ifndef __XPRENDER_VEC4_H__
#define __XPRENDER_VEC4_H__

#include "Platform.h"

typedef struct xprVec4
{
	float x;
	float y;
	float z;
	float w;
} xprVec4;

typedef struct xprVec3 xprVec3;

xprVec4 xprVec4_(float x, float y, float z, float w);
xprVec4 xprVec4_FromVec3(const xprVec3* xyz, float w);

const xprVec4* xprVec4_c0000();
const xprVec4* xprVec4_c1000();
const xprVec4* xprVec4_c0100();
const xprVec4* xprVec4_c0010();
const xprVec4* xprVec4_c0001();

void xprVec4_Set(xprVec4* _out, float x, float y, float z, float w);

xprBool xprVec4_isEquals(const xprVec4* a, const xprVec4* b, float epsilon);

/* return a + b */
xprVec4 xprVec4_Add(const xprVec4* a, const xprVec4* b);

/* _out += with */
void xprVec4_AddTo(xprVec4* _out, const xprVec4* with);

/* return a - b */
xprVec4 xprVec4_Sub(const xprVec4* a, const xprVec4* b);

/* _out -= with */
void xprVec4_SubTo(xprVec4* _out, const xprVec4* with);

/* return a * b */
xprVec4 xprVec4_Mult(const xprVec4* a, const xprVec4* b);

/* _out *= with */
void xprVec4_MultTo(xprVec4* _out, const xprVec4* with);

/* return a * b */
xprVec4 xprVec4_MultS(const xprVec4* a, float b);

/* _out *= with */
void xprVec4_MultSTo(xprVec4* _out, float with);

/* return a dot b */
float xprVec4_Dot(const xprVec4* a, const xprVec4* b);

/* return |a|^2 */
float xprVec4_SqLength(const xprVec4* a);

/* return |a| */
float xprVec4_Length(const xprVec4* a);

/* normalize a and return |a| */
float xprVec4_Normalize(xprVec4* a);

/* return normalized copy of a */
xprVec4 xprVec4_NormalizedCopy(const xprVec4* a);

#endif	// __XPRENDER_VEC4_H__
