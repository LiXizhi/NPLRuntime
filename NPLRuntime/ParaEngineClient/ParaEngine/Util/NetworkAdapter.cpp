#include "ParaEngine.h"
#include "NetworkAdapter.h"
#ifdef WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#include <sstream>

#pragma comment(lib, "IPHLPAPI.lib")
#endif

namespace ParaEngine
{

	NetworkAdapter::NetworkAdapter() :m_pNextAdapter(NULL), m_bInitialized(false)
	{
		Init();
	}
	NetworkAdapter::~NetworkAdapter()
	{

	}

	NetworkAdapter* NetworkAdapter::GetInstance()
	{
		static NetworkAdapter s_singleton;
		return &s_singleton;
	}

	const std::string& NetworkAdapter::GetMaxMacAddress()
	{
		return m_sMacAddress;
	}

	const std::string& NetworkAdapter::GetMaxIPAddress()
	{
		return m_sIpAddress;
	}

	void NetworkAdapter::Init()
	{
		if (m_bInitialized)
			return;
		m_bInitialized = true;
#ifdef WIN32
		DWORD result = 0;
		ULONG iterations = 0;
		ULONG  bufferSize = 15000;
		PIP_ADAPTER_ADDRESSES pAddresses = NULL;

		//get all adapter info
		do
		{
			pAddresses = (IP_ADAPTER_ADDRESSES *)HeapAlloc(GetProcessHeap(), 0, bufferSize);
			result = GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &bufferSize);
			if (result == ERROR_BUFFER_OVERFLOW)
			{
				HeapFree(GetProcessHeap(), 0, pAddresses);
				pAddresses = NULL;
			}
			else
			{
				break;
			}
			iterations++;
		} while ((result == ERROR_BUFFER_OVERFLOW) && iterations < 3);


		int maxValue = 0;
		uint32_t maxIPv4 = 0;
		if (result == NO_ERROR)
		{
			PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
			while (pCurrAddresses)
			{
				if (pCurrAddresses->PhysicalAddressLength != 0)
				{
					int macValueSum = 0;
					std::stringstream ss;
					ss << std::hex;

					//read mac address bits
					for (int i = 0; i < (int)pCurrAddresses->PhysicalAddressLength; i++)
					{
						int macBits = pCurrAddresses->PhysicalAddress[i];
						macValueSum += macBits;

						ss << (int)pCurrAddresses->PhysicalAddress[i];

					}

					if (maxValue < macValueSum)
					{
						maxValue = macValueSum;
						m_sMacAddress = ss.str();
					}
				}
				if (pCurrAddresses->FirstUnicastAddress)
				{
					sockaddr_in* address = (sockaddr_in*)pCurrAddresses->FirstUnicastAddress->Address.lpSockaddr;
					uint32_t ipv4 = address->sin_addr.S_un.S_addr;
					// always get the largest
					if (maxIPv4 < ipv4)
					{
						Color ipv4(ipv4);
						// skip local loopback address like 127.0.0.1
						if (ipv4.b != 127 && ipv4.b != 0 && ipv4.b != 255)
						{
							maxIPv4 = ipv4;
							std::stringstream ss;
							ss << (int)ipv4.b << "." << (int)ipv4.g << "." << (int)ipv4.r << "." << (int)ipv4.a;
							m_sIpAddress = ss.str();
						}
					}
				}
				pCurrAddresses = pCurrAddresses->Next;
			}
		}

		if (pAddresses)
		{
			HeapFree(GetProcessHeap(), 0, pAddresses);
		}

		//test
		//MessageBox(NULL,m_sMacAddress.c_str(),"mac address",0);
#endif
	}

}
