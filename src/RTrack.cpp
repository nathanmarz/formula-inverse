#include "Level.h"


#define SUB_PIECE_DIST 11

RTrack::RTrack(vector<RPieceShell *> &pieces, int widthLOD, int lengthLOD, GLuint tex, bool cyclic) {
    m_cyclic = cyclic;
    vector<RPiece *> piece_builders;
    for(int i=0;i<pieces.size();i++) {
        RPiece *prev = NULL;
        if(i>0) prev = piece_builders[i-1];
        RPiece *newPiece = new RPiece(*(pieces[i]),widthLOD,lengthLOD,prev);
        piece_builders.push_back(newPiece);
    }
    if(cyclic) piece_builders[0]->ConnectToPieceEnd(piece_builders.back());
    for(int i=0;i<piece_builders.size();i++) {
        piece_builders[i]->Build(tex);
    }
    FillOutTrack(piece_builders);
    
    // ComputeMinTravelStep();
    //cerr<<"uh oh"<<endl;
    CreateSubPieces(piece_builders);
    
    //cerr<<"hello"<<endl;
    for(int i=0;i<piece_builders.size();i++) delete piece_builders[i];
    //cerr<<"filling out"<<endl;
    FillOutTrackGrids();
}

RTrack::~RTrack() {
    for(int i=0;i<m_pieces.size();i++) {
        delete m_pieces[i];
    }
}

void RTrack::GetEdgeVertices(int i, RVertex &right,RVertex &left) {
    right = m_edges[i][0];
    left = m_edges[i][m_edges[i].size()-1];
}

RGameLogic *RTrack::CreateGameLogic(int numCheckpoints, int lapsToWin, int numBoosts) {
    int iStep = (m_edges.size()-1)/numCheckpoints;
    vector<int> checkPoints;
    // cerr<<"Max i:";
    //cerr<<m_edges.size()-1<<endl;
    for(int i=1;i<=numCheckpoints;i++) {
        checkPoints.push_back(i*iStep);
        //    cerr<<i*iStep<<endl;
    }
    //exit(0);
    return new RGameLogic(checkPoints,lapsToWin,m_edges.size()-1,numBoosts);
}

void RTrack::FillOutTrackGrids() {
    for(int i=0;i<m_edges.size()-1;i++) {
        vector<TrackGrid> toAppend;
        float minDist = ComputeMinTravelStep(i);
        for(int j=0;j<=(m_edges[i].size()-2)*2+1;j++) {
            TrackGrid tg = MakeGridOfPoint(i,j);
            tg.minTravelStep = minDist;
            toAppend.push_back(tg);
        }
        m_trackGrid.push_back(toAppend);
    }
}


//returns whether there are bounds or not at this i coordinate
bool RTrack::GetBounds(int i, Vector4 &bound1, Vector4 &bound2) {
    TrackGrid g1 = GetGridOfPoint(i,0);
    TrackGrid g2 = GetGridOfPoint(i,(m_edges[i].size()-2)*2+1);
    assert(g2.isBound);
    Vector4 e1 = g1.v2.pos-g1.v0.pos;
    Vector4 norm1 = CrossProduct(e1,g1.v1.pos-g1.v0.pos);
    bound1 = GetPlane(g1.v0.pos,CrossProduct(e1,norm1));
    Vector4 e2 = g2.v1.pos-g2.v2.pos;
    Vector4 norm2 = CrossProduct(e2,g2.v2.pos-g2.v0.pos);
    bound2 = GetPlane(g2.v2.pos,CrossProduct(norm2,e2));
    
    return !m_edges[i].loopsBack;
}

void RTrack::CreateSubPieces(vector<RPiece *> &piece_builders) {
    for(int i=0;i<piece_builders.size();i++) {
        piece_builders[i]->CreateSubPieces(m_pieces,SUB_PIECE_DIST);
    }
    //cerr<<"sub pieces: "<<m_pieces.size()<<endl;
    //exit(1);
}

int RTrack::GetStartJForPlayer(int index, int totalPlayers) {
    int totalJ = m_edges[0].size()*2;
    int jdist = totalJ/(totalPlayers+1);
    return jdist*(index+1);
    
}

