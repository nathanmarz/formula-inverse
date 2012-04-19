#include "Framework.h"
#include <math.h>
#include "RParticle.h"

#define MIN_FORCEFIELD_SPEED 1.0
#define MAX_RESISTANCE 40

#define SMOKE_SPEED_INDEX 4
#define SMOKE_INTERVAL 3

RPlayer::RPlayer(REngine *eng, int i, int j, RTrack *track, RGameLogic *gameLogic, float interpolation,const char *modelPath, MD2Texture tex) {
    m_engine = eng;
    m_lastInput = m_lastResistance = Vector4(0,0,0);
    m_gameLogic = gameLogic;
    m_track = track;
    TrackGrid tg = track->GetGridOfPoint(i,j);
    m_currGrid = tg;
    m_up = tg.GetNormal();
    m_ahead = tg.GetStraight();
    m_pos = (tg.v0.pos+tg.v1.pos+tg.v2.pos)/3;
    //PrintVector(m_pos);
    RCarInfo myInfo;
    myInfo.numBoosts = m_gameLogic->GetTotalBoosts();
    //cerr<<"num boosts:"<<myInfo.numBoosts<<endl;
	this->physicsEngine = new RPhysics(this,myInfo);
    m_gameLogicId = m_gameLogic->CreatePlayerInfo(i);
    //this transform lets the program use normal orientation definitions (like models off the web)
    Matrix4 transform;
    transform.Scale(.02,.02,.02);
    Matrix4 rotmat;
    m_timeToNextSmoke = SMOKE_INTERVAL;
    rotmat.Rotation(-5*PI/180,Vector4(1,0,0));
    transform = transform*rotmat;
    rotmat.Rotation(-PI/2,Vector4(1,0,0));
    transform = transform*rotmat;
    m_model = new RMD2Model(true,m_pos,m_up,m_ahead,transform,interpolation,modelPath,tex);
    //m_model->finalize();
}

void RPlayer::Render(RCamera *camera) {
    getModel()->Render(camera);
}


void RPlayer::Rotate(float degrees) {
    //Matrix4 rotMat;
    //rotMat.Rotation(PI/180*degrees,m_up);
    //m_ahead=Normalize3(rotMat*m_ahead);
    physicsEngine->SetInputForce(degrees);
}


void RPlayer::TransformOrientation(TrackGrid &curr) {
    Vector4 newNorm = curr.GetNormal();
    float radToRotate = Angle3(m_up,newNorm);
    Vector4 axis = CrossProduct(m_up,newNorm);
    Matrix4 rotMat;
    rotMat.Rotation(radToRotate,axis);
    m_ahead = Normalize3(rotMat*m_ahead);
    m_up = newNorm;
}

void RPlayer::DrawBounds() {
    Vector4 bound1, bound2;
    m_track->GetBounds(m_currGrid.i,bound1,bound2);
    DrawPlane(bound1);
    DrawPlane(bound2);
}

RPlayer::~RPlayer() {
    delete m_model;
    delete physicsEngine;
}


/*
   RBoundVol *myBound = getModel()->GetBoundingVolume();
    for(list<RPlayer *>::iterator it = allPlayers.begin();it!=allPlayers.end();++it) {
        RPlayer *other = *it;
        if(other!=this) {
            RBoundVol *otherBound = other->getModel()->GetBoundingVolume();
            
        }
    }

*/


