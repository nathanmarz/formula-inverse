#include "ROnScreenDisplay.h"
#include "RPhysics.h"


ROnScreenDisplay::ROnScreenDisplay(REngine *eng, RCamera *rc, RPlayer *rp)
{
  m_engine = eng;
  m_rc = rc;
  m_fm = new RFontManager("../fonts/VeraBI.ttf", 10);
  m_rp = rp;
  m_countDown = 4;
  m_lastCount = 0;

  m_speedBarHeight = (m_rc->GetHeight()*PERCENT_HEIGHT);
  m_speedBarWidth = (m_speedBarHeight*SPEED_BAR_RATIO);
  m_boostBarWidth = (1.5*m_speedBarWidth);
  m_boostBarHeight = (m_speedBarWidth);
  m_upperY = m_rc->GetHeight()-MARGIN;
  m_rightX = m_rc->GetWidth()-MARGIN;
  m_lowerY = m_upperY-m_speedBarHeight;
  m_leftX = m_rightX-m_speedBarWidth;
  m_colorTime = 0;
  m_lastLapTimeLeftMotherFucker = 3000;
}

ROnScreenDisplay::~ROnScreenDisplay()
{
  delete m_fm;
}

void ROnScreenDisplay::DrawDisplay(int deltaTime)
{
  BeginOnScreenDisplay();
  //DrawBoostGauge();
  DrawTimer(deltaTime);
  DrawSpeedGauge();
  DrawRankGauge(deltaTime);
  //DrawLapsGauge();

  DrawLastLap(deltaTime);
  // This must be done last otherwise the opaque quads for the other
  // gauges become overly dark
  //DrawFinishedPosition();
  EndOnScreenDisplay();
}

void ROnScreenDisplay::DrawLastLap(int deltaTime)
{
  if(m_rp->LapsCompleted()==m_rp->LapsToWin()-1) {
    if(m_lastLapTimeLeftMotherFucker>0) {
      SDL_Color color = { 255, 0, 0 };
      int width = m_fm->GetTextWidth("Last Lap");
      int height = m_fm->GetTextHeight("Last Lap");
      m_fm->WriteText("Last Lap", color, m_rc->GetWidth()/2-width/2, m_rc->GetHeight()/2-height/2);
      m_lastLapTimeLeftMotherFucker-=deltaTime;
    }
  }
}

//
// BeginOnScreenDisplay - sets the matrix mode to ortho
//
void ROnScreenDisplay::BeginOnScreenDisplay()
{
  int viewport[4];

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glGetIntegerv(GL_VIEWPORT, viewport);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, viewport[2], 0, viewport[3]);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  //glDisable(GL_LIGHT0);
  //glClear(GL_COLOR_BUFFER_BIT);
  
}

void ROnScreenDisplay::EndOnScreenDisplay()
{
  //glEnable(GL_LIGHT0);
  //glFlush();

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();
}

void ROnScreenDisplay::DrawBoostGauge()
{
  int boosts = m_rp->physicsEngine->BoostsLeft();
  int boostTime = m_rp->physicsEngine->BoostTimeLeft();
  float height = 20;
  float width = 20;
  float xcoord = 10;
  float ycoord = m_rc->GetHeight()-10;
  GLuint texture = RResources::GetTexture(-5);
  //glDisable(GL_CULL_FACE);
  //glBlendFunc(GL_ONE,GL_ONE);
  //glEnable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D, texture);
  glBegin(GL_QUADS);
  for (int i = 0; i < boosts; i++, ycoord -= height+10) {
    glTexCoord2f(0,0);
    glVertex3f(xcoord, ycoord, 0);
    
    glTexCoord2f(0,1);
    glVertex3f(xcoord, ycoord-height, 0);
    
    glTexCoord2f(1,1);
    glVertex3f(xcoord+width, ycoord-height, 0);
    
    glTexCoord2f(1,0);
    glVertex3f(xcoord+width, ycoord, 0);
  }
  glEnd();
}

