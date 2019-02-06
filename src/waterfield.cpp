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

//////////////////////////////////////////////////////////////////
// WATERFIELD.CPP
//
// This is a really neat water engine that uses some liquid physics
// drive deformations on a triangulated mesh.  This is meant to be
// reused and so is well commented :)
//
//////////////////////////////////////////////////////////////////

#include "waterfield.h"
#include "Water.h"
#include "Util.h"
#include <memory.h>
#include <vector>

WaterField::WaterField(CScreensaverAsterwave* base)
  : m_base(base)
{
  Init(-10,10,-50,50,160,160, 10, 0.1f, 0.7f, 1.0f, 0.54f, false);
}

WaterField::WaterField(CScreensaverAsterwave* base, float xmin, float xmax,
                       float ymin, float ymax, int xdivs, int ydivs,
                       float height, float elasticity, float viscosity,
                       float tension, float blendability, bool textureMode)
  : m_base(base)
{
  Init(xmin, xmax, ymin, ymax, xdivs, ydivs, height, elasticity, viscosity, tension, blendability, textureMode);
}

WaterField::~WaterField()
{
}

/************************************************************
Init

Sets the bounds, tesselation, hieght of the water plane
and allocates triangulated mesh;
************************************************************/
void WaterField::Init(float xmin, float xmax, float ymin, float ymax, int xdivs, int ydivs, float height, float elasticity, float viscosity, float tension, float blendability, bool textureMode)
{
  myXmin = xmin;
  myYmin = ymin;
  myXmax = xmax;
  myYmax = ymax;
  myXdivs = xdivs;
  myYdivs = ydivs;
  myHeight = height;

  m_xdivdist = (float)(myXmax - myXmin) / (float)myXdivs;
  m_ydivdist = (float)(myYmax - myYmin) / (float)myYdivs;

  m_viscosity = viscosity;
  m_elasticity = elasticity;
  m_tension = tension;
  m_blendability = blendability;
  m_textureMode = textureMode;

  myPoints = new WaterPoint*[xdivs];
  for(int i = 0; i < xdivs; i++)
  {
    myPoints[i] = new WaterPoint[ydivs];
    for(int j = 0; j < ydivs; j++)
    {
      myPoints[i][j].height = 0;
      myPoints[i][j].velocity = 0;
      myPoints[i][j].color = CRGBA(0x80,0x80,0x80,0xFF);
      myPoints[i][j].avecolor = CRGBA(0,0,0,255);
    }
  }
}


void WaterField::DrawLine(float xStart, float yStart, float xEnd, float yEnd,
    float width, float newHeight, float strength, const CRGBA& color)
{
  int xa, xb, ya, yb;
  int radiusY = (int)((float)myYdivs*(width) / (myYmax - myYmin));
  int radiusX = radiusY;

  GetIndexNearestXY(xStart,yStart,&xa, &ya);
  GetIndexNearestXY(xEnd,yEnd,&xb,&yb);
  int maxstep = abs(yb - ya) > abs(xb - xa) ? abs(yb - ya) : abs(xb - xa);

  if (maxstep == 0)
    return;
  for (int i = 0; i <= maxstep; i++) {
    int x = xa + (xb-xa)*i/maxstep;
    int y = ya + (yb-ya)*i/maxstep;
    for (int k = -radiusX;  k <= radiusX; k++)
      for (int l = -radiusY;  l <= radiusY; l++)
        if((x+k)>=0 && (y+l)>=0 && (x+k)<myXdivs && (y+l)<myYdivs) {
          if(k*k+l*l <= radiusX*radiusY)
          {
            float ratio = 1.0f-sqrt((float)(k*k+l*l)/(float)(radiusX*radiusY));
            myPoints[x+k][y+l].height = strength*newHeight + (1-strength)*myPoints[x+k][y+l].height;
            myPoints[x+k][y+l].velocity = (1-strength)*myPoints[x+k][y+l].velocity;
            myPoints[x+k][y+l].color = CRGBA::Lerp(myPoints[x+k][y+l].color, color, ratio);
          }
        }
  }
}


