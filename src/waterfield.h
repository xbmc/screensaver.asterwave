/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) 2007 Asteron (http://asteron.projects.googlepages.com/home)
 *  This file is part of Kodi - https://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Util.h"
#include "types.h"

#define STEP_TIME 0.1f

struct WaterPoint
{
  float height;
  float velocity;
  CRGBA color;
  CRGBA avecolor;
  CVector normal;
};

class CScreensaverAsterwave;

class WaterField
{
public:
  WaterField(CScreensaverAsterwave* base);
  WaterField(CScreensaverAsterwave* base, float xmin, float xmax, float ymin, float ymax, int xdivs, int ydivs, float height, float elasticity, float viscosity, float tension, float blendability, bool textureMode);
  ~WaterField();
  void Init(float xmin, float xmax, float ymin, float ymax, int xdivs, int ydivs, float height, float elasticity, float viscosity, float tension, float blendability, bool textureMode);

  void SetHeight(float xNearest, float yNearest, float spread, float newHeight, const CRGBA& color);
  void DrawLine(float xStart, float yStart, float xEnd, float yEnd, float width, float newHeight,float strength, const CRGBA& color);
  float GetHeight(float xNearest, float yNearest);
  void Render();
  void Step();
  void Step(float time);
  float xMin(){return myXmin;}
  float xMax(){return myXmax;}
  float yMin(){return myYmin;}
  float yMax(){return myYmax;}


private:
  void GetIndexNearestXY(float x, float y, int *i, int *j);
  void SetNormalForPoint(int i, int j);
  CVector* NormalForPoints(CVector* norm, int i, int j, int ai, int aj, int bi, int bj);
  CScreensaverAsterwave* m_base;
  float myXmin;
  float myYmin;
  float myXmax;
  float myYmax;
  int myXdivs;
  int myYdivs;
  float myHeight;
  float m_xdivdist;
  float m_ydivdist;
  float m_elasticity;
  float m_viscosity;
  float m_tension;
  float m_blendability;
  bool m_textureMode;
  WaterPoint** myPoints;
};
