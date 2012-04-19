#include <RUtils.h>
#include "Level.h"
#include <fstream>


float HERMITE_PARAM;

Vector4 ProjectVectorOntoVector(Vector4 toProject, Vector4 onto) {
    return DotProduct3(toProject,onto)/DotProduct3(onto,onto)*onto;
}

void CheckOpenGLErrors(int i)
{
  GLenum errCode;
  const GLubyte *errString;
  
  if ((errCode = glGetError()) != GL_NO_ERROR) {
    errString = gluErrorString(errCode);
    cerr << "OpenGL Error: " << errString << endl;
    cerr<<i<<endl;
    exit(1);
  }
}

void DisableAllLights() {
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4);
    glDisable(GL_LIGHT5);
    glDisable(GL_LIGHT6);
    glDisable(GL_LIGHT7);
}

unsigned int GetLightConstant(int lightNum) {
    switch(lightNum) {
    case 0:
        return GL_LIGHT0;
    case 1:
        return GL_LIGHT1;
    case 2:
        return GL_LIGHT2;
    case 3:
        return GL_LIGHT3;
    case 4:
        return GL_LIGHT4;
    case 5:
        return GL_LIGHT5;
    case 6:
        return GL_LIGHT6;
    case 7:
        return GL_LIGHT7;
    default:
        cerr<<"invalid light specified for getLigghtConstant"<<endl;
        exit(1);
    }
}

Vector4 StandardUpVector() {return Vector4(0,1,0); }
Vector4 StandardAheadVector() {return Vector4(0,0,1); }
bool ReadFileUntilTerminateLine(ifstream &file, const string &terminator, vector<string> &ret) {
    while(true) {
        string curLine;
        getline(file,curLine);
        if(file.fail()) return false;
        TrimString(curLine);
        if(curLine==terminator) break;
        ret.push_back(curLine);
    }
    return true;
}


void TrimString(string &s) {
  int start = 0;
  for(start;start<s.size();start++) {
    if(s[start]!=' ') break;
  }
  int end = s.size()-1;
  for(end;end>=0;end--) {
    if(s[end]!=' ' && s[end]!='\r') break;
  }
  s = s.substr(start, end-start+1);
}

//trims it first
vector<string> SplitString(string s) {
  TrimString(s);
  cerr << s << endl;
  vector<string> ret;
  int start = 0;
  while(true) {
    int end = s.find(' ', start);
    if(end!=string::npos) {
      string subs = s.substr(start, end-start);
      ret.push_back(subs);
    } else {
      string subs = s.substr(start, s.size()-start);
      ret.push_back(subs);
      break;
    }
    start = end;
    while(s[start]==' ') start++;
  }
  return ret;
}


Vector4 FindTangent(Vector4 p1, Vector4 p2, Vector4 p3) {
    Vector4 a = Normalize3(p2-p1);
    Vector4 b = Normalize3(p3-p2);
    Vector4 c = a+b;
    Vector4 ret = (DotProduct3(b,c)/DotProduct3(c,c))*c;
    return Normalize3(ret);
    
    
}

Vector4 FindPreTangentNormal(Vector4 p1,Vector4 p2, Vector4 p3) {
    Vector4 a = Normalize3(p1-p2);
    Vector4 b = Normalize3(p3-p2);
    Vector4 norm = a+b;
    if(Length3(norm)==0) return FindTangent(p1,p2,p3);
    norm = Normalize3(norm);
    if(Length3(p2-(p1-norm))<Length3(p2-(p1+norm))) norm*=-1;
    return norm;
    
}

Vector4 FindNormal(Vector4 p1,Vector4 p2, Vector4 p3, Vector4 p4, Vector4 mid, Vector4 currNormal) {
    Vector4 a = FindTangent(p1,mid,p3);
    Vector4 b = FindTangent(p2,mid,p4);

    Vector4 norm = Normalize3(CrossProduct(a,b));

    if(DotProduct3(norm,currNormal)<=0) norm*=-1;
    return norm;
}

/*
//deprecated
void RecFillOutLine(vector<RVertex> &points,int lIndex1, int lIndex2, RVertex start, RVertex end) {
        points[lIndex1] = start;
        points[lIndex2] = end;
        if(abs(lIndex1-lIndex2)<=1) return;
        RVertex forward, backward;
        float jump = 1*Length3(end.pos-start.pos)/(lIndex2-lIndex1);
        forward.pos = start.pos+start.tangent*jump;
        backward.pos = end.pos-end.tangent*jump;
        forward.tangent = FindTangent(start.pos,forward.pos,backward.pos);
        backward.tangent = FindTangent(forward.pos,backward.pos,end.pos);
        RecFillOutLine(points,lIndex1+1, lIndex2-1, forward,backward);


}
*/

