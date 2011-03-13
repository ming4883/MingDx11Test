#include "Mat44.h"
#include "Vec3.h"
#include "Vec4.h"

#include <math.h>

XprMat44 xprMat44(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33)
{
	XprMat44 self;
	self.m00 = m00; self.m01 = m01; self.m02 = m02; self.m03 = m03;
	self.m10 = m10; self.m11 = m11; self.m12 = m12; self.m13 = m13;
	self.m20 = m20; self.m21 = m21; self.m22 = m22; self.m23 = m23;
	self.m30 = m30; self.m31 = m31; self.m32 = m32; self.m33 = m33;

	return self;
}

void xprMat44Mult(XprMat44* _out, const XprMat44* a, const XprMat44* b)
{
	XprMat44 ta = *a;
	XprMat44 tb = *b;

	_out->m00 = ta.m00 * tb.m00 + ta.m01 * tb.m10 + ta.m02 * tb.m20 + ta.m03 * tb.m30;
	_out->m01 = ta.m00 * tb.m01 + ta.m01 * tb.m11 + ta.m02 * tb.m21 + ta.m03 * tb.m31;
	_out->m02 = ta.m00 * tb.m02 + ta.m01 * tb.m12 + ta.m02 * tb.m22 + ta.m03 * tb.m32;
	_out->m03 = ta.m00 * tb.m03 + ta.m01 * tb.m13 + ta.m02 * tb.m23 + ta.m03 * tb.m33;

	_out->m10 = ta.m10 * tb.m00 + ta.m11 * tb.m10 + ta.m12 * tb.m20 + ta.m13 * tb.m30;
	_out->m11 = ta.m10 * tb.m01 + ta.m11 * tb.m11 + ta.m12 * tb.m21 + ta.m13 * tb.m31;
	_out->m12 = ta.m10 * tb.m02 + ta.m11 * tb.m12 + ta.m12 * tb.m22 + ta.m13 * tb.m32;
	_out->m13 = ta.m10 * tb.m03 + ta.m11 * tb.m13 + ta.m12 * tb.m23 + ta.m13 * tb.m33;

	_out->m20 = ta.m20 * tb.m00 + ta.m21 * tb.m10 + ta.m22 * tb.m20 + ta.m23 * tb.m30;
	_out->m21 = ta.m20 * tb.m01 + ta.m21 * tb.m11 + ta.m22 * tb.m21 + ta.m23 * tb.m31;
	_out->m22 = ta.m20 * tb.m02 + ta.m21 * tb.m12 + ta.m22 * tb.m22 + ta.m23 * tb.m32;
	_out->m23 = ta.m20 * tb.m03 + ta.m21 * tb.m13 + ta.m22 * tb.m23 + ta.m23 * tb.m33;

	_out->m30 = ta.m30 * tb.m00 + ta.m31 * tb.m10 + ta.m32 * tb.m20 + ta.m33 * tb.m30;
	_out->m31 = ta.m30 * tb.m01 + ta.m31 * tb.m11 + ta.m32 * tb.m21 + ta.m33 * tb.m31;
	_out->m32 = ta.m30 * tb.m02 + ta.m31 * tb.m12 + ta.m32 * tb.m22 + ta.m33 * tb.m32;
	_out->m33 = ta.m30 * tb.m03 + ta.m31 * tb.m13 + ta.m32 * tb.m23 + ta.m33 * tb.m33;
}

