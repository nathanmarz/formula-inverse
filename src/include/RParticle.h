#ifndef _RPARTICLE_H_
#define _RPARTICLE_H_

#include "RBillBoard.h"

#include <vector>


//PARTICLE SYSTEM FIX: make it in world space, for the love of god
//convert SparkParticle to an RObject, have it create it's own bounding volume, and then make the particles billboards in world space...

//
// Each particle has this information
//
class RParticle : public RScreenBillBoard  {
 public:
  RParticle(Vector4 pos, Vector4 direction, 
	    float width, float height, GLuint texture,
	    float lifetime, Vector4 accel);
  ~RParticle();
  
  bool IsActive() { return m_active; }
  void Activate() { m_active = true; }
  void Deactivate() { m_active = false; }
  bool isDead() { return !m_active; }
  
  void Animate(int deltaTime);
  void DoRender(RCamera *viewer);

 protected:
  float m_lifetime;
  bool m_active;
  Vector4 m_direction;
  Vector4 m_acceleration;
};

class RParticleSystem : public RScenery {
 public:
  RParticleSystem(Vector4 pos, Vector4 up, Vector4 ahead,
		  int numParticles, GLuint texture, 
		  float lifetime, float fade);
  ~RParticleSystem();

  virtual void Animate(int deltaTime);
  //virtual void Render(RCamera *viewer);

 protected:
  virtual void DoRender(RCamera *viewer) { }
  RBoundVol *CreateBoundingVolume();
  vector<RParticle*> m_particles;
  Vector4 m_startPos;
  float m_startLifetime;
};

class RSparkParticleSystem : public RObject {
 public:
  RSparkParticleSystem(Vector4 pos, int numParticles, GLuint texture,
		       float lifetime, Vector4 direction);
  
  ~RSparkParticleSystem() { }
  
  virtual void Animate(int deltaTime);
  bool isDead();
  
  
 protected:
  virtual void DoRender(RCamera *viewer) { }
  RBoundVol *CreateBoundingVolume();
};

class RForceField : public RScenery {
 public:
  RForceField(Vector4 pos, Vector4 up, Vector4 ahead,
	      float width, float height, GLuint texture,
	      float lifetime, float speed);

  virtual void Animate(int deltaTime);
  bool isDead();
 protected:
  virtual void DoRender(RCamera *viewer);
  virtual RBoundVol *CreateBoundingVolume();
  float m_width;
  float m_height;
  GLuint m_texture;
  float m_lifetime;
  float m_speed;
  
};

class RSmoke : public RScreenBillBoard {
 public:
  RSmoke(Vector4 pos, Vector4 direction, float rotoRate,
	 float width, float height, GLuint texture,
	 float lifetime, float scaleRate);

  ~RSmoke();
  bool isDead() { return !m_active; }
  
  void Animate(int deltaTime);
  void FinalTransformation();

 protected:
  void DoRender(RCamera *viewer);
  bool m_active;
  float m_lifetime;
  Vector4 m_direction;
  float m_rotationRate;
  float m_rotDist;
  float m_scaleRate;
};
#endif
