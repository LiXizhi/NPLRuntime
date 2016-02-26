#pragma once
#include "ValueTracker.h"
#include <map>

/** how many times per second that a client will send normal update message to server.*/
#define CLIENT_NORMAL_UPDATE_RATE	3

/** how many times per second that a server will send normal update message to clients.*/
#define SERVER_NORMAL_UPDATE_RATE	3

namespace ParaEngine
{
	using namespace std;
	/**
	* Current network simulation time. It usually begins from 0 when a server starts. and increase by one every frame move. 
	*/
	class CNetTime
	{
	public:
		CNetTime()
		{
			Reset();
		}

		void Reset()
		{
			m_nTime = 0;
		}

		inline int GetTime()
		{
			return m_nTime;
		}

		/**
		* increase the time by 1 frame
		*/
		void FrameMove()
		{
			++m_nTime;
		}
	private:
		int m_nTime;
	};

	/**
	* OBSOLETED: It represents a normal update packet.
	*/
	class CNormalUpdatePacket
	{
	public:
		CNormalUpdatePacket(void);
		~CNormalUpdatePacket(void);
	};

	/**
	* It tracks a character on the network, such as its last update time, position track, etc. 
	*/
	class CCharacterTracker
	{
	public:
		CCharacterTracker(void){
			m_nLastSendTime.resize(3, INVALID_TIME);
			m_nLastReceiveTime.resize(3, INVALID_TIME);
			m_vPos.SetSize(6);
			m_arrayCustomModelData.SetSize(1);
		};
		~CCharacterTracker(void){};

	public:
		/**
		* reset to tacker to empty
		*/
		void Reset()
		{
			for(int i=0;i<(int)m_nLastSendTime.size();++i)
				m_nLastSendTime[i] = INVALID_TIME;

			for(int i=0;i<(int)m_nLastReceiveTime.size();++i)
				m_nLastReceiveTime[i] = INVALID_TIME;

			m_vPos.Reset();
			m_fFacing.Reset();
			m_fVerticalSpeed.Reset();
			m_fSpeed.Reset();
			m_nAnimID.Reset();
			m_bWalking.Reset();
			m_bRunning.Reset();
			m_bIsGlobal.Reset();

			m_fSizeScale.Reset();
			m_sBasemodel.Reset();
			m_bIsCustomModel.Reset();
			m_arrayCustomModelData.Reset();
			m_nSkinIndex.Reset();
		}
		/// last time that the character info is send to the other side of the network
		int GetLastSendTime(int nGroupIndex = 0)
		{
			if(nGroupIndex>=0 && nGroupIndex<(int)m_nLastSendTime.size())
				return m_nLastSendTime[nGroupIndex];
			else
				return INVALID_TIME;
		}
		/// last time that the character info is send to the other side of the network
		void SetLastSendTime(int nTime, int nGroupIndex = 0)
		{
			if(nGroupIndex>=0 && nGroupIndex<(int)m_nLastSendTime.size())
				m_nLastSendTime[nGroupIndex] = nTime;
		}

		/// last time that the character info is received from the other side of the network.
		int GetLastReceiveTime(int nGroupIndex = 0)
		{
			if(nGroupIndex>=0 && nGroupIndex<(int)m_nLastReceiveTime.size())
				return m_nLastReceiveTime[nGroupIndex];
			else
				return INVALID_TIME;
		}

		/// last time that the character info is received from the other side of the network.
		void SetLastReceiveTime(int nTime, int nGroupIndex = 0)
		{
			if(nGroupIndex>=0 && nGroupIndex<(int)m_nLastReceiveTime.size())
				m_nLastReceiveTime[nGroupIndex] = nTime;
		}

		/**
		* this function will return true if all field of the tracker are constant.
		*/
		bool IsContant()
		{
			return IsGroupConstant(1) && IsGroupConstant(2);
		}

