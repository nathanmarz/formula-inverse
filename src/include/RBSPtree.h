#ifndef _RBSPTREE_H_
#define _RBSPTREE_H_

#include "Math3D.h"
#include <list>
#include "assert.h"
#include "RObjectBase.h"

//better way to mutually link Framework and RBSPtree?
class RCamera;

using namespace Math3D;
using namespace std;


enum NODE_TYPE { PARTITION_NODE='p', IN_NODE='i', OUT_NODE='o' };

enum frustumVertices { TOP_LEFT_NEAR = 0, TOP_RIGHT_NEAR, BOTTOM_RIGHT_NEAR,
	       BOTTOM_LEFT_NEAR, TOP_LEFT_FAR, TOP_RIGHT_FAR,
	       BOTTOM_RIGHT_FAR, BOTTOM_LEFT_FAR };


enum frustumPlanes { TOP =0, BOTTOM, LEFT, RIGHT, NEAR_P, FAR_P };


const int NUM_FRUSTUM_PLANES = 6;
const int NUM_FRUSTUM_VERTICES = 8;


class RBSPnode {
 public:
  RBSPnode();
  RBSPnode(list<RObject*> *objsInPlane, PLANE *plane);
  ~RBSPnode();
  virtual bool IsEmpty() { return false; }
  // The plane selected to divide the tree.
  // We must store this since we have no other
  // way of determining it, as opposed to polygons
  // where we can always calculate its plane.
  PLANE m_plane;
  // Nodes in this plane
  list<RObject*> *m_obj;
  // Nodes on either side of the plane
  RBSPnode *m_positiveSide, *m_negativeSide;
};

class EmptyRBSPnode : public RBSPnode {
 public:
  EmptyRBSPnode() : RBSPnode() { }
  virtual bool IsEmpty() { return true; }
};

class RBSPtree {
 public:
  ~RBSPtree();
  RBSPtree(list<RObject*> *staticList, list<RObject*> *dynamicList);
  void GetViewFrustumPlanes();
  void RBSPChoosePlane(list<RObject*> *objList, PLANE *plane);
  void RBSPChoosePlaneMinSplit(list<RObject*> *objList, PLANE *plane);
  void RBSPpartitionObjectListWithPlane(PLANE *plane, 
					list<RObject*> *objList,
					list<RObject*> *objNegList, 
					list<RObject*> *objPosList,
					list<RObject*> *objsInPlane);
  void RBSPtreeRender(RCamera *rc);
  RBSPnode *ConstructTree(list<RObject*> *objList);
  void DrawPlanes();
  ORIENTATION viewFrustumSide(PLANE &plane);
  void DrawFrustumVertices(RCamera *rc);
  void AddDynamicEffect(RObject *o) { m_DynamicObjs->push_back(o); }
 protected:
  void DrawPlanesHelper(RBSPnode *node, int depth);
  void RBSPtreeRenderHelper(RBSPnode *node, RCamera *rc);
  void GetFrustumVertices(RCamera *rc);
  Matrix4 m_modelviewMatrix;
  Matrix4 m_projectionMatrix;
  Matrix4 m_frustumMatrix;
  Vector4 m_frustumPlanes[NUM_FRUSTUM_PLANES];
  Vector4 m_frustumVertices[NUM_FRUSTUM_VERTICES];

  list<RObject*> *m_DynamicObjs;
  RBSPnode *m_bspTree;
};


#endif // _RBSPTREE_H_
