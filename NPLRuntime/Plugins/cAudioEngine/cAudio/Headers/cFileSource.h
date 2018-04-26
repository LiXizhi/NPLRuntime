// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IDataSource.h"
#include "cAudioDefines.h"
#include "cMemoryOverride.h"

#if CAUDIO_COMPILE_WITH_FILE_SOURCE == 1

#include <fstream>

namespace cAudio
{

class cFileSource : public IDataSource, public cMemoryOverride
{
    public:
        cFileSource(const char* filename);
        ~cFileSource();

        virtual bool isValid();
        virtual int getCurrentPos();
        virtual int getSize();
        virtual int read(void* output, int size);
        virtual bool seek(int amount, bool relative);
    
    protected:
		//! Holds if valid        
		bool Valid;
		//! Holds file size
        int Filesize;
		//! File stream
        FILE* pFile;
    private:
};

};

#endif

