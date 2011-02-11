#ifndef __XPRENDER_VEC3_H__
#define __XPRENDER_VEC3_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XprVec2;

typedef struct XprVec3
{
	union
	{
		struct {
			float x;
			float y;
			float z;
		};
		float v[3];
	};
} XprVec3;

XprVec3 XprVec3_(float x, float y, float z);

XprVec3 XprVec3_fromVec2(const struct XprVec2* xy, float z);

const XprVec3* XprVec3_c000();
const XprVec3* XprVec3_c100();
const XprVec3* XprVec3_c010();
const XprVec3* XprVec3_c001();

void XprVec3_set(XprVec3* _out, float x, float y, float z);

XprBool XprVec3_isEquals(const XprVec3* a, const XprVec3* b, float epsilon);

/* return a + b */
XprVec3* XprVec3_add(XprVec3* _out, const XprVec3* a, const XprVec3* b);

/* return a - b */
XprVec3* XprVec3_sub(XprVec3* _out, const XprVec3* a, const XprVec3* b);

/* return term by term a * b */
XprVec3* XprVec3_mult(XprVec3* _out, const XprVec3* a, const XprVec3* b);

/* return a * b */
XprVec3* XprVec3_multS(XprVec3* _out, const XprVec3* a, float b);

/* return a dot b */
float XprVec3_dot(const XprVec3* a, const XprVec3* b);

/* return |a|^2 */
float XprVec3_sqLength(const XprVec3* a);

/* return |a| */
float XprVec3_length(const XprVec3* a);

/* return |a-b| */
float XprVec3_distance(const XprVec3* a, const XprVec3* b);

/* normalize a and return |a| */
float XprVec3_normalize(XprVec3* a);

/* return normalized copy of a */
XprVec3 XprVec3_normalizedCopy(const XprVec3* a);

/* return a cross b */
XprVec3* XprVec3_cross(XprVec3* _out, const XprVec3* a, const XprVec3* b);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_VEC3_H__