/************************************************************
SetHeight

Sets the points within spread of the nearest vertex to
(xNearest,yNearest) to a height of newHeight in a roughly
circular pattern.
************************************************************/
void WaterField::SetHeight(float xNearest, float yNearest,
    float spread, float newHeight, const CRGBA& color)
{
  int xcenter;
  int ycenter;
  int radiusIndexY = (int)((float)myYdivs*(spread) / (myYmax - myYmin));
  int radiusIndexX = radiusIndexY;
  float ratio;
  float xd = ((float)(myXmax - myXmin)) / myXdivs;
  float yd = ((float)(myYmax - myYmin)) / myYdivs;
  if (spread <= 0) return;

  GetIndexNearestXY(xNearest,yNearest,&xcenter, &ycenter);

  for(int i = xcenter-radiusIndexX; i <= xcenter+radiusIndexX; i++)
    for(int j = ycenter-radiusIndexY; j <= ycenter+radiusIndexY; j++)
      if( i>=0 && j>=0 && i<myXdivs && j<myYdivs)
      {
        float x = myXmin + xd * i; //pretend its bigger
        float y = myYmin + yd * j;
        ratio = 1.0f;
        ratio = 1.0f-sqrt((float)((xNearest-x)*(xNearest-x)*yd*yd/xd/xd+(yNearest-y)*(yNearest-y))/(spread*spread));
        if (ratio <= 0)
          continue;
        myPoints[i][j].height = ratio*newHeight + (1-ratio)*myPoints[i][j].height;
        myPoints[i][j].velocity = (1-ratio)*myPoints[i][j].velocity;
        myPoints[i][j].color = CRGBA::Lerp(myPoints[i][j].color, color, ratio);
      }
}

/************************************************************
GetHeight

Gets the height of the nearest vertex to (xNearest,yNearest)
************************************************************/
float WaterField::GetHeight(float xNearest, float yNearest)
{
  int i,j;
  GetIndexNearestXY(xNearest,yNearest,&i,&j);
  return myPoints[i][j].height;
}


void WaterField::Step()
{
  Step(STEP_TIME);
}

/************************************************************
Step

This is where knowledge of physical systems comes in handy.
For every vertex the surface tension is computed as the
cumulative sum of the difference between the vertex height and its
neighbors.  This data is fed into the change in velocity.  The
other values added are for damped oscillation (one is a proportional
control and the other is differential) and so the three physical
quantities accounted for are elasticity, viscosity, and surface tension.

Height is just incremented by time*velocity.
************************************************************/
void WaterField::Step(float time)
{
  int i, j, k, l, mi, ni, mj, nj;
  float cumulativeTension = 0;
  int calRadius = 1;

  for(i=0; i<myXdivs; i++)
    for(j=0; j<myYdivs; j++)
    {
      cumulativeTension = 0;
      myPoints[i][j].avecolor = CRGBA(0,0,0,0);
      ni = i-calRadius > 0 ? i-calRadius : 0;//
      ni = iMax(0,i-calRadius);
      mi = iMin(myXdivs-1, i+calRadius);
      nj = iMax(0,j-calRadius);
      mj = iMin(myYdivs-1, j+calRadius);
      for(k=ni; k<=mi; k++)
        for(l=nj; l<=mj; l++)
        {
          cumulativeTension += myPoints[k][l].height- myPoints[i][j].height;
        }

      myPoints[i][j].velocity +=m_elasticity*(myHeight-myPoints[i][j].height)
        - m_viscosity * myPoints[i][j].velocity
        + m_tension*cumulativeTension;
    }

  for(i=0; i<myXdivs; i++)
    for(j=0; j<myYdivs; j++)
    {
      myPoints[i][j].height += myPoints[i][j].velocity*time;
      SetNormalForPoint(i,j);
    }

}

