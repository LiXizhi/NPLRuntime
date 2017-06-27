#pragma once

#include "ParaScriptingCommon.h"
#include "ParaScriptingGlobal.h"

namespace ParaEngine
{
	class CPhysicsDynamicsWorld;
	class CPhysicsFactory;
}

namespace ParaScripting
{
	/*
	class PE_CORE_DECL ParaPhysicsObject
	{
	public:
		typedef ParaEngine::weak_ptr<ParaEngine::IObject, ParaEngine::CPhysicsObject> WeakPtr_type;
		// a pointer to the object
		WeakPtr_type m_pObj;
	};
	*/

	class PE_CORE_DECL ParaPhysicsWorld
	{
	public:
		typedef ParaEngine::weak_ptr<ParaEngine::IObject, ParaEngine::CPhysicsDynamicsWorld> WeakPtr_type;
		WeakPtr_type m_pObj;

		ParaPhysicsWorld() {};
		ParaPhysicsWorld(ParaEngine::CPhysicsDynamicsWorld* pObj);
		~ParaPhysicsWorld() {};

		/**
		* check if the object is valid
		*/
		bool IsValid() const { return m_pObj; };

		/** get the attribute object associated with an object. */
		ParaAttributeObject GetAttributeObject();
		/** for API exportation*/
		void GetAttributeObject_(ParaAttributeObject& output);
	};


	class PE_CORE_DECL ParaPhysicsFactory
	{
	public:
		typedef ParaEngine::weak_ptr<ParaEngine::IObject, ParaEngine::CPhysicsFactory> WeakPtr_type;
		// a pointer to the object
		WeakPtr_type m_pObj;

		ParaPhysicsFactory() {};
		ParaPhysicsFactory(ParaEngine::CPhysicsFactory* pObj);
		~ParaPhysicsFactory();

		ParaEngine::CPhysicsFactory* get() const {
			return m_pObj.get();
		};

		/**
		* check if the object is valid
		*/
		bool IsValid() const { return m_pObj; };

		/** get the attribute object associated with an object. */
		ParaAttributeObject GetAttributeObject();
		/** for API exportation*/
		void GetAttributeObject_(ParaAttributeObject& output);

		ParaPhysicsWorld GetCurrentWorld();

		/* create a box shape*/
		//CPhysicsShape* CreateBoxShape(const PARAVECTOR3& boxHalfExtents);
	};
}