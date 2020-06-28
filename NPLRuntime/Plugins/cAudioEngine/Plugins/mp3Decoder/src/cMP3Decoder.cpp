#include "cMP3Decoder.h"
#include <stdlib.h>
#include <string.h>

#ifdef USE_MINI_MP3

#define MINIMP3_IMPLEMENTATION
#include "../minimp3/minimp3.h"

cMP3Decoder::cMP3Decoder(IDataSource* stream)
	: IAudioDecoder(stream)
	, _valid(false)
	, _dataOffset(0)
	, _numChannels(0)
	, _frequency(0)
	, _totalSize(0)
	, _currentPosition(0)
{
	if (stream && stream->isValid() && stream->getSize() > 0)
	{
		mp3dec_init(&_context);

		//Check to see if we need to skip the idv3 header
		char idv3Header[10];
		int amountRead = stream->read(idv3Header, 10);
		if (amountRead == 10 && idv3Header[0] == 'I' && idv3Header[1] == 'D' && idv3Header[2] == '3')
		{
			int versionMajor = idv3Header[3];
			int versionMinor = idv3Header[4];
			int flags = idv3Header[5];

			int size = 0;
			size = (idv3Header[6] & 0x7f) << (3 * 7);
			size |= (idv3Header[7] & 0x7f) << (2 * 7);
			size |= (idv3Header[8] & 0x7f) << (1 * 7);
			size |= (idv3Header[9] & 0x7f);

			size += 10;

			_dataOffset = size;
			stream->seek(_dataOffset, false);
		}
		else
		{
			stream->seek(0, false);
		}

		unsigned char inputBuffer[4096];
		int inputSize = stream->read(inputBuffer, 4096);


		auto samples = mp3dec_decode_frame(&_context, inputBuffer, inputSize, nullptr, &_currentPacket.info);

		if (samples == 0 && _currentPacket.info.frame_bytes == 0)
			return;

		_numChannels	= _currentPacket.info.channels;
		_frequency		= _currentPacket.info.hz;

		stream->seek(_dataOffset, false);

	
		auto curPos = _dataOffset;

		while (curPos < (unsigned int)stream->getSize())
		{
			inputSize = stream->read(inputBuffer, 4096);
			int samples = mp3dec_decode_frame(&_context, inputBuffer, inputSize, nullptr, &_currentPacket.info);

			if (samples == 0)
			{
				if (_currentPacket.info.frame_bytes == 0)
					return;
			}
			else
			{
				_currentPacket.samples = samples;
			}


			curPos += _currentPacket.info.frame_bytes;

			_totalSize += _currentPacket.samples * sizeof(short);

			stream->seek(curPos, false);
		}

		stream->seek(_dataOffset, false);

		_valid = true;
	}
}

cMP3Decoder::~cMP3Decoder()
{

}

AudioFormats cMP3Decoder::getFormat()
{
	if (_valid)
	{
		if (_numChannels == 1)
		{
			return EAF_16BIT_MONO;
		}
		else
		{
			return EAF_16BIT_STEREO;
		}
	}
	return EAF_8BIT_MONO;
}

int cMP3Decoder::getFrequency()
{
	if (_valid)
	{
		return _frequency;
	}

	return 0;
}

bool cMP3Decoder::isSeekingSupported()
{
	return true;
}

bool cMP3Decoder::isValid()
{
	return _valid;
}

