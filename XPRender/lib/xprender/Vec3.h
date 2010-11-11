#ifndef __XPRENDER_VEC3_H__
#define __XPRENDER_VEC3_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xprVec3
{
	float x;
	float y;
	float z;
} xprVec3;

xprVec3 xprVec3_(float x, float y, float z);

const xprVec3* const xprVec3_c000();
const xprVec3* const xprVec3_c100();
const xprVec3* const xprVec3_c010();
const xprVec3* const xprVec3_c001();

void xprVec3_Set(xprVec3* _out, float x, float y, float z);

xprBool xprVec3_isEquals(const xprVec3* const a, const xprVec3* const b, float epsilon);

/* return a + b */
xprVec3* xprVec3_Add(xprVec3* _out, const xprVec3* const a, const xprVec3* const b);

/* return a - b */
xprVec3* xprVec3_Sub(xprVec3* _out, const xprVec3* const a, const xprVec3* const b);

/* return term by term a * b */
xprVec3* xprVec3_Mult(xprVec3* _out, const xprVec3* const a, const xprVec3* const b);

/* return a * b */
xprVec3* xprVec3_MultS(xprVec3* _out, const xprVec3* const a, float b);

/* return a dot b */
float xprVec3_Dot(const xprVec3* const a, const xprVec3* const b);

/* return |a|^2 */
float xprVec3_SqLength(const xprVec3* const a);

/* return |a| */
float xprVec3_Length(const xprVec3* const a);

/* return |a-b| */
float xprVec3_Distance(const xprVec3* const a, const xprVec3* const b);

/* normalize a and return |a| */
float xprVec3_Normalize(xprVec3* a);

/* return normalized copy of a */
xprVec3 xprVec3_NormalizedCopy(const xprVec3* const a);

/* return a cross b */
xprVec3 xprVec3_Cross(const xprVec3* const a, const xprVec3* const b);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_VEC3_H__
