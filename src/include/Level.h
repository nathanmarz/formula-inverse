#ifndef LEVEL_H
#define LEVEL_H

#include <Math3D.h>
#include <vector>
#include <GL/gl.h>
#include <GL/glu.h>
#include <map>
#include <RVertex.h>
#include <RUtils.h>
#include <RLibraries.h>
#include "RBSPtree.h"
#include "RGameLogic.h"

using namespace std;
using namespace Math3D;

class RPieceShell;
class RPiece;
class RSubPiece;

class RCamera;



class REdgeShell {
 public:

    //only for copying over another one to this one
    REdgeShell() { }
    
    REdgeShell(Vector4 p1, Vector4 p2) {
        LineConstruct(p1,p2);
    }
    
    REdgeShell(vector<Vector4> &p, bool loopsBack) {
        CurveConstruct(p,loopsBack);
    }
    
    REdgeShell(vector<Vector4> &p) {
        CurveConstruct(p,false);
    }

    vector<Vector4> InterpolatePoints(int i) {
        assert(i>=points.size());
        if(LoopsBack()) {
            points.push_back(points[0]);
            //            points[points.size()-1].tangent*=-1;
        }
        int numExtraPoints = i-points.size();
        int numPer;
        numPer = numExtraPoints/(points.size()-1);        
        
        vector<Vector4> ret;
        ret.push_back(points[0].pos);
        //cerr<<"starting"<<endl;
        for(int i=0;i<points.size()-1;i++) {
            int numToUse = numPer;
            if(i==points.size()-2) numToUse = numExtraPoints;
            numExtraPoints-=numToUse;

            //cerr<<"interpolating points tangents:"<<endl;
            //PrintVector(points[i].tangent);
            //PrintVector(points[i+1].tangent);
            vector<RVertex> vertices = FillOutLine(numToUse+2,points[i],points[i+1]);
            //cerr<<endl;
            for(int j=1;j<vertices.size();j++) {
                //  cerr<<vertices[j].pos<<endl;
                ret.push_back(vertices[j].pos);
            }

        }
        if(LoopsBack()) points.pop_back();
        
        return ret;
    }

    bool LoopsBack() { return m_loopBack; }

    void DumpDefinePoints() {
        for(int j=0;j<points.size();j++) {
            PrintVector(points[j].pos);
        }
    }

    bool isCurved;
    
 private:
    void CurveConstruct(vector<Vector4> &p, bool loopsBack) {
        m_loopBack = loopsBack;
        isCurved = true;
        if(p.size()<2||loopsBack&&p.size()<3) {
            cerr<<"too few points in edge"<<endl; exit(1);
        }
        if(p.size()==2) {
            LineConstruct(p[0],p[1]);
        } else {
            for(int i=0;i<p.size();i++) {
                RVertex r;
                r.pos = p[i];
                points.push_back(r);
            }
            FigureOutTangents();
        }
    }
    
    void LineConstruct(Vector4 p1, Vector4 p2) {
        isCurved = false;
        m_loopBack = false;
        RVertex r1, r2;
        r1.pos = p1;
        r2.pos = p2;
        r1.tangent = Normalize3(p2-p1);
        r2.tangent = r1.tangent;
        points.push_back(r1);
        points.push_back(r2);
        

    }

    void FigureOutTangents() {
        for(int i=1;i<points.size()-1;i++) {
            points[i].tangent = FindTangent(points[i-1].pos,points[i].pos,points[i+1].pos);
        }
        int s = points.size()-1;
        if(!m_loopBack) {
            points[0].tangent = FindPreTangentNormal(points[0].pos,points[1].pos,points[2].pos);
            
            points[s].tangent = -1*FindPreTangentNormal(points[s-2].pos,points[s-1].pos,points[s].pos);
        } else {
            
            points[0].tangent = FindTangent(points[s].pos,points[0].pos,points[1].pos);
            points[s].tangent = FindTangent(points[s-1].pos,points[s].pos,points[0].pos);

        }
        
    }
    
    vector<RVertex> points;
    bool m_loopBack;

};


