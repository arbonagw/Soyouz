/**
* This work is distributed under the General Public License,
* see LICENSE for details
*
* @author Gwenna�l ARBONA
**/

#ifndef __SHIP_H_
#define __SHIP_H_

#include "Engine/meshactor.hpp"
#include "Game/weapon.hpp"

class Game;
class Thruster;


/*----------------------------------------------
	Class definitions
----------------------------------------------*/

class Ship : public MeshActor
{

public:
	
	/**
	 * @brief Create a ship
	 * @param g				Game actor
	 * @param name			Unique name to set to the mesh
	 * @param templateFile	Ship template file
	 **/
	Ship(Game* g, String name, Ogre::String templateFile);

	/**
	 * @brief Destructor
	 **/
	virtual ~Ship();
	
	/**
	 * @brief Pre-tick event
	 * @param evt			Frame event
	 **/
	void preTick(const Ogre::FrameEvent& evt);
	
	/**
	 * @brief Main tick event
	 * @param evt			Frame event
	 **/
	void tick(const Ogre::FrameEvent& evt);

	/**
	 * @brief Set the ship speed target
	 * @param speed			Speed setting
	 **/
	void setSpeed(float speed);
	
	/**
	 * @brief Set the ship steer target
	 * @param x				X command (-1 - 1)
	 * @param y				Y command (-1 - 1)
	 **/
	void setSteer(float x, float y);
	
	/**
	 * @brief Set the ship roll target
	 * @param roll			Roll command
	 **/
	void setRoll(float roll);
	
	/**
	 * @brief Get the current direction target
	 * @return a vector materializing the travel direction
	 **/
	Vector3 getDirectionCommand(void);
	
	/**
	 * @brief Get the current angular rotation speed
	 * @return a vector materializing the angular speed
	 **/
	Vector3 getRotationCommand(void);

	/**
	 * @brief Get the current max linear speed
	 * @return speed
	 **/
	float getMaxSpeed(void);
	
	/**
	 * @brief Get the current max angular speed
	 * @return speed
	 **/
	float getMaxAngularSpeed(void);
	
	/**
	 * @brief Get the standard viewing distance
	 * @return distance
	 **/
	float getViewDistance();

	
	Weapon* getPrimaryWeapon();
	Weapon* getSecondaryWeapon();

	void setFireOrder(bool fire);

	void setAimDirection(Vector3 aimDirection);


protected:
	
	
	/**
	 * @brief Add engines to the ship
	 **/
	virtual void setupEngines();
	
	/**
	 * @brief Add weapons to the ship
	 **/
	virtual void setupWeapons();
	
	/**
	 * @brief Add add-ons to the ship
	 **/
	virtual void setupAddons();
	
	/**
	 * @brief Override this to decide wether to save the class or not
	 * @return true if this is to be saved
	 **/
	virtual bool isSavable();
	
	/**
	 * @brief Compute a steering component for linear control
	 * @param measure		Input measure
	 * @param command		Input command
	 * @return the soft steering
	 **/
	float computeSoftLinearCommand(float measure, float command);
	
	/**
	 * @brief Compute a steering component for angular control
	 * @param measure		Input measure
	 * @param command		Input command
	 * @return the soft steering
	 **/
	float computeSoftAngularCommand(float measure, float command);
	
	
protected:


	// Steering controls for the player
	float mSteerX;
	float mSteerY;
	float mSteerRoll;
	float mSpeed;

	// Steering commands for engines
	Vector3 mCommandVector;
	Vector3 mCommandRotator;

	// Steering characteristics
	float mMaxSpeed;
	float mMaxAngularSpeed;
	float mSoftModeLimit;
	float mSoftModeAngularLimit;

	// Customisation characteristics
	float mViewDistance;
	int mShipSize;
	Ogre::String mShipClass;
	Ogre::String mShipType;
	Ogre::String mShipStory;

	// Thrusters
	Ogre::list<Thruster*>::type mThrusters;
	
	// Weapons
	Ogre::vector<Weapon*>::type mWeapons;

};

#endif /* __SHIP_H_ */

