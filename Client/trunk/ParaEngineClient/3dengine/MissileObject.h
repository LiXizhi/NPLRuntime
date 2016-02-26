#pragma once
#include "TileObject.h"
#include "AnimInstanceBase.h"

namespace ParaEngine
{
	// forward declaration.
	struct ParaXEntity;

	/** a flying missile in the global space. the missile will usually delete itself automatically.*/
	class CMissileObject : public CTileObject
	{
	public:
		virtual CBaseObject::_SceneObjectType GetType(){ return CBaseObject::MissileObject; };
		CMissileObject(void);
		virtual ~CMissileObject(void);

		ATTRIBUTE_DEFINE_CLASS(CMissileObject);
		ATTRIBUTE_SUPPORT_CREATE_FACTORY(CMissileObject);
	private:
		
		/** speed of the missile type object.*/
		float m_fSpeed;

		/** the name of the destination object. this is only used for missile type object. the destination object 
		* must be the global scene objects. In most cases, it is another character object. */
		string m_sDestObjectName;
		/** the target position*/
		DVector3 m_vTargetPos;
		/** this is high resolution m_vPos.*/
		DVector3 m_vCurrentPos;
		/** whether the missile is fired. once fired, it will be independent of its source object. and will travel towards the targets.*/
		bool m_bFired:1;
		/** whether the missile is already exploded.*/
		bool m_bExploded:1;

		//------------ for rendering the missile model --------------------//
		/** the animated particle model for rendering of the missile object. */
		ParaXEntity * m_pParaXModel;

		/** the time since the missile is launched. */
		float m_fTime;

		/** the time at which the missile will automatically explode, if not hitting any target.*/
		float m_fExplotionTime;

		/** scale of the parax model.*/
		float m_fModelScale;

		// ------------- model animation information -----------------------//
		/** current animation index, this is different from sequence ID 
		* an absolute ParaX frame number denoting the current animation frame. It is always within
		* the range of the current animation sequence's start and end frame number. */
		AnimIndex m_CurrentAnim;
	
		/** by how much the blending frame should be blended with the current frame. 
		* 1.0 will use solely the blending frame, whereas 0.0 will use only the current frame.
		* [0,1), blendingFrame*(blendingFactor)+(1-blendingFactor)*currentFrame */
		float m_blendingFactor;

	public:
		/** whether it is exploded. an exploded missile shall not be rendered any more.*/
		bool IsExploded(){return m_bExploded;}
		/** to explode the missile object. */
		void Explode();
		/** whether the missile is fired from the source object. */
		bool IsFired(){return m_bFired;}
		
		/** fire a missile at a target using the specified effect model and speed. 
		* if there is no target or the target is itself, the missile will not be created.
		* the missile will automatically delete itself, when the missile has either reached the target or the target is lost.
		*/
		HRESULT InitObject(ParaXEntity *model, const string& sDestObject, float fSpeed, float fModelScale=1.0f);
		/** fire a missile at a target using the specified effect model and speed. 
		* the missile will automatically delete itself, when the missile has either reached the target or the target is lost.
		*/
		HRESULT InitObject(ParaXEntity *model, const Vector3& vTargetPos, float fSpeed, float fModelScale=1.0f);


		//-- virtual function from baseObject
		virtual HRESULT Draw( SceneState * sceneState);
		virtual void Animate( double dTimeDelta, int nRenderNumber=0  );
		virtual IViewClippingObject* GetViewClippingObject();
		
		virtual void SetPosition(const DVector3 & v);

		/// used as KEY for batch rendering
		virtual AssetEntity* GetPrimaryAsset(){return (AssetEntity*)m_pParaXModel;};
	};

}
	