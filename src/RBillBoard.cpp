#include "RBillBoard.h"

RBillBoard::RBillBoard(Vector4 pos, float width, 
		       float height, GLuint texture,
		       VIEW_ORIENTATION orientation,
		       bool bvOn)
  : RScenery(pos,Vector4(0,1,0), Vector4(1,0,0))
{
  m_bvOn = bvOn;
  m_height = height;
  m_width = width;
  m_texture = texture;
  m_orientation = orientation;
}

void RBillBoard::DoRender(RCamera *rc)
{
  //Vector4 up = Normalize3(getUpOrientation());
  //Vector4 ahead = Normalize3(getAheadOrientation());
  //Vector4 left = Normalize3(CrossProduct(up, ahead));
  Vector4 pos = getPos();
  Vector4 upperLeft(-m_width/2,m_height/2,0);
  Vector4 upperRight(m_width/2,m_height/2,0);
  Vector4 lowerLeft(-m_width/2,-m_height/2,0);
  Vector4 lowerRight(m_width/2,-m_height/2,0);

  glDisable(GL_CULL_FACE);
  glBlendFunc(GL_ONE,GL_ONE);
  glEnable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glBegin(GL_QUADS);
  //glNormal3f(ahead[0], ahead[1], ahead[2]);
  glNormal3f(0, 0, 1);

  glTexCoord2f(0,0);
  glVertex3f(upperLeft[0], upperLeft[1], upperLeft[2]);

  glTexCoord2f(0,1);
  glVertex3f(lowerLeft[0], lowerLeft[1], lowerLeft[2]);

  glTexCoord2f(1,1);
  glVertex3f(lowerRight[0], lowerRight[1], lowerRight[2]);

  glTexCoord2f(1,0);
  glVertex3f(upperRight[0], upperRight[1], upperRight[2]);
  glEnd();

  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE);
}

RBoundVol *RBillBoard::CreateBoundingVolume()
{
  if (m_bvOn) {
    float radius;
    radius = sqrt((float)(m_width/2*m_width/2 + m_height/2*m_height/2));
    return new RSphereBoundVol(getPos()[0],
			       getPos()[1],
			       getPos()[2],
			       radius);
  }
  else
    return NULL;
}

void RScreenBillBoard::Render(RCamera *viewer)
{
  Vector4 ahead;
  switch (m_orientation) {
  case SCREEN:
    ahead = -(viewer->GetAhead());
    break;
  case VIEWPOINT:
    ahead = Normalize3(viewer->GetPosition()-getPos());
    break;
  default:
    cerr << "RScreenBillBoard::Render -- orientation undefined" << endl;
    exit(1);
  }
  SetCoordinateFrame(getPos(), viewer->GetUp(), ahead);
  FinalTransformation();
  RObject::Render(viewer);
}

void RWorldBillBoard::Render(RCamera *viewer)
{
  Vector4 ahead;
  Vector4 temp;
  Vector4 up;
  // Make the ahead vector orthogonal to the predefined
  // up vector
  switch (m_orientation) {
  case SCREEN:
    ahead = -(viewer->GetAhead());
    temp = Normalize3(CrossProduct(getUpOrientation(), ahead));
    ahead = Normalize3(CrossProduct(temp,getUpOrientation()));
    break;
  case VIEWPOINT:
      ahead = Normalize3(viewer->GetPosition()-getPos());
      temp = Normalize3(CrossProduct(getUpOrientation(), ahead));
      ahead = Normalize3(CrossProduct(temp,getUpOrientation()));
    break;
  default:
    cerr << "RWorldBillBoard::Render -- orientation undefined" << endl;
    exit(1);
  }
  SetCoordinateFrame(getPos(), getUpOrientation(), ahead);
  FinalTransformation();
  RObject::Render(viewer);
}