int cMP3Decoder::readAudioData(void* output, int amount)
{
	if (_valid)
	{
		// Check to see if we have an empty packet and fill it
		if (_currentPacket.size <= _currentPacket.read)
		{
			//Empty packet, grab a new one.
			_currentPacket.reset();

			unsigned char inputBuffer[MINIMP3_MAX_SAMPLES_PER_FRAME * sizeof(short)];
			int oldPos = Stream->getCurrentPos();
			int inputSize = Stream->read(inputBuffer, _currentPacket.samples * sizeof(short) > sizeof(inputBuffer) ? sizeof(inputBuffer) : _currentPacket.samples * sizeof(short));

			if (!inputSize)
				return 0;

			int samples = mp3dec_decode_frame(&_context, inputBuffer, inputSize, (short*)_currentPacket.data, &_currentPacket.info);
			_currentPacket.size = samples * sizeof(short); //_currentPacket.info.frame_bytes;

			if (samples == 0)
			{
				if (_currentPacket.info.frame_bytes == 0)
					return -1;
			}
			else
			{
				_currentPacket.samples = samples;
			}

			if (_numChannels != _currentPacket.info.channels)
				return -1;

			if (_frequency != _currentPacket.info.hz)
				return -1;

			Stream->seek(oldPos + _currentPacket.info.frame_bytes, false);
		}

		//If we have data left since last time, put it in if we have space
		if (_currentPacket.size > _currentPacket.read)
		{
			int amountLeft = _currentPacket.size - _currentPacket.read;
			if (amountLeft < amount)
			{
				//Dump in all our data
				memcpy(output, _currentPacket.data + _currentPacket.read, amountLeft);
				_currentPacket.read += amountLeft;
				_currentPosition += amountLeft;
				return amountLeft;
			}
			else
			{
				//Not enough space for all our data, put in a little bit
				memcpy(output, _currentPacket.data + _currentPacket.read, amount);
				_currentPacket.read += amount;
				_currentPosition += amount;

				return amount;
			}
		}
	}

	return 0;
}

bool cMP3Decoder::setPosition(int position, bool relative)
{
	if (_valid)
	{
		position = relative ? Stream->getCurrentPos() + position : position += _dataOffset;

		//Just reload from scratch
		Stream->seek(_dataOffset, false);

		_currentPosition = 0;


		unsigned char inputBuffer[MINIMP3_MAX_SAMPLES_PER_FRAME * sizeof(short)];

		int lastFrameSize = 0;

		while (Stream->getCurrentPos() < position)
		{
			//Eat up frames till we reach about the right spot
			int oldPos = Stream->getCurrentPos();

			int inputSize = Stream->read(inputBuffer, _currentPacket.samples * sizeof(short) > sizeof(inputBuffer) ? sizeof(inputBuffer) : _currentPacket.samples * sizeof(short));

			if (inputSize == 0)
			{
				Stream->seek(_dataOffset, false);
				return false;
			}

			int samples = mp3dec_decode_frame(&_context, inputBuffer, inputSize, nullptr, &_currentPacket.info);

			//Oops, error...
			if (samples == 0)
			{
				if (_currentPacket.info.frame_bytes == 0)
				{
					Stream->seek(_dataOffset, false);
					return false;
				}
			}
			else
			{
				_currentPacket.samples = samples;
			}

			Stream->seek(oldPos + _currentPacket.info.frame_bytes, false);

			_currentPosition += _currentPacket.samples * sizeof(short);

			lastFrameSize = _currentPacket.info.frame_bytes;
		}

		Stream->seek(-lastFrameSize, true);

		return true;
	}

	return false;
}

bool cMP3Decoder::seek(float seconds, bool relative)
{
	if (_valid)
	{
		//int amountToSeek = seconds * (float)(Context->bit_rate / 8);
		// fixed by LiXizhi: the physical frame size is not Context->bit_rate / 8, but is some other constant value calculated below. 
		int amountToSeek2 = (int)(seconds / getTotalTime() * (float)(Stream->getSize() - _dataOffset));
		return setPosition(amountToSeek2, relative);
	}
	return false;
}

float cMP3Decoder::getTotalTime()
{
	return (float)_totalSize / ((float)_frequency * (float)_numChannels * 2.0f);
}

int cMP3Decoder::getTotalSize()
{
	return _totalSize;
}

int cMP3Decoder::getCompressedSize()
{
	return Stream->getSize();
}

float cMP3Decoder::getCurrentTime()
{
	return (float)_currentPosition / ((float)_frequency * (float)_numChannels * 2.0f);
}

int cMP3Decoder::getCurrentPosition()
{
	return _currentPosition;
}

int cMP3Decoder::getCurrentCompressedPosition()
{
	return Stream->getCurrentPos();
}

cAudioString cMP3Decoder::getType() const
{
	return cAudioString("cMP3Decoder");
}


