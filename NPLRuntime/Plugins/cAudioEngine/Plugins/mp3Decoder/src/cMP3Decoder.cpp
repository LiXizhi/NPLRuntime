#ifdef USE_MINI_MP3
#define MINIMP3_IMPLEMENTATION
#endif
#include "cMP3Decoder.h"
#include <stdlib.h>
#include <string.h>

#ifdef USE_MINI_MP3
#include <algorithm>
#include <climits>
#include <cmath>

cMP3Decoder::cMP3Decoder(IDataSource* stream) : IAudioDecoder(stream)
{
	if (!stream || !stream->isValid() || stream->getSize() <= 0)
		return;

	io.read_data = io.seek_data = stream;
	io.read = [](void* buffer, size_t size, void* data) -> size_t {
		return static_cast<IDataSource*>(data)->read(buffer, static_cast<int>(size));
	};
	io.seek = [](uint64_t pos, void* data) -> int {
		return pos <= INT_MAX && static_cast<IDataSource*>(data)->seek(static_cast<int>(pos), false) ? 0 : -1;
	};
	valid = mp3dec_ex_open_cb(&decoder, &io, MP3D_SEEK_TO_SAMPLE) == 0 &&
		decoder.info.hz > 0 && (decoder.info.channels == 1 || decoder.info.channels == 2) && decoder.samples > 0;
}

cMP3Decoder::~cMP3Decoder()
{
	mp3dec_ex_close(&decoder);
}

AudioFormats cMP3Decoder::getFormat()
{
	return decoder.info.channels == 1 ? EAF_16BIT_MONO : EAF_16BIT_STEREO;
}

int cMP3Decoder::getFrequency()
{
	return decoder.info.hz;
}

bool cMP3Decoder::isSeekingSupported()
{
	return valid;
}

bool cMP3Decoder::isValid()
{
	return valid;
}

int cMP3Decoder::readAudioData(void* output, int amount)
{
	if (!valid || amount < 2)
		return 0;
	const size_t samples = mp3dec_ex_read(&decoder, static_cast<mp3d_sample_t*>(output), amount / 2);
	if (!samples && decoder.last_error)
		return -1;
	return static_cast<int>(samples * 2);
}

bool cMP3Decoder::setPosition(int position, bool relative)
{
	if (!valid)
		return false;
	// This legacy method addresses compressed bytes. Source stop and loop use byte zero.
	int64_t offset = relative ? int64_t(getCurrentCompressedPosition()) + position : position;
	if (offset <= 0)
		return mp3dec_ex_seek(&decoder, 0) == 0;
	if (offset >= Stream->getSize())
		return mp3dec_ex_seek(&decoder, decoder.samples) == 0;
	// A tagged stream can report its duration before minimp3 has built a seek index.
	if (!decoder.indexes_built && mp3dec_ex_seek(&decoder, 1) != 0)
		return false;
	if (!decoder.index.num_frames)
		return false;
	auto* first = decoder.index.frames;
	auto* end = first + decoder.index.num_frames;
	auto* frame = std::upper_bound(first, end, static_cast<uint64_t>(offset),
		[](uint64_t value, const mp3dec_frame_t& entry) { return value < entry.offset; });
	if (frame != first)
		--frame;
	const auto delay = static_cast<uint64_t>(decoder.start_delay);
	return mp3dec_ex_seek(&decoder, frame->sample > delay ? frame->sample - delay : 0) == 0;
}

bool cMP3Decoder::seek(float seconds, bool relative)
{
	if (!valid || !std::isfinite(seconds))
		return false;
	double frame = std::round(double(seconds) * decoder.info.hz);
	if (relative)
		frame += decoder.cur_sample / decoder.info.channels;
	const double sample = (std::max)(0.0, (std::min)(frame * decoder.info.channels, double(decoder.samples)));
	return mp3dec_ex_seek(&decoder, static_cast<uint64_t>(sample)) == 0;
}

float cMP3Decoder::getTotalTime()
{
	return valid ? float(double(decoder.samples) / (decoder.info.channels * decoder.info.hz)) : 0.f;
}

int cMP3Decoder::getTotalSize()
{
	return static_cast<int>((std::min)(decoder.samples * 2, uint64_t(INT_MAX)));
}

int cMP3Decoder::getCompressedSize()
{
	return Stream->getSize();
}

float cMP3Decoder::getCurrentTime()
{
	return valid ? float(double(decoder.cur_sample) / (decoder.info.channels * decoder.info.hz)) : 0.f;
}

int cMP3Decoder::getCurrentPosition()
{
	return static_cast<int>((std::min)(decoder.cur_sample * 2, uint64_t(INT_MAX)));
}

int cMP3Decoder::getCurrentCompressedPosition()
{
	return static_cast<int>(decoder.offset);
}

cAudioString cMP3Decoder::getType() const
{
	return cAudioString(fromUTF8("cMP3Decoder"));
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

		CurrentPacket.reset();
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