void RTrack::Viewpoint(int i, int j) {

    cerr<<i<<endl;
    cerr<<m_edges.size()<<endl;
        
    if(j<0) j=0;
    if(j>=m_edges[0].size()) j = m_edges[0].size()-1;
    i-=1; //first edge doesn't count
    if(i<1) i = 1;
    if(i>=m_edges.size()-1) i = m_edges.size()-1;
    RVertex r = m_edges[i][j];
    Vector4 pos = r.pos+.7*r.normal;
    Vector4 lookAt = r.pos+r.tangent+.5*r.normal;
    gluLookAt(pos[0],pos[1],pos[2],lookAt[0],lookAt[1],lookAt[2],r.normal[0],r.normal[1],r.normal[2]);
}
    
void RTrack::RenderAll(RCamera *cam) {
    for(int i=0;i<m_pieces.size();i++) {
        m_pieces[i]->Render(cam);
    }
}

list<RObject*> RTrack::GetListOfPieces() {
  list<RObject*> retList;
  vector<RSubPiece*>::iterator it = m_pieces.begin();

  while ( it != m_pieces.end() ) {
    retList.push_front((*it));
    it++;
  }
  return retList;
}

TrackGrid RTrack::GetGridOfPoint(int i, int j) {
    return m_trackGrid[i][j];
}


TrackGrid RTrack::MakeGridOfPoint(int i, int j) {
    //later on, grid can wrap around edges
    int actualJ = j/2;
    if(i>=m_edges.size()-1||actualJ>=m_edges[0].size()-1||i<0||actualJ<0) {
        cerr<<"Query for grid outside of track"<<endl;
        RTrack *r = (RTrack *) 17612746;
        delete r;
        exit(1);
    }
    TrackGrid ret;

    ret.v0 = m_edges[i+1][actualJ];
    ret.v1 = m_edges[i][actualJ+1];
    if(j%2==0) ret.v2 = m_edges[i][actualJ];
    else
        ret.v2 = m_edges[i+1][actualJ+1];
    ret.i = i;
    ret.j = j;
    ret.isBound = false;
    int maxJ = (m_edges[i].size()-2)*2+1;
    if(!m_edges[i].loopsBack&&(j==0||j==maxJ)) {
        //   cerr<<"is bound: "<<j<<" "<<maxJ<<endl;
        ret.isBound = true;
    }
    return ret;
}

//edgeNum is 0, 1, or 2
// 0 is edge between v0 and v1
//1 is edge between v1 and v2
//2 is edge between v1 and v3
bool RTrack::GetNewGridCoordinates(TrackGrid &tg, int edgeNum, int &iNew, int &jNew) {
    assert(edgeNum>=0&&edgeNum<3);
    if(tg.j%2==0) {
        switch(edgeNum) {
        case 0:
            jNew = tg.j+1;
            iNew = tg.i;
            break;
        case 1:
            jNew = tg.j+1;
            iNew = tg.i-1;
            break;
        case 2:
            jNew = tg.j-1;
            iNew = tg.i;
            break;
        }
    } else {
        switch(edgeNum) {
        case 0:
            jNew = tg.j-1;
            iNew = tg.i;
            break;
        case 1:
            jNew = tg.j+1;
            iNew = tg.i;
            break;
        case 2:
            jNew = tg.j-1;
            iNew = tg.i+1;
            break;
        }

    }
    if(iNew<0||iNew>=m_edges.size()-1||jNew<0||jNew>=m_edges[0].size()-1) return false;
    else
        return true;
}

  
    
void RTrack::DisplayBall(int i, int j) {

}





void RTrack::FillOutTrack(vector<RPiece *> &piece_builders) {
    for(int i=0;i<piece_builders.size();i++) {
        piece_builders[i]->FillOutTrack(&m_edges);
    }
    if(m_cyclic) {
        m_edges.push_back(m_edges[0]);
    }
    int end = m_edges.size()-1;
    int j = m_edges[0].size()/2;
    //    cerr<<"end check:"<<endl;
    //PrintVector(m_edges[end][j].pos);
    //PrintVector(m_edges[0][j].pos);
    //exit(1);
}

