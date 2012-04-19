#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "RLibraries.h"
#include <list>
#include "RBSPtree.h"
#include <GL/gl.h>
#include "Level.h"
#include "md2.h"
#include "RObjects.h"
#include "RObjectBase.h"
#include "RFontManager.h"
#include "RPhysics.h"
#include "RGameLogic.h"
#include "RBillBoard.h"
#include "texture.h"
#include <hash_map.h>
//#include "ROnScreenDisplay.h"
//#include "RParticle.h"

using namespace std;

class RParticle;
class RParticleSystem;
class RSparkParticleSystem;
class RBillBoard;
class RScreenBillBoard;
class RWorldBillBoard;
class RResources;
class REngine;
class RInputSystem;
class RKeyboardSystem;
class RInputAction;
class RCamera;
class RWorld;
class RPlayer;
class RCar;
class ROnScreenDisplay;

//perspective and frustum stuff
#define FOVY 50
#define NEAR_DIST .1
#define FAR_DIST 1000




#define TEXTURE_DIR "textures/"
#define LEVELS_DIR "levels/"
#define PIECE_SHELLS_DIR "pieces/"
#define MODELS_DIR "models/"
#define FONTS_DIR "fonts/"
#define SCORES_SUFFIX ".sco"

#define LEVEL_LIST_FILE "levellist.lll"

#define TEXTURE_LIST_FILE "textureList.txl"
#define PIECE_SHELLS_LIST_FILE "pieceList.pcl"




#define MAX_PLAYERS 4

//static class
class RResources {
 public:
    static void AddPieceShell(int id, RPieceShell *resource);
    static void AddInputAction(int id, RInputAction *piece);
    static void AddTexture(int id, const string &fileName);
    
    static RPieceShell *GetPieceShell(int id);
    static RInputAction *GetInputAction(int id);
    static GLuint GetTexture(int id);
    static MD2Texture GetFullTexture(int id);

    
    static void FreeResources();
    
 private:
    static hash_map<int, RInputAction *> m_actions;
    static hash_map<int, RPieceShell *> m_pieces;
    //static hash_map<int, GLuint> m_textures;
    static hash_map<int, MD2Texture> m_fullTextures;
	static hash_map<int, string> m_texturePaths;

	static void DoTextureLoad(int id, const string &fileName);

    template <typename T>
    static void FreeResMap(T &resMap) {
        for(typename T::iterator it = resMap.begin();it!=resMap.end();++it) {
            delete it->second;
        }
    }

    template <typename T>
    static void AddResource(int id,T res, hash_map<int,T> &themap) {
//       if(themap.find(id)!=themap.end()) { 
// 	cerr<<"Resource conflict of id: "<<id<<"."<<endl; 
// 	exit(1); 
//       } 
//       themap[id] = res;

      if(themap.find(id)==themap.end()) { 
	themap[id] = res;
      } 

    }

    template <typename T>
    static T GetResource(int id, hash_map<int,T > &themap) {
        if(themap.find(id)==themap.end()) {
            cerr<<"Could not find resource id: "<<id<<"."<<endl;
	    
            exit(1);
        }
        return themap[id];
    }


};

typedef enum { STARTMENU, INGAME } GAMEMODE;

struct MotionBlurInfo {
    int id;
    float totalBlurAmt;
};

struct LevelScoreInfo {
    int bestLapTime; //in milliseconds
    int bestRaceTime; //in milliseconds
};

struct LevelStats {
    int bestLapId; //-1 if no one broke the high score
    int bestRaceId; //-1 if no one broke the high score

    int bestLapTime;
    int bestRaceTime;
};

class REngine {
 public:
    
    REngine(string windowName, bool fullScreen, string resourcesRoot, int w, int h, int frameRate);
    ~REngine();
    
    void InitGame(int numPlayers, string &levelFile);
    void GotoStartMenu();
    void ExitGame();

