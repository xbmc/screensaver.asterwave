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

#include <kodi/Filesystem.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <dirent.h>

#include "Water.h"
#include "Effect.h"
#include "Util.h"
#include "SOIL2/SOIL2.h"
#include <cstdlib>
#include <memory.h>
#include <chrono>

AnimationEffect * effects[] = {

  new EffectBoil(),
  new EffectTwist(),
  new EffectBullet(),
  new EffectRain(),
  new EffectSwirl(),
  new EffectXBMCLogo(),
  nullptr,
  //new EffectText(),
};

////////////////////////////////////////////////////////////////////////////
// Kodi has loaded us into memory, we should set our core values
// here and load any settings we may have from our config file
//
CScreensaverAsterwave::CScreensaverAsterwave()
  : m_Texture(0)
{
}

// Kodi tells us we should get ready
// to start rendering. This function
// is called once when the screensaver
// is activated by Kodi.
bool CScreensaverAsterwave::Start()
{
  std::string fraqShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
  {
    kodi::Log(ADDON_LOG_ERROR, "Failed to create and compile shader");
    return false;
  }

  //SetupGradientBackground(CRGBA(255,0,0,255), CRGBA(0,0,0,255));

  m_iWidth = Width();
  m_iHeight = Height();
  memset(&m_world, 0, sizeof(WaterSettings));

  float ratio = (float)m_iWidth/(float)m_iHeight;

  m_world.scaleX = 1.0f;

  if ( (ratio * m_iWidth / m_iHeight) > 1.5)
    m_world.scaleX = 1/1.333f;

  SetDefaults();
  CreateLight();
  m_world.waterField = new WaterField(this, xmin, xmax, ymin, ymax, xdivs, ydivs, height, elasticity, viscosity, tension, blendability, m_world.isTextureMode);
  LoadEffects();

  if (m_world.isTextureMode)
  {
    LoadTexture();
    m_world.effectCount--; //get rid of logo effect
  }

  m_world.effectType = rand()%m_world.effectCount;
  m_world.frame = 0;
  m_world.nextEffectTime = 0;

  SetCamera();

  glGenBuffers(1, &m_vertexVBO);

  auto time = std::chrono::high_resolution_clock::now();
  m_lastTime = std::chrono::duration<double>(time.time_since_epoch()).count();
  m_lastImageTime = m_lastTime;
  m_startOK = true;
  return true;
}

// Kodi tells us to stop the screensaver
// we should free any memory and release
// any resources we have created.
void CScreensaverAsterwave::Stop()
{
  if (!m_startOK)
    return;
  m_startOK = false;

  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;

  if (m_Texture != 0)
    glDeleteTextures(1, &m_Texture);
  delete m_world.waterField;
  m_world.waterField = nullptr;
  for (int i = 0; effects[i] != nullptr; i++)
    delete effects[i];
}

// Kodi tells us to render a frame of
// our screensaver. This is called on
// each frame render in Kodi, you should
// render a single frame only - the DX
// device will already have been cleared.
void CScreensaverAsterwave::Render()
{
  if (!m_startOK)
    return;

  /*
   * Following Extra work done here in render to prevent problems with controls
   * from Kodi and during window moving.
   * TODO: Maybe add a separate interface call to inform about?
   */
  //@{
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
  glBindTexture(GL_TEXTURE_2D, m_Texture);

  glVertexAttribPointer(m_hVertex, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_hVertex);

  glVertexAttribPointer(m_hNormal, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_hNormal);

  glVertexAttribPointer(m_hColor, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, color)));
  glEnableVertexAttribArray(m_hColor);

  glVertexAttribPointer(m_hCoord, 2, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, coord)));
  glEnableVertexAttribArray(m_hCoord);
  //@}

  auto time = std::chrono::high_resolution_clock::now();
  double currentTime = std::chrono::duration<double>(time.time_since_epoch()).count();
  float frameTime = currentTime - m_lastTime;
  m_lastTime = currentTime;

  // clear
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  //RenderGradientBackground();
  CreateLight();
  SetupRenderState();

  m_world.frame++;
  if (m_world.isTextureMode && m_world.nextTextureTime>0 && (m_lastImageTime+m_world.nextTextureTime < currentTime))
  {
    LoadTexture();
    m_lastImageTime = currentTime;
  }

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
  m_world.waterField->Step(frameTime);
  m_world.waterField->Render();

