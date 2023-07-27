//-----------------------------------------------------------------------------
// Class:	ParaEngine Audio Engine Wrapper for audio capture interface in caudio engine
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2021.10.2
//-----------------------------------------------------------------------------
#include "PluginAPI.h"
#include <time.h>
#include <memory>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <vorbis/vorbisenc.h>
#include "ParaAudioCapture.h"

using namespace ParaEngine;
using namespace cAudio;

CParaAudioCapture::CParaAudioCapture() : m_pAudioCapture(NULL), m_bIsCapturing(false)
{
	m_pAudioCapture = cAudio::createAudioCapture(false);
	m_deviceList = cAudio::createAudioDeviceList(DT_RECORDING);
	bool bSuccess = false;

	// some (android) devices does not support default frequency, and stereo types, so we will iterate over all possible combinations, starting from the default one. 
	unsigned int frequencies[] = { 16000, 22050, 48000, 44100, 32000, 8000 };
	ParaAudioFormats formats[] = { EAF_16BIT_MONO, EAF_16BIT_STEREO, EAF_8BIT_MONO, EAF_8BIT_STEREO };
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			auto frequency = frequencies[i];
			auto format = formats[j];
			if (initialize(NULL, frequency, format))
			{
				return;
			}
		}
	}
}

CParaAudioCapture::~CParaAudioCapture()
{
	if (m_pAudioCapture)
	{
		if (m_bIsCapturing)
			stopCapture();
		shutdown();
		cAudio::destroyAudioCapture(m_pAudioCapture);
		m_pAudioCapture = NULL;
	}
}


bool CParaAudioCapture::initialize(const char* deviceName /*= 0x0*/, unsigned int frequency /*= 22050*/, ParaAudioFormats format /*= EAF_16BIT_MONO*/, unsigned int internalBufferSize /*= 8192*/)
{
	if (m_pAudioCapture)
	{
		if (deviceName == 0)
			deviceName = "";
		return m_pAudioCapture->initialize(deviceName, frequency, (AudioFormats)format, internalBufferSize);
	}
	return false;
}

bool CParaAudioCapture::isReady()
{
	if (m_pAudioCapture)
		return m_pAudioCapture->isReady();
	return false;
}

void CParaAudioCapture::updateCaptureBuffer(bool force /*= false*/)
{
	if (m_pAudioCapture)
		m_pAudioCapture->updateCaptureBuffer(force);
}

void CParaAudioCapture::shutdown()
{
	if (m_pAudioCapture)
		m_pAudioCapture->shutdown();
}

bool CParaAudioCapture::isUpdateThreadRunning()
{
	if (m_pAudioCapture)
		return m_pAudioCapture->isUpdateThreadRunning();
	return false;
}

const char* CParaAudioCapture::getDeviceName()
{
	if (m_pAudioCapture)
		return m_pAudioCapture->getDeviceName();
	return "";
}

unsigned int CParaAudioCapture::getFrequency()
{
	if (m_pAudioCapture)
		return m_pAudioCapture->getFrequency();
	return 0;
}

ParaAudioFormats CParaAudioCapture::getFormat()
{
	if (m_pAudioCapture)
		return (ParaAudioFormats)m_pAudioCapture->getFormat();
	return ParaAudioFormats::EAF_8BIT_MONO;
}

unsigned int CParaAudioCapture::getInternalBufferSize()
{
	if (m_pAudioCapture)
		return m_pAudioCapture->getInternalBufferSize();
	return 0;
}

unsigned int CParaAudioCapture::getSampleSize()
{
	if (m_pAudioCapture)
		return m_pAudioCapture->getSampleSize();
	return 0;
}

bool CParaAudioCapture::setDevice(const char* deviceName)
{
	if (m_pAudioCapture)
		return m_pAudioCapture->setDevice(deviceName);
	return false;
}

bool CParaAudioCapture::setFrequency(unsigned int frequency)
{
	if (m_pAudioCapture)
		return m_pAudioCapture->setFrequency(frequency);
	return false;
}

bool CParaAudioCapture::setFormat(ParaAudioFormats format)
{
	if (m_pAudioCapture)
		return m_pAudioCapture->setFormat((AudioFormats)format);
	return false;
}

bool CParaAudioCapture::setInternalBufferSize(unsigned int internalBufferSize)
{
	if (m_pAudioCapture)
		return m_pAudioCapture->setInternalBufferSize(internalBufferSize);
	return false;
}

