// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IRefCounted.h"

namespace cAudio
{
	//! Interface for data providers in cAudio.
	class IDataSource : public IRefCounted
	{
		public:
			IDataSource() {  }
			virtual ~IDataSource() {  }

			//! Returns whether the source is valid.
			/**
			\return Returns false in case of an error, like the file couldn't be found. */
			virtual bool isValid() = 0;

			//! Returns the current location in the data stream.
			virtual int getCurrentPos() = 0;

			//! Returns the total size of the data stream.
			virtual int getSize() = 0;

			//! Reads out a section of the data stream.
			/** 
			\param output: Pointer to a location to put the read data.
			\param size: Size in bytes of the data to read.
			\return Number of bytes actually read.  0 bytes may indicate end of file or stream. */
			virtual int read(void* output, int size) = 0;

			//! Seek to a position in the data stream.
			/**
			\param amount: Amount in bytes to seek to.
			\param relative: If true the number of bytes in amount is relative to the current position in the stream.
			\return True if seek was successful, false if not. */
			virtual bool seek(int amount, bool relative) = 0;
	};
};