vector<RVertex> LinearlyInterpolate(int totalPoints, RVertex start, RVertex end) {
    vector<RVertex> ret;
    Vector4 slope = (end.pos-start.pos)/(totalPoints-1);
    for(int i=0;i<totalPoints;i++) {
        RVertex v;
        v.pos = start.pos+slope*i;
        v.tangent = Normalize3(slope);
        ret.push_back(v);
    }
    return ret;
}

void SetHermiteParam(float h) {
    //if(HERMITE_PARAM!=h) cerr<<"hermiteParamChanged"<<endl;
    HERMITE_PARAM = h;
}

void HermiteInterpolate(RVertex start, RVertex end, float proportion, RVertex &retVertex) {
    float tanWeight = HERMITE_PARAM;
    
    Matrix4 hermite;
    float mat[] = {2,-3,0,1,-2,3,0,0,1,-2,1,0,1,-1,0,0};
    hermite.C_Matrix(mat); 
    float weight = proportion;
    Vector4 u(pow(weight,3),pow(weight,2),weight,1);
    Vector4 lin = hermite*u;
    retVertex.pos= lin[0]*start.pos+lin[1]*end.pos+tanWeight*lin[2]*start.tangent+tanWeight*lin[3]*end.tangent;
    retVertex.tangent= FindTangent(start.pos,retVertex.pos,end.pos);
    retVertex.normal = FindPreTangentNormal(start.pos,retVertex.pos,end.pos);
    //retVertex.normal = Normalize3(start.normal+end.normal);
    if(DotProduct3(retVertex.normal,start.normal)<0) retVertex.normal*=-1;
    if(retVertex.pos[0]*0!=0) {
        cerr<<"hermite problem"<<endl;
        PrintVector(lin);
        PrintVector(start.pos);
        PrintVector(end.pos);
        PrintVector(start.tangent);
        PrintVector(end.tangent);
        int *a = (int *)667676;
        delete a;
        exit(1);
    }
}


//cubic hermite curve implementation
vector<RVertex> FillOutLine(int totalPoints,RVertex start, RVertex end) {

    //affects curvature (more cubic the higher this is)
    //    float tanWeight = 4;

    
    //    Matrix4 hermite;
    // float mat[] = {2,-3,0,1,-2,3,0,0,1,-2,1,0,1,-1,0,0};
    // hermite.C_Matrix(mat);
    vector<RVertex> ret;
    //cerr<<"tangents:"<<endl;
    //PrintVector(start.tangent);
    //PrintVector(end.tangent);
    for(int i=0;i<totalPoints;i++) {
        float weight = 1.0*i/(totalPoints-1);
    
        //    Vector4 u(pow(weight,3),pow(weight,2),weight,1);
        //Vector4 lin = hermite*u;
        RVertex rv;
        HermiteInterpolate(start,end,weight,rv);
        //        rv.pos = lin[0]*start.pos+lin[1]*end.pos+tanWeight*lin[2]*start.tangent+tanWeight*lin[3]*end.tangent;
        ret.push_back(rv);
    }
    for(int j=0;j<ret.size();j++) {
        if(j==0) ret[j].tangent = start.tangent;
        else if(j==ret.size()-1) ret[j].tangent = end.tangent;
        else
            ret[j].tangent = FindTangent(ret[j-1].pos,ret[j].pos,ret[j+1].pos);
    }
    
    if(ret[0].pos[0]*0!=0) {
        cerr<<"in line code"<<endl;
        PrintVector(start.pos);
        PrintVector(end.pos);
        exit(1);
    }
    //    RecFillOutLine(ret,0,totalPoints-1,start,end);
    return ret;
}

float DistToPlane(Vector4 plane, Vector4 testPt) {
    plane = Normalize4(plane);
    return (DotProduct3(testPt,plane)+plane[3])/Length3(plane);
}

Vector4 GetPlane(Vector4 pos, Vector4 normal) {
    float d = -DotProduct3(pos,normal);
    normal(3) = d;
    return normal;
}

Vector4 ProjectPointOntoPlane(Vector4 plane, Vector4 point) {
    float dist = DistToPlane(plane,point);
    return point-plane*dist;
    //    float d = plane[3];
    //float k = (-d-DotProduct3(point,plane))/DotProduct3(plane,plane);
    //return point+k*plane;
}

void ProjectOntoPlane(Vector4 plane,Vector4 realPos,Vector4 realDir,Vector4 &projPos,Vector4 &projDir) {
    projPos = ProjectPointOntoPlane(plane,realPos);
    plane(3)=0;
    projDir = ProjectPointOntoPlane(plane,realDir);
 
    //projDir = Normalize3(projDirPoint-projPos);
}