float FindStraightDistToThird(Vector4 p1, Vector4 p2, Vector4 p3) {
    float distHypotenuse = Length3(p3-p2);
    Vector4 a = p3-p2;
    Vector4 b = p2-p1;
    float cosTheta = DotProduct4(a,b)/(Length3(a)*Length3(b));
    float compare = fabs(distHypotenuse*cosTheta);
    //    if(compare<0) exit(1);
    float compare2 = Length3(p2-p1);
    //if(compare==0) compare = compare2;
    return min(compare,compare2);
}

int RTrack::NextJ(int i, int j) {
    int width = m_edges[i].size();
    if(j==width-1) {
        if(m_edges[i].loopsBack) {
            return 1; //because last j and 0th j are the same
        } else {
            return -1;
        }

    }
    return j+1;
}

int RTrack::PrevJ(int i, int j) {
    int width = m_edges[i].size();
    if(j==0) {
        if(m_edges[i].loopsBack) {
            return width-2; //because last j and 0th j are the same
        } else {
            return -1;
        }

    }
    return j-1;
}


//returns a very large number if can't find one
float RTrack::ComputeMinTravelStep(int iStart) {
    
    float minTravelStep = -1;
    /*
    for(int i=iStart-1;i<=iStart+1;i++) {
        for(int j=jStart-1;j<=jStart+1;j++) {
            if(i<0||i>=m_edges.size()||j<0||j>=m_edges[i].size()) continue;
            int nextI = NextI(i);
            int previousI = PreviousI(i);
            if(nextI!=-1&&previousI!=-1) {
                Vector4 p1 = m_edges[previousI][j].pos;
                Vector4 p2 =  m_edges[i][j].pos;
                Vector4 p3 = m_edges[nextI][j].pos;
                float compare = FindStraightDistToThird(p1,p2,p3);
                if(minTravelStep==-1||compare<minTravelStep)
                    minTravelStep = compare;
                compare = FindStraightDistToThird(p3,p2,p1);
                if(minTravelStep==-1||compare<minTravelStep)
                    minTravelStep = compare;        
            }
            int prevJ = PrevJ(i,j);
            int nextJ = NextJ(i,j);
            if(prevJ!=-1&&nextJ!=-1) {
                Vector4 p1 = m_edges[i][prevJ].pos;
                Vector4 p2 =  m_edges[i][j].pos;
                Vector4 p3 = m_edges[i][nextJ].pos;
                float compare = FindStraightDistToThird(p1,p2,p3);
                if(minTravelStep==-1||compare<minTravelStep)
                    minTravelStep = compare;
                compare = FindStraightDistToThird(p3,p2,p1);
                if(minTravelStep==-1||compare<minTravelStep)
                    minTravelStep = compare;        
            }
        }
    }
    */


    for(int i=iStart-1;i<=iStart+1;i++) {
        //cerr<<i<<endl;
        if(i>=0&&i<m_edges.size()) {
            for(int j=0;j<m_edges[i].size();j++) {
                
                
                Vector4 p1, p2, p3;
                int prevI = PreviousI(i);
                int nextI = NextI(i);
                if(prevI!=-1&&nextI!=-1) {
                p1 = m_edges[prevI][j].pos;
                p2 = m_edges[i][j].pos;
                p3 = m_edges[nextI][j].pos;
                float compare = FindStraightDistToThird(p1,p2,p3);
                if(minTravelStep==-1||compare<minTravelStep)
                    minTravelStep = compare;
                compare = FindStraightDistToThird(p3,p2,p1);
                if(minTravelStep==-1||compare<minTravelStep)
                    minTravelStep = compare;
                }
                int prevJ = PrevJ(i,j);
                int nextJ = NextJ(i,j);
                if(prevJ!=-1&&nextJ!=-1) {
                    p1 = m_edges[i][prevJ].pos;
                    p2 = m_edges[i][j].pos;
                    p3 = m_edges[i][nextJ].pos;
                    float compare = FindStraightDistToThird(p1,p2,p3);
                    if(minTravelStep==-1||compare<minTravelStep)
                        minTravelStep = compare;
                    compare = FindStraightDistToThird(p3,p2,p1);
                    if(minTravelStep==-1||compare<minTravelStep)
                        minTravelStep = compare;
                }
                
            }
        }
    }
    
    return .9*minTravelStep;
    
}



