#pragma once

#ifdef NDEBUG
#define Assert( _exp ) ((void)0)
#else
#define Assert(x)
#endif

#include "Definitions.h"
#include <sstream>
#include <Windows.h>
#include <math.h>
#include <emmintrin.h>
#include <xmmintrin.h>
#include <assert.h>


#define CHECK_VALID( _v ) 0

#define FastSqrt(x)	(sqrt)(x)

#define deg(a) a * 57.295779513082

#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h

#define M_PI_2      (M_PI * 2.f)

#define M_PI_F		((float)(M_PI))	// Shouldn't collide with anything.

#define M_PHI		1.61803398874989484820 // golden ratio

// NJS: Inlined to prevent floats from being autopromoted to doubles, as with the old system.
#ifndef RAD2DEG
#define RAD2DEG(x)  ((float)(x) * (float)(180.f / M_PI_F))
#endif
#define rad(a) a * 0.01745329251
#ifndef DEG2RAD
#define DEG2RAD(x)  ((float)(x) * (float)(M_PI_F / 180.f))
#endif

// MOVEMENT INFO
enum
{
    PITCH = 0,	// up / down
    YAW,		// left / right
    ROLL		// fall over
};

// decls for aligning data


#define ALIGN16 __declspec(align(16))
#define VALVE_RAND_MAX 0x7fff
#define VectorExpand(v) (v).x, (v).y, (v).z
class Quaternion				// same data-layout as engine's vec4_t,
{								//		which is a float[4]
public:
	inline Quaternion(void) {}
	inline Quaternion(float ix, float iy, float iz, float iw) : x(ix), y(iy), z(iz), w(iw) {}

	inline void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f, float iw = 0.0f) { x = ix; y = iy; z = iz; w = iw; }

	float* Base() { return (float*)this; }
	const float* Base() const { return (float*)this; }

	float x, y, z, w;
};

class ALIGN16 QuaternionAligned : public Quaternion
{
public:
	inline QuaternionAligned(void) {};
	inline QuaternionAligned(float X, float Y, float Z, float W)
	{
		Init(X, Y, Z, W);
	}
public:
	explicit QuaternionAligned(const Quaternion& vOther)
	{
		Init(vOther.x, vOther.y, vOther.z, vOther.w);
	}

	QuaternionAligned& operator=(const Quaternion& vOther)
	{
		Init(vOther.x, vOther.y, vOther.z, vOther.w);
		return *this;
	}
};
typedef float vec_t;

class Vector
{
public:
	inline void SinCosX(const float rad, float& sin, float& cos) const
	{
		const __m128 _ps_fopi = _mm_set_ss(1.27323954473516f);

		const __m128 _ps_0p5 = _mm_set_ss(0.5f);
		const __m128 _ps_1 = _mm_set_ss(1.0f);

		const __m128 _ps_dp1 = _mm_set_ss(-0.7851562f);
		const __m128 _ps_dp2 = _mm_set_ss(-2.4187564849853515625e-4f);
		const __m128 _ps_dp3 = _mm_set_ss(-3.77489497744594108e-8f);

		const __m128 _ps_sincof_p0 = _mm_set_ss(2.443315711809948e-5f);
		const __m128 _ps_sincof_p1 = _mm_set_ss(8.3321608736e-3f);
		const __m128 _ps_sincof_p2 = _mm_set_ss(-1.6666654611e-1f);
		const __m128 _ps_coscof_p0 = _mm_set_ss(2.443315711809948e-5f);
		const __m128 _ps_coscof_p1 = _mm_set_ss(-1.388731625493765e-3f);
		const __m128 _ps_coscof_p2 = _mm_set_ss(4.166664568298827e-2f);

		const __m128i _pi32_1 = _mm_set1_epi32(1);
		const __m128i _pi32_i1 = _mm_set1_epi32(~1);
		const __m128i _pi32_2 = _mm_set1_epi32(2);
		const __m128i _pi32_4 = _mm_set1_epi32(4);

		const __m128 _mask_sign_raw = *(__m128*) & _mm_set1_epi32(0x80000000);
		const __m128 _mask_sign_inv = *(__m128*) & _mm_set1_epi32(~0x80000000);


		__m128  xmm3 = _mm_setzero_ps();
		__m128i emm0, emm2, emm4;

		__m128 sign_bit_cos, sign_bit_sin;

		__m128 x, y, z;
		__m128 y1, y2;

		__m128 a = _mm_set_ss(rad);

		x = _mm_and_ps(a, _mask_sign_inv);
		y = _mm_mul_ps(x, _ps_fopi);

		emm2 = _mm_cvtps_epi32(y);
		emm2 = _mm_add_epi32(emm2, _pi32_1);
		emm2 = _mm_and_si128(emm2, _pi32_i1);
		y = _mm_cvtepi32_ps(emm2);

		emm4 = emm2;

		emm0 = _mm_and_si128(emm2, _pi32_4);
		emm0 = _mm_slli_epi32(emm0, 29);
		__m128 swap_sign_bit_sin = _mm_castsi128_ps(emm0);

		emm2 = _mm_and_si128(emm2, _pi32_2);
		emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());
		__m128 poly_mask = _mm_castsi128_ps(emm2);

		x = _mm_add_ps(x, _mm_mul_ps(y, _ps_dp1));
		x = _mm_add_ps(x, _mm_mul_ps(y, _ps_dp2));
		x = _mm_add_ps(x, _mm_mul_ps(y, _ps_dp3));

		emm4 = _mm_sub_epi32(emm4, _pi32_2);
		emm4 = _mm_andnot_si128(emm4, _pi32_4);
		emm4 = _mm_slli_epi32(emm4, 29);

		sign_bit_cos = _mm_castsi128_ps(emm4);
		sign_bit_sin = _mm_xor_ps(_mm_and_ps(a, _mask_sign_raw), swap_sign_bit_sin);

		z = _mm_mul_ps(x, x);

		y1 = _mm_mul_ps(_ps_coscof_p0, z);
		y1 = _mm_add_ps(y1, _ps_coscof_p1);
		y1 = _mm_mul_ps(y1, z);
		y1 = _mm_add_ps(y1, _ps_coscof_p2);
		y1 = _mm_mul_ps(y1, z);
		y1 = _mm_mul_ps(y1, z);
		y1 = _mm_sub_ps(y1, _mm_mul_ps(z, _ps_0p5));
		y1 = _mm_add_ps(y1, _ps_1);

		y2 = _mm_mul_ps(_ps_sincof_p0, z);
		y2 = _mm_add_ps(y2, _ps_sincof_p1);
		y2 = _mm_mul_ps(y2, z);
		y2 = _mm_add_ps(y2, _ps_sincof_p2);
		y2 = _mm_mul_ps(y2, z);
		y2 = _mm_mul_ps(y2, x);
		y2 = _mm_add_ps(y2, x);

		xmm3 = poly_mask;

		__m128 ysin2 = _mm_and_ps(xmm3, y2);
		__m128 ysin1 = _mm_andnot_ps(xmm3, y1);

		sin = _mm_cvtss_f32(_mm_xor_ps(_mm_add_ps(ysin1, ysin2), sign_bit_sin));
		cos = _mm_cvtss_f32(_mm_xor_ps(_mm_add_ps(_mm_sub_ps(y1, ysin1), _mm_sub_ps(y2, ysin2)), sign_bit_cos));
	}
    float x, y, z;
    Vector(void);
    Vector(float X, float Y, float Z);
    void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f);
    bool IsValid() const;
    float operator[](int i) const;
    float& operator[](int i);
    inline void Zero();
    bool operator==(const Vector& v) const;
    bool operator!=(const Vector& v) const;
    inline Vector&	operator+=(const Vector &v);
    inline Vector&	operator-=(const Vector &v);
    inline Vector&	operator*=(const Vector &v);
    inline Vector&	operator*=(float s);
    inline Vector&	operator/=(const Vector &v);
    inline Vector&	operator/=(float s);
    inline Vector&	operator+=(float fl);
    inline Vector&	operator-=(float fl);
    inline float	Length() const;
    inline float LengthSqr(void) const
    {
        CHECK_VALID(*this);
        return (x*x + y*y + z*z);
    }
    bool IsZero(float tolerance = 0.01f) const
    {
        return (x > -tolerance && x < tolerance &&
            y > -tolerance && y < tolerance &&
            z > -tolerance && z < tolerance);
    }
	Vector  Clamp();
    Vector	Normalize();
    float	NormalizeInPlace();
    inline float	DistTo(const Vector &vOther) const;
    inline float	DistToSqr(const Vector &vOther) const;
    float	Dot(const Vector& vOther) const;
    float	Length2D(void) const;
    float	Length2DSqr(void) const;
    void	MulAdd(const Vector& a, const Vector& b, float scalar);
	Vector	Cross(const Vector &vOther) const;
    Vector& operator=(const Vector &vOther);
    Vector	operator-(void) const;
    Vector	operator+(const Vector& v) const;
	Vector  Forward() const;
    Vector	operator-(const Vector& v) const;
    Vector	operator*(const Vector& v) const;
    Vector	operator/(const Vector& v) const;
    Vector	operator*(float fl) const;
    Vector	operator/(float fl) const;
    // Base address...
    float* Base();
    float const* Base() const;

	auto clear() -> void
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;
	}

	inline Vector Angle(Vector* up = 0)
	{
		if (!x && !y)
			return Vector(0, 0, 0);

		float roll = 0;

		if (up)
		{
			Vector left = (*up).Cross(*this);
			roll = deg(atan2f(left.z, (left.y * x) - (left.x * y)));
		}
		return Vector(deg(atan2f(-z, sqrtf(x*x + y * y))), deg(atan2f(y, x)), roll);
	}
};

//===============================================
inline void Vector::Init(float ix, float iy, float iz)
{
    x = ix; y = iy; z = iz;
    CHECK_VALID(*this);
}
//===============================================
inline Vector::Vector(float X, float Y, float Z)
{
    x = X; y = Y; z = Z;
    CHECK_VALID(*this);
}
//===============================================
inline Vector::Vector(void) { }
//===============================================
inline void Vector::Zero()
{
    x = y = z = 0.0f;
}
//===============================================
inline void VectorClear(Vector& a)
{
    a.x = a.y = a.z = 0.0f;
}
//===============================================
inline Vector& Vector::operator=(const Vector &vOther)
{
    CHECK_VALID(vOther);
    x = vOther.x; y = vOther.y; z = vOther.z;
    return *this;
}
//===============================================
inline float& Vector::operator[](int i)
{
    Assert((i >= 0) && (i < 3));
    return ((float*)this)[i];
}
//===============================================
inline float Vector::operator[](int i) const
{
    Assert((i >= 0) && (i < 3));
    return ((float*)this)[i];
}