void ROnScreenDisplay::DrawSpeedGauge()
{
  // If the boost is on draw it in red and with a height proportional
  // to how much boost time is left and the rest in 
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  int boostTime = m_rp->physicsEngine->BoostTimeLeft();
  float boostBarHeight = ((float)boostTime)/((float)BOOST_TIME) * m_speedBarHeight;

  // Draw an alpha blended background for the speed bar
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glBegin(GL_QUADS);
  glColor4f(0.0, 0.0, 0.0, 0.2);
  glVertex2f(m_rightX, m_upperY);
  glVertex2f(m_leftX, m_upperY);
  glVertex2f(m_leftX, m_lowerY);
  glVertex2f(m_rightX, m_lowerY);
  glEnd();  
  if (boostBarHeight != 0.0) {
    glBegin(GL_QUADS);
    glColor4f(0.0, 0.0, 1.0, 0.5);
    glVertex2f(m_rightX, m_lowerY+boostBarHeight);
    glVertex2f(m_leftX, m_lowerY+boostBarHeight);
    glVertex2f(m_leftX, m_lowerY);
    glVertex2f(m_rightX, m_lowerY);
    glEnd();
  }
  glDisable(GL_BLEND);
  // Draw the rectangles that represent the actual speed
  const float numSpeedRects = 15;
  const float vertSpacer = m_speedBarHeight*.02;
  const float horizSpacer = m_speedBarWidth*.1;
  const float rectHeight = (m_speedBarHeight-(numSpeedRects+1)*vertSpacer)/numSpeedRects;
  const float rectWidth = m_speedBarWidth-horizSpacer*2;
  float y = m_lowerY;
  float x = m_leftX+horizSpacer;

  float percentMaxSpeed = m_rp->GetSpeed()/m_rp->GetMaxSpeed();
  //int boxes = (int)rintf(numSpeedRects * percentMaxSpeed);
  int boxes = (int)(1.0*numSpeedRects * percentMaxSpeed+.5);
  if (boxes > numSpeedRects)
    boxes = (int)numSpeedRects;
  glBegin(GL_QUADS);
  for (int i = 1; i <= boxes; i++) {
    y += vertSpacer;
    float r = ((float)i)/numSpeedRects;
    float g = 1-r;
    glColor4f(r, g, 0.0, 1.0);
    glVertex2f(x,y);
    glVertex2f(x+rectWidth,y);
    y += rectHeight;
    glVertex2f(x+rectWidth,y);
    glVertex2f(x,y);

  }
  glEnd();
  
  // Draw the boosts

  GLuint texture = RResources::GetTexture(-5);

  glBindTexture(GL_TEXTURE_2D, texture);
  glEnable(GL_TEXTURE_2D);

  // First the background
  //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc(GL_ONE,GL_ONE);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  //glEnable(GL_LIGHTING);
  glEnable(GL_BLEND);
  glBegin(GL_QUADS);
  glColor4f(0.0, 0.0, 0.0, 0.2);
  glTexCoord2f(1,0);
  glVertex2f(m_leftX-MARGIN, m_upperY);
  
  glTexCoord2f(0,0);
  glVertex2f(m_leftX-MARGIN-m_boostBarWidth, m_upperY);
  
  glTexCoord2f(0,1);
  glVertex2f(m_leftX-MARGIN-m_boostBarWidth, m_upperY-m_boostBarHeight);
  
  glTexCoord2f(1,1);
  glVertex2f(m_leftX-MARGIN, m_upperY-m_boostBarHeight);
  glEnd();
  glDisable(GL_BLEND);
  //glDisable(GL_LIGHTING);

  glDisable(GL_TEXTURE_2D);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glBegin(GL_QUADS);

  glColor4f(0.0, 0.0, 0.0, 0.2);

  glVertex2f(m_leftX-MARGIN, m_upperY);

  glVertex2f(m_leftX-MARGIN-m_boostBarWidth, m_upperY);

  glVertex2f(m_leftX-MARGIN-m_boostBarWidth, m_upperY-m_boostBarHeight);

  glVertex2f(m_leftX-MARGIN, m_upperY-m_boostBarHeight);

  glEnd();
  glDisable(GL_BLEND);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glBegin(GL_QUADS);

  glColor4f(0.0, 0.0, 0.0, 0.2);

  glVertex2f(m_leftX-MARGIN, m_upperY-m_boostBarHeight-MARGIN);

  glVertex2f(m_leftX-MARGIN-m_boostBarWidth, m_upperY-m_boostBarHeight-MARGIN);

  glVertex2f(m_leftX-MARGIN-m_boostBarWidth, m_upperY-2*m_boostBarHeight-MARGIN);

  glVertex2f(m_leftX-MARGIN, m_upperY-2*m_boostBarHeight-MARGIN);

  glEnd();
  glDisable(GL_BLEND);

  glPopAttrib();
  int boosts = m_rp->physicsEngine->BoostsLeft();
  int FONTSIZE = (int)(m_rc->GetHeight()*.02);
  char boostsLeft[10];
  sprintf(boostsLeft, "%d", boosts);
  SDL_Color color = { 255, 0 , 0 };
  FONTSIZE*=2;
  m_fm->SetSize(FONTSIZE);

  float xpos = ((m_leftX-MARGIN)+(m_leftX-MARGIN-m_boostBarWidth))/2;
  float ypos = ((m_upperY-m_boostBarHeight-MARGIN)+(m_upperY-2*m_boostBarHeight-MARGIN))/2;
  int boostsWidth = m_fm->GetTextWidth(boostsLeft);
  int boostsHeight = FONTSIZE;// m_fm->GetTextHeight(boostsLeft);
  m_fm->WriteText(boostsLeft, color, xpos-boostsWidth/2, ypos-boostsHeight);
}

