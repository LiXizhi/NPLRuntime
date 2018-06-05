#include "cMP3Decoder.h"
#include <stdlib.h>
#include <string.h>

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

