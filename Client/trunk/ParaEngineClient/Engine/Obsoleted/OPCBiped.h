#pragma once
#include "RpgCharacter.h"
#include "GameNetCommon.h"

namespace ParaEngine
{
	/**
	* represent a player from the network.
	*/
	class COPCBiped : public CRpgCharacter, public ICharacterTrackerContainer
	{
	public:
		virtual CBaseObject::_SceneObjectType GetType(){return CBaseObject::OPCBiped;};
		COPCBiped(void);
		virtual ~COPCBiped(void);
		/** obsoleted: hash code of the connection object that is created from connectionAddress and connectionType */
		size_t	hash_code;
		static size_t GetStringHashValue(const char* str);

		/// hash code functions
		//void SetIdentifier(const char * sID);
		void SetHashCode(size_t hashvalue);
		void ReComputeHashcode();

		/** the active boolean is usually used by simulator for different purposes. */
		bool IsActive(){return m_bActive;};
		/** the active boolean is usually used by simulator for different purposes. */
		void SetActive(bool bActive){m_bActive = bActive;};

	private:
		/** whether the OPC is active during simulation.*/
		bool	m_bActive;

	};
}