XprBool xprMat44Inverse(XprMat44* _out, const XprMat44* m)
{
	float invDet;

	XprMat44 t = *m;

	float v0 = t.m20 * t.m31 - t.m21 * t.m30;
	float v1 = t.m20 * t.m32 - t.m22 * t.m30;
	float v2 = t.m20 * t.m33 - t.m23 * t.m30;
	float v3 = t.m21 * t.m32 - t.m22 * t.m31;
	float v4 = t.m21 * t.m33 - t.m23 * t.m31;
	float v5 = t.m22 * t.m33 - t.m23 * t.m32;

	float t00 = + (v5 * t.m11 - v4 * t.m12 + v3 * t.m13);
	float t10 = - (v5 * t.m10 - v2 * t.m12 + v1 * t.m13);
	float t20 = + (v4 * t.m10 - v2 * t.m11 + v0 * t.m13);
	float t30 = - (v3 * t.m10 - v1 * t.m11 + v0 * t.m12);

	float det = t00 * t.m00 + t10 * t.m01 + t20 * t.m02 + t30 * t.m03;

	if(fabsf(det) < 1e-4f)
		return XprFalse;

	invDet = 1 / det;

	_out->m00 = t00 * invDet;
	_out->m10 = t10 * invDet;
	_out->m20 = t20 * invDet;
	_out->m30 = t30 * invDet;

	_out->m01 = - (v5 * t.m01 - v4 * t.m02 + v3 * t.m03) * invDet;
	_out->m11 = + (v5 * t.m00 - v2 * t.m02 + v1 * t.m03) * invDet;
	_out->m21 = - (v4 * t.m00 - v2 * t.m01 + v0 * t.m03) * invDet;
	_out->m31 = + (v3 * t.m00 - v1 * t.m01 + v0 * t.m02) * invDet;

	v0 = t.m10 * t.m31 - t.m11 * t.m30;
	v1 = t.m10 * t.m32 - t.m12 * t.m30;
	v2 = t.m10 * t.m33 - t.m13 * t.m30;
	v3 = t.m11 * t.m32 - t.m12 * t.m31;
	v4 = t.m11 * t.m33 - t.m13 * t.m31;
	v5 = t.m12 * t.m33 - t.m13 * t.m32;

	_out->m02 = + (v5 * t.m01 - v4 * t.m02 + v3 * t.m03) * invDet;
	_out->m12 = - (v5 * t.m00 - v2 * t.m02 + v1 * t.m03) * invDet;
	_out->m22 = + (v4 * t.m00 - v2 * t.m01 + v0 * t.m03) * invDet;
	_out->m32 = - (v3 * t.m00 - v1 * t.m01 + v0 * t.m02) * invDet;

	v0 = t.m21 * t.m10 - t.m20 * t.m11;
	v1 = t.m22 * t.m10 - t.m20 * t.m12;
	v2 = t.m23 * t.m10 - t.m20 * t.m13;
	v3 = t.m22 * t.m11 - t.m21 * t.m12;
	v4 = t.m23 * t.m11 - t.m21 * t.m13;
	v5 = t.m23 * t.m12 - t.m22 * t.m13;

	_out->m03 = - (v5 * t.m01 - v4 * t.m02 + v3 * t.m03) * invDet;
	_out->m13 = + (v5 * t.m00 - v2 * t.m02 + v1 * t.m03) * invDet;
	_out->m23 = - (v4 * t.m00 - v2 * t.m01 + v0 * t.m03) * invDet;
	_out->m33 = + (v3 * t.m00 - v1 * t.m01 + v0 * t.m02) * invDet;

	return XprTrue;
}

void xprMat44Transpose(XprMat44* _out, const XprMat44* m)
{
	XprMat44 t = *m;
	_out->m00 = t.m00; _out->m01 = t.m10; _out->m02 = t.m20; _out->m03 = t.m30;
	_out->m10 = t.m01; _out->m11 = t.m11; _out->m12 = t.m21; _out->m13 = t.m31;
	_out->m20 = t.m02; _out->m21 = t.m12; _out->m22 = t.m22; _out->m23 = t.m32;
	_out->m30 = t.m03; _out->m31 = t.m13; _out->m32 = t.m23; _out->m33 = t.m33;
}

void xprMat44Transform(XprVec4* _out, const XprMat44* m)
{
	XprVec4 v = *_out;
	_out->x = m->m00 * v.x + m->m01 * v.y + m->m02 * v.z + m->m03 * v.w;
	_out->y = m->m10 * v.x + m->m11 * v.y + m->m12 * v.z + m->m13 * v.w;
	_out->z = m->m20 * v.x + m->m21 * v.y + m->m22 * v.z + m->m23 * v.w;
	_out->w = m->m30 * v.x + m->m31 * v.y + m->m32 * v.z + m->m33 * v.w;
}

void xprMat44TransformPlane(XprVec4* _out, const XprMat44* m)
{
	/*
	XprVec4 v = *_out;
	_out->x = m->m00 * v.x + m->m01 * v.y + m->m02 * v.z + m->m03 * v.w;
	_out->y = m->m10 * v.x + m->m11 * v.y + m->m12 * v.z + m->m13 * v.w;
	_out->z = m->m20 * v.x + m->m21 * v.y + m->m22 * v.z + m->m23 * v.w;
	_out->w = m->m30 * v.x + m->m31 * v.y + m->m32 * v.z + m->m33 * v.w;
	*/
}