#else
cMP3Decoder::cMP3Decoder(IDataSource* stream) : IAudioDecoder(stream), Context(0x0), 
		Valid(false), DataOffset(0), NumChannels(0), Frequency(0), TotalSize(0), CurrentPosition(0)
{
	if(Stream && Stream->isValid() && Stream->getSize() > 0)
	{
		CurrentPacket.data = new unsigned char[MPAUDEC_MAX_AUDIO_FRAME_SIZE];

		if(!CurrentPacket.data)
			return;

		Context = new MPAuDecContext();

		if(!Context || mpaudec_init(Context) < 0)
		{
			delete Context;
			Context = 0x0;
			return;
		}

		//Check to see if we need to skip the idv3 header
		char idv3Header[10];
		int amountRead = Stream->read(idv3Header, 10);
		if (amountRead == 10 && idv3Header[0] == 'I' && idv3Header[1] == 'D' && idv3Header[2] == '3')
		{
			int versionMajor = idv3Header[3];
			int versionMinor = idv3Header[4];
			int flags = idv3Header[5];

			int size = 0;
			size  = (idv3Header[6] & 0x7f) << (3*7);
			size |= (idv3Header[7] & 0x7f) << (2*7);
			size |= (idv3Header[8] & 0x7f) << (1*7);
			size |= (idv3Header[9] & 0x7f) ;

			size += 10;

			DataOffset = size;
			Stream->seek(DataOffset, false);
		}
		else
		{
			Stream->seek(0, false);
		}

		//Now, read the header for our information
		Context->parse_only = 1;
		unsigned char tempBuffer[MPAUDEC_MAX_AUDIO_FRAME_SIZE];
		int outputSize = 0;
		unsigned char inputBuffer[4096];
		int inputSize = Stream->read(inputBuffer, 4096);
		int rv = mpaudec_decode_frame( Context, tempBuffer, &outputSize, inputBuffer, inputSize);
		
		//Oops, error...
		if(rv < 0)
			return;

		NumChannels = Context->channels;
		Frequency = Context->sample_rate;

		Context->parse_only = 0;
		Stream->seek(DataOffset, false);

		//Now, calculate our max size
		Context->parse_only = 1;
		int curPos = DataOffset;
		while(curPos < Stream->getSize())
		{
			outputSize = 0;
			inputSize = Stream->read(inputBuffer, 4096);
			int rv = mpaudec_decode_frame( Context, tempBuffer, &outputSize, inputBuffer, inputSize);
			
			if(rv < 0)
				return;
			
			curPos += rv;
			TotalSize += Context->frame_size;
			Stream->seek(curPos, false);
		}
		Context->parse_only = 0;

		Stream->seek(DataOffset, false);

		//This is a hack, for some reason on every mp3 I tested, Total Size was roughly 4 times less than the real value.
		TotalSize *= 4;

		Valid = true;
	}
}

cMP3Decoder::~cMP3Decoder()
{
	if (Context)
	{
		mpaudec_clear(Context);
		delete Context;
	}

	if(CurrentPacket.data)
		delete CurrentPacket.data;
}

AudioFormats cMP3Decoder::getFormat()
{
	if(Valid)
	{
		if(NumChannels == 1)
		{
			return EAF_16BIT_MONO;
		}
		else
		{
			return EAF_16BIT_STEREO;
		}
	}
	return EAF_8BIT_MONO;
}

int cMP3Decoder::getFrequency()
{
	if(Valid)
	{
		return Frequency;
	}

	return 0;
}

bool cMP3Decoder::isSeekingSupported()
{
	return true;
}

bool cMP3Decoder::isValid()
{
	return Valid;
}

