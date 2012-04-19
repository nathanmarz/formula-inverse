#include "RGameMenu.h"
#include "Framework.h"
#include <fstream>

class RStartMenuSelectNext : public RGameAction {
public:
  RStartMenuSelectNext(REngine *r, RStartMenu *rs) : RGameAction(r) 
  { m_rs = rs; }
  void KeyPressed(int deltaTime) {
    m_rs->NextSelection();
  }
protected:
  RStartMenu *m_rs;
};

class RStartMenuSelectPrevious : public RGameAction {
public:
  RStartMenuSelectPrevious(REngine *r, RStartMenu *rs) : RGameAction(r) 
  { m_rs = rs; }
  void KeyPressed(int deltaTime) {
    m_rs->PreviousSelection();
  }
protected:
  RStartMenu *m_rs;
};

class RStartMenuNextOption : public RGameAction {
public:
  RStartMenuNextOption(REngine *r, RStartMenu *rs) : RGameAction(r) 
  { m_rs = rs; }
  void KeyPressed(int deltaTime) {
    m_rs->NextOption();
  }
protected:
  RStartMenu *m_rs;
};

class RStartMenuPreviousOption : public RGameAction {
public:
  RStartMenuPreviousOption(REngine *r, RStartMenu *rs) : RGameAction(r) 
  { m_rs = rs; }
  void KeyPressed(int deltaTime) {
    m_rs->PreviousOption();
  }
protected:
  RStartMenu *m_rs;
};

class RMenuQuit: public RGameAction {
public:
    RMenuQuit(REngine *r):RGameAction(r) { }

    void KeyPressed(int deltaTime) {
        //cerr<<"Thanks for playing!"<<endl;
        exit(0);
    }


};

class RStartMenuStartGame : public RGameAction {
public:
  RStartMenuStartGame(REngine *r, RStartMenu *rs) : RGameAction(r) 
  { m_rs = rs; }
  void KeyPressed(int deltaTime) {
    // TODO: Make RWorld take some kind of level id to choose the level
      string lev = m_rs->GetSelectedLevel();
      m_engine->InitGame(m_rs->GetNumPlayers(), lev);
  }
protected:
  RStartMenu *m_rs;
};



RGameMenu::RGameMenu(REngine *eng)
{
  m_eng = eng;
}

RGameMenu::~RGameMenu()
{
  
}

const char *RStartMenu::GetSelectedLevel() 
{ 
  return levelList.front().fileName.c_str(); 
}

RStartMenu::RStartMenu(REngine *eng) : RGameMenu(eng)
{
  // Default values for players
  m_numPlayers = 4;
  m_fm = new RFontManager("../fonts/VeraBI.ttf", 20);
  RKeyboardSystem *sys = new RKeyboardSystem(eng);
  m_inputSystem = sys;
  RResources::AddInputAction(101,new RStartMenuSelectNext(m_eng, this));
  RResources::AddInputAction(102,new RStartMenuSelectPrevious(m_eng, this));
  RResources::AddInputAction(103,new RStartMenuNextOption(m_eng, this));
  RResources::AddInputAction(104,new RStartMenuPreviousOption(m_eng, this));
  RResources::AddInputAction(105,new RStartMenuStartGame(m_eng, this));
  RResources::AddInputAction(106,new RMenuQuit(m_eng));
  sys->AddKeyAction(SDLK_UP, 101);
  sys->AddKeyAction(SDLK_DOWN, 102);
  sys->AddKeyAction(SDLK_RIGHT, 103);
  sys->AddKeyAction(SDLK_LEFT, 104);
  sys->AddKeyAction(SDLK_RETURN, 105);
  sys->AddKeyAction(SDLK_ESCAPE, 106);
  playersList.push_front(FOUR);
  playersList.push_front(THREE);
  playersList.push_front(TWO);
  playersList.push_front(ONE);
  // Generate the level list
  // Read in the file
  ifstream infile;
  string filePath = m_eng->getLevelsDir()+LEVEL_LIST_FILE;
  infile.open(filePath.c_str());
  if (!infile) {
    cerr << "Could not open level " + filePath << endl;
    exit(1);
  }
  while(true) {
    levelPair pair;
    string level, file;

    getline(infile,level,',');
    if(infile.fail()) break;
    pair.levelName = level;

    getline(infile,file,'\n');
    if(infile.fail()) break;
    TrimString(file);
    pair.fileName = file;

    levelList.push_back(pair);
  }
  infile.close();

  currentMode = NUM_PLAYER_SELECT;
}

RStartMenu::~RStartMenu()
{
  delete m_fm;
}

