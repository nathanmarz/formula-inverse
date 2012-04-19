#ifndef RMORE_OBJECTS_H
#define RMORE_OBJECTS_H

#include "RLibraries.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "RObjectBase.h"
#include "md2.h"
#include <vector>

class RTrack;

class REnclosingBackground {
 public:
    virtual void Render(RCamera *viewer) = 0;

};


struct EnclosingCubeInfo {
    int textureScale;
    GLuint texture;
};


class REnclosingCube: public REnclosingBackground {
 public:
    REnclosingCube(vector<EnclosingCubeInfo> &sides, float radius, bool doClamp);

    void Render(RCamera *viewer);
    
 private:
    bool m_clamp;
    float m_rad;
    vector<EnclosingCubeInfo> m_sides;
};

class REnclosingDome: public REnclosingBackground {
 public:
    REnclosingDome(GLuint texture,float radius);

    void Render(RCamera *viewer);
    
 private:
    float m_rad;
    GLuint m_texture;
};


//with dynamic scenery, don't forget to shift bounding volumes with the object
class RScenery:public RObject {
 public:
    RScenery();
    RScenery(Vector4 pos, Vector4 up, Vector4 ahead);
    //    void Render(RCamera *cam);
    
    //make these transform bounding volumes
    void RotateAroundAhead(float degrees);
    void RotateAroundUp(float degrees);
    //*************** (above are test functions)
    
    void SetCoordinateFrame(Vector4 pos, Vector4 up, Vector4 ahead);
    Vector4 getWorldPos() { return GetTransform()*m_pos; }
    virtual Vector4 getPos() { return m_pos; }
    virtual Vector4 getUpOrientation() { return m_up; }
    virtual Vector4 getAheadOrientation() {return m_ahead; }
    void finalize(); //call this before it's used...
 protected:
    virtual RBoundVol *CreateBoundingVolume()=0;
    virtual Matrix4 GetOrientTransform();
    Vector4 m_pos;
    Vector4 m_up;
    Vector4 m_ahead;
    RVertex m_right;
    RVertex m_left;

};

class RFinishLine: public RScenery {
 public:
    RFinishLine(RTrack *track, int i);
    void Animate(int deltaTime) { }
 protected:
    RBoundVol *CreateBoundingVolume();
    void DoRender(RCamera *viewer);
 private:
    GLuint m_texture;
    float m_width;
    float m_height;
};

struct SignFlipper {
    SignFlipper() { }
    SignFlipper(int time, GLuint tex) { timeToFlip = time; texture = tex; }
    int timeToFlip;
    GLuint texture;
};

class RSign: public RScenery {
 public:
    RSign(Vector4 pos, Vector4 up, Vector4 ahead, float w, float h, GLuint texture);
    RSign(Vector4 pos, Vector4 up, Vector4 ahead, float w, float h, vector<SignFlipper> &textures);
    void Animate(int deltaTime);
 protected:
    RBoundVol *CreateBoundingVolume();
    void DoRender(RCamera *viewer);
 private:
    vector<SignFlipper> m_textures;
    float m_width;
    float m_height;
    int m_timeLeftToFlip;
    int m_currIndex;


};

/*
class RTestScenery: public RScenery {
 public:
    RTestScenery(Vector4 pos, Vector4 up, Vector4 ahead):RScenery(pos,up,ahead) { }
    void Animate(int deltaTime) { }
 protected:
    RBoundVol *CreateBoundingVolume() { return NULL; }
    void DoRender(RCamera *cam);

};
*/

class RMD2Model:public RScenery, public CMD2Model {
 public:
    RMD2Model(bool firstFrameOnly,Vector4 pos, Vector4 up, Vector4 ahead,Matrix4 initTransform, float interpolation,const char *modelPath, MD2Texture tex):RScenery(pos,up,ahead) {
        m_interpolate = interpolation;
        m_initTransform = initTransform;
        m_firstFrameOnly = firstFrameOnly;
        
        if(!Load(modelPath,tex)) {
            cerr<<"model failed to load!"<<endl;
            exit(1);
        }
        finalize();
        
    }

    void Animate(int deltaTime);
 protected:
    bool m_firstFrameOnly;
    RBoundVol *CreateBoundingVolume();
    void DoRender(RCamera *cam);
    Matrix4 GetOrientTransform() {
        Matrix4 curr = RScenery::GetOrientTransform();
        //Matrix4 scaler, rotMat;
        //scaler.Scale(m_scale,m_scale,m_scale);
        //rotMat.Rotation(PI/2,Vector4(0,1,0));
        //scaler = scaler*rotMat;
        //rotMat.Rotation(-PI/2,Vector4(1,0,0));
        //scaler = scaler*rotMat;
        return curr*m_initTransform;
    }
   

 private:
    float m_interpolate;
    Matrix4 m_initTransform;

};


#endif
