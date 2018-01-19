#pragma once
#include "BipedController.h"

namespace ParaEngine
{
	using namespace std;
	class SequenceEntity;
	class CBipedObject;

	/** a facing tracking controller 
	* it will allow the biped to always face to a given target or another biped. */
	class CFaceTrackingCtrler : public CBipedController
	{
	public:
		CFaceTrackingCtrler(void);
		CFaceTrackingCtrler(CAIBase* pAI);
		virtual ~CFaceTrackingCtrler(void);
	
	public:
		/** a virtual function which is called every frame to process the controller. 
		* @param fDeltaTime: the time elapsed since the last frame move is called. 
		* @param pInput: It holds any information that is perceived by a Active Biped object
		*/
		virtual void FrameMove(float fDeltaTime);

		/**
		* instruct a character to face a target for a certain duration. It will smoothly rotate the character neck to face it
		@param vPos: which point in world space to face to
		@param fDuration: how many seconds the character should keeps facing the target. Set zero or a negative value to cancel facing previous point. 
		*/
		void FaceTarget(const Vector3* pvPos, float fDuration);

	private:
		/** the character will face a given target for this amount of time.
		* if this is zero or negative value, the m_vTargetPoint is ignored. 
		*/
		float m_fTargetTimer;

		/** which point that this character should try to face to.*/
		DVector3 m_vTargetPoint;
	};
}