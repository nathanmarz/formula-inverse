#include <Framework.h>
#include <SDL.h>
#include <iostream>
#include <fstream>
#include "RGameMenu.h"
#include "ROnScreenDisplay.h"
#include "RPhysics.h"
#include <algorithm>


const float MOVEMENT_DELTA = 0.2;
const int FREE_CAMERA_DEFAULT_PLAYER = 0;
#define MAX_BLUR .7

#define SHADOW_MAP_WIDTH 800
#define SHADOW_MAP_HEIGHT 600


//
// Inputs for the free floating camera
//
class RInputW:public RGameAction {
public:
    RInputW(REngine *r):RGameAction(r) { }
    void KeyDown(int deltaTime) {
      m_engine->GetCamera(FREE_CAMERA_DEFAULT_PLAYER)->IncreaseSpeed();
      
    }
};

class RInputS:public RGameAction {
public:
    RInputS(REngine *r):RGameAction(r) { }
    void KeyDown(int deltaTime) {
        m_engine->GetCamera(FREE_CAMERA_DEFAULT_PLAYER)->DecreaseSpeed();
    }
};

class RInputA:public RGameAction {
public:
    RInputA(REngine *r):RGameAction(r) { }
    void KeyDown(int deltaTime) {
        m_engine->GetCamera(FREE_CAMERA_DEFAULT_PLAYER)->Rotate(-5);
    }
};

class RInputD:public RGameAction {
public:
    RInputD(REngine *r):RGameAction(r) { }
    void KeyDown(int deltaTime) {
        m_engine->GetCamera(FREE_CAMERA_DEFAULT_PLAYER)->Rotate(5);
    }
};

class RInputV:public RGameAction {
public:
    RInputV(REngine *r):RGameAction(r) { }
    void KeyPressed(int deltaTime) {
        m_engine->GetCamera(FREE_CAMERA_DEFAULT_PLAYER)->SwitchViewpoint();
    }
};

class RFreeCameraEnable : public RGameAction {
public:
  RFreeCameraEnable(REngine *r) : RGameAction(r) { }
  void KeyPressed(int deltaTime) {
    m_engine->GetCamera(FREE_CAMERA_DEFAULT_PLAYER)->ToggleFreeCamera();
  }
};

class RCameraDown:public RGameAction {
public:
  RCameraDown(REngine *r):RGameAction(r) { }
  void KeyDown(int deltaTime) {
    m_engine->GetCamera(FREE_CAMERA_DEFAULT_PLAYER)->GoDown();
  }
};

class RCameraUp:public RGameAction {
public:
  RCameraUp(REngine *r):RGameAction(r) { }
  void KeyDown(int deltaTime) {
    m_engine->GetCamera(FREE_CAMERA_DEFAULT_PLAYER)->GoUp();
  }
};

//
// Inputs for the players
//
class RMoveForward : public RGameAction {
public:
  RMoveForward(REngine *r, int playerId):RGameAction(r)
  { m_playerId = playerId; }

  void KeyDown(int deltaTime) {
    RPlayer *p = m_engine->GetPlayer(m_playerId);
    p->physicsEngine->HitPedal();
    //Vector4 newAhead;
    //p->Move(MOVEMENT_DELTA, p->GetAhead(), newAhead);
    //p->SetAhead(newAhead);
    
  }
  
  void KeyUp(int deltaTime) {
    RPlayer *p = m_engine->GetPlayer(m_playerId);
    p->physicsEngine->ReleasePedal();
  }
protected:
  int m_playerId;
};

class RBrakeAction:public RGameAction {
public:

  RBrakeAction(REngine *r, int playerId):RGameAction(r) 
  { m_playerId = playerId; }
  
  void KeyDown(int deltaTime) {
    RPlayer *p = m_engine->GetPlayer(m_playerId);
    Vector4 newAhead;
    p->physicsEngine->HitBrake();
  }
  
  void KeyUp(int deltaTime) {
    RPlayer *p = m_engine->GetPlayer(m_playerId);
    Vector4 newAhead;
    p->physicsEngine->ReleaseBrake();
  }

protected:
  int m_playerId;
};

class RBoostAction:public RGameAction {
public:

  RBoostAction(REngine *r, int playerId):RGameAction(r) 
  { m_playerId = playerId; }
  
  void KeyPressed(int deltaTime) {
    RPlayer *p = m_engine->GetPlayer(m_playerId);
    p->physicsEngine->DoBoost();
  }
protected:
  int m_playerId;
};


class RPause:public RGameAction {
public:
    RPause(REngine *e):RGameAction(e) { }

    void KeyPressed(int deltaTime) {
        m_engine->ChangePauseState();
    }

};

class RRotate:public RGameAction {
public:
    RRotate(REngine *r, float degrees, int playerId):RGameAction(r) 
    { m_deg = degrees; m_playerId = playerId; }
    
    void KeyDown(int deltaTime) {
        RPlayer *p = m_engine->GetPlayer(m_playerId);
        p->Rotate(m_deg);
    }
    
