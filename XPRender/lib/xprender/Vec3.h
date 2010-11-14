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

void xprVec3_set(xprVec3* _out, float x, float y, float z);

xprBool xprVec3_isEquals(const xprVec3* const a, const xprVec3* const b, float epsilon);

/* return a + b */
xprVec3* xprVec3_add(xprVec3* _out, const xprVec3* const a, const xprVec3* const b);

/* return a - b */
xprVec3* xprVec3_sub(xprVec3* _out, const xprVec3* const a, const xprVec3* const b);

/* return term by term a * b */
xprVec3* xprVec3_mult(xprVec3* _out, const xprVec3* const a, const xprVec3* const b);

/* return a * b */
xprVec3* xprVec3_multS(xprVec3* _out, const xprVec3* const a, float b);

/* return a dot b */
float xprVec3_dot(const xprVec3* const a, const xprVec3* const b);

/* return |a|^2 */
float xprVec3_sqLength(const xprVec3* const a);

/* return |a| */
float xprVec3_length(const xprVec3* const a);

/* return |a-b| */
float xprVec3_distance(const xprVec3* const a, const xprVec3* const b);

/* normalize a and return |a| */
float xprVec3_normalize(xprVec3* a);

/* return normalized copy of a */
xprVec3 xprVec3_normalizedCopy(const xprVec3* const a);

/* return a cross b */
xprVec3* xprVec3_cross(xprVec3* _out, const xprVec3* const a, const xprVec3* const b);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_VEC3_H__
