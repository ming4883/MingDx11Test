
namespace mdk
{

//==============================================================================
// Vec::add
template<typename REAL>
Vec3<REAL> Vec::add (Vec3<REAL> a, Vec3<REAL> b)
{
    Vec3<REAL> dst;
    dst.x = a.x + b.x;
    dst.y = a.y + b.y;
    dst.z = a.z + b.z;
    return dst;
}

template<typename REAL>
void Vec::add (Vec3<REAL>* dst, const Vec3<REAL>* a, const Vec3<REAL>* b, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = add (a[i], b[i]);
    }
}

template<typename REAL>
Vec4<REAL> Vec::add (Vec4<REAL> a, Vec4<REAL> b)
{
    Vec4<REAL> dst;
    dst.x = a.x + b.x;
    dst.y = a.y + b.y;
    dst.z = a.z + b.z;
    dst.w = a.w + b.w;
    return dst;
}

template<typename REAL>
void Vec::add (Vec4<REAL>* dst, const Vec4<REAL>* a, const Vec4<REAL>* b, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = add (a[i], b[i]);
    }
}

// Vec::sub
template<typename REAL>
Vec3<REAL> Vec::sub (Vec3<REAL> a, Vec3<REAL> b)
{
    Vec3<REAL> dst;
    dst.x = a.x - b.x;
    dst.y = a.y - b.y;
    dst.z = a.z - b.z;

    return dst;
}

template<typename REAL>
void Vec::sub (Vec3<REAL>* dst, const Vec3<REAL>* a, const Vec3<REAL>* b, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = sub (a[i], b[i]);
    }
}

template<typename REAL>
Vec4<REAL> Vec::sub (Vec4<REAL> a, Vec4<REAL> b)
{
    Vec4<REAL> dst;
    dst.x = a.x - b.x;
    dst.y = a.y - b.y;
    dst.z = a.z - b.z;
    dst.w = a.w - b.w;

    return dst;
}

template<typename REAL>
void Vec::sub (Vec4<REAL>* dst, const Vec4<REAL>* a, const Vec4<REAL>* b, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = sub (a[i], b[i]);
    }
}

// Vec::mul
template<typename REAL>
Vec3<REAL> Vec::mul (Vec3<REAL> a, REAL b)
{
    Vec3<REAL> dst;
    dst.x = a.x * b;
    dst.y = a.y * b;
    dst.z = a.z * b;

    return dst;
}

template<typename REAL>
void Vec::mul (Vec3<REAL>* dst, const Vec3<REAL>* a, const REAL* b, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = mul (a[i], b[i]);
    }
}

template<typename REAL>
Vec4<REAL> Vec::mul (Vec4<REAL> a, REAL b)
{
    Vec4<REAL> dst;
    dst.x = a.x * b;
    dst.y = a.y * b;
    dst.z = a.z * b;
    dst.w = a.w * b;

    return ret;
}

template<typename REAL>
void Vec::mul (Vec4<REAL>* dst, const Vec4<REAL>* a, const REAL* b, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = mul (a[i], b[i]);
    }
}


// Vec::mul
template<typename REAL>
Vec3<REAL> Vec::mul (Vec3<REAL> a, Vec3<REAL> b)
{
    Vec3<REAL> dst;
    dst.x = a.x * b.x;
    dst.y = a.y * b.y;
    dst.z = a.z * b.z;

    return dst;
}

template<typename REAL>
void Vec::mul (Vec3<REAL>* dst, const Vec3<REAL>* a, const Vec3<REAL>* b, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = mul (a[i], b[i]);
    }
}

template<typename REAL>
Vec4<REAL> Vec::mul (Vec4<REAL> a, Vec4<REAL> b)
{
    Vec4<REAL> dst;
    dst.x = a.x * b.x;
    dst.y = a.y * b.y;
    dst.z = a.z * b.z;
    dst.w = a.w * b.w;

    return dst;
}

template<typename REAL>
void Vec::mul (Vec4<REAL>* dst, const Vec4<REAL>* a, const Vec4<REAL>* b, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = mul (a[i], b[i]);
    }
}

// Vec::dot
template<typename REAL>
REAL Vec::dot (Vec3<REAL> a, Vec3<REAL> b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<typename REAL>
void Vec::dot (REAL* dst, const Vec3<REAL>* a, const Vec3<REAL>* b, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = dot (a[i], b[i]);
    }
}