bool CParaAudioCapture::beginCapture()
{
	if (m_pAudioCapture)
	{
		if (m_bIsCapturing)
			stopCapture();
		m_bIsCapturing = true;
		return m_pAudioCapture->beginCapture();
	}
	return false;
}

void CParaAudioCapture::stopCapture()
{
	if (m_pAudioCapture && m_bIsCapturing)
	{
		m_bIsCapturing = false;
		m_pAudioCapture->stopCapture();
	}
}

unsigned int CParaAudioCapture::getCapturedAudio(void* outputBuffer, unsigned int outputBufferSize)
{
	if (m_pAudioCapture)
		return m_pAudioCapture->getCapturedAudio(outputBuffer, outputBufferSize);
	return 0;
}

ParaAudioCaptureBuffer* CParaAudioCapture::getCapturedAudioBuffer()
{
	if (m_pAudioCapture)
	{
		AudioCaptureBuffer* pBuffer = m_pAudioCapture->getCapturedAudioBuffer();
		if (pBuffer)
		{
			auto pBuffer2 = new ParaAudioCaptureBuffer(pBuffer->getLength());
			memcpy(pBuffer2->getWriteBuffer(), pBuffer->getReadBuffer(), pBuffer2->getLength());
			SAFE_DELETE(pBuffer);
			return pBuffer2;
		}
	}
	return NULL;
}

unsigned int CParaAudioCapture::getCurrentCapturedAudioSize()
{
	if (m_pAudioCapture)
		return m_pAudioCapture->getCurrentCapturedAudioSize();
	return 0;
}

void ParaEngine::CParaAudioCapture::Release()
{
	delete this;
}

const char* ParaEngine::CParaAudioCapture::getAvailableDeviceName(unsigned int index)
{
	auto ret = m_deviceList->getDeviceName(index).c_str();
	return toUTF8(ret);
}

unsigned int ParaEngine::CParaAudioCapture::getAvailableDeviceCount()
{
	return m_deviceList->getDeviceCount();
}

const char* ParaEngine::CParaAudioCapture::getDefaultDeviceName()
{
	return toUTF8(m_deviceList->getDefaultDeviceName().c_str());
}

static void fwrite16le(int32 val, FILE* f)
{
	byte data[2];
	data[0] = (byte)(val & 0xff);
	data[1] = (byte)(val >> 8);
	fwrite(data, 1, 2, f);
}

static void fwrite32le(int32 val, FILE* f)
{
	byte data[4];
	data[0] = (byte)(val & 0xff);
	data[1] = (byte)((val >> 8) & 0xff);
	data[2] = (byte)((val >> 16) & 0xff);
	data[3] = (byte)(val >> 24);
	fwrite(data, 1, 4, f);
}

