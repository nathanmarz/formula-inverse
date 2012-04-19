#include <Level.h>
#include "Framework.h"

#define LOD_MIN_DIST_SQ 121 //121
#define LOD_MIN_I_STEPS 5
#define LOD_FADE_DIST_SQ 3200 //2500
#define LOD_FADE_AREA 16
#define MAX_EDGE_LINE_WIDTH 3


void RPieceShell::finalize() {
    cerr <<"aaa20"<<endl;
    RPiece rp(*this,50,50,NULL,true);
    cerr <<"aaa21"<<endl;
    rp.Build(0); //will never use texture
    cerr <<"aaa22"<<endl;
    m_startOrientation  = rp.GetEdgeOrientation(0,false);
    m_endOrientation = rp.GetEdgeOrientation(rp.Length()-1,true);
    cerr <<"aaa21"<<endl;
    finalize(m_startOrientation,m_endOrientation);
    if(m_endOrientation[0][0]*0!=0) {
        cerr<<endl;
        for(int i=0;i<3;i++) {
            PrintVector(m_startOrientation[i]);
        }
        cerr<<endl;
        for(int i=0;i<3;i++) {
            PrintVector(m_endOrientation[i]);
        }
        cerr<<endl;
        exit(1);
    }
}

void RPieceShell::finalize(vector<Vector4> entrance, vector<Vector4> exit) {
    m_startOrientation = entrance;
    m_endOrientation = exit;
    hasCurves = false;
    for(int i=0;i<this->size();i++) {
        if((*this)[i].isCurved) {
            hasCurves = true;
            break;
        }
    }
}

void RPiece::ConnectToPieceEnd(RPiece *other) {
    int s = other->m_frames.size()-1;
    if(other->hasCurves) hasCurves = true;
    if(hasCurves) other->hasCurves = true;
    //cerr<<"connecting to piece end"<<endl;
    for(int j=0;j<m_frames[0].size();j++) {
        RVertex rvOther = other->m_frames[s][j];
        Vector4 newPos = (m_frames[0][j].pos+rvOther.pos)/2;
        Vector4 newTangent = FindTangent(other->m_frames[s-1][j].pos,newPos,m_frames[1][j].pos);
        m_frames[0][j].pos = newPos;
        m_frames[0][j].tangent = newTangent;
        other->m_frames[s][j].pos = newPos;
        other->m_frames[s][j].tangent = newTangent;
    }

}



void RPiece::CreateSubPieces(vector<RSubPiece *> &toAppend, float subPieceDist) {
  int startI = 0;
  int lastI = 0;
  float dist = 0;
  int j = m_edges[0].size()/2;
  while(true) {
    lastI+=1;
    dist+=Length3(m_edges[lastI][j].pos-m_edges[lastI-1][j].pos);
    if((lastI-startI)>2&&dist>=subPieceDist&&lastI<m_edges.size()-3||lastI==m_edges.size()-1) {
      toAppend.push_back(new RSubPiece(m_edges,m_texName,startI,lastI,hasCurves));
      startI = lastI;
      dist = 0;
    }
    if(lastI==m_edges.size()-1) break;

  }
}


void RPiece::Build(GLuint texName) {
    SetHermiteParam(m_hermiteParam);
    FillOutEdgesUsingKeyFrames(m_frames,m_widthDivide,m_lengthDivide);
    FillOutNormals();
    FillOutTextureCoordinates();
    m_texName = texName;
}

RPiece::RPiece(RPieceShell &rps, int widthSubdivision, int lengthSubdivision, RPiece *alignTo, bool tester) {
    ConstructHelper(rps,widthSubdivision,lengthSubdivision,alignTo,tester);
    
}

RPiece::RPiece(RPieceShell &rps, int widthSubdivision, int lengthSubdivision, RPiece *alignTo) {
    ConstructHelper(rps,widthSubdivision,lengthSubdivision,alignTo,false);

}

void RPiece::ConstructHelper(RPieceShell &rps, int widthSubdivision, int lengthSubdivision, RPiece *alignTo,bool tester) {
    cerr<<"rpiece construct helper"<<endl;
    SetHermiteParam(rps.hermiteParam);
    m_hermiteParam = rps.hermiteParam;
    //cerr<<rps.hermiteParam<<endl;
    hasCurves = rps.hasCurves;
    if(alignTo!=NULL) {
        if(alignTo->hasCurves) hasCurves = true;
        if(hasCurves) alignTo->hasCurves = true;

    }
    m_previousPiece = alignTo;
    if(alignTo!=NULL)
        alignTo->m_nextPiece = this;
    m_nextPiece = NULL;
    cerr<<"rpiece construct helper2"<<endl;
    FindAndOrientFrames(rps,m_frames,alignTo,widthSubdivision,tester);
    cerr<<"rpiece construct helper3"<<endl;
    m_widthDivide = widthSubdivision;
    m_lengthDivide = lengthSubdivision;
    m_texWidth = rps.texWidth;
    m_texHeight = rps.texHeight;
}