/*
void RTrack::ComputeMinTravelStep(int i, int j) {
    m_minTravelStep = -1;
    for(int i=0;i<m_edges.size();i++) {
        for(int j=0;j<m_edges[i].size();j++) {
            Vector4 p1, p2, p3;
            int prevI = PreviousI(i);
            int nextI = NextI(i);
            if(prevI!=-1&&nextI!=-1) {
                p1 = m_edges[prevI][j].pos;
                p2 = m_edges[i][j].pos;
                p3 = m_edges[nextI][j].pos;
                float compare = FindStraightDistToThird(p1,p2,p3);
                if(m_minTravelStep==-1||compare<m_minTravelStep)
                    m_minTravelStep = compare;
            }
            int prevJ = PrevJ(i,j);
            int nextJ = NextJ(i,j);
            if(prevJ!=-1&&nextJ!=-1) {
                p1 = m_edges[i][prevJ].pos;
                p2 = m_edges[i][j].pos;
                p3 = m_edges[i][nextJ].pos;
                float compare = FindStraightDistToThird(p1,p2,p3);
                if(m_minTravelStep==-1||compare<m_minTravelStep)
                    m_minTravelStep = compare;
            }
            
        }
    }
    
    m_minTravelStep*=.99;
    //    cerr<<"min travel step:"<<m_minTravelStep<<endl;
    //exit(1);
    
}

*/

int RTrack::FindActualTrackJ(int i, int j, int dj) {
    int newJ = j+dj;
    int maxJ = (m_edges[i].size()-2)*2+1;
    if(newJ>maxJ) {
        if(m_edges[i].loopsBack) {
            return newJ-maxJ-1;
        } else return -1;
    }
    if(newJ<0) {
        if(m_edges[i].loopsBack) {
            return maxJ+newJ+1;
        } else return -1;
    }
    return newJ;
}




void RTrack::GetClosestSectionOnStrip(const TrackGrid &gridNow, Vector4 pos, int minDj, int maxDj, float &currDist, TrackGrid &currGrid) {
    minDj = -3;
    maxDj = 3;
    //cerr<<m_edges.size()-2<<endl;
    //cerr<<"hello"<<endl;
    //int width = m_edges[0].size();
    //    int totalJ = (width-2)*2+1;
    int i = gridNow.i;
    int jNow = gridNow.j; 
    currDist = -1;
    //int minDj = -range;
    //int maxDj = range;
    /*
    if(jNow%2==0) {
        minDj = -2;
        maxDj = 3;
    } else {
        minDj = -3;
        maxDj = 2;
    }
    */
    for(int dj=minDj;dj<=maxDj;dj++) {
        int j = FindActualTrackJ(i,jNow,dj);
        if(j!=-1) {
            TrackGrid tester = GetGridOfPoint(i,j);
            //      Vector4 plane = GetPlane(tester.v0.pos,tester.GetNormal());
            float projDist;
            float dist = PointTriDistance(tester.v0.pos,tester.v1.pos,tester.v2.pos,pos,projDist);
            if(currDist==-1||dist<=currDist) {
                currDist = dist;
                currGrid = tester;
            }
        }
    }
}

/*
void RTrack::ProjectVectorOntoTrack(int i,Vector4 realPos, Vector4 realDir, Vector4 &projPos,Vector4 &projDir, TrackGrid &currGrid) {
    float dist;
    GetClosestSectionOnStrip(i,realPos,dist,currGrid);
    assert(dist!=-1);
    Vector4 plane = GetPlane(currGrid.v0.pos,currGrid.GetNormal());
   
    ProjectOntoPlane(plane,realPos,realDir,projPos,projDir);
    projDir = Normalize3(projDir);
}
*/



