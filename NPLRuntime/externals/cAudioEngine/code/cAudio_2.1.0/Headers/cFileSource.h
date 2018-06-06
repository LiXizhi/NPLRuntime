// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#ifndef CFILESOURCE_H
#define CFILESOURCE_H

#include "../include/IDataSource.h"
#include "../include/cAudioDefines.h"
#include "../Headers/cMemoryOverride.h"

#ifdef CAUDIO_COMPILE_WITH_FILE_SOURCE

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

#endif //! CFILESOURCE_H
