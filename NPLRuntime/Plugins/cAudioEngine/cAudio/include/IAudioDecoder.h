// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IRefCounted.h"
#include "IDataSource.h"
#include "EAudioFormats.h"
#include "cAudioString.h"

namespace cAudio
{
	//! Interface for all Audio Decoders in cAudio
	class IAudioDecoder : public IRefCounted
	{
		public:
			//! Default Constructor
			/** \param stream: Data Source to attach to this decoder. */
			IAudioDecoder(IDataSource* stream) : Stream(stream) { if(Stream) Stream->grab(); }
			virtual ~IAudioDecoder() { if(Stream) Stream->drop(); }

			//! Returns the format of the audio data
			virtual AudioFormats getFormat() = 0;

			//! Returns the frequency (sample rate) of the audio data
			virtual int getFrequency() = 0;

			//! Returns whether seeking is supported
			virtual bool isSeekingSupported() = 0;

			//! Returns whether the stream is valid for this codec
			virtual bool isValid() = 0;

			//! Reads a section of data out of the audio stream
			/** 
			\param output: Pointer to the buffer to put the decoded audio.
			\param amount: Amount of data in bytes to ask the decoder to output.
			\return Number of bytes of audio data actually output. */
			virtual int readAudioData(void* output, int amount) = 0;

			//! Sets the position in the stream to read from
			/** 
			\param position: Position to seek to.
			\param relative: Whether the number in position is relative to the current position.
			\return True on success, False on failure. */
			virtual bool setPosition(int position, bool relative) = 0;

			//! If seeking is supported, will seek the stream to seconds
			/** 
			\param seconds: Time in seconds to seek.
			\param relative: Whether the number in position is relative to the current position.
			\return True on success, False on failure. */
			virtual bool seek(float seconds, bool relative) = 0;

			//! If seeking is supported, will return the length of the audio steam in seconds.  Returns a negative number if the total time cannot be determined.
			virtual float getTotalTime() = 0;

			//! If available, returns the total decoded size of the audio stream.  Returns a negative number if this cannot be determined.
			virtual int getTotalSize() = 0;

			//! Returns the compressed (original) size of the audio stream, before decoding.
			virtual int getCompressedSize() = 0;

			//! If seeking is supported, will return the current position in the stream in seconds.  Returns a negative number if the current time cannot be determined.
			virtual float getCurrentTime() = 0;

			//! If available, returns the current position in the decoded audio stream in bytes.  Returns a negative number if this cannot be determined.
			virtual int getCurrentPosition() = 0;

			//! Returns the position in the compressed (original) audio stream before decoding.
			virtual int getCurrentCompressedPosition() = 0;

			//! Returns the IAudioDecoderType.
			virtual cAudioString getType() const = 0;

		protected:
			//! Pointer to the data source to take audio data from.
			IDataSource* Stream;
	};
};
