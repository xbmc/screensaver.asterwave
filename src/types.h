/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2005 Joakim Eriksson <je@plane9.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <stdint.h>
#include <math.h>
#include <stdlib.h>

/***************************** D E F I N E S *******************************/

typedef signed char      s8;
typedef unsigned char    u8;
typedef signed short     s16;
typedef unsigned short   u16;
typedef signed long      s32;
typedef unsigned long    u32;
typedef int64_t          s64;
typedef uint64_t         u64;
typedef float            f32;
typedef double           f64;
#define null        0

const f32 PI        = 3.14159265358979323846f;
const f32 FLOATEPSILON    = 0.00001f;

/****************************** M A C R O S ********************************/

#define SAFE_DELETE(_p)      { if(_p) { delete _p;    _p=nullptr; } }
#define SAFE_DELETE_ARRAY(_p)  { if(_p) { delete [] _p;  _p=nullptr; } }
#define SAFE_RELEASE(_p)    { if(_p) { _p->Release();  _p=nullptr; } }

// Direct 3d verify
#define DVERIFY( _func )        \
  {                  \
    HRESULT _retCode = _func;    \
    if( _retCode !=  D3D_OK)    \
    {                \
      return false;        \
    }                \
  }

//      char buf[2000];        \
//      sprintf(buf, "\""#_func"\" returned 0x%lx in\n" __FILE__ "(%ld)\n", _retCode, __LINE__);  \
//      OutputDebugString(buf);    \

#define SQR(_x)          ((_x)*(_x))
#define DEGTORAD(d)      ((d)*(PI / 180.0f))
#define RADTODEG(r)      ((r)*(180.0f / PI))
#ifndef assert
#define assert(_x)
#endif

/***************************** C L A S S E S *******************************/

////////////////////////////////////////////////////////////////////////////
//
class CRGBA
{
public:
  union
  {
    f32 col[4];
    struct
    {
      f32 r,g,b,a;
    };
  };

  CRGBA() { col[0] = col[1] = col[2] = 0.0f;  col[3] = 1.0f;  }
  CRGBA(f32 R, f32 G, f32 B, f32 A) { col[0]=R; col[1]=G; col[2]=B; col[3]=A; }
  u32 RenderColor(void) const;
  void Set(f32 R, f32 G, f32 B, f32 A) { col[0]=R; col[1]=G; col[2]=B; col[3]=A; }
  static CRGBA Lerp(const CRGBA& a, const CRGBA& e, float ratio)
  {
    CRGBA result;
    result.r = a.r*(1.0-ratio)+e.r*ratio;
    result.g = a.g*(1.0-ratio)+e.g*ratio;
    result.b = a.b*(1.0-ratio)+e.b*ratio;
    result.a = 1.0;
    return result;
  }
  CRGBA& operator = (const CRGBA& c) { r = c.r; g = c.g; b = c.b; a = c.a; return *this; }
};

////////////////////////////////////////////////////////////////////////////
//
class CVector
{
public:
  CVector() { }
  CVector(f32 _x, f32 _y, f32 _z) { x = _x; y = _y; z = _z; w = 0.0f; }
  CVector(f32 _x, f32 _y, f32 _z, f32 _w) { x = _x; y = _y; z = _z; w = _w; }
  void Zero() { x = y = z = 0.0f; }
  void Set(f32 _x, f32 _y, f32 _z) { x = _x; y = _y; z = _z; }
  void Cross(const CVector& a, const CVector& b)
  {
    x = a.y*b.z-b.y*a.z;
    y = a.z*b.x-b.z*a.x;
    z = a.x*b.y-b.x*a.y;
  }
  void Normalize()
  {
    double len = sqrt(x*x+y*y+z*z);
    x /= len;
    y /= len;
    z /= len;
  }
  f32 x;
  f32 y;
  f32 z;
  f32 w;
};

////////////////////////////////////////////////////////////////////////////
//
class CVector2
{
public:
  f32 x, y;

  CVector2() { }
  CVector2(f32 _x, f32 _y) { x = _x; y = _y; }
  void Zero() { x = y = 0.0f;  }
  void Set(f32 _x, f32 _y) { x = _x; y = _y; }
  CVector2  Rotate(f32 angel);

