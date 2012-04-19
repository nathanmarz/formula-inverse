/*
 * RCarPhysics.h
 *
 * Interface definition for the Car Physics class
 * 
 * Author: Taral Joglekar
 * 
 */

#ifndef __R_CAR_PHYSICS_H_
#define __R_CAR_PHYSICS_H_

#include "Math3D.h"

class RPlayer;

using namespace std;
using namespace Math3D;

#define SPEEDMAX 14.70
#define MAX_GEARS	(7)
#define MAX_ENGINE_REVS (1000000)


class RTireInfo {
	private:
		//float	rubberType;		// decide some parameters, parameter curves?
		float	fs;				// static friction
		float	fr;				// rolling friction
		float	fd;				// dynamic friction
		float	turnAngle;		// angle steered

		//float	heat;			// dynamic heat measure
		//float	heatDurability;	// some parameter that decides how the tire behaves on heating
		//float	grip;			// might be included in rolling friction, but dependant on tread, not rubber
		//float	damageLevel;
		//float	radius;			// tire radius
	public:
		//void	HeatTire(void);
		//void	CoolTire(void);
		RTireInfo();
		friend class RCar;
};


class RCar {

	private:
		Vector4 dimensions;		// height, length, breadth of car bounding box
		float mass;				// car mass
		float cgHeight;			// centre of gravity position,
		float cgRatio;			// gives ratio of back to front weight distribution
								// car assumed to be symmetric left right. So CG is 
								// in the center in that direction

		RTireInfo tire[4];		// save information of each tire so that we can model
								// per tire damage later if we want

		float frontDifferential; // dynamic differential in use
		float backDifferential;
		float rots;					// current car rotation speed
		float engineMaxAccel;		// how much force can the engine produce?
		int	  engineMaxRevs;		// max engine pumping speed
		int   engineCurRevs;		// dynamic rev counter
		float curAccel;				// current acceleration of the car
		float curVel;				// current speed of the car
		float engineHeatEffects;	// not sure yet, does too much heating jam the engine?
		float brakingMaxAccel;		// reverse traction amount
		float aerodynamicResistance;	// air resistance parameterized
		float aerodynamicPress;		// vertical air pressure, adds to traction
		float speedLongitudinal;	// dynamic speed measure
		float speedLateral;			// for spinouts
		int pedalState;				// 0 = not pressed, 1 = pressed
		int brakeState;				// 0 = not pressed, 1 = pressed
		int curGear;				// 0 = neutral, -1 = reverse, 1 - N: gear
		int maxGears;				// maximum gears for this car
		float gearRatios[MAX_GEARS];	// gear ratios for each gear selected
		float lastDistLong;				// last updated distance travelled
		float lastDistLat;
		// now other peripheral characteristics

		Vector4 position;		// where is the car currently..? in terms of eye position? have to decide
		Vector4 up;				// orientation
		Vector4 ahead;			// direction in which car is currently pointing
		long color;				// body color

	public:
		RCar();
		RCar(Vector4 dims, float ms, float cgH, float cgR, float frontDif, float backDif, float engineAccel, float engineEff, float brkMax, float aeroDR, float aeroDP, int maxGrs, long clr, Vector4 posn, Vector4 up, Vector4 ahd);
		Vector4 GetPosition() { return position; }
		//void SetNewPosition(Vector4 posn) {position = posn; }
		void HitPedal() { pedalState = 1; }
		int GetPedal() { return pedalState; }
		void ReleasePedal() {pedalState = 0; }
		void HitBrake() { brakeState = 1; }
		void ReleaseBrake() { brakeState = 0; }
		void GearUp() { if (curGear < maxGears) curGear++; }
		void GearDown() {	if(curGear <= 0) curGear--; }
        float getSpeed() { return speedLongitudinal; }
	float getMaxSpeed() { return SPEEDMAX; }
		void ProcessCollision(float distanceLeft,RPlayer *player, Vector4 collisionPlane, Vector4 direction);
		void UpdateState(int millisecs);
		void SetGearRatio(int gear, float ratio) {if(gear > -2 && gear < maxGears) gearRatios[gear + 1] = ratio;}
		float GetLastDistLong() { return lastDistLong; }
		float GetLastDistLat() { return lastDistLat; }
		void ResetLastDist() { lastDistLong = 0; lastDistLat = 0; }
};

#endif

