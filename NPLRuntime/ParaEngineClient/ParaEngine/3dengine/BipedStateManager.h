#pragma once
#include <stack>

namespace ParaEngine
{
	class CBipedObject;
	struct ActionKey;

	using namespace std;
	/** managing biped state */
	class CBipedStateManager
	{
	public:
		CBipedStateManager(void);
		CBipedStateManager(CBipedObject * pBiped);
		~CBipedStateManager(void);

		enum BipedState{
			STATE_MOVING = 0,
			STATE_WALK_FORWORD,
			STATE_RUN_FORWORD,
			STATE_WALK_LEFT,
			STATE_WALK_RIGHT,
			STATE_WALK_BACKWORD,
			STATE_WALK_POINT,
			STATE_SWIM_FORWORD,
			STATE_SWIM_LEFT,
			STATE_SWIM_RIGHT,
			STATE_SWIM_BACKWORD,
			STATE_FLY_DOWNWARD,
			STATE_STANDING = 100,	// without speed
			STATE_IN_WATER,			// under or in water
			STATE_SWIM,
			STATE_JUMP_IN_AIR,		// in air 
			STATE_JUMP_START,
			STATE_JUMP_END,
			STATE_STAND,
			STATE_TURNING,
			STATE_ATTACK,
			STATE_ATTACK1,
			STATE_ATTACK2,
			STATE_MOUNT,	// mount on target
			STATE_DANCE,
		};
		///** state object that saves in memory*/
		//struct BipedStateObj{
		//	BipedState	m_state;
		//	void *		m_data;
		//public:
		//	BipedStateObj(BipedState s, void* data):m_state(s), m_data(data){};
		//};
		enum ActionSymbols{
			S_STANDING = 0,		/// ensure the biped has no speed
			S_IN_WATER,			/// make sure that the biped is in water
			S_ON_FEET,			/// make sure that the biped is not in water (on land or in air)
			S_ON_WATER_SURFACE, /// make sure that the biped is on water surface (not in water or in air)
			POP_ACTION, /// pop the current action
			S_STAND,
			S_WALK_FORWORD,
			S_RUN_FORWORD,
			S_WALK_LEFT,
			S_WALK_RIGHT,
			S_WALK_POINT,		// walking to a point
			S_TURNING,
			S_WALK_BACKWORD,
			S_SWIM_FORWORD,
			S_SWIM_LEFT,
			S_SWIM_RIGHT,
			S_SWIM_BACKWORD,
			S_JUMP_START,
			S_JUMP_IN_AIR,	// not used.
			S_JUMP_END,
			S_MOUNT,
			S_FALLDOWN,
			S_ATTACK,
			S_ATTACK1,
			S_ATTACK2,
			S_DANCE,
			S_ACTIONKEY,  // perform the action in the action key, immediately.
			S_FLY_DOWNWARD,
			S_NONE
		};
	private:
		/// the state memory
		list <BipedState> m_memory;
		/// the biped object that is associated with this state manager.
		CBipedObject* m_pBiped;
		/// state timer, it is used for state transition timing. 
		/// for example, one can define the time from state1 to state2.
		float m_fTimer;
		/** last animation state, which is the animation played in the last frame.
		* this is used for AI or other modules to check for the current biped animations.*/
		BipedState m_nLastAnimState;
		/** true if character uses walk as the default moving animation.otherwise it uses running.
		* default value is running.*/
		bool m_bWalkOrRun:1;
		/** whether the biped's state is being recorded to the movie controllers. Default value is false. */
		bool m_bRecording:1;
		/** whether the biped is mounted on another object. */
		bool m_bIsMounted:1;

		/* User data */
		DVector3 m_vPos; // for position user data
		float m_fAngleDelta; // for angle user data
		float m_fJumpupSpeed;
	protected:
		/** remove all occurrence of a given state */
		void RemoveState(BipedState s);
		/** make sure that the memory has space left for one more state*/
		void CheckMemory();

