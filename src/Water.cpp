/*
 * Silverwave Screensaver for XBox Media Center
 * Copyright (c) 2004 Team XBMC
 *
  * Ver 1.0 2007-02-12 by Asteron  http://asteron.projects.googlepages.com/home
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2  of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <kodi/addon-instance/Screensaver.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <sys/dir.h>

#include "timer.h"
#include "Water.h"
#include "Effect.h"
#include "Util.h"
#include "SOIL2/SOIL2.h"
#include <cstdlib>
#include <memory.h>

AnimationEffect * effects[] = {

  new EffectBoil(),
  new EffectTwist(),
  new EffectBullet(),
  new EffectRain(),
  new EffectSwirl(),
  new EffectXBMCLogo(),
  NULL,
  //new EffectText(),
};

// stuff for the background plane
struct BG_VERTEX 
{
    CVector position;
    CRGBA color;
};

class CScreensaverAsterwave
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver
{
public:
  CScreensaverAsterwave();

  virtual bool Start() override;
  virtual void Stop() override;
  virtual void Render() override;

private:
  void SetDefaults();
  void SetCamera();
  void SetMaterial();
  void SetupRenderState();
  void CreateLight();
  void LoadTexture();
  void LoadEffects();
  void SetupGradientBackground(const CRGBA& dwTopColor, const CRGBA& dwBottomColor );
  void RenderGradientBackground();
  
  int m_iWidth;
  int m_iHeight;
  CVector m_lightDir;
  WaterSettings m_world;
  GLuint m_Texture;
  CTimer m_Timer;
  BG_VERTEX m_BGVertices[4];

  static constexpr float xmin = -10.0f, 
                  xmax = 10.0f, 
                  ymin = -10.0f, 
                  ymax = 10.0f, 
                  height = 0.0f, 
                  elasticity = 0.5f, 
                  viscosity = 0.05f, 
                  tension = 1.0f, 
                  blendability = 0.04f;
  static constexpr int xdivs = 50,
                       ydivs = 50,
                       divs = 50;
};

////////////////////////////////////////////////////////////////////////////
// Kodi has loaded us into memory, we should set our core values
// here and load any settings we may have from our config file
//
CScreensaverAsterwave::CScreensaverAsterwave()
  : m_Texture(0)
{
  m_iWidth = Width();
  m_iHeight = Height();
  memset(&m_world, 0, sizeof(WaterSettings));

  float ratio = (float)m_iWidth/(float)m_iHeight;

  m_world.scaleX = 1.0f;

  if ( (ratio * m_iWidth / m_iHeight) > 1.5)
    m_world.scaleX = 1/1.333f;

  SetDefaults();
  CreateLight();
  m_world.waterField = new WaterField(xmin, xmax, ymin, ymax, xdivs, ydivs, height, elasticity, viscosity, tension, blendability, m_world.isTextureMode);
  LoadEffects();
  
  if (m_world.isTextureMode)
  {
    LoadTexture();
    m_world.effectCount--; //get rid of logo effect
  }

  m_world.effectType = rand()%m_world.effectCount;
  m_world.frame = 0;
  m_world.nextEffectTime = 0;
}

// Kodi tells us we should get ready
// to start rendering. This function
// is called once when the screensaver
// is activated by Kodi.
bool CScreensaverAsterwave::Start()
{
  SetCamera();
  m_Timer.Init();
  return true;
}

// Kodi tells us to stop the screensaver
// we should free any memory and release
// any resources we have created.
void CScreensaverAsterwave::Stop()
{
  if (m_Texture != 0)
    glDeleteTextures(1, &m_Texture);
  delete m_world.waterField;
  m_world.waterField = NULL;
  for (int i = 0; effects[i] != NULL; i++)
    delete effects[i];
}

// Kodi tells us to render a frame of
// our screensaver. This is called on
// each frame render in Kodi, you should
// render a single frame only - the DX
// device will already have been cleared.
void CScreensaverAsterwave::Render()
{
  RenderGradientBackground();
  CreateLight();
  SetupRenderState();

  m_world.frame++;

  if (m_world.isTextureMode && m_world.nextTextureTime>0 && (m_world.frame % m_world.nextTextureTime) == 0)
    LoadTexture();

  if (m_world.frame > m_world.nextEffectTime)
  {
    if ((rand() % 3)==0)
      incrementColor();
    //static limit = 0;if (limit++>3)
    m_world.effectType += 1;//+rand() % (ANIM_MAX-1);
    m_world.effectType %= m_world.effectCount;
    effects[m_world.effectType]->reset();
    m_world.nextEffectTime = m_world.frame + effects[m_world.effectType]->minDuration() + 
      rand() % (effects[m_world.effectType]->maxDuration() - effects[m_world.effectType]->minDuration());
  }
  effects[m_world.effectType]->apply();
  m_Timer.Update();
  m_world.waterField->Step(m_Timer.GetDeltaTime());
  m_world.waterField->Render();
  glDisable(GL_LIGHTING);
}

void CScreensaverAsterwave::SetDefaults()
{
  m_world.frame = 0;
  m_world.nextEffectTime = 0;
  m_world.isWireframe = false;
  m_world.isTextureMode = true;
  m_lightDir = CVector(0.0f,0.8f,-0.6f);
  m_world.szTextureSearchPath = kodi::GetAddonPath() + "/resources/";
  m_world.nextTextureTime = 1024;
}

void CScreensaverAsterwave::SetCamera()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  float aspectRatio = (float)m_iWidth/(float)m_iHeight;
  gluPerspective(45, aspectRatio, 1.0, 1000.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0.0, 0.0, -15.0, 0.0, 0.0, 0.0, 0.0, -0.707, -0.707);
}

void CScreensaverAsterwave::SetMaterial()
{
  if (m_world.isTextureMode)
  {
    const GLfloat dif[] = {1.0, 1.0, 1.0, 1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dif);
  }
  else
  {
    const GLfloat dif[] = {0.5, 0.5, 0.5, 1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dif);
  }

  const GLfloat amb[] = {1.0, 1.0, 1.0, 1.0};
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
  const GLfloat spec[] = {0.4, 0.4, 0.4, 1.0};
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
  const GLfloat em[] = {0.0, 0.0, 0.0, 0.0};
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, em);
}

void CScreensaverAsterwave::CreateLight()
{
  const GLfloat amb[] = {1.0, 1.0, 1.0, 1.0};
  glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
  const GLfloat dif[] = {1.0, 1.0, 1.0, 1.0};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
  const GLfloat spec[] = {0.6, 0.6, 0.6, 1.0};
  glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
  const GLfloat pos[] = {0.0, -5.0, 5.0, 1.0};
  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  const GLfloat dir[] = {m_lightDir.x, m_lightDir.y, m_lightDir.z, m_lightDir.w};
  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
  float at = 0.5;
  glLightfv(GL_LIGHT0, GL_CONSTANT_ATTENUATION, &at);
  at = 0.02;
  glLightfv(GL_LIGHT0, GL_LINEAR_ATTENUATION, &at);
  at = 0.0;
  glLightfv(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, &at);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
}

void CScreensaverAsterwave::LoadTexture()
{
  // Setup our texture
  int numTextures = 0;
  std::string foundTexture;

  DIR* dir = opendir(m_world.szTextureSearchPath.c_str());

  struct dirent* entry;
  while ((entry=readdir(dir)))
  {
    int len = (int)strlen(entry->d_name);
    if (len < 4 || (strcasecmp(entry->d_name + len - 4, ".txt") == 0))
      continue;

    if (rand() % (numTextures+1) == 0) // after n textures each has 1/n prob
    {
      foundTexture = m_world.szTextureSearchPath + entry->d_name;
    }
    numTextures++;

  }
  closedir(dir);

  if (m_Texture != 0 && !foundTexture.empty())
    glDeleteTextures(1, &m_Texture);
  
  m_Texture = SOIL_load_OGL_texture(foundTexture.c_str(), SOIL_LOAD_RGB, 0, 0);
}

void CScreensaverAsterwave::LoadEffects()
{
  int i = 0;
  while(effects[i] != nullptr)
    effects[i++]->init(&m_world);
  m_world.effectCount = i;
}

void CScreensaverAsterwave::SetupRenderState()
{
  SetCamera();
  SetMaterial();
  if (m_world.isWireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  if (m_world.isTextureMode)
  {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_Texture);
  }
}

////////////////////////////////////////////////////////////////////////////////
// fill in background vertex array with values that will
// completely cover screen
void CScreensaverAsterwave::SetupGradientBackground(const CRGBA& dwTopColor, const CRGBA& dwBottomColor )
{
  float x1 = -0.5f;
  float y1 = -0.5f;
  float x2 = (float)m_iWidth - 0.5f;
  float y2 = (float)m_iHeight - 0.5f;
  
  m_BGVertices[0].position = CVector( x2, y1, 0.0f, 1.0f );
  m_BGVertices[0].color = dwTopColor;

  m_BGVertices[1].position = CVector( x2, y2, 0.0f, 1.0f );
  m_BGVertices[1].color = dwBottomColor;

  m_BGVertices[2].position = CVector( x1, y1, 0.0f, 1.0f );
  m_BGVertices[2].color = dwTopColor;

  m_BGVertices[3].position = CVector( x1, y2, 0.0f, 1.0f );
  m_BGVertices[3].color = dwBottomColor;

  return;
}

void CScreensaverAsterwave::RenderGradientBackground()
{
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_TRIANGLE_STRIP);
  for (size_t i=0;i<4;++i)
  {
    glColor3f(m_BGVertices[i].color.r, m_BGVertices[i].color.g,
              m_BGVertices[i].color.b);
    glVertex3f(m_BGVertices[i].position.x, m_BGVertices[i].position.y,
               m_BGVertices[i].position.z);
  }
  glEnd();
}

ADDONCREATOR(CScreensaverAsterwave);
