#include "RPhysics.h"
#include "Framework.h"

static bool collision;
#define DRAG_CONSTANT .75
#define ROLLING_RESISTANCE 10
#define BRAKE_CONSTANT 8
#define FRICTION_CONST 4


Vector4 P_ProjectVector(Vector4 orig, Vector4 onto) {
    float orig_length = Length3(orig);
    if(orig_length==0) return orig;
    float onto_length = Length3(onto);
    float cos_theta = DotProduct3(orig,onto)/(orig_length*onto_length);
    return cos_theta*onto_length*onto;
}

void RPhysics::ProcessPlayerCollision(RPlayer *other) {
    m_boostLeft = 0;
    other->physicsEngine->m_boostLeft = 0;
    Vector4 velDiff = m_velocity-other->physicsEngine->m_velocity;

    Vector4 initImp = -25*velDiff;
    AddImpulse(initImp);
    other->physicsEngine->AddImpulse(-initImp);
    /*
    if(DotProduct3(m_velocity,other->physicsEngine->m_velocity)>0) {
        if(Length3(m_velocity)-Length3(other->physicsEngine->m_velocity)<0) {
            Vector4 imp = 18*Normalize3(other->physicsEngine->m_velocity)*Length3(velDiff);
            //AddImpulse(imp);
            //other->physicsEngine->AddImpulse(-imp);
        }
    }
    */
    Vector4 dummy;
    Vector4 dirToUse = other->getPos()-m_player->getPos();
    m_player->Move(.3,-dirToUse,dummy);
    other->Move(.3,dirToUse,dummy);
    
}

void RPhysics::DoBoost() {
    if(m_car.numBoosts>0&&m_boostLeft<=0) {
        m_boostLeft = BOOST_TIME;
        m_car.numBoosts--;
    }
}


void RPhysics::ProcessCollision(float distLeft, Vector4 collPlane,Vector4 currDir) {
    currDir = Normalize3(currDir);
    collPlane = Normalize3(collPlane);
    Vector4 newDir = currDir-2*collPlane*(DotProduct3(collPlane,currDir))*Length3(currDir);
    Vector4 retDir;
    m_player->Move(distLeft,newDir,retDir);
    m_velocity = .3*Length3(m_velocity)*Normalize3(retDir);
	//m_velocity = 1.1*Length3(m_velocity)*Normalize3(retDir); //for bumper-car style racing
    collision = true;
    m_boostLeft = 0;
}

#define MIN_VEL .0001

void RPhysics::DoPhysics(int deltaTime) {
    
 
    Vector4 ahead = m_player->GetAhead();
    if(Length3(m_velocity)==0) {
        m_velocity = MIN_VEL*ahead;
    }
    Vector4 inputForce = CalculateInputForce();
    
    Vector4 resistance = CalculateResistanceForce();
    //cerr<<"input force";
    //PrintVector(inputForce);
    //cerr<<"resistance force:";
    //PrintVector(resistance);
    Vector4 turnDir = GetTurnDir();
    if(m_boostLeft>0) {
        inputForce+=BOOST_AMT*Normalize3(turnDir);
        m_boostLeft-=deltaTime;
    }
    Vector4 totalForce = inputForce+resistance;
    for(int i=0;i<m_impulseForces.size();i++) {
        totalForce+=m_impulseForces[i];
    }
    m_impulseForces.clear();
    float currSpeed = Length3(m_velocity);
    
    Vector4 newVelocity = m_velocity+deltaTime/1000.0*totalForce;
    //cerr<<"new velocity:";
    //PrintVector(newVelocity);
    float newSpeed = Length3(newVelocity);
    Vector4 deltaPos = newVelocity*deltaTime/1000.0;
    float distance = Length3(deltaPos);
    if(distance>0) {
        Vector4 oldUp = m_player->GetUp();
        Vector4 oldAhead = m_player->GetAhead();
        Vector4 oldDir = newVelocity;
        if(currSpeed==0) oldDir = oldAhead;
        collision = false;
        Vector4 newDir;
        if(newSpeed>MIN_VEL)
            m_player->Move(distance,deltaPos,newDir);
        else
            newDir = deltaPos;
        if(!collision) {
            m_velocity = Normalize3(newDir)*newSpeed;
        }
            Matrix4 transform = GetOrientTransform(oldDir,oldUp,newDir,m_player->GetUp());
            oldAhead(3) = 1;
            //cerr<<"orient transform:";
            //PrintMatrix(transform);
            Vector4 newAhead = transform*oldAhead;
            Vector4 desiredAhead = transform*turnDir;
          
            //cerr<<"new ahead:";
            //PrintVector(newAhead);
            //cerr<<deltaTime<<endl;
            Vector4 toSetAhead = Normalize3(8*33.3/deltaTime*newAhead+desiredAhead);
            //            if(currSpeed==0&&(m_brakeState==PRESSED||m_pedalState==RELEASED)) toSetAhead*=-1;
            m_player->SetAhead(toSetAhead);
            //}
    }

    
}