    GLdouble GetScreenHeight() { return m_screenHeight; }
    GLdouble GetScreenWidth() { return m_screenWidth; }

    int GetNumPlayers() { return m_numPlayers; }
    void DrawPlayerDisplay(int playerId, int deltaTime);

    void EnterGameLoop();
    virtual RCamera *GetCamera(int playerId);
    virtual RPlayer *GetPlayer(int playerId);
    virtual RWorld *GetWorld() { return m_world; }
    list<pair<string,LevelScoreInfo> > GetLevelScores();
    LevelScoreInfo GetSingleLevelScores(string &levelFile);
    string getLevelsDir() { return m_resourceRoot+LEVELS_DIR; }
    string getFontsDir() { return m_resourceRoot+FONTS_DIR; }
    string getModelsDir() {return m_resourceRoot+MODELS_DIR; }
    bool RaceStarted() { return m_raceStarted; }
    void StartRace() { m_raceStarted = true; m_gameLogic->BeginRace(); }
    void OutputCountDown(int playerId, int deltaTime);

    //first bool is if lap time is now best lap time, second bool is if race time is now best race time
    pair<bool,bool> UpdateLevelScores(string levelName, int bestLapTime, int bestRaceTime);
    void ChangePauseState();

 protected:
    virtual void GameCycle(int deltaTime);
   
    virtual void CheckInput(int deltaTime);


 private:

    void ShowLoadingScreen();
    void RenderStats(LevelStats stats, RFontManager *fontMan);

    void ShowStats(LevelStats &stats);
    LevelStats GetLevelStats();
    void WriteLevelScores(string &levelName, LevelScoreInfo toWrite);
    vector<MotionBlurInfo> GetMotionOrdering();
    void LoadInputs();
    void InitPerspective(int w, int h);
    void LoadTextures();
    void LoadPieceShells();
    void GenerateShadowMap();
    RPieceShell *LoadPieceShell(const string &filePath);

    bool m_paused;
    string m_currLevel;
    int m_lastTime;
    int m_targetFrameMS;
    RInputSystem *m_inputSystem;
    RWorld *m_world;
    RCamera *m_camera[MAX_PLAYERS];
    RPlayer *m_player[MAX_PLAYERS];
    ROnScreenDisplay *m_display[MAX_PLAYERS];
    GLdouble m_screenHeight;
    GLdouble m_screenWidth;
    GAMEMODE m_mode;
    GLuint m_shadowMap;
    int m_numPlayers;

    RGameLogic *m_gameLogic;
    string m_resourceRoot;

    // Indicates whether or not the race has started.
    // False while the countdown is occurring
    bool m_raceStarted;
};


//holds state of various inputs
class RInputSystem {
 public:
    RInputSystem(REngine *eng) { m_engine = eng; }
    virtual void ExecuteInput(int deltaTime)=0;

 protected:
    REngine *GetEngine() { return m_engine; }
 private:
    REngine *m_engine;
    
};

class RInputAction {
 public:
    //executed when key is first down
    virtual void KeyPressed(int deltaTime) { };
    //executed after the key has been down for more than one iteration
    virtual void KeyDown(int deltaTime) { };
    //executed when key is released
    virtual void KeyUp(int deltaTime) { };
};

class RGameAction:public RInputAction {
 public:
    RGameAction(REngine *eng) { m_engine = eng; }
    
 protected:
    REngine *m_engine;
};

class RKeyboardSystem: public RInputSystem {
 public:
    RKeyboardSystem(REngine *eng): RInputSystem(eng) { }
    virtual void ExecuteInput(int deltaTime);
    void AddKeyAction(char key, int actionId);
    void AddKeyAction(char key, RInputAction *action);

 private:
    hash_map<char,RInputAction *> m_actions;
    hash_map<char, bool> m_keyStates;
};


class RPlayer {
 public:
    RPlayer(REngine *eng,int i, int j, RTrack *track, RGameLogic *gameLogic, float interpolate, const char *modelPath, MD2Texture tex); //initial track coordinates
    ~RPlayer();
    