    void KeyUp(int deltaTime) {
        RPlayer *p = m_engine->GetPlayer(m_playerId);
        p->Rotate(0);
    }
    
protected:
  int m_playerId;
  float m_deg;
};

class RGotoMenu : public RGameAction {
public:
    RGotoMenu(REngine *r) : RGameAction(r) 
    { }
    
    void KeyPressed(int deltaTime) {
      m_engine->GotoStartMenu();
    }
};
/*
class RSceneryTestAhead: public RGameAction {
public:
    RSceneryTestAhead(REngine *eng, RScenery *test):RGameAction(eng) {
        m_scenery = test;
    }
    void KeyDown(int deltaTime) {
        m_scenery->RotateAroundAhead(5);
    }

private:
    RScenery *m_scenery;
};

class RSceneryTestUp: public RGameAction {
public:
    RSceneryTestUp(REngine *eng, RScenery *test):RGameAction(eng) {
        m_scenery = test;
    }
    void KeyDown(int deltaTime) {
        m_scenery->RotateAroundUp(5);
    }

private:
    RScenery *m_scenery;
};
*/


void REngine::InitPerspective(int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOVY,
		   1.0*w/h,
		   NEAR_DIST,
		   FAR_DIST);
    glMatrixMode(GL_MODELVIEW);
}

void REngine::ChangePauseState() {
    m_paused = !m_paused;
    if(m_paused) {
        cerr<<"capturing image"<<endl;
        //glAccum(GL_LOAD,1.0);
    }
}


bool ParseLevelLine(ifstream &infile, pair<string,string> &ret) {
    getline(infile,ret.first,',');
    if(infile.fail()) return false;
    getline(infile,ret.second,'\n');
    if(infile.fail()) return false;
    return true;
}


pair<bool,bool> REngine::UpdateLevelScores(string levelName, int bestLapTime, int bestRaceTime) {
    pair<bool,bool> ret;
    ret.first = false;
    ret.second = false;
    LevelScoreInfo currScores = GetSingleLevelScores(levelName);
    LevelScoreInfo toWrite = currScores;
    if(currScores.bestLapTime==-1||bestLapTime<currScores.bestLapTime) {
        toWrite.bestLapTime = bestLapTime;
        ret.first = true;
    }
    if(currScores.bestRaceTime==-1||bestRaceTime<currScores.bestRaceTime) {
        toWrite.bestRaceTime = bestRaceTime;
        ret.second = true;
    }
    WriteLevelScores(levelName,toWrite);
    return ret;
    
}

void REngine::WriteLevelScores(string &levelName, LevelScoreInfo toWrite) {
    string scoreFile = getLevelsDir()+levelName+SCORES_SUFFIX;
    ofstream outfile;
    outfile.open(scoreFile.c_str());
    outfile<<toWrite.bestLapTime<<" "<<toWrite.bestRaceTime<<endl;
    outfile.close();
}

LevelScoreInfo REngine::GetSingleLevelScores(string &levelName) {
    LevelScoreInfo ret;
    string levelScoreFile = getLevelsDir()+levelName+SCORES_SUFFIX;
    ifstream infile;
    infile.open(levelScoreFile.c_str());
    string info;
    getline(infile,info);
    if(infile.fail()) {
        ret.bestLapTime = -1;
        ret.bestRaceTime = -1;
    } else {
        vector<string> times = SplitString(info);
        ret.bestLapTime = atoi(times[0].c_str());
        ret.bestRaceTime = atoi(times[1].c_str());
    }

    //cerr<<ret.bestLapTime<<endl;
    //cerr<<ret.bestRaceTime<<endl;
    infile.close();
    return ret;
}

list<pair<string,LevelScoreInfo> > REngine::GetLevelScores() {
    string levelListFile = getLevelsDir()+LEVEL_LIST_FILE;
    ifstream infile;
    infile.open(levelListFile.c_str());
    list<pair<string,LevelScoreInfo> > ret;
    while(true) {
        pair<string,string> levelInfo;
        if(!ParseLevelLine(infile,levelInfo)) break;
        pair<string,LevelScoreInfo> toPushBack;
        toPushBack.first = levelInfo.first;
        toPushBack.second = GetSingleLevelScores(levelInfo.second);
        ret.push_back(toPushBack);
    }
    infile.close();
    return ret;
}

void REngine::ShowLoadingScreen() {
    glClear(GL_COLOR_BUFFER_BIT);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
	//cerr<<"aa"<<endl;
    RResources::AddTexture(-100,m_resourceRoot+TEXTURE_DIR+"loading.jpg");

    
    int viewport[4];
    
    glGetIntegerv(GL_VIEWPORT, viewport);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, viewport[2], 0, viewport[3]);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    MD2Texture img = RResources::GetFullTexture(-100);
    GLuint texture = img.tex;
    glBindTexture(GL_TEXTURE_2D, texture);
    
    float scale = .6;
    float finalHeight = img.height*scale;
    float finalWidth = finalHeight/((float)img.height) * img.width;
    float bottomMargin = img.height*.1-50;
    
    float top_y = bottomMargin+finalHeight;
    float bottom_y = bottomMargin;
    float right_x = m_screenWidth;
    float left_x = m_screenWidth-finalWidth;
    
    
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
    glPopAttrib();
    SDL_GL_SwapBuffers();
}

