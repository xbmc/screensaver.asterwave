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

#include "xbmc_scr_dll.h"
#include "libXBMC_addon.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <SOIL.h>
#include <sys/dir.h>

#include "timer.h"
#include "Water.h"
#include "Effect.h"
#include "Util.h"
#include <cstdlib>
#include <memory.h>

WaterSettings world;
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
  
int  m_iWidth;
int m_iHeight;
CVector g_lightDir;
float g_shininess = 0.4f;
float xmin = -10.0f, 
      xmax = 10.0f, 
      ymin = -10.0f, 
      ymax = 10.0f, 
      height = 0.0f, 
      elasticity = 0.5f, 
      viscosity = 0.05f, 
      tension = 1.0f, 
      blendability = 0.04f;

int xdivs = 50; 
int ydivs = 50;
int divs = 50;

CTimer gTimer;
GLuint gTexture=0;
ADDON::CHelper_libXBMC_addon *XBMC           = NULL;

void CreateLight()
{
  const GLfloat amb[] = {1.0, 1.0, 1.0, 1.0};
  glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
  const GLfloat dif[] = {1.0, 1.0, 1.0, 1.0};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
  const GLfloat spec[] = {0.6, 0.6, 0.6, 1.0};
  glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
  const GLfloat pos[] = {0.0, -5.0, 5.0, 1.0};
  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  const GLfloat dir[] = {g_lightDir.x, g_lightDir.y, g_lightDir.z, g_lightDir.w};
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



////////////////////////////////////////////////////////////////////////////////

// stuff for the background plane
struct BG_VERTEX 
{
    CVector position;
    CRGBA color;
};
BG_VERTEX g_BGVertices[4];
////////////////////////////////////////////////////////////////////////////////
// fill in background vertex array with values that will
// completely cover screen
void SetupGradientBackground(const CRGBA& dwTopColor, const CRGBA& dwBottomColor )
{
  float x1 = -0.5f;
  float y1 = -0.5f;
  float x2 = (float)m_iWidth - 0.5f;
  float y2 = (float)m_iHeight - 0.5f;
  
  g_BGVertices[0].position = CVector( x2, y1, 0.0f, 1.0f );
  g_BGVertices[0].color = dwTopColor;

  g_BGVertices[1].position = CVector( x2, y2, 0.0f, 1.0f );
  g_BGVertices[1].color = dwBottomColor;

  g_BGVertices[2].position = CVector( x1, y1, 0.0f, 1.0f );
  g_BGVertices[2].color = dwTopColor;

  g_BGVertices[3].position = CVector( x1, y2, 0.0f, 1.0f );
  g_BGVertices[3].color = dwBottomColor;

  return;
}
///////////////////////////////////////////////////////////////////////////////
void RenderGradientBackground()
{
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_TRIANGLE_STRIP);
  for (size_t i=0;i<4;++i)
  {
    glColor3f(g_BGVertices[i].color.r, g_BGVertices[i].color.g,
              g_BGVertices[i].color.b);
    glVertex3f(g_BGVertices[i].position.x, g_BGVertices[i].position.y,
               g_BGVertices[i].position.z);
  }
  glEnd();
}

void LoadEffects()
{
  int i = 0;
  while(effects[i] != NULL)
    effects[i++]->init(&world);
  world.effectCount = i;
}

