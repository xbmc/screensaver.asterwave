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
#include "Water.h"

#define MAX_COLORS 160

class AnimationEffect
{
  public:
    virtual ~AnimationEffect() {}
    void init(WaterSettings * settings);
    void reset();
    virtual void apply() = 0;

    int minDuration(){return minTime;};
    int maxDuration(){return maxTime;};

  protected:

    virtual void load(){};
    virtual void start(){};

    WaterSettings * m_pSettings;
    CRGBA palette[MAX_COLORS];
    float scalex, scaley, cenx, ceny, minx, miny, minscale, maxscale;
    int minTime,maxTime, startFrame;

};

class EffectRain : public AnimationEffect
{
  public:
    void start();
    void apply();
  protected:
    float rainDensity;
};

class EffectSwirl : public AnimationEffect
{
  public:
    void start();
    void apply();
  protected:
    int swirlImages;
    bool invertSwirls;
};
class EffectXBMCLogo : public AnimationEffect
{
  public:
    void start();
    void apply();
};

#define NUM_BUBBLES 160
class EffectBoil : public AnimationEffect
{
  struct Bubble
  {
    float size;
    float x;
    float y;
    float speed;
    bool alive;
  };

  public:
    void start();
    void apply();
  private:
    void drawBubbles();
    void incrementBubbles();
    void combineBubbles(Bubble * bubbleA, Bubble * bubbleB);
    bool bubblesTooClose(Bubble * bubbleA, Bubble * bubbleB);
    void popBubble(Bubble * bub);
    Bubble bubbles[NUM_BUBBLES];
    float boilingDensity;
};

#define NUM_BULLETS 160
class EffectBullet : public AnimationEffect
{
  struct Bullet
  {
    float size;
    float x;
    float y;
    float dx;
    float dy;
    float speed;
    bool alive;
    int deadTime;
  };

  public:
    void start();
    void apply();
  private:
    void drawBullets();
    void incrementBullets();
    void bounceBullets(Bullet * bulletA, Bullet * bulletB);
    bool bulletsTooClose(Bullet * bulletA, Bullet * bulletB);
    void resetBullet(Bullet * bul);
    int timeToHit(Bullet*bul);

    Bullet bullets[NUM_BULLETS];
    float bulletDensity;
    float minsize;
    float maxsize;
};

class EffectTwist : public AnimationEffect
{
  public:
    void start();
    void apply();
  protected:
    float speed;
    float rotate;
    float modulate;
    int count;
};

class EffectText : public AnimationEffect
{
  public:
    void start();
    void apply();
    void drawString(char * sz, float spacing, float sizex, float sizey, float width, float posx, float posy);
  protected:
    char marqueeString[256];
    void drawLine(float xa, float ya, float xb, float yb, float width);
    void drawChar(char c, float sizex, float sizey, float width, float posx, float posy);
};
