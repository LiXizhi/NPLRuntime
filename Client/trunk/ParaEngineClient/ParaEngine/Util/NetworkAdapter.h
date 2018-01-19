
#pragma once

#include <string>


namespace ParaEngine
{
	class NetworkAdapter
	{
	public:
		NetworkAdapter();
		~NetworkAdapter();

		/** get the default adapter */
		static NetworkAdapter* GetInstance();

		/**	local ip address
		* return value format is something like "bcaec5bd9833"
		*/
		const std::string & GetMaxMacAddress();

		/** local ip address
		*/
		const std::string& GetMaxIPAddress();
		
		void Init();
	public:
		bool m_bInitialized;
		std::string m_sMacAddress;
		std::string m_sIpAddress;
		NetworkAdapter * m_pNextAdapter;
	};
}