#include "GL/gl.h"
#include "Math3D.h"
#include "RBSPtree.h"
#include <stdlib.h>
#include <math.h>
#include "Framework.h"
#include "RUtils.h"

#define MAX_INT (1 << 30)
using namespace std;
using namespace Math3D;



RSphereBoundVol::RSphereBoundVol(float x, float y, float z, float r):RBoundVol(Vector4(x,y,z))
{
  m_radius = r;
  m_origRadius = r;
}

RAxisAlignedBoundingBox::RAxisAlignedBoundingBox(float xmin, float xmax,
						 float ymin, float ymax,
                                                 float zmin, float zmax):RBoundVol(Vector4((xmin+xmax)/2,(ymin+ymax)/2,(zmin+zmax)/2))
{
  VERTEX temp;

  temp(0) = xmin; temp(1) = ymin; temp(2) = zmin; temp(3) = 1;
  m_vertices[LLN_AABB] = temp;
  temp(0) = xmin; temp(1) = ymin; temp(2) = zmax; temp(3) = 1;
  m_vertices[LLF_AABB] = temp;
  temp(0) = xmin; temp(1) = ymax; temp(2) = zmin; temp(3) = 1;
  m_vertices[ULN_AABB] = temp;
  temp(0) = xmin; temp(1) = ymax; temp(2) = zmax; temp(3) = 1;
  m_vertices[ULF_AABB] = temp;
  temp(0) = xmax; temp(1) = ymin; temp(2) = zmin; temp(3) = 1;
  m_vertices[LRN_AABB] = temp;
  temp(0) = xmax; temp(1) = ymin; temp(2) = zmax; temp(3) = 1;
  m_vertices[LRF_AABB] = temp;
  temp(0) = xmax; temp(1) = ymax; temp(2) = zmin; temp(3) = 1;
  m_vertices[URN_AABB] = temp;
  temp(0) = xmax; temp(1) = ymax; temp(2) = zmax; temp(3) = 1;
  m_vertices[URF_AABB] = temp;
  for(int i=0;i<NUM_AABB_VERTICES;i++) {
      m_origVertices[i] = m_vertices[i];
  }
  GetMyPlanes(m_planes);
}

ORIENTATION RAxisAlignedBoundingBox::DetermineOrientation(PLANE *plane)
{
  bool anyPositive = false;
  bool anyNegative = false;

  for (int i = 0; i < NUM_AABB_VERTICES; i++) {
      float dist = DistToPlane(*plane,m_vertices[i]);
      
      if (dist >= 0)
          anyPositive = true;
      else
          anyNegative = true;
  }

  if (anyPositive && anyNegative)
    return STRADDLE;
  else if (anyPositive && !anyNegative)
    return POSITIVE;
  else if (anyNegative && !anyPositive)
    return NEGATIVE;
  else {
    cerr << "RAxisAlignedBoundingBox::DetermineOrientation "
	 << "-- no orientation found!" << endl;
    exit(1);
  }
}

RBSPnode::RBSPnode()
{
  m_obj = NULL;
  m_positiveSide = NULL;
  m_negativeSide = NULL;
}

RBSPnode::~RBSPnode()
{
  if (m_obj != NULL) {
//     list<RObject*>::iterator it = m_obj->begin();
//     while( it != m_obj->end() ) {
//       delete (*it);
//       it++;
//     }
    delete m_obj;
  }
  if (m_positiveSide != NULL)
    delete m_positiveSide;
  if (m_negativeSide != NULL)
    delete m_negativeSide;
}

RBSPnode::RBSPnode(list<RObject*> *objsInPlane, PLANE *plane)
{
  m_obj = objsInPlane;
  m_plane(0) = (*plane)[0];
  m_plane(1) = (*plane)[1];
  m_plane(2) = (*plane)[2];
  m_plane(3) = (*plane)[3];
}