//===============================================
inline bool Vector::operator==(const Vector& src) const
{
    CHECK_VALID(src);
    CHECK_VALID(*this);
    return (src.x == x) && (src.y == y) && (src.z == z);
}
//===============================================
inline bool Vector::operator!=(const Vector& src) const
{
    CHECK_VALID(src);
    CHECK_VALID(*this);
    return (src.x != x) || (src.y != y) || (src.z != z);
}
//===============================================
inline void VectorCopy(const Vector& src, Vector& dst)
{
    CHECK_VALID(src);
    dst.x = src.x;
    dst.y = src.y;
    dst.z = src.z;
}
//===============================================
inline  Vector& Vector::operator+=(const Vector& v)
{
    CHECK_VALID(*this);
    CHECK_VALID(v);
    x += v.x; y += v.y; z += v.z;
    return *this;
}
//===============================================
inline  Vector& Vector::operator-=(const Vector& v)
{
    CHECK_VALID(*this);
    CHECK_VALID(v);
    x -= v.x; y -= v.y; z -= v.z;
    return *this;
}
//===============================================
inline  Vector& Vector::operator*=(float fl)
{
    x *= fl;
    y *= fl;
    z *= fl;
    CHECK_VALID(*this);
    return *this;
}
//===============================================
inline  Vector& Vector::operator*=(const Vector& v)
{
    CHECK_VALID(v);
    x *= v.x;
    y *= v.y;
    z *= v.z;
    CHECK_VALID(*this);
    return *this;
}
//===============================================
inline Vector&	Vector::operator+=(float fl)
{
    x += fl;
    y += fl;
    z += fl;
    CHECK_VALID(*this);
    return *this;
}
//===============================================
inline Vector&	Vector::operator-=(float fl)
{
    x -= fl;
    y -= fl;
    z -= fl;
    CHECK_VALID(*this);
    return *this;
}
//===============================================
inline  Vector& Vector::operator/=(float fl)
{
    Assert(fl != 0.0f);
    float oofl = 1.0f / fl;
    x *= oofl;
    y *= oofl;
    z *= oofl;
    CHECK_VALID(*this);
    return *this;
}
//===============================================
inline Vector Vector::Clamp()
{
	CHECK_VALID(*this);

	if (x < -89.0f)
		x = -89.0f;

	if (x > 89.0f)
		x = 89.0f;

	while (y < -180.0f)
		y += 360.0f;

	while (y > 180.0f)
		y -= 360.0f;

	z = 0.0f;

	return *this;
}
//===============================================
inline  Vector& Vector::operator/=(const Vector& v)
{
    CHECK_VALID(v);
    Assert(v.x != 0.0f && v.y != 0.0f && v.z != 0.0f);
    x /= v.x;
    y /= v.y;
    z /= v.z;
    CHECK_VALID(*this);
    return *this;
}
//===============================================
inline float Vector::Length(void) const
{
    CHECK_VALID(*this);

    float root = 0.0f;

    float sqsr = x*x + y*y + z*z;

    root = sqrt(sqsr);

    return root;
}
//===============================================
inline float Vector::Length2D(void) const
{
    CHECK_VALID(*this);

    float root = 0.0f;

    float sqst = x*x + y*y;

    root = sqrt(sqst);

    return root;
}
//===============================================
inline float Vector::Length2DSqr(void) const
{
    return (x*x + y*y);
}
//===============================================
inline Vector CrossProduct(const Vector& a, const Vector& b)
{
    return Vector(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}
//===============================================
inline Vector Vector::Cross(const Vector& vOther) const
{
	return CrossProduct(*this, vOther);
}
//===============================================
float Vector::DistTo(const Vector &vOther) const
{
    Vector delta;

    delta.x = x - vOther.x;
    delta.y = y - vOther.y;
    delta.z = z - vOther.z;

    return delta.Length();
}
float Vector::DistToSqr(const Vector &vOther) const
{
    Vector delta;

    delta.x = x - vOther.x;
    delta.y = y - vOther.y;
    delta.z = z - vOther.z;

    return delta.LengthSqr();
}
//===============================================
inline Vector Vector::Normalize()
{
    Vector vector;
    float length = Length();

    if (length != 0)
    {
        vector.x = x / length;
        vector.y = y / length;
        vector.z = z / length;
    }
    else
    {
        vector.x = vector.y = 0.0f; vector.z = 1.0f;
    }

    return vector;
}
//===============================================
// changed that to fit awall, paste from xaE
inline float Vector::NormalizeInPlace()
{
	Vector res = *this;
	float l = res.Length();
	if (l != 0.0f) {
		res /= l;
	}
	else {
		res.x = res.y = res.z = 0.0f;
	}
	return l;
}
//===============================================
inline void Vector::MulAdd(const Vector& a, const Vector& b, float scalar)
{
    x = a.x + b.x * scalar;
    y = a.y + b.y * scalar;
    z = a.z + b.z * scalar;
}
//===============================================
inline float VectorNormalize(Vector& v)
{
    Assert(v.IsValid());
    float l = v.Length();
    if (l != 0.0f)
    {
        v /= l;
    }
    else
    {
        // FIXME:
        // Just copying the existing implemenation; shouldn't res.z == 0?
        v.x = v.y = 0.0f; v.z = 1.0f;
    }
    return l;
}
//===============================================
inline float VectorNormalize(float * v)
{
    return VectorNormalize(*(reinterpret_cast<Vector *>(v)));
}
//===============================================
inline Vector Vector::operator+(const Vector& v) const
{
    Vector res;
    res.x = x + v.x;
    res.y = y + v.y;
    res.z = z + v.z;
    return res;
}
inline Vector Vector::Forward() const
{
	float cp, cy, sp, sy;

	SinCosX(rad(x), sp, cp);
	SinCosX(rad(y), sy, cy);

	return Vector(cp * cy, cp * sy, -sp);
}
//===============================================
inline Vector Vector::operator-(const Vector& v) const
{
    Vector res;
    res.x = x - v.x;
    res.y = y - v.y;
    res.z = z - v.z;
    return res;
}
//===============================================
inline Vector Vector::operator*(float fl) const
{
    Vector res;
    res.x = x * fl;
    res.y = y * fl;
    res.z = z * fl;
    return res;
}
//===============================================
inline Vector Vector::operator*(const Vector& v) const
{
    Vector res;
    res.x = x * v.x;
    res.y = y * v.y;
    res.z = z * v.z;
    return res;
}
//===============================================
inline Vector Vector::operator/(float fl) const
{
    Vector res;
    res.x = x / fl;
    res.y = y / fl;
    res.z = z / fl;
    return res;
}
//===============================================
inline Vector Vector::operator/(const Vector& v) const
{
    Vector res;
    res.x = x / v.x;
    res.y = y / v.y;
    res.z = z / v.z;
    return res;
}
inline float Vector::Dot(const Vector& vOther) const
{
    const Vector& a = *this;

    return(a.x*vOther.x + a.y*vOther.y + a.z*vOther.z);
}

FORCEINLINE vec_t DotProduct(const Vector& a, const Vector& b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

//-----------------------------------------------------------------------------
// length
//-----------------------------------------------------------------------------

inline float VectorLength(const Vector& v)
{
    CHECK_VALID(v);
    return (float)FastSqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

//VECTOR SUBTRAC
inline void VectorSubtract(const Vector& a, const Vector& b, Vector& c)
{
    CHECK_VALID(a);
    CHECK_VALID(b);
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;
}

//VECTORADD
inline void VectorAdd(const Vector& a, const Vector& b, Vector& c)
{
    CHECK_VALID(a);
    CHECK_VALID(b);
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    c.z = a.z + b.z;
}

//-----------------------------------------------------------------------------
// Base address...
//-----------------------------------------------------------------------------
inline float* Vector::Base()
{
    return (float*)this;
}

inline float const* Vector::Base() const
{
    return (float const*)this;
}

inline void VectorMAInline(const float* start, float scale, const float* direction, float* dest)
{
    dest[0] = start[0] + direction[0] * scale;
    dest[1] = start[1] + direction[1] * scale;
    dest[2] = start[2] + direction[2] * scale;
}

inline void VectorMAInline(const Vector& start, float scale, const Vector& direction, Vector& dest)
{
    dest.x = start.x + direction.x*scale;
    dest.y = start.y + direction.y*scale;
    dest.z = start.z + direction.z*scale;
}

inline void VectorMA(const Vector& start, float scale, const Vector& direction, Vector& dest)
{
    VectorMAInline(start, scale, direction, dest);
}

inline void VectorMA(const float * start, float scale, const float *direction, float *dest)
{
    VectorMAInline(start, scale, direction, dest);
}


class ALIGN16 VectorAligned : public Vector
{
public:
    inline VectorAligned(void) {};
    inline VectorAligned(float X, float Y, float Z)
    {
        Init(X, Y, Z);
    }

#ifdef VECTOR_NO_SLOW_OPERATIONS

private:
    // No copy constructors allowed if we're in optimal mode
    VectorAligned(const VectorAligned& vOther);
    VectorAligned(const Vector &vOther);

#else
public:
    explicit VectorAligned(const Vector &vOther)
    {
        Init(vOther.x, vOther.y, vOther.z);
    }

    VectorAligned& operator=(const Vector &vOther)
    {
        Init(vOther.x, vOther.y, vOther.z);
        return *this;
    }

#endif
    float w;	// this space is used anyway
};


inline unsigned long& FloatBits(float& f)
{
    return *reinterpret_cast<unsigned long*>(&f);
}

inline bool IsFinite(float f)
{
    return ((FloatBits(f) & 0x7F800000) != 0x7F800000);
}

//=========================================================
// 2D Vector2D
//=========================================================

class Vector2D
{
public:
    // Members
    float x, y;

    // Construction/destruction
    Vector2D(void);
    Vector2D(float X, float Y);
    Vector2D(const float *pFloat);

    // Initialization
    void Init(float ix = 0.0f, float iy = 0.0f);

    // Got any nasty NAN's?
    bool IsValid() const;

    // array access...
    float  operator[](int i) const;
    float& operator[](int i);

    // Base address...
    float*       Base();
    float const* Base() const;

    // Initialization methods
    void Random(float minVal, float maxVal);

    // equality
    bool operator==(const Vector2D& v) const;
    bool operator!=(const Vector2D& v) const;

    // arithmetic operations
    Vector2D& operator+=(const Vector2D& v);
    Vector2D& operator-=(const Vector2D& v);
    Vector2D& operator*=(const Vector2D& v);
    Vector2D& operator*=(float           s);
    Vector2D& operator/=(const Vector2D& v);
    Vector2D& operator/=(float           s);

    // negate the Vector2D components
    void Negate();

    // Get the Vector2D's magnitude.
    float Length() const;

    // Get the Vector2D's magnitude squared.
    float LengthSqr(void) const;

    // return true if this vector is (0,0) within tolerance
    bool IsZero(float tolerance = 0.01f) const
    {
        return (x > -tolerance && x < tolerance &&
            y > -tolerance && y < tolerance);
    }

    float Normalize();

    // Normalize in place and return the old length.
    float NormalizeInPlace();

    // Compare length.
    bool IsLengthGreaterThan(float val) const;
    bool IsLengthLessThan(float    val) const;

    // Get the distance from this Vector2D to the other one.
    float DistTo(const Vector2D& vOther) const;

    // Get the distance from this Vector2D to the other one squared.
    float DistToSqr(const Vector2D& vOther) const;

    // Copy
    void CopyToArray(float* rgfl) const;

    // Multiply, add, and assign to this (ie: *this = a + b * scalar). This
    // is about 12% faster than the actual Vector2D equation (because it's done per-component
    // rather than per-Vector2D).
    void MulAdd(const Vector2D& a, const Vector2D& b, float scalar);

    // Dot product.
    float Dot(const Vector2D& vOther) const;

    // assignment
    Vector2D& operator=(const Vector2D& vOther);

#ifndef VECTOR_NO_SLOW_OPERATIONS
    // copy constructors
    Vector2D(const Vector2D &vOther);

    // arithmetic operations
    Vector2D operator-(void) const;

    Vector2D operator+(const Vector2D& v) const;
    Vector2D operator-(const Vector2D& v) const;
    Vector2D operator*(const Vector2D& v) const;
    Vector2D operator/(const Vector2D& v) const;
    Vector2D operator+(const int       i1) const;
    Vector2D operator+(const float     fl) const;
    Vector2D operator*(const float     fl) const;
    Vector2D operator/(const float     fl) const;

    // Cross product between two vectors.
    Vector2D Cross(const Vector2D& vOther) const;

    // Returns a Vector2D with the min or max in X, Y, and Z.
    Vector2D Min(const Vector2D& vOther) const;
    Vector2D Max(const Vector2D& vOther) const;

#else

private:
    // No copy constructors allowed if we're in optimal mode
    Vector2D(const Vector2D& vOther);
#endif
};

//-----------------------------------------------------------------------------

const Vector2D vec2_origin(0, 0);
//const Vector2D vec2_invalid(3.40282347E+38F, 3.40282347E+38F);

//-----------------------------------------------------------------------------
// Vector2D related operations
//-----------------------------------------------------------------------------

// Vector2D clear
void Vector2DClear(Vector2D& a);

// Copy
void Vector2DCopy(const Vector2D& src, Vector2D& dst);

// Vector2D arithmetic
void Vector2DAdd(const Vector2D&      a, const Vector2D& b, Vector2D&       result);
void Vector2DSubtract(const Vector2D& a, const Vector2D& b, Vector2D&       result);
void Vector2DMultiply(const Vector2D& a, float           b, Vector2D&       result);
void Vector2DMultiply(const Vector2D& a, const Vector2D& b, Vector2D&       result);
void Vector2DDivide(const Vector2D&   a, float           b, Vector2D&       result);
void Vector2DDivide(const Vector2D&   a, const Vector2D& b, Vector2D&       result);
void Vector2DMA(const Vector2D&       start, float       s, const Vector2D& dir, Vector2D& result);

// Store the min or max of each of x, y, and z into the result.
void Vector2DMin(const Vector2D& a, const Vector2D& b, Vector2D& result);
void Vector2DMax(const Vector2D& a, const Vector2D& b, Vector2D& result);

#define Vector2DExpand( v ) (v).x, (v).y

// Normalization
float Vector2DNormalize(Vector2D& v);

// Length
float Vector2DLength(const Vector2D& v);

// Dot Product
float DotProduct2D(const Vector2D& a, const Vector2D& b);

// Linearly interpolate between two vectors
void Vector2DLerp(const Vector2D& src1, const Vector2D& src2, float t, Vector2D& dest);


//-----------------------------------------------------------------------------
//
// Inlined Vector2D methods
//
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// constructors
//-----------------------------------------------------------------------------

inline Vector2D::Vector2D(void)
{
#ifdef _DEBUG
    // Initialize to NAN to catch errors
    //x = y = float_NAN;
#endif
}

inline Vector2D::Vector2D(float X, float Y)
{
    x = X; y = Y;
    Assert(IsValid());
}

inline Vector2D::Vector2D(const float *pFloat)
{
    Assert(pFloat);
    x = pFloat[0]; y = pFloat[1];
    Assert(IsValid());
}


//-----------------------------------------------------------------------------
// copy constructor
//-----------------------------------------------------------------------------

inline Vector2D::Vector2D(const Vector2D &vOther)
{
    Assert(vOther.IsValid());
    x = vOther.x; y = vOther.y;
}

//-----------------------------------------------------------------------------
// initialization
//-----------------------------------------------------------------------------

inline void Vector2D::Init(float ix, float iy)
{
    x = ix; y = iy;
    Assert(IsValid());
}

inline void Vector2D::Random(float minVal, float maxVal)
{
    x = minVal + ((float)rand() / VALVE_RAND_MAX) * (maxVal - minVal);
    y = minVal + ((float)rand() / VALVE_RAND_MAX) * (maxVal - minVal);
}

inline void Vector2DClear(Vector2D& a)
{
    a.x = a.y = 0.0f;
}

//-----------------------------------------------------------------------------
// assignment
//-----------------------------------------------------------------------------

inline Vector2D& Vector2D::operator=(const Vector2D &vOther)
{
    Assert(vOther.IsValid());
    x = vOther.x; y = vOther.y;
    return *this;
}

//-----------------------------------------------------------------------------
// Array access
//-----------------------------------------------------------------------------

inline float& Vector2D::operator[](int i)
{
    Assert((i >= 0) && (i < 2));
    return ((float*)this)[i];
}

inline float Vector2D::operator[](int i) const
{
    Assert((i >= 0) && (i < 2));
    return ((float*)this)[i];
}

//-----------------------------------------------------------------------------
// Base address...
//-----------------------------------------------------------------------------

inline float* Vector2D::Base()
{
    return (float*)this;
}

inline float const* Vector2D::Base() const
{
    return (float const*)this;
}

//-----------------------------------------------------------------------------
// IsValid?
//-----------------------------------------------------------------------------

inline bool Vector2D::IsValid() const
{
    return IsFinite(x) && IsFinite(y);
}

//-----------------------------------------------------------------------------
// comparison
//-----------------------------------------------------------------------------

inline bool Vector2D::operator==(const Vector2D& src) const
{
    Assert(src.IsValid() && IsValid());
    return (src.x == x) && (src.y == y);
}

inline bool Vector2D::operator!=(const Vector2D& src) const
{
    Assert(src.IsValid() && IsValid());
    return (src.x != x) || (src.y != y);
}


//-----------------------------------------------------------------------------
// Copy
//-----------------------------------------------------------------------------

inline void Vector2DCopy(const Vector2D& src, Vector2D& dst)
{
    Assert(src.IsValid());
    dst.x = src.x;
    dst.y = src.y;
}

inline void	Vector2D::CopyToArray(float* rgfl) const
{
    Assert(IsValid());
    Assert(rgfl);
    rgfl[0] = x; rgfl[1] = y;
}

//-----------------------------------------------------------------------------
// standard math operations
//-----------------------------------------------------------------------------

inline void Vector2D::Negate()
{
    Assert(IsValid());
    x = -x; y = -y;
}

inline Vector2D& Vector2D::operator+=(const Vector2D& v)
{
    Assert(IsValid() && v.IsValid());
    x += v.x; y += v.y;
    return *this;
}

inline Vector2D& Vector2D::operator-=(const Vector2D& v)
{
    Assert(IsValid() && v.IsValid());
    x -= v.x; y -= v.y;
    return *this;
}

inline Vector2D& Vector2D::operator*=(float fl)
{
    x *= fl;
    y *= fl;
    Assert(IsValid());
    return *this;
}

inline Vector2D& Vector2D::operator*=(const Vector2D& v)
{
    x *= v.x;
    y *= v.y;
    Assert(IsValid());
    return *this;
}

inline Vector2D& Vector2D::operator/=(float fl)
{
    Assert(fl != 0.0f);
    float oofl = 1.0f / fl;
    x *= oofl;
    y *= oofl;
    Assert(IsValid());
    return *this;
}

inline Vector2D& Vector2D::operator/=(const Vector2D& v)
{
    Assert(v.x != 0.0f && v.y != 0.0f);
    x /= v.x;
    y /= v.y;
    Assert(IsValid());
    return *this;
}

inline void Vector2DAdd(const Vector2D& a, const Vector2D& b, Vector2D& c)
{
    Assert(a.IsValid() && b.IsValid());
    c.x = a.x + b.x;
    c.y = a.y + b.y;
}

inline void Vector2DAdd(const Vector2D& a, const int b, Vector2D& c)
{
    Assert(a.IsValid());
    c.x = a.x + b;
    c.y = a.y + b;
}

inline void Vector2DAdd(const Vector2D& a, const float b, Vector2D& c)
{
    Assert(a.IsValid());
    c.x = a.x + b;
    c.y = a.y + b;
}

inline void Vector2DSubtract(const Vector2D& a, const Vector2D& b, Vector2D& c)
{
    Assert(a.IsValid() && b.IsValid());
    c.x = a.x - b.x;
    c.y = a.y - b.y;
}

inline void Vector2DMultiply(const Vector2D& a, const float b, Vector2D& c)
{
    Assert(a.IsValid() && IsFinite(b));
    c.x = a.x * b;
    c.y = a.y * b;
}

inline void Vector2DMultiply(const Vector2D& a, const Vector2D& b, Vector2D& c)
{
    Assert(a.IsValid() && b.IsValid());
    c.x = a.x * b.x;
    c.y = a.y * b.y;
}


inline void Vector2DDivide(const Vector2D& a, const float b, Vector2D& c)
{
    Assert(a.IsValid());
    Assert(b != 0.0f);
    float oob = 1.0f / b;
    c.x = a.x * oob;
    c.y = a.y * oob;
}

inline void Vector2DDivide(const Vector2D& a, const Vector2D& b, Vector2D& c)
{
    Assert(a.IsValid());
    Assert((b.x != 0.0f) && (b.y != 0.0f));
    c.x = a.x / b.x;
    c.y = a.y / b.y;
}

inline void Vector2DMA(const Vector2D& start, float s, const Vector2D& dir, Vector2D& result)
{
    Assert(start.IsValid() && IsFinite(s) && dir.IsValid());
    result.x = start.x + s*dir.x;
    result.y = start.y + s*dir.y;
}

// FIXME: Remove
// For backwards compatability
inline void	Vector2D::MulAdd(const Vector2D& a, const Vector2D& b, float scalar)
{
    x = a.x + b.x * scalar;
    y = a.y + b.y * scalar;
}

inline void Vector2DLerp(const Vector2D& src1, const Vector2D& src2, float t, Vector2D& dest)
{
    dest[0] = src1[0] + (src2[0] - src1[0]) * t;
    dest[1] = src1[1] + (src2[1] - src1[1]) * t;
}

//-----------------------------------------------------------------------------
// dot, cross
//-----------------------------------------------------------------------------
inline float DotProduct2D(const Vector2D& a, const Vector2D& b)
{
    Assert(a.IsValid() && b.IsValid());
    return(a.x*b.x + a.y*b.y);
}

// for backwards compatability
inline float Vector2D::Dot(const Vector2D& vOther) const
{
    return DotProduct2D(*this, vOther);
}


//-----------------------------------------------------------------------------
// length
//-----------------------------------------------------------------------------
inline float Vector2DLength(const Vector2D& v)
{
    Assert(v.IsValid());
    return (float)FastSqrt(v.x*v.x + v.y*v.y);
}

inline float Vector2D::LengthSqr(void) const
{
    Assert(IsValid());
    return (x*x + y*y);
}

inline float Vector2D::NormalizeInPlace()
{
    return Vector2DNormalize(*this);
}

inline bool Vector2D::IsLengthGreaterThan(float val) const
{
    return LengthSqr() > val*val;
}

inline bool Vector2D::IsLengthLessThan(float val) const
{
    return LengthSqr() < val*val;
}

inline float Vector2D::Length(void) const
{
    return Vector2DLength(*this);
}


inline void Vector2DMin(const Vector2D &a, const Vector2D &b, Vector2D &result)
{
    result.x = (a.x < b.x) ? a.x : b.x;
    result.y = (a.y < b.y) ? a.y : b.y;
}


inline void Vector2DMax(const Vector2D &a, const Vector2D &b, Vector2D &result)
{
    result.x = (a.x > b.x) ? a.x : b.x;
    result.y = (a.y > b.y) ? a.y : b.y;
}


//-----------------------------------------------------------------------------
// Normalization
//-----------------------------------------------------------------------------
inline float Vector2DNormalize(Vector2D& v)
{
    Assert(v.IsValid());
    float l = v.Length();
    if (l != 0.0f)
    {
        v /= l;
    }
    else
    {
        v.x = v.y = 0.0f;
    }
    return l;
}


//-----------------------------------------------------------------------------
// Get the distance from this Vector2D to the other one
//-----------------------------------------------------------------------------
inline float Vector2D::DistTo(const Vector2D &vOther) const
{
    Vector2D delta;
    Vector2DSubtract(*this, vOther, delta);
    return delta.Length();
}

inline float Vector2D::DistToSqr(const Vector2D &vOther) const
{
    Vector2D delta;
    Vector2DSubtract(*this, vOther, delta);
    return delta.LengthSqr();
}


//-----------------------------------------------------------------------------
// Computes the closest point to vecTarget no farther than flMaxDist from vecStart
//-----------------------------------------------------------------------------
inline void ComputeClosestPoint2D(const Vector2D& vecStart, float flMaxDist, const Vector2D& vecTarget, Vector2D *pResult)
{
    Vector2D vecDelta;
    Vector2DSubtract(vecTarget, vecStart, vecDelta);
    float flDistSqr = vecDelta.LengthSqr();
    if (flDistSqr <= flMaxDist * flMaxDist)
    {
        *pResult = vecTarget;
    }
    else
    {
        vecDelta /= FastSqrt(flDistSqr);
        Vector2DMA(vecStart, flMaxDist, vecDelta, *pResult);
    }
}



//-----------------------------------------------------------------------------
//
// Slow methods
//
//-----------------------------------------------------------------------------

#ifndef VECTOR_NO_SLOW_OPERATIONS
#endif
//-----------------------------------------------------------------------------
// Returns a Vector2D with the min or max in X, Y, and Z.
//-----------------------------------------------------------------------------

inline Vector2D Vector2D::Min(const Vector2D &vOther) const
{
    return Vector2D(x < vOther.x ? x : vOther.x,
        y < vOther.y ? y : vOther.y);
}

inline Vector2D Vector2D::Max(const Vector2D &vOther) const
{
    return Vector2D(x > vOther.x ? x : vOther.x,
        y > vOther.y ? y : vOther.y);
}


//-----------------------------------------------------------------------------
// arithmetic operations
//-----------------------------------------------------------------------------

inline Vector2D Vector2D::operator-(void) const
{
    return Vector2D(-x, -y);
}

inline Vector2D Vector2D::operator+(const Vector2D& v) const
{
    Vector2D res;
    Vector2DAdd(*this, v, res);
    return res;
}

inline Vector2D Vector2D::operator-(const Vector2D& v) const
{
    Vector2D res;
    Vector2DSubtract(*this, v, res);
    return res;
}

inline Vector2D Vector2D::operator+(const int i1) const
{
    Vector2D res;
    Vector2DAdd(*this, i1, res);
    return res;
}

inline Vector2D Vector2D::operator+(const float fl) const
{
    Vector2D res;
    Vector2DAdd(*this, fl, res);
    return res;
}

inline Vector2D Vector2D::operator*(const float fl) const
{
    Vector2D res;
    Vector2DMultiply(*this, fl, res);
    return res;
}

inline Vector2D Vector2D::operator*(const Vector2D& v) const
{
    Vector2D res;
    Vector2DMultiply(*this, v, res);
    return res;
}

inline Vector2D Vector2D::operator/(const float fl) const
{
    Vector2D res;
    Vector2DDivide(*this, fl, res);
    return res;
}

inline Vector2D Vector2D::operator/(const Vector2D& v) const
{
    Vector2D res;
    Vector2DDivide(*this, v, res);
    return res;
}

inline Vector2D operator*(const float fl, const Vector2D& v)
{
    return v * fl;
}

class QAngleByValue;
class QAngle
{
public:
    // Members
    float x, y, z;

    // Construction/destruction
    QAngle(void);
    QAngle(float X, float Y, float Z);
    //      QAngle(RadianEuler const &angles);      // evil auto type promotion!!!

    // Allow pass-by-value
    operator QAngleByValue &() { return *((QAngleByValue *)(this)); }
    operator const QAngleByValue &() const { return *((const QAngleByValue *)(this)); }

    // Initialization
    void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f);
    void Random(float minVal, float maxVal);

    // Got any nasty NAN's?
    bool IsValid() const;
    void Invalidate();

    // array access...
    float operator[](int i) const;
    float& operator[](int i);

    // Base address...
    float* Base();
    float const* Base() const;

    // equality
    bool operator==(const QAngle& v) const;
    bool operator!=(const QAngle& v) const;

    // arithmetic operations
    QAngle& operator+=(const QAngle &v);
    QAngle& operator-=(const QAngle &v);
    QAngle& operator*=(float s);
    QAngle& operator/=(float s);

    // Get the vector's magnitude.
    float   Length() const;
    float   LengthSqr() const;

    // negate the QAngle components
    //void  Negate();
	auto QAngle::Normalize() {
		auto x_rev = x / 360.f;
		if (x > 180.f || x < -180.f) {
			x_rev = abs(x_rev);
			x_rev = round(x_rev);

			if (x < 0.f)
				x = (x + 360.f * x_rev);

			else
				x = (x - 360.f * x_rev);
		}

		auto y_rev = y / 360.f;
		if (y > 180.f || y < -180.f) {
			y_rev = abs(y_rev);
			y_rev = round(y_rev);

			if (y < 0.f)
				y = (y + 360.f * y_rev);

			else
				y = (y - 360.f * y_rev);
		}

		auto z_rev = z / 360.f;
		if (z > 180.f || z < -180.f) {
			z_rev = abs(z_rev);
			z_rev = round(z_rev);

			if (z < 0.f)
				z = (z + 360.f * z_rev);

			else
				z = (z - 360.f * z_rev);
		}
	}
    // No assignment operators either...
    QAngle& operator=(const QAngle& src);

#ifndef VECTOR_NO_SLOW_OPERATIONS
    // copy constructors

    // arithmetic operations
    QAngle  operator-(void) const;

    QAngle  operator+(const QAngle& v) const;
    QAngle  operator-(const QAngle& v) const;
    QAngle  operator*(float fl) const;
    QAngle  operator*(const QAngle& v) const;
    QAngle  operator/(float fl) const;
#else

private:
    // No copy constructors allowed if we're in optimal mode
    QAngle(const QAngle& vOther);

#endif
};

//-----------------------------------------------------------------------------
// constructors
//-----------------------------------------------------------------------------
inline QAngle::QAngle(void)
{
#ifdef _DEBUG
#ifdef VECTOR_PARANOIA
    // Initialize to NAN to catch errors
    x = y = z = VEC_T_NAN;
#endif
#endif
}

inline QAngle::QAngle(float X, float Y, float Z)
{
    x = X; y = Y; z = Z;
    CHECK_VALID(*this);
}

//-----------------------------------------------------------------------------
// initialization
//-----------------------------------------------------------------------------
inline void QAngle::Init(float ix, float iy, float iz)
{
    x = ix; y = iy; z = iz;
    CHECK_VALID(*this);
}

inline void QAngle::Random(float minVal, float maxVal)
{
    x = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
    y = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
    z = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
    CHECK_VALID(*this);
}

//-----------------------------------------------------------------------------
// assignment
//-----------------------------------------------------------------------------
inline QAngle& QAngle::operator=(const QAngle &vOther)
{
    CHECK_VALID(vOther);
    x = vOther.x; y = vOther.y; z = vOther.z;
    return *this;
}

//-----------------------------------------------------------------------------
// comparison
//-----------------------------------------------------------------------------
inline bool QAngle::operator==(const QAngle& src) const
{
    CHECK_VALID(src);
    CHECK_VALID(*this);
    return (src.x == x) && (src.y == y) && (src.z == z);
}

inline bool QAngle::operator!=(const QAngle& src) const
{
    CHECK_VALID(src);
    CHECK_VALID(*this);
    return (src.x != x) || (src.y != y) || (src.z != z);
}

//-----------------------------------------------------------------------------
// standard math operations
//-----------------------------------------------------------------------------
inline QAngle& QAngle::operator+=(const QAngle& v)
{
    CHECK_VALID(*this);
    CHECK_VALID(v);
    x += v.x; y += v.y; z += v.z;
    return *this;
}

inline QAngle& QAngle::operator-=(const QAngle& v)
{
    CHECK_VALID(*this);
    CHECK_VALID(v);
    x -= v.x; y -= v.y; z -= v.z;
    return *this;
}

inline QAngle& QAngle::operator*=(float fl)
{
    x *= fl;
    y *= fl;
    z *= fl;
    CHECK_VALID(*this);
    return *this;
}

inline QAngle& QAngle::operator/=(float fl)
{
    Assert(fl != 0.0f);
    float oofl = 1.0f / fl;
    x *= oofl;
    y *= oofl;
    z *= oofl;
    CHECK_VALID(*this);
    return *this;
}

//-----------------------------------------------------------------------------
// Base address...
//-----------------------------------------------------------------------------
inline float* QAngle::Base()
{
    return (float*)this;
}

inline float const* QAngle::Base() const
{
    return (float const*)this;
}

//-----------------------------------------------------------------------------
// Array access
//-----------------------------------------------------------------------------
inline float& QAngle::operator[](int i)
{
    Assert((i >= 0) && (i < 3));
    return ((float*)this)[i];
}

inline float QAngle::operator[](int i) const
{
    Assert((i >= 0) && (i < 3));
    return ((float*)this)[i];
}

//-----------------------------------------------------------------------------
// length
//-----------------------------------------------------------------------------
inline float QAngle::Length() const
{
    CHECK_VALID(*this);
    return (float)FastSqrt(LengthSqr());
}


inline float QAngle::LengthSqr() const
{
    CHECK_VALID(*this);
    return x * x + y * y + z * z;
}


//-----------------------------------------------------------------------------
// arithmetic operations (SLOW!!)
//-----------------------------------------------------------------------------
#ifndef VECTOR_NO_SLOW_OPERATIONS

inline QAngle QAngle::operator-(void) const
{
    return QAngle(-x, -y, -z);
}

inline QAngle QAngle::operator+(const QAngle& v) const
{
    QAngle res;
    res.x = x + v.x;
    res.y = y + v.y;
    res.z = z + v.z;
    return res;
}

inline QAngle QAngle::operator-(const QAngle& v) const
{
    QAngle res;
    res.x = x - v.x;
    res.y = y - v.y;
    res.z = z - v.z;
    return res;
}

inline QAngle QAngle::operator*(float fl) const
{
    QAngle res;
    res.x = x * fl;
    res.y = y * fl;
    res.z = z * fl;
    return res;
}

inline QAngle QAngle::operator*(const QAngle& v) const
{
    QAngle res;
    res.x = x * v.x;
    res.y = y * v.y;
    res.z = z * v.z;
    return res;
}

inline QAngle QAngle::operator/(float fl) const
{
    QAngle res;
    res.x = x / fl;
    res.y = y / fl;
    res.z = z / fl;
    return res;
}

inline QAngle operator*(float fl, const QAngle& v)
{
    return v * fl;
}

#endif // VECTOR_NO_SLOW_OPERATIONS


//QANGLE SUBTRAC
inline void QAngleSubtract(const QAngle& a, const QAngle& b, QAngle& c)
{
    CHECK_VALID(a);
    CHECK_VALID(b);
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;
}

//QANGLEADD
inline void QAngleAdd(const QAngle& a, const QAngle& b, QAngle& c)
{
    CHECK_VALID(a);
    CHECK_VALID(b);
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    c.z = a.z + b.z;
}
template <typename T>
class UtlVector {
public:
	constexpr T& operator[](int i) noexcept { return memory[i]; };

	T* memory;
	int allocationCount;
	int growSize;
	int size;
	T* elements;
};
template < class T, class I = int >
class CUtlMemory
{
public:
	// constructor, destructor
	CUtlMemory(int nGrowSize = 0, int nInitSize = 0);
	CUtlMemory(T* pMemory, int numElements);
	CUtlMemory(const T* pMemory, int numElements);
	~CUtlMemory();

	// Set the size by which the memory grows
	void Init(int nGrowSize = 0, int nInitSize = 0);

	class Iterator_t
	{
	public:
		Iterator_t(I i) : index(i)
		{
		}

		I index;

		bool operator== (const Iterator_t it) const
		{
			return index == it.index;
		}

		bool operator!= (const Iterator_t it) const
		{
			return index != it.index;
		}
	};

	Iterator_t First() const
	{
		return Iterator_t(IsIdxValid(0) ? 0 : InvalidIndex());
	}

	Iterator_t Next(const Iterator_t& it) const
	{
		return Iterator_t(IsIdxValid(it.index + 1) ? it.index + 1 : InvalidIndex());
	}

	I GetIndex(const Iterator_t& it) const
	{
		return it.index;
	}

	bool IsIdxAfter(I i, const Iterator_t& it) const
	{
		return i > it.index;
	}

	bool IsValidIterator(const Iterator_t& it) const
	{
		return IsIdxValid(it.index);
	}

	Iterator_t InvalidIterator() const
	{
		return Iterator_t(InvalidIndex());
	}

	// element access
	T& operator[] (I i);
	const T& operator[] (I i) const;
	T& Element(I i);
	const T& Element(I i) const;

	bool IsIdxValid(I i) const;

	static const I INVALID_INDEX = (I)-1; // For use with COMPILE_TIME_ASSERT
	static I InvalidIndex()
	{
		return INVALID_INDEX;
	}

	T* Base();
	const T* Base() const;

	void SetExternalBuffer(T* pMemory, int numElements);
	void SetExternalBuffer(const T* pMemory, int numElements);
	void AssumeMemory(T* pMemory, int nSize);
	T* Detach();
	void* DetachMemory();

	void Swap(CUtlMemory< T, I >& mem);
	void ConvertToGrowableMemory(int nGrowSize);
	int NumAllocated() const;
	int Count() const;
	void Grow(int num = 1);
	void EnsureCapacity(int num);
	void Purge();
	void Purge(int numElements);
	bool IsExternallyAllocated() const;
	bool IsReadOnly() const;
	void SetGrowSize(int size);

protected:
	void ValidateGrowSize()
	{
	}

	enum
	{
		EXTERNAL_BUFFER_MARKER = -1,
		EXTERNAL_CONST_BUFFER_MARKER = -2,
	};

	T* m_pMemory;
	int m_nAllocationCount;
	int m_nGrowSize;
};

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------

template < class T, class I >
CUtlMemory< T, I >::CUtlMemory(int nGrowSize, int nInitAllocationCount) : m_pMemory(0),
m_nAllocationCount(nInitAllocationCount), m_nGrowSize(nGrowSize)
{
	ValidateGrowSize();
	assert(nGrowSize >= 0);

	if (m_nAllocationCount)
	{
		m_pMemory = (T*)new unsigned char[m_nAllocationCount * sizeof(T)];
		//m_pMemory = (T*)malloc(m_nAllocationCount * sizeof(T));
	}
}

template < class T, class I >
CUtlMemory< T, I >::CUtlMemory(T* pMemory, int numElements) : m_pMemory(pMemory),
m_nAllocationCount(numElements)
{
	// Special marker indicating externally supplied modifyable memory
	m_nGrowSize = EXTERNAL_BUFFER_MARKER;
}

template < class T, class I >
CUtlMemory< T, I >::CUtlMemory(const T* pMemory, int numElements) : m_pMemory((T*)pMemory),
m_nAllocationCount(numElements)
{
	// Special marker indicating externally supplied modifyable memory
	m_nGrowSize = EXTERNAL_CONST_BUFFER_MARKER;
}

template < class T, class I >
CUtlMemory< T, I >::~CUtlMemory()
{
	Purge();
}

template < class T, class I >
void CUtlMemory< T, I >::Init(int nGrowSize /*= 0*/, int nInitSize /*= 0*/)
{
	Purge();

	m_nGrowSize = nGrowSize;
	m_nAllocationCount = nInitSize;
	ValidateGrowSize();
	assert(nGrowSize >= 0);

	if (m_nAllocationCount)
	{
		UTLMEMORY_TRACK_ALLOC();
		MEM_ALLOC_CREDIT_CLASS();
		m_pMemory = (T*)malloc(m_nAllocationCount * sizeof(T));
	}
}

//-----------------------------------------------------------------------------
// Fast swap
//-----------------------------------------------------------------------------
template < class T, class I >
void CUtlMemory< T, I >::Swap(CUtlMemory< T, I >& mem)
{
	V_swap(m_nGrowSize, mem.m_nGrowSize);
	V_swap(m_pMemory, mem.m_pMemory);
	V_swap(m_nAllocationCount, mem.m_nAllocationCount);
}

//-----------------------------------------------------------------------------
// Switches the buffer from an external memory buffer to a reallocatable buffer
//-----------------------------------------------------------------------------
template < class T, class I >
void CUtlMemory< T, I >::ConvertToGrowableMemory(int nGrowSize)
{
	if (!IsExternallyAllocated())
		return;

	m_nGrowSize = nGrowSize;

	if (m_nAllocationCount)
	{
		int nNumBytes = m_nAllocationCount * sizeof(T);
		T* pMemory = (T*)malloc(nNumBytes);

		if (!pMemory)
			return;

		memcpy(pMemory, m_pMemory, nNumBytes);
		m_pMemory = pMemory;
	}
	else
		m_pMemory = NULL;
}

//-----------------------------------------------------------------------------
// Attaches the buffer to external memory....
//-----------------------------------------------------------------------------
template < class T, class I >
void CUtlMemory< T, I >::SetExternalBuffer(T* pMemory, int numElements)
{
	// Blow away any existing allocated memory
	Purge();

	m_pMemory = pMemory;
	m_nAllocationCount = numElements;

	// Indicate that we don't own the memory
	m_nGrowSize = EXTERNAL_BUFFER_MARKER;
}

template < class T, class I >
void CUtlMemory< T, I >::SetExternalBuffer(const T* pMemory, int numElements)
{
	// Blow away any existing allocated memory
	Purge();

	m_pMemory = const_cast<T*> (pMemory);
	m_nAllocationCount = numElements;

	// Indicate that we don't own the memory
	m_nGrowSize = EXTERNAL_CONST_BUFFER_MARKER;
}

template < class T, class I >
void CUtlMemory< T, I >::AssumeMemory(T* pMemory, int numElements)
{
	// Blow away any existing allocated memory
	Purge();

	// Simply take the pointer but don't mark us as external
	m_pMemory = pMemory;
	m_nAllocationCount = numElements;
}

template < class T, class I >
void* CUtlMemory< T, I >::DetachMemory()
{
	if (IsExternallyAllocated())
		return NULL;

	void* pMemory = m_pMemory;
	m_pMemory = 0;
	m_nAllocationCount = 0;
	return pMemory;
}

template < class T, class I >
inline T* CUtlMemory< T, I >::Detach()
{
	return (T*)DetachMemory();
}

//-----------------------------------------------------------------------------
// element access
//-----------------------------------------------------------------------------
template < class T, class I >
inline T& CUtlMemory< T, I >::operator[] (I i)
{
	assert(!IsReadOnly());
	assert(IsIdxValid(i));
	return m_pMemory[i];
}

template < class T, class I >
inline const T& CUtlMemory< T, I >::operator[] (I i) const
{
	assert(IsIdxValid(i));
	return m_pMemory[i];
}

template < class T, class I >
inline T& CUtlMemory< T, I >::Element(I i)
{
	assert(!IsReadOnly());
	assert(IsIdxValid(i));
	return m_pMemory[i];
}

template < class T, class I >
inline const T& CUtlMemory< T, I >::Element(I i) const
{
	assert(IsIdxValid(i));
	return m_pMemory[i];
}

//-----------------------------------------------------------------------------
// is the memory externally allocated?
//-----------------------------------------------------------------------------
template < class T, class I >
bool CUtlMemory< T, I >::IsExternallyAllocated() const
{
	return (m_nGrowSize < 0);
}

//-----------------------------------------------------------------------------
// is the memory read only?
//-----------------------------------------------------------------------------
template < class T, class I >
bool CUtlMemory< T, I >::IsReadOnly() const
{
	return (m_nGrowSize == EXTERNAL_CONST_BUFFER_MARKER);
}

template < class T, class I >
void CUtlMemory< T, I >::SetGrowSize(int nSize)
{
	assert(!IsExternallyAllocated());
	assert(nSize >= 0);
	m_nGrowSize = nSize;
	ValidateGrowSize();
}

//-----------------------------------------------------------------------------
// Gets the base address (can change when adding elements!)
//-----------------------------------------------------------------------------
template < class T, class I >
inline T* CUtlMemory< T, I >::Base()
{
	assert(!IsReadOnly());
	return m_pMemory;
}

template < class T, class I >
inline const T* CUtlMemory< T, I >::Base() const
{
	return m_pMemory;
}

//-----------------------------------------------------------------------------
// Size
//-----------------------------------------------------------------------------
template < class T, class I >
inline int CUtlMemory< T, I >::NumAllocated() const
{
	return m_nAllocationCount;
}

template < class T, class I >
inline int CUtlMemory< T, I >::Count() const
{
	return m_nAllocationCount;
}

//-----------------------------------------------------------------------------
// Is element index valid?
//-----------------------------------------------------------------------------
template < class T, class I >
inline bool CUtlMemory< T, I >::IsIdxValid(I i) const
{
	// GCC warns if I is an unsigned type and we do a ">= 0" against it (since the comparison is always 0).
	// We Get the warning even if we cast inside the expression. It only goes away if we assign to another variable.
	long x = i;
	return (x >= 0) && (x < m_nAllocationCount);
}

//-----------------------------------------------------------------------------
// Grows the memory
//-----------------------------------------------------------------------------
inline int UtlMemory_CalcNewAllocationCount(int nAllocationCount, int nGrowSize, int nNewSize, int nBytesItem)
{
	if (nGrowSize)
		nAllocationCount = ((1 + ((nNewSize - 1) / nGrowSize)) * nGrowSize);

	else
	{
		if (!nAllocationCount)
		{
			// Compute an allocation which is at least as big as a cache line...
			nAllocationCount = (31 + nBytesItem) / nBytesItem;
		}

		while (nAllocationCount < nNewSize)
		{
#ifndef _X360
			nAllocationCount *= 2;
#else
			int nNewAllocationCount = (nAllocationCount * 9) / 8; // 12.5 %

			if (nNewAllocationCount > nAllocationCount)
				nAllocationCount = nNewAllocationCount;
			else
				nAllocationCount *= 2;

#endif
		}
	}

	return nAllocationCount;
}

template < class T, class I >
void CUtlMemory< T, I >::Grow(int num)
{
	assert(num > 0);

	if (IsExternallyAllocated())
	{
		// Can't grow a buffer whose memory was externally allocated
		assert(0);
		return;
	}

	auto oldAllocationCount = m_nAllocationCount;
	// Make sure we have at least numallocated + num allocations.
	// Use the grow rules specified for this memory (in m_nGrowSize)
	int nAllocationRequested = m_nAllocationCount + num;

	int nNewAllocationCount = UtlMemory_CalcNewAllocationCount(m_nAllocationCount, m_nGrowSize, nAllocationRequested, sizeof(T));

	// if m_nAllocationRequested wraps index type I, recalculate
	if ((int)(I)nNewAllocationCount < nAllocationRequested)
	{
		if ((int)(I)nNewAllocationCount == 0 && (int)(I)(nNewAllocationCount - 1) >= nAllocationRequested)
		{
			--nNewAllocationCount; // deal w/ the common case of m_nAllocationCount == MAX_USHORT + 1
		}
		else
		{
			if ((int)(I)nAllocationRequested != nAllocationRequested)
			{
				// we've been asked to grow memory to a size s.t. the index type can't address the requested amount of memory
				assert(0);
				return;
			}

			while ((int)(I)nNewAllocationCount < nAllocationRequested)
				nNewAllocationCount = (nNewAllocationCount + nAllocationRequested) / 2;
		}
	}

	m_nAllocationCount = nNewAllocationCount;

	if (m_pMemory)
	{
		auto ptr = new unsigned char[m_nAllocationCount * sizeof(T)];

		memcpy(ptr, m_pMemory, oldAllocationCount * sizeof(T));
		m_pMemory = (T*)ptr;
	}
	else
		m_pMemory = (T*)new unsigned char[m_nAllocationCount * sizeof(T)];
}

//-----------------------------------------------------------------------------
// Makes sure we've got at least this much memory
//-----------------------------------------------------------------------------
template < class T, class I >
inline void CUtlMemory< T, I >::EnsureCapacity(int num)
{
	if (m_nAllocationCount >= num)
		return;

	if (IsExternallyAllocated())
	{
		// Can't grow a buffer whose memory was externally allocated
		assert(0);
		return;
	}

	m_nAllocationCount = num;

	if (m_pMemory)
		m_pMemory = (T*)realloc(m_pMemory, m_nAllocationCount * sizeof(T));

	else
		m_pMemory = (T*)malloc(m_nAllocationCount * sizeof(T));
}

//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------
template < class T, class I >
void CUtlMemory< T, I >::Purge()
{
	if (!IsExternallyAllocated())
	{
		if (m_pMemory)
		{
			free((void*)m_pMemory);
			m_pMemory = 0;
		}

		m_nAllocationCount = 0;
	}
}

template < class T, class I >
void CUtlMemory< T, I >::Purge(int numElements)
{
	assert(numElements >= 0);

	if (numElements > m_nAllocationCount)
	{
		// Ensure this isn't a grow request in disguise.
		assert(numElements <= m_nAllocationCount);
		return;
	}

	// If we have zero elements, simply do a purge:
	if (numElements == 0)
	{
		Purge();
		return;
	}

	if (IsExternallyAllocated())
	{
		// Can't shrink a buffer whose memory was externally allocated, fail silently like purge
		return;
	}

	// If the number of elements is the same as the allocation count, we are done.
	if (numElements == m_nAllocationCount)
		return;

	if (!m_pMemory)
	{
		// Allocation count is non zero, but memory is null.
		assert(m_pMemory);
		return;
	}

	m_nAllocationCount = numElements;
	m_pMemory = (T*)realloc(m_pMemory, m_nAllocationCount * sizeof(T));
}

//-----------------------------------------------------------------------------
// The CUtlMemory class:
// A growable memory class which doubles in size by default.
//-----------------------------------------------------------------------------
template < class T, int nAlignment >
class CUtlMemoryAligned : public CUtlMemory< T >
{
public:
	// constructor, destructor
	CUtlMemoryAligned(int nGrowSize = 0, int nInitSize = 0);
	CUtlMemoryAligned(T* pMemory, int numElements);
	CUtlMemoryAligned(const T* pMemory, int numElements);
	~CUtlMemoryAligned();

	// Attaches the buffer to external memory....
	void SetExternalBuffer(T* pMemory, int numElements);
	void SetExternalBuffer(const T* pMemory, int numElements);

	// Grows the memory, so that at least allocated + num elements are allocated
	void Grow(int num = 1);

	// Makes sure we've got at least this much memory
	void EnsureCapacity(int num);

	// Memory deallocation
	void Purge();

	// Purge all but the given number of elements (NOT IMPLEMENTED IN CUtlMemoryAligned)
	void Purge(int numElements)
	{
		assert(0);
	}

private:
	void* Align(const void* pAddr);
};

//-----------------------------------------------------------------------------
// Aligns a pointer
//-----------------------------------------------------------------------------
template < class T, int nAlignment >
void* CUtlMemoryAligned< T, nAlignment >::Align(const void* pAddr)
{
	size_t nAlignmentMask = nAlignment - 1;
	return (void*)(((size_t)pAddr + nAlignmentMask) & (~nAlignmentMask));
}

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
template < class T, int nAlignment >
CUtlMemoryAligned< T, nAlignment >::CUtlMemoryAligned(int nGrowSize, int nInitAllocationCount)
{
	CUtlMemory< T >::m_pMemory = 0;
	CUtlMemory< T >::m_nAllocationCount = nInitAllocationCount;
	CUtlMemory< T >::m_nGrowSize = nGrowSize;
	this->ValidateGrowSize();

	// Alignment must be a power of two
	COMPILE_TIME_ASSERT((nAlignment & (nAlignment - 1)) == 0);
	assert((nGrowSize >= 0) && (nGrowSize != CUtlMemory<T>::EXTERNAL_BUFFER_MARKER));

	if (CUtlMemory< T >::m_nAllocationCount)
	{
		UTLMEMORY_TRACK_ALLOC();
		MEM_ALLOC_CREDIT_CLASS();
		CUtlMemory< T >::m_pMemory = (T*)_aligned_malloc(nInitAllocationCount * sizeof(T), nAlignment);
	}
}

template < class T, int nAlignment >
CUtlMemoryAligned< T, nAlignment >::CUtlMemoryAligned(T* pMemory, int numElements)
{
	// Special marker indicating externally supplied memory
	CUtlMemory< T >::m_nGrowSize = CUtlMemory< T >::EXTERNAL_BUFFER_MARKER;

	CUtlMemory< T >::m_pMemory = (T*)Align(pMemory);
	CUtlMemory< T >::m_nAllocationCount = ((int)(pMemory + numElements) - (int)CUtlMemory< T >::m_pMemory) / sizeof(T);
}

template < class T, int nAlignment >
CUtlMemoryAligned< T, nAlignment >::CUtlMemoryAligned(const T* pMemory, int numElements)
{
	// Special marker indicating externally supplied memory
	CUtlMemory< T >::m_nGrowSize = CUtlMemory< T >::EXTERNAL_CONST_BUFFER_MARKER;

	CUtlMemory< T >::m_pMemory = (T*)Align(pMemory);
	CUtlMemory< T >::m_nAllocationCount = ((int)(pMemory + numElements) - (int)CUtlMemory< T >::m_pMemory) / sizeof(T);
}

template < class T, int nAlignment >
CUtlMemoryAligned< T, nAlignment >::~CUtlMemoryAligned()
{
	Purge();
}

//-----------------------------------------------------------------------------
// Attaches the buffer to external memory....
//-----------------------------------------------------------------------------
template < class T, int nAlignment >
void CUtlMemoryAligned< T, nAlignment >::SetExternalBuffer(T* pMemory, int numElements)
{
	// Blow away any existing allocated memory
	Purge();

	CUtlMemory< T >::m_pMemory = (T*)Align(pMemory);
	CUtlMemory< T >::m_nAllocationCount = ((int)(pMemory + numElements) - (int)CUtlMemory< T >::m_pMemory) / sizeof(T);

	// Indicate that we don't own the memory
	CUtlMemory< T >::m_nGrowSize = CUtlMemory< T >::EXTERNAL_BUFFER_MARKER;
}

template < class T, int nAlignment >
void CUtlMemoryAligned< T, nAlignment >::SetExternalBuffer(const T* pMemory, int numElements)
{
	// Blow away any existing allocated memory
	Purge();

	CUtlMemory< T >::m_pMemory = (T*)Align(pMemory);
	CUtlMemory< T >::m_nAllocationCount = ((int)(pMemory + numElements) - (int)CUtlMemory< T >::m_pMemory) / sizeof(T);

	// Indicate that we don't own the memory
	CUtlMemory< T >::m_nGrowSize = CUtlMemory< T >::EXTERNAL_CONST_BUFFER_MARKER;
}

//-----------------------------------------------------------------------------
// Grows the memory
//-----------------------------------------------------------------------------
template < class T, int nAlignment >
void CUtlMemoryAligned< T, nAlignment >::Grow(int num)
{
	assert(num > 0);

	if (this->IsExternallyAllocated())
	{
		// Can't grow a buffer whose memory was externally allocated
		assert(0);
		return;
	}

	UTLMEMORY_TRACK_FREE();

	// Make sure we have at least numallocated + num allocations.
	// Use the grow rules specified for this memory (in m_nGrowSize)
	int nAllocationRequested = CUtlMemory< T >::m_nAllocationCount + num;

	CUtlMemory< T >::m_nAllocationCount = UtlMemory_CalcNewAllocationCount(CUtlMemory< T >::m_nAllocationCount, CUtlMemory< T >::m_nGrowSize,
		nAllocationRequested, sizeof(T));

	UTLMEMORY_TRACK_ALLOC();

	if (CUtlMemory< T >::m_pMemory)
	{
		MEM_ALLOC_CREDIT_CLASS();
		CUtlMemory< T >::m_pMemory = (T*)MemAlloc_ReallocAligned(CUtlMemory< T >::m_pMemory, CUtlMemory< T >::m_nAllocationCount * sizeof(T),
			nAlignment);
		assert(CUtlMemory<T>::m_pMemory);
	}
	else
	{
		MEM_ALLOC_CREDIT_CLASS();
		CUtlMemory< T >::m_pMemory = (T*)MemAlloc_AllocAligned(CUtlMemory< T >::m_nAllocationCount * sizeof(T), nAlignment);
		assert(CUtlMemory<T>::m_pMemory);
	}
}

//-----------------------------------------------------------------------------
// Makes sure we've got at least this much memory
//-----------------------------------------------------------------------------
template < class T, int nAlignment >
inline void CUtlMemoryAligned< T, nAlignment >::EnsureCapacity(int num)
{
	if (CUtlMemory< T >::m_nAllocationCount >= num)
		return;

	if (this->IsExternallyAllocated())
	{
		// Can't grow a buffer whose memory was externally allocated
		assert(0);
		return;
	}

	UTLMEMORY_TRACK_FREE();

	CUtlMemory< T >::m_nAllocationCount = num;

	UTLMEMORY_TRACK_ALLOC();

	if (CUtlMemory< T >::m_pMemory)
	{
		MEM_ALLOC_CREDIT_CLASS();
		CUtlMemory< T >::m_pMemory = (T*)MemAlloc_ReallocAligned(CUtlMemory< T >::m_pMemory, CUtlMemory< T >::m_nAllocationCount * sizeof(T),
			nAlignment);
	}
	else
	{
		MEM_ALLOC_CREDIT_CLASS();
		CUtlMemory< T >::m_pMemory = (T*)MemAlloc_AllocAligned(CUtlMemory< T >::m_nAllocationCount * sizeof(T), nAlignment);
	}
}

//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------
template < class T, int nAlignment >
void CUtlMemoryAligned< T, nAlignment >::Purge()
{
	if (!this->IsExternallyAllocated())
	{
		if (CUtlMemory< T >::m_pMemory)
		{
			UTLMEMORY_TRACK_FREE();
			MemAlloc_FreeAligned(CUtlMemory< T >::m_pMemory);
			CUtlMemory< T >::m_pMemory = 0;
		}

		CUtlMemory< T >::m_nAllocationCount = 0;
	}
}
template < class T, class A = CUtlMemory< T > >
class CUtlVector
{
	typedef A CAllocator;
public:
	typedef T ElemType_t;

	// constructor, destructor
	CUtlVector(int growSize = 0, int initSize = 0);
	CUtlVector(T* pMemory, int allocationCount, int numElements = 0);
	~CUtlVector();

	// Copy the array.
	CUtlVector< T, A >& operator= (const CUtlVector< T, A >& other);

	// element access
	T& operator[] (int i);
	const T& operator[] (int i) const;
	T& Element(int i);
	const T& Element(int i) const;
	T& Head();
	const T& Head() const;
	T& Tail();
	const T& Tail() const;

	// Gets the base address (can change when adding elements!)
	T* Base()
	{
		return m_Memory.Base();
	}

	const T* Base() const
	{
		return m_Memory.Base();
	}

	// Returns the number of elements in the vector
	int Count() const;
	// Is element index valid?
	bool IsValidIndex(int i) const;
	static int InvalidIndex();
	// Adds an element, uses default constructor
	int AddToHead();
	int AddToTail();
	int InsertBefore(int elem);
	int InsertAfter(int elem);
	// Adds an element, uses copy constructor
	int AddToHead(const T& src);
	int AddToTail(const T& src);
	int InsertBefore(int elem, const T& src);
	int InsertAfter(int elem, const T& src);
	// Adds multiple elements, uses default constructor
	int AddMultipleToHead(int num);
	int AddMultipleToTail(int num);
	int AddMultipleToTail(int num, const T* pToCopy);
	int InsertMultipleBefore(int elem, int num);
	int InsertMultipleBefore(int elem, int num, const T* pToCopy);
	int InsertMultipleAfter(int elem, int num);
	// Calls RemoveAll() then AddMultipleToTail.
	void SetSize(int size);
	void SetCount(int count);
	void SetCountNonDestructively(int count);
	//sets count by adding or removing elements to tail TODO: This should probably be the default behavior for SetCount
	void CopyArray(const T* pArray, int size); //Calls SetSize and copies each element.
	// Fast swap
	void Swap(CUtlVector< T, A >& vec);
	// Add the specified array to the tail.
	int AddVectorToTail(CUtlVector< T, A > const& src);
	// Finds an element (element needs operator== defined)
	int GetOffset(const T& src) const;
	void FillWithValue(const T& src);
	bool HasElement(const T& src) const;
	// Makes sure we have enough memory allocated to store a requested # of elements
	void EnsureCapacity(int num);
	// Makes sure we have at least this many elements
	void EnsureCount(int num);
	// Element removal
	void FastRemove(int elem); // doesn't preserve order
	void Remove(int elem); // preserves order, shifts elements
	bool FindAndRemove(const T& src); // removes first occurrence of src, preserves order, shifts elements
	bool FindAndFastRemove(const T& src); // removes first occurrence of src, doesn't preserve order
	void RemoveMultiple(int elem, int num); // preserves order, shifts elements
	void RemoveMultipleFromHead(int num); // removes num elements from tail
	void RemoveMultipleFromTail(int num); // removes num elements from tail
	void RemoveAll(); // doesn't deallocate memory
	void Purge(); // Memory deallocation
	// Purges the list and calls delete on each element in it.
	void PurgeAndDeleteElements();
	// Compacts the vector to the number of elements actually in use 
	void Compact();
	// Set the size by which it grows when it needs to allocate more memory.
	void SetGrowSize(int size)
	{
		m_Memory.SetGrowSize(size);
	}

	int NumAllocated() const; // Only use this if you really know what you're doing!
	void Sort(int(__cdecl* pfnCompare) (const T*, const T*));


	// Can't copy this unless we explicitly do it!
	CUtlVector(CUtlVector const& vec)
	{
		assert(0);
	}

	// Grows the vector
	void GrowVector(int num = 1);

	// Shifts elements....
	void ShiftElementsRight(int elem, int num = 1);
	void ShiftElementsLeft(int elem, int num = 1);

public:
	CAllocator m_Memory;
	int m_Size;

	// For easier access to the elements through the debugger
	// it's in release builds so this can be used in libraries correctly
	T* m_pElements;

	inline void ResetDbgInfo()
	{
		m_pElements = Base();
	}
};

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
template < typename T, class A >
inline CUtlVector< T, A >::CUtlVector(int growSize, int initSize) :
	m_Memory(growSize, initSize), m_Size(0)
{
	ResetDbgInfo();
}

template < typename T, class A >
inline CUtlVector< T, A >::CUtlVector(T* pMemory, int allocationCount, int numElements) :
	m_Memory(pMemory, allocationCount), m_Size(numElements)
{
	ResetDbgInfo();
}

template < typename T, class A >
inline CUtlVector< T, A >::~CUtlVector()
{
	Purge();
}

template < typename T, class A >
inline CUtlVector< T, A >& CUtlVector< T, A >::operator= (const CUtlVector< T, A >& other)
{
	int nCount = other.Count();
	SetSize(nCount);
	for (int i = 0; i < nCount; i++)
	{
		(*this)[i] = other[i];
	}
	return *this;
}

//-----------------------------------------------------------------------------
// element access
//-----------------------------------------------------------------------------
template < typename T, class A >
inline T& CUtlVector< T, A >::operator[] (int i)
{
	assert(i < m_Size);
	return m_Memory[i];
}

template < typename T, class A >
inline const T& CUtlVector< T, A >::operator[] (int i) const
{
	assert(i < m_Size);
	return m_Memory[i];
}

template < typename T, class A >
inline T& CUtlVector< T, A >::Element(int i)
{
	assert(i < m_Size);
	return m_Memory[i];
}

template < typename T, class A >
inline const T& CUtlVector< T, A >::Element(int i) const
{
	assert(i < m_Size);
	return m_Memory[i];
}

template < typename T, class A >
inline T& CUtlVector< T, A >::Head()
{
	assert(m_Size > 0);
	return m_Memory[0];
}

template < typename T, class A >
inline const T& CUtlVector< T, A >::Head() const
{
	assert(m_Size > 0);
	return m_Memory[0];
}

template < typename T, class A >
inline T& CUtlVector< T, A >::Tail()
{
	assert(m_Size > 0);
	return m_Memory[m_Size - 1];
}

template < typename T, class A >
inline const T& CUtlVector< T, A >::Tail() const
{
	assert(m_Size > 0);
	return m_Memory[m_Size - 1];
}

//-----------------------------------------------------------------------------
// Count
//-----------------------------------------------------------------------------
template < typename T, class A >
inline int CUtlVector< T, A >::Count() const
{
	return m_Size;
}

//-----------------------------------------------------------------------------
// Is element index valid?
//-----------------------------------------------------------------------------
template < typename T, class A >
inline bool CUtlVector< T, A >::IsValidIndex(int i) const
{
	return (i >= 0) && (i < m_Size);
}

//-----------------------------------------------------------------------------
// Returns in invalid index
//-----------------------------------------------------------------------------
template < typename T, class A >
inline int CUtlVector< T, A >::InvalidIndex()
{
	return -1;
}

//-----------------------------------------------------------------------------
// Grows the vector
//-----------------------------------------------------------------------------
template < typename T, class A >
void CUtlVector< T, A >::GrowVector(int num)
{
	if (m_Size + num > m_Memory.NumAllocated())
	{
		m_Memory.Grow(m_Size + num - m_Memory.NumAllocated());
	}

	m_Size += num;
	ResetDbgInfo();
}

//-----------------------------------------------------------------------------
// Sorts the vector
//-----------------------------------------------------------------------------
template < typename T, class A >
void CUtlVector< T, A >::Sort(int(__cdecl* pfnCompare) (const T*, const T*))
{
	typedef int(__cdecl * QSortCompareFunc_t) (const void*, const void*);
	if (Count() <= 1)
		return;

	if (Base())
	{
		qsort(Base(), Count(), sizeof(T), (QSortCompareFunc_t)(pfnCompare));
	}
	else
	{
		assert(0);
		// this path is untested
		// if you want to sort vectors that use a non-sequential memory allocator,
		// you'll probably want to patch in a quicksort algorithm here
		// I just threw in this bubble sort to have something just in case...

		for (int i = m_Size - 1; i >= 0; --i)
		{
			for (int j = 1; j <= i; ++j)
			{
				if (pfnCompare(&Element(j - 1), &Element(j)) < 0)
				{
					V_swap(Element(j - 1), Element(j));
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Makes sure we have enough memory allocated to store a requested # of elements
//-----------------------------------------------------------------------------
template < typename T, class A >
void CUtlVector< T, A >::EnsureCapacity(int num)
{
	MEM_ALLOC_CREDIT_CLASS();
	m_Memory.EnsureCapacity(num);
	ResetDbgInfo();
}

//-----------------------------------------------------------------------------
// Makes sure we have at least this many elements
//-----------------------------------------------------------------------------
template < typename T, class A >
void CUtlVector< T, A >::EnsureCount(int num)
{
	if (Count() < num)
	{
		AddMultipleToTail(num - Count());
	}
}

//-----------------------------------------------------------------------------
// Shifts elements
//-----------------------------------------------------------------------------
template < typename T, class A >
void CUtlVector< T, A >::ShiftElementsRight(int elem, int num)
{
	assert(IsValidIndex(elem) || (m_Size == 0) || (num == 0));
	int numToMove = m_Size - elem - num;
	if ((numToMove > 0) && (num > 0))
		memmove(&Element(elem + num), &Element(elem), numToMove * sizeof(T));
}

template < typename T, class A >
void CUtlVector< T, A >::ShiftElementsLeft(int elem, int num)
{
	assert(IsValidIndex(elem) || (m_Size == 0) || (num == 0));
	int numToMove = m_Size - elem - num;
	if ((numToMove > 0) && (num > 0))
	{
		memmove(&Element(elem), &Element(elem + num), numToMove * sizeof(T));

#ifdef _DEBUG
		memset(&Element(m_Size - num), 0xDD, num * sizeof(T));
#endif
	}
}

//-----------------------------------------------------------------------------
// Adds an element, uses default constructor
//-----------------------------------------------------------------------------
template < typename T, class A >
inline int CUtlVector< T, A >::AddToHead()
{
	return InsertBefore(0);
}

template < typename T, class A >
inline int CUtlVector< T, A >::AddToTail()
{
	return InsertBefore(m_Size);
}

template < typename T, class A >
inline int CUtlVector< T, A >::InsertAfter(int elem)
{
	return InsertBefore(elem + 1);
}

template < typename T, class A >
int CUtlVector< T, A >::InsertBefore(int elem)
{
	// Can insert at the end
	assert((elem == Count()) || IsValidIndex(elem));

	GrowVector();
	ShiftElementsRight(elem);
	Construct(&Element(elem));
	return elem;
}

//-----------------------------------------------------------------------------
// Adds an element, uses copy constructor
//-----------------------------------------------------------------------------
template < typename T, class A >
inline int CUtlVector< T, A >::AddToHead(const T& src)
{
	// Can't insert something that's in the list... reallocation may hose us
	assert((Base() == NULL) || (&src < Base()) || (&src >= (Base() + Count())));
	return InsertBefore(0, src);
}

template < typename T, class A >
inline int CUtlVector< T, A >::AddToTail(const T& src)
{
	// Can't insert something that's in the list... reallocation may hose us
	assert((Base() == NULL) || (&src < Base()) || (&src >= (Base() + Count())));
	return InsertBefore(m_Size, src);
}

template < typename T, class A >
inline int CUtlVector< T, A >::InsertAfter(int elem, const T& src)
{
	// Can't insert something that's in the list... reallocation may hose us
	assert((Base() == NULL) || (&src < Base()) || (&src >= (Base() + Count())));
	return InsertBefore(elem + 1, src);
}

template < typename T, class A >
int CUtlVector< T, A >::InsertBefore(int elem, const T& src)
{
	// Can't insert something that's in the list... reallocation may hose us
	assert((Base() == NULL) || (&src < Base()) || (&src >= (Base() + Count())));

	// Can insert at the end
	assert((elem == Count()) || IsValidIndex(elem));

	GrowVector();
	ShiftElementsRight(elem);
	CopyConstruct(&Element(elem), src);
	return elem;
}

//-----------------------------------------------------------------------------
// Adds multiple elements, uses default constructor
//-----------------------------------------------------------------------------
template < typename T, class A >
inline int CUtlVector< T, A >::AddMultipleToHead(int num)
{
	return InsertMultipleBefore(0, num);
}

template < typename T, class A >
inline int CUtlVector< T, A >::AddMultipleToTail(int num)
{
	return InsertMultipleBefore(m_Size, num);
}

template < typename T, class A >
inline int CUtlVector< T, A >::AddMultipleToTail(int num, const T* pToCopy)
{
	// Can't insert something that's in the list... reallocation may hose us
	assert((Base() == NULL) || !pToCopy || (pToCopy + num <= Base()) || (pToCopy >= (Base() + Count())));

	return InsertMultipleBefore(m_Size, num, pToCopy);
}

template < typename T, class A >
int CUtlVector< T, A >::InsertMultipleAfter(int elem, int num)
{
	return InsertMultipleBefore(elem + 1, num);
}

template < typename T, class A >
void CUtlVector< T, A >::SetCount(int count)
{
	RemoveAll();
	AddMultipleToTail(count);
}

template < typename T, class A >
inline void CUtlVector< T, A >::SetSize(int size)
{
	SetCount(size);
}

template < typename T, class A >
void CUtlVector< T, A >::SetCountNonDestructively(int count)
{
	int delta = count - m_Size;
	if (delta > 0)
		AddMultipleToTail(delta);
	else
		if (delta < 0)
			RemoveMultipleFromTail(-delta);
}

template < typename T, class A >
void CUtlVector< T, A >::CopyArray(const T* pArray, int size)
{
	// Can't insert something that's in the list... reallocation may hose us
	assert((Base() == NULL) || !pArray || (Base() >= (pArray + size)) || (pArray >= (Base() + Count())));

	SetSize(size);
	for (int i = 0; i < size; i++)
	{
		(*this)[i] = pArray[i];
	}
}

template < typename T, class A >
void CUtlVector< T, A >::Swap(CUtlVector< T, A >& vec)
{
	m_Memory.Swap(vec.m_Memory);
	V_swap(m_Size, vec.m_Size);
#ifndef _X360
	V_swap(m_pElements, vec.m_pElements);
#endif
}

template < typename T, class A >
int CUtlVector< T, A >::AddVectorToTail(CUtlVector const& src)
{
	assert(&src != this);

	int base = Count();

	// Make space.
	int nSrcCount = src.Count();
	EnsureCapacity(base + nSrcCount);

	// Copy the elements.	
	m_Size += nSrcCount;
	for (int i = 0; i < nSrcCount; i++)
	{
		CopyConstruct(&Element(base + i), src[i]);
	}
	return base;
}

template < typename T, class A >
inline int CUtlVector< T, A >::InsertMultipleBefore(int elem, int num)
{
	if (num == 0)
		return elem;

	// Can insert at the end
	assert((elem == Count()) || IsValidIndex(elem));

	GrowVector(num);
	ShiftElementsRight(elem, num);

	// Invoke default constructors
	for (int i = 0; i < num; ++i)
	{
		Construct(&Element(elem + i));
	}

	return elem;
}

template < typename T, class A >
inline int CUtlVector< T, A >::InsertMultipleBefore(int elem, int num, const T* pToInsert)
{
	if (num == 0)
		return elem;

	// Can insert at the end
	assert((elem == Count()) || IsValidIndex(elem));

	GrowVector(num);
	ShiftElementsRight(elem, num);

	// Invoke default constructors
	if (!pToInsert)
	{
		for (int i = 0; i < num; ++i)
		{
			Construct(&Element(elem + i));
		}
	}
	else
	{
		for (int i = 0; i < num; i++)
		{
			CopyConstruct(&Element(elem + i), pToInsert[i]);
		}
	}

	return elem;
}

//-----------------------------------------------------------------------------
// Finds an element (element needs operator== defined)
//-----------------------------------------------------------------------------
template < typename T, class A >
int CUtlVector< T, A >::GetOffset(const T& src) const
{
	for (int i = 0; i < Count(); ++i)
	{
		if (Element(i) == src)
			return i;
	}
	return -1;
}

template < typename T, class A >
void CUtlVector< T, A >::FillWithValue(const T& src)
{
	for (int i = 0; i < Count(); i++)
	{
		Element(i) = src;
	}
}

template < typename T, class A >
bool CUtlVector< T, A >::HasElement(const T& src) const
{
	return (GetOffset(src) >= 0);
}

//-----------------------------------------------------------------------------
// Element removal
//-----------------------------------------------------------------------------
template < typename T, class A >
void CUtlVector< T, A >::FastRemove(int elem)
{
	assert(IsValidIndex(elem));

	Destruct(&Element(elem));
	if (m_Size > 0)
	{
		if (elem != m_Size - 1)
			memcpy(&Element(elem), &Element(m_Size - 1), sizeof(T));
		--m_Size;
	}
}

template < typename T, class A >
void CUtlVector< T, A >::Remove(int elem)
{
	Destruct(&Element(elem));
	ShiftElementsLeft(elem);
	--m_Size;
}

template < typename T, class A >
bool CUtlVector< T, A >::FindAndRemove(const T& src)
{
	int elem = GetOffset(src);
	if (elem != -1)
	{
		Remove(elem);
		return true;
	}
	return false;
}

template < typename T, class A >
bool CUtlVector< T, A >::FindAndFastRemove(const T& src)
{
	int elem = GetOffset(src);
	if (elem != -1)
	{
		FastRemove(elem);
		return true;
	}
	return false;
}

template < typename T, class A >
void CUtlVector< T, A >::RemoveMultiple(int elem, int num)
{
	assert(elem >= 0);
	assert(elem + num <= Count());

	for (int i = elem + num; --i >= elem; )
		Destruct(&Element(i));

	ShiftElementsLeft(elem, num);
	m_Size -= num;
}

template < typename T, class A >
void CUtlVector< T, A >::RemoveMultipleFromHead(int num)
{
	assert(num <= Count());

	for (int i = num; --i >= 0; )
		Destruct(&Element(i));

	ShiftElementsLeft(0, num);
	m_Size -= num;
}

template < typename T, class A >
void CUtlVector< T, A >::RemoveMultipleFromTail(int num)
{
	assert(num <= Count());

	for (int i = m_Size - num; i < m_Size; i++)
		Destruct(&Element(i));

	m_Size -= num;
}

template < typename T, class A >
void CUtlVector< T, A >::RemoveAll()
{
	for (int i = m_Size; --i >= 0; )
	{
		Destruct(&Element(i));
	}

	m_Size = 0;
}

//-----------------------------------------------------------------------------
// Memory deallocation
//-----------------------------------------------------------------------------

template < typename T, class A >
inline void CUtlVector< T, A >::Purge()
{
	RemoveAll();
	m_Memory.Purge();
	ResetDbgInfo();
}

template < typename T, class A >
inline void CUtlVector< T, A >::PurgeAndDeleteElements()
{
	for (int i = 0; i < m_Size; i++)
	{
		delete Element(i);
	}
	Purge();
}

template < typename T, class A >
inline void CUtlVector< T, A >::Compact()
{
	m_Memory.Purge(m_Size);
}

template < typename T, class A >
inline int CUtlVector< T, A >::NumAllocated() const
{
	return m_Memory.NumAllocated();
}

//-----------------------------------------------------------------------------
// Data and memory validation
//-----------------------------------------------------------------------------
#ifdef DBGFLAG_VALIDATE
template< typename T, class A >
void CUtlVector<T, A>::Validate(CValidator& validator, char* pchName)
{
	validator.Push(typeid(*this).name(), this, pchName);

	m_Memory.Validate(validator, "m_Memory");

	validator.Pop();
}
#endif // DBGFLAG_VALIDATE

// A vector class for storing pointers, so that the elements pointed to by the pointers are deleted
// on exit.
template < class T >
class CUtlVectorAutoPurge : public CUtlVector< T, CUtlMemory< T, int > >
{
public:
	~CUtlVectorAutoPurge(void)
	{
		this->PurgeAndDeleteElements();
	}
};

// easy string list class with dynamically allocated strings. For use with V_SplitString, etc.
// Frees the dynamic strings in destructor.
class CUtlStringList : public CUtlVectorAutoPurge< char* >
{
public:
	void CopyAndAddToTail(char const* pString) // clone the string and add to the end
	{
		char* pNewStr = new char[1 + strlen(pString)];
		strcpy_s(pNewStr, 1 + strlen(pString), pString);
		AddToTail(pNewStr);
	}

	static int __cdecl SortFunc(char* const* sz1, char* const* sz2)
	{
		return strcmp(*sz1, *sz2);
	}
};
//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose:
//
// $NoKeywords: $
//
//===========================================================================//
#pragma once

#include <malloc.h>
#include <limits.h>
#include <float.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <new.h>

#define COMPILER_MSVC

//-----------------------------------------------------------------------------
// NOTE: All compiler defines are Set up in the base VPC scripts
// COMPILER_MSVC, COMPILER_MSVC32, COMPILER_MSVC64, COMPILER_MSVCX360
// COMPILER_GCC
// The rationale for this is that we need COMPILER_MSVC for the pragma blocks
// #pragma once that occur at the top of all header files, therefore we can't
// place the defines for these in here.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Set up platform defines.
//-----------------------------------------------------------------------------
#ifdef _WIN32
#define IsPlatformLinux()	false
#define IsPlatformPosix()	false
#define IsPlatformOSX()		false
#define IsPlatformPS3()		false
#define IsPlatformWindows() true
#ifndef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS	1
#endif

#ifndef _X360
#define IsPlatformX360() false
#define IsPlatformWindowsPC() true
#define PLATFORM_WINDOWS_PC 1

#ifdef _WIN64
#define IsPlatformWindowsPC64() true
#define IsPlatformWindowsPC32() false
#define PLATFORM_WINDOWS_PC64 1
#else
#define IsPlatformWindowsPC64() false
#define IsPlatformWindowsPC32() true
#define PLATFORM_WINDOWS_PC32 1
#endif

#else // _X360

#define IsPlatformWindowsPC()	false
#define IsPlatformWindowsPC64() false
#define IsPlatformWindowsPC32() false
#define IsPlatformX360()		true
#define PLATFORM_X360 1

#endif // _X360

#elif defined(POSIX)
#define IsPlatformX360()		false
#define IsPlatformPS3()			false
#define IsPlatformWindows()		false
#define IsPlatformWindowsPC()	false
#define IsPlatformWindowsPC64()	false
#define IsPlatformWindowsPC32()	false
#define IsPlatformPosix()		true
#ifndef PLATFORM_POSIX
#define PLATFORM_POSIX 1
#endif

#if defined( LINUX )
#define IsPlatformLinux() true
#define IsPlatformOSX() false
#ifndef PLATFORM_LINUX
#define PLATFORM_LINUX 1
#endif
#elif defined ( OSX )
#define IsPlatformLinux() false
#define IsPlatformOSX() true
#ifndef PLATFORM_OSX
#define PLATFORM_OSX 1
#endif
#else
#define IsPlatformLinux() false
#define IsPlatformOSX() false
#endif

#else
#error
#endif

//-----------------------------------------------------------------------------
// Set up platform type defines.
//-----------------------------------------------------------------------------
#ifdef PLATFORM_X360
#ifndef _CONSOLE
#define _CONSOLE
#endif
#define IsPC()		false
#define IsConsole() true
#else
#define IsPC()		true
#define IsConsole() false
#endif

//-----------------------------------------------------------------------------
// Set up build configuration defines.
//-----------------------------------------------------------------------------
#ifdef _CERT
#define IsCert() true
#else
#define IsCert() false
#endif

#ifdef _DEBUG
#define IsRelease() false
#define IsDebug() true
#else
#define IsRelease() true
#define IsDebug() false
#endif

#ifdef _RETAIL
#define IsRetail() true
#else
#define IsRetail() false
#endif

// Maximum and minimum representable values
#if !defined(PLATFORM_OSX) && !defined(__STDC_LIMIT_MACROS)

#ifndef INT8_MAX
#define  INT8_MAX			SCHAR_MAX
#endif
#ifndef INT16_MAX
#define  INT16_MAX			SHRT_MAX
#endif
#ifndef INT32_MAX
#define  INT32_MAX			LONG_MAX
#endif
#ifndef INT64_MAX
#define  INT64_MAX			(((int64_t)~0) >> 1)
#endif

#ifndef INT8_MIN
#define  INT8_MIN			SCHAR_MIN
#endif
#ifndef INT16_MIN
#define  INT16_MIN			SHRT_MIN
#endif
#ifndef INT32_MIN
#define  INT32_MIN			LONG_MIN
#endif
#ifndef INT64_MIN
#define  INT64_MIN			(((int64_t)1) << 63)
#endif

#ifndef UINT8_MAX
#define  UINT8_MAX			((uint8_t)~0)
#endif
#ifndef UINT16_MAX
#define  UINT16_MAX			((uint16)~0)
#endif
#ifndef UINT32_MAX
#define  UINT32_MAX			((uint32_t)~0)
#endif
#ifndef UINT16_MAX
#define  UINT64_MAX			((uint64_t)~0)
#endif

#ifndef UINT8_MIN
#define  UINT8_MIN			0
#endif
#ifndef UINT16_MIN
#define  UINT16_MIN			0
#endif
#ifndef UINT32_MIN
#define  UINT32_MIN			0
#endif
#ifndef UINT64_MIN
#define  UINT64_MIN			0
#endif

#endif // !PLATFORM_OSX && !__STDC_LIMIT_MACROS

#ifndef  UINT_MIN
#define  UINT_MIN			UINT32_MIN
#endif

#define  FLOAT32_MAX		FLT_MAX
#define  FLOAT64_MAX		DBL_MAX

#ifdef GNUC
#undef offsetof
//#define offsetof( type, var ) __builtin_offsetof( type, var )
#define offsetof(s,m)	(size_t)&(((s *)0)->m)
#else
#include <stddef.h>
#undef offsetof
#define offsetof(s,m)	(size_t)&(((s *)0)->m)
#endif

#define  FLOAT32_MIN		FLT_MIN
#define  FLOAT64_MIN		DBL_MIN

//-----------------------------------------------------------------------------
// Long is evil because it's treated differently by different compilers
// Preventing its use is nasty however. This #define, which should be
// turned on in individual VPC files, causes you to include tier0/valve_off.h
// before standard C + windows headers, and include tier0/valve_on.h after
// standard C + windows headers. So, there's some painful overhead to disabling long
//-----------------------------------------------------------------------------
#ifdef DISALLOW_USE_OF_LONG
#define long			long_is_the_devil_stop_using_it_use_int32_or_int64
#endif

//-----------------------------------------------------------------------------
// Various compiler-specific keywords
//-----------------------------------------------------------------------------
#ifdef COMPILER_MSVC

#ifdef FORCEINLINE
#undef FORCEINLINE
#endif
#define STDCALL					__stdcall
#ifndef FASTCALL
#define  FASTCALL			__fastcall
#endif
#define FORCEINLINE				__forceinline
#define FORCEINLINE_TEMPLATE	__forceinline
#define NULLTERMINATED			__nullterminated

// This can be used to ensure the size of pointers to members when declaring
// a pointer type for a class that has only been forward declared
#define SINGLE_INHERITANCE		__single_inheritance
#define MULTIPLE_INHERITANCE	__multiple_inheritance
#define EXPLICIT				explicit
#define NO_VTABLE				__declspec( novtable )

// gcc doesn't allow storage specifiers on explicit template instatiation, but visual studio needs them to avoid link errors.
#define TEMPLATE_STATIC			static

// Used for dll exporting and importing
#define DLL_EXPORT				extern "C" __declspec( dllexport )
#define DLL_IMPORT				extern "C" __declspec( dllimport )

// Can't use extern "C" when DLL exporting a class
#define DLL_CLASS_EXPORT		__declspec( dllexport )
#define DLL_CLASS_IMPORT		__declspec( dllimport )

// Can't use extern "C" when DLL exporting a global
#define DLL_GLOBAL_EXPORT		extern __declspec( dllexport )
#define DLL_GLOBAL_IMPORT		extern __declspec( dllimport )

// Pass hints to the compiler to prevent it from generating unnessecary / stupid code
// in certain situations.  Several compilers other than MSVC also have an equivilent
// construct.
//
// Essentially the 'Hint' is that the condition specified is assumed to be true at
// that point in the compilation.  If '0' is passed, then the compiler assumes that
// any subsequent code in the same 'basic block' is unreachable, and thus usually
// removed.
#define HINT(THE_HINT)			__assume((THE_HINT))

// decls for aligning data
#define DECL_ALIGN(x)			__declspec( align( x ) )

// GCC had a few areas where it didn't construct objects in the same order
// that Windows does. So when CVProfile::CVProfile() would access g_pMemAlloc,
// it would crash because the allocator wasn't initalized yet.
#define CONSTRUCT_EARLY

#define SELECTANY				__declspec(selectany)

#define RESTRICT				__restrict
#define RESTRICT_FUNC			__declspec(restrict)
#define FMTFUNCTION( a, b )
#define NOINLINE

#if !defined( NO_THREAD_LOCAL )
#define DECL_THREAD_LOCAL		__declspec(thread)
#endif

#define DISABLE_VC_WARNING( x ) __pragma(warning(disable:4310) )
#define DEFAULT_VC_WARNING( x ) __pragma(warning(default:4310) )

#elif defined ( COMPILER_GCC )

#if (CROSS_PLATFORM_VERSION >= 1) && !defined( PLATFORM_64BITS )
#define  STDCALL			__attribute__ ((__stdcall__))
#else
#define  STDCALL
#define  __stdcall			__attribute__ ((__stdcall__))
#endif

#define  FASTCALL
#ifdef _LINUX_DEBUGGABLE
#define  FORCEINLINE
#else
#define  FORCEINLINE		inline
#endif

// GCC 3.4.1 has a bug in supporting forced inline of templated functions
// this macro lets us not force inlining in that case
#define FORCEINLINE_TEMPLATE	inline
#define SINGLE_INHERITANCE
#define MULTIPLE_INHERITANCE
#define EXPLICIT
#define NO_VTABLE

#define NULLTERMINATED

#define TEMPLATE_STATIC

// Used for dll exporting and importing
#define DLL_EXPORT				extern "C" __attribute__ ((visibility("default")))
#define DLL_IMPORT				extern "C"

// Can't use extern "C" when DLL exporting a class
#define DLL_CLASS_EXPORT		__attribute__ ((visibility("default")))
#define DLL_CLASS_IMPORT

// Can't use extern "C" when DLL exporting a global
#define DLL_GLOBAL_EXPORT		__attribute__((visibility("default")))
#define DLL_GLOBAL_IMPORT		extern

#define HINT(THE_HINT)			0
#define DECL_ALIGN(x)			__attribute__( ( aligned( x ) ) )
#define CONSTRUCT_EARLY			__attribute__((init_priority(101)))
#define SELECTANY				__attribute__((weak))
#define RESTRICT
#define RESTRICT_FUNC
#define FMTFUNCTION( fmtargnumber, firstvarargnumber ) __attribute__ (( format( printf, fmtargnumber, firstvarargnumber )))
#define NOINLINE				__attribute__ ((noinline))

#if !defined( NO_THREAD_LOCAL )
#define DECL_THREAD_LOCAL		__thread
#endif

#define DISABLE_VC_WARNING( x )
#define DEFAULT_VC_WARNING( x )

#else

#define DECL_ALIGN(x)			/* */
#define SELECTANY				static

#endif

#if defined( GNUC )
// gnuc has the align decoration at the end
#define ALIGN4
#define ALIGN8
#define ALIGN16
#define ALIGN32
#define ALIGN128

#define ALIGN4_POST DECL_ALIGN(4)
#define ALIGN8_POST DECL_ALIGN(8)
#define ALIGN16_POST DECL_ALIGN(16)
#define ALIGN32_POST DECL_ALIGN(32)
#define ALIGN128_POST DECL_ALIGN(128)
#else
// MSVC has the align at the start of the struct
#define ALIGN4 DECL_ALIGN(4)
#define ALIGN8 DECL_ALIGN(8)
#define ALIGN16 DECL_ALIGN(16)
#define ALIGN32 DECL_ALIGN(32)
#define ALIGN128 DECL_ALIGN(128)

#define ALIGN4_POST
#define ALIGN8_POST
#define ALIGN16_POST
#define ALIGN32_POST
#define ALIGN128_POST
#endif

// This can be used to declare an abstract (interface only) class.
// Classes marked abstract should not be instantiated.  If they are, and access violation will occur.
//
// Example of use:
//
// abstract_class CFoo
// {
//      ...
// }
//
// MSDN __declspec(novtable) documentation: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_langref_novtable.asp
//
// Note: NJS: This is not enabled for regular PC, due to not knowing the implications of exporting a class with no no vtable.
//       It's probable that this shouldn't be an issue, but an experiment should be done to verify this.
//
#ifndef COMPILER_MSVCX360
#define abstract_class class
#else
#define abstract_class class NO_VTABLE
#endif

//-----------------------------------------------------------------------------
// Why do we need this? It would be nice to make it die die die
//-----------------------------------------------------------------------------
// Alloca defined for this platform
#if defined( COMPILER_MSVC ) && !defined( WINDED )
#if defined(_M_IX86)
#define __i386__	1
#endif
#endif

#if defined __i386__ && !defined __linux__
#define id386	1
#else
#define id386	0
#endif  // __i386__

//-----------------------------------------------------------------------------
// Disable annoying unhelpful warnings
//-----------------------------------------------------------------------------
#ifdef COMPILER_MSVC
// Remove warnings from warning level 4.
#pragma warning(disable : 4514) // warning C4514: 'acosl' : unreferenced inline function has been removed
#pragma warning(disable : 4100) // warning C4100: 'hwnd' : unreferenced formal parameter
#pragma warning(disable : 4127) // warning C4127: conditional expression is constant
#pragma warning(disable : 4512) // warning C4512: 'InFileRIFF' : assignment operator could not be generated
#pragma warning(disable : 4611) // warning C4611: interaction between '_setjmp' and C++ object destruction is non-portable
#pragma warning(disable : 4710) // warning C4710: function 'x' not inlined
#pragma warning(disable : 4702) // warning C4702: unreachable code
#pragma warning(disable : 4505) // unreferenced local function has been removed
#pragma warning(disable : 4239) // nonstandard extension used : 'argument' ( conversion from class Vector to class Vector& )
#pragma warning(disable : 4097) // typedef-name 'BaseClass' used as synonym for class-name 'CFlexCycler::CBaseFlex'
#pragma warning(disable : 4324) // Padding was added at the end of a structure
#pragma warning(disable : 4244) // type conversion warning.
#pragma warning(disable : 4305)	// truncation from 'const double ' to 'float '
#pragma warning(disable : 4786)	// Disable warnings about long symbol names
#pragma warning(disable : 4250) // 'X' : inherits 'Y::Z' via dominance
#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union

#if _MSC_VER >= 1300
#pragma warning(disable : 4511)	// Disable warnings about private copy constructors
#pragma warning(disable : 4121)	// warning C4121: 'symbol' : alignment of a member was sensitive to packing
#pragma warning(disable : 4530)	// warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc (disabled due to std headers having exception syntax)
#endif

#if _MSC_VER >= 1400
#pragma warning(disable : 4996)	// functions declared deprecated
#endif

// When we port to 64 bit, we'll have to resolve the int, ptr vs size_t 32/64 bit problems...
#if !defined( COMPILER_MSVC64 )
#if ( CROSS_PLATFORM_VERSION < 1 )
#pragma warning( disable : 4267 )	// conversion from 'size_t' to 'int', possible loss of data
#pragma warning( disable : 4311 )	// pointer truncation from 'char *' to 'int'
#pragma warning( disable : 4312 )	// conversion from 'unsigned int' to 'memhandle_t' of greater size
#endif
#endif

#endif

//-----------------------------------------------------------------------------
// Stack-based allocation related helpers
//-----------------------------------------------------------------------------
#if defined( COMPILER_GCC )

#define stackalloc( _size )		alloca( ALIGN_VALUE( _size, 16 ) )

#ifdef PLATFORM_OSX
#define mallocsize( _p )	( malloc_size( _p ) )
#else
#define mallocsize( _p )	( malloc_usable_size( _p ) )
#endif

#elif defined ( COMPILER_MSVC )

#define stackalloc( _size )		_alloca( ALIGN_VALUE( _size, 16 ) )
#define mallocsize( _p )		( _msize( _p ) )

#endif

#define  stackfree( _p )			0

//-----------------------------------------------------------------------------
// Used to break into the debugger
//-----------------------------------------------------------------------------
#ifdef COMPILER_MSVC64
#define DebuggerBreak()		__debugbreak()
#elif COMPILER_MSVC32
#define DebuggerBreak()		__asm { int 3 }
#elif COMPILER_MSVCX360
#define DebuggerBreak()		DebugBreak()
#elif COMPILER_GCC
#if defined( PLATFORM_CYGWIN ) || defined( PLATFORM_POSIX )
#define DebuggerBreak()		__asm__( "int $0x3;")
#else
#define DebuggerBreak()		asm( "int3" )
#endif
#endif

//-----------------------------------------------------------------------------
// DLL export for platform utilities
//-----------------------------------------------------------------------------
#ifndef STATIC_TIER0

#ifdef TIER0_DLL_EXPORT
#define PLATFORM_INTERFACE	DLL_EXPORT
#define PLATFORM_OVERLOAD	DLL_GLOBAL_EXPORT
#define PLATFORM_CLASS		DLL_CLASS_EXPORT
#else
#define PLATFORM_INTERFACE	DLL_IMPORT
#define PLATFORM_OVERLOAD	DLL_GLOBAL_IMPORT
#define PLATFORM_CLASS		DLL_CLASS_IMPORT
#endif

#else	// BUILD_AS_DLL

#define PLATFORM_INTERFACE	extern
#define PLATFORM_OVERLOAD
#define PLATFORM_CLASS

#endif	// BUILD_AS_DLL

//-----------------------------------------------------------------------------
// Posix platform helpers
//-----------------------------------------------------------------------------
#ifdef PLATFORM_POSIX

// Visual Studio likes to put an underscore in front of anything that looks like a portable function.
#define _strupr strupr
#define _getcwd getcwd
#define _open open
#define _lseek lseek
#define _read read
#define _close close
#define _vsnprintf vsnprintf
#define _stat stat
#define _O_RDONLY O_RDONLY
#define _stricmp strcasecmp
#define _finite finite
#define _unlink unlink
#define _putenv putenv
#define _chdir chdir
#define _access access
#define _strtoi64 strtoll

#if !defined( _snprintf )	// some vpc's define this on the command line
#define _snprintf snprintf
#endif

#include <alloca.h>
#include <unistd.h>											// Get unlink
#include <errno.h>

#endif // PLATFORM_POSIX

//-----------------------------------------------------------------------------
// Generally useful platform-independent macros (move to another file?)
//-----------------------------------------------------------------------------

// need macro for constant expression
#define ALIGN_VALUE( val, alignment ) ( ( val + alignment - 1 ) & ~( alignment - 1 ) )

// Force a function call site -not- to inlined. (useful for profiling)
#define DONT_INLINE(a) (((int)(a)+1)?(a):(a))

// Marks the codepath from here until the next branch entry point as unreachable,
// and asserts if any attempt is made to execute it.
#define UNREACHABLE() { Assert(0); HINT(0); }

// In cases where no default is present or appropriate, this causes MSVC to generate
// as little code as possible, and throw an assertion in debug.
#define NO_DEFAULT default: UNREACHABLE();

// Defines MAX_PATH
#ifndef MAX_PATH
#define MAX_PATH  260
#endif

//-----------------------------------------------------------------------------
// FP exception handling
//-----------------------------------------------------------------------------
//#define CHECK_FLOAT_EXCEPTIONS		1
//#define CHECK_FPU_CONTROL_WORD_SET	1	// x360 only

#if defined( COMPILER_MSVC64 )

inline void SetupFPUControlWord()
{
}

#elif defined ( COMPILER_MSVC32 )

inline void SetupFPUControlWordForceExceptions()
{
	// use local to Get and store control word
	uint16 tmpCtrlW;
	__asm
	{
		fnclex						/* clear all current exceptions */
		fnstcw word ptr[tmpCtrlW]	/* Get current control word */
		and [tmpCtrlW], 0FCC0h		/* Keep infinity control + rounding control */
		or [tmpCtrlW], 0230h		/* Set to 53-bit, mask only inexact, underflow */
		fldcw word ptr[tmpCtrlW]	/* put new control word in FPU */
	}
}

#ifdef CHECK_FLOAT_EXCEPTIONS

inline void SetupFPUControlWord()
{
	SetupFPUControlWordForceExceptions();
}

#else

inline void SetupFPUControlWord()
{
	// use local to Get and store control word
	uint16 tmpCtrlW;
	__asm
	{
		fnstcw word ptr[tmpCtrlW]	/* Get current control word */
		and [tmpCtrlW], 0FCC0h		/* Keep infinity control + rounding control */
		or [tmpCtrlW], 023Fh		/* Set to 53-bit, mask only inexact, underflow */
		fldcw word ptr[tmpCtrlW]	/* put new control word in FPU */
	}
}

#endif

#elif defined ( COMPILER_GCC )

inline void SetupFPUControlWord()
{
	__volatile unsigned short int __cw;
	__asm __volatile("fnstcw %0" : "=m" (__cw));
	__cw = __cw & 0x0FCC0;	// keep infinity control, keep rounding mode
	__cw = __cw | 0x023F;	// Set 53-bit, no exceptions
	__asm __volatile("fldcw %0" : : "m" (__cw));
}

#elif defined( COMPILER_MSVCX360 )

#ifdef CHECK_FPU_CONTROL_WORD_SET
FORCEINLINE bool IsFPUControlWordSet()
{
	float f = 0.996f;
	union
	{
		double flResult;
		int pResult[2];
	};
	flResult = __fctiw(f);
	return (pResult[1] == 1);
}
#else
#define IsFPUControlWordSet() true
#endif

inline void SetupFPUControlWord()
{
	// Set round-to-nearest in FPSCR
	// (cannot assemble, must use op-code form)
	__emit(0xFF80010C);	// mtfsfi  7,0

// Favour compatibility over speed (make sure the VPU Set to Java-compliant mode)
	// NOTE: the VPU *always* uses round-to-nearest
	__vector4  a = { 0.0f, 0.0f, 0.0f, 0.0f };
	a;				//	Avoid compiler warning
	__asm
	{
		mtvscr a;	// Clear the Vector Status & Control Register to zero
	}
}

#endif // COMPILER_MSVCX360

//-----------------------------------------------------------------------------
// Purpose: Standard functions for handling endian-ness
//-----------------------------------------------------------------------------

//-------------------------------------
// Basic swaps
//-------------------------------------

template < typename T >
inline T WordSwapC(T w)
{
	uint16 temp = ((*((uint16*)& w) & 0xff00) >> 8);
	temp |= ((*((uint16*)& w) & 0x00ff) << 8);

	return *reinterpret_cast<T*> (&temp);
}

template < typename T >
inline T DWordSwapC(T dw)
{
	uint32_t temp = *((uint32_t*)& dw) >> 24;
	temp |= ((*((uint32_t*)& dw) & 0x00FF0000) >> 8);
	temp |= ((*((uint32_t*)& dw) & 0x0000FF00) << 8);
	temp |= ((*((uint32_t*)& dw) & 0x000000FF) << 24);

	return *((T*)& temp);
}

//-------------------------------------
// Fast swaps
//-------------------------------------

#if defined( COMPILER_MSVCX360 )

#define WordSwap  WordSwap360Intr
#define DWordSwap DWordSwap360Intr

template <typename T>
inline T WordSwap360Intr(T w)
{
	T output;
	__storeshortbytereverse(w, 0, &output);
	return output;
}

template <typename T>
inline T DWordSwap360Intr(T dw)
{
	T output;
	__storewordbytereverse(dw, 0, &output);
	return output;
}

#elif defined( COMPILER_MSVC32 )

#define WordSwap  WordSwapAsm
#define DWordSwap DWordSwapAsm

#pragma warning(push)
#pragma warning (disable:4035) // no return value

template <typename T>
inline T WordSwapAsm(T w)
{
	__asm
	{
		mov ax, w
		xchg al, ah
	}
}

template <typename T>
inline T DWordSwapAsm(T dw)
{
	__asm
	{
		mov eax, dw
		bswap eax
	}
}

#pragma warning(pop)

#else

#define WordSwap  WordSwapC
#define DWordSwap DWordSwapC

#endif

//-------------------------------------
// The typically used methods.
//-------------------------------------

#if defined( _SGI_SOURCE ) || defined( PLATFORM_X360 )
#define	PLAT_BIG_ENDIAN 1
#else
#define PLAT_LITTLE_ENDIAN 1
#endif

// If a swapped float passes through the fpu, the bytes may Get changed.
// Prevent this by swapping floats as DWORDs.
#define SafeSwapFloat( pOut, pIn )	(*((unsigned int*)pOut) = DWordSwap( *((unsigned int*)pIn) ))

#if defined(PLAT_LITTLE_ENDIAN)
#define BigShort( val )				WordSwap( val )
#define BigWord( val )				WordSwap( val )
#define BigLong( val )				DWordSwap( val )
#define BigDWord( val )				DWordSwap( val )
#define LittleShort( val )			( val )
#define LittleWord( val )			( val )
#define LittleLong( val )			( val )
#define LittleDWord( val )			( val )
#define SwapShort( val )			BigShort( val )
#define SwapWord( val )				BigWord( val )
#define SwapLong( val )				BigLong( val )
#define SwapDWord( val )			BigDWord( val )

// Pass floats by pointer for swapping to avoid truncation in the fpu
#define BigFloat( pOut, pIn )		SafeSwapFloat( pOut, pIn )
#define LittleFloat( pOut, pIn )	( *pOut = *pIn )
#define SwapFloat( pOut, pIn )		BigFloat( pOut, pIn )

#elif defined(PLAT_BIG_ENDIAN)

#define BigShort( val )				( val )
#define BigWord( val )				( val )
#define BigLong( val )				( val )
#define BigDWord( val )				( val )
#define LittleShort( val )			WordSwap( val )
#define LittleWord( val )			WordSwap( val )
#define LittleLong( val )			DWordSwap( val )
#define LittleDWord( val )			DWordSwap( val )
#define SwapShort( val )			LittleShort( val )
#define SwapWord( val )				LittleWord( val )
#define SwapLong( val )				LittleLong( val )
#define SwapDWord( val )			LittleDWord( val )

// Pass floats by pointer for swapping to avoid truncation in the fpu
#define BigFloat( pOut, pIn )		( *pOut = *pIn )
#define LittleFloat( pOut, pIn )	SafeSwapFloat( pOut, pIn )
#define SwapFloat( pOut, pIn )		LittleFloat( pOut, pIn )

#else

// @Note (toml 05-02-02): this technique expects the compiler to
// optimize the expression and eliminate the other path. On any new
// platform/compiler this should be tested.
inline short BigShort(short val)
{
	int test = 1;
	return (*(char*)& test == 1) ? WordSwap(val) : val;
}
inline uint16 BigWord(uint16 val)
{
	int test = 1;
	return (*(char*)& test == 1) ? WordSwap(val) : val;
}
inline long BigLong(long val)
{
	int test = 1;
	return (*(char*)& test == 1) ? DWordSwap(val) : val;
}
inline uint32_t BigDWord(uint32_t val)
{
	int test = 1;
	return (*(char*)& test == 1) ? DWordSwap(val) : val;
}
inline short LittleShort(short val)
{
	int test = 1;
	return (*(char*)& test == 1) ? val : WordSwap(val);
}
inline uint16 LittleWord(uint16 val)
{
	int test = 1;
	return (*(char*)& test == 1) ? val : WordSwap(val);
}
inline long LittleLong(long val)
{
	int test = 1;
	return (*(char*)& test == 1) ? val : DWordSwap(val);
}
inline uint32_t LittleDWord(uint32_t val)
{
	int test = 1;
	return (*(char*)& test == 1) ? val : DWordSwap(val);
}
inline short SwapShort(short val)
{
	return WordSwap(val);
}
inline uint16 SwapWord(uint16 val)
{
	return WordSwap(val);
}
inline long SwapLong(long val)
{
	return DWordSwap(val);
}
inline uint32_t SwapDWord(uint32_t val)
{
	return DWordSwap(val);
}

// Pass floats by pointer for swapping to avoid truncation in the fpu
inline void BigFloat(float* pOut, const float* pIn)
{
	int test = 1;
	(*(char*)& test == 1) ? SafeSwapFloat(pOut, pIn) : (*pOut = *pIn);
}
inline void LittleFloat(float* pOut, const float* pIn)
{
	int test = 1;
	(*(char*)& test == 1) ? (*pOut = *pIn) : SafeSwapFloat(pOut, pIn);
}
inline void SwapFloat(float* pOut, const float* pIn)
{
	SafeSwapFloat(pOut, pIn);
}

#endif

inline uint32_t LoadLittleDWord(uint32_t* base, unsigned int dwordIndex)
{
	return LittleDWord(base[dwordIndex]);
}

inline void StoreLittleDWord(uint32_t* base, unsigned int dwordIndex, uint32_t dword)
{
	base[dwordIndex] = LittleDWord(dword);
}

// Protect against bad auto operator=
#define DISALLOW_OPERATOR_EQUAL( _classname )			\
	private:											\
		_classname &operator=( const _classname & );	\
	public:

// Define a reasonable operator=
#define IMPLEMENT_OPERATOR_EQUAL( _classname )			\
	public:												\
		_classname &operator=( const _classname &src )	\
		{												\
			memcpy( this, &src, sizeof(_classname) );	\
			return *this;								\
		}

#if _X360
#define Plat_FastMemset XMemSet
#define Plat_FastMemcpy XMemCpy
#else
#define Plat_FastMemset memset
#define Plat_FastMemcpy memcpy
#endif

//-----------------------------------------------------------------------------
// XBOX Components valid in PC compilation space
//-----------------------------------------------------------------------------

#define XBOX_DVD_SECTORSIZE			2048
#define XBOX_DVD_ECC_SIZE			32768 // driver reads in quantum ECC blocks
#define XBOX_HDD_SECTORSIZE			512

// Custom windows messages for Xbox input
#define WM_XREMOTECOMMAND					(WM_USER + 100)
#define WM_XCONTROLLER_KEY					(WM_USER + 101)
#define WM_SYS_UI							(WM_USER + 102)
#define WM_SYS_SIGNINCHANGED				(WM_USER + 103)
#define WM_SYS_STORAGEDEVICESCHANGED		(WM_USER + 104)
#define WM_SYS_PROFILESETTINGCHANGED		(WM_USER + 105)
#define WM_SYS_MUTELISTCHANGED				(WM_USER + 106)
#define WM_SYS_INPUTDEVICESCHANGED			(WM_USER + 107)
#define WM_SYS_INPUTDEVICECONFIGCHANGED		(WM_USER + 108)
#define WM_LIVE_CONNECTIONCHANGED			(WM_USER + 109)
#define WM_LIVE_INVITE_ACCEPTED				(WM_USER + 110)
#define WM_LIVE_LINK_STATE_CHANGED			(WM_USER + 111)
#define WM_LIVE_CONTENT_INSTALLED			(WM_USER + 112)
#define WM_LIVE_MEMBERSHIP_PURCHASED		(WM_USER + 113)
#define WM_LIVE_VOICECHAT_AWAY				(WM_USER + 114)
#define WM_LIVE_PRESENCE_CHANGED			(WM_USER + 115)
#define WM_FRIENDS_PRESENCE_CHANGED			(WM_USER + 116)
#define WM_FRIENDS_FRIEND_ADDED				(WM_USER + 117)
#define WM_FRIENDS_FRIEND_REMOVED			(WM_USER + 118)
#define WM_CUSTOM_GAMEBANNERPRESSED			(WM_USER + 119)
#define WM_CUSTOM_ACTIONPRESSED				(WM_USER + 120)
#define WM_XMP_STATECHANGED					(WM_USER + 121)
#define WM_XMP_PLAYBACKBEHAVIORCHANGED		(WM_USER + 122)
#define WM_XMP_PLAYBACKCONTROLLERCHANGED	(WM_USER + 123)

inline const char* GetPlatformExt(void)
{
	return IsPlatformX360() ? ".360" : "";
}

// flat view, 6 hw threads
#define XBOX_PROCESSOR_0			( 1<<0 )
#define XBOX_PROCESSOR_1			( 1<<1 )
#define XBOX_PROCESSOR_2			( 1<<2 )
#define XBOX_PROCESSOR_3			( 1<<3 )
#define XBOX_PROCESSOR_4			( 1<<4 )
#define XBOX_PROCESSOR_5			( 1<<5 )

// core view, 3 cores with 2 hw threads each
#define XBOX_CORE_0_HWTHREAD_0		XBOX_PROCESSOR_0
#define XBOX_CORE_0_HWTHREAD_1		XBOX_PROCESSOR_1
#define XBOX_CORE_1_HWTHREAD_0		XBOX_PROCESSOR_2
#define XBOX_CORE_1_HWTHREAD_1		XBOX_PROCESSOR_3
#define XBOX_CORE_2_HWTHREAD_0		XBOX_PROCESSOR_4
#define XBOX_CORE_2_HWTHREAD_1		XBOX_PROCESSOR_5

//-----------------------------------------------------------------------------
// Include additional dependant header components.
//-----------------------------------------------------------------------------
#if defined( PLATFORM_X360 )
#include "xbox/xbox_core.h"
#endif

//-----------------------------------------------------------------------------
// Methods to invoke the constructor, copy constructor, and destructor
//-----------------------------------------------------------------------------

template < class T >
inline T* Construct(T* pMemory)
{
	return ::new (pMemory) T;
}

template < class T, typename ARG1 >
inline T* Construct(T* pMemory, ARG1 a1)
{
	return ::new (pMemory) T(a1);
}

template < class T, typename ARG1, typename ARG2 >
inline T* Construct(T* pMemory, ARG1 a1, ARG2 a2)
{
	return ::new (pMemory) T(a1, a2);
}

template < class T, typename ARG1, typename ARG2, typename ARG3 >
inline T* Construct(T* pMemory, ARG1 a1, ARG2 a2, ARG3 a3)
{
	return ::new (pMemory) T(a1, a2, a3);
}

template < class T, typename ARG1, typename ARG2, typename ARG3, typename ARG4 >
inline T* Construct(T* pMemory, ARG1 a1, ARG2 a2, ARG3 a3, ARG4 a4)
{
	return ::new (pMemory) T(a1, a2, a3, a4);
}

template < class T, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5 >
inline T* Construct(T* pMemory, ARG1 a1, ARG2 a2, ARG3 a3, ARG4 a4, ARG5 a5)
{
	return ::new (pMemory) T(a1, a2, a3, a4, a5);
}

template < class T >
inline T* CopyConstruct(T* pMemory, T const& src)
{
	return ::new (pMemory) T(src);
}

template < class T >
inline void Destruct(T* pMemory)
{
	pMemory->~T();

#ifdef _DEBUG
	memset(pMemory, 0xDD, sizeof(T));
#endif
}

//
// GET_OUTER()
//
// A platform-independent way for a contained class to Get a pointer to its
// owner. If you know a class is exclusively used in the context of some
// "outer" class, this is a much more space efficient way to Get at the outer
// class than having the inner class store a pointer to it.
//
//	class COuter
//	{
//		class CInner // Note: this does not need to be a nested class to work
//		{
//			void PrintAddressOfOuter()
//			{
//				printf( "Outer is at 0x%x\n", GET_OUTER( COuter, m_Inner ) );
//			}
//		};
//
//		CInner m_Inner;
//		friend class CInner;
//	};

#define GET_OUTER( OuterType, OuterMember ) \
   ( ( OuterType * ) ( (uint8_t *)this - offsetof( OuterType, OuterMember ) ) )

/*	TEMPLATE_FUNCTION_TABLE()

(Note added to platform.h so platforms that correctly support templated
functions can handle portions as templated functions rather than wrapped
functions)

Helps automate the process of creating an array of function
templates that are all specialized by a single integer.
This sort of thing is often useful in optimization work.

For example, using TEMPLATE_FUNCTION_TABLE, this:

TEMPLATE_FUNCTION_TABLE(int, Function, ( int blah, int blah ), 10)
{
return argument * argument;
}

is equivilent to the following:

(NOTE: the function has to be wrapped in a class due to code
generation bugs involved with directly specializing a function
based on a constant.)

template<int argument>
class FunctionWrapper
{
public:
int Function( int blah, int blah )
{
return argument*argument;
}
}

typedef int (*FunctionType)( int blah, int blah );

class FunctionName
{
public:
enum { count = 10 };
FunctionType functions[10];
};

FunctionType FunctionName::functions[] =
{
FunctionWrapper<0>::Function,
FunctionWrapper<1>::Function,
FunctionWrapper<2>::Function,
FunctionWrapper<3>::Function,
FunctionWrapper<4>::Function,
FunctionWrapper<5>::Function,
FunctionWrapper<6>::Function,
FunctionWrapper<7>::Function,
FunctionWrapper<8>::Function,
FunctionWrapper<9>::Function
};
*/

PLATFORM_INTERFACE bool vtune(bool resume);

#define TEMPLATE_FUNCTION_TABLE(RETURN_TYPE, NAME, ARGS, COUNT)			  \
                                                                      \
typedef RETURN_TYPE (FASTCALL *__Type_##NAME) ARGS;						        \
																		                                  \
template<const int nArgument>											                    \
struct __Function_##NAME												                      \
{																		                                  \
	static RETURN_TYPE FASTCALL Run ARGS;								                \
};																		                                \
																		                                  \
template <const int i>														                    \
struct __MetaLooper_##NAME : __MetaLooper_##NAME<i-1>					        \
{																		                                  \
	__Type_##NAME func;													                        \
	inline __MetaLooper_##NAME() { func = __Function_##NAME<i>::Run; }	\
};																		                                \
																		                                  \
template<>																                            \
struct __MetaLooper_##NAME<0>											                    \
{																		                                  \
	__Type_##NAME func;													                        \
	inline __MetaLooper_##NAME() { func = __Function_##NAME<0>::Run; }	\
};																		                                \
																		                                  \
class NAME																                            \
{																		                                  \
private:																                              \
    static const __MetaLooper_##NAME<COUNT> m;							          \
public:																	                              \
	enum { count = COUNT };												                      \
	static const __Type_##NAME* functions;								              \
};																		                                \
const __MetaLooper_##NAME<COUNT> NAME::m;								              \
const __Type_##NAME* NAME::functions = (__Type_##NAME*)&m;				    \
template<const int nArgument>													                \
RETURN_TYPE FASTCALL __Function_##NAME<nArgument>::Run ARGS

#define LOOP_INTERCHANGE(BOOLEAN, CODE)\
	if( (BOOLEAN) )\
	{\
		CODE;\
	} else\
	{\
		CODE;\
	}

#ifdef COMPILER_MSVC
FORCEINLINE uint32_t RotateBitsLeft32(uint32_t nValue, int nRotateBits)
{
	return _rotl(nValue, nRotateBits);
}

FORCEINLINE uint64_t RotateBitsLeft64(uint64_t nValue, int nRotateBits)
{
	return _rotl64(nValue, nRotateBits);
}

FORCEINLINE uint32_t RotateBitsRight32(uint32_t nValue, int nRotateBits)
{
	return _rotr(nValue, nRotateBits);
}

FORCEINLINE uint64_t RotateBitsRight64(uint64_t nValue, int nRotateBits)
{
	return _rotr64(nValue, nRotateBits);
}
#endif