vector<Vector4> RPiece::GetEdgeOrientation(int i, bool isEnd) {
    int width = m_edges[i].size();
 
    RVertex start = m_edges[i][0];
    RVertex end = m_edges[i][width-1];
    Vector4 slope = end.pos-start.pos;
    float len = Length3(slope);
    slope = Normalize3(slope);
    Vector4 p1 = (len/2.0-.5)*slope + start.pos;
    Vector4 p2 = (len/2.0+.5)*slope + start.pos;
 
    Vector4 toCross;
    if(!isEnd) {
        toCross = m_edges[i+1][0].pos-start.pos;
    } else {
        toCross = start.pos-m_edges[i-1][0].pos;
    }

    Vector4 norm = Normalize3(CrossProduct(toCross,slope));
    Vector4 p3 = p2+norm;
 
    vector<Vector4> ret;
    ret.push_back(p1);
    ret.push_back(p2);
    ret.push_back(p3);
    //cerr<<"slope:"<<endl;
    //PrintVector(slope);
    //PrintVector(end.pos);
    //PrintVector(start.pos);
    if(start.pos[0]*0!=0) {
        for(int j=0;j<m_edges[i].size();j++) {
            PrintVector(m_edges[i][j].pos);
        }
    }
    
    return ret;
}


float RPiece::TotalDistVertical(int j) {
    float ret = 0;
    for(int i=0;i<m_edges.size()-1;i++) {
        ret+=Length3(m_edges[i+1][j].pos-m_edges[i][j].pos);
    }
    return ret;
}

float RPiece::TotalDistHorizontal(int i) {
    float ret = 0;
    for(int j=0;j<m_edges[i].size()-1;j++) {
        ret+=Length3(m_edges[i][j+1].pos-m_edges[i][j].pos);
    }
    return ret;
}

void RPiece::FillOutTextureCoordinates() {
    assert(m_texWidth>=1);
    assert(m_texHeight>=1);
    for(int j=0;j<m_edges[0].size();j++) {
        float totalDist = TotalDistVertical(j);
        float currDist = 0;
        for(int i=0;i<m_edges.size();i++) {
            m_edges[i][j].v = currDist/totalDist*m_texHeight;
            if(i==m_edges.size()-1) break;
            currDist+=Length3(m_edges[i+1][j].pos-m_edges[i][j].pos);
        }
    }
    for(int i=0;i<m_edges.size();i++) {
        float totalDist = TotalDistHorizontal(i);
        float currDist = 0;
        for(int j=0;j<m_edges[i].size();j++) {
            m_edges[i][j].u = currDist/totalDist*m_texWidth;
            if(j==m_edges[i].size()-1) break;
            currDist+=Length3(m_edges[i][j+1].pos-m_edges[i][j].pos);
        }
    }
}