RBSPtree::RBSPtree(list<RObject*> *staticList, 
		   list<RObject*> *dynamicList)
{
  m_bspTree = ConstructTree(staticList);
  m_DynamicObjs = dynamicList;
  assert(m_DynamicObjs!=NULL);
}

RBSPtree::~RBSPtree()
{
  if (m_bspTree != NULL)
    delete m_bspTree;
  // the dynamic object list is destroyed by rworld
  //  if (m_DynamicObjs != NULL) {
//     list<RObject*>::iterator it = m_DynamicObjs->begin();
//     while (it != m_DynamicObjs->end()) {
//       delete (*it);
//       it++;
//     }
//     delete m_DynamicObjs;
//   }
}

void RBSPtree::GetViewFrustumPlanes()
{
  // Use cross products to calculate these planes

  Vector4 TLNtoTRN = Normalize3(m_frustumVertices[TOP_RIGHT_NEAR] -
				m_frustumVertices[TOP_LEFT_NEAR]);
  Vector4 TLNtoBLN = Normalize3(m_frustumVertices[BOTTOM_LEFT_NEAR] -
				m_frustumVertices[TOP_LEFT_NEAR]);
  Vector4 TLNtoTLF = Normalize3(m_frustumVertices[TOP_LEFT_FAR] -
				m_frustumVertices[TOP_LEFT_NEAR]);

  m_frustumPlanes[NEAR_P] =
    GetPlane(m_frustumVertices[TOP_LEFT_NEAR],
	     Normalize3(CrossProduct(TLNtoTRN,TLNtoBLN)));
  m_frustumPlanes[TOP] = 
    GetPlane(m_frustumVertices[TOP_LEFT_NEAR],
	     Normalize3(CrossProduct(TLNtoTLF,TLNtoTRN)));
  m_frustumPlanes[LEFT] = 
    GetPlane(m_frustumVertices[TOP_LEFT_NEAR],
	     Normalize3(CrossProduct(TLNtoBLN,TLNtoTLF)));

  Vector4 BRFtoTRF = Normalize3(m_frustumVertices[TOP_RIGHT_FAR] -
				m_frustumVertices[BOTTOM_RIGHT_FAR]);
  Vector4 BRFtoBLF = Normalize3(m_frustumVertices[BOTTOM_LEFT_FAR] -
				m_frustumVertices[BOTTOM_RIGHT_FAR]);
  Vector4 BRFtoBRN = Normalize3(m_frustumVertices[BOTTOM_RIGHT_NEAR] -
				m_frustumVertices[BOTTOM_RIGHT_FAR]);

  m_frustumPlanes[FAR_P] = 
    GetPlane(m_frustumVertices[BOTTOM_RIGHT_FAR],
	     Normalize3(CrossProduct(BRFtoTRF,BRFtoBLF)));
  m_frustumPlanes[BOTTOM] = 
    GetPlane(m_frustumVertices[BOTTOM_RIGHT_FAR],
	     Normalize3(CrossProduct(BRFtoBLF,BRFtoBRN)));
  m_frustumPlanes[RIGHT] = 
    GetPlane(m_frustumVertices[BOTTOM_RIGHT_FAR],
	     Normalize3(CrossProduct(BRFtoBRN,BRFtoTRF)));

 //  glBegin(GL_QUADS);

//   glNormal3f(BRFtoTRF[0], BRFtoTRF[1], BRFtoTRF[2]);
//   Vector4 ahead = BRFtoTRF*0.1;
//   ahead*=0;
//   glVertex3f(m_frustumVertices[BOTTOM_LEFT_NEAR][0]+ahead[0],
// 	     m_frustumVertices[BOTTOM_LEFT_NEAR][1]+ahead[1],
// 	     m_frustumVertices[BOTTOM_LEFT_NEAR][2]+ahead[2]);
//   glVertex3f(m_frustumVertices[BOTTOM_RIGHT_NEAR][0]+ahead[0],
// 	     m_frustumVertices[BOTTOM_RIGHT_NEAR][1]+ahead[1],
// 	     m_frustumVertices[BOTTOM_RIGHT_NEAR][2]+ahead[2]);
//   glVertex3f(m_frustumVertices[BOTTOM_RIGHT_FAR][0]+ahead[0],
// 	     m_frustumVertices[BOTTOM_RIGHT_FAR][1]+ahead[1],
// 	     m_frustumVertices[BOTTOM_RIGHT_FAR][2]+ahead[2]);
//   glVertex3f(m_frustumVertices[BOTTOM_LEFT_FAR][0]+ahead[0],
// 	     m_frustumVertices[BOTTOM_LEFT_FAR][1]+ahead[1],
// 	     m_frustumVertices[BOTTOM_LEFT_FAR][2]+ahead[2]);
  
//   glEnd();

//   glBegin(GL_QUADS);

//   glNormal3f(TLNtoBLN[0], TLNtoBLN[1], TLNtoBLN[2]);
//   Vector4 down = TLNtoBLN*0.1;
//   down*=0;
//   glVertex3f(m_frustumVertices[TOP_LEFT_NEAR][0]+down[0],
// 	     m_frustumVertices[TOP_LEFT_NEAR][1]+down[1],
// 	     m_frustumVertices[TOP_LEFT_NEAR][2]+down[2]);
//   glVertex3f(m_frustumVertices[TOP_RIGHT_NEAR][0]+down[0],
// 	     m_frustumVertices[TOP_RIGHT_NEAR][1]+down[1],
// 	     m_frustumVertices[TOP_RIGHT_NEAR][2]+down[2]);
//   glVertex3f(m_frustumVertices[TOP_RIGHT_FAR][0]+down[0],
// 	     m_frustumVertices[TOP_RIGHT_FAR][1]+down[1],
// 	     m_frustumVertices[TOP_RIGHT_FAR][2]+down[2]);
//   glVertex3f(m_frustumVertices[TOP_LEFT_FAR][0]+down[0],
// 	     m_frustumVertices[TOP_LEFT_FAR][1]+down[1],
// 	     m_frustumVertices[TOP_LEFT_FAR][2]+down[2]);
  
//  glEnd();

//   glBegin(GL_QUADS);
  
//   glNormal3f(TLNtoTRN[0], TLNtoTRN[1], TLNtoTRN[2]);
//   Vector4 right = TLNtoTRN*0.1;
//   right*=0;
//   glVertex3f(m_frustumVertices[TOP_LEFT_NEAR][0]+right[0],
// 	     m_frustumVertices[TOP_LEFT_NEAR][1]+right[1],
// 	     m_frustumVertices[TOP_LEFT_NEAR][2]+right[2]);
//   glVertex3f(m_frustumVertices[BOTTOM_LEFT_NEAR][0]+right[0],
// 	     m_frustumVertices[BOTTOM_LEFT_NEAR][1]+right[1],
// 	     m_frustumVertices[BOTTOM_LEFT_NEAR][2]+right[2]);
//   glVertex3f(m_frustumVertices[BOTTOM_LEFT_FAR][0]+right[0],
// 	     m_frustumVertices[BOTTOM_LEFT_FAR][1]+right[1],
// 	     m_frustumVertices[BOTTOM_LEFT_FAR][2]+right[2]);
//   glVertex3f(m_frustumVertices[TOP_LEFT_FAR][0]+right[0],
// 	     m_frustumVertices[TOP_LEFT_FAR][1]+right[1],
// 	     m_frustumVertices[TOP_LEFT_FAR][2]+right[2]);
  
//   glEnd();

//   glBegin(GL_QUADS);
  
//   glNormal3f(-TLNtoTRN[0], -TLNtoTRN[1], -TLNtoTRN[2]);
  
//   glVertex3f(m_frustumVertices[TOP_RIGHT_NEAR][0],
// 	     m_frustumVertices[TOP_RIGHT_NEAR][1],
// 	     m_frustumVertices[TOP_RIGHT_NEAR][2]);
//   glVertex3f(m_frustumVertices[BOTTOM_RIGHT_NEAR][0],
// 	     m_frustumVertices[BOTTOM_RIGHT_NEAR][1],
// 	     m_frustumVertices[BOTTOM_RIGHT_NEAR][2]);
//   glVertex3f(m_frustumVertices[BOTTOM_RIGHT_FAR][0],
// 	     m_frustumVertices[BOTTOM_RIGHT_FAR][1],
// 	     m_frustumVertices[BOTTOM_RIGHT_FAR][2]);
//   glVertex3f(m_frustumVertices[TOP_RIGHT_FAR][0],
// 	     m_frustumVertices[TOP_RIGHT_FAR][1],
// 	     m_frustumVertices[TOP_RIGHT_FAR][2]);
  
//   glEnd();


 //   GLfloat *mat_model, *mat_projection;
//   Vector4 *p;

//   glGetFloatv(GL_MODELVIEW_MATRIX, mat_model);
//   glGetFloatv(GL_PROJECTION_MATRIX, mat_projection);

//   m_modelviewMatrix.OpenGL_Matrix(mat_model);
//   m_projectionMatrix.OpenGL_Matrix(mat_projection);

//   m_frustumMatrix = m_modelviewMatrix * m_projectionMatrix;

//   p = &m_frustumPlanes[RIGHT];
//   (*p)(0) = m_frustumMatrix[3][0]-m_frustumMatrix[0][0];
//   (*p)(1) = m_frustumMatrix[3][1]-m_frustumMatrix[0][1];
//   (*p)(2) = m_frustumMatrix[3][2]-m_frustumMatrix[0][2];
//   (*p)(3) = m_frustumMatrix[3][3]-m_frustumMatrix[0][3];

//   p = &m_frustumPlanes[LEFT];
//   (*p)(0) = m_frustumMatrix[3][0]+m_frustumMatrix[0][0];
//   (*p)(1) = m_frustumMatrix[3][1]+m_frustumMatrix[0][1];
//   (*p)(2) = m_frustumMatrix[3][2]+m_frustumMatrix[0][2];
//   (*p)(3) = m_frustumMatrix[3][3]+m_frustumMatrix[0][3];

//   p = &m_frustumPlanes[BOTTOM];
//   (*p)(0) = m_frustumMatrix[3][0]+m_frustumMatrix[1][0];
//   (*p)(1) = m_frustumMatrix[3][1]+m_frustumMatrix[1][1];
//   (*p)(2) = m_frustumMatrix[3][2]+m_frustumMatrix[1][2];
//   (*p)(3) = m_frustumMatrix[3][3]+m_frustumMatrix[1][3];

//   p = &m_frustumPlanes[TOP];
//   (*p)(0) = m_frustumMatrix[3][0]-m_frustumMatrix[1][0];
//   (*p)(1) = m_frustumMatrix[3][1]-m_frustumMatrix[1][1];
//   (*p)(2) = m_frustumMatrix[3][2]-m_frustumMatrix[1][2];
//   (*p)(3) = m_frustumMatrix[3][3]-m_frustumMatrix[1][3];

//   p = &m_frustumPlanes[NEAR];
//   (*p)(0) = m_frustumMatrix[3][0]+m_frustumMatrix[2][0];
//   (*p)(1) = m_frustumMatrix[3][1]+m_frustumMatrix[2][1];
//   (*p)(2) = m_frustumMatrix[3][2]+m_frustumMatrix[2][2];
//   (*p)(3) = m_frustumMatrix[3][3]+m_frustumMatrix[2][3];

//   p = &m_frustumPlanes[FAR];
//   (*p)(0) = m_frustumMatrix[3][0]-m_frustumMatrix[2][0];
//   (*p)(1) = m_frustumMatrix[3][1]-m_frustumMatrix[2][1];
//   (*p)(2) = m_frustumMatrix[3][2]-m_frustumMatrix[2][2];
//   (*p)(3) = m_frustumMatrix[3][3]-m_frustumMatrix[2][3];

//   for (int i = 0; i < NUM_FRUSTUM_PLANES; i++)
//     Normalize4(m_frustumPlanes[i]);
} // GetViewFrustumPlanes()