/*
int RTrack::FindNewICoordinate(Vector4 projPos,Vector4 projDir,float distTraveled,TrackGrid currTri) {
    int currI = currTri.i;
    //    int bestI = middleI;
    Vector4 newPos = projPos+projDir*distTraveled;
    //cerr<<"hello"<<endl;
    while(true) {
        Vector4 plane = GetPlane(currTri.v0.pos,currTri.GetNormal());
        newPos = ProjectPointOntoPlane(plane,newPos);
        if(currTri.PointWithinMe(newPos)) break;
        //cerr<<currI<<endl;
        if(currI==0) {
            currI = 1;
            float dist;
            GetClosestSectionOnStrip(1,newPos,dist,currTri);
        } else if(currI==m_edges.size()-1) {
            currI = m_edges.size()-2;
            float dist;
            GetClosestSectionOnStrip(currI,newPos,dist,currTri);
        } else {
            float dist1, dist2;
            TrackGrid tri1, tri2;
            GetClosestSectionOnStrip(currI+1,newPos,dist1,tri1);
            GetClosestSectionOnStrip(currI-1,newPos,dist2,tri2);
            if(dist1<dist2) {
                currI++;
                currTri = tri1;
            } else {
                currI--;
                currTri = tri2;
            }
        }
        
    }
    return currI;
}
*/

/*
int RTrack::FindNewICoordinate(Vector4 pos, TrackGrid &currTri) {
    int middleI = currTri.i;
    float bestDist;
    int bestI = middleI;
    TrackGrid dummy;
    GetClosestSectionOnStrip(middleI,pos,bestDist,dummy);
    if(middleI-1>=0) {
        float tryDist;
        GetClosestSectionOnStrip(middleI-1,pos,tryDist,dummy);
        if(tryDist<bestDist) {
            bestI = middleI-1;
            bestDist = tryDist;
        }
    }
    if(middleI+1<m_edges.size()) {
        float tryDist;
        GetClosestSectionOnStrip(middleI+1,pos,tryDist,dummy);
        if(tryDist<bestDist) {
            bestI = middleI+1;
            bestDist = tryDist;
        }
    }
    return bestI;
}
*/

int RTrack::NextI(int currI) {
    int i = currI+1;
    if(i>=m_edges.size()-1) {
        if(m_cyclic) return 0;
        else {
            return -1;
        }
    }
    return i;
}

int RTrack::PreviousI(int currI) {
    int i = currI-1;
    if(i<0) {
        if(m_cyclic) {
            return m_edges.size()-2;
        }
        else
            return -1;
    }
    return i;
}

void RTrack::FindCloseTrackPosition(Vector4 pos, TrackGrid &currTri, TrackGrid &newTri) {
    //cerr<<"entering"<<endl;
	int middleI = currTri.i;
    float bestDist;
    GetClosestSectionOnStrip(currTri,pos,-2,2,bestDist,newTri);
    int prevI = PreviousI(middleI);
    if(prevI!=-1) {
        float tryDist;
        int tryJ;
        int minDj, maxDj;
        if(currTri.j%2==0) {
            tryJ = currTri.j+1;
            minDj = -2;
            maxDj = 2;
        }
        else {
            tryJ = currTri.j;
            minDj = -2;
            maxDj = 0;
        }
        TrackGrid tryGrid = GetGridOfPoint(prevI,tryJ);
        TrackGrid tester;
        GetClosestSectionOnStrip(tryGrid,pos,minDj,maxDj,tryDist,tester);
        if(tryDist<=bestDist||bestDist==-1) {
            newTri = tester;
            bestDist = tryDist;
        }
    }
    int nextI = NextI(middleI);
    if(nextI!=-1) {
        float tryDist;
        int tryJ;
        int minDj, maxDj;
        if(currTri.j%2==0) {
            tryJ = currTri.j;
            minDj = 0;
            maxDj = 2;
        }
        else {
            tryJ = currTri.j-1;
            minDj = -2;
            maxDj = 2;
        }
        TrackGrid tryGrid = GetGridOfPoint(nextI,tryJ);

        TrackGrid tester;
        GetClosestSectionOnStrip(tryGrid,pos,minDj,maxDj,tryDist,tester);
        if(tryDist<=bestDist||bestDist==-1) {
            newTri = tester;
            bestDist = tryDist;
        }
    }
	//cerr<<"exiting"<<endl;
    assert(bestDist!=-1);
    //    cerr<<bestDist<<endl;
    
}