REngine::REngine(string windowName, bool fullScreen, string resourcesRoot, int w, int h, int frameRate) {
    cerr<<"a"<<endl;
	m_screenHeight = h;
    m_screenWidth = w;
    m_resourceRoot = resourcesRoot;
    //cerr<<SHADOW_MAP_WIDTH<<
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE);
    //SDL_Init(SDL_INIT_EVERYTHING);
    atexit(SDL_Quit);
    int flags = SDL_OPENGL|SDL_GL_DOUBLEBUFFER;
    if(fullScreen) flags = flags | SDL_FULLSCREEN;
    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE,16);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE,16);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE,16);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE,16);
	cerr<<"b"<<endl;
    SDL_SetVideoMode(w,h,32,flags);
    SDL_WM_SetCaption(windowName.c_str(),windowName.c_str());
    SDL_ShowCursor(0);
    //CheckOpenGLErrors(-5);
	cerr<<"c"<<endl;
    //glClearAccum(0,0,0,1);
    //glClear(GL_ACCUM_BUFFER_BIT);
    
  
    //CheckOpenGLErrors(-4);
    ShowLoadingScreen();
	cerr<<"d"<<endl;
    LoadTextures();
	//while(true);
    //CheckOpenGLErrors(-6);
    cerr<<"e"<<endl;
	LoadPieceShells();
	cerr<<"f"<<endl;
    LoadInputs();
    
    cerr<<"f"<<endl;
    //InitPerspective(w,h);
    m_paused = false;
    m_mode = STARTMENU;
    m_targetFrameMS = (int) 1000/frameRate; //ms per frame

    cerr<<"g"<<endl;

    for (int i = 0; i < MAX_PLAYERS; i++) {
      m_camera[i] = NULL;
      m_display[i] = NULL;
    }
	cerr<<"q"<<endl;
}

void REngine::LoadInputs() {
    const int rotAmt = 30;
    RResources::AddInputAction(901,new RPause(this));
    
    RResources::AddInputAction(41,new RMoveForward(this, 3));
    RResources::AddInputAction(42,new RBrakeAction(this, 3));
    RResources::AddInputAction(43,new RRotate(this, rotAmt, 3));
    RResources::AddInputAction(44,new RRotate(this, -rotAmt, 3));
    RResources::AddInputAction(46,new RBoostAction(this, 3));

    RResources::AddInputAction(31,new RMoveForward(this, 2));
    RResources::AddInputAction(32,new RBrakeAction(this, 2));
    RResources::AddInputAction(33,new RRotate(this, rotAmt, 2));
    RResources::AddInputAction(34,new RRotate(this, -rotAmt, 2));
    RResources::AddInputAction(36,new RBoostAction(this, 2));
    
    RResources::AddInputAction(21,new RMoveForward(this, 1));
    RResources::AddInputAction(22,new RBrakeAction(this, 1));
    RResources::AddInputAction(23,new RRotate(this, rotAmt, 1));
    RResources::AddInputAction(24,new RRotate(this, -rotAmt, 1));
    RResources::AddInputAction(26,new RBoostAction(this, 1));

    RResources::AddInputAction(11,new RMoveForward(this, 0));
    RResources::AddInputAction(12,new RBrakeAction(this, 0));
    RResources::AddInputAction(13,new RRotate(this, rotAmt, 0));
    RResources::AddInputAction(14,new RRotate(this, -rotAmt, 0));
    RResources::AddInputAction(16,new RBoostAction(this, 0));

    RResources::AddInputAction(15, new RGotoMenu(this));
    RResources::AddInputAction(1,new RInputW(this));
    RResources::AddInputAction(2,new RInputS(this));  
    RResources::AddInputAction(3,new RInputA(this));
    RResources::AddInputAction(4,new RInputD(this));
    RResources::AddInputAction(5,new RCameraUp(this));
    RResources::AddInputAction(6,new RCameraDown(this));
    RResources::AddInputAction(7,new RFreeCameraEnable(this));  
}



REngine::~REngine() {
    //delete m_inputSystem;
    //delete m_world;
    //delete m_camera;
    RResources::FreeResources();
}


void REngine::LoadTextures() {
    string texturesPath = m_resourceRoot+TEXTURE_DIR;
    string listFilePath = texturesPath+TEXTURE_LIST_FILE;
    //cerr<<listFilePath<<endl;
    ifstream texList;
    texList.open(listFilePath.c_str());
    if(!texList) {
        cerr<<"Could not open texture list file."<<endl;
        exit(1);
    }
    while(true) {
		//cerr<<"whee"<<endl;
        string curLine;
        getline(texList,curLine);
        if(texList.fail()) break;
        vector<string> idAndFilePath = SplitString(curLine);
        if(idAndFilePath.size()!=2) {
            cerr<<"Improperly formatted texture list file."<<endl;
            exit(1);
        }
        int id = atoi(idAndFilePath[0].c_str());
        string filePath = texturesPath+idAndFilePath[1];
        RResources::AddTexture(id,filePath);
        
    }
    texList.close();
}