RBSPnode *RBSPtree::ConstructTree(list<RObject*> *objList)
{
  RBSPnode *newRBSPnode;
  PLANE plane;
  list<RObject*> *objNegList, *objPosList, *objsInPlane;
  
  objNegList = new list<RObject*>;
  objPosList = new list<RObject*>;
  objsInPlane = new list<RObject*>;
  // Choose an object to split the plane with
  // we will select a randomly oriented plane within
  // the object
  RBSPChoosePlane(objList, &plane);
  //RBSPChoosePlaneMinSplit(objList, &plane);
  // Partition the objects along the chosen plane
  // Objects incident to the plane will be stored in this node
  //cout << "Object list size: " << objList->size() << endl;
  RBSPpartitionObjectListWithPlane(&plane, objList, objNegList,
				   objPosList, objsInPlane);
  // cout << "objNegList size: " << objNegList->size() << endl;
  //cout << "objPosList size: " << objPosList->size() << endl;
  //cout << "objsInPlane size: " << objsInPlane->size() << endl;
  assert(objList->size() == 0);
  //  assert(objsInPlane->size() > 0);

  newRBSPnode = new RBSPnode(objsInPlane, &plane);

  // construct the negative side
  if (objNegList->size() == 0)
    newRBSPnode->m_negativeSide = new EmptyRBSPnode();
  else newRBSPnode->m_negativeSide = ConstructTree(objNegList);

  // construct the positive side
  if (objPosList->size() == 0)
    newRBSPnode->m_positiveSide = new EmptyRBSPnode();
  else newRBSPnode->m_positiveSide = ConstructTree(objPosList);

  return newRBSPnode;
} // ConstructTree()


