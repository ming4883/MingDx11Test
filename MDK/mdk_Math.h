#ifndef MDK_MATH_H_INCLUDED
#define MDK_MATH_H_INCLUDED

#include "mdk_Config.h"
#include <cmath>
#include <cassert>

#pragma warning(push)
#pragma warning(disable:4201)

// All function will return by value instead of reference
// http://www.gamasutra.com/view/feature/4248/designing_fast_crossplatform_simd_.php

namespace mdk
{

struct Scalar
{
    template<typename REAL>
    static m_inline REAL cPI()
    {
        return (REAL)3.141592654f;
    }

    template<typename REAL>
    static m_inline REAL c2PI()
    {
        return (REAL)6.283185307f;
    }

    template<typename REAL>
    static m_inline REAL cPIdiv2()
    {
        return (REAL)1.570796327f;
    }

    template<typename REAL>
    static m_inline REAL cPIdiv4()
    {
        return (REAL)0.785398163f;
    }

    template<typename REAL>
    static m_inline REAL c1divPI()
    {
        return (REAL)0.318309886f;
    }

    template<typename REAL>
    static m_inline REAL c1div2PI()
    {
        return (REAL)0.159154943f;
    }

    template<typename REAL>
    static m_inline REAL rad (REAL valueInDeg)
    {
        return (valueInDeg * cPI<REAL>()) / (REAL)180.0f;
    }

    template<typename REAL>
    static m_inline REAL deg (REAL valueInRad)
    {
        return (valueInRad * 180.0f) * c1divPI<REAL>();
    }

    template<typename REAL>
    static m_inline REAL sign (REAL value)
    {
        return (REAL) (value > -1e-6 ? 1 : -1);
    }

    template<typename REAL>
    static m_inline REAL abs (REAL value)
    {
        return (REAL)fabsf ((float)value);
    }

    template<typename REAL>
    static m_inline void calcSinCos (REAL& retSin, REAL& retCos, REAL angleInRad)
    {
        float a = (float)angleInRad;
        retSin = (REAL) sinf (a);
        retCos = (REAL) cosf (a);
    }
};

/*! Simply a 3d vector.
 */
template<typename REAL>
struct Vec3
{
    REAL x, y, z;

    Vec3()
    {
        static_assert (sizeof (Vec3<REAL>) == sizeof (REAL) * 3, "Invalid Vec3 size");
    }

    explicit Vec3 (REAL x, REAL y, REAL z)
        : x (x), y (y), z (z)
    {
    }

    explicit Vec3 (const REAL* xyz)
        : x (xyz[0]), y (xyz[1]), z (xyz[2])
    {
    }

    m_inline operator REAL* ()
    {
        return &x;
    }

    m_inline operator const REAL* () const
    {
        return &x;
    }
};

/*! Simply a 4d vector.
 */
template<typename REAL>
struct Vec4
{
    REAL x, y, z, w;

    Vec4()
    {
        static_assert (sizeof (Vec4<REAL>) == sizeof (REAL) * 4, "Invalid Vec4 size");
    }

    explicit Vec4 (REAL x, REAL y, REAL z, REAL w)
        : x (x), y (y), z (z), w (w)
    {
    }

    explicit Vec4 (Vec3<REAL> xyz, REAL w)
        : x (xyz.x), y (xyz.y), z (xyz.z), w (w)
    {
    }

    explicit Vec4 (const REAL* xyzw)
        : x (xyzw[0]), y (xyzw[1]), z (xyzw[2]), w (xyzw[3])
    {
    }

    m_inline operator REAL* ()
    {
        return &x;
    }

    m_inline operator const REAL* () const
    {
        return &x;
    }

    m_inline Vec3<REAL> xyz() const
    {
        return Vec3<REAL> (x, y, z);
    }
};


/*! Simply a 4x4 row-major matrix.
 */
template<typename REAL>
struct Mat44
{
    union
    {
        struct
        {
            float
            // convention: row, column
            m00, m01, m02, m03,
                 m10, m11, m12, m13,
                 m20, m21, m22, m23,
                 m30, m31, m32, m33;
        };

        float m[16];
    };

    enum
    {
        // convention: row, column
        M00, M01, M02, M03,
        M10, M11, M12, M13,
        M20, M21, M22, M23,
        M30, M31, M32, M33,
    };

    Mat44()
    {
        static_assert (sizeof (Mat44<REAL>) == sizeof (REAL) * 16, "Invalid Mat44 size");
    }