void NormalizeTangentsIntoLine(RVertex &rv1, RVertex &rv2) {
    Vector4 tan = rv2.pos-rv1.pos;
    Vector4 temp = CrossProduct(tan,rv1.normal);
    rv1.tangent = CrossProduct(rv1.normal,temp);
    temp = CrossProduct(tan,rv2.normal);
    rv2.tangent = CrossProduct(rv2.normal,temp);
}
/*
void SurfaceInterpolation(TrackGrid tg, Vector4 triPos, Vector4 &interpPos, Vector4 &interpNorm) {
    NormalizeTangentsIntoLine(tg.v1,tg.v2);
    Vector4 z = tg.v2.pos-tg.v1.pos;
    Vector4 t = triPos-tg.v0.pos;
    Vector4 x = tg.v0.pos;
    Vector4 y = tg.v1.pos;
    float b = 1/(z[0]*t[1]-z[1]*t[0])*(x[0]*t[1]-x[1]*t[0]+y[1]*t[0]-y[0]*t[1]);
    Vector4 intersectPoint = y+z*b;
    
    RVertex interVertex;
    HermiteInterpolate(tg.v1,tg.v2,b,interVertex);
    interVertex.normal = Normalize3(b*tg.v2.normal+(1-b)*tg.v1.normal);
    //interVertex.pos = intersectPoint; // hack take this out
    NormalizeTangentsIntoLine(interVertex,tg.v0);
    float a = Length3(triPos-intersectPoint)/Length3(tg.v0.pos-intersectPoint);
    RVertex ret;
    HermiteInterpolate(interVertex,tg.v0,a,ret);
    interVertex.normal = Normalize3(a*tg.v0.normal+(1-a)*interVertex.normal);

    interpPos = ret.pos;
    interpNorm = ret.normal;
    //cerr<<endl;
    //cerr<<a<<endl;
    //cerr<<b<<endl;
    //    exit(1);
}
*/

void SurfaceInterpolation(TrackGrid tg, Vector4 triPos, Vector4 &interpPos, Vector4 &interpNorm) {
    interpPos = triPos;
    Vector4 z = tg.v2.pos-tg.v1.pos;
    Vector4 t = triPos-tg.v0.pos;
    Vector4 x = tg.v0.pos;
    Vector4 y = tg.v1.pos;
    float b = 1/(z[0]*t[1]-z[1]*t[0])*(x[0]*t[1]-x[1]*t[0]+y[1]*t[0]-y[0]*t[1]);
    //check for singularities and use alternate solutions in these cases
    if(b*0!=0) {
        b = 1/(z[1]*t[2]-z[2]*t[1])*(x[1]*t[2]-x[2]*t[1]+y[2]*t[1]-y[1]*t[2]);
    }
    if(b*0!=0) {
        b = 1/(z[2]*t[0]-z[0]*t[2])*(x[2]*t[0]-x[0]*t[2]+y[0]*t[2]-y[2]*t[0]);
    }
    if(b*0!=0) {
        //cerr<<endl;
        //PrintVector(z);
        //PrintVector(t);
        //PrintVector(triPos);
        //PrintVector(tg.v0.pos);
        //cerr<<"bad surface interpolation"<<endl;
        //exit(1);
        b=0;
    }
    Vector4 intersectPoint = y+z*b;
    Vector4 intersectNormal = Normalize3(tg.v2.normal*b+(1-b)*tg.v1.normal);
    float a = Length3(triPos-intersectPoint)/Length3(tg.v0.pos-intersectPoint);
    interpNorm = Normalize3(a*tg.v0.normal+(1-a)*intersectNormal);
    //PrintVector(interpNorm);
    if(!(interpNorm[0]<=3||interpNorm[0]>=3)) {
        cerr<<"surface interpolate: "<<endl;
        cerr<<b<<endl;
        cerr<<a<<endl;
        PrintVector(triPos);
        PrintVector(z);
        PrintVector(t);
        PrintVector(x);
        PrintVector(y);
        cerr<<(z[0]*t[1]-z[1]*t[0])<<endl;
    }
    
}

