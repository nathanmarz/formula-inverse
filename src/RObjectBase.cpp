#include "RObjectBase.h"
#include <GL/gl.h>
#include "RUtils.h"
#include "Level.h"

void TrackGrid::GetBoundParams(Vector4 &outPos, Vector4 &outVec) {
    if(!isBound) {
        cerr<<"asking for bound collision pos from a non-bound"<<endl;
        exit(1);
    }
    if(j==0) {
        outVec = v0.pos-v2.pos;
        outPos = (v0.pos+v2.pos)/2;
    } else {
        outVec = v1.pos-v2.pos;
        outPos = (v1.pos+v2.pos)/2;
    }
    Vector4 normal = GetNormal();
    outVec = Normalize3(CrossProduct(outVec,normal));
    
}


//normal is p3-p2 x p1-p2
PLANE RetrievePlane(Vector4 p1, Vector4 p2, Vector4 p3) {
    Vector4 norm = CrossProduct(p3-p2,p1-p2);
    return GetPlane(p2,norm);
}

RAxisAlignedBoundingBox::~RAxisAlignedBoundingBox()
{

}

PLANE RAxisAlignedBoundingBox::GetAABBPlane(int ind1, int ind2, int ind3) {
    return RetrievePlane(m_vertices[ind1],m_vertices[ind2],m_vertices[ind3]);
}

bool RAxisAlignedBoundingBox::CollidesWith(RBoundVol *other) {
    vector<PLANE> planes;
    GetMyPlanes(planes);
    for(int i=0;i<planes.size();i++) {
        if(other->DetermineOrientation(&planes[i])==NEGATIVE) return false;
    }
    return true;
}

bool RAxisAlignedBoundingBox::IntersectsRay(const Vector4 &rayStart, const Vector4 &rayEnd) {
    //cerr<<"hello"<<endl;
    //vector<PLANE> planes;
    //cerr<<"aa"<<endl;
    //    GetMyPlanes(planes);
    //cerr<<"bb"<<endl;
    //cerr<<endl;
    //cerr<<planes.size()<<endl;
    //cerr<<"PLANES:"<<endl;
    //for(int i=0;i<planes.size();i++) {
    //    PrintVector(planes[i]);
    //}
    for(int i=m_planes.size()-1;i>=0;i--) {
        
        float d1 = DistToPlane(m_planes[i],rayStart);
        float d2 = DistToPlane(m_planes[i],rayEnd);
        //cerr<<d1<<","<<d2<<endl;
        if(d1<0&&d2<0) return false;
        //PrintVector(planes[i]);
    }
    
    
    return true;
}
void RAxisAlignedBoundingBox::GetMyPlanes(vector<PLANE> &planes) {
    planes.push_back(GetAABBPlane(URN_AABB,ULN_AABB,LLN_AABB));
    planes.push_back(GetAABBPlane(LLN_AABB,ULN_AABB,ULF_AABB));
    planes.push_back(GetAABBPlane(URF_AABB,URN_AABB,LRN_AABB));
    planes.push_back(GetAABBPlane(ULF_AABB,URF_AABB,LRF_AABB));
    planes.push_back(GetAABBPlane(ULF_AABB,ULN_AABB,URN_AABB));
    planes.push_back(GetAABBPlane(LRF_AABB,LRN_AABB,LLN_AABB));
    
}

RBoundVol::~RBoundVol()
{
  //cerr << "Destroying this bounding volume..." << endl;
}

void RBoundVol::ApplyTransform(const Matrix4 &transform) {
    Vector4 oldCenter(m_centerpointx, m_centerpointy, m_centerpointz);
    Vector4 newCenter = transform*oldCenter;
    m_centerpointx = newCenter[0];
    m_centerpointy = newCenter[1];
    m_centerpointz = newCenter[2];
    DoTransform(transform,oldCenter,newCenter);
}

void RBoundVol::SetTransform(const Matrix4 &transform) {
    Vector4 newCenter = transform*m_origCenter;
    m_centerpointx = newCenter[0];
    m_centerpointx = newCenter[1];
    m_centerpointx = newCenter[2];
    DoSetTransform(transform);
}
  
void RBoundVol::Render() {
    glPushMatrix();
    glTranslatef(m_centerpointx,m_centerpointy,m_centerpointz);
    //glRotatef(90,1,0,0);
    GLUquadricObj *s = gluNewQuadric();
    gluQuadricOrientation(s,GLU_OUTSIDE);
    //glBindTexture(GL_TEXTURE_2D,m_texName);
    gluQuadricTexture(s,GL_FALSE);
    gluSphere(s,20,30,30);
    gluDeleteQuadric(s);
    glPopMatrix();
      
}

RSphereBoundVol::~RSphereBoundVol()
{

}

void RSphereBoundVol::DoSetTransform(const Matrix4 &transform) {
    Vector4 tryV = m_origCenter+Vector4(m_origRadius,0,0);
    tryV = transform*tryV;
    m_radius = Length3(tryV-Vector4(m_centerpointx,m_centerpointy,m_centerpointz));
}
void RSphereBoundVol::DoTransform(const Matrix4 &transform, Vector4 &oldCenter, Vector4 &newCenter) {
    Vector4 tryV = oldCenter+Vector4(m_radius,0,0);
    tryV = transform*tryV;
    m_radius = Length3(tryV-newCenter);
}

Matrix4 RObject::GetTransform() {
    //cerr<<"getting transform..."<<endl;
    if(m_parent==NULL) return m_transform;
    else
        return m_parent->GetTransform()*m_transform;
}

void RObject::AddChild(RObject *o) {
    o->ApplyTransformToBVRec(m_transform);
    o->m_parent = this;
    m_children.push_back(o);
}

void RObject::RenderBV() {
    if(m_bvol!=NULL)
        m_bvol->Render();
}
void RObject::SetTransform(const Matrix4 &newTransform) {
      
    //Matrix4 currInverse = Inverse(m_transform);
    //    Matrix4 id = currInverse*m_transform;
    //cerr<<endl<<"ID:"<<endl;
    //PrintMatrix(id);
    //exit(1);

    //Matrix4 toTransform = newTransform*currInverse;
    //ApplyTransform(toTransform);
    m_transform = newTransform;
    SetTransformToBVRec(newTransform);
}

void RObject::SetTransformToBVRec(const Matrix4 &toSet) {
    if(m_bvol!=NULL) {
        
        m_bvol->SetTransform(toSet);
        for(list<RObject *>::iterator it = m_children.begin();it!=m_children.end();++it) {
            (*it)->SetTransformToBVRec(toSet);
        }
    }

}

void RObject::ApplyTransform(const Matrix4 &toApply) {
    m_transform = toApply*m_transform;
    ApplyTransformToBVRec(toApply);
}
void RObject::ApplyTransformToBVRec(const Matrix4 &toApply) {
    if(m_bvol!=NULL) {
        m_bvol->ApplyTransform(toApply);
        for(list<RObject *>::iterator it = m_children.begin();it!=m_children.end();++it) {
            (*it)->ApplyTransformToBVRec(toApply);
        }
    }
}
void RObject::Render(RCamera *viewer) {
    glPushMatrix();
    glMultMatrixf(m_transform.readArray());
    //PrintMatrix(m_transform);
    DoRender(viewer);
    for(list<RObject *>::iterator it = m_children.begin();it!=m_children.end();++it) {
        (*it)->Render(viewer);
    }

    glPopMatrix();
}
