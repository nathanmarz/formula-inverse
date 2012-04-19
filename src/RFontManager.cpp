#include "RFontManager.h"
#include <GL/gl.h>
#include "glext.h"
#include <iostream>
#include <math.h>
#include <assert.h>

using namespace std;

RFontManager::RFontManager(const char *fontPath, int size)
{
  if (TTF_Init() == -1) {
    cerr << "TTF_Init: " << TTF_GetError() << endl;
    exit(2);
  }
  //atexit(TTF_Quit);

  //font = TTF_OpenFont(fontPath, size);
  m_size = size;
  font = NULL;
  SetFont(fontPath);
}

RFontManager::~RFontManager()
{
  TTF_CloseFont(font);    
}

void RFontManager::SetFont(const char *fontPath)
{
    m_path = fontPath;
    if(font!=NULL) TTF_CloseFont(font);
    if (!(font = TTF_OpenFont(fontPath, m_size))) {
      cerr << "Error loading font: " << TTF_GetError() << endl;
      return;
    }   
}

void RFontManager::SetSize(int size)
{
  m_size = size;
  SetFont(m_path);
}

int RFontManager::GetTextWidth(char *text)
{
  int width, dummy_height;
  TTF_SizeText(font, text, &width, &dummy_height);
  return width;
}

int RFontManager::GetTextHeight(char *text)
{
  int height, dummy_width;
  TTF_SizeText(font, text, &dummy_width, &height);
  return height;
}

void RFontManager::WriteText(const char *text, SDL_Color color, float x, float y)
{
  int vPort[4];

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glGetIntegerv(GL_VIEWPORT, vPort);

  // Goto a 2D mode
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, vPort[2], 0, vPort[3], -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_DEPTH_TEST);
  // Draw the text

  glDisable(GL_LIGHTING);
  //glClearColor(0.0f,0.0f,0.0f,0.0f);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendEquation(GL_FUNC_SUBTRACT);
  glBlendFunc(GL_ONE,GL_ONE);
  glEnable(GL_BLEND);
  SDL_Surface *initial;
  SDL_Surface *intermediate;
  GLuint texture;
  int w,h;
  SDL_Color bgcolor = { 0, 0, 0, 255 };
  initial = TTF_RenderText_Blended(font, text, color);
  //initial = TTF_RenderText_Shaded(font, text, color, bgcolor);

  assert(initial!=NULL);

  w = NextPowerOfTwo(initial->w);
  h = NextPowerOfTwo(initial->h);

  intermediate = SDL_CreateRGBSurface(0, w, h, 32,
				      0x000000FF,
				      0x0000FF00,
				      0x00FF0000,
				      0xFF000000);
  
  //SDL_SetAlpha(intermediate, 0, SDL_ALPHA_TRANSPARENT);
  SDL_BlitSurface(initial, 0, intermediate, 0);
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA,
	       GL_UNSIGNED_BYTE, intermediate->pixels);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture);
  glColor4f(1.0f, 1.0f, 1.0f,0.0f);

  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 1.0f);
  glVertex2f(x, y);
  glTexCoord2f(1.0f, 1.0f);
  glVertex2f(x+w, y);
  glTexCoord2f(1.0f, 0.0f);
  glVertex2f(x+w, y+h);
  glTexCoord2f(0.0f, 0.0f);
  glVertex2f(x, y+h);
  glEnd();

  glFinish();
  glDisable(GL_BLEND);
  SDL_FreeSurface(initial);
  SDL_FreeSurface(intermediate);
  glDeleteTextures(1, &texture);


  // End the 2D mode
  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();
}

int RFontManager::NextPowerOfTwo(int x)
{
  float logbase2 = log((float)x) / log(2.0);
  float a = ceil(logbase2);
  return (int)(pow((float)2,(float)a)+.5);
}
