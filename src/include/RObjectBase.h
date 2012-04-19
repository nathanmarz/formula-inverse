#ifndef R_OBJECT_BASE_H
#define R_OBJECT_BASE_H

#include "RLibraries.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <list>
#include "Math3D.h"

typedef Vector4 PLANE;
typedef Vector4 VERTEX;
typedef enum { STRADDLE, POSITIVE, NEGATIVE } ORIENTATION;

const int NUM_AABB_VERTICES = 8;
const int NUM_AABB_PLANES = 6;

#define LLN_AABB 0
#define LRN_AABB 1
#define ULN_AABB 2
#define URN_AABB 3
#define LLF_AABB 4
#define LRF_AABB 5
#define ULF_AABB 6
#define URF_AABB 7


class RCamera;

class RBoundVol {
 public:
    RBoundVol(Vector4 center) {
        m_origCenter = center;
        m_centerpointx = center[0];
        m_centerpointy = center[1];
        m_centerpointz = center[2];
    }
    ~RBoundVol();
    virtual bool CollidesWith(RBoundVol *other)=0;
    // Determines if this object's bounding volume
    // is in the positive, or negative side of the plane,
    // or that it straddles the plane;
    virtual ORIENTATION DetermineOrientation(PLANE *plane) = 0;
    virtual bool IntersectsRay(const Vector4 &rayStart, const Vector4 &rayEnd)=0;
    void ApplyTransform(const Matrix4 &transform);
    void SetTransform(const Matrix4 &transform);
  
  virtual void Render();
  float m_centerpointx, m_centerpointy, m_centerpointz;

  
 protected:
  Vector4 m_origCenter;
  virtual void DoTransform(const Matrix4 &transform, Vector4 &oldCenter, Vector4 &newCenter) = 0;
  virtual void DoSetTransform(const Matrix4 &transform)=0;
};

class RSphereBoundVol : public RBoundVol {
 public:
  RSphereBoundVol(float x, float y, float z, float r);
  ~RSphereBoundVol();
  ORIENTATION DetermineOrientation(PLANE *plane);
  //write this later
  bool CollidesWith(RBoundVol *other) { return false; }
  //write this later
  bool IntersectsRay(const Vector4 &rayStart, const Vector4 &rayEnd) { return false; }

 protected:
  void DoSetTransform(const Matrix4 &transform);
  void DoTransform(const Matrix4 &transform, Vector4 &oldCenter, Vector4 &newCenter);
  float m_radius;
  float m_origRadius;
};

class RAxisAlignedBoundingBox : public RBoundVol {
 public:
  RAxisAlignedBoundingBox(float xmin, float xmax,
			  float ymin, float ymax,
			  float zmin, float zmax);

  ~RAxisAlignedBoundingBox();
  ORIENTATION DetermineOrientation(PLANE *plane);
  void DumpVertexCoords() {
      cerr<<"LLN:";
      PrintVector(m_vertices[LLN_AABB]);
      cerr<<"LRN:";
      PrintVector(m_vertices[LRN_AABB]);
      cerr<<"ULN:";
      PrintVector(m_vertices[ULN_AABB]);
      cerr<<"URN:";
      PrintVector(m_vertices[URN_AABB]);
      cerr<<"LLF:";
      PrintVector(m_vertices[LLF_AABB]);
      cerr<<"LRF:";
      PrintVector(m_vertices[LRF_AABB]);
      cerr<<"ULF:";
      PrintVector(m_vertices[ULF_AABB]);
      cerr<<"URF:";
      PrintVector(m_vertices[URF_AABB]);
      
  }
  bool IntersectsRay(const Vector4 &rayStart, const Vector4 &rayEnd);
  void Render() {
      glPointSize(20);
      glBegin(GL_POINTS);
      for(int i=0;i<NUM_AABB_VERTICES;i++) {
          Vector4 v = m_vertices[i];
          glVertex3f(v[0],v[1],v[2]);
      }


      glEnd();
  }
  bool CollidesWith(RBoundVol *other);
 protected:
  PLANE GetAABBPlane(int ind1, int ind2, int ind3);
  void GetMyPlanes(vector<PLANE> &planes);
  void DoSetTransform(const Matrix4 &transform) {
      for(int i=0;i<NUM_AABB_VERTICES;i++) {
          m_vertices[i] = transform*m_origVertices[i];
      }
      
  }
  void DoTransform(const Matrix4 &transform, Vector4 &oldTransform, Vector4 &newTransform) {
      for(int i=0;i<NUM_AABB_VERTICES;i++) {
          m_vertices[i] = transform*m_vertices[i];
      }
      GetMyPlanes(m_planes);
  }
  vector<PLANE> m_planes;
  VERTEX m_vertices[NUM_AABB_VERTICES];
  VERTEX m_origVertices[NUM_AABB_VERTICES];
};


//all children must be added in the beginning
class RObject {
 public:
  RObject() { Init(); };
  RObject(RBoundVol *bv) { Init(); m_bvol = bv; }
  ~RObject() {
      if (m_bvol != NULL) {
          delete m_bvol;
      }
      for(list<RObject *>::iterator it = m_children.begin();it!=m_children.end();++it) {
          delete *it;
      }
  }
  void AddChild(RObject *o);
  void RenderBV();
  virtual bool isDead() { return false; }
  void SetTransform(const Matrix4 &newTransform);
  void ApplyTransform(const Matrix4 &toApply);
  list<RObject*> *GetChildren() { return &m_children; }
  virtual void Render(RCamera *viewer);
  virtual void Animate(int deltaTime) = 0;
  RBoundVol *GetBoundingVolume() { return m_bvol; }
  Matrix4 GetTransform();
 protected:
  RObject *m_parent;
  RBoundVol *m_bvol;
  list<RObject *> m_children;
  virtual void DoRender(RCamera *viewer) = 0;
  void SetTransformToBVRec(const Matrix4 &toSet);
  void ApplyTransformToBVRec(const Matrix4 &toApply);
 private:
  void Init() {
      m_transform.Identity();
      m_parent = NULL;
      m_bvol = NULL;
  }
  Matrix4 m_transform;
};







#endif