void REngine::LoadPieceShells() {
    string piecePath = m_resourceRoot + PIECE_SHELLS_DIR;
    string pieceFilePath = piecePath + PIECE_SHELLS_LIST_FILE;
    ifstream pieceList;
    pieceList.open(pieceFilePath.c_str());
    if(!pieceList) {
        cerr<<"Failed to open piece list file."<<endl;
        exit(1);
    }
    cerr<<"aaa2"<<endl;
    while(true) {
	cerr<<"aaa"<<endl;
        string curLine;
        getline(pieceList,curLine);
        if(pieceList.fail()) break;
        vector<string> idAndFilePath = SplitString(curLine);
        if(idAndFilePath.size()!=2) {
            cerr<<"Improperly formatted piece list file."<<endl;
            exit(1);
        }
	cerr<<"aaa3"<<endl;
        int id = atoi(idAndFilePath[0].c_str());
        string filePath = piecePath+idAndFilePath[1];
	cerr<<"aaa4"<<endl;
        RPieceShell *rps = LoadPieceShell(filePath);
  
        RResources::AddPieceShell(id,rps);
  
        
    }

    pieceList.close();
}

void ReadTexDim(string &dimline, int &texWidth, int &texHeight) {
    vector<string> dimSplit = SplitString(dimline);
    if(dimSplit[0]=="texWidth") {
        texWidth = atoi(dimSplit[1].c_str());
    } else if(dimSplit[0]=="texHeight") {
        texHeight = atoi(dimSplit[1].c_str());
    } else {
        cerr<<"bad texture dimentsion input"<<endl;
        exit(1);
    }
}

bool ReadInEdgeShell(ifstream &file, REdgeShell &ret) {
    vector<string> data;
    if(ReadFileUntilTerminateLine(file,"end",data)) {
        bool loopsBack = data[0]=="loop";
        vector<Vector4> vertices;
        for(int i=1;i<data.size();i++) {
            vector<string> nums = SplitString(data[i]);
            Vector4 v(atof(nums[0].c_str()),atof(nums[1].c_str()),atof(nums[2].c_str()));
            vertices.push_back(v);
        }
        if(!loopsBack&&vertices.size()==2) {
            ret = REdgeShell(vertices[0],vertices[1]);
        } else {
            ret = REdgeShell(vertices,loopsBack);
        }
        return true;
    } else return false;
}

void ReadHermite(string &line,float &hermite) {
    vector<string> lineSplit = SplitString(line);
    if(lineSplit.size()==1) hermite=DEFAULT_HERMITE;
    else {
        hermite = atof(lineSplit[1].c_str());
        
    }
}


RPieceShell *REngine::LoadPieceShell(const string &filePath) {
    ifstream pieceFile;
    cerr << "Loading " << filePath <<endl;
    pieceFile.open(filePath.c_str());
    if(!pieceFile) {
	cerr<<filePath.length()<<endl;
        cerr<<"Failed to open piece file: "<< filePath << "!" << endl;
        exit(1);
    }
    cerr << "aaa6" <<endl;
    int texWidth=-1, texHeight=-1;
    float hermite;
    string texDim;
    getline(pieceFile,texDim);
    ReadHermite(texDim,hermite);
    cerr << "aaa7" <<endl;
    getline(pieceFile,texDim);
    ReadTexDim(texDim,texWidth,texHeight);
    getline(pieceFile,texDim);
    ReadTexDim(texDim,texWidth,texHeight);
    if(texWidth==-1||texHeight==-1) {
        cerr<<"Must specify texture width and height for piece shell"<<endl;
        exit(1);
    }
    cerr << "aaa8" <<endl;

    RPieceShell *rps = new RPieceShell(texWidth,texHeight,hermite);
    cerr << "aaa9" <<endl;
    
    while(true) {
        REdgeShell nextShell;
        if(!ReadInEdgeShell(pieceFile,nextShell)) break;
        rps->push_back(nextShell);
    }

    pieceFile.close();
    cerr << "aaa10" <<endl;
    rps->finalize();
    /*
    for(int i=0;i<rps->size();i++) {
        REdgeShell edge = (*rps)[i];
        cerr<<edge.LoopsBack()<<endl;
        edge.DumpDefinePoints();
    }
    */
    cerr << "aaa11" <<endl;

    
    return rps;
}

void REngine::OutputCountDown(int playerId, int deltaTime)
{
  m_display[playerId]->OutputCountDown(deltaTime);
}

