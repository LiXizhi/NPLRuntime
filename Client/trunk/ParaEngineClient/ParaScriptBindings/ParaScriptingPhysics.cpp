#include "ParaEngine.h"
#include "ParaScriptingPhysics.h"
#include "Physics/PhysicsFactory.h"
#include "Physics/PhysicsObject.h"
#include "Physics/PhysicsDynamicsWorld.h"

namespace ParaScripting
{
	ParaPhysicsWorld::ParaPhysicsWorld(ParaEngine::CPhysicsDynamicsWorld* pObj)
		: m_pObj(pObj)
	{

	}

	/** get the attribute object associated with an object. */
	ParaAttributeObject ParaPhysicsWorld::GetAttributeObject()
	{
		return ParaAttributeObject(m_pObj);
	}

	/** for API exportation*/
	void ParaPhysicsWorld::GetAttributeObject_(ParaAttributeObject& output)
	{
		output = GetAttributeObject();
	}




	ParaPhysicsFactory::ParaPhysicsFactory(ParaEngine::CPhysicsFactory* pObj)
		: m_pObj(pObj)
	{
	}

	ParaPhysicsFactory::~ParaPhysicsFactory()
	{

	}

	/** get the attribute object associated with an object. */
	ParaAttributeObject ParaPhysicsFactory::GetAttributeObject()
	{
		return ParaAttributeObject(m_pObj);
	}

	/** for API exportation*/
	void ParaPhysicsFactory::GetAttributeObject_(ParaAttributeObject& output)
	{
		output = GetAttributeObject();
	}

	ParaPhysicsWorld ParaPhysicsFactory::GetCurrentWorld()
	{
		if (IsValid())
		{
			return ParaPhysicsWorld(m_pObj->GetCurrentWorld());
		}
		else
		{
			return ParaPhysicsWorld();
		}
	}
}