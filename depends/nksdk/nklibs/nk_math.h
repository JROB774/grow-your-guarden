#ifndef NK_MATH_H__ /*////////////////////////////////////////////////////////*/
#define NK_MATH_H__

#include "nk_define.h"

#include <math.h>

#if !defined(NK_HAS_CPP)
#error nk_math requires C++ in order to be used
#endif

NKGLOBAL NKCONSTEXPR nkF32 NK_PI_F32  = 3.141592653590f;
NKGLOBAL NKCONSTEXPR nkF32 NK_TAU_F32 = 6.283185307180f;
NKGLOBAL NKCONSTEXPR nkF64 NK_PI_F64  = 3.14159265358979323846;
NKGLOBAL NKCONSTEXPR nkF64 NK_TAU_F64 = 6.28318530717958647693;

// nkVec2 ======================================================================
union nkVec2
{
    struct { nkF32 x, y; };
    struct { nkF32 r, g; };
    struct { nkF32 s, t; };

    nkF32 raw[2];

    nkVec2& operator+=(const nkVec2& rhs) { x += rhs.x, y += rhs.y; return *this; }
    nkVec2& operator-=(const nkVec2& rhs) { x -= rhs.x, y -= rhs.y; return *this; }
    nkVec2& operator/=(const nkVec2& rhs) { x /= rhs.x, y /= rhs.y; return *this; }
    nkVec2& operator*=(const nkVec2& rhs) { x *= rhs.x, y *= rhs.y; return *this; }
    nkVec2& operator+=(const nkF32&  rhs) { x += rhs,   y += rhs;   return *this; }
    nkVec2& operator-=(const nkF32&  rhs) { x -= rhs,   y -= rhs;   return *this; }
    nkVec2& operator/=(const nkF32&  rhs) { x /= rhs,   y /= rhs;   return *this; }
    nkVec2& operator*=(const nkF32&  rhs) { x *= rhs,   y *= rhs;   return *this; }

    const nkF32& operator[](size_t idx) const { return raw[idx]; }
          nkF32& operator[](size_t idx)       { return raw[idx]; }
};

NKAPI NKFORCEINLINE nkVec2 operator+ (nkVec2        a, const nkVec2& b) { a += b; return a; }
NKAPI NKFORCEINLINE nkVec2 operator- (nkVec2        a, const nkVec2& b) { a -= b; return a; }
NKAPI NKFORCEINLINE nkVec2 operator/ (nkVec2        a, const nkVec2& b) { a /= b; return a; }
NKAPI NKFORCEINLINE nkVec2 operator* (nkVec2        a, const nkVec2& b) { a *= b; return a; }
NKAPI NKFORCEINLINE nkVec2 operator+ (nkVec2        a, const nkF32&  b) { a += b; return a; }
NKAPI NKFORCEINLINE nkVec2 operator- (nkVec2        a, const nkF32&  b) { a -= b; return a; }
NKAPI NKFORCEINLINE nkVec2 operator/ (nkVec2        a, const nkF32&  b) { a /= b; return a; }
NKAPI NKFORCEINLINE nkVec2 operator* (nkVec2        a, const nkF32&  b) { a *= b; return a; }
NKAPI NKFORCEINLINE nkVec2 operator+ (nkF32         a, const nkVec2& b) { nkVec2 v = { a,a }; v += b; return v; }
NKAPI NKFORCEINLINE nkVec2 operator- (nkF32         a, const nkVec2& b) { nkVec2 v = { a,a }; v -= b; return v; }
NKAPI NKFORCEINLINE nkVec2 operator/ (nkF32         a, const nkVec2& b) { nkVec2 v = { a,a }; v /= b; return v; }
NKAPI NKFORCEINLINE nkVec2 operator* (nkF32         a, const nkVec2& b) { nkVec2 v = { a,a }; v *= b; return v; }
NKAPI NKFORCEINLINE nkBool operator==(const nkVec2& a, const nkVec2& b) { return (a.x == b.x && a.y == b.y); }
NKAPI NKFORCEINLINE nkBool operator!=(const nkVec2& a, const nkVec2& b) { return !(a == b); }

NKGLOBAL NKCONSTEXPR nkVec2 NK_V2_ZERO   = {  0, 0 };
NKGLOBAL NKCONSTEXPR nkVec2 NK_V2_ONE    = {  1, 1 };
NKGLOBAL NKCONSTEXPR nkVec2 NK_V2_UNIT_X = {  1, 0 };
NKGLOBAL NKCONSTEXPR nkVec2 NK_V2_UNIT_Y = {  0, 1 };
NKGLOBAL NKCONSTEXPR nkVec2 NK_V2_POS_X  = {  1, 0 };
NKGLOBAL NKCONSTEXPR nkVec2 NK_V2_POS_Y  = {  0, 1 };
NKGLOBAL NKCONSTEXPR nkVec2 NK_V2_NEG_X  = { -1, 0 };
NKGLOBAL NKCONSTEXPR nkVec2 NK_V2_NEG_Y  = {  0,-1 };
// =============================================================================

// nkVec3 ======================================================================
union nkVec3
{
    struct { nkF32 x, y, z; };
    struct { nkF32 r, g, b; };
    struct { nkF32 s, t, p; };

    nkF32 raw[3];

    nkVec3& operator+=(const nkVec3& rhs) { x += rhs.x, y += rhs.y, z += rhs.z; return *this; }
    nkVec3& operator-=(const nkVec3& rhs) { x -= rhs.x, y -= rhs.y, z -= rhs.z; return *this; }
    nkVec3& operator/=(const nkVec3& rhs) { x /= rhs.x, y /= rhs.y, z /= rhs.z; return *this; }
    nkVec3& operator*=(const nkVec3& rhs) { x *= rhs.x, y *= rhs.y, z *= rhs.z; return *this; }
    nkVec3& operator+=(const nkF32&  rhs) { x += rhs,   y += rhs,   z += rhs;   return *this; }
    nkVec3& operator-=(const nkF32&  rhs) { x -= rhs,   y -= rhs,   z -= rhs;   return *this; }
    nkVec3& operator/=(const nkF32&  rhs) { x /= rhs,   y /= rhs,   z /= rhs;   return *this; }
    nkVec3& operator*=(const nkF32&  rhs) { x *= rhs,   y *= rhs,   z *= rhs;   return *this; }