void REngine::GameCycle(int deltaTime) {
    //cerr<<deltaTime<<endl;
    //cerr<<"accumulating"<<endl;
  if (RaceStarted()) {
    CheckInput(deltaTime);
  }
  if(m_paused) {
    //glAccum(GL_RETURN,1.0);
      SDL_GL_SwapBuffers();
      return;
  }
  
  //glAccum(GL_LOAD,1);
    //cerr<<"done"<<endl;
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //move to world->Prepare
  //CheckOpenGLErrors(1);
  //glEnable(GL_SCISSOR_TEST);
  //glScissor(0,0,600,600);
  
  list<RPlayer *> allPlayers;
  for(int i=0;i<m_numPlayers;i++) {
      allPlayers.push_back(GetPlayer(i));
  }
  
  for(int i=0;i<m_numPlayers;i++) {
      RPlayer *p = GetPlayer(i);
      float lastDLong;
      float lastDLat;
      Vector4 newAhead;
      Vector4 realDir;
      if(RaceStarted())
          p->physicsEngine->DoPhysics(deltaTime);
       p->FinishTurn(allPlayers,deltaTime);
  }
  
  

  //CheckOpenGLErrors(2);
  
  

  RWorld *world = GetWorld();
  
  world->Animate(deltaTime);
  world->Prepare();

  //CheckOpenGLErrors(21);  
  vector<MotionBlurInfo> blurInfo = GetMotionOrdering();
  for(vector<MotionBlurInfo>::iterator it = blurInfo.begin();it!=blurInfo.end();++it) {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      MotionBlurInfo mbi = *it;
      GLfloat accumAmt;
      
      accumAmt = mbi.totalBlurAmt;
      glEnable(GL_SCISSOR_TEST);
      RCamera *camera = GetCamera(mbi.id);
      camera->SetViewport();
      camera->Animate(deltaTime);
      //glAccum(GL_MULT,accumAmt);
      
      //CheckOpenGLErrors(22);  
      world->Draw(camera);
      //CheckOpenGLErrors(23);  

      //glAccum(GL_ACCUM,1-mbi.totalBlurAmt);
      //glAccum(GL_RETURN,1.0);
      glDisable(GL_SCISSOR_TEST);
      glPopAttrib();
  }

  //CheckOpenGLErrors(20);
  for (int i = 0; i < m_numPlayers; i++) {
    RCamera *camera = GetCamera(i);
    camera->SetViewport();
    camera->Animate(deltaTime);
    world->SetLights();
    for(int j=0;j<m_numPlayers;j++) {
        //if(j!=i) {
        RPlayer *p = GetPlayer(j);
        //cerr<<"rendering player"<<endl;
        p->Render(camera);
            //p->getModel()->RenderBV();
            //}
    }
    DrawPlayerDisplay(i, deltaTime);
    if (!RaceStarted())
        OutputCountDown(i, deltaTime);
    //render players here or put them in bsp tree
    
  }

  //CheckOpenGLErrors(3);
  SDL_GL_SwapBuffers();

}

void REngine::DrawPlayerDisplay(int playerId, int deltaTime)
{
  if (!m_display[playerId]) {
    cerr << "REngine::DrawPlayerDisplay() -- "
	 << "No display created for player: "
	 << endl;
    exit(1);
  }
  
  m_display[playerId]->DrawDisplay(deltaTime);
  if(GetPlayer(playerId)->GoingWrongWay()) {
      SDL_Color color = { 255, 0, 0, 255 };
//       color.r = 255;
//       color.g = 0;
//       color.b = 0;
      RCamera *viewer = GetCamera(playerId);
      float y = (viewer->GetHeight()*0.8);
      float x = (viewer->GetWidth()*0.45);
      //m_fontManager->SetSize((int)(20.0/600.0*viewer->GetHeight()));
      m_world->getFontManager()->WriteText("Wrong way!", color, x,y);
  }
}


bool SortMotionBlurInfo(MotionBlurInfo mbi1, MotionBlurInfo mbi2) {
    return mbi1.totalBlurAmt>mbi2.totalBlurAmt;
}

vector<MotionBlurInfo> REngine::GetMotionOrdering() {
    vector<MotionBlurInfo> ret;
    for(int i=0;i<m_numPlayers;i++) {
        RPlayer *p = GetPlayer(i);
        MotionBlurInfo mbi;
        mbi.id = i;
        float fraction = p->physicsEngine->getSpeed()/p->physicsEngine->getMaxSpeed();
        mbi.totalBlurAmt = fraction*fraction*fraction*pow(MAX_BLUR,1/3);
        if(mbi.totalBlurAmt<.1) mbi.totalBlurAmt = .1;
        if(mbi.totalBlurAmt>MAX_BLUR) mbi.totalBlurAmt = MAX_BLUR;
        ret.push_back(mbi);
    }
    
    std::sort(ret.begin(),ret.end(),&SortMotionBlurInfo);
    return ret;
}


RCamera *REngine::GetCamera(int playerId) 
{
  if (!m_camera[playerId]) {
    cerr << "REngine::GetCamera() -- No camera created for player: " 
	 << playerId << endl;
    exit(1);
  }

  return m_camera[playerId];
}

RPlayer *REngine::GetPlayer(int playerId)
{
  if (!m_player[playerId]) {
    cerr << "REngine::GetPlayer() -- No player created for player: " 
	 << playerId << endl;
    exit(1);
  }

  return m_player[playerId];
}

