// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cMemorySource.h"

#include <cstring>

namespace cAudio
{

cMemorySource::cMemorySource(const void* data, int size, bool copy) : Data(NULL), Size(0), Valid(false), Pos(0)
{
    if(data && size > 0)
    {
        Size = size;
        if(copy)
        {
            Data = (char*)CAUDIO_MALLOC(Size);
			if(Data)
				memcpy(Data, data, Size);
        }
        else
        {
            Data = (char*)data;
        }
		if(Data)
			Valid = true;
    }
}

cMemorySource::~cMemorySource()
{
    CAUDIO_FREE(Data);
}

bool cMemorySource::isValid()
{
    return Valid;
}

int cMemorySource::getCurrentPos()
{
    return Pos;
}

int cMemorySource::getSize()
{
    return Size;
}

int cMemorySource::read(void* output, int size)
{

    //memset(output, 0, size);
    if(Pos+size <= Size)
    {
        memcpy(output, Data+Pos, size);
        Pos += size;
        return size;
    }
    else
    {
        int extra = (Pos+size) - Size;
        int copied = size - extra;
        memcpy(output, Data+Pos, copied);
        Pos = Size;
        return copied;
    }
}

bool cMemorySource::seek(int amount, bool relative)
{
    if(relative)
    {
        Pos += amount;
        if(Pos > Size)
        {
            Pos = Size;
            return false;
        }
    }
    else
    {
        Pos = amount;
        if(Pos > Size)
        {
            Pos = Size;
            return false;
        }
    }

    return true;
}

};
