#ifndef CMP3DECODER_H
#define CMP3DECODER_H

#include "IAudioDecoder.h"
#include "../mpaudec/mpaudec.h"

using namespace cAudio;

class cMP3Packet
{
public:
	cMP3Packet()
	{
		data = 0x0;
		reset();
	}

	void reset()
	{
		size = 0;
		read = 0;
	}
	int size;
	int read;
	unsigned char* data;
};

class cMP3Decoder : public IAudioDecoder
{
	public:
		cMP3Decoder(IDataSource* stream);
		~cMP3Decoder();

		//!Returns the format of the audio data
		virtual AudioFormats getFormat();

		//!Returns the frequency of the audio data
		virtual int getFrequency();

		//!Returns whether seeking is supported
		virtual bool isSeekingSupported();

		//!Returns whether the stream is valid for this codec
		virtual bool isValid();

		//!Reads a section of data out of the audio stream
		virtual int readAudioData(void* output, int amount);

		//!Sets the position to read data out of
		virtual bool setPosition(int position, bool relative);

		//!If seeking is supported, will seek the stream to seconds
		virtual bool seek(float seconds, bool relative);

		virtual float getTotalTime();
		virtual int getTotalSize();
		virtual int getCompressedSize();

		virtual float getCurrentTime();
		virtual int getCurrentPosition();
		virtual int getCurrentCompressedPosition();
		cAudioString getType() const;

	protected:
		MPAuDecContext* Context;
		bool Valid;
		unsigned int DataOffset;
		int NumChannels;
		int Frequency;

		int TotalSize;
		int CurrentPosition;

		cMP3Packet CurrentPacket;
};

#endif //! CMP3DECODER_H