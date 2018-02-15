#pragma once

#include <list>
#include <vector>

namespace ParaEngine
{
	using namespace std;
	/**
	* data structures used to describe the task that a AI module is planning to do or is currently doing
	* the difference between a movie track in the AI task and that from a global eye is that
	* the biped associated with this Movie track would interact with the physical environment
	* simulator. And the biped will use its intelligence to complete the movie even with some
	* interventions in the course, such as being blocked or attacked. Once a movie track is 
	* completed it is deleted from the task list. the movie task will override some other task
	* and may coexist with the rest.
	*/
	struct AITask_MovieTrack
	{
	public:
	
		struct Keyframe
		{
			/// object will always walk to the position (vPos), during which time (sAnimation) is 
			/// played. if the animation is does not contain a speed, then the biped must first
			/// walk there with the default walk animation and play (sAnimation) til the time (fDuration)
			/// is up. 
			/// animation of the biped to play in this frame.
			string sAnimation;		
			/// position of the biped when this frame ends
			Vector3 vPos;		
			/// facing of the biped when this frame ends
			float	fFacing;		
			/// time (in seconds) within which this frame must end, and proceed to the next one in the list
			float	fDuration;		
			Keyframe();
			Keyframe(const char* anim, float x, float y, float z, float facing, float duration);
		};

	public:
		list <Keyframe>		listKeyframes;
		/// how long  a frame has been playing
		float				fStartTime;     
		/// For how long the biped has been finding to reach its position specified in a key frame.
		float				fPathFindingTime; 


		AITask_MovieTrack();
		AITask_MovieTrack(const char * str);
	};

	struct AITask_DieAndReborn
	{
	public:
		/// in seconds
		float		m_fRebornTimeLeft; 

		AITask_DieAndReborn(float fRebornTimeLeft)
		{
			m_fRebornTimeLeft = fRebornTimeLeft;
		};
	};

	struct AITask_WanderNearby
	{
	public:
		float		m_fX, m_fY;
		float		m_fRadius;

		AITask_WanderNearby(float fRadius, float fX, float fY)
		{
			m_fRadius = fRadius;
			m_fX = fX; 
			m_fY = fY;
		};
	};

	struct AITask_Evade
	{
	public:
		char		m_sTarget[20];
		/// distance within which to stop and start attacking
		float		m_fDist1;	
		/// distance within which to retreat to dist1
		float		m_fDist2;	

		AITask_Evade(const char* target, float distance1, float distance2);
		AITask_Evade(const char *);
	};

	class CBipedObject;
	struct AITask_Follow
	{
	public:
		char		m_sTarget[20];
		/// distance within which to stop following
		float		m_fDist1;	
		/// distance outside which to start following
		float		m_fDist2;	
		CBipedObject*	m_pTarget;

		AITask_Follow(const char* target, float distance1, float distance2);
		AITask_Follow(const char *);
	};

	/**
	* a union of all kinds of task defined
	*/
	struct AITask
	{
	public:
		enum AITaskType	{
			DieAndReborn = 0, 
			WanderNearby = 1,
			Evade=2, 
			Follow=3, 
			Movie=4
		}m_nType;
		/// set to true if finished
		bool m_bIsFinished;	
		union
		{
			AITask_DieAndReborn*	pAITask_DieAndReborn;
			AITask_WanderNearby*	pAITask_WanderNearby;
			AITask_Evade*			pAITask_Evade;
			AITask_Follow*			pAITask_Follow;
			AITask_MovieTrack*		pAITask_Movie;
			void*					pAITask;		//anomynous
		};
	public:
		AITask();
		AITask(AITaskType type, void* pt);
		void DestroyMe();
	};
}