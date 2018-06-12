
#if defined(WIN32)

#include "ParaEngine.h"
#include "Win32MCI.h"
#include <Windows.h>
#include <stdio.h>


#define ALIAS "recsound"

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

void Win32MCI::Release()
{

}

#endif