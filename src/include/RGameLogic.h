#ifndef RGAME_LOGIC
#define RGAME_LOGIC

#include "Math3D.h"
#include <list>
#include <vector>
#include "RLibraries.h"
#include <algorithm>

#define MAX_PLAYERS 4
#define POSITION Vector4

enum Direction {
    forward,
    backward
};


class RPlayerInfo {
 public:
    int rank;
    int iPosition;
    int lapsCompleted;
    int nextCheckpointIndex;
    int id;
    int consecutiveWrongWay;
    int wrongWayCountdown;
    int totalTimeElapsed;
    int bestLapTime;
    int currLapTime;
    int totalRaceTime;
    
    Direction prevDirection;
    static int lastCheckpointI;
    static int lastI;
};




//
// Sets up information about the track.
// Data about the track includes:
// @ the i positions of the checkpoints, 
// @ the number of players,
// @ the location of the players, 
// @ how many laps the player has completed, 
// @ how many laps are needed to win
//
class RGameLogic {
 public:
    //point to point should be one lap...
    //first check point should be ahead from start line
    //finish i coordinate should be at start of race
    RGameLogic(vector<int> &checkPts, int lapsToWin, int lastI, int numBoosts);

    void BeginRace();
    int GetTotalBoosts() { return m_boosts; }
    //returns id of player
    int CreatePlayerInfo(int startI);
    RPlayerInfo GetPlayerInfo(int id) {return m_players[id]; }
    //position in race (i.e. 1st, 2nd, 3rd)
    int GetRank(int playerId);
    int GetFinishI();
    bool PlayerFinished(int playerId) { return m_players[playerId].rank!=-1; }
    //figures out if they've finished a lap or whatnot - returns if they've finished the race
    bool SetPlayerPosition(int playerId, int iCoord, int timeElapsed);
    bool GoingWrongWay(int playerId) {
        return m_players[playerId].wrongWayCountdown>0;  
    }
    int NumLapsCompleted(int playerId);
    int NumLaps() {return m_lapsToWin; }
    
 protected:
    Direction DirectionBetweenICoordinates(int startI, int endI, Direction prevdir);
    bool PassedCheckpoint(int oldI, int newI, int checkI);
    vector<RPlayerInfo> m_players;

    bool m_raceStarted;
    int m_lastI;
    int m_lapsToWin;
    int m_iFinishLine;
    int m_nextRank;
    int m_boosts;
    vector<int> m_iCheckpoints;
};


#endif
