#ifndef _RGAMEMENU_H_
#define _RGAMEMENU_H_
#include "GL/gl.h"
#include "RFontManager.h"
#include "Framework.h"

typedef enum { NUM_PLAYER_SELECT, LEVEL_SELECT } START_MENU_SELECTIONS;
typedef enum { ONE = 1, TWO = 2, THREE = 3, FOUR = 4 } NUM_PLAYERS;
//typedef enum { LEVEL_ONE, LEVEL_TWO } LEVEL_ID;

//
// GameMenus handle input differently than the normal game,
// so we have our own input manager
//
class RGameMenu {
 public:
  RGameMenu(REngine *eng);
  ~RGameMenu();
  virtual void HandleInput() = 0;
  virtual void ResetMenu() = 0;

 protected:
  REngine *m_eng;
};

typedef struct {
  string levelName;
  string fileName;
} levelPair;

class RStartMenu : public RGameMenu {
 public:
  RStartMenu(REngine *eng);
  ~RStartMenu();

  void DrawStartMenu(GLdouble screenWidth, GLdouble screenHeight);
  void HandleInput();
  void ResetMenu();

  void NextSelection();
  void PreviousSelection();
  void NextOption();
  void PreviousOption();
  int GetNumPlayers() { return m_numPlayers; }
  const char *GetSelectedLevel(); 

 protected:
  int m_numPlayers;
  //int m_level;

  START_MENU_SELECTIONS currentMode;
  list<NUM_PLAYERS> playersList;
  list<levelPair> levelList;
  RFontManager *m_fm;
  RInputSystem *m_inputSystem;
};
#endif