void xprMat44TransformAffineDir(XprVec3* _out, const XprMat44* m)
{
	XprVec3 v = *_out;
	_out->x = m->m00 * v.x + m->m01 * v.y + m->m02 * v.z;
	_out->y = m->m10 * v.x + m->m11 * v.y + m->m12 * v.z;
	_out->z = m->m20 * v.x + m->m21 * v.y + m->m22 * v.z;
}

void xprMat44TransformAffinePt(XprVec3* _out, const XprMat44* m)
{
	XprVec3 v = *_out;
	_out->x = m->m00 * v.x + m->m01 * v.y + m->m02 * v.z + m->m03;
	_out->y = m->m10 * v.x + m->m11 * v.y + m->m12 * v.z + m->m13;
	_out->z = m->m20 * v.x + m->m21 * v.y + m->m22 * v.z + m->m23;
}

void xprMat44SetIdentity(XprMat44* _out)
{
	_out->m00 = 1; _out->m01 = 0; _out->m02 = 0; _out->m03 = 0;
	_out->m10 = 0; _out->m11 = 1; _out->m12 = 0; _out->m13 = 0;
	_out->m20 = 0; _out->m21 = 0; _out->m22 = 1; _out->m23 = 0;
	_out->m30 = 0; _out->m31 = 0; _out->m32 = 0; _out->m33 = 1;
}

void xprMat44SetTranslation(XprMat44* _out, const XprVec3* v)
{
	_out->m03 = v->x;
	_out->m13 = v->y;
	_out->m23 = v->z;
}

void xprMat44GetTranslation(XprVec3* v, const XprMat44* m)
{
	v->x = m->m03;
	v->y = m->m13;
	v->z = m->m23;
}

void xprMat44MakeTranslation(XprMat44* _out, const XprVec3* v)
{
	_out->m00 = 1; _out->m01 = 0; _out->m02 = 0; _out->m03 = v->x;
	_out->m10 = 0; _out->m11 = 1; _out->m12 = 0; _out->m13 = v->y;
	_out->m20 = 0; _out->m21 = 0; _out->m22 = 1; _out->m23 = v->z;
	_out->m30 = 0; _out->m31 = 0; _out->m32 = 0; _out->m33 = 1;
}

void xprMat44MakeScale(XprMat44* _out, const XprVec3* v)
{
	_out->m00 = v->x; _out->m01 = 0; _out->m02 = 0; _out->m03 = 0;
	_out->m10 = 0; _out->m11 = v->y; _out->m12 = 0; _out->m13 = 0;
	_out->m20 = 0; _out->m21 = 0; _out->m22 = v->z; _out->m23 = 0;
	_out->m30 = 0; _out->m31 = 0; _out->m32 = 0; _out->m33 = 1;
}

void xprMat44MakeRotationX(XprMat44* _out, float angleInDeg)
{
	float a = angleInDeg * 3.1415926f / 180;
	float sa = sinf(a);
	float ca = cosf(a);

	_out->m00 = 1; _out->m01 = 0; _out->m02 = 0; _out->m03 = 0;
	_out->m10 = 0; _out->m11 = ca; _out->m12 = -sa; _out->m13 = 0;
	_out->m20 = 0; _out->m21 = sa; _out->m22 = ca; _out->m23 = 0;
	_out->m30 = 0; _out->m31 = 0; _out->m32 = 0; _out->m33 = 1;
}

void xprMat44MakeRotationY(XprMat44* _out, float angleInDeg)
{
	float a = angleInDeg * 3.1415926f / 180;
	float sa = sinf(a);
	float ca = cosf(a);

	_out->m00 = ca; _out->m01 = 0; _out->m02 = sa; _out->m03 = 0;
	_out->m10 = 0; _out->m11 = 1; _out->m12 = 0; _out->m13 = 0;
	_out->m20 = -sa; _out->m21 = 0; _out->m22 = ca; _out->m23 = 0;
	_out->m30 = 0; _out->m31 = 0; _out->m32 = 0; _out->m33 = 1;
}

void xprMat44MakeRotationZ(XprMat44* _out, float angleInDeg)
{
	float a = angleInDeg * 3.1415926f / 180;
	float sa = sinf(a);
	float ca = cosf(a);

	_out->m00 = ca; _out->m01 = -sa; _out->m02 = 0; _out->m03 = 0;
	_out->m10 = sa; _out->m11 = ca; _out->m12 = 0; _out->m13 = 0;
	_out->m20 = 0; _out->m21 = 0; _out->m22 = 1; _out->m23 = 0;
	_out->m30 = 0; _out->m31 = 0; _out->m32 = 0; _out->m33 = 1;
}

