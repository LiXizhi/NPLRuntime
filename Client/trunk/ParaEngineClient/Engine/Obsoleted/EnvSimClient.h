#pragma once
#include "IEnvironmentSim.h"
#include "GameNetCommon.h"
#include <list>

namespace ParaEngine
{
	class IGameObject;
	class CBipedObject;
	using namespace std;
	/**
	* this is the environment simulator of a standard game world server. 
	*/
	class CEnvSimClient : public IEnvironmentSim, ICharacterTrackerContainer
	{
	public:
		CEnvSimClient(void);
		~CEnvSimClient(void);
	public:
		/**
		* return the current type of simulator. 0 means standalone simulator, 1 means server, 2 means client. 
		*/
		virtual SIMULATOR_TYPE GetSimulatorType(){return SIM_TYPE_CLIENT;};

		/** called each frame to simulate the game world. 
		* @param dTimeDelta: in seconds.
		*/
		virtual void Animate( double dTimeDelta );

		/**
		* when this function is called, it ensures that the physics object around an object is properly loaded.
		* It increases the hit count of these physics objects by 1. The garbage collector in the physics world
		* may use the hit count to move out unused static physics object from the physics scene.
		* This function might be called for the current player, each active mobile object in the scene and the camera eye position.
		* @param vCenter: the center of the object in world coordinates 
		* @param fRadius: the radius of the object within which all physics object must be active.
		*/
		virtual void CheckLoadPhysics(const D3DXVECTOR3& vCenter, float fRadius);

		/** delete this object. Such as Delete this;*/
		virtual void Release();

		/**
		* get the net time object
		* @return 
		*/
		virtual CNetTime* GetNetTime()
		{
			return &m_time;
		}

		/** send normal update information to a given client. 
		* a normal update usually contains the positions of all characters in the given character's perceptive radius.
		* when a client receives such as package, it will use the information to update its local networked characters, as well as itself. 
		* @param id: the id of the player, to which the package is sent.
		* @param s: [in|out] the bit stream to which the data are written. 
		* @return: true if succeed.
		*/
		bool SendNormalUpdate(const string& id, NPL::CBitStream* s);

		/** called when received normal update messages from a given client. 
		* @param id: the id of the player, from which the package is received.
		* @param s: [in|out] the bit stream from which the data are read. 
		*/
		void OnReceiveNormalUpdate(const string& id, NPL::CBitStream* s);
		
		/**
		* send a block for just a single character.
		* @param pChar 
		* @param s 
		* @param sIdentifier if this is not NULL, the player ID of pChar will be replaced by this one.
		* @return true if the successfully sent; false if not send such as the character is not valid. 
		*/
		bool SendCharacterUpdate(CBipedObject* pChar, CCharacterTracker* pCharTracker, NPL::CBitStream* s, const char* sIdentifier = NULL);
		/**
		* receive a block of a single character. 
		* @param s 
		* @param sIdentifier if this is not NULL, the player ID read from s will be replaced by this one.
		*	so it will be this specified character that get updated. 
		* @return 
		*/
		IGameObject* ReceiveCharacterUpdate(NPL::CBitStream* s, const char* sIdentifier = NULL);

		/** move a character. 
		* @param vDest: the absolute solution from the last network. 
		* @param bForceRun: force using running animation.
		* @param bForceWalk: force using walking animation.
		*/
		bool NetworkMoveCharacter(CBipedObject* pBiped,const D3DXVECTOR3& vDest,float fFacing, float fSpeed, float fVerticalSpeed, bool bForceRun, bool bForceWalk);


		/**
		* search all OPC in the sentient object list and hide them if any of them's LastUpdateTime is smaller than a internally defined value, such as 5 seconds
		*/
		void HideTimeoutOPC();

	private:
		/** update the game objects, find out the terrain tile the game object is currently located. and detect
		which objects are it each object's perceptive radius. and wake up or sleep corresponding actors. */
		void UpdateGameObjects(double dTimeDelta);

		/** obsoleted: search all OPC in the sentient object list and make them inactive if any of them has not been updated for the given time, 
		* yet it does not remove them from the sentient list. 
		*/
		void InactivateOPC();
		/** obsoleted: search all OPC in the sentient object list and remove inactive ones.*/
		void RemoveInactiveOPC();
		
		/// a list of terrain that contains all active biped.
		list <ActiveTerrain*>	m_listActiveTerrain;
		/// currently active biped: mobile or its brain is functioning, this may contain biped
		/// that is out of the view-culling radius. 
		list <ActiveBiped*>		m_listActiveBiped;

		/// very important biped list
		list <CBipedObject*> m_listVIPBipeds;

		/**
		* Current network simulation time. It usually begins from 0 when a server starts. and increase by one every frame move. 
		*/
		CNetTime m_time;
	};

}
