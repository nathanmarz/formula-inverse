#ifndef RUTILS_H
#define RUTILS_H

#include <RLibraries.h>

#define DEFAULT_HERMITE 4.35

struct TrackGrid;

void CheckOpenGLErrors(int i);
void SetHermiteParam(float h);
unsigned int GetLightConstant(int lightNum);
void DisableAllLights();
void TrimString(string &s);
//trims it first
vector<string> SplitString(string s);
bool ReadFileUntilTerminateLine(ifstream &file, const string &terminator, vector<string> &ret);

Vector4 FindTangent(Vector4 p1, Vector4 p2, Vector4 p3);
Vector4 FindPreTangentNormal(Vector4 p1,Vector4 p2, Vector4 p3);
Vector4 FindNormal(Vector4 p1,Vector4 p2, Vector4 p3, Vector4 p4, Vector4 mid, Vector4 currNormal);
/*
//deprecated
void RecFillOutLine(vector<RVertex> &points,int lIndex1, int lIndex2, RVertex start, RVertex end);
*/

vector<RVertex> LinearlyInterpolate(int totalPoints, RVertex start, RVertex end);
//cubic hermite curve implementation
vector<RVertex> FillOutLine(int totalPoints,RVertex start, RVertex end);
float DistToPlane(Vector4 plane, Vector4 testPt);
Vector4 GetPlane(Vector4 pos, Vector4 normal);
void ProjectOntoPlane(Vector4 plane,Vector4 realPos,Vector4 realDir,Vector4 &projPos,Vector4 &projDir);
Vector4 ProjectPointOntoPlane(Vector4 plane, Vector4 point);
Vector4 ProjectVectorOntoVector(Vector4 toProject, Vector4 onto);

void SurfaceInterpolation(TrackGrid tg, Vector4 triPos, Vector4 &interpPos, Vector4 &interpNorm);
Vector4 StandardUpVector();
Vector4 StandardAheadVector();
float PointTriDistance(Vector4 tri1, Vector4 tri2, Vector4 tri3, Vector4 point, float &projDist);
bool PointInTriangle(Vector4 tri1, Vector4 tri2, Vector4 tri3, Vector4 point);
float PointRayDistance(Vector4 x1, Vector4 x2, Vector4 pt);
float PointLineDistance(Vector4 end1, Vector4 end2, Vector4 pt);
void ClosestPointOnTri(Vector4 tri1, Vector4 tri2, Vector4 tri3, Vector4 point, Vector4 &closest, float &distance);
void DrawPlane(Vector4 &plane);
#endif
