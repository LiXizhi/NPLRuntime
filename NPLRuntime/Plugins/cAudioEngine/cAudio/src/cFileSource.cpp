// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cFileSource.h"
#include "cUtils.h"

#if CAUDIO_COMPILE_WITH_FILE_SOURCE == 1

#include <cstring>

namespace cAudio
{

cFileSource::cFileSource(const char* filename) : pFile(NULL), Valid(false), Filesize(0)
{
	cAudioString safeFilename = fromUTF8(filename);
    if(safeFilename.length() != 0)
    {
		pFile = fopen(toUTF8(safeFilename),"rb");
		if(pFile)
			Valid = true;
    }

    if(Valid)
    {
        fseek(pFile, 0, SEEK_END);
        Filesize = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);
    }
}

cFileSource::~cFileSource()
{
	if(pFile)
		fclose(pFile);
}

bool cFileSource::isValid()
{
    return Valid;
}

int cFileSource::getCurrentPos()
{
    return ftell(pFile);
}

int cFileSource::getSize()
{
    return Filesize;
}

int cFileSource::read(void* output, int size)
{
	return fread(output, sizeof(char), size, pFile);
}

bool cFileSource::seek(int amount, bool relative)
{
    if(relative == true)
    {
        int oldamount = ftell(pFile);
        fseek(pFile, amount, SEEK_CUR);

        //check against the absolute position
        if(oldamount+amount != ftell(pFile))
            return false;
    }
    else
    {
        fseek(pFile, amount, SEEK_SET);
        if(amount != ftell(pFile))
            return false;
    }

    return true;
}

};

#endif