#ifndef HAS_GLES
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

  glDisableVertexAttribArray(m_hVertex);
  glDisableVertexAttribArray(m_hNormal);
  glDisableVertexAttribArray(m_hColor);
  glDisableVertexAttribArray(m_hCoord);
}

void CScreensaverAsterwave::SetDefaults()
{
  m_world.frame = 0;
  m_world.nextEffectTime = 0;
  m_world.isWireframe = false;
  m_world.isTextureMode = true;
  m_lightDir = CVector(0.0f,0.6f,-0.8f);

  std::string szTextureSearchPath;
  kodi::CheckSettingBoolean("wireframe", m_world.isWireframe);
  kodi::CheckSettingBoolean("texturemode", m_world.isTextureMode);
  if (!kodi::CheckSettingString("texturefolder", szTextureSearchPath) ||
      szTextureSearchPath.empty() ||
      !kodi::vfs::DirectoryExists(szTextureSearchPath))
    m_world.szTextureSearchPath = kodi::GetAddonPath("/resources/images/");
  else
    m_world.szTextureSearchPath = szTextureSearchPath;
  m_world.nextTextureTime = kodi::GetSettingInt("nexttexture");
  kodi::CheckSettingFloat("viscosity", viscosity);
  kodi::CheckSettingFloat("elasticity", elasticity);
  kodi::CheckSettingFloat("height", height);
  xdivs = divs;
  ydivs = divs;
  m_shininess = kodi::GetSettingFloat("shininess")*100.0f;
  xmin = kodi::GetSettingInt("xmin");
  xmax = kodi::GetSettingInt("xmax");
  ymin = kodi::GetSettingInt("ymin");
  xmax = kodi::GetSettingInt("xmax");
  divs = kodi::GetSettingInt("quality");
}

void CScreensaverAsterwave::SetCamera()
{
  float aspectRatio = (float)m_iWidth/(float)m_iHeight;
  m_projMat = glm::perspective(glm::radians(45.0f), aspectRatio, 1.0f, 1000.0f);
  m_modelMat = glm::lookAt(glm::vec3(0.0f, 0.0f, -15.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -0.707f, -0.707f));
}