void ROnScreenDisplay::DrawRankGauge(int deltaTime)
{
  char rank[20];
  m_colorTime += deltaTime;
  SDL_Color color;

  int red, blue;

  if(m_colorTime>=500) {
    red = 255;
    blue = 0;
  } else {
    blue = 255;
    red = 0;
  }
  if(m_colorTime>=1000) m_colorTime = 0;
  if (m_rp->TimeToCompleteRace() != -1) {
    color.r = red; color.g = blue; color.b = 0;
  }
  else {
    color.r = 255; color.g = 255; color.b = 255;
  }
  m_fm->SetSize((int)(m_rc->GetHeight()*0.08));
  switch(m_rp->PositionInRace()) {
  case 1:
    sprintf(rank, "1st");
    break;
  case 2:
    sprintf(rank, "2nd");
    break;
  case 3:
    sprintf(rank, "3rd");
    break;
  case 4:
    sprintf(rank, "4th");
    break;
  default:
    cerr << "Invalid Position " << endl;
  }

  m_fm->WriteText(rank, color, MARGIN, MARGIN);
}

void ROnScreenDisplay::DrawLapsGauge(float x, float y)
{
  char laps[20];
  SDL_Color color;
  color.r = 255; color.g = 255; color.b = 255;
  //m_fm->SetSize(();
  sprintf(laps, "Laps Completed: %d/%d", m_rp->LapsCompleted(), m_rp->LapsToWin());
  m_fm->WriteText(laps, color, m_rc->GetWidth()-120, 10);
}

void ROnScreenDisplay::DrawFinishedPosition()
{
  char finish[5];
  SDL_Color color = { 0, 255, 0 };
  if (m_rp->FinishedRace()) {
    sprintf(finish, "%d", m_rp->PositionInRace());
    m_fm->SetSize(100);
    m_fm->WriteText(finish, color, 30, 100);
  }
}

void ROnScreenDisplay::OutputCountDown(int deltaTime)
{
  if (m_countDown == 0) {
    m_engine->StartRace();
    m_lastCount = 0;
  }
  else {
    if (m_countDown != 4) {
      char count[3];
      int width, height;
      SDL_Color color = { 255, 0, 0 };
      sprintf(count, "%d", m_countDown);
      // SetSize must be done before getting width/height
      m_fm->SetSize((int)(m_rc->GetHeight()/4));
      width = m_fm->GetTextWidth(count);
      height = m_fm->GetTextHeight(count);
      m_fm->WriteText(count, color, 
		      m_rc->GetWidth()/2-width/2,
		      m_rc->GetHeight()/2-height/2);
    }
    //cout << "m_lastCount: " << m_lastCount << ", deltaTime: " << deltaTime << endl;
    m_lastCount += deltaTime;
    if (m_lastCount > 1000) {
      // if we get an absurdly large deltaTime, just set m_lastCount to
      // zero 
      if (m_lastCount%1000 > 100) {
        m_lastCount = 0;
      }
        //else {
	m_lastCount = m_lastCount%1000;
	m_countDown--;
    //}
    }
  }
}