void DrawPlane(Vector4 &plane)
{
    const int scale = 20;
  GLfloat zcoord = -(plane[0]*scale + plane[1]*scale + plane[3])/plane[2];

  glBegin(GL_QUADS);
  glNormal3f(plane[0], plane[1], plane[2]);
  glVertex3f(scale,scale,zcoord);
  zcoord = -(plane[0]*-scale + plane[1]*scale + plane[3])/plane[2];
  glVertex3f(-scale,scale,zcoord);
  zcoord = -(plane[0]*-scale + plane[1]*-scale + plane[3])/plane[2];
  glVertex3f(-scale,-scale,zcoord);
  zcoord = -(plane[0]*scale + plane[1]*-scale + plane[3])/plane[2];
  glVertex3f(scale,-scale,zcoord);
  glEnd();

}


float PointLineDistance(Vector4 end1, Vector4 end2, Vector4 pt) {
    
    Vector4 a = end2-end1;
    Vector4 b = pt-end1;
    Vector4 par = CrossProduct(a,b);
    if(Length3(par)==0) return 0;
    Vector4 norm = CrossProduct(par,a);
    Vector4 plane = GetPlane(end1,norm);
    return fabs(DistToPlane(plane,pt));
}

float PointRayDistance(Vector4 x1, Vector4 x2, Vector4 pt) {
    float d1 = PointLineDistance(x1,x2,pt);
    float d2 = Length3(pt-x1);
    float d3 = Length3(pt-x2);
    return min(d1,min(d2,d3));
}

bool PointInTriangle(Vector4 tri1, Vector4 tri2, Vector4 tri3, Vector4 point) {
    Vector4 a = tri2-tri1;
    Vector4 b = tri3-tri1;
    Vector4 c = tri3-tri2;
    Vector4 norm = CrossProduct(b,a);
    Vector4 plane1 = GetPlane(tri1,CrossProduct(norm,b));
    Vector4 plane2 = GetPlane(tri1,CrossProduct(a,norm));
    Vector4 plane3 = GetPlane(tri3,CrossProduct(c,norm));
    return DistToPlane(plane1,point)>=0&&DistToPlane(plane2,point)>=0&&DistToPlane(plane3,point)>=0;
}

float PointTriDistance(Vector4 tri1, Vector4 tri2, Vector4 tri3, Vector4 point, float &projDist) {
    Vector4 plane = GetPlane(tri1,CrossProduct(tri3-tri1,tri2-tri1));
    Vector4 projPoint = ProjectPointOntoPlane(plane,point);
    float horizDist;
    if(PointInTriangle(tri1,tri2,tri3,projPoint)) {
        horizDist = 0;
    }
    else {
        float d1 = PointRayDistance(tri1,tri2,projPoint);
        float d2 = PointRayDistance(tri1,tri3,projPoint);
        float d3 = PointRayDistance(tri2,tri3,projPoint);
        horizDist = min(d1,min(d2,d3));
    }
    float vertDist = Length3(point-projPoint);
    projDist = horizDist;
    return sqrt(vertDist*vertDist+horizDist*horizDist);

}

void ProjectPointOntoLineSegment(Vector4 end1, Vector4 end2, Vector4 pt, Vector4 &proj) {
    Vector4 a = end2-end1;
    Vector4 b = pt-end1;
    Vector4 projected = DotProduct3(a,b)/DotProduct3(a,a)*a;
    float segLength = LengthSquared3(a);
    float projLength = LengthSquared3(projected);
    if(projLength>=segLength) proj = end2;
    else if(DotProduct3(projected,a)<0) proj = end1;
    else {
        proj = projected+end1;
    }
}


void ClosestPointOnTri(Vector4 tri1, Vector4 tri2, Vector4 tri3, Vector4 point, Vector4 &closest, float &distance) {
    Vector4 plane = GetPlane(tri1,CrossProduct(tri3-tri1,tri2-tri1));
    Vector4 projPoint = ProjectPointOntoPlane(plane,point);
    float projDist = Length3(projPoint-point);
    if(PointInTriangle(tri1,tri2,tri3,projPoint)) {
        distance = projDist;
        closest = projPoint;
    } else {
        float bestDistance;
        Vector4 bestPoint;
        ProjectPointOntoLineSegment(tri1,tri2,projPoint,bestPoint);
        bestDistance = Length3(bestPoint-projPoint);

        Vector4 tryPt;
        ProjectPointOntoLineSegment(tri2,tri3,projPoint,tryPt);
        float tryDist = Length3(tryPt-projPoint);
        if(tryDist<bestDistance) {
            bestPoint = tryPt;
            bestDistance = tryDist;
        }
        ProjectPointOntoLineSegment(tri1,tri3,projPoint,tryPt);
        tryDist = Length3(tryPt-projPoint);
        if(tryDist<bestDistance) {
            bestPoint = tryPt;
            bestDistance = tryDist;
        }
        distance = sqrt(bestDistance*bestDistance+projDist*projDist);
        closest = bestPoint;
        
        
    }

}