void RStartMenu::DrawStartMenu(GLdouble screenWidth, GLdouble screenHeight)
{
  SDL_Color color;
  float x, y;
  x = (short int)(screenWidth*0.5);
  y = (short int)(screenHeight*0.66);
  color.r = 255;
  color.g = 255;
  color.b = 255;
  glClearColor(0.0,0.0,0.0,0.0);
  glClearDepth(1.0);
  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
  glViewport(0,0,(int)screenWidth,(int)screenHeight);

  // Draw the background
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  int viewport[4];

  glGetIntegerv(GL_VIEWPORT, viewport);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, viewport[2], 0, viewport[3]);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  //GLuint texture = RResources::GetTexture(-10);
  MD2Texture img = RResources::GetFullTexture(-10);
  GLuint texture = img.tex;
  glBindTexture(GL_TEXTURE_2D, texture);

  float scale = .6;
  float finalHeight = img.height*scale;;
  float finalWidth = finalHeight/((float)img.height) * img.width;
  float bottomMargin = img.height*.1;

  float top_y = bottomMargin+finalHeight;
  float bottom_y = bottomMargin;
  float right_x = screenWidth;
  float left_x = screenWidth-finalWidth;

  glBegin(GL_LINES);
  glColor4f(1.0, 1.0, 0, 1.0);
  glVertex2f(0, top_y);
  glVertex2f(right_x, top_y);
  glVertex2f(0, bottom_y);
  glVertex2f(right_x, bottom_y);
  glEnd();

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  glBegin(GL_QUADS);
  glTexCoord2f(0,0);
  glVertex2f(left_x, top_y);

  glTexCoord2f(0,1);
  glVertex2f(left_x, bottom_y);

  glTexCoord2f(1,1);
  glVertex2f(right_x, bottom_y);

  glTexCoord2f(1,0);
  glVertex2f(right_x, top_y);
  glEnd();
  glDisable(GL_TEXTURE_2D);

  glPopAttrib();
  glBlendFunc(GL_ONE,GL_ONE);
  glEnable(GL_BLEND);
  // Draw the title
  m_fm->SetSize(100);
  int width = m_fm->GetTextWidth("Formula Inverse");
  m_fm->WriteText("Formula Inverse", color, x-width/2,top_y);

  int OPTION_X = 100;
  // Draw the first option
  if ( currentMode == NUM_PLAYER_SELECT ) {
    color.r = 255;
    color.g = 0;
    color.b = 0;
  }
  else {
    color.r = 255;
    color.g = 255;
    color.b = 255;
  }

  int tagSize = 10;
  int tagOffset = 30;
  float OPTION_Y = screenHeight*.55;
  SDL_Color tags = { 255, 255, 0 };
  m_fm->SetSize(tagSize);
  m_fm->WriteText("Players", tags, OPTION_X, OPTION_Y+tagOffset);
  m_fm->SetSize(20);
  switch (playersList.front()) {
  case ONE:
    m_numPlayers = 1;
    width = m_fm->GetTextWidth("1 Player");
    m_fm->WriteText("1 Player", color, OPTION_X,OPTION_Y);
    break;
  case TWO:
    m_numPlayers = 2;
    width = m_fm->GetTextWidth("2 Player");
    m_fm->WriteText("2 Player", color, OPTION_X,OPTION_Y);
    break;
  case THREE:
    m_numPlayers = 3;
    width = m_fm->GetTextWidth("3 Player");
    m_fm->WriteText("3 Player", color, OPTION_X,OPTION_Y);
    break;
  case FOUR:
    m_numPlayers = 4;
    width = m_fm->GetTextWidth("4 Player");
    m_fm->WriteText("4 Player", color, OPTION_X,OPTION_Y);
    break;
  default:
    cerr << "Invalid value at front of number of players list" << endl;
    exit(1);
  }
  if ( currentMode == NUM_PLAYER_SELECT ) {
    color.r = 255;
    color.g = 255;
    color.b = 255;
  }
  else {
    color.r = 255;
    color.g = 0;
    color.b = 0;
  }
  OPTION_Y -= 50;

  m_fm->SetSize(tagSize);
  m_fm->WriteText("Level", tags, OPTION_X, OPTION_Y+tagOffset);
  m_fm->SetSize(20);
  width = m_fm->GetTextWidth((char*)levelList.front().levelName.c_str());
  m_fm->WriteText((char*)levelList.front().levelName.c_str(), color, OPTION_X,OPTION_Y);

  // Draw the background for the scores
  float score_leftx = screenWidth*.5;
  float score_topy = screenHeight*.5;
  float score_rightx = screenWidth*.95;
  float score_bottomy = screenHeight*.2;

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glBegin(GL_QUADS);
  glColor4f(1.0, 1.0, 1.0, 0.1);
  glVertex2f(score_leftx, score_topy);
  glVertex2f(score_leftx, score_bottomy);
  glVertex2f(score_rightx, score_bottomy);
  glVertex2f(score_rightx, score_topy);
  glEnd();

  glDisable(GL_BLEND);

  // Draw the high scores for the level

  char scoreTitle[20] = "Course Records";
  float scoreTitleposx = (score_leftx+score_rightx)/2 -
    m_fm->GetTextWidth(scoreTitle)/2;
  float scoreTitleposy = score_topy -
    m_fm->GetTextHeight(scoreTitle) - 20;
  float timeInfox = m_fm->GetTextWidth("Best Course Time:")+30;
  color.r = 0; color.g = 255; color.b = 0;
  m_fm->WriteText(scoreTitle, color, scoreTitleposx, scoreTitleposy);
  LevelScoreInfo scores =
    m_eng->GetSingleLevelScores(levelList.front().fileName);

  float scoreTextLocx = score_leftx+75;
  float scoreTextLocy = score_bottomy+100;

  char laptime[50];
  char racetime[50];
  int minutes;
  int seconds;
  int milliseconds;
  int temp;
  minutes = scores.bestLapTime/60000;
  temp = scores.bestLapTime%60000;
  seconds = temp/1000;
  milliseconds = temp%1000;
  sprintf(laptime, "Best Lap Time:");
  m_fm->SetSize(20);
  color.r = 255; color.g = 0; color.b = 0;
  m_fm->WriteText(laptime, color, scoreTextLocx, scoreTextLocy);
  if (scores.bestLapTime < 0)
    sprintf(laptime, "-- : -- : ---", 
	    minutes, seconds,  milliseconds);
  else    
    sprintf(laptime, "%02d : %02d : %03d", minutes, seconds,  milliseconds);
  color.r = 255; color.g = 255; color.b = 255;
  m_fm->WriteText(laptime, color, scoreTextLocx+timeInfox, scoreTextLocy);
  minutes = scores.bestRaceTime/60000;
  temp = scores.bestRaceTime%60000;
  seconds = temp/1000;
  milliseconds = temp%1000;
  sprintf(racetime, "Best Course Time:");
  color.r = 255; color.g = 0; color.b = 0;
  m_fm->WriteText(racetime, color, scoreTextLocx, scoreTextLocy+50);
  if (scores.bestRaceTime < 0)
    sprintf(racetime, "-- : -- : ---", 
	    minutes, seconds, milliseconds);
  else
    sprintf(racetime, "%02d : %02d : %03d", minutes, seconds, milliseconds);


  color.r = 255; color.g = 255; color.b = 255;
  m_fm->WriteText(racetime, color, scoreTextLocx+timeInfox, scoreTextLocy+50);

  glDisable(GL_BLEND);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();

  SDL_GL_SwapBuffers();
}

