#ifndef RPHYSICS_H
#define RPHYSICS_H

#include "RLibraries.h"

#define BOOST_TIME 2500
#define BOOST_AMT 50

class RPlayer;

struct RCarInfo {
    int numBoosts;
};


#define PRESSED true
#define RELEASED false

class RPhysics {
 public:
    RPhysics(RPlayer *player, RCarInfo &info) {
        m_player = player;
        m_car = info;
        m_velocity = Vector4(0,0,0);
        m_pedalState = false;
        m_brakeState = false;
        m_inputForceDegrees = 0;
        m_boostLeft = 0;
    }
    bool VelIsPositive();

    int BoostsLeft() { return m_car.numBoosts; }
    int BoostTimeLeft() { return m_boostLeft; }
    void DoBoost();
    void DoPhysics(int deltaTime);
    void SetInputForce(float degrees) {m_inputForceDegrees = degrees; }
    void ProcessCollision(float distLeft, Vector4 collPlane,Vector4 currDir);
    void HitPedal() { m_pedalState = true; }
    void ReleasePedal() {m_pedalState = false; }
    void HitBrake() { m_brakeState = true; m_boostLeft = 0; }
    void ReleaseBrake() {m_brakeState = false; }
    float getSpeed() { return Length3(m_velocity); }
    float getMaxSpeed() {return 50; } //work this out later
    void ProcessPlayerCollision(RPlayer *other);
    Vector4 CalculateResistanceForce();
    Vector4 CalculateInputForce();

 private:
    void AddImpulse(Vector4 force) { m_impulseForces.push_back(force); }
    
    float GetAccelConst();
    Matrix4 GetOrientTransform(Vector4 dirStart, Vector4 upStart1, Vector4 dirEnd, Vector4 upEnd1);
    Vector4 GetTurnDir();
    float GetAcceleration();
    RPlayer *m_player;
    RCarInfo m_car;
    Vector4 m_velocity;
    bool m_pedalState;
    bool m_brakeState;
    float m_inputForceDegrees;
    int m_boostLeft; //in milliseconds
    vector<Vector4> m_impulseForces;



};















#endif
