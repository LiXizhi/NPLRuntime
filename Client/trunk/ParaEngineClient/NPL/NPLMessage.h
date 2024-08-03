#pragma once
#include "util/StringBuilder.h"
#include "util/PoolBase.h"

#include <boost/core/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace NPL
{
	/**
	* message type in NPLMessage
	*/
	enum NPLMessageType
	{
		/// normal file activation 
		MSG_TYPE_FILE_ACTIVATION = 0,
		MSG_TYPE_COMMAND_BEGIN,
		MSG_TYPE_RESET,
		/// exit the runtime state
		MSG_TYPE_EXIT,
		/// frame move tick
		MSG_TYPE_TICK,
		/// load without activation
		MSG_TYPE_FILE_LOAD,
	};

	/** an NPL message in the message queue.
	* always create using reference counted pointer like this: 
	*	NPLMessage_ptr msg(new NPLMessage(...));
	*/
	struct NPLMessage: 
		public ParaEngine::PoolBase<NPLMessage>,
		public ParaEngine::intrusive_ptr_single_thread_base,
		private boost::noncopyable
	{
	public:
		NPLMessage();
		~NPLMessage();
	public:
		/// message type
		NPLMessageType m_type;
		/// NPL or dll file name
		std::string m_filename;
		/// must be secure code. 
		ParaEngine::StringBuilder m_code;
	};


	//////////////////////////////////////////////////////////////////////////
	//
	// NPL packet structures
	//
	//////////////////////////////////////////////////////////////////////////
#ifdef OLD_CODE
	/* Specifies packing alignment for NPL packet structures.*/
#pragma pack(push,1)
	/**
	* Obsoleted: The NPL neuron file activate() function packet body
	*/
	struct PKG_ActivateNeuron
	{
		// TODO: make variable length transmission. 
		/// local file name
		char sFilePath[MAX_FILENAME_LEN];

		// TODO: make variable length transmission. 
		/// each receiver address is separated by ';'
		char sUIReceviers[MAX_FILENAME_LEN];

		/** length of sCode in bytes.*/
		int nCodeLength;
	public:
		PKG_ActivateNeuron()
		{
			sFilePath[0] = '\0';
			sUIReceviers[0] = '\0';
			nCodeLength = 0;
		}
		/** return the sCode, NULL will be returned if there is no sCode.*/
		const char* GetSCode()
		{
			if(nCodeLength==0)
				return NULL;
			else
				return (const char*)((byte*)this)+sizeof(PKG_ActivateNeuron);
		}
	};

	/**
	* Obsoleted: The NPL glia file activate() function package body
	*/
	struct PKG_ActivateGlia
	{
		/// if true, it means activating the glia file, otherwise, it means loading but not activating.
		bool bActivate;
		// TODO: make variable length transmission. 
		/// local file name
		char sFilePath[MAX_FILENAME_LEN];

		// TODO: make variable length transmission. 
		/// the DNS server which will parse the name spaces that appear within the file.
		char sDNSServerName[MAX_FILENAME_LEN];

		/** length of sCode in bytes.*/
		int nCodeLength;

	public:
		PKG_ActivateGlia()
		{
			bActivate = true;
			sFilePath[0] = '\0';
			sDNSServerName[0] = '\0';
			nCodeLength = 0;
		}
	public:
		/** get the size of the packet in bytes.*/
		int GetPackageSize() const {
			return sizeof(PKG_ActivateGlia);
		}
		/** return the sCode, NULL will be returned if there is no sCode.*/
		const char* GetSCode()
		{
			if(nCodeLength==0)
				return NULL;
			else
				return (const char*)((byte*)this)+sizeof(PKG_ActivateGlia);
		}
	};
#pragma pack(pop)
#endif
}