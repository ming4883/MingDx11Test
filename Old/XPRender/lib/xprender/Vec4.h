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

XprVec4 xprVec4(float x, float y, float z, float w);
XprVec4 xprVec4FromVec3(const struct XprVec3* xyz, float w);

const XprVec4* XprVec4_c0000();
const XprVec4* XprVec4_c1000();
const XprVec4* XprVec4_c0100();
const XprVec4* XprVec4_c0010();
const XprVec4* XprVec4_c0001();

void xprVec4Set(XprVec4* _out, float x, float y, float z, float w);

XprBool xprVec4IsEqual(const XprVec4* a, const XprVec4* b, float epsilon);

/* return a + b */
XprVec4* xprVec4Add(XprVec4* _out, const XprVec4* a, const XprVec4* b);

/* return a - b */
XprVec4* xprVec4Sub(XprVec4* _out, const XprVec4* a, const XprVec4* b);

/* return a * b */
XprVec4* xprVec4Mult(XprVec4* _out, const XprVec4* a, const XprVec4* b);

/* return a * b */
XprVec4* xprVec4MultS(XprVec4* _out, const XprVec4* a, float b);

/* return a dot b */
float xprVec4Dot(const XprVec4* a, const XprVec4* b);

/* return |a|^2 */
float xprVec4SqLength(const XprVec4* a);

/* return |a| */
float xprVec4Length(const XprVec4* a);

/* return |a-b| */
float xprVec4Distance(const XprVec4* a, const XprVec4* b);

/* normalize a and return |a| */
float xprVec4Normalize(XprVec4* a);

/* return normalized copy of a */
XprVec4 xprVec4NormalizedCopy(const XprVec4* a);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_VEC4_H__
