//#include "Framework.h"
#include "RBillBoard.h"
#include "RParticle.h"

RParticle::RParticle(Vector4 pos, Vector4 direction, 
		     float width, float height, GLuint texture,
		     float lifetime, Vector4 accel)
  : RScreenBillBoard(pos, width, height, texture, SCREEN, false)
{
  m_direction = direction;
  m_lifetime = lifetime;
  m_acceleration = accel;
  m_active = true;
  m_bvol = NULL;
  
  //m_bvol = CreateBoundingVolume();
}

RParticle::~RParticle()
{

}

//
// Moves the particle
//
void RParticle::Animate(int deltaTime)
{
  assert(m_bvol==NULL);
  if (!IsActive())
    return;

  m_lifetime -= deltaTime;

  if (m_lifetime <= 0) {
    Deactivate();
    
    return;
  }

  SetCoordinateFrame(getPos()+m_direction, 
 		     getUpOrientation(),
 		     getAheadOrientation());
  m_direction = m_direction + m_acceleration;
}

//
// Draws the particle after its been moved
//
void RParticle::DoRender(RCamera *viewer)
{
  glPushAttrib(GL_TEXTURE_BIT);
  glDisable(GL_LIGHTING);
  
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  if (m_active) {
    glDepthMask(GL_FALSE);
    RScreenBillBoard::DoRender(viewer);
    glDepthMask(GL_TRUE);
    RScreenBillBoard::DoRender(viewer);
  }
  
  glEnable(GL_LIGHTING);
  
  glPopAttrib();
}


bool RSparkParticleSystem::isDead() {
    for(list<RObject *>::iterator it = m_children.begin();it!=m_children.end();++it) {
        RScenery *s = (RScenery *) (*it);
        if(!s->isDead()) return false;
    }
    return true;
}

RParticleSystem::RParticleSystem(Vector4 pos, Vector4 up, Vector4 ahead,
				 int numParticles, GLuint texture, 
				 float lifetime, float fade)
  : RScenery(pos, up, ahead)
{
  RParticle *prt;
  
  for (int i = 0; i < numParticles; i++) {
    // Generate a random direction
    float phi = rand()%360;
    float theta = rand()%360;
    Vector4 dir = Vector4(cos(theta)*cos(phi), 
			  sin(theta)*sin(phi),
			  sin(theta));
    dir = dir/100;
    Vector4 left = Vector4(-0.001,0,0);
    Vector4 right = Vector4(0.001,0,0);
    left = left*0;
    right = right*0;
    // Generate a random acceleration
    if (i%2 == 0) {
      prt = new RParticle(pos, dir, .1, .1, texture,
			  lifetime, right);
    }
    else {
      prt = new RParticle(pos, dir, .1, .1, texture,
			  lifetime, left);
    }
    
    AddChild(prt);
  }
}

RParticleSystem::~RParticleSystem()
{
    
}

//
// Moves all of the individual particles
//
void RParticleSystem::Animate(int deltaTime)
{
  list<RObject*> *children = GetChildren();
  list<RObject*>::iterator iter = children->begin();
  
  while ( iter != children->end() ) {
    (*iter)->Animate(deltaTime);
    iter++;
  }
}

RBoundVol *RParticleSystem::CreateBoundingVolume()
{
  // Calculate where the furthest a particle could be
  return new RSphereBoundVol(getPos()[0], getPos()[1], getPos()[2], 10);
}

Vector4 ArbitraryNormal(Vector4 other) {
    Vector4 ret = other;
    if(other[0]!=0) {
        ret(2)++;
    } else {
        ret(0)++;
    }
    ret = CrossProduct(ret,other);
    return ret;
}