Matrix4 RPiece::FindOrientTransform(RPieceShell &rps, RPiece *other) {
    cerr<<"fot"<<endl;
    Matrix4 ret;
    ret.Identity();
    if(other==NULL) return ret;
    

    vector<Vector4> startPs = rps.getStartOrientation();
    vector<Vector4> endPs = other->m_exit;
    Vector4 toTrans = endPs[1];
    Vector4 sec1, sec2;



    //for(int i=0;i<3;i++) {
    //    PrintVector(startPs[i]);
    // }

    //for(int i=0;i<3;i++) {
    //    PrintVector(endPs[i]);

    //}
    
    Vector4 ahead_s = Normalize3(startPs[0]-startPs[1]);
    Vector4 up_s = Normalize3(startPs[2]-startPs[1]);

    Vector4 ahead_e = Normalize3(endPs[0]-endPs[1]);
    Vector4 up_e = Normalize3(endPs[2]-endPs[1]);

    //    cerr<<Angle3(ahead_s,up_s)<<endl;
    //cerr<<Angle3(ahead_e,up_e)<<endl;
    
    ret.Translation(endPs[1]);

    Vector4 axis1 = CrossProduct(up_s,up_e);
    if(Length3(axis1)==0) axis1 = ahead_e;
    float angle1 = Angle3(up_e,up_s);
    Matrix4 rotMat;
    rotMat.Rotation(angle1,axis1);
    Vector4 newSAhead = rotMat*ahead_s;
    Vector4 axis2 = CrossProduct(newSAhead,ahead_e);
    if(Length3(axis2)==0) axis2 = up_e;
    float angle2 = Angle3(ahead_e,newSAhead);
    Matrix4 toRot;
    toRot.Rotation(angle2,axis2);
    ret = ret*toRot;
    toRot.Rotation(angle1,axis1);
    ret = ret*toRot;
    toRot.Translation(-startPs[1]);
    ret = ret*toRot;
    if(ret[0][0]*0!=0) {
        PrintVector(endPs[2]);
        PrintVector(endPs[1]);
        PrintVector(endPs[0]);
        PrintVector(startPs[2]);
        PrintVector(startPs[1]);
        PrintVector(startPs[0]);
        PrintVector(up_e);
        PrintVector(up_s);
        cerr<<angle1<<endl;
        cerr<<angle2<<endl;
        cerr<<"Bad transform in RPiece"<<endl;
        exit(1);
    }
    return ret;
    
    
    




    
    /*
    sec1 = startPs[0]-startPs[1];
    sec2 = endPs[0]-endPs[1];
    //PrintVector(sec1);
    //PrintVector(sec2);

    bool noFirst = false, noSecond = false;
    
    float angle1 = Angle3(sec1,sec2);
    Vector4 axis1 = CrossProduct(sec1,sec2);

    if(Length3(axis1)==0) axis1 = CrossProduct(sec1,startPs[2]-startPs[1]);

    Matrix4 rotMat;
    rotMat.Rotation(angle1,axis1);
    //   PrintVector(axis1);
    //cerr<<angle1<<endl;
    
    startPs[2] = rotMat*(startPs[2]-startPs[1])+startPs[1];
    startPs[0] = rotMat*(startPs[0]-startPs[1])+startPs[1];
    //    PrintVector(startPs[2]);
    
    sec1 = startPs[2]-startPs[1];
    sec2 = endPs[2]-endPs[1];
    //    PrintVector(sec1);
    //PrintVector(sec2);
    float angle2 = Angle3(sec1,sec2);
    Vector4 axis2 = CrossProduct(sec1,sec2);
    if(Length3(axis2)==0) axis1 = CrossProduct(sec1,startPs[0]-startPs[1]);

    Matrix4 trans;
    trans.Translation(toTrans);
    ret = ret*trans;
    trans.Rotation(angle2,axis2);
    ret = ret*trans;
    trans.Rotation(angle1,axis1);
    ret = ret*trans;
    trans.Translation(-startPs[1]);
    ret = ret*trans;
    //PrintMatrix(ret);
    //    exit(1);
    return ret;
    */
}


//other piece is not constructed yet
void RPiece::FindAndOrientFrames(RPieceShell &rps, vector<REdge> &frames, RPiece *other, int widthSubdivision, bool tester) {
    Matrix4 transform;
    cerr <<"fof0"<<endl;
    transform.Identity();
    cerr <<"fof0.1"<<endl;
    if(!tester) {
        cerr<<"fof1"<<endl;
        transform = FindOrientTransform(rps,other);
        
        vector<Vector4> startO = rps.getStartOrientation();
        vector<Vector4> endO = rps.getEndOrientation();
        for(int i=0;i<3;i++) {
            startO[i](3) = 1;
            endO[i](3) = 1; //make homogenous
            m_entrance.push_back(transform*startO[i]);
            m_exit.push_back(transform*endO[i]);
        }
        cerr<<"fof2"<<endl;

        
    }
    cerr<<"fof3 "<<rps.size() << endl;

    for(int i=0;i<rps.size();i++) {
        vector<Vector4> points = rps[i].InterpolatePoints(widthSubdivision);
        REdge keyFrame;
        keyFrame.loopsBack = rps[i].LoopsBack();
        for(int j=0;j<points.size();j++) {
            RVertex rv;
            points[j](3) = 1; //it's supposed to be homogeneous
            rv.pos = transform*points[j];
 
            keyFrame.push_back(rv);
        }
        frames.push_back(keyFrame);
    }

    cerr<<"fof4 "<< endl;

    if(other!=NULL)
        assert((m_frames[0].size())==other->m_frames[other->m_frames.size()-1].size());

        
    for(int j=0;j<widthSubdivision;j++) {
        for(int i=0;i<frames.size();i++) {

            // if(tester&&i>0&&i<frames.size()-1) i = frames.size()-1;
            Vector4 nextSlope, prevSlope;
            if(i>0) {
                prevSlope = frames[i][j].pos-frames[i-1][j].pos;
            }
            if(i<frames.size()-1) {
                nextSlope = frames[i+1][j].pos-frames[i][j].pos;
            }
            if(i==0) {
                if(other==NULL)
                    frames[0][j].tangent = Normalize3(nextSlope);
                else {
                    int s = other->m_frames.size()-1;
                    RVertex rvOther = other->m_frames[s][j];
                    Vector4 newPos = (frames[0][j].pos+rvOther.pos)/2;
                    Vector4 newTangent = FindTangent(other->m_frames[s-1][j].pos,newPos,frames[1][j].pos);
                    frames[0][j].pos = newPos;
                    frames[0][j].tangent = newTangent;
                    other->m_frames[s][j].pos = newPos;
                    other->m_frames[s][j].tangent = newTangent;
                }
            } else if(i==frames.size()-1) {
                frames[i][j].tangent = Normalize3(prevSlope);
            } else {
                frames[i][j].tangent = FindTangent(frames[i-1][j].pos,frames[i][j].pos,frames[i+1][j].pos);
            }
            
        }
    }

    cerr<<"fof5 " << endl;


    //correct edges so it smooths out correctly without any humps
    for(int j=0;j<widthSubdivision;j++) {
        int lastI = m_frames.size()-1;
        m_frames[1][j].tangent = m_frames[0][j].tangent;
        m_frames[lastI-1][j].tangent = m_frames[lastI][j].tangent;

    }
    
    int width = frames[0].size();
    for(int i=0;i<frames.size();i++) {
        if(frames[i].loopsBack) {
            Vector4 newTan = Normalize3(frames[i][0].tangent+frames[i][width-1].tangent);
            //assert(Length3(newTan)==1);
            frames[i][0].tangent = frames[i][width-1].tangent = newTan;
        }
    }
    

}



