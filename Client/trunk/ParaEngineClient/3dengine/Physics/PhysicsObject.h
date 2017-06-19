#pragma once

#include "Physics/IParaPhysics.h"

namespace ParaEngine {
	class CPhysicsObject : public IAttributeFields
	{
	public:
		CPhysicsObject() : m_pUserData(nullptr) {};
		virtual ~CPhysicsObject() {};

		void SetUserData(void* pData) { m_pUserData = pData; };
		void* GetUserData() { return m_pUserData; };

		ATTRIBUTE_DEFINE_CLASS(CPhysicsObject);
	private:
		void* m_pUserData;
	};
}