//
// Select a plane at random to partition the tree
//
void RBSPtree::RBSPChoosePlane(list<RObject*> *objList, PLANE *plane)
{
  float phi, theta;
  list<RObject*>::iterator it = objList->begin();
  int count = 0;
  int chosenItem = rand() % objList->size();
  phi = rand()%360;
  theta = rand()%360;
 
  while (count < chosenItem) {
    count++;
    it++;
  }
  (*plane)(0) = cos(theta)*cos(phi);
  (*plane)(1) = sin(theta)*sin(phi);
  (*plane)(2) = sin(theta);
  (*plane)(3) = -((*plane)[0]*(*it)->GetBoundingVolume()->m_centerpointx +
		   (*plane)[1]*(*it)->GetBoundingVolume()->m_centerpointy +
		   (*plane)[2]*(*it)->GetBoundingVolume()->m_centerpointz);
} // RBSPChoosePlane

void RBSPtree::RBSPChoosePlaneMinSplit(list<RObject*> *objList, PLANE *plane)
{
  int minCount = MAX_INT;
  RObject *chosenRoot = objList->front();
  float phi, theta;
  list<RObject*>::iterator iter;
  PLANE usedPlane;
  PLANE storedPlane;
  int ii = 0;
  
  // we can try different orientations of planes within the bounding
  // volume before we try other objects
#define MAX_CANDIDATES 100
  for (iter = objList->begin(); iter != objList->end() && ii < MAX_CANDIDATES; iter++) {
    int count = 0;

    phi = rand()%360;
    theta = rand()%360;
    usedPlane(0) = cos(theta)*cos(phi);
    usedPlane(1) = sin(theta)*sin(phi);
    usedPlane(2) = sin(theta);
    usedPlane(3) = -(usedPlane[0]*(*iter)->GetBoundingVolume()->m_centerpointx +
		 usedPlane[1]*(*iter)->GetBoundingVolume()->m_centerpointy +
		 usedPlane[2]*(*iter)->GetBoundingVolume()->m_centerpointz);
    // see how many objects straddle this plane
    list<RObject*>::iterator it = objList->begin();
    while (it != objList->end()) {
      if ((*it)->GetBoundingVolume()->DetermineOrientation(&usedPlane) 
	  == STRADDLE) {
	if ((*it) != (*iter))
	  count++;
      }
      it++;
    }
    if (count < minCount) { 
      minCount = count; 
      chosenRoot = (*iter);
      storedPlane = usedPlane;
    }
    if (count == 0) break;
  }
  (*plane)(0) = storedPlane[0];
  (*plane)(1) = storedPlane[1];
  (*plane)(2) = storedPlane[2];
  (*plane)(3) = storedPlane[3];
} // RBSPChoosePlaneMinSplit