		/** replace the current state with the one in the input*/
		void ReplaceState(BipedState s);
		/** add a new state to state memory */
		void PushState(BipedState s);
		/** all occurrence of the same state will be removed from the memory before this state is pushed to the memory*/
		void PushUniqueState(BipedState s);
		/** prepend a new state to state memory*/
		void PrependState(BipedState s);
		/** all occurrence of the same state will be removed from the memory before this state is prepended to the memory*/
		void PrependUniqueState(BipedState s);
		/** ensure that state s is unique in the memory. If there is one than one such state, it will be deleted.*/
		void SetUniqueState(BipedState s);
	public:
		/** whether the object is mounted on another object. */
		bool IsMounted();
		/** set mount state */
		void SetMounted(bool bIsMounted);
		/** whether the state is being recorded to a movie controller.*/
		bool IsRecording();
		/** Set the recording state */
		void SetRecording(bool bIsRecording = true);
		/** return true if character uses walk as the default moving animation.otherwise it uses running.*/
		bool WalkingOrRunning();
		/** Set default moving style
		*@param bWalk: true if character uses walk as the default moving animation.otherwise it uses running.*/
		void SetWalkOrRun(bool bWalk);


		/** get the default animation name for a given state.
		* @param s: the biped state
		* @param sName: [out] a buffer to get the name
		* @param nNameSize: size of the name in bytes, the default value is 8. 
		* @return: true if sName is filled(an animation is found). Otherwise false.
		*/
		bool GetStateAnimName(BipedState s, char* sName, int nNameSize=8);

		/** last animation state, which is the animation played in the last frame.
		* this is used for AI or other modules to check for the current biped animations.*/
		BipedState GetLastAnimState();
		/** Find a specified state in memory. 
		* @param s: the state to be found.
		* @return: If the state is not found, a negative value(-1) will be returned; otherwise the current index of the
		* first occurrence of the state is returned.*/
		int FindStateInMemory(BipedState s);
		
		/** get the last biped state */
		BipedState GetLastState();
		/** get the last biped state */
		BipedState GetFirstState();
		
		static bool IsStandingState(BipedState s);
		static bool IsMovingState(BipedState s);
		/** whether the memory contains at least one moving state.*/
		bool HasMovingState();
		/// whether the biped is swimming
		bool IsSwimming();
		/// whether the biped  is flying
		bool IsFlying();

		CBipedObject* GetBiped();
		void SetBiped(CBipedObject* pBiped);

		/** read an action symbol, and let the manager determine which states it should go to.
		* this function will not perform any concrete actions on the biped objects. Call 
		* Update() method to perform these actions on the associated biped after adding series of 
		* actions to the state manager. The recommended order of calling is
		*  StateManager::Update()->Environment Simulator->IO:{StateManager::AddAction}
		* @param nAct: the action
		* @param data: the data specifying more details about the action. This value default to NULL
		*	if nAct is S_ACTIONKEY, then pData is const ActionKey* 
		*	if nAct is S_WALK_POINT, then pData is NULL or 1, specifying whether to use angle.
		* @return: the current state is returned. */
		BipedState AddAction(ActionSymbols nAct, const void* pData=NULL);

		/** set user data: angle delta */
		void SetAngleDelta(float fAngleDelta);
		/** Get user data: angle delta */
		float GetAngleDelta();

		/** set user data: jump up speed */
		void SetJumpupSpeed(float fSpeed);
		/** Get user data: jump up speed */
		float GetJumpupSpeed();

		/** set user data: position*/
		void SetPos(const DVector3& v);
		/** Get user data: position*/
		const DVector3& GetPos();

		/** update the associated biped according to the memory or state of the manager.
		* this function is automatically by each active biped during each frame move.
		* it may sometimes change the memory of state manager. But in most cases, it just carries out 
		* actions according to the current state. 
		* @param fTimeDelta: the time elapsed since the last time this function is called. Units in seconds.*/
		void Update(float fTimeDelta);
		friend class CAutoCamera;
	};
}