void ROnScreenDisplay::DrawTimer(int deltaTime)
{
  const int FONTSIZE = (int)(m_rc->GetHeight()*.02);
  int minutes;
  int seconds;
  int milliseconds;
  int temp;
  char time[35];
  int timeElapsed = m_rp->TimeToCompleteRace();
  if(timeElapsed==-1)
    timeElapsed = m_rp->TotalTimeElapsed();

  minutes = timeElapsed/60000;
  temp = timeElapsed%60000;
  seconds = temp/1000;
  milliseconds = temp%1000;
  m_fm->SetSize(FONTSIZE);
  int width = m_fm->GetTextWidth("Current Lap Time: ");
  int height = m_fm->GetTextHeight(time);
  int timeWidth = m_fm->GetTextWidth("-- : -- : ---");
  // Draw the background first
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glBegin(GL_QUADS);
  glColor4f(0.0, 0.0, 0.0, 0.2);
  glVertex2f(MARGIN, m_rc->GetHeight()-MARGIN);
  glVertex2f(MARGIN, m_rc->GetHeight()-2*MARGIN-(height*4));
  glVertex2f(2*width, m_rc->GetHeight()-2*MARGIN-(height*4));
  glVertex2f(2*width, m_rc->GetHeight()-MARGIN);
  glEnd();  

  glDisable(GL_BLEND);

  SDL_Color color = { 255, 0, 0 };
  sprintf(time, "Total Time:");
  m_fm->WriteText(time, color, 2*MARGIN, m_rc->GetHeight()-2*MARGIN-height);  
  m_fm->WriteText(time, color, 2*MARGIN, m_rc->GetHeight()-2*MARGIN-height);  
  color.r = 255; color.g = 255; color.b = 255;
  sprintf(time, "%02d : %02d : %03d", minutes, seconds, milliseconds);
  m_fm->WriteText(time, color, (2*MARGIN)+width, m_rc->GetHeight()-2*MARGIN-height);  

  // Current Lap Time
  char curLapTime[40];
  timeElapsed = m_rp->CurrLapTime();
  minutes = timeElapsed/60000;
  temp = timeElapsed%60000;
  seconds = temp/1000;
  milliseconds = temp%1000;
  sprintf(curLapTime, "Current Lap Time: ");
  color.r = 255; color.g = 0; color.b = 0;
  m_fm->WriteText(curLapTime, color, 2*MARGIN, m_rc->GetHeight()-2*MARGIN-(height*2));  

  color.r = 255; color.g = 255; color.b = 255;
  if (timeElapsed == -1) {
    m_fm->WriteText("-- : -- : ---", color,
		    (2*MARGIN)+width, m_rc->GetHeight()-2*MARGIN-(height*2));
  }
  else {
    sprintf(curLapTime, "%02d : %02d : %03d", minutes, seconds,  milliseconds);
    m_fm->WriteText(curLapTime, color, 
		    (2*MARGIN)+width, m_rc->GetHeight()-2*MARGIN-(height*2));
  }

  // Best Lap Time
  char bestLapTime[40];
  timeElapsed = m_rp->BestLapTime();
  minutes = timeElapsed/60000;
  temp = timeElapsed%60000;
  seconds = temp/1000;
  milliseconds = temp%1000;
  sprintf(bestLapTime, "Best Lap Time: ");
  color.r = 255; color.g = 0; color.b = 0;
  m_fm->WriteText(bestLapTime, color, 
		  2*MARGIN, m_rc->GetHeight()-2*MARGIN-(height*3));

  color.r = 255; color.g = 255; color.b = 255;
  if (timeElapsed == -1) {
    m_fm->WriteText("-- : -- : ---", color,
		    (2*MARGIN)+width, m_rc->GetHeight()-2*MARGIN-(height*3));
  }
  else {
    sprintf(bestLapTime, "%02d : %02d : %03d", minutes,
	    seconds, milliseconds);
    m_fm->WriteText(bestLapTime, color, 
		    (2*MARGIN)+width, m_rc->GetHeight()-2*MARGIN-(height*3));
  }

  char laps[20];
  color.r = 255; color.g = 0; color.b = 0;
  sprintf(laps, "Laps Completed: ");
  m_fm->WriteText(laps, color, 2*MARGIN, m_rc->GetHeight()-2*MARGIN-(height*4));
  sprintf(laps, "%d/%d", m_rp->LapsCompleted(), m_rp->LapsToWin());
  color.r = 255; color.g = 255; color.b = 255;
  m_fm->WriteText(laps, color, (2*MARGIN)+width, m_rc->GetHeight()-2*MARGIN-(height*4));

}
