#include "ParaEngine.h"
#include "MCIController.h"

using namespace ParaEngine;

#if defined(WIN32)
	#include "Win32MCI.h"
#elif defined(PLATFORM_MAC) | defined(__APPLE__)
	// include mac version mci cpp
#elif defined(__linux__)
	// include Linux version mci cpp
#else
	// 
#endif

MCIController::MCIController()
	:m_pImpl(nullptr)
{
	#if defined(WIN32)
		m_pImpl = new Win32MCI();
	#elif defined(PLATFORM_MAC)
	// TODO: MAC version
	#elif defined(__linux__)
	// TODO: Linux Version
	#else
	//
	#endif
}

MCIController::~MCIController()
{
	if (m_pImpl) {
		m_pImpl->Release();
		delete m_pImpl;
		m_pImpl = nullptr;
	}
}

bool MCIController::Start()
{
	return m_pImpl != nullptr ? m_pImpl->Start() : false;
}

bool MCIController::Stop()
{
	return m_pImpl != nullptr ? m_pImpl->Stop() : false;
}

bool MCIController::Save(const char* fileName)
{
	return m_pImpl != nullptr ? m_pImpl->Save(fileName) : false;
}