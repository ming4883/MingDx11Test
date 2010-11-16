#include "Mat44.h"
#include "Vec3.h"
#include "Vec4.h"

xprMat44 xprMat44_(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33)
{
	xprMat44 self;
	self.m00 = m00; self.m01 = m01; self.m02 = m02; self.m03 = m03;
	self.m10 = m10; self.m11 = m11; self.m12 = m12; self.m13 = m13;
	self.m20 = m20; self.m21 = m21; self.m22 = m22; self.m23 = m23;
	self.m30 = m30; self.m31 = m31; self.m32 = m32; self.m33 = m33;

	return self;
}

void xprMat44_transpose(xprMat44* _out, const xprMat44* m)
{
	xprMat44 t = *m;
	_out->m00 = t.m00; _out->m01 = t.m10; _out->m02 = t.m20; _out->m03 = t.m30;
	_out->m10 = t.m01; _out->m11 = t.m11; _out->m12 = t.m21; _out->m13 = t.m31;
	_out->m20 = t.m02; _out->m21 = t.m12; _out->m22 = t.m22; _out->m23 = t.m32;
	_out->m30 = t.m03; _out->m31 = t.m13; _out->m32 = t.m23; _out->m33 = t.m33;
}

void xprMat44_transform(xprVec4* _out, const xprMat44* m)
{
	xprVec4 v = *_out;
	_out->x = m->m00 * v.x + m->m01 * v.y + m->m02 * v.z + m->m03 * v.w;
	_out->y = m->m10 * v.x + m->m11 * v.y + m->m12 * v.z + m->m13 * v.w;
	_out->z = m->m20 * v.x + m->m21 * v.y + m->m22 * v.z + m->m23 * v.w;
	_out->w = m->m30 * v.x + m->m31 * v.y + m->m32 * v.z + m->m33 * v.w;
}

void xprMat44_transformAffineDir(xprVec3* _out, const xprMat44* m)
{
	xprVec3 v = *_out;
	_out->x = m->m00 * v.x + m->m01 * v.y + m->m02 * v.z;
	_out->y = m->m10 * v.x + m->m11 * v.y + m->m12 * v.z;
	_out->z = m->m20 * v.x + m->m21 * v.y + m->m22 * v.z;
}

void xprMat44_transformAffinePt(xprVec3* _out, const xprMat44* m)
{
	xprVec3 v = *_out;
	_out->x = m->m00 * v.x + m->m01 * v.y + m->m02 * v.z + m->m03;
	_out->y = m->m10 * v.x + m->m11 * v.y + m->m12 * v.z + m->m13;
	_out->z = m->m20 * v.x + m->m21 * v.y + m->m22 * v.z + m->m23;
}

void xprMat44_cameraLookAt(xprMat44* _out, const xprVec3* eyeAt, const xprVec3* looAt, const xprVec3* eyeUp)
{

}