#include "RGameLogic.h"
#include <list>
#include <algorithm>

#define WRONG_WAY_AMT 5
#define WRONG_WAY_COUNTDOWN 5


int RPlayerInfo::lastCheckpointI;
int RPlayerInfo::lastI;

RGameLogic::RGameLogic(vector<int> &checkPts, int lapsToWin, int lastI, int numBoosts) {
    m_boosts = numBoosts;
    m_iCheckpoints = checkPts;
    std::sort(m_iCheckpoints.begin(),m_iCheckpoints.end());
    m_lapsToWin = lapsToWin;
    m_lastI = lastI;
    m_nextRank = 1;
    m_raceStarted = false;
    RPlayerInfo::lastCheckpointI = checkPts.back();
    RPlayerInfo::lastI = m_lastI;
}

int RGameLogic::CreatePlayerInfo(int startI) {
    RPlayerInfo newPlayer;
    newPlayer.nextCheckpointIndex = 0;
    newPlayer.lapsCompleted = 0;
    newPlayer.iPosition = startI;
    newPlayer.id = m_players.size();
    newPlayer.consecutiveWrongWay = 0;
    newPlayer.wrongWayCountdown = 0;
    newPlayer.prevDirection = forward;
    newPlayer.rank = -1;
    newPlayer.totalTimeElapsed = 0;
    newPlayer.bestLapTime = -1;
    newPlayer.currLapTime = 0;
    newPlayer.totalRaceTime = -1; //player has not completed race yet
    m_players.push_back(newPlayer);
    return newPlayer.id;

}

int RGameLogic::GetFinishI() {
    return m_iCheckpoints[m_iCheckpoints.size()-1];
}

int RGameLogic::NumLapsCompleted(int playerId) {
    return m_players[playerId].lapsCompleted;
}


int GetIDistance(int i) {
    if(i<=RPlayerInfo::lastI&&i>=RPlayerInfo::lastCheckpointI) {
        return i-RPlayerInfo::lastCheckpointI;
    } else {
        return RPlayerInfo::lastI-RPlayerInfo::lastCheckpointI+i+1;
    }
}

bool SortPositionOfPlayers(RPlayerInfo p1, RPlayerInfo p2) {
    if(p1.rank==-1&&p2.rank!=-1) return false;
    if(p1.rank!=-1&&p2.rank==-1) return true;
    if(p2.rank<p1.rank) return false;
    if(p2.rank>p1.rank) return true;
    if(p2.lapsCompleted>p1.lapsCompleted) return false;
    else if(p2.lapsCompleted<p1.lapsCompleted) return true;
    else {
        //i hope this favors whoever's in front
        if(p2.nextCheckpointIndex>p1.nextCheckpointIndex) return false;
        else if(p2.nextCheckpointIndex<p1.nextCheckpointIndex) return true;
        else {
            int p2i = GetIDistance(p2.iPosition);
            int p1i = GetIDistance(p1.iPosition);
            if(p2i>=p1i) return false;
            else
                return true;

        }
    }
}

int RGameLogic::GetRank(int playerId) {
    vector<RPlayerInfo> toSort = m_players;
    std::sort(toSort.begin(),toSort.end(),&SortPositionOfPlayers);
    for(int i=0;i<toSort.size();i++) {
        if(toSort[i].id==playerId) return i+1;
    }
    cerr<<"could not find player id in RGameLogic module"<<endl;
    exit(1);
}

void RGameLogic::BeginRace() {
    m_raceStarted = true;
}

bool RGameLogic::SetPlayerPosition(int id, int iCoord, int timeElapsed) {
    bool stillRacing = m_players[id].lapsCompleted<NumLaps();
    
    int oldI = m_players[id].iPosition;
    m_players[id].iPosition = iCoord;

    if(m_raceStarted) {
        m_players[id].currLapTime+=timeElapsed;
        m_players[id].totalTimeElapsed+=timeElapsed;
    }
    //cerr<<"current lap:"<<m_players[id].currLapTime<<endl;
    //cerr<<"best lap:"<<m_players[id].bestLapTime<<endl;
    
    Direction dir = DirectionBetweenICoordinates(oldI,iCoord,m_players[id].prevDirection);
    if(dir==backward) {
        m_players[id].consecutiveWrongWay++;
        if(m_players[id].consecutiveWrongWay>=WRONG_WAY_AMT)
            m_players[id].wrongWayCountdown = WRONG_WAY_COUNTDOWN;
        
    } else {
        m_players[id].wrongWayCountdown--;
        if(m_players[id].wrongWayCountdown<0) m_players[id].wrongWayCountdown = 0;
        m_players[id].consecutiveWrongWay = 0;
    }
    m_players[id].prevDirection = dir;
    if(PassedCheckpoint(oldI,iCoord,m_iCheckpoints[m_players[id].nextCheckpointIndex])) {
        //cerr<<"passed checkpt:"<<m_iCheckpoints[m_players[id].nextCheckpointIndex]<<endl;
      m_players[id].nextCheckpointIndex++;
        if(m_players[id].nextCheckpointIndex>=m_iCheckpoints.size()) {
            m_players[id].nextCheckpointIndex = 0;
            m_players[id].lapsCompleted++;
            if(stillRacing&&m_raceStarted) {
                if(m_players[id].bestLapTime==-1||m_players[id].currLapTime<m_players[id].bestLapTime) {
                    m_players[id].bestLapTime = m_players[id].currLapTime;
                }
            }
            m_players[id].currLapTime = 0;
            if(m_players[id].lapsCompleted>=m_lapsToWin) {
                if(m_players[id].lapsCompleted==m_lapsToWin) {
                    m_players[id].rank = m_nextRank;
                    m_nextRank++;
                    m_players[id].totalRaceTime = m_players[id].totalTimeElapsed;

                }
                return true;
            }
        }
    }
    return false;
}

bool RGameLogic::PassedCheckpoint(int oldI, int newI, int checkI) {
    if(oldI==newI) return false;
    if(DirectionBetweenICoordinates(oldI,newI,forward)==forward) {

      if(newI>oldI) {
	if(checkI<=newI&&checkI>oldI) return true;
      } else {
	if(checkI>oldI||checkI<=newI) {
	
	  return true;
	}
      }
    }
    return false;
}


Direction RGameLogic::DirectionBetweenICoordinates(int startI, int endI, Direction prevDirection) {
    int forwardLength, backwardLength;
    if(startI==endI) return prevDirection;
    if(endI>startI) {
      forwardLength = endI-startI;
      backwardLength = m_lastI-endI+startI+1;
    }
    else {
        forwardLength = m_lastI-startI+endI+1;
	backwardLength = startI-endI;
    }
    
    if(backwardLength>forwardLength) return forward;
    else return backward;
}
