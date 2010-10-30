#ifndef __XPRENDER_VEC3_H__
#define __XPRENDER_VEC3_H__

#include "Platform.h"

typedef struct xprVec3
{
	float x;
	float y;
	float z;
} xprVec3;

xprVec3 xprVec3_(float x, float y, float z);

const xprVec3* xprVec3_c000();
const xprVec3* xprVec3_c100();
const xprVec3* xprVec3_c010();
const xprVec3* xprVec3_c001();

void xprVec3_Set(xprVec3* _out, float x, float y, float z);

xprBool xprVec3_isEquals(const xprVec3* a, const xprVec3* b, float epsilon);

/* return a + b */
xprVec3 xprVec3_Add(const xprVec3* a, const xprVec3* b);

/* _out += with */
void xprVec3_AddTo(xprVec3* _out, const xprVec3* with);

/* return a - b */
xprVec3 xprVec3_Sub(const xprVec3* a, const xprVec3* b);

/* _out -= with */
void xprVec3_SubTo(xprVec3* _out, const xprVec3* with);

/* return a * b */
xprVec3 xprVec3_Mult(const xprVec3* a, const xprVec3* b);

/* _out *= with */
void xprVec3_MultTo(xprVec3* _out, const xprVec3* with);

/* return a * b */
xprVec3 xprVec3_MultS(const xprVec3* a, float b);

/* _out *= with */
void xprVec3_MultSTo(xprVec3* _out, float with);

/* return a dot b */
float xprVec3_Dot(const xprVec3* a, const xprVec3* b);

/* return |a|^2 */
float xprVec3_SqLength(const xprVec3* a);

/* return |a| */
float xprVec3_Length(const xprVec3* a);

/* normalize a and return |a| */
float xprVec3_Normalize(xprVec3* a);

/* return normalized copy of a */
xprVec3 xprVec3_NormalizedCopy(const xprVec3* a);

/* return a cross b */
xprVec3 xprVec3_Cross(const xprVec3* a, const xprVec3* b);


#endif	// __XPRENDER_VEC3_H__