template<typename REAL>
REAL Vec::dot (Vec4<REAL> a, Vec4<REAL> b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

template<typename REAL>
void Vec::dot (REAL* dst, const Vec4<REAL>* a, const Vec4<REAL>* b, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = dot (a[i], b[i]);
    }
}

template<typename REAL>
Vec4<REAL> Vec::dotSIMD (Vec4<REAL> a, Vec4<REAL> b)
{
    REAL _ = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return Vec4<REAL> (_, _, _, _);
}

// Vec::reciprocal
template<typename REAL>
Vec3<REAL> Vec::reciprocal (Vec3<REAL> v)
{
    Vec3<REAL> dst;
    dst.x = (REAL)1.0f / v.x;
    dst.y = (REAL)1.0f / v.y;
    dst.z = (REAL)1.0f / v.z;
    return dst;
}

template<typename REAL>
void Vec::reciprocal (Vec3<REAL>* dst, const Vec3<REAL>* v, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = reciprocal (v[i]);
    }
}

template<typename REAL>
Vec4<REAL> Vec::reciprocal (Vec4<REAL> v)
{
    Vec4<REAL> dst;
    dst.x = (REAL)1.0f / v.x;
    dst.y = (REAL)1.0f / v.y;
    dst.z = (REAL)1.0f / v.z;
    dst.w = (REAL)1.0f / v.w;
    return dst;
}

template<typename REAL>
void Vec::reciprocal (Vec4<REAL>* dst, const Vec4<REAL>* v, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = reciprocal (v[i]);
    }
}


// Vec::neg
template<typename REAL>
Vec3<REAL> Vec::neg (Vec3<REAL> v)
{
    Vec3<REAL> dst;
    dst.x = -v.x;
    dst.y = -v.y;
    dst.z = -v.z;
    return dst;
}

template<typename REAL>
void Vec::neg (Vec3<REAL>* dst, const Vec3<REAL>* v, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = neg (v[i]);
    }
}

template<typename REAL>
Vec4<REAL> Vec::neg (Vec4<REAL> v)
{
    Vec4<REAL> dst;
    dst.x = -v.x;
    dst.y = -v.y;
    dst.z = -v.z;
    dst.w = -v.w;
    return dst;
}

template<typename REAL>
void Vec::neg (Vec4<REAL>* dst, const Vec4<REAL>* v, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = neg (v[i]);
    }
}

// Vec::cross
template<typename REAL>
Vec3<REAL> Vec::cross (Vec3<REAL> a, Vec3<REAL> b)
{
    Vec3<REAL> dst;
    dst.x = a.y * b.z - a.z * b.y;
    dst.y = a.z * b.x - a.x * b.z;
    dst.z = a.x * b.y - a.y * b.x;

    return dst;
}

template<typename REAL>
void Vec::cross (Vec3<REAL>* dst, const Vec3<REAL>* a, const Vec3<REAL>* b, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = cross (a[i], b[i]);
    }
}


//==============================================================================
template<typename REAL>
void Mat::setIdentity (Mat44<REAL>& m)
{
    m[0][0] = (REAL)1;
    m[0][1] = (REAL)0;
    m[0][2] = (REAL)0;
    m[0][3] = (REAL)0;

    m[1][0] = (REAL)0;
    m[1][1] = (REAL)1;
    m[1][2] = (REAL)0;
    m[1][3] = (REAL)0;

    m[2][0] = (REAL)0;
    m[2][1] = (REAL)0;
    m[2][2] = (REAL)1;
    m[2][3] = (REAL)0;

    m[3][0] = (REAL)0;
    m[3][1] = (REAL)0;
    m[3][2] = (REAL)0;
    m[3][3] = (REAL)1;
}

// Mat::fromQuat
template<typename REAL>
void Mat::fromQuat (Mat44<REAL>& dst, Vec4<REAL> q)
{
    typedef Mat44<REAL> M;

    // Reference: OgreQuaternion.cpp
    const REAL tx  = 2 * q.x;
    const REAL ty  = 2 * q.y;
    const REAL tz  = 2 * q.z;
    const REAL twx = tx * q.w;
    const REAL twy = ty * q.w;
    const REAL twz = tz * q.w;
    const REAL txx = tx * q.x;
    const REAL txy = ty * q.x;
    const REAL txz = tz * q.x;
    const REAL tyy = ty * q.y;
    const REAL tyz = tz * q.y;
    const REAL tzz = tz * q.z;

    dst[0][0] = 1 - (tyy + tzz);
    dst[0][1] = txy - twz;
    dst[0][2] = txz + twy;
    dst[0][3] = (REAL)0;
    
    dst[1][0] = txy + twz;
    dst[1][1] = 1 - (txx + tzz);
    dst[1][2] = tyz - twx;
    dst[1][3] = (REAL)0;
    
    dst[2][0] = txz - twy;
    dst[2][1] = tyz + twx;
    dst[2][2] = 1 - (txx + tyy);
    dst[2][3] = (REAL)0;

    dst[3][0] = 0;
    dst[3][1] = 0;
    dst[3][2] = 0;
    dst[3][3] = (REAL)1;
}