void REngine::CheckInput(int deltaTime) {
    m_inputSystem->ExecuteInput(deltaTime);
}

void REngine::InitGame(int numPlayers, string &levelFile)
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);
    RWorld *rw = new RWorld(this,levelFile);
    m_currLevel = levelFile;
    if (numPlayers > MAX_PLAYERS || numPlayers < 1) {
        cerr << "REngine::InitGame -- invalid number of players" << endl;
        exit(1);
    }
  //  glClear(GL_COLOR_BUFFER_BIT);
    
  m_numPlayers = numPlayers;
  RKeyboardSystem *input = new RKeyboardSystem(this);
  m_inputSystem = input;
  m_world = rw;
  m_gameLogic = rw->getGameLogic();
  m_lastTime = 0;

  // Create a camera for each player
  for (int i = 0; i < numPlayers; i++) {
    m_camera[i] = new RCamera(this, i, numPlayers);
    m_camera[i]->SetFovy(FOVY);
    m_camera[i]->SetNearDist(NEAR_DIST);
    m_camera[i]->SetFarDist(FAR_DIST);
  }

  assert(m_camera[0] != NULL);

  switch (numPlayers) {
  case 1:
    for (int i = 0; i < numPlayers; i++) {
      m_camera[i]->SetHeight(m_screenHeight);
      m_camera[i]->SetWidth(m_screenWidth);
    }
    break;
  case 2:
    for (int i = 0; i < numPlayers; i++) {
      m_camera[i]->SetHeight(m_screenHeight/2);
      m_camera[i]->SetWidth(m_screenWidth);
    }
    break;
  case 3:
  case 4:
    for (int i = 0; i < numPlayers; i++) {
      m_camera[i]->SetHeight(m_screenHeight/2);
      m_camera[i]->SetWidth(m_screenWidth/2);
    }
    break;
  default:
    cerr << "REngine::InitGame() -- invalid number of players" << endl;
    exit(1);
  }

  // Initialize all the players.
  // This must be done after the world has been initialized
  // and has created its track
  // Once the cameras have been defined, tell the world
  // to initialize its drawing parameters.


  for (int i = 0; i < numPlayers; i++) {
      //    m_player[i] = new RPlayer(this,2,m_world->getTrack()->GetStartJForPlayer(i,numPlayers),m_world->getTrack(), m_world->getGameLogic(),.25,"../models/Car/car1.md2","../models/Car/grassLarge.pcx");
      int texId = -6-i;
      m_player[i] = new RPlayer(this,2,m_world->getTrack()->GetStartJForPlayer(i,numPlayers),m_world->getTrack(), m_world->getGameLogic(),.25,"../models/Car/new_hover_color.md2",RResources::GetFullTexture(texId));
  }

  // Initialize the on screen displays

  for (int i = 0; i < numPlayers; i++) {
    m_display[i] = new ROnScreenDisplay(this, m_camera[i], m_player[i]);
  }

  // Only set the keys if they haven't been set
  switch (numPlayers) {
  case 4:
    // Player 4's Movement keys
    
    
    input->AddKeyAction(SDLK_KP5, 41);
    input->AddKeyAction(SDLK_KP2, 42);
    input->AddKeyAction(SDLK_KP1, 43);
    input->AddKeyAction(SDLK_KP3, 44);
    input->AddKeyAction(SDLK_KP4, 46);
  case 3:
    // Player 3's Movement keys
    
    input->AddKeyAction(SDLK_UP, 31);
    input->AddKeyAction(SDLK_DOWN, 32);
    input->AddKeyAction(SDLK_LEFT, 33);
    input->AddKeyAction(SDLK_RIGHT, 34);
    input->AddKeyAction(SDLK_DELETE, 36);
  case 2:
    // Player 2's Movement keys
    
    input->AddKeyAction('i', 21);
    input->AddKeyAction('k', 22);
    input->AddKeyAction('j', 23);
    input->AddKeyAction('l', 24);
    input->AddKeyAction('u', 26);
  case 1:
    // Player 1's Movement keys
    input->AddKeyAction('w', 11);
    input->AddKeyAction('s', 12);
    input->AddKeyAction('a', 13);
    input->AddKeyAction('d', 14);
    input->AddKeyAction('q', 16);

    // Default control buttons
    
    input->AddKeyAction(SDLK_ESCAPE,15);
    //input->AddKeyAction(' ',901); //FOR PAUSING
    break;
  default:
    cerr << "REngine::InitGame() -- invalid number of players" << endl;
    exit(1);
  }
  // Free floating camera 
 
  input->AddKeyAction(SDLK_KP7,3);
  input->AddKeyAction(SDLK_KP8,2);
  input->AddKeyAction(SDLK_KP9,4);
  input->AddKeyAction(SDLK_KP_DIVIDE,1);
  input->AddKeyAction(SDLK_PAGEUP,5);
  input->AddKeyAction(SDLK_PAGEDOWN,6);
  input->AddKeyAction(SDLK_F12,7);
  
  // makes the game loop goto game mode
  m_mode = INGAME;
  m_raceStarted = false;
}