  CVector2& operator += (const CVector2& v) {   x += v.x;   y += v.y;   return *this;  }

  friend CVector2 operator - (const CVector2& v1, const CVector2& v2) { return CVector2(v1.x-v2.x, v1.y-v2.y); }
  friend CVector2 operator + (const CVector2& v1, const CVector2& v2) { return CVector2(v1.x+v2.x, v1.y+v2.y); }
  friend CVector2 operator * (const CVector2& v, f32 s) { return CVector2(s*v.x, s*v.y); }
  friend CVector2 operator / (const CVector2& v, f32 s) { f32 oneOver = 1.0f/s; return CVector2(oneOver*v.x, oneOver*v.y); }
};

/***************************** G L O B A L S *******************************/
/***************************** I N L I N E S *******************************/

inline f32 Clamp(f32 x, f32 min, f32 max) { return (x <= min ? min : (x >= max ? max : x)); }
inline f32 RandFloat(void) { return (1.0f / RAND_MAX) * ((f32)rand());  }
inline f32 RandSFloat(void) { return (RandFloat()*2.0f)-1.0f;  }
inline f32 RandFloat(f32 min, f32 max) { return min + ((max-min)*RandFloat()); }
inline int Rand(int max) { return rand() % max; }
inline f32 SquareMagnitude(const CVector2& v) { return v.x*v.x + v.y*v.y;  }


////////////////////////////////////////////////////////////////////////////
//
inline CVector2 CVector2::Rotate(f32 angleDeg)
{
  CVector2  v;
  f32 rad = DEGTORAD(angleDeg);
    v.x = x * cos(rad) +  y * sin(rad);
    v.y = y * cos(rad) -  x * sin(rad);
  return v;
}


////////////////////////////////////////////////////////////////////////////
//
inline f32 DotProduct(const CVector& v1, const CVector& v2)
{
  return v1.x*v2.x + v1.y * v2.y + v1.z*v2.z;
}

////////////////////////////////////////////////////////////////////////////
//
inline f32 DotProduct(const CVector2& v1, const CVector2& v2)
{
  return v1.x*v2.x + v1.y * v2.y;
}

////////////////////////////////////////////////////////////////////////////
//
inline CVector2 Normalized(const CVector2& v)
{
  f32 length = sqrtf(v.x*v.x + v.y*v.y);
  if (length < FLOATEPSILON)
    return CVector2(0.0f, 0.0f);
  return v/length;
}

////////////////////////////////////////////////////////////////////////////
//
inline  f32 InterpolateFloat(f32 v1, f32 v2, f32 t, bool linear)
{
  assert((t >= 0.0f) && (t <= 1.0f));
  if (linear)
  {
    return v1 + t*(v2 - v1);
  }

  // Compute Hermite spline coefficients for t, where 0 <= t <= 1.
  f32 t2 = t * t;
  f32 t3 = t * t2;
  f32 z  = 3.0f * t2 - t3 - t3;
  return v1*(1.0f - z) + v2*z;
}

class CMatrix
{
public:
  void Identity(void);
  void Rotate(f32 angleX, f32 angleY, f32 angleZ);
  void Translate(f32 x, f32 y, f32 z);
  void Scale(f32 sx, f32 sy, f32 sz)          { _11 *= sx; _22 *= sy; _33 *= sz; }
  void Multiply(const CMatrix& m1, const CMatrix& m2);
  CVector operator * ( const CVector& v ) const;

  f32 _11;
  f32 _12;
  f32 _13;
  f32 _14;
  f32 _21;
  f32 _22;
  f32 _23;
  f32 _24;
  f32 _31;
  f32 _32;
  f32 _33;
  f32 _34;
  f32 _41;
  f32 _42;
  f32 _43;
  f32 _44;
};

////////////////////////////////////////////////////////////////////////////
//
inline void CMatrix::Identity(void)
{
  _12 = _13 = _14 = 0.0f;
  _21 = _23 = _24 = 0.0f;
  _31 = _32 = _34 = 0.0f;
  _41 = _42 = _43 = 0.0f;
  _11 = _22 = _33 = _44 = 1.0f;
}

