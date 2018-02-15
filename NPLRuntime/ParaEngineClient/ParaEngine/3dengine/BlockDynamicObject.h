#pragma once
#include "BlockEngine/BlockCommon.h"
#include "TileObject.h"

namespace ParaEngine
{
	/* base class for dynamic object in block world. 
	* The base implementation supports simple sphere physics.
	*/
	class CBlockDynamicObject : public CTileObject
	{
	public:
		CBlockDynamicObject();
		virtual ~CBlockDynamicObject();

		ATTRIBUTE_DEFINE_CLASS(CBlockDynamicObject);
		ATTRIBUTE_SUPPORT_CREATE_FACTORY(CBlockDynamicObject);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);


		ATTRIBUTE_METHOD1(CBlockDynamicObject, GetLifetime_s, float*)	{ *p1 = cls->GetLifetime(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockDynamicObject, SetLifetime_s, float)	{ cls->SetLifetime(p1); return S_OK; }
						  
		ATTRIBUTE_METHOD1(CBlockDynamicObject, GetFadeOutTime_s, float*)	{ *p1 = cls->GetFadeOutTime(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockDynamicObject, SetFadeOutTime_s, float)	{ cls->SetFadeOutTime(p1); return S_OK; }
						  
		ATTRIBUTE_METHOD1(CBlockDynamicObject, CanBounce_s, bool*)	{ *p1 = cls->CanBounce(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockDynamicObject, SetCanBounce_s, bool)	{ cls->SetCanBounce(p1); return S_OK; }
						  
		ATTRIBUTE_METHOD1(CBlockDynamicObject, GetSpeed_s, Vector3*)	{ *p1 = cls->GetSpeed(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockDynamicObject, SetSpeed_s, Vector3)	{ cls->SetSpeed(p1); return S_OK; }


	public:
		virtual bool IsTileObject() { return false; };
		virtual void SetPosition(const DVector3 & v);

		virtual void Animate(double dTimeDelta, int nRenderNumber = 0);

		/** check if the object's original shape can be seen via a camera. it will return false if the object is completely out of the 6 frustum planes of the camera
		*/
		virtual bool TestCollision(CBaseCamera* pCamera);

		void UpdateParams();

		float GetLifetime() const { return m_lifetime; }
		void SetLifetime(float val) { m_lifetime = val; }

		float GetFadeOutTime() const { return m_fade_out_time; }
		void SetFadeOutTime(float val) { m_fade_out_time = val; }

		/** whether the object will bounce when hitting the ground. */
		bool CanBounce() const { return m_can_bounce; }
		void SetCanBounce(bool val) { m_can_bounce = val; }

		const Vector3& GetSpeed() const { return m_vSpeed; }
		void SetSpeed(const Vector3& val) { m_vSpeed = val; }


		virtual void SetParent(CBaseObject* pParent);
		virtual CBaseObject* GetParent();
		virtual bool IsDead();
		virtual void SetDead();

		bool IsOnGround() const;
		void SetOnGround(bool val);
	protected:
		
		Vector3           m_vSpeed;

		Vector3           m_vBlockCenter;
		Vector3           m_vOffsetFromCenter;
		Uint16x3          m_vBlockPos;

		/** in seconds*/
		float m_lifetime;
		float m_fade_out_time;

		bool m_is_stopped;
		bool m_can_bounce;
		bool m_isOnGround;

		bool m_bIsDead;
	
		CBaseObject::WeakPtr_type m_parent;
	public:
		static float default_min_speed;
		static float default_surface_decay;
		static float default_air_decay;
		static float default_gravity;
		static float default_speedlost_perbounce;
	};
}