void RBSPtree::RBSPpartitionObjectListWithPlane(PLANE *plane, 
						list<RObject*> *objList,
						list<RObject*> *objNegList,
						list<RObject*> *objPosList,
						list<RObject*> *objsInPlane)
{
  while (objList->size() > 0) {
    RObject *item = objList->front();
    objList->pop_front();
    switch (item->GetBoundingVolume()->DetermineOrientation(plane)) {
    case STRADDLE: objsInPlane->push_front(item); break;
    case POSITIVE: objPosList->push_front(item); break;
    case NEGATIVE: objNegList->push_front(item); break;
    default:
      cerr << "ERROR: RBSPpartitionObjectListWithPlane: "
	   << "Could not categorize object" << endl;
    }
  }
} // RBSPpartitionObjectListWithPlane

void RBSPtree::DrawPlanes()
{
  DrawPlanesHelper(m_bspTree, 10);
}

void RBSPtree::DrawPlanesHelper(RBSPnode *node, int depth)
{
  if (depth == 0)
    return;
  else
    depth--;

  if (node->IsEmpty())
    return;

  PLANE plane = node->m_plane;
  GLfloat zcoord = -(plane[0]*2 + plane[1]*2 + plane[3])/plane[2];
  glBegin(GL_QUADS);
  glNormal3f(plane[0], plane[1], plane[2]);
  glVertex3f(2,2,zcoord);
  zcoord = -(plane[0]*-2 + plane[1]*2 + plane[3])/plane[2];
  glVertex3f(-2,2,zcoord);
  zcoord = -(plane[0]*-2 + plane[1]*-2 + plane[3])/plane[2];
  glVertex3f(-2,-2,zcoord);
  zcoord = -(plane[0]*2 + plane[1]*-2 + plane[3])/plane[2];
  glVertex3f(2,-2,zcoord);
  glEnd();
  DrawPlanesHelper(node->m_positiveSide, depth);
  DrawPlanesHelper(node->m_negativeSide, depth);
}

