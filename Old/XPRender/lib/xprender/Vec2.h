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

XprVec2 xprVec2(float x, float y);

const XprVec2* XprVec2_c00();
const XprVec2* XprVec2_c10();
const XprVec2* XprVec2_c01();

void xprVec2_set(XprVec2* _out, float x, float y);

XprBool xprVec2IsEqual(const XprVec2* a, const XprVec2* b, float epsilon);

/* return a + b */
XprVec2* xprVec2Add(XprVec2* _out, const XprVec2* a, const XprVec2* b);

/* return a - b */
XprVec2* xprVec2Sub(XprVec2* _out, const XprVec2* a, const XprVec2* b);

/* return a * b */
XprVec2* xprVec2Mult(XprVec2* _out, const XprVec2* a, const XprVec2* b);

/* return a * b */
XprVec2* xprVec2MultS(XprVec2* _out, const XprVec2* a, float b);

/* return a dot b */
float xprVec2Dot(const XprVec2* a, const XprVec2* b);

/* return |a|^2 */
float xprVec2SqLength(const XprVec2* a);

/* return |a| */
float xprVec2Length(const XprVec2* a);

/* return |a-b| */
float xprVec2Distance(const XprVec2* a, const XprVec2* b);

/* normalize a and return |a| */
float xprVec2Normalize(XprVec2* a);

/* return normalized copy of a */
XprVec2 xprVec2NormalizedCopy(const XprVec2* a);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_VEC2_H__
