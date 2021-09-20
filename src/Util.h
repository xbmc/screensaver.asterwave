/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2007 Asteron (http://asteron.projects.googlepages.com/home)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "types.h"

struct COLORVERTEX
{
  float x, y, z; // The untransformed position for the vertex.
  float nx, ny, nz; // Normal vector for lighting calculations	
  CRGBA color; // The vertex colour.
};

struct TEXTUREDVERTEX
{
  float x, y, z; // The untransformed position for the vertex.
  float nx, ny, nz; // Normal vector for lighting calculations	
  float tu, tv; //texture
};

CRGBA HSVtoRGB( float h, float s, float v );
inline float frand(){return ((float) rand() / (float) RAND_MAX);};

#define iMin(a,b) ((a)<(b)?(a):(b))
#define iMax(a,b) ((a)>(b)?(a):(b))

void incrementColor();
CRGBA randColor();
void TransformCoord(CVector* pOut, CVector* pIn, CMatrix* pMat);