void CScreensaverAsterwave::SetMaterial()
{
  if (m_world.isTextureMode)
    m_materialDiffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  else
    m_materialDiffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

  m_materialAmbient = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  m_materialSpecular = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
  m_materialEmission = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void CScreensaverAsterwave::CreateLight()
{
  m_lightAmbient = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  m_lightDiffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  m_lightSpecular = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
  m_lightPosition = glm::vec4(0.0f, -5.0, 5.0f, 1.0f);
  m_lightSpotDirection = glm::vec4(m_lightDir.x, m_lightDir.y, m_lightDir.z, m_lightDir.w);
  m_lightConstantAttenuation = 0.5f;
  m_lightLinearAttenuation = 0.2f;
  m_lightQuatraticAttenuation = 0.0f;
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
    if (len < 4 || (strcasecmp(entry->d_name + len - 4, ".png") != 0 &&
                    strcasecmp(entry->d_name + len - 4, ".bmp") != 0 &&
                    strcasecmp(entry->d_name + len - 4, ".jpg") != 0 &&
                    strcasecmp(entry->d_name + len - 4, ".jpeg") != 0))
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

#ifndef HAS_GLES
  if (m_world.isWireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
  if (m_world.isTextureMode)
  {
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
  sLight light[4];
  for (size_t i=0;i<4;++i)
  {
    light[i].color = sColor(m_BGVertices[i].color.r, m_BGVertices[i].color.g, m_BGVertices[i].color.b);
    light[i].vertex = sPosition(m_BGVertices[i].position.x, m_BGVertices[i].position.y, m_BGVertices[i].position.z);
  }

  Draw(GL_TRIANGLE_STRIP, light, 4, false);
}

void CScreensaverAsterwave::Draw(int primitive, const sLight* data, unsigned int size, bool withTexture)
{
  GLuint oldTexture = m_Texture;
  if (!withTexture)
    m_Texture = 0;

  m_normalMat = glm::transpose(glm::inverse(glm::mat3(m_modelMat)));

  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*size, data, GL_DYNAMIC_DRAW);
  glDrawArrays(primitive, 0, size);
  DisableShader();

  if (!withTexture)
    m_Texture = oldTexture;
}

void CScreensaverAsterwave::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_projMatLoc = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_modelViewMatLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_transposeAdjointModelViewMatrixLoc = glGetUniformLocation(ProgramHandle(), "u_transposeAdjointModelViewMatrix");
  m_textureIdLoc = glGetUniformLocation(ProgramHandle(), "u_textureId");

  m_light0_ambientLoc = glGetUniformLocation(ProgramHandle(), "u_light0.ambient");
  m_light0_diffuseLoc = glGetUniformLocation(ProgramHandle(), "u_light0.diffuse");
  m_light0_specularLoc = glGetUniformLocation(ProgramHandle(), "u_light0.specular");
  m_light0_positionLoc = glGetUniformLocation(ProgramHandle(), "u_light0.position");
  m_light0_constantAttenuationLoc = glGetUniformLocation(ProgramHandle(), "u_light0.constantAttenuation");
  m_light0_linearAttenuationLoc = glGetUniformLocation(ProgramHandle(), "u_light0.linearAttenuation");
  m_light0_quadraticAttenuationLoc = glGetUniformLocation(ProgramHandle(), "u_light0.quadraticAttenuation");
  m_light0_spotDirectionLoc = glGetUniformLocation(ProgramHandle(), "u_light0.spotDirection");
  m_light0_spotExponentLoc = glGetUniformLocation(ProgramHandle(), "u_light0.spotExponent");
  m_light0_spotCutoffAngleCosLoc = glGetUniformLocation(ProgramHandle(), "u_light0.spotCutoffAngleCos");

  m_material_ambientLoc = glGetUniformLocation(ProgramHandle(), "u_material.ambient");
  m_material_diffuseLoc = glGetUniformLocation(ProgramHandle(), "u_material.diffuse");
  m_material_specularLoc = glGetUniformLocation(ProgramHandle(), "u_material.specular");
  m_material_emissionLoc = glGetUniformLocation(ProgramHandle(), "u_material.emission");
  m_material_shininessLoc = glGetUniformLocation(ProgramHandle(), "u_material.shininess");

  m_hVertex = glGetAttribLocation(ProgramHandle(), "a_position");
  m_hNormal = glGetAttribLocation(ProgramHandle(), "a_normal");
  m_hColor = glGetAttribLocation(ProgramHandle(), "a_color");
  m_hCoord = glGetAttribLocation(ProgramHandle(), "a_coord");
}

bool CScreensaverAsterwave::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_projMatLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_modelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  glUniformMatrix3fv(m_transposeAdjointModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_normalMat));
  glUniform1i(m_textureIdLoc, m_Texture);

  glUniform4fv(m_light0_ambientLoc, 1, glm::value_ptr(m_lightAmbient));
  glUniform4fv(m_light0_diffuseLoc, 1, glm::value_ptr(m_lightDiffuse));
  glUniform4fv(m_light0_specularLoc, 1, glm::value_ptr( m_lightSpecular));
  glUniform4fv(m_light0_positionLoc, 1, glm::value_ptr(m_lightPosition));
  glUniform3fv(m_light0_spotDirectionLoc, 1, glm::value_ptr(m_lightSpotDirection));
  glUniform1f(m_light0_constantAttenuationLoc, m_lightConstantAttenuation);
  glUniform1f(m_light0_linearAttenuationLoc, m_lightLinearAttenuation);
  glUniform1f(m_light0_quadraticAttenuationLoc, m_lightQuatraticAttenuation);
  glUniform1f(m_light0_spotExponentLoc, 0.0f);
  glUniform1f(m_light0_spotCutoffAngleCosLoc, -1.0f);

  glUniform4fv(m_material_ambientLoc, 1, glm::value_ptr(m_materialAmbient));
  glUniform4fv(m_material_diffuseLoc, 1, glm::value_ptr(m_materialDiffuse));
  glUniform4fv(m_material_specularLoc, 1, glm::value_ptr(m_materialSpecular));
  glUniform4fv(m_material_emissionLoc, 1, glm::value_ptr( m_materialEmission));
  glUniform1f(m_material_shininessLoc, m_shininess);

  return true;
}

ADDONCREATOR(CScreensaverAsterwave);
