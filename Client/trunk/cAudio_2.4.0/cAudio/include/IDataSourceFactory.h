// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IDataSource.h"

namespace cAudio
{
	//! Interface for creating data sources for use with the engine.
	class IDataSourceFactory
	{
		public:
			IDataSourceFactory() { }
			virtual ~IDataSourceFactory() { }

			//! Creates a data source instance for use with the engine.
			/**
			\param filename: Filename of the file to get a stream for. 
			\param streamingRequested: True if the user requested streaming capabilities from the data source.
			\return A pointer to a data source instance or NULL on failure to allocate. */
			virtual IDataSource* CreateDataSource(const char* filename, bool streamingRequested) = 0;
		protected:
		private:
	};
};