void RPiece::Viewpoint(int i, int j) {
    //glPushMatrix();
    if(j<0) j=0;
    if(j>=m_edges[0].size()) j = m_edges[0].size()-1;
    i-=1; //first edge doesn't count
    if(i<1) i = 1;
    if(i>=m_edges.size()-1) i = m_edges.size()-1;
    RVertex r = m_edges[i][j];
    Vector4 pos = r.pos+.7*r.normal;
    Vector4 lookAt = r.pos+r.tangent+.5*r.normal;
    gluLookAt(pos[0],pos[1],pos[2],lookAt[0],lookAt[1],lookAt[2],r.normal[0],r.normal[1],r.normal[2]);
    //    OrientSelf(true);

    //glPopMatrix();
}

void RPiece::DumpGrid() {
    for(int i=0;i<m_edges.size();i++) {
        for(int j=0;j<m_edges[i].size();j++) {
            //            if(i==0)                PrintVector(m_edges[i][j].pos);
        }
    }
}

void RPiece::DisplayBall(int i, int j) {
    if(i<1) i = 1;
    if(i>=m_edges.size()-1) i = m_edges.size()-2;
    RVertex r = m_edges[i][j];
    Vector4 pos = r.pos+.7*r.normal;
    Vector4 lookAt = r.pos+r.tangent+.5*r.normal;
    //gluLookAt(pos[0],pos[1],pos[2],lookAt[0],lookAt[1],lookAt[2],r.normal[0],r.normal[1],r.normal[2]);
    
    glPushMatrix();
    //    OrientSelf();
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    glColor3f(0,0,.8);
    glTranslatef(pos[0],pos[1],pos[2]);

    //use glu function here instead
    //glutSolidSphere(.7,50,50);
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    glColor3f(.5,.5,.5);
    glDisable(GL_COLOR_MATERIAL);
        
    glPopMatrix();
    
}

/*

void RPiece::OrientSelf() { OrientSelf(false); }

void RPiece::OrientSelf(bool forView) {
      
    if(m_alignTo==NULL) {   
        return;
    }



    
    vector<Vector4> startPs = m_startAlignment;
    vector<Vector4> endPs = m_alignTo->m_endAlignment;
    Vector4 toTrans = endPs[1];
    Vector4 sec1, sec2;

    sec1 = startPs[0]-startPs[1];
    sec2 = endPs[0]-endPs[1];
    //PrintVector(sec1);
    //PrintVector(sec2);

    bool noFirst = false, noSecond = false;
    
    float angle1 = Angle3(sec1,sec2);
    Vector4 axis1 = CrossProduct(sec1,sec2);

    if(Length3(axis1)==0) axis1 = CrossProduct(sec1,startPs[2]-startPs[1]);

    Matrix4 rotMat;
    rotMat.Rotation(angle1,axis1);
    //   PrintVector(axis1);
    //cerr<<angle1<<endl;
    
    startPs[2] = rotMat*(startPs[2]-startPs[1])+startPs[1];
    startPs[0] = rotMat*(startPs[0]-startPs[1])+startPs[1];
    //    PrintVector(startPs[2]);
    
    sec1 = startPs[2]-startPs[1];
    sec2 = endPs[2]-endPs[1];
    //    PrintVector(sec1);
    //PrintVector(sec2);
    float angle2 = Angle3(sec1,sec2);
    Vector4 axis2 = CrossProduct(sec1,sec2);
    if(Length3(axis2)==0) axis1 = CrossProduct(sec1,startPs[0]-startPs[1]);

    //PrintVector(axis2);
    //cerr<<angle2<<endl;
    
    if(!forView) {
        glTranslatef(toTrans[0],toTrans[1],toTrans[2]);    
        glRotatef(180/PI*angle2,axis2[0],axis2[1],axis2[2]);
        glRotatef(180/PI*angle1,axis1[0],axis1[1],axis1[2]);
        glTranslatef(-startPs[1][0],-startPs[1][1],-startPs[1][2]);
    } else {
        glTranslatef(startPs[1][0],startPs[1][1],startPs[1][2]);
        glRotatef(-180/PI*angle1,axis1[0],axis1[1],axis1[2]);
        glRotatef(-180/PI*angle2,axis2[0],axis2[1],axis2[2]);
        glTranslatef(-toTrans[0],-toTrans[1],-toTrans[2]);    
        
    }
}

*/

