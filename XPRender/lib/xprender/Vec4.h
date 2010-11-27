#ifndef __XPRENDER_VEC4_H__
#define __XPRENDER_VEC4_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xprVec4
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
} xprVec4;

struct xprVec3;

xprVec4 xprVec4_(float x, float y, float z, float w);
xprVec4 xprVec4_FromVec3(const struct xprVec3* xyz, float w);

const xprVec4* xprVec4_c0000();
const xprVec4* xprVec4_c1000();
const xprVec4* xprVec4_c0100();
const xprVec4* xprVec4_c0010();
const xprVec4* xprVec4_c0001();

void xprVec4_set(xprVec4* _out, float x, float y, float z, float w);

xprBool xprVec4_isEquals(const xprVec4* a, const xprVec4* b, float epsilon);

/* return a + b */
xprVec4* xprVec4_add(xprVec4* _out, const xprVec4* a, const xprVec4* b);

/* return a - b */
xprVec4* xprVec4_sub(xprVec4* _out, const xprVec4* a, const xprVec4* b);

/* return a * b */
xprVec4* xprVec4_mult(xprVec4* _out, const xprVec4* a, const xprVec4* b);

/* return a * b */
xprVec4* xprVec4_multS(xprVec4* _out, const xprVec4* a, float b);

/* return a dot b */
float xprVec4_dot(const xprVec4* a, const xprVec4* b);

/* return |a|^2 */
float xprVec4_sqLength(const xprVec4* a);

/* return |a| */
float xprVec4_length(const xprVec4* a);

/* return |a-b| */
float xprVec4_distance(const xprVec4* a, const xprVec4* b);

/* normalize a and return |a| */
float xprVec4_normalize(xprVec4* a);

/* return normalized copy of a */
xprVec4 xprVec4_normalizedCopy(const xprVec4* a);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_VEC4_H__