    void Rotate(float degrees);
    void Move(float distance, Vector4 dir, Vector4 &newDir);
    Vector4 GetPosition() {
        return m_pos;
    }
    void Render(RCamera *cam);
    void FinishTurn(list<RPlayer *> allPlayers, int deltaTime);
	Vector4 GetUp() {return m_up; }
    Vector4 GetAhead() {return m_ahead; }
    void SetAhead(Vector4 ahead) { m_ahead = ahead; }
    Vector4 getPos() { return m_pos; }
    bool GoingWrongWay() {return m_gameLogic->GoingWrongWay(m_gameLogicId); }
    //Vector4 getAheadOrientation() { return m_ahead; }
    //Vector4 getUpOrientation() { return m_up; }
    void Animate(int deltaTime) { }
    int PositionInRace() { return m_gameLogic->GetRank(m_gameLogicId); }
    int LapsCompleted() { return m_gameLogic->NumLapsCompleted(m_gameLogicId); }
    int LapsToWin() { return m_gameLogic->NumLaps(); }
    RMD2Model *getModel() {return m_model; }
    void DrawBounds();
    float GetSpeed() { return physicsEngine->getSpeed(); }
    float GetMaxSpeed() { return physicsEngine->getMaxSpeed(); }
    bool FinishedRace();

    //returns -1 if a lap hasn't been completed yet
    int BestLapTime() { return m_gameLogic->GetPlayerInfo(m_gameLogicId).bestLapTime; }
    int CurrLapTime() { return m_gameLogic->GetPlayerInfo(m_gameLogicId).currLapTime; }
    int TotalTimeElapsed() { return m_gameLogic->GetPlayerInfo(m_gameLogicId).totalTimeElapsed; }

    //returns -1 if race hasn't been completed yet
    int TimeToCompleteRace() { return m_gameLogic->GetPlayerInfo(m_gameLogicId).totalRaceTime; }
 private:
    void OrientToForces(int deltaTime);
    void GenerateSmokeEffect(int deltaTime);

    int m_timeToNextSmoke;
    RMD2Model *m_model;
    Vector4 m_ahead;
    Vector4 m_up;
    Vector4 m_pos;
    RTrack *m_track;
    REngine *m_engine;
    RGameLogic *m_gameLogic;
    TrackGrid m_currGrid;
    void TransformOrientation(TrackGrid &curr);
    int m_gameLogicId;
    Vector4 m_lastResistance;
    Vector4 m_lastInput;
 public:
	RPhysics *physicsEngine;	
};


//should be abstract, specific implementations under it
class RCamera {
 public:
    RCamera(REngine *eng, int playerId, int numPlayers);
    
    void SetViewport();
    virtual void Animate(int deltaTime);
    void IncreaseSpeed() { if(!onTrack) speed+=.02; else trackDistance+=1;}
    void DecreaseSpeed() { if(!onTrack) speed-=.02; else trackDistance-=1;}
    void Rotate(float deltaAngle) { viewAngle+=deltaAngle; }
    void SwitchViewpoint();
    void GoUp() { viewPos[1]-=.2; }
    void GoDown() {viewPos[1]+=.2; }
    void ToggleFreeCamera() { freeCameraEnabled = !freeCameraEnabled; }
    void SetPosition(Vector4 curpos) { m_pos = curpos; }
    void SetAhead(Vector4 ahead) { m_ahead = ahead; }
    void SetUp(Vector4 up) { m_up = up; }
    int GetPlayerId() { return m_playerId; }
    int GetNumPlayers() { return m_numPlayers; }
    //returns parameters in start and end
    void GetViewRay(Vector4 &start, Vector4 &end);

    Vector4 GetPosition() { return m_pos; }
    Vector4 GetAhead() { return m_ahead; }
    Vector4 GetUp() { return m_up; }

