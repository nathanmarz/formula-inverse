#ifndef _RBILLBOARD_H_
#define _RBILLBOARD_H_

#include "RObjects.h"
#include "Framework.h"

typedef enum { SCREEN, VIEWPOINT } VIEW_ORIENTATION;

class RBillBoard : public RScenery {
 public:
  RBillBoard(Vector4 pos, float width, 
	     float height, GLuint texture,
	     VIEW_ORIENTATION orientation,
	     bool bvOn);

  virtual void FinalTransformation() { }
 protected:
  virtual void DoRender(RCamera *viewer);
  RBoundVol *CreateBoundingVolume();
  GLuint m_texture;
  float m_width, m_height;
  VIEW_ORIENTATION m_orientation;
  bool m_bvOn;
};

//
// Screen-Aligned Billboard
//
// The up vector is from the camera itself.
// The normal vector can be selected to either
// be screen parallel or perpendicular to
// the ray from the viewpoint to the billboard
class RScreenBillBoard : public RBillBoard {
 public:
  RScreenBillBoard(Vector4 pos, float width, 
		   float height, GLuint texture,
		   VIEW_ORIENTATION orientation,
		   bool bvOn)
    : RBillBoard(pos, width, height, texture, orientation, bvOn) { }
  
  virtual void Animate(int deltaTime) { };
  virtual void Render(RCamera *viewer);
};

//
// World-Oriented Billboard
//
// The up vector stays constant and is relative to
// the world, not the screen
class RWorldBillBoard : public RBillBoard {
 public:
  RWorldBillBoard(Vector4 pos, float width,
		  float height, GLuint texture,
		  VIEW_ORIENTATION orientation,
		  bool bvOn)
    : RBillBoard(pos, width, height, texture, orientation, bvOn) { }

  virtual void Animate(int deltaTime) { };
  virtual void Render(RCamera *viewer);
};

#endif
