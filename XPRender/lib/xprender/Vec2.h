#ifndef __XPRENDER_VEC2_H__
#define __XPRENDER_VEC2_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XprVec2
{
	union
	{
		struct {
			float x;
			float y;
		};
		float v[2];
	};
} XprVec2;

XprVec2 XprVec2_(float x, float y);

const XprVec2* XprVec2_c00();
const XprVec2* XprVec2_c10();
const XprVec2* XprVec2_c01();

void XprVec2_set(XprVec2* _out, float x, float y);

XprBool XprVec2_isEquals(const XprVec2* a, const XprVec2* b, float epsilon);

/* return a + b */
XprVec2* XprVec2_add(XprVec2* _out, const XprVec2* a, const XprVec2* b);

/* return a - b */
XprVec2* XprVec2_sub(XprVec2* _out, const XprVec2* a, const XprVec2* b);

/* return a * b */
XprVec2* XprVec2_mult(XprVec2* _out, const XprVec2* a, const XprVec2* b);

/* return a * b */
XprVec2* XprVec2_multS(XprVec2* _out, const XprVec2* a, float b);

/* return a dot b */
float XprVec2_dot(const XprVec2* a, const XprVec2* b);

/* return |a|^2 */
float XprVec2_sqLength(const XprVec2* a);

/* return |a| */
float XprVec2_length(const XprVec2* a);

/* return |a-b| */
float XprVec2_distance(const XprVec2* a, const XprVec2* b);

/* normalize a and return |a| */
float XprVec2_normalize(XprVec2* a);

/* return normalized copy of a */
XprVec2 XprVec2_normalizedCopy(const XprVec2* a);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_VEC2_H__