void RPiece::Render(RCamera *cam) {
  //    glDisable(GL_TEXTURE_2D); 
    glPushMatrix();
    //    OrientSelf();
    //glPointSize(6);
    //glColor3f(1,0,0);
    /*
    glBegin(GL_POINTS);
    for(int i=0;i<3;i++) {
        Vector4 v = m_entrance[i];
        glVertex3f(v[0],v[1],v[2]);
        v = m_exit[i];
        glVertex3f(v[0],v[1],v[2]);
    }
    glEnd();
    */
    glDisable(GL_TEXTURE_2D);

    GLfloat mat[4] = {1,.5,0,1};
    GLfloat spec[4] = {.8,.6,.1,.3};

    glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,mat);
    glMaterialfv(GL_FRONT,GL_SPECULAR,spec);
    glMaterialf(GL_FRONT,GL_SHININESS,10);

    glColor3f(1.0,1.0,0);
    glBegin(GL_LINES);   
    for(int i=0;i<m_edges.size();i++) {
        
        for(int j=0;j<m_edges[i].size();j++) {
            glVertex3f(m_edges[i][j].pos[0],m_edges[i][j].pos[1],m_edges[i][j].pos[2]);
            Vector4 normP = m_edges[i][j].pos+m_edges[i][j].normal/2;
            glVertex3f(normP[0],normP[1],normP[2]);
        }
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
    

    glBindTexture(GL_TEXTURE_2D,m_texName);
    
    for(int j=0;j<m_edges[0].size()-1;j++) {
        glBegin(GL_TRIANGLE_STRIP);
        for(int i=0;i<m_edges.size();i++) {
            glNormal3f(m_edges[i][j+1].normal[0],m_edges[i][j+1].normal[1],m_edges[i][j+1].normal[2]);
            glTexCoord2f(m_edges[i][j+1].u,m_edges[i][j+1].v);
            glVertex3f(m_edges[i][j+1].pos[0],m_edges[i][j+1].pos[1],m_edges[i][j+1].pos[2]);
            glNormal3f(m_edges[i][j].normal[0],m_edges[i][j].normal[1],m_edges[i][j].normal[2]);
            glTexCoord2f(m_edges[i][j].u,m_edges[i][j].v);
            glVertex3f(m_edges[i][j].pos[0],m_edges[i][j].pos[1],m_edges[i][j].pos[2]);
            
        }
        glEnd();
    }
    //glEnable(GL_TEXTURE_2D);
    glPopMatrix();
}

RSubPiece::RSubPiece(vector<REdge> &edges, GLuint texName, 
		     int iStart, int iEnd,bool hasCurves) 
  : RObject()
{
    m_hasCurves = hasCurves;
    m_texName = texName;
    for(int i=iStart;i<=iEnd;i++) {
        m_edges.push_back(edges[i]);
    }
    ComputeBoundingVolume();
    m_dispList = glGenLists(1);
    if(m_dispList!=0) {
        glNewList(m_dispList,GL_COMPILE);
        //cerr<<"drawing list"<<endl;
        DoDraw(1,1,false,MAX_EDGE_LINE_WIDTH);
        glEndList();
    }
}

void RSubPiece::ComputeBoundingVolume() {
  float xmin, xmax;
  float ymin, ymax;
  float zmin, zmax;
  
  xmin = xmax = m_edges[0][0].pos[0];
  ymin = ymax = m_edges[0][0].pos[1];
  zmin = zmax = m_edges[0][0].pos[2];

  vector<REdge>::iterator it = m_edges.begin();
  while ( it != m_edges.end() ) {
    REdge::iterator edgeIter = (*it).begin();
    while ( edgeIter != (*it).end() ) {
      if ((*edgeIter).pos[0] < xmin)
	xmin = (*edgeIter).pos[0];
      else if ((*edgeIter).pos[0] > xmax)
	xmax = (*edgeIter).pos[0];
      
      if ((*edgeIter).pos[1] < ymin)
	ymin = (*edgeIter).pos[1];
      else if ((*edgeIter).pos[1] > ymax)
	ymax = (*edgeIter).pos[1];
      
      if ((*edgeIter).pos[2] < zmin)
	zmin = (*edgeIter).pos[2];
      else if ((*edgeIter).pos[2] > zmax)
	zmax = (*edgeIter).pos[2];
      edgeIter++;
    }
    it++;
  }
  m_bvol = new RAxisAlignedBoundingBox(xmin, xmax,
				       ymin, ymax,
				       zmin, zmax);
}

