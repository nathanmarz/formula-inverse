/*
 * RCarPhysics.cc
 *
 * Implements the algorithms for simulation of car dynamics
 * 
 * Author: Taral Joglekar
 * 
 */
#include "RCarPhysics.h"
#include "RLibraries.h"
#include "Framework.h"

#define SCALE_DIST (.5)

RTireInfo::RTireInfo()
{
	fs = .2;
	fr = .05;
	fd = .1;
}

RCar::RCar(){
	//dimensions = dims;
	mass = 500;
	cgHeight = 1;
	cgRatio = .5;
	frontDifferential = .5;
	backDifferential = 1;
	engineMaxAccel = 5;
	engineHeatEffects = 0;
	engineMaxRevs = MAX_ENGINE_REVS;
	engineCurRevs = 0;
	brakingMaxAccel = 5;
	aerodynamicResistance = .01;
	aerodynamicPress = 5;
	maxGears = 4;
	color = 1;
	//ahead = ahd;
//	position = posn;
	gearRatios[0] = -.5;
	gearRatios[1] = 0;
	gearRatios[2] = .6;
	gearRatios[3] = .7;
	gearRatios[4] = .85;
	gearRatios[5] = 1;
	pedalState = 0;
	brakeState = 0;
	curGear = 0;
	curAccel = 0;
	curVel = 0;
	speedLateral = 1;
	speedLongitudinal = 0;
	lastDistLong = 0;
	lastDistLat = 0;
}


RCar::RCar(Vector4 dims, float ms, float cgH, float cgR, float frontDif, float backDif, float engineAccel, float engineEff, float brkMax, float aeroDR, float aeroDP, int maxGrs, long clr, Vector4 posn, Vector4 up, Vector4 ahd){
	dimensions = dims;
	mass = ms;
	cgHeight = cgH;
	cgRatio = cgR;
	frontDifferential = frontDif;
	backDifferential = backDif;
	engineMaxAccel = engineAccel;
	engineHeatEffects = engineEff;
	engineMaxRevs = MAX_ENGINE_REVS;
	engineCurRevs = 0;
	brakingMaxAccel = brkMax;
	aerodynamicResistance = aeroDR;
	aerodynamicPress = aeroDP;
	maxGears = maxGrs;
	color = clr;
	ahead = ahd;
	position = posn;
	pedalState = 0;
	brakeState = 0;
	curGear = 0;
	speedLateral = 1;
	speedLongitudinal = 0;
	lastDistLong = 0;
	lastDistLat = 0;
}


int Getincr(int engineCurRevs, int engineMaxRevs)
{
#define MAXINCR		(engineMaxRevs / 10)
#define PEAKPT		(engineMaxRevs / 5)
	if(engineCurRevs <= PEAKPT) {
		return (int)(1 + MAXINCR + MAXINCR * SINE_FUNCTION(PI / 2 * (float)engineCurRevs / PEAKPT - PI / 4));
	} else {
		engineCurRevs -= MAXINCR;
		return (int)(MAXINCR + MAXINCR * SINE_FUNCTION(PI / 2 + PI / 2 * ( 1 - exp(-(float)engineMaxRevs))));
	}
}