void SetMaterial()
{
  if (world.isTextureMode)
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

void LoadTexture()
{
  // Setup our texture
  //long hFile;
  int numTextures = 0;
  static char szPath[512];
  static char foundTexture[1024];

  strcpy(szPath,world.szTextureSearchPath);
  if (world.szTextureSearchPath[strlen(world.szTextureSearchPath) - 1] != '/')
    strcat(szPath,"/");

  DIR* dir = opendir(szPath);

  struct dirent* entry = readdir(dir);

  while ((entry=readdir(dir)))
  {
    int len = (int)strlen(entry->d_name);
    if (len < 4 || (strcasecmp(entry->d_name + len - 4, ".txt") == 0))
      continue;

    if (rand() % (numTextures+1) == 0) // after n textures each has 1/n prob
    {
      strcpy(foundTexture,szPath);
      strcat(foundTexture,entry->d_name);
    }
    numTextures++;

  }
  closedir(dir);

  if (gTexture != 0&& numTextures > 0)
    glDeleteTextures(1, &gTexture);
  
  gTexture = SOIL_load_OGL_texture(foundTexture, SOIL_LOAD_RGB, 0, 0);
}

void SetDefaults()
{
  world.frame = 0;
  world.nextEffectTime = 0;
  world.isWireframe = false;
  world.isTextureMode = true;
  g_lightDir = CVector(0.0f,0.8f,-0.6f);
  XBMC->GetSetting("__addonpath__", world.szTextureSearchPath);
  strcat(world.szTextureSearchPath,"/resources");
  world.nextTextureTime = 1024;
}

////////////////////////////////////////////////////////////////////////////
// XBMC has loaded us into memory, we should set our core values
// here and load any settings we may have from our config file
//
ADDON_STATUS ADDON_Create(void* hdl, void* props)
{
  if (!props)
    return ADDON_STATUS_UNKNOWN;

  if (!XBMC)
    XBMC = new ADDON::CHelper_libXBMC_addon;

  if (!XBMC->RegisterMe(hdl))
  {
    delete XBMC, XBMC=NULL;
    return ADDON_STATUS_PERMANENT_FAILURE;
  }

  SCR_PROPS* scrprops = (SCR_PROPS*)props;

  m_iWidth = scrprops->width;
  m_iHeight  = scrprops->height;
  memset(&world,0,sizeof(WaterSettings));

  float ratio = (float)m_iWidth/(float)m_iHeight;

  world.scaleX = 1.0f;

  if ( (ratio * m_iWidth / m_iHeight) > 1.5)
    world.scaleX = 1/1.333f;

  SetDefaults();
  CreateLight();
  world.waterField = new WaterField(xmin, xmax, ymin, ymax, xdivs, ydivs, height, elasticity, viscosity, tension, blendability, world.isTextureMode);
  LoadEffects();
  
  if (world.isTextureMode)
  {
    LoadTexture();
    world.effectCount--; //get rid of logo effect
  }

  world.effectType = rand()%world.effectCount;
  world.frame = 0;
  world.nextEffectTime = 0;

  return ADDON_STATUS_OK;
}

void SetCamera()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  float aspectRatio = (float)m_iWidth/(float)m_iHeight;
  gluPerspective(45, aspectRatio, 1.0, 1000.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0.0, 0.0, -15.0, 0.0, 0.0, 0.0, 0.0, -0.707, -0.707);
}

void SetupRenderState()
{
  SetCamera();
  SetMaterial();
  if (world.isWireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  if (world.isTextureMode)
  {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gTexture);
  }
}

// XBMC tells us we should get ready
// to start rendering. This function
// is called once when the screensaver
// is activated by XBMC.
extern "C" void Start()
{
  SetCamera();
  gTimer.Init();
}

// XBMC tells us to render a frame of
// our screensaver. This is called on
// each frame render in XBMC, you should
// render a single frame only - the DX
// device will already have been cleared.

extern "C" void Render()
{
  RenderGradientBackground();
  CreateLight();
  SetupRenderState();

  world.frame++;

  if (world.isTextureMode && world.nextTextureTime>0 && (world.frame % world.nextTextureTime) == 0)
    LoadTexture();

  if (world.frame > world.nextEffectTime)
  {
    if ((rand() % 3)==0)
      incrementColor();
    //static limit = 0;if (limit++>3)
    world.effectType += 1;//+rand() % (ANIM_MAX-1);
    world.effectType %= world.effectCount;
    effects[world.effectType]->reset();
    world.nextEffectTime = world.frame + effects[world.effectType]->minDuration() + 
      rand() % (effects[world.effectType]->maxDuration() - effects[world.effectType]->minDuration());
  }
  effects[world.effectType]->apply();
  gTimer.Update();
  world.waterField->Step(gTimer.GetDeltaTime());
  world.waterField->Render();
  glDisable(GL_LIGHTING);
}

// XBMC tells us to stop the screensaver
// we should free any memory and release
// any resources we have created.
extern "C" void ADDON_Stop()
{
  if (gTexture != 0)
    glDeleteTextures(1, &gTexture);
  delete world.waterField;
  world.waterField = NULL;
  for (int i = 0; effects[i] != NULL; i++)
    delete effects[i];
}

extern "C" void ADDON_Destroy()
{
}

extern "C" ADDON_STATUS ADDON_GetStatus()
{
  return ADDON_STATUS_OK;
}

extern "C" bool ADDON_HasSettings()
{
  return false;
}

extern "C" unsigned int ADDON_GetSettings(ADDON_StructSetting ***sSet)
{
  return 0;
}

extern "C" ADDON_STATUS ADDON_SetSetting(const char *strSetting, const void *value)
{
  return ADDON_STATUS_OK;
}

extern "C" void ADDON_FreeSettings()
{
}

extern "C" void ADDON_Announce(const char *flag, const char *sender, const char *message, const void *data)
{
}

extern "C" void GetInfo(SCR_INFO *info)
{
}