    explicit Mat44 (REAL all)
    {
        m[M00] = all;
        m[M01] = all;
        m[M02] = all;
        m[M03] = all;
        m[M10] = all;
        m[M11] = all;
        m[M12] = all;
        m[M13] = all;
        m[M20] = all;
        m[M21] = all;
        m[M22] = all;
        m[M23] = all;
        m[M30] = all;
        m[M31] = all;
        m[M32] = all;
        m[M33] = all;
    }

    explicit Mat44 (const REAL* m44)
    {
        m[M00] = m44[M00];
        m[M01] = m44[M01];
        m[M02] = m44[M02];
        m[M03] = m44[M03];
        m[M10] = m44[M10];
        m[M11] = m44[M11];
        m[M12] = m44[M12];
        m[M13] = m44[M13];
        m[M20] = m44[M20];
        m[M21] = m44[M21];
        m[M22] = m44[M22];
        m[M23] = m44[M23];
        m[M30] = m44[M30];
        m[M31] = m44[M31];
        m[M32] = m44[M32];
        m[M33] = m44[M33];
    }

    explicit Mat44 (
        REAL _00, REAL _01, REAL _02, REAL _03,
        REAL _10, REAL _11, REAL _12, REAL _13,
        REAL _20, REAL _21, REAL _22, REAL _23,
        REAL _30, REAL _31, REAL _32, REAL _33)
    {
        m[M00] = _00;
        m[M01] = _01;
        m[M02] = _02;
        m[M03] = _03;
        m[M10] = _10;
        m[M11] = _11;
        m[M12] = _12;
        m[M13] = _13;
        m[M20] = _20;
        m[M21] = _21;
        m[M22] = _22;
        m[M23] = _23;
        m[M30] = _30;
        m[M31] = _31;
        m[M32] = _32;
        m[M33] = _33;
    }

    // Returns the i-th row
    m_inline Vec4<REAL>& operator[] (size_t i)
    {
        Vec4<REAL>* vec4 = (Vec4<REAL>*)m;
        assert (i < 4);
        return vec4[i];
    }

    // Returns the i-th row
    m_inline const Vec4<REAL>& operator[] (size_t i) const
    {
        const Vec4<REAL>* vec4 = (const Vec4<REAL>*)m;
        assert (i < 4);
        return vec4[i];
    }
};

/*! A transform in 3d space
 */
template<typename REAL>
struct Transform3
{
    Vec3<REAL> scaling;  // vec3
    Vec3<REAL> position; // vec3
    Vec4<REAL> rotation; // quaternion
};

struct Vec
{
    //! dst[i] = a[i] + b[i]
    template<typename REAL>
    static m_inline Vec3<REAL> add (Vec3<REAL> a, Vec3<REAL> b);

    template<typename REAL>
    static m_inline Vec4<REAL> add (Vec4<REAL> a, Vec4<REAL> b);

    //! dst[i] = a[i] - b[i]
    template<typename REAL>
    static m_inline Vec3<REAL> sub (Vec3<REAL> a, Vec3<REAL> b);

    template<typename REAL>
    static m_inline Vec4<REAL> sub (Vec4<REAL> a, Vec4<REAL> b);

    //! dst[i] = a[i] * b
    template<typename REAL>
    static m_inline Vec3<REAL> mul (Vec3<REAL> a, REAL b);

    template<typename REAL>
    static m_inline Vec4<REAL> mul (Vec4<REAL> a, REAL b);

    //! dst[i] = a[i] * b[i]
    template<typename REAL>
    static m_inline Vec3<REAL> mul (Vec3<REAL> a, Vec3<REAL> b);

    template<typename REAL>
    static m_inline Vec4<REAL> mul (Vec4<REAL> a, Vec4<REAL> b);

    //! dst = sum (a[i] * b[i])
    template<typename REAL>
    static m_inline REAL dot (Vec3<REAL> a, Vec3<REAL> b);

    template<typename REAL>
    static m_inline REAL dot (Vec4<REAL> a, Vec4<REAL> b);

    //! dot and replicated the result
    template<typename REAL>
    static m_inline Vec4<REAL> dotSIMD (Vec4<REAL> a, Vec4<REAL> b);

    //! dst[i] = 1.0 / v[i]
    template<typename REAL>
    static m_inline Vec3<REAL> reciprocal (Vec3<REAL> v);

    template<typename REAL>
    static m_inline Vec4<REAL> reciprocal (Vec4<REAL> v);

    // length
    template<typename REAL>
    static m_inline REAL length (Vec3<REAL> v);