    void SetFovy(GLdouble fovy) { m_persFovy = fovy; }
    void SetNearDist(GLdouble dist) { m_nearFrustumDist = dist; }
    void SetFarDist(GLdouble dist) { m_farFrustumDist = dist; }
    void SetHeight(GLdouble h) { m_height = h; }
    void SetWidth(GLdouble w) { m_width = w; }

    
    GLdouble GetFovy() { return m_persFovy; }
    GLdouble GetHeight() { return m_height; }
    GLdouble GetWidth() { return m_width; }
    GLdouble GetRatio() { return m_width/m_height; }
    GLdouble GetNearDist() { return m_nearFrustumDist; }
    GLdouble GetFarDist() { return m_farFrustumDist; }
    
 private:

    //FOR SPECIAL FREE CAMERA
    bool freeCameraEnabled;
    bool onTrack;
    int timeSinceLastMove;
    float speed;
    float viewAngle;
    GLfloat viewPos[3];
    int trackDistance;
    //FOR SPECIAL FREE CAMERA

    REngine *m_engine;
    // Vectors for gluLookAt/View frustum culling
    Vector4 m_pos;
    Vector4 m_ahead;
    Vector4 m_up;
    Vector4 velocity; //length is the speed
    
    // Projection parameters for view frustum culling
    GLdouble m_persFovy;
    GLdouble m_width;
    GLdouble m_height;
    GLdouble m_nearFrustumDist;
    GLdouble m_farFrustumDist;
    // Parameters for setting the viewport
    int m_numPlayers;
    int m_playerId;
};


struct LightInfo {
    GLfloat ambient_diffuse[4];
    GLfloat position[4];
};

struct FogInfo {
    GLfloat fogVals[4];
    float fogDensity;
};

//scene graph (lights and whatnot)
//make it a scene graph of BSP trees
//should scene graph itself be a BSP tree (each node is an RObject which contains global lighting conditions,etc) - also, this scene graph would be
//totally static, except for dynamic objects moving between them. what happens when an object is in 2 scenes at once?
class RWorld {
 public:
    RWorld(REngine *eng, const string &fileToLoad);
    ~RWorld();
    virtual void Prepare();
    virtual void Animate(int deltaTime);
    virtual void Draw(RCamera *viewer);
    vector<RPlayer *> GeneratePlayers(int numPlayers);
    RTrack *getTrack() { return m_track; }
    RGameLogic *getGameLogic() {return m_gameLogic; }
    RFontManager *getFontManager() {return m_fontManager; }
    void SetLights();

    //for testing purposes...
    //RScenery *getTestScenery() { return m_testScenery; }
    void AddDynamicEffect(RObject *ro);
 private:
    void LoadFogInfo(vector<string> &data,FogInfo &ret);
    bool TreeBuiltp() { return (m_tree != NULL); }
    void BuildTree(list<RObject *> &objects);
    void init(const string &fileName);
    void LoadLightData(vector<string> &data);
    void LoadFontManager(vector<string> &data);
    void LoadTrackAndGameLogic(vector<string> &data);
    void LoadScenery(vector<string> &data);
    void LoadLightDataLine(string &line, LightInfo &li);
    void LoadDome(vector<string> &data);
    void LoadCube(vector<string> &data);
    RBSPtree *m_tree;
    REngine *m_engine;
    //generated after track is made
    RGameLogic *m_gameLogic;

    
    //read in from file
    RTrack *m_track;
    list<RScenery *> m_scenery;
    list<RObject *> m_dynScenery;
    RFontManager *m_fontManager;
    REnclosingBackground *m_bg;
    vector<LightInfo> m_lights;
    //**********************


    
    
    //test data
    //RScenery *m_testScenery;
    //RTrack *track;
    //RMD2Model *myModel;
    //RMD2Model *gunModel;
    //RWorldBillBoard *rbb;
    //RSparkParticleSystem *ps;
};





#endif