void REngine::GotoStartMenu()
{
  m_mode = STARTMENU;
}


LevelStats REngine::GetLevelStats() {
    int bestLapId = -1;
    int bestRaceId = -1;
    LevelScoreInfo currScores = GetSingleLevelScores(m_currLevel);
    int bestLapTime = currScores.bestLapTime;
    int bestRaceTime = currScores.bestRaceTime;
    for(int i=0;i<m_numPlayers;i++) {
        RPlayer *p = GetPlayer(i);
        int bLap = p->BestLapTime();
        if(bLap!=-1&&(bestLapTime==-1||bLap<bestLapTime)) {
            bestLapTime = bLap;
            bestLapId = i;
        }
        int bRace = p->TimeToCompleteRace();
        if(bRace!=-1&&(bestRaceTime==-1||bRace<bestRaceTime)) {
            bestRaceTime = bRace;
            bestRaceId = i;
        }
    }
    LevelStats ret;
    ret.bestLapId = bestLapId;
    ret.bestRaceId = bestRaceId;
    ret.bestLapTime = bestLapTime;
    ret.bestRaceTime = bestRaceTime;
    return ret;
}

class RStatsAction:public RInputAction {
public:
    RStatsAction(bool *finished) {
        m_finished = finished;
    }

    void KeyPressed(int deltaTime) {
        *m_finished = true;
    }

private:
    bool *m_finished;
};


void REngine::RenderStats(LevelStats stats, RFontManager *fontMan) {
  SDL_Color color = { 255, 255, 0 };
  int escapeTextSize = 10;

  float statWidth = 450;
  float statHeight = 775;
  float xleft = 10;
  float xright = xleft+statWidth;
  float ybot = 100;
  float ytop = ybot+statHeight;
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  MD2Texture img = RResources::GetFullTexture(-10);
  GLuint texture = img.tex;
  glBindTexture(GL_TEXTURE_2D, texture);

  float scale = .6;
  float finalHeight = img.height*scale;;
  float finalWidth = finalHeight/((float)img.height) * img.width;
  float bottomMargin = img.height*.1;

  float top_y = bottomMargin+finalHeight;
  float bottom_y = bottomMargin;
  float right_x = m_screenWidth;
  float left_x = m_screenWidth-finalWidth;

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
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glBegin(GL_QUADS);
  glColor4f(1.0, 1.0, 1.0, 0.1);
  glVertex2f(xleft, ytop);
  glVertex2f(xleft, ybot);
  glVertex2f(xright, ybot);
  glVertex2f(xright, ytop);
  glEnd();
  glDisable(GL_BLEND);
  //stats

  float score_leftx = xleft;
  float score_topy = ytop;
  float score_rightx = xright;
  float score_bottomy = ybot;

  fontMan->SetSize(20);
  char scoreTitle[20] = "Race Results";
  float scoreTitleposx = (score_leftx+score_rightx)/2 -
    fontMan->GetTextWidth(scoreTitle)/2;
  float scoreTitleposy = score_topy -
    fontMan->GetTextHeight(scoreTitle) - 20;
  float timeInfox = fontMan->GetTextWidth("Best Course Time:")+30;
  color.r = 255; color.g = 255; color.b = 255;
  fontMan->WriteText(scoreTitle, color, scoreTitleposx, scoreTitleposy);

  float scoreTextLocx = score_leftx+20;
  float scoreTextLocy = score_topy-200;
  for (int i = 0; i<m_numPlayers; i++, scoreTextLocy-=150) {
    float playerTexty = scoreTextLocy+60;
    color.r = 255; color.g = 255; color.b = 0;
    fontMan->SetSize(10);
    char playerid[10];
    sprintf(playerid,"Player %d",i+1);

    string playerName = playerid;
    if(m_numPlayers>1) {
      playerName+=" (";
      switch(i) {
      case 0:
	if(m_numPlayers==2)
	  playerName+="Top";
	else 
	  playerName+="Upper left";
	break;
      case 1:
	if(m_numPlayers==2)
	  playerName+="Bottom";
	else
	  playerName+="Upper right";
	break;
      case 2:
	playerName+="Lower left";
	break;
      case 3:
	playerName+="Lower right";
	   break;
      }
      playerName+=")";
    }
    

    fontMan->WriteText(playerName.c_str(), color, scoreTextLocx, playerTexty);

    // Check to see if this player Won
    if (GetPlayer(i)->FinishedRace() &&
	GetPlayer(i)->PositionInRace() == 1) {
      // output the winner text
      color.r = 0; color.g = 255; color.b = 0;
      char nameString[30];
      strcpy(nameString, playerName.c_str());
      fontMan->WriteText("- WINNER!", color, 
			 scoreTextLocx+fontMan->GetTextWidth(nameString)+8, 
			 playerTexty);
    }
    if (stats.bestLapId == i || stats.bestRaceId == i) {
      color.r = 0; color.g = 255; color.b = 0;
      // Write "new record" above the course time
      fontMan->SetSize(10);
      fontMan->WriteText("NEW RECORD!", color, scoreTextLocx+timeInfox,
			 playerTexty);
    }
    // Write out the best lap time
    char laptime[50];
    char racetime[50];
    int minutes;
    int seconds;
    int milliseconds;
    int temp;
    int bestLapTime = GetPlayer(i)->BestLapTime();
    minutes = bestLapTime/60000;
    temp = bestLapTime%60000;
    seconds = temp/1000;
    milliseconds = temp%1000;
    sprintf(laptime, "Best Lap Time:");
    fontMan->SetSize(20);
    color.r = 255; color.g = 0; color.b = 0;
    fontMan->WriteText(laptime, color, scoreTextLocx, scoreTextLocy);
    if (bestLapTime < 0)
      sprintf(laptime, "-- : -- : ---", 
	      minutes, seconds,  milliseconds);
    else    
      sprintf(laptime, "%02d : %02d : %03d", minutes, seconds,
	      milliseconds);

    // Check to see if this player set any records
    if (stats.bestLapId == i) {
      color.r = 0; color.g = 255; color.b = 0;
    }
    else {
      color.r = 255; color.g = 255; color.b = 255;      
    }

    fontMan->WriteText(laptime, color, scoreTextLocx+timeInfox, scoreTextLocy);
    
    // Write out the Course Time
    int bestRaceTime = GetPlayer(i)->TimeToCompleteRace();
    minutes = bestRaceTime/60000;
    temp = bestRaceTime%60000;
    seconds = temp/1000;
    milliseconds = temp%1000;

    sprintf(racetime, "Course Time:");
    color.r = 255; color.g = 0; color.b = 0;
    fontMan->WriteText(racetime, color, scoreTextLocx, scoreTextLocy+30);
    if (bestRaceTime < 0)
      sprintf(racetime, "-- : -- : ---", 
	      minutes, seconds, milliseconds);
    else
      sprintf(racetime, "%02d : %02d : %03d", minutes, seconds, milliseconds);
    
    if (stats.bestRaceId == i) {
      color.r = 0; color.g = 255; color.b = 0;
    }
    else {
      color.r = 255; color.g = 255; color.b = 255;      
    }

    fontMan->WriteText(racetime, color, scoreTextLocx+timeInfox,
		       scoreTextLocy+30);
    
  }
  color.r = 255; color.g = 255; color.b = 255;
  fontMan->SetSize(escapeTextSize);
  fontMan->WriteText("Press <Esc> to Exit to Main Menu...",
		     color, xleft, ybot);
}