    template<typename REAL>
    static m_inline REAL length (Vec4<REAL> v);

    //! isNear (0, dot (v, v))
    template<typename REAL>
    static m_inline bool isNearZero (Vec3<REAL> v);

    template<typename REAL>
    static m_inline bool isNearZero (Vec4<REAL> v);

    //! dst[i] = -v[i]
    template<typename REAL>
    static m_inline Vec3<REAL> neg (Vec3<REAL> v);

    template<typename REAL>
    static m_inline Vec4<REAL> neg (Vec4<REAL> v);

    //! dst = v / |v|
    template<typename REAL>
    static m_inline Vec3<REAL> normalize (Vec3<REAL> v);

    template<typename REAL>
    static m_inline Vec4<REAL> normalize (Vec4<REAL> v);

    //! dst[i] = crossProduct (a[i], b[i])
    template<typename REAL>
    static m_inline Vec3<REAL> cross (Vec3<REAL> a, Vec3<REAL> b);

    //! dst[i] = a[i] * (1 - t) + b[i] * t
    template<typename REAL>
    static m_inline Vec3<REAL> lerp (Vec3<REAL> a, Vec3<REAL> b, REAL t);

    template<typename REAL>
    static m_inline Vec4<REAL> lerp (Vec4<REAL> a, Vec4<REAL> b, REAL t);
};

struct Mat
{
    template<typename REAL>
    static m_inline void setIdentity (Mat44<REAL>& m);

    template<typename REAL>
    static m_inline void fromTransform3 (Mat44<REAL>& dst, const Transform3<REAL>& src);

    //! convert q to matrix
    template<typename REAL>
    static m_inline void fromQuat (Mat44<REAL>& dst, Vec4<REAL> q);

    template<typename REAL>
    static m_inline void fromQuat (Mat44<REAL>* dst, const Vec4<REAL>* q, size_t cnt);

    // dst[i][j] = m[j][i]
    template<typename REAL>
    static m_inline void transpose (Mat44<REAL>& dst, const Mat44<REAL>& m);

    // dst = a * b
    template<typename REAL>
    static m_inline void mul (Mat44<REAL>& dst, const Mat44<REAL>& a, const Mat44<REAL>& b);
};

struct Quat
{
    template<typename REAL>
    static m_inline void setIdentity (Vec4<REAL>& q);

    //! construct a quaternion from xyz rotation (in radian).
    template<typename REAL>
    static m_inline Vec4<REAL> fromXYZRotation (Vec3<REAL> rotationInRad);

    //! construct a quaternion from a normalized axis and an angle (in radian).
    template<typename REAL>
    static m_inline Vec4<REAL> fromUnitAxisAngle (Vec3<REAL> axis, REAL angleInRad);

    //! construct a quaternion which transform dirBeg to dirEnd.
    template<typename REAL>
    static m_inline Vec4<REAL> fromDirs (Vec3<REAL> dirBeg, Vec3<REAL> dirEnd);

    //! construct a quaternion from the rotation part of a matrix.
    template<typename REAL>
    static m_inline Vec4<REAL> fromMatrix (const Mat44<REAL>& m);

    //! dst = a * b
    template<typename REAL>
    static m_inline Vec4<REAL> mul (Vec4<REAL> a, Vec4<REAL> b);

    //! inverse of q
    template<typename REAL>
    static m_inline Vec4<REAL> inverse (Vec4<REAL> q);

    //! transform v by q
    template<typename REAL>
    static m_inline Vec3<REAL> transform (Vec4<REAL> q, Vec3<REAL> v);
};

struct Transform
{
    template<typename REAL>
    static m_inline void setIdentity (Transform3<REAL>& dst);

    template<typename REAL>
    static m_inline void fromLookAt (Transform3<REAL>& dst, const Vec3<REAL>& eyeAt, const Vec3<REAL>& lookAt, const Vec3<REAL>& unitRefUp);

    template<typename REAL>
    static m_inline void inverse (Transform3<REAL>& dst, const Transform3<REAL>& src);

    template<typename REAL>
    static m_inline void derive (Transform3<REAL>& dst, const Transform3<REAL>& parent, const Transform3<REAL>& child);

};

typedef Vec3<float> Vec3f;

typedef Vec4<float> Vec4f;

typedef Mat44<float> Mat44f;

typedef Transform3<float> Transform3f;

} // namespace

#include "mdk_Math.inl"

#pragma warning(pop)

#endif // MDK_MATH_H_INCLUDED