#define TRACK_THICKNESS .02


RSubPiece::~RSubPiece() {
    glDeleteLists(m_dispList,1);
}


void RSubPiece::DoRender(RCamera *cam) {
    //RenderBV();
    //    glDisable(GL_TEXTURE_2D); 
    //glPushMatrix();
    //    OrientSelf();
    //glPointSize(6);
    //glColor3f(1,0,0);
    /*
    glBegin(GL_POINTS);
    for(int i=0;i<3;i++) {
        Vector4 v = m_entrance[i];
        glVertex3f(v[0],v[1],v[2]);
        v = m_exit[i];
        glVertex3f(v[0],v[1],v[2]);
    }
    glEnd();
    */
    /*
    glDisable(GL_TEXTURE_2D);        
    glBegin(GL_LINES);   
    for(int i=0;i<m_edges.size();i++) {
        //if(i==0) {
        for(int j=0;j<m_edges[i].size();j++) {
            
            glVertex3f(m_edges[i][j].pos[0],m_edges[i][j].pos[1],m_edges[i][j].pos[2]);
            Vector4 normP = m_edges[i][j].pos+-m_edges[i][j].normal/2;
            glVertex3f(normP[0],normP[1],normP[2]);
            
        }
        //}
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
    */
    //    GLfloat mat[4] = {1,.5,0,1};
    //GLfloat spec[4] = {.8,.6,.1,.3};
    //    glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,mat);
    //glMaterialfv(GL_FRONT,GL_SPECULAR,spec);
    //glMaterialf(GL_FRONT,GL_SHININESS,10);
    int totalJ = m_edges[0].size()-1;
    int totalI = m_edges.size();
    float IJRatio = 1.0*totalI/totalJ;
    int middleJ = m_edges[0].size()/2;
    int middleI = m_edges.size()/2;
    float dist1 = LengthSquared3(m_edges[middleI][middleJ].pos-cam->GetPosition());
    float dist2 = LengthSquared3(m_edges[0][middleJ].pos-cam->GetPosition());
    float dist3 = LengthSquared3(m_edges[totalI-1][middleJ].pos-cam->GetPosition());
    float dist = min(dist1,min(dist2,dist3));
    float lineWidth = (1-MAX_EDGE_LINE_WIDTH)/60.0*dist+MAX_EDGE_LINE_WIDTH;
    if(lineWidth<1) lineWidth = 1;
    int lod_i_step = 1;
    int lod_j_step = 1;
    int currentArea = totalJ*totalI;
    if(dist>LOD_MIN_DIST_SQ) {
        float past = dist-LOD_MIN_DIST_SQ;
        if(past>LOD_FADE_DIST_SQ) past = LOD_FADE_DIST_SQ;
        //float a = LOD_FADE_DIST_SQ;
        //float b = currentArea - log(a);
        //int desiredArea = (int)(log(a-past)+b);
        


        ///////// linear lod
        float m = 1.0*(1.0*LOD_FADE_AREA-currentArea)/LOD_FADE_DIST_SQ;
        float b = currentArea;
        int desiredArea = (int)(m*past+b);
        /////////
        if(desiredArea<LOD_FADE_AREA) desiredArea = LOD_FADE_AREA;
        
        
        //lod_j_step = (int)(sqrt(1.0*desiredArea/IJRatio));
        //lod_i_step = (int)(IJRatio*lod_j_step);
        int wantedJ = (int)(sqrt(1.0*desiredArea/IJRatio));
        if(m_hasCurves) wantedJ = totalJ;
        int wantedI = (int)(desiredArea*1.0/wantedJ);
        //if(m_hasCurves) {
        //    wantedJ = totalJ;
        //    wantedI = max(wantedI,(int)(desiredArea*1.0/totalJ));
        //}
        if(wantedI<2) wantedI = 2;
        if(wantedJ<2) wantedJ = 2;
        lod_i_step = totalI/wantedI;
        lod_j_step = totalJ/wantedJ;

        
        if(desiredArea>=.9*currentArea) {
            desiredArea = currentArea; //for debug output later
            lod_i_step = 1;
            lod_j_step = 1;
        }
        

        if(lod_i_step<1) lod_i_step = 1;
        if(lod_j_step<1) lod_j_step = 1;
        
        //cerr<<"lod:"<<desiredArea<<"vs"<<currentArea<<endl;
        //cerr<<"total I:"<<totalI<<endl;
        //cerr<<"steps:"<<lod_i_step<<","<<lod_j_step<<endl;
        
    }

    //lod_j_step = 1;
    //    lod_i_step= 1;

    Vector4 rayStart, rayEnd;
    cam->GetViewRay(rayStart,rayEnd);
    //PrintVector(rayEnd-rayStart);
    //find better way of determining this #
    bool doCameraCull;
    if(lod_i_step*lod_j_step==1&&GetBoundingVolume()->IntersectsRay(rayStart,rayEnd)) {
        //if(m_edges[0].loopsBack) cerr<<"clipped!"<<endl;
        //glEnable(GL_CULL_FACE);
        doCameraCull = true;
    } else {
        doCameraCull = false;
        //glDisable(GL_CULL_FACE);
    }
    //glEnable(GL_CULL_FACE);
    glBindTexture(GL_TEXTURE_2D,m_texName);

    if(lod_i_step==1&&lod_j_step==1&&m_dispList!=0&&!doCameraCull) {
        //cerr<<"calling list"<<endl;
        glCallList(m_dispList);
        //cerr<<"WHOWOWOWOWO"<<endl;
    } else {
        //cerr<<"drawing manually"<<endl;
        DoDraw(lod_i_step,lod_j_step,doCameraCull,lineWidth);
    }

    //CheckOpenGLErrors(40);
    //glEnable(GL_CULL_FACE);

    //glEnable(GL_TEXTURE_2D);
    //    glPopMatrix();
}