class RPieceShell: public vector<REdgeShell> {
 public:
    RPieceShell(int texW, int texH, float hermiteAmt) {texWidth = texW; texHeight = texH; hermiteParam = hermiteAmt; }
    void finalize();
    void finalize(vector<Vector4> entrance, vector<Vector4> exit);
    vector<Vector4> getStartOrientation() { return m_startOrientation; }
    vector<Vector4> getEndOrientation() { return m_endOrientation; }
    int texWidth, texHeight;
    float hermiteParam;
    bool hasCurves;
 private:
    vector<Vector4> m_startOrientation;
    vector<Vector4> m_endOrientation;
};

//typedef vector<REdgeShell> RPieceShell;



class REdge:public vector<RVertex> {
 public:
    REdge() { loopsBack = false; }
    bool loopsBack;
};

struct TrackGrid {
    RVertex v0, v1, v2;
    int i,j; //coordinates that r00 corresponds to inside track
    bool isBound;
    float minTravelStep;
    
    Vector4 GetNormal() {
        Vector4 a = v0.pos-v2.pos;
        Vector4 b = v1.pos-v2.pos;
        Vector4 ret = Normalize3(CrossProduct(a,b));
        if(j%2!=0)
            ret*=-1;
        return ret;
    }
    
    Vector4 GetStraight() {
        if(j%2==0)
            return Normalize3(v0.pos-v2.pos);
        else
            return Normalize3(v2.pos-v1.pos);
    }

    Vector4 GetCenter() {
        return (v0.pos+v1.pos+v2.pos)/3;
    }

    Vector4 GetGridPlane() {
        Vector4 norm = GetNormal();
        return GetPlane(v0.pos,norm);
    }

    bool GetBound(Vector4 &bound) {
        if(!isBound) return false;
       
        if(j==0) {
            Vector4 e1 = v2.pos-v0.pos;
            Vector4 norm1 = CrossProduct(e1,v1.pos-v0.pos);
            bound = GetPlane(v0.pos,CrossProduct(e1,norm1));
        } else {
            Vector4 e2 = v1.pos-v2.pos;
            Vector4 norm2 = CrossProduct(e2,v2.pos-v0.pos);
            bound = GetPlane(v2.pos,CrossProduct(norm2,e2));
        }
        return true;
    }

    void GetBoundParams(Vector4 &outPos, Vector4 &outVec);

    bool PointWithinMe(Vector4 pos) {
        Vector4 norm = GetNormal();
        if(j%2==0) norm*=-1;
        Vector4 a = v2.pos-v0.pos;
        Vector4 b = v1.pos-v2.pos;
        Vector4 c = v1.pos-v0.pos;
        Vector4 plane1 = GetPlane(v0.pos,CrossProduct(norm,c));
        Vector4 plane2 = GetPlane(v2.pos,CrossProduct(b,norm));
        Vector4 plane3 = GetPlane(v0.pos,CrossProduct(a,norm));
        return DistToPlane(plane1,pos)>=0&&DistToPlane(plane2,pos)>=0&&DistToPlane(plane3,pos)>=0;
    }
    
};





//entrance and exit define 3 points that should align to each other
//they should be a triangle, with bottom right of base specified first, and rest counterclockwise) - exit is bottom left first and clockwise
//triangle is ABC, with AB and BC being unit length, and angle ABC being 90 degrees

class RPiece {
 public:
    friend class RPieceShell;
    //only use this constructor when most of start and end edges are around line between end points
    RPiece(RPieceShell &rps, int widthSubdivision, int lengthSubdivision, RPiece *alignTo, bool tester);
    RPiece(RPieceShell &rps, int widthSubdivision, int lengthSubdivision, RPiece *alignTo);
    
    
    void Build(GLuint texName);

    void FillOutTrack(vector<REdge> *track) {
        for(int i=0;i<m_edges.size()-1;i++) {
            track->push_back(m_edges[i]);
        }
    }

    void ConnectToPieceEnd(RPiece *other);

    void CreateSubPieces(vector<RSubPiece *> &toAppend, float subPieceDist);
    
    void DumpGrid();    
    void Viewpoint(int i, int j);
    void DisplayBall(int i, int j);
    void Render(RCamera *cam);
    int Length() { return m_edges.size(); }
    bool hasCurves;
 private:
    float m_hermiteParam;
    void ConstructHelper(RPieceShell &rps, int widthSubdivision, int lengthSubdivision, RPiece *alignTo, bool tester);
    Matrix4 GetOrientationMatrix();
   
