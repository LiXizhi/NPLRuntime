#pragma once
#include "BipedController.h"

namespace ParaEngine
{
	using namespace std;
	class SequenceEntity;
	class CBipedObject;

	/** A sequence controller is a biped controller which moves the biped according to some predefined sequence. */
	class CSequenceCtler : public CBipedController
	{
	public:
		CSequenceCtler();
		CSequenceCtler(CAIBase* pAI);
		virtual ~CSequenceCtler(void);
	public:
		/** a virtual function which is called every frame to process the controller. 
		* @param fDeltaTime: the time elapsed since the last frame move is called. 
		*/
		virtual void FrameMove(float fDeltaTime);

		bool Save(bool bOverride);
		bool Load(int nSequenceID);
		bool Load(const string& fileName);
		int  Create(const string& name, const string& description, const char* pData,bool bInMemory);
		string ToString();

		/** Get the current absolute playing cursor position*/
		int GetKeyPos(){return m_nKeyPos;};
		/** set the current absolute playing cursor position*/
		void SetKeyPos(int nPos);
		
		/** get total key count*/
		int GetTotalKeys();
		/** offset the key index according to the current play mode. i.e. it will automatically wrap to the beginning if looping. 
		@param nOffset: number of keys to advance. 
		@return: the number of keys that have been successfully offseted. Usually if the returned value is not equal to the input value, it means
		that the sequence should be paused. */
		int AdvanceKey(int nOffset);
		/** call the command functions(RunTo, MoveTo, etc) only between the matching pair of BeginAddKeys() and EndAddKeys()*/
		void BeginAddKeys();
		/** call the command functions(RunTo, MoveTo, etc) only between the matching pair of BeginAddKeys() and EndAddKeys()*/
		void EndAddKeys();
		/** get sequence ID*/
		int GetSequenceID();
		/** delete keys range 
		@param nFrom: 0 based index. 
		@param nTo: 0 based index, if -1, it means the last one. */
		bool DeleteKeysRange(int nFrom, int nTo);
		
		/** get the play direction. */
		bool GetPlayDirection(){return m_bForward;};
		/** set the play direction. */
		void SetPlayDirection(bool bForward){m_bForward = bForward;};
		
		/** the minimum time between two successive calls. */
		float GetInterval(){return m_fMinInterval;}
		/** the minimum time between two successive calls. */
		void SetInterval(float fInterval){m_fMinInterval = fInterval;}

		/** get the starting position. */
		Vector3 GetStartPos(){return m_vStartPos;}
		/** set the starting position. */
		void SetStartPos(const Vector3& vPos){m_vStartPos = vPos;}

		/** get the start facing. usually default to 0. */
		float GetStartFacing(){return m_fStartFacing;}
		/** Set the start facing. usually default to 0. */
		void SetStartFacing(float facing){m_fStartFacing = facing;}

		/** get the current play mode */
		int GetPlayMode(){return (int)m_nPlayMode;}
		/** set the current play mode */
		void SetPlayMode(int mode);
		
		/** get the number of seconds after which all move commands will be treated as finished. 
		default value is 30 seconds. */
		float GetMovingTimeout(){return m_fMovingTimeOut;}
		/** set the number of seconds after which all move commands will be treated as finished. 
		default value is 30 seconds. */
		void SetMovingTimeout(float fTimeout){m_fMovingTimeOut=fTimeout;}

	//////////////////////////////////////////////////////////////////////////
	// commands:
	public:
		/** run to a position relative to the current position. */
		void RunTo(double x, double y, double z);
		/** walk to a position relative to the current position. */
		void WalkTo(double x, double y, double z);
		/** move (using the current style i.e. walk or run) to a position relative to the current position. */
		void MoveTo(double x, double y, double z);
		/** move to a given position and turn to a given facing. */
		void MoveAndTurn(double x, double y, double z, float facing);
		/** play an animation by animation name */
		void PlayAnim(const string& sAnim);
		/** play an animation by animation id */
		void PlayAnim(int nAnimID);
		/** wait the specified seconds, without further processing commands */
		void Wait(float fSeconds);
		/** execute a given script command*/
		void Exec(const string& sCmd);
		/** pause the sequence infinitely until some one resumes it. */
		void Pause();
		/** turn the character to face a given absolute direction in radian value. */
		void Turn(float fAngleAbsolute);
		/** move forward using the current facing a given distance*/
		void MoveForward(float fDistance);
		/** move backward using the current facing a given distance*/
		void MoveBack(float fDistance);
		/** move left using the current facing a given distance*/
		void MoveLeft(float fDistance);
		/** move right using the current facing a given distance*/
		void MoveRight(float fDistance);
		/** Jump once */
		void Jump();
		/** offset the current sequence commands by a given steps. */
		void Goto(int nOffset);
		/** offset to a label, if label not found, it will wrap to the beginning. */
		void Goto(const string& sLable);
		/** add a new label at the current position. */
		void Lable(const string& sLable);
	
		/** execute a move command
		* @param pBiped: the biped to move
		* @param vDest: relative position to the current position.
		* @param bForceRun: force using running animation.
		* @param bForceWalk: force using walking animation.
		* @param pFacing: if it is not NULL, we will face the new target, after reaching the position. 
		*/
		static void MoveCommand(CBipedObject* pBiped, const DVector3& vDest, bool bForceRun=false, bool bForceWalk=false, float * pFacing=NULL);
		/**
		* stop a biped if it is moving.
		* @param pBiped: the biped to stop
		*/
		static void Stop(CBipedObject* pBiped);

	private:
		asset_ptr<SequenceEntity> m_pSequenceEntity;
		
		enum SEQ_STATE{
			SEQ_EMPTY,	 // empty 
			SEQ_CREATED, // only resides in memory
			SEQ_MANAGED, // is being managed by the manager
		}m_nSequenceState;
		/** sequence name */
		string			m_name;
		/** description */
		string			m_description;
		/** whether how sequence should be played. */
		int	m_nPlayMode;
		/** initial position in world coordinate */
		Vector3		m_vStartPos;
		/** initial facing */
		float			m_fStartFacing;
		int m_nKeyPos;
		bool m_bIsAddingKey;
		/** whether is playing forward*/
		bool m_bForward;
		/** 1/FPS, where FPS is the frequency of valid activation. */
		float m_fMinInterval;
		float m_fUnprocessedTime;
		/** the number seconds that a sequence item has been executed. */
		float m_fItemDuration;
		/** the number of seconds after which all move commands will be treated as finished. 
		default value is 30 seconds. */
		float m_fMovingTimeOut;
		/** default to 1 seconds */
		float m_fTurningTimeOut;
		
		/** copy the entity parameter to this controller.*/
		void CopyEntityParamter(const SequenceEntity& e);
		/** assign a new sequence to replace the current one. */
		void SetEntity(SequenceEntity* e);
	};

}
