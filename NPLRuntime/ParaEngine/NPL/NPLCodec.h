#pragma once

namespace NPL
{
#define NPL_PRIVATE_KEY_SIZE	64
	/**
	* transport layer NPL message compressor(encoder) and decompresser(decoder)
	* Currently, we use a very simply custom encoding/decoding algorithm. 
	* The idea is to change the NPL codecs implementation very often so that 
	* no one will easily decode it without spending a lot of human time. 
	* 
	* [thread safe]: class is thread safe. 
	*/
	class NPLCodec
	{
	public:
		/** encode a unsigned char array from src to dest
		* @note: this algorithm allows the dest to be the same as src, which performs encoding in place of src
		* @param dest: the destination buffer, which needs to be at least nSrcSize in length
		* @param src: the input source bytes to encode
		* @param nSrcSize: length of source in unsigned char
		* @param nPublicKey: the public key. Sometimes, we use the message length as public key to make the message more difficult to decode
		* @return: the number of bytes written to dest. 
		*/
		static int Encode(char* dest, const char* src, int nSrcSize, uint32 nPublicKey = 0);

		/** decode a unsigned char array from src to dest
		* @note: this algorithm allows the dest to be the same as src, which performs decoding in place of src
		* @param dest: the destination buffer, which needs to be at least nSrcSize in length
		* @param src: the input source bytes to decode
		* @param nSrcSize: length of source in unsigned char
		* @param nPublicKey: the public key. Sometimes, we use the message length as public key to make the message more difficult to decode
		* @return: the number of bytes written to dest. 
		*/
		static int Decode(char* dest, const char* src, int nSrcSize, uint32 nPublicKey = 0);

		/**
		* no matter what public key is used, we will ensure that the encoded message is plain text [0,128)
		* [Not Thread Safe]: one must call this function before sending or receiving any encoded messages. 
		* so it is usually called when the game engine starts. 
		*/
		static void UsePlainTextEncoding(bool bUsePlainTextEncoding);

		/**
		* set the generic key that is used for encoding/decoding. If this is not called, the default internal key is used for message encoding. 
		* [Not Thread Safe]: one must call this function before sending or receiving any encoded messages. 
		* so it is usually called when the game engine starts. 
		* @param sKey: the unsigned char array of key
		* @param nSize: size in bytes of the sKey. default is 64 bytes
		*/
		static void SetGenericKey(const unsigned char* sKey, int nSize);

		/** Compress src to a string buffer using zlib. 
		* [Thread safe]
		* @param outstring: the output buffer to store compressed data. Data will be appended to it.  
		* @param src: the input source bytes 
		* @param nSrcSize: length of source in unsigned char
		* @param compressionlevel: compression level, which is an integer in the range of -1 to 9. 
		* Lower compression levels result in faster execution, but less compression. Higher levels result in greater compression, 
		* but slower execution. The zlib constant Z_DEFAULT_COMPRESSION, equal to -1, provides a good compromise between compression 
		* and speed and is equivalent to level 6. Level 0 actually does no compression at all, and in fact expands the data slightly 
		* to produce the zlib format (it is not a unsigned char-for-unsigned char copy of the input). 
		* @return: -1 if failed, 1 if succeed. 
		*/
		static int Compress(string& outstring, const char* src, int nSrcSize, int compressionlevel = -1);

		/** Decompress src to destination. 
		* [Thread safe]: 
		* @param outstring: the output buffer to store decompressed data. Data will be appended to it. 
		* @param src: the input source bytes 
		* @param nSrcSize: length of source in unsigned char
		*/
		static int Decompress(string& outstring, const char* src, int nSrcSize);
	private:
		static unsigned char m_sPrivateKey[NPL_PRIVATE_KEY_SIZE+4];
	};
}
