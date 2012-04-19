#include "RObjects.h"
#include "RUtils.h"
#include "Framework.h"

#define FINISH_HEIGHT 2


RSign::RSign(Vector4 pos, Vector4 up, Vector4 ahead, float w, float h, GLuint texture)
    :RScenery(pos,up,ahead)
{
    m_width = w;
    m_height = h;
    SignFlipper sf;
    sf.texture = texture;
    m_timeLeftToFlip = sf.timeToFlip = 100000; //doesn't matter
    m_currIndex = 0;
    m_textures.push_back(sf);
    finalize();
}

RSign::RSign(Vector4 pos, Vector4 up, Vector4 ahead, float w, float h, vector<SignFlipper> &textures)
    :RScenery(pos,up,ahead)
{
    m_width = w;
    m_height = h;
    m_textures = textures;
    m_currIndex = 0;
    m_timeLeftToFlip = m_textures[0].timeToFlip;
    finalize();
}

void RSign::Animate(int deltaTime) {
    m_timeLeftToFlip-=deltaTime;
    if(m_timeLeftToFlip<=0) {
        m_currIndex++;
        if(m_currIndex>=m_textures.size()) m_currIndex = 0;
        m_timeLeftToFlip = m_textures[m_currIndex].timeToFlip;
    }
}

RBoundVol *RSign::CreateBoundingVolume() {
    return new RAxisAlignedBoundingBox(-m_width/2-.1,m_width/2+.1,-m_height/2-.1,m_height/2+.1,-.2,.2);
}

void RSign::DoRender(RCamera *viewer) {
    Vector4 upperLeft(-m_width/2,m_height/2,0);
    Vector4 upperRight(m_width/2,m_height/2,0);
    Vector4 lowerLeft(-m_width/2,-m_height/2,0);
    Vector4 lowerRight(m_width/2,-m_height/2,0);

    glBindTexture(GL_TEXTURE_2D,m_textures[m_currIndex].texture);

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

    glDisable(GL_TEXTURE_2D);

    glPushAttrib(GL_LIGHTING_BIT);
    glEnable(GL_COLOR_MATERIAL);
    glColor3f(.06,.06,.06);
    
    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    glVertex3f(upperLeft[0], upperLeft[1], upperLeft[2]);
    glVertex3f(upperRight[0], upperRight[1], upperRight[2]);
    glVertex3f(lowerRight[0], lowerRight[1], lowerRight[2]);
    glVertex3f(lowerLeft[0], lowerLeft[1], lowerLeft[2]);
    
    glEnd();

    glPopAttrib();

    glEnable(GL_TEXTURE_2D);

}


RFinishLine::RFinishLine(RTrack *track, int i) {
    track->GetEdgeVertices(i,m_right,m_left);
    Vector4 averageNorm = Normalize3(m_right.normal+m_left.normal);
    Vector4 toRight = m_right.pos-m_left.pos;
    averageNorm = Normalize3(CrossProduct(CrossProduct(toRight,averageNorm),toRight));
    Vector4 pos = (m_right.pos+averageNorm*FINISH_HEIGHT+m_left.pos+averageNorm*FINISH_HEIGHT)/2;
    
    m_width = Length3(toRight);
    m_height = .7;
    m_texture = RResources::GetTexture(-3);
    Vector4 ahead = CrossProduct(toRight,averageNorm);
    m_pos = pos;
    m_ahead = ahead;
    m_up = averageNorm;
    finalize();
}

RBoundVol *RFinishLine::CreateBoundingVolume() {
    return new RAxisAlignedBoundingBox(-m_width/2,m_width/2,-m_height/2-FINISH_HEIGHT,m_height/2,-.2,.2);
}



void RFinishLine::DoRender(RCamera *viewer) {
    Vector4 upperLeft(-m_width/2,m_height/2,0);
    Vector4 upperRight(m_width/2,m_height/2,0);
    Vector4 lowerLeft(-m_width/2,-m_height/2,0);
    Vector4 lowerRight(m_width/2,-m_height/2,0);

    int lastTex = (int)(m_width/m_height);
    glDisable(GL_CULL_FACE);
    
    glBindTexture(GL_TEXTURE_2D,m_texture);
    
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    
    glTexCoord2f(0,0);
    glVertex3f(upperLeft[0], upperLeft[1], upperLeft[2]);
    
    glTexCoord2f(0,1);
    glVertex3f(lowerLeft[0], lowerLeft[1], lowerLeft[2]);
    
    glTexCoord2f(lastTex,1);
    glVertex3f(lowerRight[0], lowerRight[1], lowerRight[2]);
    
    glTexCoord2f(lastTex,0);
    glVertex3f(upperRight[0], upperRight[1], upperRight[2]);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glPushAttrib(GL_LIGHTING_BIT);    
    glEnable(GL_COLOR_MATERIAL);
    glColor3f(0,0,0);
    GLUquadricObj *cyl = gluNewQuadric();
    gluQuadricDrawStyle(cyl,GLU_FILL);
    gluQuadricNormals(cyl,GLU_SMOOTH);
    glPushMatrix();
    glTranslatef(m_width/2,-FINISH_HEIGHT,0);
    glRotatef(-90,1,0,0);
    gluCylinder(cyl,.08,.08,FINISH_HEIGHT+m_height/2,16,4);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-m_width/2,-FINISH_HEIGHT,0);
    glRotatef(-90,1,0,0);
    gluCylinder(cyl,.08,.08,FINISH_HEIGHT+m_height/2,16,4);
    glPopMatrix();
    glPopAttrib();
    //glDisable(GL_COLOR_MATERIAL);

    

    gluDeleteQuadric(cyl);
    glEnable(GL_TEXTURE_2D);    
    glEnable(GL_CULL_FACE);
}