void RCar::UpdateState (int millisecs) 
{
	float secs = 2 * millisecs;
	secs /= 1000.0;
	float effRevs;
	float FEng;		// forward force due to engine
	float FAeroR;	// force due to air resistance
	float Fweight;	// Weight of the car
	float FAeroP;	// weight of the aerodynamic press
	float FBrake;	// Braking force
	float FLong;	// longitudinal part
	float FLat;		// lateral part
	float Ftire;	// drag due to tires

	float maxEngineForce = mass * engineMaxAccel;	// decide what this value should be...
	float Caero = 10.4257;
	float Crb = 16.8;
	// let us resolve everthing into real forces and then solve the force system later.
	
	if(brakeState != 0) {
		FBrake = -brakingMaxAccel * mass;
	} else {
		FBrake = 0;
	}

	if(pedalState != 0) {
		// Pedal is pressed. //
		if (curGear == 0) curGear = 1;
		if(engineCurRevs < engineMaxRevs) {
			//cout << Getincr(engineCurRevs, engineMaxRevs) << endl;
			engineCurRevs += Getincr(engineCurRevs, engineMaxRevs);//(int) ((engineMaxRevs / 3.0) * secs);
		}
		if(engineCurRevs > engineMaxRevs) {
		    if(curGear < maxGears) {
				curGear++;
				engineCurRevs = (int)(engineCurRevs * gearRatios[curGear] / gearRatios[curGear + 1]);
			} else {
				engineCurRevs = engineMaxRevs;
			}
		}
		effRevs = engineCurRevs * gearRatios[curGear + 1];
		FEng += (float)effRevs / (engineMaxRevs) * maxEngineForce;
	} else {
		engineCurRevs -= (int) ((engineMaxRevs / 4.0) * secs);
		if(engineCurRevs < (engineMaxRevs / 4) ) {
			if(curGear > 1) {
				curGear--;
				engineCurRevs = (int)(engineCurRevs * gearRatios[curGear + 2] / gearRatios[curGear + 1]);
			} else {
				engineCurRevs = 0;
			}
		}
		effRevs = engineCurRevs * gearRatios[curGear + 1];
		FEng = 0; 	// engine not driving the vehicle forward
	}

	FAeroR = -Caero * speedLongitudinal * fabs(speedLongitudinal);
	Ftire = -Crb * speedLongitudinal;
	FLong = FEng + FAeroR + Ftire;
	//if(-FBrake > FLong) {
	//   FBrake = -FLong;
	//}
	//FLong += FBrake;
	curAccel = FLong / mass;

	//cout << FEng <<" "<<FAeroR <<" "<< Ftire <<" ";
	lastDistLong += SCALE_DIST * (speedLongitudinal * secs + .5 * curAccel * secs * secs);
	speedLongitudinal = speedLongitudinal + curAccel * secs;

	//cout << curGear << " " << FLong << " " << curAccel << " " << secs <<" " << speedLongitudinal << " " << engineCurRevs << " " << lastDistLat << " " <<endl;

/*
	if(pedalState != 0) {
		if (curGear == 0) curGear = 1;
		engineCurRevs += (int) ((MAX_ENGINE_REVS / 3.0) * secs);
		curAccel = engineMaxAccel * gearRatios[curGear + 1];
		if(engineCurRevs > engineMaxRevs) {
		    if(curGear < maxGears) {
				curGear++;
				engineCurRevs = engineMaxRevs / 100;
			} else {
				engineCurRevs = engineMaxRevs;
			}
		}
	} else {
		engineCurRevs -= (int) ((MAX_ENGINE_REVS * 2 / 5) * secs);
		curAccel = 0;
		if(engineCurRevs < 0 ) {
			if(curGear > 1) {
				curGear--;
				engineCurRevs = engineMaxRevs;
			} else {
				engineCurRevs = 0;
			}
		}
	}
	cout << curGear << " " << speedLongitudinal << " " << speedLateral << " " << engineCurRevs << " " << lastDistLat << " " <<endl;
	if(brakeState != 0) {
		curAccel = -brakingMaxAccel;
		engineCurRevs -= (int)((MAX_ENGINE_REVS / 3.0) * secs);
		if(engineCurRevs < 0) {
		    if(curGear > 1) {
				curGear--;
				engineCurRevs = engineMaxRevs;
			} else {
				engineCurRevs = 0;
			}
		}
	}
*/
	if(speedLateral > 0) {
		float latAccel = -1.0 + (brakeState == 1 ? -.4 : 0);
		lastDistLat += (SCALE_DIST) * speedLateral * secs;
		speedLateral += (latAccel * mass * secs * secs);
		if(speedLateral < .1) speedLateral = 0; // static friction kicks in
	}
	if(speedLateral < 0) {
		float latAccel = 1.0 + (brakeState == 1 ? .4 : 0);
		lastDistLat += (SCALE_DIST) * speedLateral * secs;
		speedLateral += (latAccel * mass * secs * secs);
		if(speedLateral > -.1) speedLateral = 0; // static friction kicks in
	}
	
	//lastDistLong += (speedLongitudinal * secs + .5 * curAccel * secs * secs);
	//speedLongitudinal = speedLongitudinal + curAccel * secs - aerodynamicResistance * speedLongitudinal * speedLongitudinal;
	
	if(speedLongitudinal < .2 && speedLongitudinal > -.2) speedLongitudinal = 0;

}


inline Vector4 Collide(float e, Vector4 inc_vector, Vector4 collisionPlane)
{
	float j;
	//collisionPlane = Normalize3(collisionPlane);
	j = DotProduct3(-(1 + e) * inc_vector, collisionPlane) / DotProduct3(collisionPlane, collisionPlane);
	inc_vector = inc_vector + j * Normalize3(collisionPlane);  // Force Field :)
	return inc_vector;
}
	
void RCar::ProcessCollision(float distanceLeft,RPlayer *player, Vector4 collisionPlane, Vector4 direction) 
{
	Vector4 ahd = Normalize3(player->GetAhead());
	Vector4 up = Normalize3(player->GetUp());
	Vector4 colP = Normalize3(collisionPlane);
	Vector4 car_right = CrossProduct(ahd, up);
	Vector4 car_vector = speedLongitudinal * ahd + speedLateral * up;
	float e = 1;
	cout << "collision!" << endl;
	//cout << "collision plane:" << collisionPlane[0] << " " << collisionPlane[1] << " " << collisionPlane[2] << " " << collisionPlane[3] << " " << endl;
	colP = -1 * colP;
	car_vector = Collide(e, car_vector, colP);
	speedLongitudinal = DotProduct3(car_vector, ahd);
	curGear = 1;
	engineCurRevs = 0;
	//speedLongitudinal = 0;
	speedLateral = DotProduct3(car_vector, car_right);
}