ORIENTATION RSphereBoundVol::DetermineOrientation(PLANE *plane)
{
  float distance;
  
  distance = 
    (*plane)[0]*m_centerpointx +
    (*plane)[1]*m_centerpointy +
    (*plane)[2]*m_centerpointz +
    (*plane)[3];

  if ( (fabs(distance)-m_radius) <= 0 )
    return STRADDLE;
  else if (distance > 0)
    return POSITIVE;
  else
    return NEGATIVE;
}


void RBSPtree::RBSPtreeRender(RCamera *rc)
{
  // ViewFrustumPlanes is calculated from the vertices
  // so these must be called in this order
  GetFrustumVertices(rc);
  GetViewFrustumPlanes();
  //  DrawFrustumVertices(rc);
  //DrawPlanes();
  RBSPtreeRenderHelper(m_bspTree, rc);
  // Render the dynamic objects after
  if (m_DynamicObjs != NULL) {

      
    list<RObject*>::iterator it = m_DynamicObjs->begin();
    while (it != m_DynamicObjs->end()) {
      bool drawP = true;
      
      for(int i = 0; i < NUM_FRUSTUM_PLANES; i++) {
	switch ((*it)->GetBoundingVolume()->DetermineOrientation(&m_frustumPlanes[i])) {
	case STRADDLE: break;
	case POSITIVE: break;
	case NEGATIVE: drawP = false; break;
	}
      }
      if (drawP) {
	(*it)->Render(rc);
      }
      it++;    
    }
  }
}