    vector<Vector4> GetEdgeOrientation(int i, bool end);
    void FillOutNormals();
    void FillOutTextureCoordinates();
    float TotalDistHorizontal(int i);
    float TotalDistVertical(int j);
    void FillOutEdgesUsingKeyFrames(vector<REdge> &keyFrames, int width, int lengthSubdivision);
    //    void OrientSelf();
    //void OrientSelf(bool forView);
    //void ConnectToPiece(RPiece *other);
    void FindAndOrientFrames(RPieceShell &rps, vector<REdge> &frames, RPiece *other, int widthSubdivision, bool tester);
    Matrix4 FindOrientTransform(RPieceShell &rps, RPiece *other);
    
    //have a map from level of detail to edges (divide by 2 each time) (or possible make it exponential)
    //hash_map<int,vector<REdge> > m_edges;
    vector<REdge> m_edges;
    vector<REdge> m_frames;

    int m_widthDivide, m_lengthDivide;
    int m_texWidth, m_texHeight;

    GLuint m_texName;

    vector<Vector4> m_entrance;
    vector<Vector4> m_exit;

    RPiece *m_previousPiece;
    RPiece *m_nextPiece;
};


class RSubPiece: public RObject {
 public:
    //grabs pieces of edges from iStart to iEnd
    RSubPiece(vector<REdge> &edges, GLuint texName, int iStart, int iEnd, bool hasCurves);
    ~RSubPiece();

    //un-used, never called
    void Animate(int deltaTime) { }
 protected:
    void DoRender(RCamera *viewer);
 private:
    bool m_hasCurves;
    void DoDraw(int lod_i_step, int lod_j_step, bool doCameraCull,float lineWidth);
    vector<REdge> m_edges;
    GLuint m_texName;
    void ComputeBoundingVolume();
    GLuint m_dispList;
    
};


class RTrack {
 public:
    RTrack(vector<RPieceShell *> &pieces, int widthLOD, int lengthLOD, GLuint tex, bool cyclic);
    ~RTrack();
    void GetEdgeVertices(int i, RVertex &right,RVertex &left);
    void Viewpoint(int i, int j);    
    void RenderAll(RCamera *cam);
    void FindCloseTrackPosition(Vector4 pos, TrackGrid &currTri, TrackGrid &newTri);
    int GetStartJForPlayer(int index, int numPlayers);
    list<RObject*> GetListOfPieces();
    TrackGrid GetGridOfPoint(int i, int j);
    //int FindNewICoordinate(Vector4 pos, TrackGrid &currTri);
    //int FindNewICoordinate(Vector4 projPos,Vector4 projDir,float distTraveled,TrackGrid currTri);
    //edgeNum is 0, 1, or 2
    // 0 is edge between v0 and v1
    //1 is edge between v1 and v2
    //2 is edge between v1 and v3
    bool GetNewGridCoordinates(TrackGrid &tg, int edgeNum, int &iNew, int &jNew);

    //must be at least 2 checkpoints (more is better... but not too much)
    RGameLogic *CreateGameLogic(int numCheckpoints, int lapsToWin, int numBoosts);
    int NextI(int currI); //returns -1 if off track (loops if not)
    int PreviousI(int currI); //returns -1 if off track
    void DisplayBall(int i, int j);
    //    float minTravelStep();
    //void ProjectVectorOntoTrack(int i,Vector4 realPos, Vector4 realDir, Vector4 &projPos,Vector4 &projDir, TrackGrid &currGrid);
    bool GetBounds(int i, Vector4 &bound1, Vector4 &bound2);

 private:
    void FillOutTrackGrids();
    TrackGrid MakeGridOfPoint(int i, int j);
    int PrevJ(int i, int j);
    int NextJ(int i, int j);
    int FindActualTrackJ(int i, int j, int dj);
    void CreateSubPieces(vector<RPiece *> &piece_builders);
    void GetClosestSectionOnStrip(const TrackGrid& gridNow, Vector4 pos, int minDj, int maxDj, float &currDist, TrackGrid &currGrid);
    float ComputeMinTravelStep(int i);
    void FillOutTrack(vector <RPiece *> &piece_builders);
    
    
    vector<RSubPiece *> m_pieces;
	
    vector<REdge> m_edges;
    vector<vector<TrackGrid> > m_trackGrid;

    //    float m_minTravelStep;
    bool m_cyclic;

};

#endif
