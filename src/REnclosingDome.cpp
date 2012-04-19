#include "RObjects.h"
#include "RLibraries.h"
#include "Framework.h"

#define GL_CLAMP_TO_EDGE 0x812F

REnclosingDome::REnclosingDome(GLuint texture, float radius) {
    m_rad = radius;
    m_texture = texture;
}

void REnclosingDome::Render(RCamera *viewer) {
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D,m_texture);
    glPushMatrix();
    Vector4 pos = viewer->GetPosition();
    glTranslatef(pos[0],pos[1],pos[2]);
    glRotatef(90,1,0,0);
    GLUquadricObj *s = gluNewQuadric();
    gluQuadricOrientation(s,GLU_INSIDE);
    gluQuadricTexture(s,GL_TRUE);
    gluSphere(s,m_rad,50,50);
    gluDeleteQuadric(s);
    glPopMatrix();
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
}


REnclosingCube::REnclosingCube(vector<EnclosingCubeInfo> &sides, float radius, bool doClamp) {
    m_rad = radius;
    m_sides = sides;
    m_clamp = doClamp;
}

void REnclosingCube::Render(RCamera *viewer) {

    
    glDepthMask(GL_FALSE);
    //glPushAttrib(GL_FOG_BIT);
    //glDisable(GL_FOG);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    //glDisable(GL_BLEND);
    glPushMatrix();
    Vector4 pos = viewer->GetPosition();
    glTranslatef(pos[0],pos[1],pos[2]);
    
    //glRotatef(90,1,0,0);
    //    GLUquadricObj *s = gluNewQuadric();
    //gluQuadricOrientation(s,GLU_INSIDE);
    
    //gluQuadricTexture(s,GL_TRUE);
    //gluSphere(s,m_rad,30,30);
    //gluDeleteQuadric(s);
    float c = m_rad;
    float c2 = c;
    int t;
    GLuint tex;

    
    tex = m_sides[0].texture;
    t = m_sides[0].textureScale;
    glBindTexture(GL_TEXTURE_2D,tex);
    if(m_clamp) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBegin(GL_QUADS);
    //glNormal3f(0,1,0);

    glTexCoord2f(0,t);
    glVertex3f(-c2,-c,-c2);

    glTexCoord2f(t,t);
    glVertex3f(-c2,-c,c2);

    glTexCoord2f(t,0);
    glVertex3f(c2,-c,c2);

    glTexCoord2f(0,0);
    glVertex3f(c2,-c,-c2);
    glEnd();
    
    
    tex = m_sides[1].texture;
    t = m_sides[1].textureScale;
    glBindTexture(GL_TEXTURE_2D,tex);
    if(m_clamp) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBegin(GL_QUADS);
    //glNormal3f(1,0,0);
    glTexCoord2f(0,t);
    glVertex3f(-c,-c2,c2);
    glTexCoord2f(0,0);
    glVertex3f(-c,c2,c2);
    glTexCoord2f(t,0);
    glVertex3f(-c,c2,-c2);
    glTexCoord2f(t,t);
    glVertex3f(-c,-c2,-c2);
    glEnd();
    
    
    
    tex = m_sides[2].texture;
    t = m_sides[2].textureScale;
    glBindTexture(GL_TEXTURE_2D,tex);
    if(m_clamp) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBegin(GL_QUADS);
    //glNormal3f(0,-1,0);

    glTexCoord2f(0,t);
    glVertex3f(c,c,-c);

    glTexCoord2f(t,t);
    glVertex3f(c,c,c);

    glTexCoord2f(t,0);
    glVertex3f(-c,c,c);

    glTexCoord2f(0,0);
    glVertex3f(-c,c,-c);

    glEnd();
    

    
    tex = m_sides[3].texture;
    t = m_sides[3].textureScale;
    glBindTexture(GL_TEXTURE_2D,tex);
    if(m_clamp) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBegin(GL_QUADS);
    //glNormal3f(-1,0,0);

        glTexCoord2f(t,t);
    glVertex3f(c,-c2,c2);

    glTexCoord2f(t,0);
    glVertex3f(c,c2,c2);

    glTexCoord2f(0,0);
    glVertex3f(c,c2,-c2);

    glTexCoord2f(0,t);
    glVertex3f(c,-c2,-c2);
    glEnd();
    

    
    tex = m_sides[4].texture;
    t = m_sides[4].textureScale;
    glBindTexture(GL_TEXTURE_2D,tex);
    if(m_clamp) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBegin(GL_QUADS);
    //glNormal3f(0,0,1);
    glTexCoord2f(0,0);
    glVertex3f(-c,c,-c);

    glTexCoord2f(t,0);
    glVertex3f(c,c,-c);

    glTexCoord2f(t,t);
    glVertex3f(c,-c,-c);

    glTexCoord2f(0,t);
    glVertex3f(-c,-c,-c);
    glEnd();
    

    
    tex = m_sides[5].texture;
    t = m_sides[5].textureScale;
    glBindTexture(GL_TEXTURE_2D,tex);
    if(m_clamp) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBegin(GL_QUADS);
    //glNormal3f(0,0,-1);

    glTexCoord2f(t,t);
    glVertex3f(-c,-c,c);

    glTexCoord2f(0,t);
    glVertex3f(c,-c,c);

    glTexCoord2f(0,0);
    glVertex3f(c,c,c);

    glTexCoord2f(t,0);
    glVertex3f(-c,c,c);
    glEnd();
    
    
    glPopMatrix();
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
    //glPopAttrib();
    glEnable(GL_CULL_FACE);
    //glClear(GL_DEPTH_BUFFER_BIT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

}