    const nkF32& operator[](size_t idx) const { return raw[idx]; }
          nkF32& operator[](size_t idx)       { return raw[idx]; }
};

NKAPI NKFORCEINLINE nkVec3 operator+ (nkVec3        a, const nkVec3& b) { a += b; return a; }
NKAPI NKFORCEINLINE nkVec3 operator- (nkVec3        a, const nkVec3& b) { a -= b; return a; }
NKAPI NKFORCEINLINE nkVec3 operator/ (nkVec3        a, const nkVec3& b) { a /= b; return a; }
NKAPI NKFORCEINLINE nkVec3 operator* (nkVec3        a, const nkVec3& b) { a *= b; return a; }
NKAPI NKFORCEINLINE nkVec3 operator+ (nkVec3        a, const nkF32&  b) { a += b; return a; }
NKAPI NKFORCEINLINE nkVec3 operator- (nkVec3        a, const nkF32&  b) { a -= b; return a; }
NKAPI NKFORCEINLINE nkVec3 operator/ (nkVec3        a, const nkF32&  b) { a /= b; return a; }
NKAPI NKFORCEINLINE nkVec3 operator* (nkVec3        a, const nkF32&  b) { a *= b; return a; }
NKAPI NKFORCEINLINE nkVec3 operator+ (nkF32         a, const nkVec3& b) { nkVec3 v = { a,a,a }; v += b; return v; }
NKAPI NKFORCEINLINE nkVec3 operator- (nkF32         a, const nkVec3& b) { nkVec3 v = { a,a,a }; v -= b; return v; }
NKAPI NKFORCEINLINE nkVec3 operator/ (nkF32         a, const nkVec3& b) { nkVec3 v = { a,a,a }; v /= b; return v; }
NKAPI NKFORCEINLINE nkVec3 operator* (nkF32         a, const nkVec3& b) { nkVec3 v = { a,a,a }; v *= b; return v; }
NKAPI NKFORCEINLINE nkBool operator==(const nkVec3& a, const nkVec3& b) { return (a.x == b.x && a.y == b.y && a.z == b.z); }
NKAPI NKFORCEINLINE nkBool operator!=(const nkVec3& a, const nkVec3& b) { return !(a == b); }

NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_ZERO    = {  0, 0, 0 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_ONE     = {  1, 1, 1 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_UNIT_X  = {  1, 0, 0 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_UNIT_Y  = {  0, 1, 0 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_UNIT_Z  = {  0, 0, 1 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_POS_X   = {  1, 0, 0 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_POS_Y   = {  0, 1, 0 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_POS_Z   = {  0, 0, 1 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_NEG_X   = { -1, 0, 0 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_NEG_Y   = {  0,-1, 0 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_NEG_Z   = {  0, 0,-1 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_BLACK   = {  0, 0, 0 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_WHITE   = {  1, 1, 1 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_RED     = {  1, 0, 0 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_GREEN   = {  0, 1, 0 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_BLUE    = {  0, 0, 1 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_CYAN    = {  0, 1, 1 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_MAGENTA = {  1, 0, 1 };
NKGLOBAL NKCONSTEXPR nkVec3 NK_V3_YELLOW  = {  1, 1, 0 };
// =============================================================================

// nkVec4 ======================================================================
union nkVec4
{
    struct { nkF32 x, y, z, w; };
    struct { nkF32 r, g, b, a; };
    struct { nkF32 s, t, p, q; };

    nkF32 raw[4];

    nkVec4& operator+=(const nkVec4& rhs) { x += rhs.x, y += rhs.y, z += rhs.z, w += rhs.w; return *this; }
    nkVec4& operator-=(const nkVec4& rhs) { x -= rhs.x, y -= rhs.y, z -= rhs.z, w -= rhs.w; return *this; }
    nkVec4& operator/=(const nkVec4& rhs) { x /= rhs.x, y /= rhs.y, z /= rhs.z, w /= rhs.w; return *this; }
    nkVec4& operator*=(const nkVec4& rhs) { x *= rhs.x, y *= rhs.y, z *= rhs.z, w *= rhs.w; return *this; }
    nkVec4& operator+=(const nkF32&  rhs) { x += rhs,   y += rhs,   z += rhs,   w += rhs;   return *this; }
    nkVec4& operator-=(const nkF32&  rhs) { x -= rhs,   y -= rhs,   z -= rhs,   w -= rhs;   return *this; }
    nkVec4& operator/=(const nkF32&  rhs) { x /= rhs,   y /= rhs,   z /= rhs,   w /= rhs;   return *this; }
    nkVec4& operator*=(const nkF32&  rhs) { x *= rhs,   y *= rhs,   z *= rhs,   w *= rhs;   return *this; }

    const nkF32& operator[](size_t idx) const { return raw[idx]; }
          nkF32& operator[](size_t idx)       { return raw[idx]; }
};

NKAPI NKFORCEINLINE nkVec4 operator+ (nkVec4        a, const nkVec4& b) { a += b; return a; }
NKAPI NKFORCEINLINE nkVec4 operator- (nkVec4        a, const nkVec4& b) { a -= b; return a; }
NKAPI NKFORCEINLINE nkVec4 operator/ (nkVec4        a, const nkVec4& b) { a /= b; return a; }
NKAPI NKFORCEINLINE nkVec4 operator* (nkVec4        a, const nkVec4& b) { a *= b; return a; }
NKAPI NKFORCEINLINE nkVec4 operator+ (nkVec4        a, const nkF32&  b) { a += b; return a; }
NKAPI NKFORCEINLINE nkVec4 operator- (nkVec4        a, const nkF32&  b) { a -= b; return a; }
NKAPI NKFORCEINLINE nkVec4 operator/ (nkVec4        a, const nkF32&  b) { a /= b; return a; }
NKAPI NKFORCEINLINE nkVec4 operator* (nkVec4        a, const nkF32&  b) { a *= b; return a; }
NKAPI NKFORCEINLINE nkVec4 operator+ (nkF32         a, const nkVec4& b) { nkVec4 v = { a,a,a,a }; v += b; return v; }
NKAPI NKFORCEINLINE nkVec4 operator- (nkF32         a, const nkVec4& b) { nkVec4 v = { a,a,a,a }; v -= b; return v; }
NKAPI NKFORCEINLINE nkVec4 operator/ (nkF32         a, const nkVec4& b) { nkVec4 v = { a,a,a,a }; v /= b; return v; }
NKAPI NKFORCEINLINE nkVec4 operator* (nkF32         a, const nkVec4& b) { nkVec4 v = { a,a,a,a }; v *= b; return v; }
NKAPI NKFORCEINLINE nkBool operator==(const nkVec4& a, const nkVec4& b) { return (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w); }
NKAPI NKFORCEINLINE nkBool operator!=(const nkVec4& a, const nkVec4& b) { return !(a == b); }

NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_ZERO    = {  0, 0, 0, 0 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_ONE     = {  1, 1, 1, 1 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_UNIT_X  = {  1, 0, 0, 0 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_UNIT_Y  = {  0, 1, 0, 0 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_UNIT_Z  = {  0, 0, 1, 0 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_UNIT_W  = {  0, 0, 0, 1 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_POS_X   = {  1, 0, 0, 0 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_POS_Y   = {  0, 1, 0, 0 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_POS_Z   = {  0, 0, 1, 0 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_POS_W   = {  0, 0, 0, 1 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_NEG_X   = { -1, 0, 0, 0 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_NEG_Y   = {  0,-1, 0, 0 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_NEG_Z   = {  0, 0,-1, 0 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_NEG_W   = {  0, 0, 0,-1 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_BLACK   = {  0, 0, 0, 1 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_WHITE   = {  1, 1, 1, 1 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_RED     = {  1, 0, 0, 1 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_GREEN   = {  0, 1, 0, 1 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_BLUE    = {  0, 0, 1, 1 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_CYAN    = {  0, 1, 1, 1 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_MAGENTA = {  1, 0, 1, 1 };
NKGLOBAL NKCONSTEXPR nkVec4 NK_V4_YELLOW  = {  1, 1, 0, 1 };
// =============================================================================

// nkMat2 ======================================================================
union nkMat2
{
    struct
    {
        nkF32 x00,x01;
        nkF32 x10,x11;
    };

    nkF32 x[2][2];
    nkF32 raw[2*2];

    nkMat2& operator+=(const nkF32&  rhs) { for(nkS32 c=0; c<2; ++c) for(nkS32 r=0; r<2; ++r) x[c][r] += rhs;         return *this; }
    nkMat2& operator-=(const nkF32&  rhs) { for(nkS32 c=0; c<2; ++c) for(nkS32 r=0; r<2; ++r) x[c][r] -= rhs;         return *this; }
    nkMat2& operator/=(const nkF32&  rhs) { for(nkS32 c=0; c<2; ++c) for(nkS32 r=0; r<2; ++r) x[c][r] /= rhs;         return *this; }
    nkMat2& operator*=(const nkF32&  rhs) { for(nkS32 c=0; c<2; ++c) for(nkS32 r=0; r<2; ++r) x[c][r] *= rhs;         return *this; }

    nkMat2& operator+=(const nkMat2& rhs) { for(nkS32 c=0; c<2; ++c) for(nkS32 r=0; r<2; ++r) x[c][r] += rhs.x[c][r]; return *this; }
    nkMat2& operator-=(const nkMat2& rhs) { for(nkS32 c=0; c<2; ++c) for(nkS32 r=0; r<2; ++r) x[c][r] -= rhs.x[c][r]; return *this; }
    nkMat2& operator/=(const nkMat2& rhs) { for(nkS32 c=0; c<2; ++c) for(nkS32 r=0; r<2; ++r) x[c][r] /= rhs.x[c][r]; return *this; }

    nkMat2& operator*=(const nkMat2& rhs)
    {
        nkMat2 m = NK_ZERO_MEM;
        for(nkS32 r=0; r<2; ++r)
            for(nkS32 c1=0; c1<2; ++c1)
                for(nkS32 c0=0; c0<2; ++c0)
                    m.x[c1][r] += x[c0][r] * rhs.x[c1][c0];
        *this = m;
        return *this;
    }
};

NKAPI NKFORCEINLINE nkMat2 nk_m2_identity(void)
{
    nkMat2 m = NK_ZERO_MEM;
    m.x00 = 1;
    m.x11 = 1;
    return m;
}

NKAPI NKFORCEINLINE nkMat2 operator+(nkMat2 a, const nkF32&  b) { a += b; return a; }
NKAPI NKFORCEINLINE nkMat2 operator-(nkMat2 a, const nkF32&  b) { a -= b; return a; }
NKAPI NKFORCEINLINE nkMat2 operator/(nkMat2 a, const nkF32&  b) { a /= b; return a; }
NKAPI NKFORCEINLINE nkMat2 operator*(nkMat2 a, const nkF32&  b) { a *= b; return a; }
NKAPI NKFORCEINLINE nkMat2 operator+(nkMat2 a, const nkMat2& b) { a += b; return a; }
NKAPI NKFORCEINLINE nkMat2 operator-(nkMat2 a, const nkMat2& b) { a -= b; return a; }
NKAPI NKFORCEINLINE nkMat2 operator/(nkMat2 a, const nkMat2& b) { a /= b; return a; }
NKAPI NKFORCEINLINE nkMat2 operator*(nkMat2 a, const nkMat2& b) { a *= b; return a; }

NKAPI NKFORCEINLINE nkVec2 operator*(nkMat2 a, const nkVec2& b)
{
    nkVec2 v = NK_ZERO_MEM;
    for(nkS32 r=0; r<2; ++r)
        for(nkS32 c=0; c<2; ++c)
            v.raw[r] += a.x[c][r] * b.raw[c];
    return v;
}

NKAPI NKFORCEINLINE nkBool operator==(const nkMat2& a, const nkMat2& b)
{
    for(nkS32 r=0; r<2; ++r)
        for(nkS32 c=0; c<2; ++c)
            if(a.x[c][r] != b.x[c][r])
                return NK_FALSE;
    return NK_TRUE;
}
NKAPI NKFORCEINLINE nkBool operator!=(const nkMat2& a, const nkMat2& b)
{
    return !(a == b);
}
// =============================================================================

// nkMat3 ======================================================================
union nkMat3
{
    struct
    {
        nkF32 x00,x01,x02;
        nkF32 x10,x11,x12;
        nkF32 x20,x21,x22;
    };

    nkF32 x[3][3];
    nkF32 raw[3*3];

    nkMat3& operator+=(const nkF32&  rhs) { for(nkS32 c=0; c<3; ++c) for(nkS32 r=0; r<3; ++r) x[c][r] += rhs;         return *this; }
    nkMat3& operator-=(const nkF32&  rhs) { for(nkS32 c=0; c<3; ++c) for(nkS32 r=0; r<3; ++r) x[c][r] -= rhs;         return *this; }
    nkMat3& operator/=(const nkF32&  rhs) { for(nkS32 c=0; c<3; ++c) for(nkS32 r=0; r<3; ++r) x[c][r] /= rhs;         return *this; }
    nkMat3& operator*=(const nkF32&  rhs) { for(nkS32 c=0; c<3; ++c) for(nkS32 r=0; r<3; ++r) x[c][r] *= rhs;         return *this; }

    nkMat3& operator+=(const nkMat3& rhs) { for(nkS32 c=0; c<3; ++c) for(nkS32 r=0; r<3; ++r) x[c][r] += rhs.x[c][r]; return *this; }
    nkMat3& operator-=(const nkMat3& rhs) { for(nkS32 c=0; c<3; ++c) for(nkS32 r=0; r<3; ++r) x[c][r] -= rhs.x[c][r]; return *this; }
    nkMat3& operator/=(const nkMat3& rhs) { for(nkS32 c=0; c<3; ++c) for(nkS32 r=0; r<3; ++r) x[c][r] /= rhs.x[c][r]; return *this; }

    nkMat3& operator*=(const nkMat3& rhs)
    {
        nkMat3 m = NK_ZERO_MEM;
        for(nkS32 r=0; r<3; ++r)
            for(nkS32 c1=0; c1<3; ++c1)
                for(nkS32 c0=0; c0<3; ++c0)
                    m.x[c1][r] += x[c0][r] * rhs.x[c1][c0];
        *this = m;
        return *this;
    }
};

NKAPI NKFORCEINLINE nkMat3 nk_m3_identity(void)
{
    nkMat3 m = NK_ZERO_MEM;
    m.x00 = 1;
    m.x11 = 1;
    m.x22 = 1;
    return m;
}

NKAPI NKFORCEINLINE nkMat3 operator+(nkMat3 a, const nkF32&  b) { a += b; return a; }
NKAPI NKFORCEINLINE nkMat3 operator-(nkMat3 a, const nkF32&  b) { a -= b; return a; }
NKAPI NKFORCEINLINE nkMat3 operator/(nkMat3 a, const nkF32&  b) { a /= b; return a; }
NKAPI NKFORCEINLINE nkMat3 operator*(nkMat3 a, const nkF32&  b) { a *= b; return a; }
NKAPI NKFORCEINLINE nkMat3 operator+(nkMat3 a, const nkMat3& b) { a += b; return a; }
NKAPI NKFORCEINLINE nkMat3 operator-(nkMat3 a, const nkMat3& b) { a -= b; return a; }
NKAPI NKFORCEINLINE nkMat3 operator/(nkMat3 a, const nkMat3& b) { a /= b; return a; }
NKAPI NKFORCEINLINE nkMat3 operator*(nkMat3 a, const nkMat3& b) { a *= b; return a; }

NKAPI NKFORCEINLINE nkVec3 operator*(nkMat3 a, const nkVec3& b)
{
    nkVec3 v = NK_ZERO_MEM;
    for(nkS32 r=0; r<3; ++r)
        for(nkS32 c=0; c<3; ++c)
            v.raw[r] += a.x[c][r] * b.raw[c];
    return v;
}

NKAPI NKFORCEINLINE nkBool operator==(const nkMat3& a, const nkMat3& b)
{
    for(nkS32 r=0; r<3; ++r)
        for(nkS32 c=0; c<3; ++c)
            if(a.x[c][r] != b.x[c][r])
                return NK_FALSE;
    return NK_TRUE;
}
NKAPI NKFORCEINLINE nkBool operator!=(const nkMat3& a, const nkMat3& b)
{
    return !(a == b);
}
// =============================================================================

// nkMat4 ======================================================================
union nkMat4
{
    struct
    {
        nkF32 x00,x01,x02,x03;
        nkF32 x10,x11,x12,x13;
        nkF32 x20,x21,x22,x23;
        nkF32 x30,x31,x32,x33;
    };

    nkF32 x[4][4];
    nkF32 raw[4*4];

    nkMat4& operator+=(const nkF32&  rhs) { for(nkS32 c=0; c<4; ++c) for(nkS32 r=0; r<4; ++r) x[c][r] += rhs;         return *this; }
    nkMat4& operator-=(const nkF32&  rhs) { for(nkS32 c=0; c<4; ++c) for(nkS32 r=0; r<4; ++r) x[c][r] -= rhs;         return *this; }
    nkMat4& operator/=(const nkF32&  rhs) { for(nkS32 c=0; c<4; ++c) for(nkS32 r=0; r<4; ++r) x[c][r] /= rhs;         return *this; }
    nkMat4& operator*=(const nkF32&  rhs) { for(nkS32 c=0; c<4; ++c) for(nkS32 r=0; r<4; ++r) x[c][r] *= rhs;         return *this; }

    nkMat4& operator+=(const nkMat4& rhs) { for(nkS32 c=0; c<4; ++c) for(nkS32 r=0; r<4; ++r) x[c][r] += rhs.x[c][r]; return *this; }
    nkMat4& operator-=(const nkMat4& rhs) { for(nkS32 c=0; c<4; ++c) for(nkS32 r=0; r<4; ++r) x[c][r] -= rhs.x[c][r]; return *this; }
    nkMat4& operator/=(const nkMat4& rhs) { for(nkS32 c=0; c<4; ++c) for(nkS32 r=0; r<4; ++r) x[c][r] /= rhs.x[c][r]; return *this; }

    nkMat4& operator*=(const nkMat4& rhs)
    {
        nkMat4 m = NK_ZERO_MEM;
        for(nkS32 r=0; r<4; ++r)
            for(nkS32 c1=0; c1<4; ++c1)
                for(nkS32 c0=0; c0<4; ++c0)
                    m.x[c1][r] += x[c0][r] * rhs.x[c1][c0];
        *this = m;
        return *this;
    }
};

NKAPI NKFORCEINLINE nkMat4 nk_m4_identity(void)
{
    nkMat4 m = NK_ZERO_MEM;
    m.x00 = 1;
    m.x11 = 1;
    m.x22 = 1;
    m.x33 = 1;
    return m;
}

NKAPI NKFORCEINLINE nkMat4 operator+(nkMat4 a, const nkF32&  b) { a += b; return a; }
NKAPI NKFORCEINLINE nkMat4 operator-(nkMat4 a, const nkF32&  b) { a -= b; return a; }
NKAPI NKFORCEINLINE nkMat4 operator/(nkMat4 a, const nkF32&  b) { a /= b; return a; }
NKAPI NKFORCEINLINE nkMat4 operator*(nkMat4 a, const nkF32&  b) { a *= b; return a; }
NKAPI NKFORCEINLINE nkMat4 operator+(nkMat4 a, const nkMat4& b) { a += b; return a; }
NKAPI NKFORCEINLINE nkMat4 operator-(nkMat4 a, const nkMat4& b) { a -= b; return a; }
NKAPI NKFORCEINLINE nkMat4 operator/(nkMat4 a, const nkMat4& b) { a /= b; return a; }
NKAPI NKFORCEINLINE nkMat4 operator*(nkMat4 a, const nkMat4& b) { a *= b; return a; }

NKAPI NKFORCEINLINE nkVec4 operator*(nkMat4 a, const nkVec4& b)
{
    nkVec4 v = NK_ZERO_MEM;
    for(nkS32 r=0; r<4; ++r)
        for(nkS32 c=0; c<4; ++c)
            v.raw[r] += a.x[c][r] * b.raw[c];
    return v;
}

NKAPI NKFORCEINLINE nkBool operator==(const nkMat4& a, const nkMat4& b)
{
    for(nkS32 r=0; r<4; ++r)
        for(nkS32 c=0; c<4; ++c)
            if(a.x[c][r] != b.x[c][r])
                return NK_FALSE;
    return NK_TRUE;
}
NKAPI NKFORCEINLINE nkBool operator!=(const nkMat4& a, const nkMat4& b)
{
    return !(a == b);
}
// =============================================================================

// nk_torad --------------------------------------------------------------------
NKAPI NKFORCEINLINE nkF32 nk_torad(nkF32 deg)
{
    return (deg * NK_PI_F32 / 180.0f);
}
NKAPI NKFORCEINLINE nkF64 nk_torad(nkF64 deg)
{
    return (deg * NK_PI_F64 / 180.0);
}
// -----------------------------------------------------------------------------

// nk_todeg --------------------------------------------------------------------
NKAPI NKFORCEINLINE nkF32 nk_todeg(nkF32 rad)
{
    return (rad * 180.0f / NK_PI_F32);
}
NKAPI NKFORCEINLINE nkF64 nk_todeg(nkF64 rad)
{
    return (rad * 180.0 / NK_PI_F64);
}
// -----------------------------------------------------------------------------

// nk_clamp --------------------------------------------------------------------
NKAPI NKFORCEINLINE nkS32 nk_clamp(nkS32 x, nkS32 l, nkS32 u)
{
    return ((x < l) ? l : ((u < x) ? u : x));
}
NKAPI NKFORCEINLINE nkS64 nk_clamp(nkS64 x, nkS64 l, nkS64 u)
{
    return ((x < l) ? l : ((u < x) ? u : x));
}
NKAPI NKFORCEINLINE nkU32 nk_clamp(nkU32 x, nkU32 l, nkU32 u)
{
    return ((x < l) ? l : ((u < x) ? u : x));
}
NKAPI NKFORCEINLINE nkU64 nk_clamp(nkU64 x, nkU64 l, nkU64 u)
{
    return ((x < l) ? l : ((u < x) ? u : x));
}
NKAPI NKFORCEINLINE nkF32 nk_clamp(nkF32 x, nkF32 l, nkF32 u)
{
    return ((x < l) ? l : ((u < x) ? u : x));
}
NKAPI NKFORCEINLINE nkF64 nk_clamp(nkF64 x, nkF64 l, nkF64 u)
{
    return ((x < l) ? l : ((u < x) ? u : x));
}
NKAPI NKFORCEINLINE nkVec2 nk_clamp(const nkVec2& x, nkF32 l, nkF32 u)
{
    nkVec2 res;
    res.x = nk_clamp(x.x, l,u);
    res.y = nk_clamp(x.y, l,u);
    return res;
}
NKAPI NKFORCEINLINE nkVec3 nk_clamp(const nkVec3& x, nkF32 l, nkF32 u)
{
    nkVec3 res;
    res.x = nk_clamp(x.x, l,u);
    res.y = nk_clamp(x.y, l,u);
    res.z = nk_clamp(x.y, l,u);
    return res;
}
NKAPI NKFORCEINLINE nkVec4 nk_clamp(const nkVec4& x, nkF32 l, nkF32 u)
{
    nkVec4 res;
    res.x = nk_clamp(x.x, l,u);
    res.y = nk_clamp(x.y, l,u);
    res.z = nk_clamp(x.y, l,u);
    res.w = nk_clamp(x.y, l,u);
    return res;
}
// -----------------------------------------------------------------------------

// nk_min ----------------------------------------------------------------------
NKAPI NKFORCEINLINE nkS32 nk_min(nkS32 a, nkS32 b)
{
    return ((a < b) ? a : b);
}
NKAPI NKFORCEINLINE nkS64 nk_min(nkS64 a, nkS64 b)
{
    return ((a < b) ? a : b);
}
NKAPI NKFORCEINLINE nkU32 nk_min(nkU32 a, nkU32 b)
{
    return ((a < b) ? a : b);
}
NKAPI NKFORCEINLINE nkU64 nk_min(nkU64 a, nkU64 b)
{
    return ((a < b) ? a : b);
}
NKAPI NKFORCEINLINE nkF32 nk_min(nkF32 a, nkF32 b)
{
    return ((a < b) ? a : b);
}
NKAPI NKFORCEINLINE nkF64 nk_min(nkF64 a, nkF64 b)
{
    return ((a < b) ? a : b);
}
// -----------------------------------------------------------------------------

// nk_max ----------------------------------------------------------------------
NKAPI NKFORCEINLINE nkS32 nk_max(nkS32 a, nkS32 b)
{
    return ((a < b) ? b : a);
}
NKAPI NKFORCEINLINE nkS64 nk_max(nkS64 a, nkS64 b)
{
    return ((a < b) ? b : a);
}
NKAPI NKFORCEINLINE nkU32 nk_max(nkU32 a, nkU32 b)
{
    return ((a < b) ? b : a);
}
NKAPI NKFORCEINLINE nkU64 nk_max(nkU64 a, nkU64 b)
{
    return ((a < b) ? b : a);
}
NKAPI NKFORCEINLINE nkF32 nk_max(nkF32 a, nkF32 b)
{
    return ((a < b) ? b : a);
}
NKAPI NKFORCEINLINE nkF64 nk_max(nkF64 a, nkF64 b)
{
    return ((a < b) ? b : a);
}
// -----------------------------------------------------------------------------

// nk_map_range ----------------------------------------------------------------
NKAPI NKFORCEINLINE nkS32 nk_map_range(nkS32 x, nkS32 amin, nkS32 amax, nkS32 bmin, nkS32 bmax)
{
    return (bmin + (bmax - bmin) * ((x - amin) / (amax - amin)));
}
NKAPI NKFORCEINLINE nkS64 nk_map_range(nkS64 x, nkS64 amin, nkS64 amax, nkS64 bmin, nkS64 bmax)
{
    return (bmin + (bmax - bmin) * ((x - amin) / (amax - amin)));
}
NKAPI NKFORCEINLINE nkU32 nk_map_range(nkU32 x, nkU32 amin, nkU32 amax, nkU32 bmin, nkU32 bmax)
{
    return (bmin + (bmax - bmin) * ((x - amin) / (amax - amin)));
}
NKAPI NKFORCEINLINE nkU64 nk_map_range(nkU64 x, nkU64 amin, nkU64 amax, nkU64 bmin, nkU64 bmax)
{
    return (bmin + (bmax - bmin) * ((x - amin) / (amax - amin)));
}
NKAPI NKFORCEINLINE nkF32 nk_map_range(nkF32 x, nkF32 amin, nkF32 amax, nkF32 bmin, nkF32 bmax)
{
    return (bmin + (bmax - bmin) * ((x - amin) / (amax - amin)));
}
NKAPI NKFORCEINLINE nkF64 nk_map_range(nkF64 x, nkF64 amin, nkF64 amax, nkF64 bmin, nkF64 bmax)
{
    return (bmin + (bmax - bmin) * ((x - amin) / (amax - amin)));
}
// -----------------------------------------------------------------------------

// nk_sin_range ----------------------------------------------------------------
NKAPI NKFORCEINLINE nkF32 nk_sin_range(nkF32 min, nkF32 max, nkF32 t)
{
    nkF32 half_range = (max - min) / 2.0f;
    return (min + half_range + sinf(t) * half_range);
}
NKAPI NKFORCEINLINE nkF64 nk_sin_range(nkF64 min, nkF64 max, nkF64 t)
{
    nkF64 half_range = (max - min) / 2.0;
    return (min + half_range + sin(t) * half_range);
}
// -----------------------------------------------------------------------------

// nk_lerp ---------------------------------------------------------------------
NKAPI NKFORCEINLINE nkF32 nk_lerp(nkF32 a, nkF32 b, nkF32 t)
{
    return (a + t * (b - a));
}
NKAPI NKFORCEINLINE nkF64 nk_lerp(nkF64 a, nkF64 b, nkF64 t)
{
    return (a + t * (b - a));
}
NKAPI NKFORCEINLINE nkVec2 nk_lerp(const nkVec2& a, const nkVec2& b, nkF32 t)
{
    return (a + t * (b - a));
}
NKAPI NKFORCEINLINE nkVec3 nk_lerp(const nkVec3& a, const nkVec3& b, nkF32 t)
{
    return (a + t * (b - a));
}
NKAPI NKFORCEINLINE nkVec4 nk_lerp(const nkVec4& a, const nkVec4& b, nkF32 t)
{
    return (a + t * (b - a));
}
// -----------------------------------------------------------------------------

// nk_length -------------------------------------------------------------------
NKAPI NKFORCEINLINE nkF32 nk_length(const nkVec2& v)
{
    return sqrtf((v.x*v.x) + (v.y*v.y));
}
NKAPI NKFORCEINLINE nkF32 nk_length(const nkVec3& v)
{
    return sqrtf((v.x*v.x) + (v.y*v.y) + (v.z*v.z));
}
NKAPI NKFORCEINLINE nkF32 nk_length(const nkVec4& v)
{
    return sqrtf((v.x*v.x) + (v.y*v.y) + (v.z*v.z) + (v.w*v.w));
}
// -----------------------------------------------------------------------------

// nk_length2 ------------------------------------------------------------------
NKAPI NKFORCEINLINE nkF32 nk_length2(const nkVec2& v)
{
    return ((v.x*v.y) + (v.y*v.y));
}
NKAPI NKFORCEINLINE nkF32 nk_length2(const nkVec3& v)
{
    return ((v.x*v.y) + (v.y*v.y) + (v.z*v.z));
}
NKAPI NKFORCEINLINE nkF32 nk_length2(const nkVec4& v)
{
    return ((v.x*v.y) + (v.y*v.y) + (v.z*v.z) + (v.w*v.w));
}
// -----------------------------------------------------------------------------

// nk_normalize ----------------------------------------------------------------
NKAPI NKFORCEINLINE nkVec2 nk_normalize(const nkVec2& v)
{
    nkF32 len = nk_length(v);
    return ((len) ? v / len : v);
}
NKAPI NKFORCEINLINE nkVec3 nk_normalize(const nkVec3& v)
{
    nkF32 len = nk_length(v);
    return ((len) ? v / len : v);
}
NKAPI NKFORCEINLINE nkVec4 nk_normalize(const nkVec4& v)
{
    nkF32 len = nk_length(v);
    return ((len) ? v / len : v);
}
// -----------------------------------------------------------------------------

// nk_dot ----------------------------------------------------------------------
NKAPI NKFORCEINLINE nkF32 nk_dot(const nkVec2& a, const nkVec2& b)
{
    return ((a.x*b.x) + (a.y*b.y));
}
NKAPI NKFORCEINLINE nkF32 nk_dot(const nkVec3& a, const nkVec3& b)
{
    return ((a.x*b.x) + (a.y*b.y) + (a.z*b.z));
}
NKAPI NKFORCEINLINE nkF32 nk_dot(const nkVec4& a, const nkVec4& b)
{
    return ((a.x*b.x) + (a.y*b.y) + (a.z*b.z) + (a.w*b.w));
}
// -----------------------------------------------------------------------------

// nk_rotate -------------------------------------------------------------------
NKAPI NKFORCEINLINE nkVec2 nk_rotate(const nkVec2& v, nkF32 angle)
{
    nkVec2 r;
    r.x = v.x * cosf(angle) - v.y * sinf(angle);
    r.y = v.x * sinf(angle) + v.y * cosf(angle);
    return r;
}
// -----------------------------------------------------------------------------

// nk_cross --------------------------------------------------------------------
NKAPI NKFORCEINLINE nkVec3 nk_cross(const nkVec3& a, const nkVec3& b)
{
    nkVec3 v;
    v.x = (a.y*b.z) - (a.z*b.y);
    v.y = (a.z*b.x) - (a.x*b.z);
    v.z = (a.x*b.y) - (a.y*b.x);
    return v;
}
// -----------------------------------------------------------------------------

// nk_inverse ------------------------------------------------------------------
NKAPI NKFORCEINLINE nkMat2 nk_inverse(const nkMat2& m)
{
    nkMat2 inv;

    inv.x00 =  m.x11;
    inv.x01 = -m.x01;
    inv.x10 = -m.x10;
    inv.x11 =  m.x00;

    nkF32 det = m.x00 * m.x11 - m.x01 * m.x10;

    inv /= det;

    return inv;
}
NKAPI NKFORCEINLINE nkMat3 nk_inverse(const nkMat3& m)
{
    nkMat3 inv;

    inv.x00 =  (m.x11 * m.x22 - m.x12 * m.x21);
    inv.x01 = -(m.x01 * m.x22 - m.x21 * m.x02);
    inv.x02 =  (m.x01 * m.x12 - m.x11 * m.x02);
    inv.x10 = -(m.x10 * m.x22 - m.x20 * m.x12);
    inv.x11 =  (m.x00 * m.x22 - m.x02 * m.x20);
    inv.x12 = -(m.x00 * m.x12 - m.x10 * m.x02);
    inv.x20 =  (m.x10 * m.x21 - m.x20 * m.x11);
    inv.x21 = -(m.x00 * m.x21 - m.x20 * m.x01);
    inv.x22 =  (m.x00 * m.x11 - m.x01 * m.x10);

    nkF32 det = m.x00 * inv.x00 + m.x01 * inv.x10 + m.x02 * inv.x20;

    inv /= det;

    return inv;
}
NKAPI NKFORCEINLINE nkMat4 nk_inverse(const nkMat4& m)
{
    nkF32 t0,t1,t2,t3,t4,t5;

    t0 = m.x22 * m.x33 - m.x32 * m.x23;
    t1 = m.x21 * m.x33 - m.x31 * m.x23;
    t2 = m.x21 * m.x32 - m.x31 * m.x22;
    t3 = m.x20 * m.x33 - m.x30 * m.x23;
    t4 = m.x20 * m.x32 - m.x30 * m.x22;
    t5 = m.x20 * m.x31 - m.x30 * m.x21;

    nkMat4 inv;

    inv.x00 =  (m.x11 * t0 - m.x12 * t1 + m.x13 * t2);
    inv.x10 = -(m.x10 * t0 - m.x12 * t3 + m.x13 * t4);
    inv.x20 =  (m.x10 * t1 - m.x11 * t3 + m.x13 * t5);
    inv.x30 = -(m.x10 * t2 - m.x11 * t4 + m.x12 * t5);

    inv.x01 = -(m.x01 * t0 - m.x02 * t1 + m.x03 * t2);
    inv.x11 =  (m.x00 * t0 - m.x02 * t3 + m.x03 * t4);
    inv.x21 = -(m.x00 * t1 - m.x01 * t3 + m.x03 * t5);
    inv.x31 =  (m.x00 * t2 - m.x01 * t4 + m.x02 * t5);

    t0 = m.x12 * m.x33 - m.x32 * m.x13;
    t1 = m.x11 * m.x33 - m.x31 * m.x13;
    t2 = m.x11 * m.x32 - m.x31 * m.x12;
    t3 = m.x10 * m.x33 - m.x30 * m.x13;
    t4 = m.x10 * m.x32 - m.x30 * m.x12;
    t5 = m.x10 * m.x31 - m.x30 * m.x11;

    inv.x02 =  (m.x01 * t0 - m.x02 * t1 + m.x03 * t2);
    inv.x12 = -(m.x00 * t0 - m.x02 * t3 + m.x03 * t4);
    inv.x22 =  (m.x00 * t1 - m.x01 * t3 + m.x03 * t5);
    inv.x32 = -(m.x00 * t2 - m.x01 * t4 + m.x02 * t5);

    t0 = m.x12 * m.x23 - m.x22 * m.x13;
    t1 = m.x11 * m.x23 - m.x21 * m.x13;
    t2 = m.x11 * m.x22 - m.x21 * m.x12;
    t3 = m.x10 * m.x23 - m.x20 * m.x13;
    t4 = m.x10 * m.x22 - m.x20 * m.x12;
    t5 = m.x10 * m.x21 - m.x20 * m.x11;

    inv.x03 = -(m.x01 * t0 - m.x02 * t1 + m.x03 * t2);
    inv.x13 =  (m.x00 * t0 - m.x02 * t3 + m.x03 * t4);
    inv.x23 = -(m.x00 * t1 - m.x01 * t3 + m.x03 * t5);
    inv.x33 =  (m.x00 * t2 - m.x01 * t4 + m.x02 * t5);

    nkF32 det = m.x00 * inv.x00 + m.x01 * inv.x10 + m.x02 * inv.x20 + m.x03 * inv.x30;

    inv /= det;

    return inv;
}
// -----------------------------------------------------------------------------

// nk_transforms ---------------------------------------------------------------
NKAPI NKFORCEINLINE nkMat4 nk_orthographic(nkF32 l, nkF32 r, nkF32 b, nkF32 t, nkF32 n = 0.0f, nkF32 f = 1.0f)
{
    nkF32 inv_right_left =  1.0f / (r - l);
    nkF32 inv_top_bottom =  1.0f / (t - b);
    nkF32 inv_far_near   = -1.0f / (f - n);

    nkMat4 res = NK_ZERO_MEM;
    res.x00 =  (2.0f   * inv_right_left);
    res.x11 =  (2.0f   * inv_top_bottom);
    res.x22 =  (2.0f   * inv_far_near);
    res.x30 = -(r + l) * inv_right_left;
    res.x31 = -(t + b) * inv_top_bottom;
    res.x32 =  (f + n) * inv_far_near;
    res.x33 =  (1.0f);
    return res;
}
NKAPI NKFORCEINLINE nkMat4 nk_perspective(nkF32 fov, nkF32 aspect, nkF32 znear = 0.001f, nkF32 zfar = 100.0f)
{
    nkF32 inv_tan_half_fovy = 1.0f / tanf(fov * 0.5f);
    nkF32 inv_far_near = 1.0f / (znear - zfar);

    nkMat4 res = NK_ZERO_MEM;
    res.x00 =  (inv_tan_half_fovy / aspect);
    res.x11 =  (inv_tan_half_fovy);
    res.x22 =  (znear + zfar) * inv_far_near;
    res.x23 = -(1.0f);
    res.x32 =  (2.0f * znear * zfar * inv_far_near);
    return res;
}
NKAPI NKFORCEINLINE nkMat4 nk_lookat(const nkVec3& eye, const nkVec3& center, const nkVec3& up)
{
    nkVec3 f = nk_normalize(center - eye);
    nkVec3 s = nk_normalize(nk_cross(f, up));
    nkVec3 u = nk_cross(s, f);

    nkMat4 res;
    res.x00 =  s.x;
    res.x01 =  u.x;
    res.x02 = -f.x;
    res.x10 =  s.y;
    res.x11 =  u.y;
    res.x12 = -f.y;
    res.x20 =  s.z;
    res.x21 =  u.z;
    res.x22 = -f.z;
    res.x30 = -nk_dot(s, eye);
    res.x31 = -nk_dot(u, eye);
    res.x32 =  nk_dot(f, eye);
    res.x03 =  0.0f;
    res.x13 =  0.0f;
    res.x23 =  0.0f;
    res.x33 =  1.0f;
    return res;
}
NKAPI NKFORCEINLINE nkMat4 nk_translate(const nkMat4& m, const nkVec3& pos)
{
    nkF32 x30 = m.x00 * pos.x + m.x10 * pos.y + m.x20 * pos.z + m.x30;
    nkF32 x31 = m.x01 * pos.x + m.x11 * pos.y + m.x21 * pos.z + m.x31;
    nkF32 x32 = m.x02 * pos.x + m.x12 * pos.y + m.x22 * pos.z + m.x32;

    nkMat4 res = m;
    res.x30 = x30;
    res.x31 = x31;
    res.x32 = x32;
    return res;
}
NKAPI NKFORCEINLINE nkMat4 nk_rotate(const nkMat4& m, const nkVec3& axis, nkF32 angle)
{
    nkF32 c = cosf(angle);
    nkF32 s = sinf(angle);

    nkVec3 a = nk_normalize(axis); // Ensure that the rotation axis is unit length.
    nkVec3 t = { ((1.f-c)*a.x), ((1.f-c)*a.y), ((1.f-c)*a.z) };

    nkMat4 r;
    r.x00 = c + t.x * a.x;
    r.x01 = t.x * a.y + s * a.z;
    r.x02 = t.x * a.z - s * a.y;
    r.x10 = t.y * a.x - s * a.z;
    r.x11 = c + t.y * a.y;
    r.x12 = t.y * a.z + s * a.x;
    r.x20 = t.z * a.x + s * a.y;
    r.x21 = t.z * a.y - s * a.x;
    r.x22 = c + t.z * a.z;

    nkMat4 tmp;
    tmp.x00 = m.x00 * r.x00 + m.x10 * r.x01 + m.x20 * r.x02;
    tmp.x01 = m.x01 * r.x00 + m.x11 * r.x01 + m.x21 * r.x02;
    tmp.x02 = m.x02 * r.x00 + m.x12 * r.x01 + m.x22 * r.x02;
    tmp.x10 = m.x00 * r.x10 + m.x10 * r.x11 + m.x20 * r.x12;
    tmp.x11 = m.x01 * r.x10 + m.x11 * r.x11 + m.x21 * r.x12;
    tmp.x12 = m.x02 * r.x10 + m.x12 * r.x11 + m.x22 * r.x12;
    tmp.x20 = m.x00 * r.x20 + m.x10 * r.x21 + m.x20 * r.x22;
    tmp.x21 = m.x01 * r.x20 + m.x11 * r.x21 + m.x21 * r.x22;
    tmp.x22 = m.x02 * r.x20 + m.x12 * r.x21 + m.x22 * r.x22;

    nkMat4 res = m;
    res.x00 = tmp.x00;
    res.x01 = tmp.x01;
    res.x02 = tmp.x02;
    res.x10 = tmp.x10;
    res.x11 = tmp.x11;
    res.x12 = tmp.x12;
    res.x20 = tmp.x20;
    res.x21 = tmp.x21;
    res.x22 = tmp.x22;
    return res;
}
NKAPI NKFORCEINLINE nkMat4 nk_scale(const nkMat4& m, const nkVec3& scale)
{
    nkMat4 res = m;
    res.x00 *= scale.x;
    res.x01 *= scale.x;
    res.x02 *= scale.x;
    res.x10 *= scale.y;
    res.x11 *= scale.y;
    res.x12 *= scale.y;
    res.x20 *= scale.z;
    res.x21 *= scale.z;
    res.x22 *= scale.z;
    return res;
}
// -----------------------------------------------------------------------------

#endif /* NK_MATH_H__ ////////////////////////////////////////////////////////*/

/*******************************************************************************
 * MIT License
 *
 * Copyright (c) 2022-2023 Joshua Robertson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
*******************************************************************************/
