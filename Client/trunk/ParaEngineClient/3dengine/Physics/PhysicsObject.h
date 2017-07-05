#pragma once

#include "ParaEngine.h"
#include "IAttributeFields.h"
#include "Physics/IParaPhysics.h"

namespace ParaEngine {
	class CPhysicsObject : public IAttributeFields
	{
	public:
		CPhysicsObject() : m_pUserData(nullptr) {};
		virtual ~CPhysicsObject() {};

		void SetUserData(void* pData) { m_pUserData = pData; };
		void* GetUserData() { return m_pUserData; };

		//ATTRIBUTE_DEFINE_CLASS(CPhysicsObject);
		virtual int GetAttributeClassID() { return ATTRIBUTE_CLASSID_CPhysicsObject; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "CPhysicsObject"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }
	private:
		void* m_pUserData;
	};
}
