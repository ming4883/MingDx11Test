#ifndef __XPRENDER_VEC4_H__
#define __XPRENDER_VEC4_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xprVec4
{
	float x;
	float y;
	float z;
	float w;
} xprVec4;

typedef struct xprVec3 xprVec3;

xprVec4 xprVec4_(float x, float y, float z, float w);
xprVec4 xprVec4_FromVec3(const xprVec3* const xyz, float w);

const xprVec4* const xprVec4_c0000();
const xprVec4* const xprVec4_c1000();
const xprVec4* const xprVec4_c0100();
const xprVec4* const xprVec4_c0010();
const xprVec4* const xprVec4_c0001();

void xprVec4_Set(xprVec4* _out, float x, float y, float z, float w);

xprBool xprVec4_isEquals(const xprVec4* const a, const xprVec4* const b, float epsilon);

/* return a + b */
xprVec4* xprVec4_Add(xprVec4* _out, const xprVec4* const a, const xprVec4* const b);

/* return a - b */
xprVec4* xprVec4_Sub(xprVec4* _out, const xprVec4* const a, const xprVec4* const b);

/* return a * b */
xprVec4* xprVec4_Mult(xprVec4* _out, const xprVec4* const a, const xprVec4* const b);

/* return a * b */
xprVec4* xprVec4_MultS(xprVec4* _out, const xprVec4* const a, float b);

/* return a dot b */
float xprVec4_Dot(const xprVec4* const a, const xprVec4* const b);

/* return |a|^2 */
float xprVec4_SqLength(const xprVec4* const a);

/* return |a| */
float xprVec4_Length(const xprVec4* const a);

/* return |a-b| */
float xprVec4_Distance(const xprVec4* const a, const xprVec4* const b);

/* normalize a and return |a| */
float xprVec4_Normalize(xprVec4* a);

/* return normalized copy of a */
xprVec4 xprVec4_NormalizedCopy(const xprVec4* const a);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_VEC4_H__