void RSubPiece::DoDraw(int lod_i_step, int lod_j_step, bool doCameraCull, float lineWidth) {
    //cerr<<"starting sub piece "<<lod_i_step<<" "<<lod_j_step<<endl;
    //cerr<<m_edges.size()<<endl;
    //cerr<<m_edges[0].size()<<endl;
    //CheckOpenGLErrors(41);  
    glEnable(GL_CULL_FACE);
    //CheckOpenGLErrors(44);  
    for(int j=0;j<m_edges[0].size()-1;j+=lod_j_step) {
        ////CheckOpenGLErrors(43);  
        if(m_edges[0].size()-1-j<=lod_j_step) j = m_edges[0].size()-1-lod_j_step;
        glCullFace(GL_BACK);
        //CheckOpenGLErrors(52);  
        int nextJ = j+lod_j_step;
        if(nextJ>=m_edges[0].size()) nextJ = m_edges[0].size()-1;
        //if(j==0||nextJ==m_edges[0].size()-1) glDepthMask(GL_FALSE);

       

        glBegin(GL_TRIANGLE_STRIP);
        //cerr<<"starting strip:"<<endl;
        for(int i=0;i<m_edges.size();i+=lod_i_step) {
            //cerr<<i<<endl;
            if(m_edges.size()-i<=lod_i_step) i = m_edges.size()-1;
            RVertex rv1 = m_edges[i][j];
            //CheckOpenGLErrors(55);  
            
            RVertex rv2 = m_edges[i][nextJ];
            //PrintVector(rv1.pos);
            //PrintVector(rv2.pos);
            //PrintVector(rv1.normal);
            //PrintVector(rv2.normal);
            //cerr<<rv1.u<<rv1.v<<endl;
            //cerr<<rv2.u<<rv2.v<<endl;
            glNormal3f(rv2.normal[0],rv2.normal[1],rv2.normal[2]);
            glTexCoord2f(rv2.u,rv2.v);
            //CheckOpenGLErrors(54);  
            glVertex3f(rv2.pos[0],rv2.pos[1],rv2.pos[2]);
            glNormal3f(rv1.normal[0],rv1.normal[1],rv1.normal[2]);
            glTexCoord2f(rv1.u,rv1.v);
            glVertex3f(rv1.pos[0],rv1.pos[1],rv1.pos[2]);
            //CheckOpenGLErrors(53);  
        }
        //CheckOpenGLErrors(55);  
        glEnd();
        //CheckOpenGLErrors(61);  
        //glDepthMask(GL_TRUE);
        glDisable(GL_TEXTURE_2D);
        glPushAttrib(GL_LIGHTING_BIT);
        GLfloat back_amb_diff[] = {.1,.1,.1,1};
        //CheckOpenGLErrors(50);  
        glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,back_amb_diff);
        //CheckOpenGLErrors(49);  
        if(!doCameraCull) glCullFace(GL_BACK);
        else
            glCullFace(GL_FRONT_AND_BACK);
        //CheckOpenGLErrors(48);
        nextJ = j+lod_j_step;
        if(nextJ>=m_edges[0].size()) nextJ = m_edges[0].size()-1;
        //if(j==0||nextJ==m_edges[0].size()-1) glDepthMask(GL_FALSE);
        glBegin(GL_TRIANGLE_STRIP);
        for(int i=0;i<m_edges.size();i+=lod_i_step) {
            if(m_edges.size()-i<=lod_i_step) i = m_edges.size()-1;
            RVertex rv1 = m_edges[i][j];
            Vector4 pos1 = rv1.pos-rv1.normal*TRACK_THICKNESS;
            Vector4 norm1 = -rv1.normal;
            RVertex rv2 = m_edges[i][nextJ];
            Vector4 pos2 = rv2.pos-rv2.normal*TRACK_THICKNESS;
            Vector4 norm2 = -rv2.normal;
            glNormal3f(norm1[0],norm1[1],norm1[2]);
            glVertex3f(pos1[0],pos1[1],pos1[2]);
            glNormal3f(norm2[0],norm2[1],norm2[2]);
            glVertex3f(pos2[0],pos2[1],pos2[2]);
        }
        glEnd();
        //glDepthMask(GL_TRUE);
        //CheckOpenGLErrors(46);  
        glPopAttrib();
        //CheckOpenGLErrors(47);  
        glEnable(GL_TEXTURE_2D);

    }
    //CheckOpenGLErrors(42);  
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(.1,.5,.8);
    glLineWidth(lineWidth);
    //if(!doCameraCull) {
        glBegin(GL_LINE_STRIP);
        {
            for(int i=0;i<m_edges.size();i+=lod_i_step) {
                if(m_edges.size()-i<=lod_i_step) i = m_edges.size()-1;
                RVertex rv = m_edges[i][0];
                glVertex3f(rv.pos[0],rv.pos[1],rv.pos[2]);
            }
        }
        glEnd();
        glBegin(GL_LINE_STRIP);
        {
            int w = m_edges[0].size()-1;
            for(int i=0;i<m_edges.size();i+=lod_i_step) {
                if(m_edges.size()-i<=lod_i_step) i = m_edges.size()-1;
                RVertex rv = m_edges[i][w];
                glVertex3f(rv.pos[0],rv.pos[1],rv.pos[2]);
            }
        }
        glEnd();
        //}
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glCullFace(GL_BACK);
}


