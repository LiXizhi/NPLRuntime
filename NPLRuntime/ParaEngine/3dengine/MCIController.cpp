#include "ParaEngine.h"
#include "MCIController.h"

#include <stdio.h>

using namespace ParaEngine;

#if defined(WIN32)

#include <Windows.h>

#define ALIAS "recsound"


class Win32MCI: public MCIController::Interface
{
public:
	~Win32MCI(){}

	bool Start();

	bool Stop();

	bool Save(const char* fileName);
};

bool Win32MCI::Start()
{
	char mci_command[512];
	char ReturnString[512];
	int mci_error;
	sprintf(mci_command, "open new Type waveaudio Alias %s", ALIAS);
	mci_error = mciSendString(mci_command, ReturnString, sizeof(ReturnString), NULL);

	// start recording
	sprintf(mci_command, "record %s", ALIAS);
	mci_error = mciSendString(mci_command, ReturnString, sizeof(ReturnString), NULL);
	return mci_error == 0 ? true : false;
}

bool Win32MCI::Stop()
{
	char mci_command[512];
	char ReturnString[512];
	int mci_error;
	//stop recording
	sprintf(mci_command, "stop %s", ALIAS);
	mci_error = mciSendString(mci_command, ReturnString, sizeof(ReturnString), NULL);

	return mci_error == 0 ? true : false;
}

bool Win32MCI::Save(const char* fileName)
{
	char mci_command[512];
	char ReturnString[512];
	int mci_error;

	// save the file
	sprintf(mci_command, "save %s %s", ALIAS, fileName);
	mci_error = mciSendString(mci_command, ReturnString, sizeof(ReturnString), NULL);

	// close the device
	sprintf(mci_command, "close %s", ALIAS);
	mci_error = mciSendString(mci_command, ReturnString, sizeof(ReturnString), NULL);

	return mci_error == 0 ? true : false;
}

#endif

MCIController::MCIController()
	:m_pImpl(nullptr)
{
	#if defined(WIN32)
		m_pImpl = new Win32MCI();
	#elif defined(PLATFORM_MAC)
	// TODO: MAC version
	#else
	// TODO: Linux Version
	#endif
}

MCIController::~MCIController()
{
	if (m_pImpl) {
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