void RBSPtree::DrawFrustumVertices(RCamera *rc)
{
  Vector4 ahead = rc->GetAhead();
  glPushMatrix();
  glBegin(GL_QUADS);

  glNormal3f(ahead[0], ahead[1], ahead[2]);
  ahead = ahead*0.01;

  glVertex3f(m_frustumVertices[0][0]+ahead[0],
	     m_frustumVertices[0][1]+ahead[1],
	     m_frustumVertices[0][2]+ahead[2]);
  glVertex3f(m_frustumVertices[1][0]+ahead[0],
	     m_frustumVertices[1][1]+ahead[1],
	     m_frustumVertices[1][2]+ahead[2]);
  glVertex3f(m_frustumVertices[2][0]+ahead[0],
	     m_frustumVertices[2][1]+ahead[1],
	     m_frustumVertices[2][2]+ahead[2]);
  glVertex3f(m_frustumVertices[3][0]+ahead[0],
	     m_frustumVertices[3][1]+ahead[1],
	     m_frustumVertices[3][2]+ahead[2]);
  
  glEnd();
  glPopMatrix();
}

// 
// Renders objects in a BSP tree with view frustum culling
//
void RBSPtree::RBSPtreeRenderHelper(RBSPnode *node, RCamera *rc)
{
  if (node==NULL||node->IsEmpty()) return;

  list<RObject*>::iterator it = node->m_obj->begin();
  while (it != node->m_obj->end()) {
    // only render the items if they are inside or straddle the view
    // frustum
    bool drawP = true;
    //    int posCount = 0;
    for(int i = 0; i < NUM_FRUSTUM_PLANES; i++) {
      switch ((*it)->GetBoundingVolume()->DetermineOrientation(&m_frustumPlanes[i])) {
      case STRADDLE:  break;
      case POSITIVE: break;
      case NEGATIVE: drawP = false; break;
	//default:
	//cerr<<"culled object"<<endl;
      }
      //      if (drawP)
      //break;
    }
    if ( drawP) {
      (*it)->Render(rc);
    }
    it++;
  }
//     (*it)->Render(rc);
//     it++;
//   }

  if (viewFrustumSide(node->m_plane) == POSITIVE) {
    RBSPtreeRenderHelper(node->m_positiveSide, rc);
  }
  else if (viewFrustumSide(node->m_plane) == NEGATIVE) {
    RBSPtreeRenderHelper(node->m_negativeSide, rc);
  }
  else {
    assert(viewFrustumSide(node->m_plane) == STRADDLE);
    RBSPtreeRenderHelper(node->m_positiveSide, rc);
    RBSPtreeRenderHelper(node->m_negativeSide, rc);
  }
}

ORIENTATION RBSPtree::viewFrustumSide(PLANE &plane)
{
  bool anyPositive = false;
  bool anyNegative = false;

  for (int i = 0; i < NUM_FRUSTUM_VERTICES; i++) {
    float dist = 
      m_frustumVertices[i][0]*plane[0] +
      m_frustumVertices[i][1]*plane[1] +
      m_frustumVertices[i][2]*plane[2] +
      plane[3];

    if (dist >= 0)
      anyPositive = true;
    else
      anyNegative = true;
  }

  if (anyPositive && anyNegative)
    return STRADDLE;
  else if (anyPositive && !anyNegative)
    return POSITIVE;
  else if (anyNegative && !anyPositive)
    return NEGATIVE;
  else {
    cerr << "RBSPtree::viewFrustumSide -- no orientation found!" << endl;
    exit(1);
  }
}

