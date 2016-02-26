#pragma once
#include "AIBase.h"

namespace ParaEngine
{
	/* forward declarition */
	class CBipedObject;
	class CFaceTrackingCtrler;
	class CFollowCtrler;
	class CMovieCtrler;
	class CSequenceCtler;
	
	/** NPC AI module: it is just an aggregation of AI controllers.*/
	class CAIModuleNPC : public CAIBase
	{
	public:
		virtual AIObjectType GetType(){return NPC;};

		CAIModuleNPC(CBipedObject* pBiped);
		CAIModuleNPC(void);
		virtual ~CAIModuleNPC(void);

	public:
		/** enable face tracking 
		* @param bFaceTracking: true to enable, false to disable. */
		void EnableFaceTracking(bool bFaceTracking);
		bool IsFaceTrackingEnabled();
		CFaceTrackingCtrler* GetFaceController();

		/** enable the follow controller to follow a named biped within its perceptive radius. 
		* @param sName: if this is "", the follow controller will be disabled.
		*/
		void FollowBiped(const std::string& sName);
		bool IsFollowEnabled();
		CFollowCtrler* GetFollowController();

		/** enable or disable a movie controller. all other controllers are disabled when a movie controller is active.
		* @param bEnable: true to enable it.
		*/
		void EnableMovieControler(bool bEnable);
		bool IsMovieControlerEnabled();
		/** return the movie controller. if there is no movie controller, NULL is returned.*/
		CMovieCtrler* GetMovieController();

		/** enable or disable a sequence controller. a sequence controller has the lowest execution priority compared to other controllers.
		* @param bEnable: true to enable it.
		*/
		void EnableSequenceControler(bool bEnable);
		bool IsSequenceControlerEnabled();
		/** return the sequence controller. if there is no sequence controller, NULL is returned.*/
		CSequenceCtler* GetSequenceController();

		/** a virtual function which is called every frame to process the controller. 
		* @param fDeltaTime: the time elapsed since the last frame move is called. 
		* @param pInput: It holds any information that is perceived by a Active Biped object
		*/
		virtual void FrameMove(float fDeltaTime);
	private:
		CFaceTrackingCtrler*	m_pFaceTrackingCtrler;
		CFollowCtrler*			m_pFollowCtrler;
		CMovieCtrler*			m_pMovieCtrler;
		CSequenceCtler*			m_pSequenceCtrler;
	};

}