void RPlayer::Move(float distance, Vector4 dirNonNorm, Vector4 &newDir) {
    //cerr<<distance<<endl;
    if(distance==0) {
        newDir = Normalize3(dirNonNorm);
        return;
    }
    Vector4 startPos = m_pos;
    Vector4 dir = Normalize3(dirNonNorm);
    while(true) {
        float distTraveled = min(distance,m_currGrid.minTravelStep);
		if(distTraveled==0) distTraveled = distance;
        //cerr<<"min travel step:"<<m_currGrid.minTravelStep<<endl;
        distance-=distTraveled;
        Vector4 projPos, projDir;
        ProjectOntoPlane(m_currGrid.GetGridPlane(),m_pos,dir,projPos,projDir);
        projDir = Normalize3(projDir);
        //        m_track->ProjectVectorOntoTrack(trackI,m_pos,dir,projPos,projDir, currTri);
        Vector4 newPoint = projPos + projDir * distTraveled;
        TrackGrid newTri;
        m_track->FindCloseTrackPosition(newPoint,m_currGrid,newTri);
        
        Vector4 plane = newTri.GetGridPlane();
        Vector4 oldDir = projDir;
        ProjectOntoPlane(plane,newPoint,oldDir,projPos,projDir);
        projDir = Normalize3(projDir);
        //PrintVector(projDir);
        
       
        Vector4 bound;
        
        if(m_currGrid.GetBound(bound)) {
            //cerr<<"checking collision"<<endl;
            bool collision = false;
            Vector4 collPlane;
            if(DistToPlane(bound,newPoint)>= 0) {
                //cerr<<"Distance: "<<DistToPlane(bound1,projPos);
                //exit(1);
                collision = true;
                collPlane = bound;
            } 
            if(collision) {
                //cerr<<"collisionL "<<newTri.i<<","<<newTri.j/2<<endl;
                //m_pos = ProjectPointOntoPlane(collPlane,projPos);
                //m_pos = projPos;
                //dir = projDir;
                newDir = dir;
		
               
                
                float distLeft = distance;
                
                    
                //Vector4 psPos = m_pos+m_up*.35;
                //GLuint texture = RResources::GetTexture(-1);
                //RObject * ps = new RSparkParticleSystem(psPos, 30, texture, 1000, .1, dir/100);
                if(physicsEngine->getSpeed()>MIN_FORCEFIELD_SPEED) {
                    Vector4 outPos,outVec;
                    m_currGrid.GetBoundParams(outPos,outVec);
                    Vector4 collPos = ProjectPointOntoPlane(collPlane,m_pos);
                    GLuint texture = RResources::GetTexture(-2);
                    int numForceField = 8;
                    for(int i=1;i<=numForceField;i++) {
                        RObject *ps = new RForceField(collPos, m_up, 
                                                      -outVec,
                                                          .5, 1, texture, i*600/numForceField,i*.02/numForceField);
                        m_engine->GetWorld()->AddDynamicEffect(ps);  
                    }
                    }
                physicsEngine->ProcessCollision(distLeft,collPlane,projDir);
                
                return;
            }
        }
        float dist;
        ClosestPointOnTri(newTri.v0.pos,newTri.v1.pos,newTri.v2.pos,projPos,projPos,dist); 
        plane = newTri.GetGridPlane();
        //oldDir = projDir;
        //        ProjectOntoPlane(plane,newPoint,oldDir,projPos,projDir);
        //projDir = Normalize3(projDir);
        //        trackI = newTri.i;
        m_currGrid = newTri;
        //cerr<<"amount moved: "<<Length3(m_pos-projPos)<<endl;
        m_pos = projPos;
        if(distance > 0) { //use surface interpolation when numSteps==0 (once it's working)
            
            //        m_up = Normalize3(currTri.v0.normal+currTri.v1.normal+currTri.v2.normal);
            
            //m_up = Normalize3((td-distTo0)*currTri.v0.normal+(td-distTo1)*currTri.v1.normal+(td-distTo2)*currTri.v2.normal);
            m_up = m_currGrid.GetNormal();
        } else {
            SurfaceInterpolation(m_currGrid,projPos,m_pos,m_up); //use this when it's working
            if(!(m_up[0]<=3||m_up[0]>=3)) {
                cerr<<"m_up problem"<<endl;
                exit(1);
            }
            
        }
        
        
        //cerr<<"dirNormal"<<endl;
        //PrintVector(projDir);
        //PrintVector(m_up);
        //PrintVector(dirNormal);
        //if(!(dirNormal[0]>=0||dirNormal[0]<=3)) exit(1);
        dir = projDir;
        //cerr<<distance<<endl;
        if(distance<=0) break;
    }
    //    Vector4 dirNormal = CrossProduct(dir,m_up);
 //   newDir = Normalize3(CrossProduct(m_up,dirNormal));
    
    newDir = dir;
    if(Length3(m_pos-startPos)<=.5*distance) {
        m_currGrid = m_track->GetGridOfPoint(m_track->NextI(m_currGrid.i),m_currGrid.j);
        m_pos = m_currGrid.GetCenter();
        //cerr<<"correction!"<<endl;
    }
}

bool RPlayer::FinishedRace() {
    return m_gameLogic->PlayerFinished(m_gameLogicId);
}

