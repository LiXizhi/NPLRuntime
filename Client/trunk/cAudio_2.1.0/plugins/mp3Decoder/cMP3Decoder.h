#ifndef CMP3DECODER_H
#define CMP3DECODER_H

#include "../../include/IAudioDecoder.h"

#define USE_MINI_MP3

#ifdef USE_MINI_MP3
	#include "minimp3/minimp3.h"
#else
	#include "mpaudec/mpaudec.h"
#endif

using namespace cAudio;
#ifdef USE_MINI_MP3
class cMP3Decoder : public IAudioDecoder
{
public:
	cMP3Decoder(IDataSource* stream);
	virtual ~cMP3Decoder();

	//!Returns the format of the audio data
	virtual AudioFormats getFormat() override;

	//!Returns the frequency of the audio data
	virtual int getFrequency() override;

	//!Returns whether seeking is supported
	virtual bool isSeekingSupported() override;

	//!Returns whether the stream is valid for this codec
	virtual bool isValid() override;

	//!Reads a section of data out of the audio stream
	virtual int readAudioData(void* output, int amount) override;

	//!Sets the position to read data out of
	virtual bool setPosition(int position, bool relative) override;

	//!If seeking is supported, will seek the stream to seconds
	virtual bool seek(float seconds, bool relative) override;

	virtual float getTotalTime() override;
	virtual int getTotalSize() override;
	virtual int getCompressedSize() override;

	virtual float getCurrentTime() override;
	virtual int getCurrentPosition() override;
	virtual int getCurrentCompressedPosition() ;

protected:
	class cMP3Packet
	{
	public:
		cMP3Packet()
		{
			reset();
		}

		void reset()
		{
			size = 0;
			read = 0;
		}
		int size;
		int read;

		int samples;

		mp3dec_frame_info_t info;
		unsigned char data[MINIMP3_MAX_SAMPLES_PER_FRAME * sizeof(short)];
	};

	cMP3Packet		_currentPacket;


	mp3dec_t		_context;
	bool			_valid;
	unsigned int	_dataOffset;

	int				_numChannels;
	int				_frequency;
	int				_totalSize;
	int				_currentPosition;
};

#else

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

#endif // #if USE_MINI_MP3

#endif //! CMP3DECODER_H