		/**
		* this function will return true if all trackers in the given group are a constant.
		*/
		bool IsGroupConstant(int nGroup)
		{
			switch(nGroup)
			{
			case 1:
				return m_vPos.IsConstant() && m_fFacing.IsConstant() &&  m_fVerticalSpeed.IsConstant() &&  m_fSpeed.IsConstant() &&  m_nAnimID.IsConstant() && 
					m_bWalking.IsConstant() &&  m_bRunning.IsConstant();
			case 2:
				return  m_fSizeScale.IsConstant() && m_sBasemodel.IsConstant();
			default:
				return false;
			}
		}

	public:
		//////////////////////////////////////////////////////////////////////////
		// group 1: position, facing, speed and animation
		CVector3Tracker m_vPos; /// character position tracker
		CFloatTracker	m_fFacing;
		CFloatTracker	m_fVerticalSpeed;
		CFloatTracker	m_fSpeed;
		CIntTracker		m_nAnimID;
		CBooleanTracker m_bWalking;
		CBooleanTracker m_bRunning;
		CBooleanTracker m_bIsGlobal; // not in any group

		//////////////////////////////////////////////////////////////////////////
		// group 2: appearance and attribute
		CFloatTracker	m_fSizeScale;
		CStringTracker m_sBasemodel;
		CBooleanTracker m_bIsCustomModel; // not in any group
		CNetByteArrayTracker m_arrayCustomModelData; // not in any group
		CIntTracker		m_nSkinIndex; // not in any group
	private:
		/// last time that the character info is send to the other side of the network
		vector<int>	 m_nLastSendTime;

		/// last time that the character info is received from the other side of the network.
		vector<int>	 m_nLastReceiveTime;
		// invalid time
		static const int INVALID_TIME = -1;
	};



	/**
	* it is a class for holding various character trackers by their name. 
	* some network object, such as the COPCObject and EnvSimClient may derive from it. 
	*/
	class ICharacterTrackerContainer
	{
	public:
		ICharacterTrackerContainer();
		virtual ~ICharacterTrackerContainer();

		/** reset all character trackers. */
		void ResetCharTrackers();

		/**
		* Get the tracker for a character
		* @param sName if this is NULL, it means this character. Otherwise, it is a character in its perception.
		* @return if NULL, the tracker is not found and one should call CreateCharTracker() to create it.
		*/
		CCharacterTracker* GetCharTracker(const char* sName=NULL);

		/**
		* create a new character tracker with the given name. If the tracker already exists, it will be emptied.
		* @param sName if this is NULL, it means this character. Otherwise, it is a character in its perception.
		* @return the tracker created is returned.
		*/
		CCharacterTracker* CreateCharTracker(const char* sName=NULL);

		/**
		* remove a given character tacker by name
		* @param sName name of the tacker. 
		* @return true if removed.
		*/
		bool RemoveCharTracker(const char* sName);
		/**
		* remove any tracker whose perceived time is smaller than or equal to nBeforePerceivedTime.
		* this function may be called periodically at low frame rate to release memory.
		* note: it will not remove newly created trackers, which has time 0. 
		* @param nBeforePerceivedTime this should be above 1 to prevent newly created trackers to be removed.
		* @return the number of tackers removed are returned
		*/
		int RemoveCharTracker(int nBeforePerceivedTime);


		/** get last character tracker garbage collection time. */
		int GetLastTrackerGCTime() {return m_nLastTrackerGCTime;};
		/** set last character tracker garbage collection time. */
		void SetLastTrackerGCTime(int nLag) {m_nLastTrackerGCTime = nLag;};

		/** get the time since last network synchronization. */
		int GetTimeLag() {return m_nTimeLag;};
		/** set the time since last network synchronization. */
		void SetTimeLag(int nLag) {m_nTimeLag = nLag;};
	private:
		/** time since last network synchronization. */ 
		int		m_nTimeLag;
		/** last character tracker garbage collection time. */ 
		int		m_nLastTrackerGCTime;

		CCharacterTracker m_charTracker;
		map<string, CCharacterTracker> m_perceptionTracker;
	};

}