Matrix4 RPhysics::GetOrientTransform(Vector4 dirStart, Vector4 upStart1, Vector4 dirEnd, Vector4 upEnd1) {
    Vector4 temp = CrossProduct(dirStart,upStart1);
    Vector4 upStart = CrossProduct(temp,dirStart);
    temp = CrossProduct(dirEnd,upEnd1);
    Vector4 upEnd = CrossProduct(temp,dirEnd);
    dirStart = Normalize3(dirStart);
    dirEnd = Normalize3(dirEnd);
    upStart = Normalize3(upStart);
    upEnd = Normalize3(upEnd);
    Matrix4 transform;
    transform.Identity();
    
    Vector4 axis1 = CrossProduct(dirStart,dirEnd);
    if(Length3(axis1)==0) axis1 = upEnd;
    float angle1 = Angle3(dirStart,dirEnd);
    Matrix4 rotMat;
    rotMat.Rotation(angle1,axis1);
    Vector4 newUp = rotMat*upStart;
    Vector4 axis2 = CrossProduct(newUp,upEnd);
    if(Length3(axis2)==0) axis2 = dirEnd;
    float angle2 = Angle3(upEnd,newUp);
    if(angle1*angle2*0!=0) return transform;
    Matrix4 toRot;
    toRot.Rotation(angle2,axis2);
    transform= transform*toRot;
    toRot.Rotation(angle1,axis1);
    transform = transform*toRot;
    if(!(transform[0][0]<=3||transform[0][0]>=3)) {
        cerr<<endl;
        cerr<<angle1<<endl;
        cerr<<angle2<<endl;

        PrintVector(dirStart);
        PrintVector(upStart);
        PrintVector(dirEnd);
        PrintVector(upEnd);
        cout<<flush;
        exit(1);
    }
    return transform;

    
}



Vector4 RPhysics::CalculateResistanceForce() {
    Vector4 drag = -DRAG_CONSTANT*m_velocity;

    Vector4 ahead = m_player->GetAhead();
    
    float currSpeed = Length3(m_velocity);
    float frictionPercent=0;
    if(currSpeed>0) {
        frictionPercent = Angle3(m_velocity,ahead);
        if(frictionPercent>PI/2) frictionPercent = PI-frictionPercent;
        frictionPercent/=(PI/2);
        //frictionPercent = frictionPercent*frictionPercent;
        //cerr<<"f:"<<frictionPercent<<endl;
    }
    Vector4 friction = -FRICTION_CONST*frictionPercent*m_velocity/PI;
    Vector4 ret = friction+drag;
    if(ret[0]*0!=0) return Vector4(0,0,0);
    return ret;
}


Vector4 RPhysics::CalculateInputForce() {

    const float accel = 1.25*GetAccelConst();
    Vector4 ret(0,0,0);
    if(m_pedalState==PRESSED||m_boostLeft>0) {
        Vector4 inputDir = GetTurnDir();
        ret = inputDir*accel;
        float angle = fabs(Angle3(m_player->GetAhead(),m_velocity));
        if(angle*0==0)
            ret*=(1+angle);
        //ret*=2;
        //cerr<<"A:"<<angle<<endl;
    } else if(m_brakeState==PRESSED) {
        Vector4 ahead = m_player->GetAhead();
        ret = -ahead*BRAKE_CONSTANT;
    }
    return ret;
}

bool RPhysics::VelIsPositive() {
    if(Length3(m_velocity)>MIN_VEL) return true;
    else return false;
}

float RPhysics::GetAccelConst() {
    int speedIndex = (int)(Length3(m_velocity)/5);
    //cerr<<"speed index:"<<speedIndex<<endl;
    //cerr<<"speed:"<<Length3(m_velocity)<<endl;
    switch(speedIndex) {
    case 0:
        return 9;
    case 1:
        return 12;
    case 2:
        return 16;
    case 3:
        return 22;
    case 4:
        return 25;
    case 5:
        return 28;
    case 6:
        return 30;
    case 7:
        return 33;
    default:
        return 35;
    }
}

Vector4 RPhysics::GetTurnDir() {
    Matrix4 rotMat;
    rotMat.Rotation(PI/180*m_inputForceDegrees,m_player->GetUp());
    return Normalize3(rotMat*m_player->GetAhead());
}


// float RPhysics::GetTraction() {
//     return 1;
// }

float RPhysics::GetAcceleration() {
    return 10;
}