RSparkParticleSystem::RSparkParticleSystem(Vector4 pos, int numParticles, 
					   GLuint texture, float lifetime, 
					    Vector4 direction)
{
  m_bvol = new RSphereBoundVol(pos[0], pos[1], pos[2], 2);
  RParticle *prt;
  float size = .02;
  for (int i = 0; i < numParticles; i++) {
    // all the particles go in the same direction but slightly jittered
      Vector4 jitterDir = Vector4(.1-2*(rand()%100)*1.0/1000,
					    .1-2*(rand()%100)*1.0/1000,
					    .1-2*(rand()%100)*1.0/1000);

      
      Vector4 pPos = pos+ .5*Vector4(.125-rand()%100*1.0/400,
					    .125-rand()%100*1.0/400,
					    .125-rand()%100*1.0/400);

    prt = new RParticle(pPos, jitterDir, size, size, texture,
			lifetime, Vector4(0,0,0));
    AddChild(prt);
    int count=0;
    while(rand()%2==0) {
        prt = new RParticle(pPos, jitterDir, size, size, texture,
                            lifetime, Vector4(0,0,0));
        AddChild(prt);
        count++;
        if(count==4) break;
    }
  }
}

void RSparkParticleSystem::Animate(int deltaTime)
{
  list<RObject*> *children = GetChildren();
  list<RObject*>::iterator iter = children->begin();

  while ( iter != children->end() ) {
    (*iter)->Animate(deltaTime);
    iter++;
  }
}

RBoundVol *RSparkParticleSystem::CreateBoundingVolume()
{
  return NULL;
  // Calculate where the furthest a particle could be
  
}

RForceField::RForceField(Vector4 pos, Vector4 up, Vector4 ahead,
			 float width, float height, GLuint texture,
			 float lifetime, float speed)
  : RScenery(pos, up, ahead)
{
  m_height = height;
  m_width = width;
  m_texture = texture;
  m_lifetime = lifetime;
  m_speed = speed;

  finalize();
}

void RForceField::Animate(int deltaTime)
{
  if (m_lifetime <= 0)
    return;

  m_lifetime -= deltaTime;
  // Scale the texture by changing the width/height
  m_width +=m_speed*deltaTime;
  m_height +=m_speed*deltaTime;
}

bool RForceField::isDead()
{
  return (m_lifetime <= 0);
}

void RForceField::DoRender(RCamera *viewer)
{
  
  if (m_lifetime <= 0)
    return;

  glPushAttrib(GL_FOG_BIT);
  glDisable(GL_FOG);
  
  glDepthMask(GL_FALSE);
  //Vector4 ahead = Normalize3(getAheadOrientation());
  Vector4 upperLeft(-m_width/2,m_height/2,0);
  Vector4 upperRight(m_width/2,m_height/2,0);
  Vector4 lowerLeft(-m_width/2,-m_height/2,0);
  Vector4 lowerRight(m_width/2,-m_height/2,0);

  glDisable(GL_CULL_FACE);
  glBlendFunc(GL_ONE,GL_ONE);
  glEnable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glBegin(GL_QUADS);
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
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  glPopAttrib();
}

RBoundVol *RForceField::CreateBoundingVolume()
{
  //
  return new RSphereBoundVol(0, 0, 0, 5);
}

RSmoke::RSmoke(Vector4 pos, Vector4 direction, float rotoRate,
	       float width, float height, GLuint texture,
	       float lifetime, float scaleRate)
  : RScreenBillBoard(pos, width, height, texture, SCREEN, true)
{
  m_direction = direction;
  m_rotationRate = rotoRate;
  m_lifetime = lifetime;
  m_rotDist = 0;
  m_active = true;
  m_scaleRate = scaleRate;
  finalize();
}

void RSmoke::Animate(int deltaTime)
{
  if (!m_active)
    return;
  m_rotDist += m_rotationRate*deltaTime;
  m_width += m_scaleRate*deltaTime;
  m_height += m_scaleRate*deltaTime;

  m_lifetime -= deltaTime;
  if (m_lifetime <= 0)
    m_active = false;
}

void RSmoke::FinalTransformation()
{
  // this will probably never occur
  if (!m_active)
    return;
  RotateAroundAhead(m_rotDist);
}

void RSmoke::DoRender(RCamera *viewer)
{
  glDepthMask(GL_FALSE);
  if (m_active) {
    RScreenBillBoard::DoRender(viewer);
    //    RScreenBillBoard::DoRender(viewer);
  }
  glDepthMask(GL_TRUE);
}