template<typename REAL>
void Mat::fromQuat (Mat44<REAL>* dst, const Vec4<REAL>* q, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        toMatrix (dst[i], q[i]);
    }
}

// Mat::fromTransform3
template<typename REAL>
void Mat::fromTransform3 (Mat44<REAL>& dst, const Transform3<REAL>& src)
{
    Mat::fromQuat (dst, src.rotation);

    Vec4<REAL> scaling (src.scaling.x, src.scaling.y, src.scaling.z, (REAL)1.0f);
    dst[0] = Vec::mul (dst[0], scaling);
    dst[1] = Vec::mul (dst[1], scaling);
    dst[2] = Vec::mul (dst[2], scaling);

    dst[0][3] = src.position.x;
    dst[1][3] = src.position.y;
    dst[2][3] = src.position.z;
}

// Mat::mul
template<typename REAL>
void Mat::mul (Mat44<REAL>& dst, const Mat44<REAL>& a, const Mat44<REAL>& b)
{
    REAL a0, a1, a2, a3;

    a0 = a.m00;
    a1 = a.m01;
    a2 = a.m02;
    a3 = a.m03;
    dst.m00 = a0 * b.m00 + a1 * b.m10 + a2 * b.m20 + a3 * b.m30;
    dst.m01 = a0 * b.m01 + a1 * b.m11 + a2 * b.m21 + a3 * b.m31;
    dst.m02 = a0 * b.m02 + a1 * b.m12 + a2 * b.m22 + a3 * b.m32;
    dst.m03 = a0 * b.m03 + a1 * b.m13 + a2 * b.m23 + a3 * b.m33;

    a0 = a.m10;
    a1 = a.m11;
    a2 = a.m12;
    a3 = a.m13;
    dst.m10 = a0 * b.m00 + a1 * b.m10 + a2 * b.m20 + a3 * b.m30;
    dst.m11 = a0 * b.m01 + a1 * b.m11 + a2 * b.m21 + a3 * b.m31;
    dst.m12 = a0 * b.m02 + a1 * b.m12 + a2 * b.m22 + a3 * b.m32;
    dst.m13 = a0 * b.m03 + a1 * b.m13 + a2 * b.m23 + a3 * b.m33;

    a0 = a.m20;
    a1 = a.m21;
    a2 = a.m22;
    a3 = a.m23;
    dst.m20 = a0 * b.m00 + a1 * b.m10 + a2 * b.m20 + a3 * b.m30;
    dst.m21 = a0 * b.m01 + a1 * b.m11 + a2 * b.m21 + a3 * b.m31;
    dst.m22 = a0 * b.m02 + a1 * b.m12 + a2 * b.m22 + a3 * b.m32;
    dst.m23 = a0 * b.m03 + a1 * b.m13 + a2 * b.m23 + a3 * b.m33;

    a0 = a.m30;
    a1 = a.m31;
    a2 = a.m32;
    a3 = a.m33;
    dst.m30 = a0 * b.m00 + a1 * b.m10 + a2 * b.m20 + a3 * b.m30;
    dst.m31 = a0 * b.m01 + a1 * b.m11 + a2 * b.m21 + a3 * b.m31;
    dst.m32 = a0 * b.m02 + a1 * b.m12 + a2 * b.m22 + a3 * b.m32;
    dst.m33 = a0 * b.m03 + a1 * b.m13 + a2 * b.m23 + a3 * b.m33;
}

template<typename REAL>
void Mat::mul (Mat44<REAL>* dst, const Mat44<REAL>* a, const Mat44<REAL>* b, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        mul (dst[i], a[i], b[i]);
    }
}

//==============================================================================
template<typename REAL>
void Quat::setIdentity (Vec4<REAL>& q)
{
    q.x = 0;
    q.y = 0;
    q.z = 0;
    q.w = 1;
}