int cMP3Decoder::readAudioData(void* output, int amount)
{
	if(Valid)
	{
		//Check to see if we have an empty packet and fill it
		if(CurrentPacket.size <= CurrentPacket.read)
		{
			//Empty packet, grab a new one.
			CurrentPacket.reset();
			memset(CurrentPacket.data, 0, MPAUDEC_MAX_AUDIO_FRAME_SIZE);

			unsigned char inputBuffer[MPAUDEC_MAX_AUDIO_FRAME_SIZE];
			int oldPos = Stream->getCurrentPos();
			int inputSize = Stream->read(inputBuffer, Context->frame_size > MPAUDEC_MAX_AUDIO_FRAME_SIZE ? MPAUDEC_MAX_AUDIO_FRAME_SIZE : Context->frame_size);

			if(inputSize == 0)
				return 0;

			int rv = mpaudec_decode_frame( Context, CurrentPacket.data, &CurrentPacket.size, inputBuffer, inputSize);
			
			//Oops, error...
			if(rv < 0)
				return -1;

			//OpenAL really hates it if you try to change format in the middle
			if(NumChannels != Context->channels)
				return -1;

			if(Frequency != Context->sample_rate)
				return -1;

			Stream->seek(oldPos + rv, false);
		}

		//If we have data left since last time, put it in if we have space
		if( CurrentPacket.size > CurrentPacket.read )
		{
			int amountLeft = CurrentPacket.size - CurrentPacket.read;
			if(amountLeft < amount)
			{
				//Dump in all our data
				memcpy(output, CurrentPacket.data + CurrentPacket.read, amountLeft);
				CurrentPacket.read += amountLeft;
				CurrentPosition += amountLeft;
				return amountLeft;
			}
			else
			{
				//Not enough space for all our data, put in a little bit
				memcpy(output, CurrentPacket.data + CurrentPacket.read, amount);
				CurrentPacket.read += amount;
				CurrentPosition += amount;
				return amount;
			}
		}
	}

	return 0;
}

bool cMP3Decoder::setPosition(int position, bool relative)
{
	if(Valid)
	{
		if(relative)
			position = Stream->getCurrentPos() + position;
		else
		{
			// Fixed seeking by LiXizhi, 
			position += DataOffset;
		}

		//Just reload from scratch
		Stream->seek(DataOffset, false);

		MPAuDecContext oldContext = *Context;

		mpaudec_clear(Context);
		mpaudec_init(Context);

		Context->bit_rate = oldContext.bit_rate;
		Context->channels = oldContext.channels;
		Context->frame_size = oldContext.frame_size;
		Context->sample_rate = oldContext.sample_rate;

		CurrentPosition = 0;

		Context->parse_only = 1;
		int lastFrameSize = 0;
		while(Stream->getCurrentPos() < position)
		{
			//Eat up frames till we reach about the right spot
			unsigned char inputBuffer[MPAUDEC_MAX_AUDIO_FRAME_SIZE];
			int oldPos = Stream->getCurrentPos();
			int inputSize = Stream->read(inputBuffer, Context->frame_size > MPAUDEC_MAX_AUDIO_FRAME_SIZE ? MPAUDEC_MAX_AUDIO_FRAME_SIZE : Context->frame_size);

			if(inputSize == 0)
			{
				Stream->seek(DataOffset, false);
				return false;
			}

			unsigned char tempBuffer[MPAUDEC_MAX_AUDIO_FRAME_SIZE];
			int outputSize = 0;

			int rv = mpaudec_decode_frame( Context, tempBuffer, &outputSize, inputBuffer, inputSize);
			
			//Oops, error...
			if(rv < 0)
			{
				Stream->seek(DataOffset, false);
				return false;
			}

			Stream->seek(oldPos + rv, false);

			CurrentPosition += Context->frame_size;
			lastFrameSize = rv;
		}
		Context->parse_only = 0;

		Stream->seek(-lastFrameSize, true);

		return true;
	}
	return false;
}

bool cMP3Decoder::seek(float seconds, bool relative)
{	
	if(Valid)
	{
		//int amountToSeek = seconds * (float)(Context->bit_rate / 8);
		// fixed by LiXizhi: the physical frame size is not Context->bit_rate / 8, but is some other constant value calculated below. 
		int amountToSeek2 = (int)(seconds / getTotalTime() * (float)(Stream->getSize() - DataOffset));
        return setPosition(amountToSeek2, relative);
	}
	return false;
}

float cMP3Decoder::getTotalTime()
{
	return (float)TotalSize / ((float)Frequency * (float)NumChannels * 2.0f);
}

int cMP3Decoder::getTotalSize()
{
	return TotalSize;
}

int cMP3Decoder::getCompressedSize()
{
	return Stream->getSize();
}

float cMP3Decoder::getCurrentTime()
{
	return (float)CurrentPosition / ((float)Frequency * (float)NumChannels * 2.0f);
}

int cMP3Decoder::getCurrentPosition()
{
	return CurrentPosition;
}

int cMP3Decoder::getCurrentCompressedPosition()
{
	return Stream->getCurrentPos();
}

cAudioString cMP3Decoder::getType() const
{
	return cAudioString("cMP3Decoder");
}

#endif // USE_MINI_MP3