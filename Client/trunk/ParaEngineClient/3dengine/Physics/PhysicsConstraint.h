#pragma once

#include "Physics/PhysicsObject.h"

namespace ParaEngine {

	class CPhysicsRigidBody;

	class CPhysicsConstraint : public CPhysicsObject
	{
		friend class CPhysicsFactory;
	protected:
		CPhysicsConstraint(IParaPhysicsConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB);
	public:
		virtual ~CPhysicsConstraint();

		IParaPhysicsConstraint* get();

		//ATTRIBUTE_DEFINE_CLASS(CPhysicsConstraint);
		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID() { return ATTRIBUTE_CLASSID_CPhysicsConstraint; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "CPhysicsConstraint"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }

		ATTRIBUTE_METHOD1(CPhysicsConstraint, isEnabled_s, bool*) { *p1 = cls->isEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CPhysicsConstraint, setEnabled_s, bool) { cls->setEnabled(p1); return S_OK; }

		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		bool isEnabled() const;
		void setEnabled(bool enabled);

	protected:
		IParaPhysicsConstraint* m_pConstraint;

		ref_ptr<CPhysicsRigidBody> m_rbA;
		ref_ptr<CPhysicsRigidBody> m_rbB;
	};


	class CPhysicsP2PConstraint : public CPhysicsConstraint
	{
		friend class CPhysicsFactory;
	protected:
		CPhysicsP2PConstraint(IParaPhysicsPoint2PointConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB);
	public:
		virtual ~CPhysicsP2PConstraint();
		//ATTRIBUTE_DEFINE_CLASS(CPhysicsP2PConstraint);
		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID() { return ATTRIBUTE_CLASSID_CPhysicsP2PConstraint; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "CPhysicsP2PConstraint"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }
	};

	class CPhysicsHingeConstraint : public CPhysicsConstraint
	{
		friend class CPhysicsFactory;
	protected:
		CPhysicsHingeConstraint(IParaPhysicsHingeConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB);
	public:
		virtual ~CPhysicsHingeConstraint();
		//ATTRIBUTE_DEFINE_CLASS(CPhysicsHingeConstraint);
		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID() { return ATTRIBUTE_CLASSID_CPhysicsHingeConstraint; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "CPhysicsHingeConstraint"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }
	};

	class CPhysicsSliderConstraint : public CPhysicsConstraint
	{
		friend class CPhysicsFactory;
	protected:
		CPhysicsSliderConstraint(IParaPhysicsSliderConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB);
	public:
		virtual ~CPhysicsSliderConstraint();
		//ATTRIBUTE_DEFINE_CLASS(CPhysicsSliderConstraint);
		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID() { return ATTRIBUTE_CLASSID_CPhysicsSliderConstraint; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "CPhysicsSliderConstraint"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }
	};

	class CPhysicsConeTwistConstraint : public CPhysicsConstraint
	{
		friend class CPhysicsFactory;
	protected:
		CPhysicsConeTwistConstraint(IParaPhysicsConeTwistConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB);
	public:
		virtual ~CPhysicsConeTwistConstraint();
		//ATTRIBUTE_DEFINE_CLASS(CPhysicsConeTwistConstraint);
		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID() { return ATTRIBUTE_CLASSID_CPhysicsConstraint; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "CPhysicsConstraint"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }
	};
	
	class CPhysicsGeneric6DofSpringConstraint : public CPhysicsConstraint
	{
		friend class CPhysicsFactory;
	protected:
		CPhysicsGeneric6DofSpringConstraint(IParaPhysicsGeneric6DofSpringConstraint* pConstraint, CPhysicsRigidBody* rbA, CPhysicsRigidBody* rbB);
	public:
		virtual ~CPhysicsGeneric6DofSpringConstraint();
		//ATTRIBUTE_DEFINE_CLASS(CPhysicsGeneric6DofSpringConstraint);
		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID() { return ATTRIBUTE_CLASSID_CPhysicsGeneric6DofSpringConstraint; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "CPhysicsGeneric6DofSpringConstraint"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }
	};

	
}