/************************************************************
Render

Renders the water to the screen relative to the currect origin.
Doesnt do any translations on the matrix.  Each row is done
in one individual triangular strip (so all interior vertices are
done twice :-|  I do not believe it is possible to do it all
as one triangular strip (and have every vertex specified once).
************************************************************/
void WaterField::Render()
{
  int i, j, k;

  if (!m_textureMode)
  {
    std::vector<sLight> verts(2*myYdivs);
    for(i=0; i<myXdivs-1; i++)
    {
      for(j=0; j<myYdivs; j++)
      {
        for (k=0; k<2; k++)
        {
          verts[2*j+k].vertex.x = myXmin + (float)((i+k)*m_xdivdist);
          verts[2*j+k].vertex.y = myYmin + (float)(j*m_ydivdist);
          verts[2*j+k].vertex.z = (float)(myPoints[i+k][j].height);
          verts[2*j+k].normal.x = myPoints[i+k][j].normal.x;
          verts[2*j+k].normal.y = myPoints[i+k][j].normal.y;
          verts[2*j+k].normal.z = myPoints[i+k][j].normal.z;
          verts[2*j+k].color = sColor(myPoints[i+k][j].color.col);
        }
      }
      m_base->Draw(GL_TRIANGLE_STRIP, &verts[0], verts.size(), false);
    }
  }
  else
  {
    std::vector<sLight> verts(2*myYdivs);
    for(i=0; i<myXdivs-1; i++)
    {
      for(j=0; j<myYdivs; j++)
      {
        for (k=0; k<2; k++)
        {
          verts[2*j+k].vertex.x = myXmin + (float)((i+k)*m_xdivdist);
          verts[2*j+k].vertex.y = myYmin + (float)(j*m_ydivdist);
          verts[2*j+k].vertex.z = (float)(myPoints[i+k][j].height);
          verts[2*j+k].normal.x = myPoints[i+k][j].normal.x;
          verts[2*j+k].normal.y = myPoints[i+k][j].normal.y;
          verts[2*j+k].normal.z = myPoints[i+k][j].normal.z;
          verts[2*j+k].coord.u = 0.0f+1.0f*(float)(i+k)/(float)myXdivs + 0.5f*myPoints[i+k][j].normal.x;
          verts[2*j+k].coord.v = 0.0f+1.0f*(float)j/(float)myYdivs + 0.5f*myPoints[i+k][j].normal.y;
          verts[2*j+k].color = 1.0f;
        }
      }
      // Draw it
      m_base->Draw(GL_TRIANGLE_STRIP, &verts[0], verts.size(), true);
    }
  }
}

/************************************************************
GetIndexNearestXY

Calculates the nearest water point index (i,j) from the world
position (x,y).
************************************************************/
void WaterField::GetIndexNearestXY(float x, float y, int *i, int *j)
{
  *i = x <= myXmin ? 0: x >= myXmax ? myXdivs-1:
    (int)((float)myXdivs*(x - myXmin) / (myXmax - myXmin)) ;
  *j = y <= myYmin ? 0: y >= myYmax ? myYdivs-1:
    (int)((float)myYdivs*(y - myYmin) / (myYmax - myYmin)) ;
}

/************************************************************
SetNormalForPoint

Calculates and sets a normal for any point ij in the water mesh
by taking the normal to the plane that goes through three neighboring
points.  This has the effect of smoothing out the lighting.
************************************************************/
void WaterField::SetNormalForPoint(int i, int j)
{
// Formula for the cross product p = v1 x v2
//  p.x = v1.y * v2.z - v2.y * v1.z;
//  p.y = v1.z * v2.x - v2.z * v1.x;
//  p.z = v1.x * v2.y - v2.x * v1.y;

//  these are the vectors to use.
//   v1 = (bi-ai)*xdivdist, (bj-aj)*ydivdist, (myPoints[bi][bj].height-myPoints[ai][aj].height)
//   v2 = (ci-ai)*xdivdist, (cj-aj)*ydivdist, (myPoints[ci][cj].height-myPoints[ai][aj].height)

  CVector* norm = &(myPoints[i][j].normal);
  CVector temp;
  int s = 2; //spread
  int mi = i > s ? i-s : 0;
  int ni = i+s < myXdivs ? i+s : myXdivs-1;
  int mj = j > s ? j-s : 0;
  int nj = j+s < myYdivs ? j+s : myYdivs-1;

  NormalForPoints(norm, mi,j,ni,mj,ni,nj);
}

CVector* WaterField::NormalForPoints(CVector* norm, int i, int j, int ai, int aj, int bi, int bj)
{
  CVector a = CVector((ai-i)*m_xdivdist, (aj-j)*m_ydivdist, (myPoints[ai][aj].height-myPoints[i][j].height));
  CVector b = CVector((bi-i)*m_xdivdist, (bj-j)*m_ydivdist, (myPoints[bi][bj].height-myPoints[i][j].height));
  norm->Cross(a,b);
  norm->Normalize();
  return norm;
}
