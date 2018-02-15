#pragma once
#include "AssetEntity.h"
#include <vector>
#include <map>

namespace ParaEngine
{
	using namespace std;
	/** a reusable entity of sequence which contains an array of sequence items. 
	Sequence Entity can be stored in file or database*/
	class SequenceEntity : public AssetEntity
	{
	public:
		SequenceEntity();
		SequenceEntity(const AssetKey& key);
		virtual ~SequenceEntity();
		
		enum CommandType
		{
			CMD_RunTo,// void RunTo(int x,int y,int z), relative to current position
			CMD_WalkTo,// void WalkTo(int x,int y,int z), relative to current position
			CMD_MoveTo,// void MoveTo(int x,int y,int z), relative to current position
			CMD_PlayAnim,//void PlayAnim(string sAnim)
			CMD_Wait, // void Wait(float fSeconds)
			CMD_Exec, // void Exec()
			CMD_Pause, // void Pause()
			CMD_Turn, // void Turn(float fAngleAbsolute)
			CMD_MoveForward, // void Move(fDistance)
			CMD_MoveBack, // void Move(fDistance)
			CMD_MoveLeft, // void Move(fDistance)
			CMD_MoveRight, // void Move(fDistance)
			CMD_Jump, // void Jump();
			CMD_Goto, // void Goto(int nOffset)
			CMD_Invaild,
		};
		enum SEQ_PLAYMODE
		{
			PLAYMODE_FORWORD,
			PLAYMODE_FORWORD_LOOPED,
			PLAYMODE_BACKWORD,
			PLAYMODE_BACKWORD_LOOPED,
			PLAYMODE_ZIGZAG, // first forward and then backward repeatedly
		};
		/** a sequence item or command*/
		struct SequenceItem
		{
			SequenceItem():m_commandtype(CMD_Invaild), m_vPos_R(0,0,0){};
			SequenceItem(CommandType type):m_commandtype(type), m_vPos_R(0,0,0){};
			CommandType m_commandtype;
			/** objective position relative to the last one*/
			Vector3 m_vPos_R;
			/** objective position relative to the start of the sequence. This value is automatically calculated according to m_vPos_R and should not be assigned by the user.*/
			DVector3 m_vPos;
			/** objective absolute facing */
			float	m_fFacing;

			/** any string parameter associated with the command */
			string		m_strParam;
			
			/** any value type parameter associated with the command */
			union{
				int		m_nGotoOffset;
				float	m_fWaitSeconds;
				float	m_fMoveDistance;
				DWORD	m_dwValue;
			};
		};
		vector<SequenceItem> m_items;

		/** description */
		string			m_description;
		/** whether how sequence should be played. */
		SEQ_PLAYMODE	m_nPlayMode;
		/** initial position in world coordinate */
		DVector3		m_vStartPos;
		/** initial facing */
		float			m_fStartFacing;

		/** Label mapping: <sLabel, nIndex> */
		map<string, int> m_lables;
	public:
		virtual HRESULT InitDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();
		virtual void Cleanup();
		/** reset this object */
		void Reset();

		/** convert to NPL script string. */
		string ToString();

		/** add a new item */
		void PushItem(const SequenceItem& item){m_items.push_back(item);};
		/** add a new label */
		void AddLable(const string& sLable);
		/** return the index of the lable, if the lable does not exists 0 is returned. */
		int GetLableIndex(const string& sLable);

		/** nIndex must be valid. */
		SequenceItem& GetItem(int nIndex){return m_items[nIndex];};
		/** get total key count*/
		int GetTotalKeys(){return (int)m_items.size();};
		/** delete keys range 
		@param nFrom: 0 based index. 
		@param nTo: 0 based index, if -1, it means the last one. */
		bool DeleteKeysRange(int nFrom, int nTo);
		
	protected:
		/** after loading keys, must call this function to compute the automatic parameters in key items. */
		void CompileKeys();
		friend class CSequenceCtler;
	};
}