////////////////////////////////////////////////////////////////////////////
// Create a rotation matrix
//
inline void CMatrix::Rotate(f32 angleX, f32 angleY, f32 angleZ)
{
  f32 x = DEGTORAD(angleX);
  f32 y = DEGTORAD(angleY);
  f32 z = DEGTORAD(angleZ);
  _11 = cos(z)*cos(y)+sin(z)*sin(x)*sin(y);
  _12 = sin(z)*cos(x);
  _13 = cos(z)*-sin(y)+sin(z)*sin(x)*cos(y);
  _21 = -sin(z)*cos(y)+cos(z)*sin(x)*sin(y);
  _22 = cos(z)*cos(x);
  _23 = sin(z)*sin(y)+cos(z)*sin(x)*cos(y);
  _31 = cos(x)*sin(y);
  _32 = -sin(x);
  _33 = cos(x)*cos(y);
  _14 = _24 = _34 = _41 = _42 = _43 = 0.0;
  _44 = 1.0;
}

////////////////////////////////////////////////////////////////////////////
// Create a translation matrix
//
inline void CMatrix::Translate(f32 x, f32 y, f32 z)
{
  _11 = 1.0;
  _12 = 0.0;
  _13 = 0.0;
  _14 = -x;
  _21 = 0.0;
  _22 = 1.0;
  _23 = 0.0;
  _24 = -y;
  _31 = 0.0;
  _32 = 0.0;
  _33 = 1.0;
  _34 = -z;
  _41 = _42 = _43 = 0.0;
  _44 = 1.0;
}

////////////////////////////////////////////////////////////////////////////
//
inline CVector CMatrix::operator * ( const CVector& v ) const
{
  return CVector(  v.x * _11 + v.y * _21 + v.z * _31 + _41,
                   v.x * _12 + v.y * _22 + v.z * _32 + _42,
                   v.x * _13 + v.y * _23 + v.z * _33 + _43);
}

////////////////////////////////////////////////////////////////////////////
//
inline void CMatrix::Multiply(const CMatrix& m1, const CMatrix& m2)
{
  _11 = m1._11*m2._11 + m1._12*m2._21 + m1._13*m2._31 + m1._14*m2._41;
  _21 = m1._21*m2._11 + m1._22*m2._21 + m1._23*m2._31 + m1._24*m2._41;
  _31 = m1._31*m2._11 + m1._32*m2._21 + m1._33*m2._31 + m1._34*m2._41;
  _41 = m1._31*m2._11 + m1._32*m2._21 + m1._33*m2._31 + m1._34*m2._41;
  _12 = m1._11*m2._12 + m1._12*m2._22 + m1._13*m2._32 + m1._14*m2._42;
  _22 = m1._21*m2._12 + m1._22*m2._22 + m1._23*m2._32 + m1._24*m2._42;
  _32 = m1._31*m2._12 + m1._32*m2._22 + m1._33*m2._32 + m1._34*m2._42;
  _42 = m1._31*m2._12 + m1._32*m2._22 + m1._33*m2._32 + m1._34*m2._42;
  _13 = m1._11*m2._13 + m1._12*m2._23 + m1._13*m2._33 + m1._14*m2._43;
  _23 = m1._21*m2._13 + m1._22*m2._23 + m1._23*m2._33 + m1._24*m2._43;
  _33 = m1._31*m2._13 + m1._32*m2._23 + m1._33*m2._33 + m1._34*m2._43;
  _43 = m1._31*m2._13 + m1._32*m2._23 + m1._33*m2._33 + m1._34*m2._43;
  _14 = m1._11*m2._14 + m1._12*m2._24 + m1._13*m2._34 + m1._14*m2._44;
  _24 = m1._21*m2._14 + m1._22*m2._24 + m1._23*m2._34 + m1._24*m2._44;
  _34 = m1._31*m2._14 + m1._32*m2._24 + m1._33*m2._34 + m1._34*m2._44;
  _44 = m1._31*m2._14 + m1._32*m2._24 + m1._33*m2._34 + m1._34*m2._44;
}