void xprMat44MakeRotation(XprMat44* _out, const XprVec3* axis, float angleInDeg)
{
	float a = angleInDeg * 3.1415926f / 180;
	float sa = sinf(a);
	float ca = cosf(a);
	float _ca = 1 - ca;
	float xx = axis->x * axis->x;
	float xy = axis->x * axis->y;
	float xz = axis->x * axis->z;
	float yy = axis->y * axis->y;
	float yz = axis->y * axis->z;
	float zz = axis->z * axis->z;

	_out->m00 = ca + xx * _ca;
	_out->m01 = xy * _ca - axis->z * sa;
	_out->m02 = xz * _ca + axis->y * sa;
	_out->m03 = 0;

	_out->m10 = xy * _ca + axis->z * sa;
	_out->m11 = ca + yy * _ca;
	_out->m12 = yz * _ca - axis->x * sa;
	_out->m13 = 0;

	_out->m20 = xz * _ca - axis->y * sa;
	_out->m21 = yz * _ca + axis->x * sa;
	_out->m22 = ca + zz * _ca;
	_out->m23 = 0;

	_out->m30 = 0; _out->m31 = 0; _out->m32 = 0; _out->m33 = 1;
}

void xprMat44GetBasis(XprVec3* xaxis, XprVec3* yaxis, XprVec3* zaxis, const XprMat44* m)
{
	// reference: http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q5
	xaxis->x = m->m00;
	xaxis->y = m->m10;
	xaxis->z = m->m20;

	yaxis->x = m->m01;
	yaxis->y = m->m11;
	yaxis->z = m->m21;

	zaxis->x = m->m02;
	zaxis->y = m->m12;
	zaxis->z = m->m22;
}

void xprMat44CameraLookAt(XprMat44* _out, const XprVec3* eyeAt, const XprVec3* lookAt, const XprVec3* eyeUp)
{
	XprVec3 fwd, side, up;

	xprVec3Normalize(xprVec3Sub(&fwd, eyeAt, lookAt));
	xprVec3Normalize(xprVec3Cross(&side, eyeUp, &fwd));
	xprVec3Cross(&up, &fwd, &side);

	xprMat44SetIdentity(_out);
	_out->m00 = side.x; 
	_out->m01 = side.y;
	_out->m02 = side.z;

	_out->m10 = up.x;
	_out->m11 = up.y;
	_out->m12 = up.z;

	_out->m20 = fwd.x;
	_out->m21 = fwd.y;
	_out->m22 = fwd.z;

	_out->m03 = -xprVec3Dot(&side, eyeAt);
	_out->m13 = -xprVec3Dot(&up, eyeAt);
	_out->m23 = -xprVec3Dot(&fwd, eyeAt);
}

void xprMat44Prespective(XprMat44* _out, float fovyDeg, float aspect, float znear, float zfar)
{
	float f = 1 / tanf((fovyDeg * 3.1415926f / 180) * 0.5f);
	float nf = 1 / (znear - zfar);

	xprMat44SetIdentity(_out);
	_out->m00 = f / aspect;
	_out->m11 = f;
	_out->m22 = (zfar + znear) * nf;
	_out->m23 = (2 * zfar * znear) * nf;
	_out->m32 = -1;
	_out->m33 = 0;
}

void xprMat44PlanarReflect(XprMat44* _out, const XprVec3* normal, const XprVec3* point)
{
	float vxx = -2 * normal->x * normal->x;
	float vxy = -2 * normal->x * normal->y;
	float vxz = -2 * normal->x * normal->z;
	float vyy = -2 * normal->y * normal->y;
	float vyz = -2 * normal->y * normal->z;
	float vzz = -2 * normal->z * normal->z;

	float pv = 2 * xprVec3Dot(normal, point);

	_out->m00 = 1 + vxx;
	_out->m01 = vxy;
	_out->m02 = vxz;
	_out->m03 = pv * normal->x;

	_out->m10 = vxy;
	_out->m11 = 1 + vyy;
	_out->m12 = vyz;
	_out->m13 = pv * normal->y;

	_out->m20 = vxz;
	_out->m21 = vyz;
	_out->m22 = 1 + vzz;
	_out->m23 = pv * normal->z;

	_out->m30 = 0;
	_out->m31 = 0;
	_out->m32 = 0;
	_out->m33 = 1;
}