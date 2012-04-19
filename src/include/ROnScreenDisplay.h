#ifndef _RONSCREENDISPLAY_H_
#define _RONSCREENDISPLAY_H_

#include "Framework.h"

const int MARGIN = 10;
const float PERCENT_HEIGHT = 0.3;
const float SPEED_BAR_RATIO = 0.2;

class ROnScreenDisplay {
 public:
  ROnScreenDisplay(REngine *eng, RCamera *rc, RPlayer *rp);
  ~ROnScreenDisplay();
  // Draws the whole on screen display for a
  // certain player
  void DrawDisplay(int deltaTime);

  // The rest of the draw commands
  // need to be surrounded by these functions
  void BeginOnScreenDisplay();
  void EndOnScreenDisplay();

  // These draw the displays, assuming
  // you're in the mode created by the functions above
  void DrawBoostGauge();
  void DrawSpeedGauge();
  void DrawRankGauge(int deltaTime);
  void DrawLapsGauge(float x, float y);
  void DrawFinishedPosition();
  void DrawLastLap(int deltaTime);
  void OutputCountDown(int deltaTime);
  void DrawTimer(int deltaTime);
 protected:
  int m_lastLapTimeLeftMotherFucker;

  REngine *m_engine;
  RFontManager *m_fm;
  RCamera *m_rc;
  RPlayer *m_rp;
  int m_countDown;
  int m_lastCount;
  // Coordinates for the speed bar
  float m_upperY;
  float m_rightX;
  float m_lowerY;
  float m_leftX;
  float m_speedBarHeight;
  float m_speedBarWidth;
  float m_boostBarWidth;
  float m_boostBarHeight;
  int m_colorTime;
};

#endif