void RPiece::FillOutNormals() {
    for(int j=0;j<m_edges[0].size();j++) {
        
        Vector4 prevNormal;
        for(int i=0;i<m_edges.size();i++) {
            
            if(i==0||j==0||j==m_edges[0].size()-1||i==m_edges.size()-1) {

                Vector4 tan1 = m_edges[i][j].tangent;
                
                Vector4 tan2;
                if(j==0&&!m_edges[i].loopsBack) {
                    tan2 = m_edges[i][j+1].pos-m_edges[i][j].pos;
                } else if(j==m_edges[0].size()-1&&!m_edges[i].loopsBack) {
                    tan2 = m_edges[i][j].pos-m_edges[i][j-1].pos;
                } else {
                    if(j==0||j==m_edges[i].size()-1) {
                        tan2 = FindTangent(m_edges[i][m_edges[i].size()-2].pos,m_edges[i][0].pos,m_edges[i][1].pos);
                    } else
                        tan2 = FindTangent(m_edges[i][j-1].pos,m_edges[i][j].pos,m_edges[i][j+1].pos);
                }
                m_edges[i][j].normal = Normalize3(CrossProduct(tan1,tan2));
                prevNormal = m_edges[i][j].normal;
            }
            else {
                
                prevNormal = FindNormal(m_edges[i-1][j].pos,m_edges[i][j-1].pos,m_edges[i+1][j].pos,m_edges[i][j+1].pos,m_edges[i][j].pos,prevNormal);
                m_edges[i][j].normal = prevNormal;
                
            }
            
        }
    }
}

void RPiece::FillOutEdgesUsingKeyFrames(vector<REdge> &keyFrames, int width, int lengthSubdivision) {
    m_edges.resize((lengthSubdivision-1)*(keyFrames.size()-1)+1);
    for(int i=0;i<m_edges.size();i++) {
        m_edges[i].resize(width);
    }


    for(int j=0;j<width;j++) {
        
        for(int i=0;i<keyFrames.size()-1;i++) {
            bool loopBack = keyFrames[i].loopsBack&&keyFrames[i+1].loopsBack;
            RVertex start = keyFrames[i][j];
            RVertex end = keyFrames[i+1][j];
            /*
            if(keyFrames[i].loopsBack) {
                for(int ll=0;ll<keyFrames[i].size();ll++) {
                    PrintVector(keyFrames[i][ll].pos);
                }
                //                exit(1);
            }
            */
            //cerr<<"filling out:"<<endl;
            //PrintVector(start.tangent);
            //PrintVector(end.tangent);
            vector<RVertex> v = FillOutLine(lengthSubdivision,start,end);
            for(int k=0;k<v.size();k++) {
                m_edges[i*(lengthSubdivision-1)+k].loopsBack = loopBack;
                m_edges[i*(lengthSubdivision-1)+k][j] = v[k];
                //PrintVector(v[k].pos);
            }
 
            
        }
    }
}
