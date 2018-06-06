// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "../Headers/cFileSource.h"
#include "../Headers/cUtils.h"

#ifdef CAUDIO_COMPILE_WITH_FILE_SOURCE

#include <cstring>

namespace cAudio
{

cFileSource::cFileSource(const char* filename) : pFile(NULL), Valid(false), Filesize(0)
{
	cAudioString safeFilename = safeCStr(filename);
    if(safeFilename.length() != 0)
    {
		pFile = fopen(safeFilename.c_str(),"rb");
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
