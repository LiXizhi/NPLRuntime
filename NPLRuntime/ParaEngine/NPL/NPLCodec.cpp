//-----------------------------------------------------------------------------
// Class:	NPLCodec
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.11.29
// Desc:
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLCommon.h"
#include "NPLCodec.h"
#include "zlib.h"

using namespace NPL;

namespace NPL
{
	/** the private is usually NPL_PRIVATE_KEY_SIZE+1 long, and should contain \0 at some position but not too close to the beginning.*/
	unsigned char NPLCodec::m_sPrivateKey[] = "\
NPLCodec Secret Private Key.ParaEngine All Rights Reserved.V001\0";
}

/**
The current custom algorithm is as follow
1. shift the private key by nPublicKey mod private key size. 
2. for each src char, computer the dest char as XOR of src and private key
3. increase src, dest, and private key by 1 unsigned char, and repeat 2. If private key char is '\0', reset the private key to the beginning.
*/
int NPLCodec::Decode(char* dest, const char* src, int nSrcSize, uint32 nPublicKey)
{
	const unsigned char* pSrc = (const unsigned char*)src;
	unsigned char* pDest = (unsigned char*)dest;
	unsigned char* pPrivateKey = m_sPrivateKey + (nPublicKey % NPL_PRIVATE_KEY_SIZE);

	int i=0;
	for(; i<nSrcSize; ++i, ++pDest, ++pSrc, ++pPrivateKey)
	{
		if((*pPrivateKey) == '\0')
			pPrivateKey = m_sPrivateKey;
		(*pDest) = (*pSrc) ^ (*pPrivateKey);
	}
	return i;
}

int NPLCodec::Encode(char* dest, const char* src, int nSrcSize, uint32 nPublicKey)
{
	const unsigned char* pSrc = (const unsigned char*)src;
	unsigned char* pDest = (unsigned char*)dest;
	unsigned char* pPrivateKey = m_sPrivateKey + (nPublicKey % NPL_PRIVATE_KEY_SIZE);

	int i=0;
	for(; i<nSrcSize; ++i, ++pDest, ++pSrc, ++pPrivateKey)
	{
		if((*pPrivateKey) == '\0')
			pPrivateKey = m_sPrivateKey;
		(*pDest) = (*pSrc) ^ (*pPrivateKey);
	}
	return i;
}

void NPLCodec::UsePlainTextEncoding(bool bUsePlainTextEncoding)
{
	if(bUsePlainTextEncoding)
	{
		// make sure all higher bit of char is 0
		for(int i=0; i<NPL_PRIVATE_KEY_SIZE; i++)
		{
			unsigned char key = m_sPrivateKey[i];
			if((key & 0x80) != 0)
			{
				m_sPrivateKey[i] = key & 0x7f;
			}
		}
	}
	else
	{
		// make sure all higher bit of char is 1
		for(int i=0; i<NPL_PRIVATE_KEY_SIZE; i++)
		{
			unsigned char key = m_sPrivateKey[i];
			if((key & 0x80) == 0)
			{
				m_sPrivateKey[i] = key | 0x80;
			}
		}
	}
}

void NPLCodec::SetGenericKey(const unsigned char* sKey, int nSize)
{
	if(sKey == 0)
		return;
	for(int i=0; i<NPL_PRIVATE_KEY_SIZE; i++)
	{
		if(i < nSize)
		{
			m_sPrivateKey[i] = sKey[i];
		}
		else
		{
			m_sPrivateKey[i] = 0;
		}
	}
}

/**@def CHUNK is simply the buffer size for feeding data to and pulling data from the zlib routines. 
Larger buffer sizes would be more efficient, especially for inflate(). If the memory is available, 
buffers sizes on the order of 128K or 256K bytes should be used. */ 
#define NPL_ZLIB_CHUNK 32768

 int NPLCodec::Compress(std::string& outstring, const char* src, int nSrcSize, int compressionlevel)
{
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	if (deflateInit(&zs, compressionlevel) != Z_OK)
	{
		OUTPUT_LOG("warning: NPLCodec::Compress deflateInit failed while compressing.\n");
		return -1;
	}

	zs.next_in = (Bytef*)src;
	// set the z_stream's input
	zs.avail_in = nSrcSize;

	int ret;
	char outbuffer[NPL_ZLIB_CHUNK];
	// retrieve the compressed bytes blockwise
	do {
		zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
		zs.avail_out = sizeof(outbuffer);

		ret = deflate(&zs, Z_FINISH);

		if (outstring.size() < zs.total_out) {
			// append the block to the output string
			outstring.append(outbuffer,
				zs.total_out - outstring.size());
		}
	} while (ret == Z_OK);

	deflateEnd(&zs);

	if (ret != Z_STREAM_END) {
		OUTPUT_LOG("warning: NPLCodec::Compress failed an error occurred that was not EOF.\n");
		return -1;
	}
	return 1;
}

int NPLCodec::Decompress(string& outstring, const char* src, int nSrcSize)
{
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	if (inflateInit(&zs) != Z_OK)
	{
		OUTPUT_LOG("warning: NPLCodec::Decompress inflateInit failed while decompressing.\n");
		return -1;
	}

	zs.next_in = (Bytef*)src;
	zs.avail_in = nSrcSize;

	int ret;
	char outbuffer[NPL_ZLIB_CHUNK];

	// get the decompressed bytes blockwise using repeated calls to inflate
	do {
		zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
		zs.avail_out = sizeof(outbuffer);

		ret = inflate(&zs, 0);

		if (outstring.size() < zs.total_out) {
			outstring.append(outbuffer,
				zs.total_out - outstring.size());
		}

	} while (ret == Z_OK);

	inflateEnd(&zs);

	if (ret != Z_STREAM_END) {
		OUTPUT_LOG("warning: NPLCodec::Decompress inflateInit an error occurred that was not EOF\n");
		return -1;
	}

	return 1;
}