void REngine::ShowStats(LevelStats &stats) {
//     glAccum(GL_LOAD,1.0);
//     for(int i=0;i<4000;i++) {
        
//     }
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,(int)m_screenWidth,(int)m_screenHeight);
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
    RKeyboardSystem *inputSystem = new RKeyboardSystem(this);
    bool finished = false;
    RStatsAction rsa(&finished);
    inputSystem->AddKeyAction(SDLK_ESCAPE,&rsa);
    RFontManager *fontMan = new RFontManager("../fonts/VeraBI.ttf",10);
    while(!finished) {
        RenderStats(stats, fontMan);
        inputSystem->ExecuteInput(25);
	SDL_GL_SwapBuffers();
        SDL_Delay(20);
    }
    delete inputSystem;
    delete fontMan;
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
}

void REngine::ExitGame() 
{
  glPopAttrib();
    LevelStats stats = GetLevelStats();
    //cerr<<"best lap:"<<stats.bestLapId<<endl;
    //cerr<<"best race:"<<stats.bestRaceId<<endl;
    UpdateLevelScores(m_currLevel,stats.bestLapTime,stats.bestRaceTime);
    ShowStats(stats);
    for (int i = 0; i < m_numPlayers; i++) {
        delete m_player[i];
        delete m_camera[i]; 
        delete m_display[i];
    }
    delete m_world;
    delete m_inputSystem;
    //RResources::FreeResources();
    //   glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
    //   SDL_GL_SwapBuffers();
    // Delete all the specific resources used by this 
    
}

void REngine::EnterGameLoop() {
  RStartMenu *menu = new RStartMenu(this);
  while (1) {
    if (m_mode == INGAME) {
      int currMS = SDL_GetTicks();
      int delta_ms = currMS-m_lastTime;
      m_lastTime = currMS;
      GameCycle(delta_ms);
      int elapsedMS = SDL_GetTicks();
      int MSleft = m_targetFrameMS-(elapsedMS-currMS);
      if(MSleft>0)
          SDL_Delay(MSleft);
      else {
          //cerr<<"frame rate drop!"<<endl;
      }
      if (m_mode == STARTMENU)
          ExitGame();
    }
    else if (m_mode == STARTMENU) {
      menu->DrawStartMenu(m_screenWidth, m_screenHeight);
      menu->HandleInput();
    }
  }
  delete menu;
}
