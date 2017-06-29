#pragma once

#include "Physics/PhysicsObject.h"

namespace ParaEngine {
	class CPhysicsShape;
	class CPhysicsConstraint;

	class CPhysicsBody : public CPhysicsObject
	{
		friend class CPhysicsFactory;
	protected:
		CPhysicsBody(IParaPhysicsBody* pBody);

	public:
		virtual ~CPhysicsBody();

		IParaPhysicsBody* get();

		//ATTRIBUTE_DEFINE_CLASS(CPhysicsBody);
		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID() { return ATTRIBUTE_CLASSID_CPhysicsBody; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "CPhysicsBody"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }

		int GetInternalType() const;

		// don't remove any body in this callback, because it will be called any times in one tick
		typedef std::function<void(CPhysicsBody* bodyA, CPhysicsBody* bodyB, float dis, const PARAVECTOR3& posA, const PARAVECTOR3& posB, const PARAVECTOR3& normalOnB)> ContactCallBack;
		void SetContactCallBack(const ContactCallBack& cb);

	protected:
		IParaPhysicsBody* m_pBody;
		ContactCallBack m_cb;
	};

	class CPhysicsRigidBody : public CPhysicsBody
	{
		friend class CPhysicsFactory;
	protected:
		CPhysicsRigidBody(IParaPhysicsRigidbody* pBody, CPhysicsShape* pShape);
	public:
		virtual ~CPhysicsRigidBody();

		//ATTRIBUTE_DEFINE_CLASS(CPhysicsRigidBody);
		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID() { return ATTRIBUTE_CLASSID_CPhysicsRigidBody; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "CPhysicsRigidBody"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }

		void SetConstraint(CPhysicsConstraint* p);
		bool isConstrainted();

		// is this rigidbody added to physics world?
		bool isInWorld() const;
		// convert this body to kinematic body
		void Convert2Kinematic();

		typedef weak_ptr<IObject, CPhysicsRigidBody>  WeakPtr;
	private:
		ref_ptr<CPhysicsShape> m_pShape;
		CPhysicsConstraint* m_pConstraint;
	};
}