#ifndef __XPRENDER_VEC3_H__
#define __XPRENDER_VEC3_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XprVec2;
typedef struct XprVec2 XprVec2;

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

XprVec3 xprVec3(float x, float y, float z);

XprVec3 xprVec3FromVec2(const XprVec2* xy, float z);

const XprVec3* XprVec3_c000();
const XprVec3* XprVec3_c100();
const XprVec3* XprVec3_c010();
const XprVec3* XprVec3_c001();

void xprVec3Set(XprVec3* _out, float x, float y, float z);

XprBool xprVec3IsEqual(const XprVec3* a, const XprVec3* b, float epsilon);

/* return a + b */
XprVec3* xprVec3Add(XprVec3* _out, const XprVec3* a, const XprVec3* b);

/* return a - b */
XprVec3* xprVec3Sub(XprVec3* _out, const XprVec3* a, const XprVec3* b);

/* return term by term a * b */
XprVec3* xprVec3Mult(XprVec3* _out, const XprVec3* a, const XprVec3* b);

/* return a * b */
XprVec3* xprVec3MultS(XprVec3* _out, const XprVec3* a, float b);

/* return a dot b */
float xprVec3Dot(const XprVec3* a, const XprVec3* b);

/* return |a|^2 */
float xprVec3SqLength(const XprVec3* a);

/* return |a| */
float xprVec3Length(const XprVec3* a);

/* return |a-b| */
float xprVec3Distance(const XprVec3* a, const XprVec3* b);

/* normalize a and return |a| */
float xprVec3Normalize(XprVec3* a);

/* return normalized copy of a */
XprVec3 xprVec3NormalizedCopy(const XprVec3* a);

/* return a cross b */
XprVec3* xprVec3Cross(XprVec3* _out, const XprVec3* a, const XprVec3* b);

#ifdef __cplusplus
}
#endif

#endif	// __XPRENDER_VEC3_H__
