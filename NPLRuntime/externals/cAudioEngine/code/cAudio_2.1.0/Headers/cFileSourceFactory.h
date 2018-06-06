// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#ifndef CFILESOURCEFACTORY_H
#define CFILESOURCEFACTORY_H

#include "../include/IDataSourceFactory.h"

#ifdef CAUDIO_COMPILE_WITH_FILE_SOURCE

#include "../Headers/cFileSource.h"
#include "../Headers/cMemorySource.h"
#include "../Headers/cMutex.h"

namespace cAudio
{
	class cFileSourceFactory : public IDataSourceFactory
	{
		public:
			cFileSourceFactory() { }
			virtual ~cFileSourceFactory() { }

			virtual IDataSource* CreateDataSource(const char* filename, bool streamingRequested)
			{
				cAudioMutexBasicLock lock(Mutex);
				IDataSource* source = CAUDIO_NEW cFileSource(filename);

				if(!streamingRequested && source && source->isValid())
				{
					//A bit hackish, but if the user doesn't want streaming, make this a memory source
					int length = source->getSize();
					char* tempbuf = (char*)CAUDIO_MALLOC(length);
					if(tempbuf)
					{
						source->read(tempbuf, length);
						IDataSource* memSource = CAUDIO_NEW cMemorySource(tempbuf, length, true);
						CAUDIO_FREE(tempbuf);

						if(memSource && memSource->isValid())
						{
							source->drop();
							return memSource;
						}

						if(memSource)
							memSource->drop();
					}
				}
				return source;
			}
		protected:
			cAudioMutex Mutex;
		private:
	};
};

#endif

#endif //! CFILESOURCEFACTORY_H