/** for WAV encoding, see alrecord.c in OpenAL Recording Example
* for ogg encoding, see examples in https://gitlab.xiph.org/xiph/vorbis/-/blob/v1.3.2/examples/encoder_example.c
*/
unsigned int ParaEngine::CParaAudioCapture::saveToFile(const char* filename, float baseQuality)
{
	stopCapture();
	if (!m_pAudioCapture || getCurrentCapturedAudioSize() == 0)
		return 0;

	{
		// print all recording devices
		int nCount = getAvailableDeviceCount();
		getLogger()->logInfo("CParaAudioCapture", "default recording device: %s", getDefaultDeviceName());
		for (int i = 0; i < nCount; ++i)
		{
			getLogger()->logInfo("CParaAudioCapture", "%d: %s", i, getAvailableDeviceName(i));
		}
	}
	std::unique_ptr<ParaAudioCaptureBuffer> pBuffer(getCapturedAudioBuffer());
	auto format = getFormat();
	int nFrequency = getFrequency();
	int nChannels = ((format == EAF_8BIT_MONO) || (format == EAF_16BIT_MONO)) ? 1 : 2;
	int nFileSize = 0;

	std::string sFilename = filename;
	std::string sFilenameExt;
	if (sFilename.length() > 4)
		sFilenameExt = sFilename.substr(sFilename.length() - 4, 4);

	// if file extension is "wav"
	if (sFilenameExt == ".wav")
	{
		int mBits = (format == EAF_8BIT_MONO || format == EAF_8BIT_STEREO) ? 8 : 16;
		int mFrameSize = getSampleSize();
		int mSampleRate = nFrequency;
		// wav file
		FILE* pFile = cfopen(fromUTF8(filename).c_str(), fwriteMode);
		if (!pFile)
		{
			getLogger()->logWarning("ParaAudioCapture", "can not open %s file for writing", filename);
			return 0;
		}

		fputs("RIFF", pFile);
		fwrite32le(0xFFFFFFFF, pFile); // 'RIFF' header len; filled in at close

		fputs("WAVE", pFile);

		fputs("fmt ", pFile);
		fwrite32le(16, pFile); // 'fmt ' header len

		// 16-bit val, format type id (1 = integer PCM, 3 = float PCM)
		fwrite16le((mBits == 32) ? 0x0003 : 0x0001, pFile);
		// 16-bit val, channel count
		fwrite16le(nChannels, pFile);
		// 32-bit val, frequency
		fwrite32le(mSampleRate, pFile);
		// 32-bit val, bytes per second
		fwrite32le(mSampleRate * mFrameSize, pFile);
		// 16-bit val, frame size
		fwrite16le(mFrameSize, pFile);
		// 16-bit val, bits per sample
		fwrite16le(mBits, pFile);

		fputs("data", pFile);
		const char* pData = pBuffer->getReadBuffer();
		int nDataLength = pBuffer->getLength();

		fwrite32le(nDataLength, pFile);
		fwrite(pData, 1, nDataLength, pFile);

		nFileSize = ftell(pFile);
		if (fseek(pFile, 4, SEEK_SET) == 0)
			fwrite32le(nFileSize - 8, pFile);
		fclose(pFile);
	}
	else if (sFilenameExt == ".ogg")
	{
		// ogg file
		// 0.1 for low, 1 for high. 
		float nOggQuality = baseQuality;

		/* takes a stereo 16bit 44.1kHz WAV file from stdin and encodes it into a Vorbis bitstream */

		ogg_stream_state os; /* take physical pages, weld into a logical
							 stream of packets */
		ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
		ogg_packet       op; /* one raw packet of data for decode */

		vorbis_info      vi; /* struct that stores all the static vorbis bitstream
							 settings */
		vorbis_comment   vc; /* struct that stores all the user comments */

		vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
		vorbis_block     vb; /* local working space for packet->PCM decode */

		int eos = 0, ret;

		vorbis_info_init(&vi);

		/* choose an encoding mode.  A few possibilities commented out, one
		actually used: */

		/*********************************************************************
		Encoding using a VBR quality mode.  The usable range is -.1
		(lowest quality, smallest file) to 1. (highest quality, largest file).
		Example quality mode .4: 44kHz stereo coupled, roughly 128kbps VBR

		ret = vorbis_encode_init_vbr(&vi,2,44100,.4);

		---------------------------------------------------------------------

		Encoding using an average bitrate mode (ABR).
		example: 44kHz stereo coupled, average 128kbps VBR

		ret = vorbis_encode_init(&vi,2,44100,-1,128000,-1);

		---------------------------------------------------------------------

		Encode using a quality mode, but select that quality mode by asking for
		an approximate bitrate.  This is not ABR, it is true VBR, but selected
		using the bitrate interface, and then turning bitrate management off:

		ret = ( vorbis_encode_setup_managed(&vi,2,44100,-1,128000,-1) ||
		vorbis_encode_ctl(&vi,OV_ECTL_RATEMANAGE2_SET,NULL) ||
		vorbis_encode_setup_init(&vi));

		*********************************************************************/

		ret = vorbis_encode_init_vbr(&vi, nChannels, nFrequency, nOggQuality);

		/* do not continue if setup failed; this can happen if we ask for a
		mode that libVorbis does not support (eg, too low a bitrate, etc,
		will return 'OV_EIMPL') */

		if (ret) return 0;

		/* add a comment */
		vorbis_comment_init(&vc);
		vorbis_comment_add_tag(&vc, "ENCODER", "NPL/ParaEngine ParaAudioCapture");

		/* set up the analysis state and auxiliary encoding storage */
		vorbis_analysis_init(&vd, &vi);
		vorbis_block_init(&vd, &vb);

		/* set up our packet->stream encoder */
		/* pick a random serial number; that way we can more likely build
		chained streams just by concatenation */
		srand(time(NULL));
		ogg_stream_init(&os, rand());


		FILE* pFile = cfopen(fromUTF8(filename).c_str(), fwriteMode);
		if (!pFile)
		{
			getLogger()->logWarning("ParaAudioCapture", "can not open %s file for writing", filename);
			return 0;
		}


		/* Vorbis streams begin with three headers; the initial header (with
		most of the codec setup parameters) which is mandated by the Ogg
		bitstream spec.  The second header holds any comment fields.  The
		third header holds the bitstream codebook.  We merely need to
		make the headers, then pass them to libvorbis one at a time;
		libvorbis handles the additional Ogg bitstream constraints */

		{
			ogg_packet header;
			ogg_packet header_comm;
			ogg_packet header_code;

			vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
			ogg_stream_packetin(&os, &header); /* automatically placed in its own
											   page */
			ogg_stream_packetin(&os, &header_comm);
			ogg_stream_packetin(&os, &header_code);

			/* This ensures the actual
			* audio data will start on a new page, as per spec
			*/
			while (!eos) {
				int result = ogg_stream_flush(&os, &og);
				if (result == 0)break;
				fwrite(og.header, 1, og.header_len, pFile);
				fwrite(og.body, 1, og.body_len, pFile);
				nFileSize += og.header_len + og.body_len;
			}

		}
		const char* remainingBuffer = pBuffer->getReadBuffer();
		long remainingBytes = pBuffer->getLength();
		const long READ = 1024;

		while (!eos) {
			long i;

			// each time, we will process at most READ number of audio frame data
			long bytes = READ * (2 * nChannels);
			const char* readbuffer = remainingBuffer;
			if (remainingBytes < bytes)
				bytes = remainingBytes;
			remainingBuffer += bytes;
			remainingBytes -= bytes;

			if (bytes == 0) {
				/* end of file.  this can be done implicitly in the mainline,
				but it's easier to see here in non-clever fashion.
				Tell the library we're at end of stream so that it can handle
				the last frame and mark end of stream in the output properly */
				vorbis_analysis_wrote(&vd, 0);
			}
			else {
				/* data to encode */

				/* expose the buffer to submit data */
				float** buffer = vorbis_analysis_buffer(&vd, READ);

				if (nChannels == 1)
				{
					long nFrameCount = bytes / 2;
					for (i = 0; i < nFrameCount; i++) {
						buffer[0][i] = ((readbuffer[i * 2 + 1] << 8) |
							(0x00ff & (int)readbuffer[i * 2])) / 32768.f;
					}
				}
				else // if (nChannels == 2)
				{
					long nFrameCount = bytes / 4;
					/* un-interleaved samples */
					for (i = 0; i < nFrameCount; i++) {
						buffer[0][i] = ((readbuffer[i * 4 + 1] << 8) |
							(0x00ff & (int)readbuffer[i * 4])) / 32768.f;
						buffer[1][i] = ((readbuffer[i * 4 + 3] << 8) |
							(0x00ff & (int)readbuffer[i * 4 + 2])) / 32768.f;
					}
				}

				/* tell the library how much we actually submitted */
				vorbis_analysis_wrote(&vd, i);
			}

			/* vorbis does some data pre-analysis, then divides up blocks for
			more involved (potentially parallel) processing.  Get a single
			block for encoding now */
			while (vorbis_analysis_blockout(&vd, &vb) == 1) {

				/* analysis, assume we want to use bit rate management */
				vorbis_analysis(&vb, NULL);
				vorbis_bitrate_addblock(&vb);

				while (vorbis_bitrate_flushpacket(&vd, &op)) {

					/* weld the packet into the bitstream */
					ogg_stream_packetin(&os, &op);

					/* write out pages (if any) */
					while (!eos) {
						int result = ogg_stream_pageout(&os, &og);
						if (result == 0)break;
						fwrite(og.header, 1, og.header_len, pFile);
						fwrite(og.body, 1, og.body_len, pFile);
						nFileSize += og.header_len + og.body_len;

						/* this could be set above, but for illustrative purposes, I do
						it here (to show that vorbis does know where the stream ends) */

						if (ogg_page_eos(&og))eos = 1;
					}
				}
			}
		}

		/* clean up and exit.  vorbis_info_clear() must be called last */

		ogg_stream_clear(&os);
		vorbis_block_clear(&vb);
		vorbis_dsp_clear(&vd);
		vorbis_comment_clear(&vc);
		vorbis_info_clear(&vi);

		fclose(pFile);
	}
	else
	{
		getLogger()->logError("CParaAudioCapture", "unknown file extension: %s", filename);
	}

	return nFileSize;
}