void RPlayer::FinishTurn(list<RPlayer *> allPlayers, int deltaTime) {
    //cerr<<m_currGrid.i<<endl;
    //cerr<<"dot product:"<<endl;
    //cerr<<DotProduct3(m_up,m_ahead)<<endl;
    //cerr<<"pos:";
    //PrintVector(m_pos);
    //cerr<<"ahead::"<<endl;
    //PrintVector(m_ahead);
    //cerr<<"up::"<<endl;
    //PrintVector(m_up);

    for(list<RPlayer *>::iterator it = allPlayers.begin();it!=allPlayers.end();++it) {
        RPlayer *otherPlayer = *it;
        if(otherPlayer!=this) {
            if(m_model->GetBoundingVolume()->CollidesWith(otherPlayer->m_model->GetBoundingVolume())) {
                //cerr<<"collision"<<otherPlayer<<endl;
                
                physicsEngine->ProcessPlayerCollision(otherPlayer);
                RObject *ps = new RSparkParticleSystem((m_pos+otherPlayer->m_pos)/2+Normalize3(m_up)*.35,25,RResources::GetTexture(-1),300,Normalize3(m_up)/20);
                m_engine->GetWorld()->AddDynamicEffect(ps);
            }
        }
    }

    OrientToForces(deltaTime);
    //GenerateSmokeEffect(deltaTime);
    m_gameLogic->SetPlayerPosition(m_gameLogicId,m_currGrid.i,deltaTime);
    

}


float RandRange(float radius) {
    return radius-1.0/100.0*(rand()%101)*2*radius;
}

void RPlayer::GenerateSmokeEffect(int deltaTime) {
    m_timeToNextSmoke-=deltaTime;
    if(m_timeToNextSmoke<=0) {
        m_timeToNextSmoke = SMOKE_INTERVAL;
        float speed = physicsEngine->getSpeed();
        int numSmoke = (int)(1+speed/SMOKE_SPEED_INDEX);
        numSmoke*=2;
        Vector4 ahead = GetAhead();
        Vector4 startPos = getPos()+m_up*.2;
        Vector4 baseDir = -Normalize3(ahead);
        float s_speed = .5;
        cerr<<"generating smoke:"<<numSmoke<<endl;
        for(int i=0;i<numSmoke;i++) {
            Vector4 pos = startPos + Vector4(RandRange(.3),RandRange(.3),RandRange(.3));
            Vector4 dir = s_speed*baseDir;
            RObject *ss = new RSmoke(pos,dir,.1,.3,.3,RResources::GetTexture(-1),1000,.00015);
            m_engine->GetWorld()->AddDynamicEffect(ss);
        }
    }
}


Vector4 ScaleVectorToVector(Vector4 start, Vector4 end, float maxChange) {
    //PrintVector(start);
    //PrintVector(end);
    float totalDist = Length3(end-start);
    if(totalDist<=maxChange) return end;
    else {
        Vector4 direction = Normalize3(end-start);
        return start+direction*maxChange;
    }
}


void RPlayer::OrientToForces(int deltaTime) {
    Vector4 desiredUp = Normalize3(GetUp());
    Vector4 playAhead = GetAhead();
    Vector4 dirNormal = CrossProduct(playAhead,desiredUp);
    playAhead = Normalize3(CrossProduct(desiredUp,dirNormal));
    Vector4 right = Normalize3(CrossProduct(playAhead,desiredUp));
    Vector4 desiredPos = m_pos+desiredUp*.4;

    Vector4 desiredInputForce(0,0,0);
    //Vector4 inputForce = physicsEngine->CalculateInputForce();
    if(Length3(physicsEngine->CalculateInputForce())>0) {
        desiredInputForce = -desiredUp*.4;
        if(DotProduct3(physicsEngine->CalculateInputForce(),playAhead)<0) {
            desiredInputForce*=-1;
        }
    }

    float scaler = deltaTime/(50.0/3.0);
    Vector4 resistanceForce = physicsEngine->CalculateResistanceForce();
    if(Length3(resistanceForce)>MAX_RESISTANCE) resistanceForce = MAX_RESISTANCE*Normalize3(resistanceForce);
    //cerr<<Length3(resistanceForce)<<endl;
    resistanceForce = ScaleVectorToVector(m_lastResistance,resistanceForce,scaler*.9);
    Vector4 latForce = ProjectVectorOntoVector(resistanceForce,right);
    Vector4 inputForce = ScaleVectorToVector(m_lastInput,desiredInputForce,scaler*.035);

    //if(Length3(resistanceForce)>.5)
    desiredUp = desiredUp+latForce/30.0;
    playAhead = playAhead+inputForce;
    
    m_lastResistance = resistanceForce;
    m_lastInput = inputForce;
    m_model->SetCoordinateFrame(desiredPos,desiredUp,playAhead);
}
