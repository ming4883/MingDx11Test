#ifndef __XPRENDER_VEC4_H__
#define __XPRENDER_VEC4_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XprVec3;

typedef struct XprVec4
{
	union
	{
		struct {
			float x;
			float y;
			float z;
			float w;
		};
		float v[4];
	};
} XprVec4;

XprVec4 XprVec4_(float x, float y, float z, float w);
XprVec4 XprVec4_fromVec3(const struct XprVec3* xyz, float w);

const XprVec4* XprVec4_c0000();
const XprVec4* XprVec4_c1000();
const XprVec4* XprVec4_c0100();
const XprVec4* XprVec4_c0010();
const XprVec4* XprVec4_c0001();

void XprVec4_set(XprVec4* _out, float x, float y, float z, float w);

XprBool XprVec4_isEquals(const XprVec4* a, const XprVec4* b, float epsilon);

/* return a + b */
XprVec4* XprVec4_add(XprVec4* _out, const XprVec4* a, const XprVec4* b);

/* return a - b */
XprVec4* XprVec4_sub(XprVec4* _out, const XprVec4* a, const XprVec4* b);

/* return a * b */
XprVec4* XprVec4_mult(XprVec4* _out, const XprVec4* a, const XprVec4* b);

/* return a * b */
XprVec4* XprVec4_multS(XprVec4* _out, const XprVec4* a, float b);

/* return a dot b */
float XprVec4_dot(const XprVec4* a, const XprVec4* b);

/* return |a|^2 */
float XprVec4_sqLength(const XprVec4* a);

/* return |a| */
float XprVec4_length(const XprVec4* a);

/* return |a-b| */
float XprVec4_distance(const XprVec4* a, const XprVec4* b);

/* normalize a and return |a| */
float XprVec4_normalize(XprVec4* a);

/* return normalized copy of a */
XprVec4 XprVec4_normalizedCopy(const XprVec4* a);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_VEC4_H__