void RStartMenu::HandleInput()
{
  m_inputSystem->ExecuteInput(0);
}

void RStartMenu::ResetMenu()
{

}

void RStartMenu::NextSelection()
{
  if (currentMode == NUM_PLAYER_SELECT)
    currentMode = LEVEL_SELECT;
  else if (currentMode == LEVEL_SELECT)
    currentMode = NUM_PLAYER_SELECT;
  else {
    cerr << "RStartMenu::NextSelection() -- Invalid mode" << endl;
    exit(1);
  }
}

void RStartMenu::PreviousSelection()
{
  if (currentMode == NUM_PLAYER_SELECT)
    currentMode = LEVEL_SELECT;
  else if (currentMode == LEVEL_SELECT)
    currentMode = NUM_PLAYER_SELECT;
  else {
    cerr << "RStartMenu::PreviousSelection() -- Invalid mode" << endl;
    exit(1);
  }
}

void RStartMenu::NextOption()
{
  if (currentMode == NUM_PLAYER_SELECT) {
    NUM_PLAYERS tmp = playersList.front();
    playersList.pop_front();
    playersList.push_back(tmp);
  }
  else if (currentMode == LEVEL_SELECT) {
    levelPair tmp = levelList.front();
    levelList.pop_front();
    levelList.push_back(tmp);
  }
  else {
    cerr << "RStartMenu::NextOption() -- Invalid mode" << endl;
    exit(1);
  }
}

void RStartMenu::PreviousOption()
{
  if (currentMode == NUM_PLAYER_SELECT) {
    NUM_PLAYERS tmp = playersList.back();
    playersList.pop_back();
    playersList.push_front(tmp);
  }
  else if (currentMode == LEVEL_SELECT) {
    levelPair tmp = levelList.back();
    levelList.pop_back();
    levelList.push_front(tmp);
  }
  else {
    cerr << "RStartMenu::PreviousOption() -- Invalid mode" << endl;
    exit(1);
  }
}