//correct this for orientation - also, what about dynamic objects!! (there should be a transform call in RBoundVol to move it - it should take a Matrix4).
RBoundVol *RMD2Model::CreateBoundingVolume() {
    float xmin,xmax,ymin,ymax,zmin,zmax;
    int vertexToTry = numVertices*numFrames;
    if(m_firstFrameOnly) {
        vertexToTry = numVertices;
    }
    xmin = xmax = vertexList[0].point[0];
    ymin = ymax = vertexList[0].point[1];
    zmin = zmax = vertexList[0].point[2];
    for(int i=1;i<vertexToTry;i++) {
        vector_t v = vertexList[i];
        xmin = min(v.point[0],xmin);
        xmax = max(v.point[0],xmax);
        ymin = min(v.point[1],ymin);
        ymax = max(v.point[1],ymax);
        zmin = min(v.point[2],zmin);
        zmax = max(v.point[2],zmax);
    }
    return new RAxisAlignedBoundingBox(xmin,xmax,ymin,ymax,zmin,zmax);
}



void RMD2Model::DoRender(RCamera *cam) {
    //Animate(0,numFrames-1,m_interpolate);
    //cerr<<"rendering md2"<<endl;
    RenderCurrentFrame();
}

void RMD2Model::Animate(int deltaTime) {
    
    CMD2Model::Animate(0,numFrames-1,m_interpolate);
}

RScenery::RScenery() {
    m_pos = Vector4(0,0,0);
    m_up = StandardUpVector();
    m_ahead = StandardAheadVector();
}

RScenery::RScenery(Vector4 pos, Vector4 up, Vector4 ahead) : RObject() {
    m_pos = pos;
    Vector4 right = CrossProduct(ahead,up);
    m_up = Normalize3(CrossProduct(right,ahead));
    m_ahead = Normalize3(ahead);
}


void RScenery::RotateAroundAhead(float degrees) {
    Matrix4 rotMat;
    rotMat.Rotation(PI/180.0*degrees,m_ahead);
    SetCoordinateFrame(m_pos,Normalize3(rotMat*m_up),m_ahead);
}

void RScenery::RotateAroundUp(float degrees) {
    Matrix4 rotMat;
    rotMat.Rotation(PI/180.0*degrees,m_up);
    SetCoordinateFrame(m_pos,m_up,Normalize3(rotMat*m_ahead));
}


void RScenery::finalize() {
    m_bvol = CreateBoundingVolume();
    Matrix4 transform = GetOrientTransform();
    SetTransform(transform);
}

void RScenery::SetCoordinateFrame(Vector4 pos, Vector4 up, Vector4 ahead) {
  
    m_pos = pos;
    m_ahead = Normalize3(ahead);
    m_up = Normalize3(up);
    Vector4 upNormalizer = CrossProduct(ahead,m_up);
    m_up = Normalize3(CrossProduct(upNormalizer,ahead));
    Matrix4 transform = GetOrientTransform();
    SetTransform(transform);

}


bool IsZeroLength(Vector4 &v) {
    if(v[0]==0&&v[1]==0&&v[2]==0) return true;
    else return false;
}


Matrix4 RScenery::GetOrientTransform() {
    Matrix4 ret;
    ret.Translation(m_pos);
    //return ret;
    Vector4 axis1 = CrossProduct(StandardUpVector(),m_up);
    if(IsZeroLength(axis1)) axis1 = m_ahead;
    float angle1 = Angle3(m_up,StandardUpVector());
    Matrix4 rotMat;
    rotMat.Rotation(angle1,axis1);
    Vector4 newSAhead = rotMat*StandardAheadVector();
    Vector4 axis2 = CrossProduct(newSAhead,m_ahead);
    if(IsZeroLength(axis2)) axis2 = m_up;
    float angle2 = Angle3(m_ahead,newSAhead);

    Matrix4 toRot;
    toRot.Rotation(angle2,axis2);
    ret = ret*toRot;
    toRot.Rotation(angle1,axis1);
    ret = ret*toRot;
    //cerr<<"orient transform:"<<endl;
    //PrintMatrix(ret);
    return ret;
}


/*
void RScenery::Render(RCamera *cam) {
    ((RAxisAlignedBoundingBox *)m_bvol)->Render();
    glPushMatrix();
    
    //glBegin(GL_LINES);
    //glVertex3f(m_pos[0],m_pos[1],m_pos[2]);
    //Vector4 a = m_pos+m_ahead*20;
    //glVertex3f(a[0],a[1],a[2]);
    //glVertex3f(m_pos[0],m_pos[1],m_pos[2]);
    //Vector4 n = m_pos+m_up*10;
    //glVertex3f(n[0],n[1],n[2]);
    //glEnd();
    
    
    glTranslatef(m_pos[0],m_pos[1],m_pos[2]);
    Vector4 axis1 = CrossProduct(StandardUpVector(),m_up);
    if(Length3(axis1)==0) axis1 = m_ahead;
    float angle1 = Angle3(m_up,StandardUpVector());
    Matrix4 rotMat;
    rotMat.Rotation(angle1,axis1);
    Vector4 newSAhead = rotMat*StandardAheadVector();
    Vector4 axis2 = CrossProduct(newSAhead,m_ahead);
    float angle2 = Angle3(m_ahead,newSAhead);

    glRotatef(180.0/PI*angle2,axis2[0],axis2[1],axis2[2]);
    glRotatef(180.0/PI*angle1,axis1[0],axis1[1],axis1[2]);
    
    DoRender(cam);
    glPopMatrix();
}
*/
