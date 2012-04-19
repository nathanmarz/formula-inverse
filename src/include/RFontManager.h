#ifndef _RFONTMANAGER_H_
#define _RFONTMANAGER_H_
#include "SDL.h"
#include "SDL_ttf.h"



class RFontManager {
 public:
  RFontManager(const char *fontPath, int size);
  ~RFontManager();

  void SetFont(const char *fontPath);
  void SetSize(int size);

  int GetSize() { return m_size; }
  int GetTextWidth(char *text);
  int GetTextHeight(char *text);
  // SDL_Color has r,g,b members
  // SDL_Rect has x,y members, where 0,0 is the lower left of the screen
  // All text is blended against a black background, then blended
  // against the scene. Therefore, if color is { r=0,g=0,b=0 },
  // then the text will not show up.
  void WriteText(const char *text, SDL_Color color, float x, float y);
  
 protected:
  int NextPowerOfTwo(int x);
  TTF_Font *font;
  const char *m_path;
  int m_size;
};
#endif
