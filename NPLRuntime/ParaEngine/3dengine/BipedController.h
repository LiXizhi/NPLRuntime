#pragma once
#include "KeyFrame.h"

namespace ParaEngine
{
	/* forward declarition */
	class CAIBase;
	class CBaseObject;
	class CBipedObject;
	class IGameObject;

	/**
	* Base Interface for Biped controller. The AI module contains a collection of biped controllers
	* for directing the behavior of the biped.Each biped controller derived from this class may
	* control a certain aspect of the biped, such as UserController, MovieController, OPCController
	* NPLController, FacingController, CollisionController, etc.
	*/
	class CBipedController
	{
	public:
		CBipedController(CAIBase* pAI);
		CBipedController(void);
		virtual ~CBipedController(void);
	private:
		/** whether the controller is suspended , so it does not take effects on the next frame move.*/
		bool	m_bSuspended;
		/** to which this biped controller is associated.*/
		CAIBase*		m_pAI;

	protected:
		/** total time elapsed since the controller is active(not suspended). */
		float	m_fTimeElapsed;
	
	public:
		/** check whether the controller is active(not suspended).*/
		bool IsActive();
		/** suspend the controller, so it does not take effects on the next frame move.*/
		virtual void Suspend();
		/** resume the controller. */
		virtual void Resume();
		/** set the internal timer. This is useful when the behavior is relevant to the timer.*/
		virtual void SetTime(float fTime);
		/** get the current movie time*/
		float GetTime();
		/** get the biped in the perceived biped list, which is closet to the current biped. 
		* @param pInput: the active biped list.
		* return: NULL is returned if there is no closest biped. The pointer returned is only valid where pInput is valid*/
		IGameObject* GetClosestBiped(IGameObject* pInput);

		/** Find a biped with a matching name. * is supported,, which matches any character(s). 
		* @param pInput: the active biped list.
		* @param searchString: e.g. "LiXizhi", "LXZ*"
		*/
		IGameObject* FindBiped(IGameObject* pInput, const std::string& searchString);

		/** get the biped, to which this biped controller is associated. */
		CBipedObject* GetBiped();
		/** set the AI object to which this object is associated.*/
		void SetAI(CAIBase* pAI);
		
		/** a virtual function which is called every frame to process the controller. 
		* @param fDeltaTime: the time elapsed since the last frame move is called. 
		*/
		virtual void FrameMove(float fDeltaTime);
	};

	/** a follow controller 
	* it will follow a given target or another biped. 
	* please note that the controller will only follow an object within its perceptive radius. */
	class CFollowCtrler : public CBipedController
	{
	public:
		CFollowCtrler(void);
		CFollowCtrler(CAIBase* pAI);
		virtual ~CFollowCtrler(void);
	private:
		/// name of the biped to follow
		std::string		m_sFollowTarget;
		/// default radius around the target biped. it will control the biped to try it best to stand on this circle.
		float	m_fRadius;
		/// it will control the biped to stand beside the target with the target facing shifted by this value. 
		/// note that +-Pi means behind the biped. 
		float	m_fAngleShift;
	public:
		/*
		* Follow a biped at a specified circle position.
		* @param obj: format "sName radius angle" | "sName"
		* sName: it is the name of the biped to follow,
		* radius: [optional, default to 2.5f] it is the default radius around the target biped. it will control the biped to try it best to stand on this circle.
		* angle: [optional, default to Pi] it will control the biped to stand beside the target with the target facing shifted by this value. 
		* note that +-Pi means behind the biped. 
		* e.g. "lixizhi", "lixizhi 2.5 3.14", "lixizhi 3.0 0", "lixizhi 3.0 1.57", "lixizhi 3.0 -1.57"
		*/
		void SetFollowTarget(const std::string& obj);
		const std::string& GetFollowTarget();

		/** a virtual function which is called every frame to process the controller. 
		* @param fDeltaTime: the time elapsed since the last frame move is called. 
		* @param pInput: It holds any information that is perceived by a Active Biped object
		*/
		virtual void FrameMove(float fDeltaTime);
	};
}