template<typename REAL>
Vec4<REAL> Quat::fromRotation (Vec3<REAL> rotationInRad)
{
    REAL sx, cx, sy, cy, sz, cz;
    REAL sxcy, cxcy, sxsy, cxsy;

    REAL halfx = rotationInRad.x * -0.5f;
    REAL halfy = rotationInRad.y * -0.5f;
    REAL halfz = rotationInRad.z * -0.5f;

    sx = (REAL)sinf (halfx); cx = (REAL)cosf (halfx);
    sy = (REAL)sinf (halfy); cy = (REAL)cosf (halfy);
    sz = (REAL)sinf (halfz); cz = (REAL)cosf (halfz);

    sxcy = sx * cy;
    cxcy = cx * cy;
    sxsy = sx * sy;
    cxsy = cx * sy;

    Vec4<REAL> dst;
    dst.x =  cxsy * sz - sxcy * cz;
    dst.y = -cxsy * cz - sxcy * sz;
    dst.z =  sxsy * cz - cxcy * sz;
    dst.w =  cxcy * cz + sxsy * sz;

    return dst;
}

// Quat::mul
template<typename REAL>
Vec4<REAL> Quat::mul (Vec4<REAL> a, Vec4<REAL> b)
{
    Vec4<REAL> dst;
    dst.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    dst.y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;
    dst.z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;
    dst.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    return dst;
}

template<typename REAL>
void Quat::mul (Vec4<REAL>* dst, const Vec4<REAL>* a, const Vec4<REAL>* b, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = mul (a[i], b[i]);
    }
}

// Quat::inverse
template<typename REAL>
Vec4<REAL> Quat::inverse (Vec4<REAL> q)
{
    Vec4<REAL> flip (-1, -1, -1, 1);
    Vec4<REAL> sqLen = Vec::dotSIMD (q, q);

    if (sqLen.x > 0)
    {
#if 0
        sqLen = Vec::reciprocal (sqLen);
        
        Vec4<REAL> dst;
        dst.x = -q.x * sqLen;
        dst.y = -q.y * sqLen;
        dst.z = -q.z * sqLen;
        dst.w =  q.w * sqLen;
#endif
        return Vec::mul (Vec::mul (flip, q),  Vec::reciprocal (sqLen));
    }
    else
    {
        return q;
    }
}

template<typename REAL>
void Quat::inverse (Vec4<REAL>* dst, const Vec4<REAL>* q, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = inverse (q[i]);
    }
}

// Quat::transform
template<typename REAL>
Vec3<REAL> Quat::transform (Vec4<REAL> q, Vec3<REAL> v)
{
    // t = 2 * cross(q.xyz, v)
    // v' = v + q.w * t + cross(q.xyz, t)
    Vec3<REAL> t = Vec::mul (Vec::cross (q.xyz(), v), (REAL)2.0f);

    return Vec::add (v, Vec::add (Vec::mul (t, q.w), Vec::cross (q.xyz(), t)));
}

template<typename REAL>
void Quat::transform (Vec3<REAL>* dst, const Vec4<REAL>* q, const Vec3<REAL>* v, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i)
    {
        dst[i] = inverse (q[i], v[i]);
    }
}

//==============================================================================
template<typename REAL>
void Transform::setIdentity (Transform3<REAL>& dst)
{
    dst.scaling = Vec3<REAL> ((REAL)1.0f, (REAL)1.0f, (REAL)1.0f);
    dst.position = Vec3<REAL> ((REAL)0.0f, (REAL)0.0f, (REAL)0.0f);
    Quat::setIdentity<REAL> (dst.rotation);
}

template<typename REAL>
void Transform::inverse (Transform3<REAL>& dst, const Transform3<REAL>& src)
{
    dst.scaling = Vec::reciprocal (src.scaling);

    dst.rotation = Quat::inverse (src.rotation);

    dst.position = Vec::mul (Quat::transform (dst.rotation, Vec::neg (src.position)), dst.scaling);
}

template<typename REAL>
void Transform::derive (Transform3<REAL>& dst, const Transform3<REAL>& parent, const Transform3<REAL>& child)
{
    dst.rotation = Quat::mul (parent.rotation, child.rotation);

    dst.scaling = Vec::mul (parent.scaling, child.scaling);

    dst.position = Vec::add (parent.position, Quat::transform (parent.rotation, Vec::mul (parent.scaling, child.position)));
}

} // namespace