void RBSPtree::GetFrustumVertices(RCamera *rc)
{
  Vector4 temp;
  Vector4 camPos = rc->GetPosition();
  Vector4 camAhead = rc->GetAhead();
  Vector4 camUp = rc->GetUp();
  Vector4 camLeft = Normalize3(CrossProduct(camUp, camAhead));
  GLdouble fovy = rc->GetFovy();
  GLdouble ratio = rc->GetRatio();
  GLdouble nearDist = rc->GetNearDist();
  GLdouble farDist = rc->GetFarDist();
  GLdouble vertDistNear = nearDist*tan(fovy/360*PI);
  GLdouble vertDistFar = farDist*tan(fovy/360*PI);

  //
  // Vertices for the front plane
  //

  // Calculate the top,left,near vertex
  // to move up, we need to move in the up direction a certain amount of units
  temp = camPos + (vertDistNear*ratio)*camLeft;
  temp = temp + vertDistNear*camUp;
  temp = temp + nearDist*camAhead;
  m_frustumVertices[TOP_LEFT_NEAR] = temp;

  // Calculate the top,right,near vertex
  temp = camPos - (vertDistNear*ratio)*camLeft;
  temp = temp + vertDistNear*camUp;
  temp = temp + nearDist*camAhead;
  m_frustumVertices[TOP_RIGHT_NEAR] = temp;  

  // Calculate the bottom,left,near vertex
  temp = camPos + (vertDistNear*ratio)*camLeft;
  temp = temp - vertDistNear*camUp;
  temp = temp + nearDist*camAhead;
  m_frustumVertices[BOTTOM_LEFT_NEAR] = temp;  

  // Calculate the bottom,right,near vertex
  temp = camPos - (vertDistNear*ratio)*camLeft;
  temp = temp - vertDistNear*camUp;
  temp = temp + nearDist*camAhead;
  m_frustumVertices[BOTTOM_RIGHT_NEAR] = temp;  

  //
  // Vertices for the far plane
  //

  // Calculate the left,top,far vertex
  temp = camPos + (vertDistFar*ratio)*camLeft;
  temp = temp + vertDistFar*camUp;
  temp = temp + farDist*camAhead;
  m_frustumVertices[TOP_LEFT_FAR] = temp;

  // Calculate the right,top,far vertex
  temp = camPos - (vertDistFar*ratio)*camLeft;
  temp = temp + vertDistFar*camUp;
  temp = temp + farDist*camAhead;
  m_frustumVertices[TOP_RIGHT_FAR] = temp;

  // Calculate the left,bottom,far vertex
  temp = camPos + (vertDistFar*ratio)*camLeft;
  temp = temp - vertDistFar*camUp;
  temp = temp + farDist*camAhead;
  m_frustumVertices[BOTTOM_LEFT_FAR] = temp;

  // Calculate the right,bottom,far vertex
  temp = camPos - (vertDistFar*ratio)*camLeft;
  temp = temp - vertDistFar*camUp;
  temp = temp + farDist*camAhead;
  m_frustumVertices[BOTTOM_RIGHT_FAR] = temp;

//   cout << "Camera Position: " << flush;
//   PrintVector(camPos);
//   cout << endl;
//   cout << "Left Vector: " << flush;
//   PrintVector(camLeft);
//   cout << endl;
//   cout << "Up Vector: " << flush;
//   PrintVector(camUp);
//   cout << endl;
//   for (int i = 0; i < NUM_FRUSTUM_VERTICES; i++) {
//     cout << "Vector: " << i << endl;
//     PrintVector(m_frustumVertices[i]);
//     cout << endl;
//   }
//   cout << "Done:" <<endl;
}
