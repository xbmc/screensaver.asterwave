/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2007 Asteron (http://asteron.projects.googlepages.com/home)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <kodi/addon-instance/Screensaver.h>
#include <kodi/gui/gl/GL.h>
#include <kodi/gui/gl/Shader.h>
#include <glm/gtc/type_ptr.hpp>

#include "waterfield.h"

void SetAnimation();

struct WaterSettings
{
  WaterField * waterField;
  int effectType;
  int frame;
  int nextEffectTime;
  int nextTextureTime;
  int effectCount;
  float scaleX;
  bool isWireframe;
  bool isTextureMode;
  std::string szTextureSearchPath;
};

// stuff for the background plane
struct BG_VERTEX
{
    CVector position;
    CRGBA color;
};

struct sPosition
{
  sPosition() : x(0.0f), y(0.0f), z(0.0f), u(1.0f) {}
  sPosition(float* d) : x(d[0]), y(d[1]), z(d[2]), u(1.0f) {}
  sPosition(float x, float y, float z = 0.0f) : x(x), y(y), z(z), u(1.0f) {}
  float x,y,z,u;
};

struct sCoord
{
  sCoord() : u(0.0f), v(0.0f) {}
  sCoord(float u, float v) : u(u), v(v) {}
  float u,v;
};

struct sColor
{
  sColor() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
  sColor(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
  sColor(float* c) : r(c[0]), g(c[1]), b(c[2]), a(c[3]) {}
  sColor& operator=(float* rhs)
  {
    r = rhs[0];
    g = rhs[1];
    b = rhs[2];
    return *this;
  }
  sColor& operator=(float rhs)
  {
    r = rhs;
    g = rhs;
    b = rhs;
    return *this;
  }
  float r,g,b,a;
};

struct sLight
{
  sPosition vertex;
  sPosition normal;
  sColor color;
  sCoord coord;
};

class ATTR_DLL_LOCAL CScreensaverAsterwave
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverAsterwave();

  // kodi::addon::CInstanceScreensaver
  bool Start() override;
  void Stop() override;
  void Render() override;

  // kodi::gui::gl::CShaderProgram
  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

  void Draw(int primitive, const sLight* data, unsigned int size, bool withTexture);

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

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;
  glm::mat3 m_normalMat;
  glm::vec4 m_materialAmbient;
  glm::vec4 m_materialDiffuse;
  glm::vec4 m_materialSpecular;
  glm::vec4 m_materialEmission;
  glm::vec4 m_lightAmbient;
  glm::vec4 m_lightDiffuse;
  glm::vec4 m_lightSpecular;
  glm::vec4 m_lightPosition;
  glm::vec4 m_lightSpotDirection;
  float m_lightConstantAttenuation;
  float m_lightLinearAttenuation;
  float m_lightQuatraticAttenuation;
  float m_shininess = 20.0f;

  GLint m_projMatLoc = -1;
  GLint m_modelViewMatLoc = -1;
  GLint m_transposeAdjointModelViewMatrixLoc = -1;
  GLint m_textureIdLoc = -1;
  GLint m_hVertex = -1;
  GLint m_hNormal = -1;
  GLint m_hCoord = -1;
  GLint m_hColor = -1;

  GLint m_light0_ambientLoc = -1;
  GLint m_light0_diffuseLoc = -1;
  GLint m_light0_specularLoc = -1;
  GLint m_light0_positionLoc = -1;
  GLint m_light0_constantAttenuationLoc = -1;
  GLint m_light0_linearAttenuationLoc = -1;
  GLint m_light0_quadraticAttenuationLoc = -1;
  GLint m_light0_spotDirectionLoc = -1;
  GLint m_light0_spotExponentLoc = -1;
  GLint m_light0_spotCutoffAngleCosLoc = -1;
  GLint m_material_ambientLoc = -1;
  GLint m_material_diffuseLoc = -1;
  GLint m_material_specularLoc = -1;
  GLint m_material_emissionLoc = -1;
  GLint m_material_shininessLoc = -1;

  int m_iWidth;
  int m_iHeight;
  CVector m_lightDir;
  WaterSettings m_world;
  GLuint m_Texture;
  BG_VERTEX m_BGVertices[4];
  double m_lastTime;
  double m_lastImageTime = 0;
  bool m_startOK = false;

  GLuint m_vertexVBO = 0;

  float xmin = -10.0f;
  float xmax = 10.0f;
  float ymin = -10.0f;
  float ymax = 10.0f;
  float height = 0.0f;
  float elasticity = 0.5f;
  float viscosity = 0.05f;
  float tension = 1.0f;
  float blendability = 0.04f;
  int xdivs = 50;
  float ydivs = 50;
  float divs = 50;
};
