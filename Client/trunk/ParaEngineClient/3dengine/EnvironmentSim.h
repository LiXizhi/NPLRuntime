#pragma once
#include "IEnvironmentSim.h"
#include <list>
using namespace std;

namespace ParaEngine
{
/**
Animate the entire scene for a fixed time interval advance. Path-finding, collision detection and response
are implemented in this module. Game physics, AI controller, some script triggering are also done by 
the simulator. The simulator will not allow a time advance that is too big, but smaller time advance will
make the game run too slow. In most cases, it is called 30 times per seconds.
The most frequently used functions are:
(1)	m_pRootScene	    : Set the scene to compute
(2)	Animate(dTimeDelta) : Compute next scene

Character simulation now supports unlimited number of global and local characters. The simulator will automatically
put unrelated game object to sleep and wake up related ones.In case of a single server, 2000 players with 100,000++ 
creatures may be in the scene. However, there may only be 3000+ sentient game objects, all others are put to sleep. 
The simulation complexity is O(N.log(M)), where N is number of sentient game objects, M is the total number of characters attached to the scene. 

Some attributes of IGameObject may affect the simulation behavior of the simulator. The most important ones are:

m_nGroup: the group ID to which this object belongs to. In order to be detected by other game object. 
	Object needs to be in group 0 to 31. default value is 0
m_dwSentientField: a bit field of sentient object. from lower bit to higher bits, it matches to the 0-31 groups. @see m_dwGroup
	if this is 0x0000, it will detect no objects. If this is 0xffff, it will detects all objects in any of the 32 groups. 
	if this is 0x0001, it will only detect group 0. 
m_fPeceptiveRadius:the radius within which this biped could see or perceive other bipeds. 

An object can only be waken up by or perceive other objects whose group ID is marked in the object's m_dwSentientField.
For example. if the sentient field of object is 101 (binary form), then it can only detect objects whose group ID is 0 and 2. 

*/
class CEnvironmentSim : public IEnvironmentSim
{

public:
	CEnvironmentSim(void);
	~CEnvironmentSim(void);

	/**
	* return the current type of simulator. 0 means standalone simulator, 1 means server, 2 means client. 
	*/
	virtual SIMULATOR_TYPE GetSimulatorType(){return SIM_TYPE_STANDALONE;};

	/** called each frame to simulate the game world. 
	* @param dTimeDelta: in seconds.
	*/
	virtual void Animate( double dTimeDelta );

	/**
	* when this function is called, it ensures that the physics object around one or more points are properly loaded.
	* It increases the hit count of these physics objects by 1. The garbage collector in the physics world
	* may use the hit count to move out unused static physics object from the physics scene (Novodex).
	* This function might be called for the current player, each active mobile object in the scene and the camera eye position.
	* @param points: if not NULL, it will further check if it hits any of the sphere points. 
	* @param nPointCount: the number of points defined in points. 
	*/
	virtual void CheckLoadPhysics(CShapeSphere* points, int nPointCount = 1);

	/** delete this object. Such as Delete this;*/
	virtual void Release();
private:
#ifdef  OLD_2005_5_ALGORITHM

	void BipedSimulation();
	void PlayerSimulation(double dTimeDelta);
	void GenerateActiveBipedList();
	void GenerateStaticCP(ActiveBiped* pActiveBiped);
	void CleanupImtermediateData();

	void SimulateOld(double dTimeDelta);

#endif

#ifdef  OLD_2004_5_ALGORITHM
	bool PlayerAnimateStrictly(ActiveBiped* pBiped, double dTimeDelta);
	bool PathFindingForBiped(ActiveBiped* pBiped, double dTimeDelta);
#endif
	
	/** simulate by a small time step.*/
	void Simulate(double dTimeDelta);

	/** update the game objects, find out the terrain tile the game object is currently located. and detect
	which objects are it each object's perceptive radius. and wake up or sleep corresponding actors. */
	void UpdateGameObjects(double dTimeDelta);
private:

	/// a list of terrain that contains all active biped.
	list <ActiveTerrain*>	m_listActiveTerrain;
	/// currently active biped: mobile or its brain is functioning, this may contain biped
	/// that is out of the view-culling radius. 
	list <ActiveBiped*>		m_listActiveBiped;

	/// very important biped list
	list <CBipedObject*> m_listVIPBipeds;
};
}