#pragma once
//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2006.10
// Description: 
//-----------------------------------------------------------------------------
#include <list>
#include "ShapeSphere.h"

namespace ParaEngine
{
	using namespace std;

	class CBipedObject;
	class CTerrainTile;
	struct ActiveBiped;
	class CBaseObject;
	class CNetTime;
	class CShapeSphere;

	/// Type of the collision
	enum CollisionPairType
	{
		/// Object A run into object B, so A should reponse 
		_A_runinto_B,         
		/// Object B run into object A, so B should reponse 
		_B_runinto_A,         
		/// Object A,B run into each other, both or the faster one should reponse 
		_AB_runinto_eachother 
	};

	/// a pair of object that collide into each other. And desciption of 
	/// how they collide.
	struct CollisionPair
	{
	public:
		CBaseObject* pA;
		CBaseObject* pB;
		CollisionPairType cptCollisionType;
	public:
		CollisionPair()
		{
			cptCollisionType = _A_runinto_B; // default type
		}
	};


	//-----------------------------------------------------------------
	// intermediate object definitions
	//-----------------------------------------------------------------
	struct ActiveTerrain
	{
		CTerrainTile*	pTile;
		list <ActiveBiped*> listActiveBiped;
	};

	//----------------------------------------------------------------
	/// It holds any information that is perceived by a Active Biped object
	/// including himself. This is one of the major product that environment
	/// simulator produces for each living bipeds.
	//----------------------------------------------------------------
	struct ActiveBiped
	{
		struct PerceivedBiped
		{
			/// the distance of the perceived biped to the host
			float fDistance;	
			CBipedObject* pBiped;
			PerceivedBiped(float fDist, CBipedObject* pB){
				pBiped = pB;
				fDistance = fDist;
			};
			PerceivedBiped(){pBiped=NULL;};
		};
		/// the biped scene object.
		CBipedObject*	pBiped;		
		/// tile that tells most exactly where the object is in the scene
		/// the terrain in which the biped is in
		CTerrainTile*	pTerrain;
		/// object list that collide with this biped. It can be solid object or other bipeds.
		list <CBaseObject*>	listCollisionPairs;
		/// object list that this biped could see or perceive. this includes all object
		/// that is in the perceptive radius.
		list <PerceivedBiped*> listPerceptibleBipeds;
	public:
		ActiveBiped(CBipedObject* b){
			pBiped = b;
			pTerrain = NULL;
		}
		~ActiveBiped(){
			list< PerceivedBiped* >::iterator itCurCP1, itEndCP1 = listPerceptibleBipeds.end();
			for( itCurCP1 = listPerceptibleBipeds.begin(); itCurCP1 != itEndCP1; ++ itCurCP1)
			{
				delete *itCurCP1;
			}
			listPerceptibleBipeds.clear();
		}
	};

	/**
	* environment simulator. 
	*/
	class IEnvironmentSim
	{
	public:
		enum SIMULATOR_TYPE{
			SIM_TYPE_STANDALONE = 0,
			SIM_TYPE_SERVER = 1,
			SIM_TYPE_CLIENT = 2,
		};
		/**
		* return the current type of simulator. 0 means standalone simulator, 1 means server, 2 means client. 
		*/
		virtual SIMULATOR_TYPE GetSimulatorType()=0;

		/** called each frame to simulate the game world. 
		* @param dTimeDelta: in seconds.
		*/
		virtual void Animate( double dTimeDelta ) = 0;

		/**
		* when this function is called, it ensures that the physics object around one or more points are properly loaded.
		* It increases the hit count of these physics objects by 1. The garbage collector in the physics world
		* may use the hit count to move out unused static physics object from the physics scene (Novodex).
		* This function might be called for the current player, each active mobile object in the scene and the camera eye position.
		* @param points: if not NULL, it will further check if it hits any of the sphere points. 
		* @param nPointCount: the number of points defined in points. 
		*/
		virtual void CheckLoadPhysics(CShapeSphere* points, int nPointCount = 1) = 0;
		void CheckLoadPhysics(const Vector3& vCenter, float fRadius)
		{
			CShapeSphere point(vCenter, fRadius);
			CheckLoadPhysics(&point, 1);
		}

		/** delete this object. Such as Delete this;*/
		virtual void Release() = 0;

		static void EnableLog(bool bWriteLog){m_bWriteLog = bWriteLog;};
		static inline bool IsLogEnabled(){return m_bWriteLog;};

		/**
		* get the net time object
		* @return 
		*/
		virtual CNetTime* GetNetTime(){return NULL;};

	private:
		static bool m_bWriteLog;
	};
}