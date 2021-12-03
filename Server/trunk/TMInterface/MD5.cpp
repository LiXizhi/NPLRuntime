#include "TMInterface.h"
#include "MD5.h"
#include <assert.h>

#include <iomanip>
#include <sstream>

#ifdef WIN32
#include <Rpc.h>
#include <Winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <string.h>

using std::setw;
using std::ostringstream;

#pragma warning(disable:4996)

const std::string MD5::Base64EncodeString( const unsigned char* input,  std::string::size_type length)
{
	static unsigned char alphabet64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	const char pad = '=';
	std::string encoded;
	char c;
	
	if(input == 0){
		return "";
	}
	encoded.reserve( length * 2 );

	for( std::string::size_type i = 0; i < length; ++i )
	{
		c = static_cast<char>( ( input[i] >> 2 ) & 0x3f );
		encoded += alphabet64[c];

		c = static_cast<char>( ( input[i] << 4 ) & 0x3f );
		if( ++i < length )
			c = static_cast<char>( c | static_cast<char>( ( input[i] >> 4 ) & 0x0f ) );
		encoded += alphabet64[c];

		if( i < length )
		{
			c = static_cast<char>( ( input[i] << 2 ) & 0x3c );
			if( ++i < length )
				c = static_cast<char>( c | static_cast<char>( ( input[i] >> 6 ) & 0x03 ) );
			encoded += alphabet64[c];
		}
		else
		{
			++i;
			encoded += pad;
		}

		if( i < length )
		{
			c = static_cast<char>( input[i] & 0x3f );
			encoded += alphabet64[c];
		}
		else
		{
			encoded += pad;
		}
	}

	return encoded;
}

string MD5::Base64Encode(const unsigned char* in_str, size_t in_len)
{ 
	static unsigned char base64[] ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	size_t curr_out_len = 0;
	size_t i = 0;
	unsigned char a, b, c;
	char* out_str = new char[in_len * 4 / 3 + 1];
	out_str[0] = '\0';
	if (in_len > 0)
	{
	   while (i < in_len)
	   {
	    a = in_str[i];
	    b = (i + 1 >= in_len) ? 0 : in_str[i + 1];
	    c = (i + 2 >= in_len) ? 0 : in_str[i + 2];
	    if (i + 2 < in_len)
	    {
	     out_str[curr_out_len++] = (base64[(a >> 2) & 0x3F]);
	     out_str[curr_out_len++] = (base64[((a << 4) & 0x30) + ((b >> 4) & 0xf)]);
	     out_str[curr_out_len++] = (base64[((b << 2) & 0x3c) + ((c >> 6) & 0x3)]);
	     out_str[curr_out_len++] = (base64[c & 0x3F]);
	    }
	    else if (i + 1 < in_len)
	    {
	     out_str[curr_out_len++] = (base64[(a >> 2) & 0x3F]);
	     out_str[curr_out_len++] = (base64[((a << 4) & 0x30) + ((b >> 4) & 0xf)]);
	     out_str[curr_out_len++] = (base64[((b << 2) & 0x3c) + ((c >> 6) & 0x3)]);
	     out_str[curr_out_len++] = '=';
	    }
	    else
	    {
	     out_str[curr_out_len++] = (base64[(a >> 2) & 0x3F]);
	     out_str[curr_out_len++] = (base64[((a << 4) & 0x30) + ((b >> 4) & 0xf)]);
	     out_str[curr_out_len++] = '=';
	     out_str[curr_out_len++] = '=';
	    }
	    i += 3;
	   }
	   out_str[curr_out_len] = '\0';
	}

	string ret = out_str;
	delete [] out_str;
	return ret;
}


string MD5::ToHexFormat(const char* pBuf, size_t len)
{
	char* hexStr = new char[len * 2 + 1];
	//string.resize(len * 2 + 1)

	memset(hexStr, 0, len * 2 + 1);
	for(size_t i = 0; i < len; i++) {
		sprintf(hexStr+(i*2), "%02x", (unsigned char)pBuf[i]);
	}

	string ret = hexStr;
	delete [] hexStr;
	return ret;
}

//string MD5::ToHexFormat2(unsigned char* pBuf, size_t len)
//{
//	char* hexStr = new char[len * 2 + 1];
//	//string.resize(len * 2 + 1)
//
//	memset(hexStr, 0, len * 2 + 1);
//	for(size_t i = 0; i < len; i++) {
//		sprintf(hexStr+(i*2), "%02x", (unsigned char)pBuf[i]);
//	}
//
//	string ret = hexStr;
//	delete [] hexStr;
//	return ret;
//}

//string MD5::ToHexFormat(const char* pBuf, size_t len)
//{
//	string ret;
//	ret.resize(len * 2);
//	for(size_t i = 0; i < len; i++) 
//	{
//		snprintf(&(ret[i*2]), 2, "%02x", (unsigned char)pBuf[i]);
//	}
//	return ret;
//}

MD5::MD5(void)
{
}

MD5::~MD5(void)
{
}

void MD5::md5_16to32(const unsigned char *p_md5_buffer_16, unsigned char *p_md5_buffer_32, int is_upper)
{
	const char *lower_set = "0123456789abcdef";
	const char *upper_set = "0123456789ABCDEF";

	if (is_upper)
	{
		for (int i = 0; i < 16; ++ i)
		{
			p_md5_buffer_32[2 * i]
			= upper_set[static_cast<int>(p_md5_buffer_16[i] >> 4)];
			p_md5_buffer_32[2 * i + 1]
			= upper_set[static_cast<int>(p_md5_buffer_16[i] & 0x0F)];
		}
	}
	else
	{
		for (int i = 0; i < 16; ++ i)
		{
			p_md5_buffer_32[2 * i]
			= lower_set[static_cast<int>(p_md5_buffer_16[i] >> 4)];
			p_md5_buffer_32[2 * i + 1]
			= lower_set[static_cast<int>(p_md5_buffer_16[i] & 0x0F)];
		}
	}

	return;
}

int MD5::gen_chnlhash32(int chnl_id, const char *p_chnl_key, const char *p_data, int data_length, unsigned char *vfy_code)
{
	if (!p_chnl_key || !p_data || !vfy_code || data_length < 0)
	{
		return -1;
	}

	char hash_buffer[MAX_HASH_BUFFER_LENGTH];
	unsigned char md5_buffer_16[16];
	unsigned char md5_buffer_32[32];
	int length = sprintf(hash_buffer,
		"channelId=%d&securityCode=%s&data=",
		chnl_id,
		p_chnl_key);

	if (length <= 0)
	{
		return -1;
	}

	// 用户hash的缓冲区不大于MAX_HASH_BUFFER_LENGTH
	int length_empty = static_cast<int>(sizeof(hash_buffer)) - length;
	int length_copy = data_length <= length_empty ? data_length : length_empty;

	memcpy(hash_buffer + length, p_data, length_copy);
	length += length_copy;

	GenerateDigest(reinterpret_cast<unsigned char *>(hash_buffer),length,md5_buffer_16);
	md5_16to32(md5_buffer_16, md5_buffer_32);
	char x[33] = {'\0'};
	memcpy(x, md5_buffer_32, 32);

	memcpy(vfy_code, md5_buffer_32, 32);
	return 0;
}

int MD5::Compare(const unsigned char *pLeft, const unsigned char *pRight)
{
	for (int i = 0; i < LENGTH; ++i)
	{
		if ( pLeft[i] > pRight[i] ) 
		{
			return 1;
		}
		else if ( pLeft[i] < pRight[i] )
		{
			return -1;
		}
	}

	return 0;
}

void MD5::FromString(char* stringGuid, unsigned char* pDstMD5)
{
#ifdef WIN32
	UUID guid;
	UuidFromStringA( (unsigned char*)(stringGuid), &guid );
	memcpy(pDstMD5, &guid, LENGTH);
#else
	unsigned int i1=0,  i5=0;
	unsigned short i2=0, i3=0,i4=0;

	sscanf(stringGuid, "%08x-%04x-%04x-%04x-%08x", &i1,&i2,&i3,&i4,&i5);

	i1=htonl(i1);
	i2=htons(i2);
	i3=htons(i3);

	memcpy(pDstMD5, &i1, sizeof(i1));
	memcpy(pDstMD5+4, &i2, sizeof(i2));
	memcpy(pDstMD5+2, &i3, sizeof(i3));
	memcpy(pDstMD5+2, &i4, sizeof(i4));
	memcpy(pDstMD5+2, &i5, sizeof(i5));
#endif
}

string MD5::ToString(const unsigned char* pMd5)
{
	unsigned long data1 = *( (unsigned long*)pMd5 );
	data1 = ntohl(data1);
	size_t pos = sizeof(data1);
	unsigned short data2 = *( (unsigned short*)(pMd5 + pos) );
	pos += sizeof(data2);
	data2 = ntohs(data2);
	unsigned short data3 = *( (unsigned short*)(pMd5 + pos) );
	pos += sizeof(data3);
	data3 = ntohs(data3);
	
	ostringstream os;
	os.fill('0');
	os<<setw(8)<<std::hex<<data1<<'-'<<setw(4)<<data2<<'-'<<setw(4)<<data3<<'-';
	os<<ToHexFormat((const char*)(pMd5 + pos), 2)<<'-';
	pos += 2;
	os<<ToHexFormat((const char*)(pMd5 + pos), LENGTH - pos);

	return os.str();
}

string MD5::ToGuidString(const unsigned char* pMd5)
{
	return ToString(pMd5);
}
string MD5::ToHexString(const unsigned char* pMD5)
{
	char* hexStr = new char[LENGTH * 2 + 1];
	memset(hexStr, 0, LENGTH * 2 + 1);
	for(size_t i = 0; i < LENGTH; i++)
	{
		sprintf(hexStr+(i*2), "%02x", (unsigned char)pMD5[i]);
	}
	string ret = hexStr;
	delete [] hexStr;
	return ret;
}

/* MD5 initialization. Begins an MD5 operation, writing a new context.
*/
void MD5Init (MD5_CTX *context)
{
	context->count[0] = context->count[1] = 0;
	/* Load magic initialization constants.
	*/
	context->state[0] = 0x67452301;
	context->state[1] = 0xefcdab89;
	context->state[2] = 0x98badcfe;
	context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
operation, processing another message block, and updating the
context.
*/
void MD5::MD5Update (MD5_CTX *context, const unsigned char *input, size_t inputLen)
{
	unsigned int i, index, partLen;

	/* Compute number of bytes mod 64 */
	index = (unsigned int)((context->count[0] >> 3) & 0x3F);

	/* Update number of bits */
	if ((context->count[0] += ((UINT4)inputLen << 3))
		< ((UINT4)inputLen << 3))
		context->count[1]++;
	context->count[1] += ((UINT4)inputLen >> 29);

	partLen = 64 - index;

	/* Transform as many times as possible.
	*/
	if (inputLen >= partLen) {
		MD5_memcpy
			((POINTER)&context->buffer[index], (POINTER)input, partLen);
		MD5Transform (context->state, context->buffer);

		for (i = partLen; i + 63 < inputLen; i += 64)
			MD5Transform (context->state, &input[i]);

		index = 0;
	}
	else
		i = 0;

	/* Buffer remaining input */
	MD5_memcpy
		((POINTER)&context->buffer[index], (POINTER)&input[i],
		inputLen-i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
the message digest and zeroizing the context.
*/
void MD5::MD5Final (unsigned char digest[16], MD5_CTX *context)
{
	unsigned char bits[8];
	unsigned int index, padLen;

	/* Save number of bits */
	Encode (bits, context->count, 8);

	/* Pad out to 56 mod 64.
	*/
	index = (unsigned int)((context->count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	MD5Update (context, PADDING, padLen);

	/* Append length (before padding) */
	MD5Update (context, bits, 8);

	/* Store state in digest */
	Encode (digest, context->state, 16);

	/* Zeroize sensitive information.
	*/
	MD5_memset ((POINTER)context, 0, sizeof (*context));
}

/* MD5 basic transformation. Transforms state based on block.
*/
static void Encode (unsigned char *output, UINT4 *input, unsigned int len)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4) {
		output[j] = (unsigned char)(input[i] & 0xff);
		output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
		output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
		output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
	}
}

/* Decodes input (unsigned char) into output (UINT4). Assumes len is
a multiple of 4.
*/
static void Decode (UINT4 *output, const unsigned char *input, unsigned int len)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4)
		output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
		(((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}

/* Note: Replace "for loop" with standard memcpy if possible.
*/

static void MD5_memcpy (POINTER output, POINTER input, size_t len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		output[i] = input[i];
}

/* Note: Replace "for loop" with standard memset if possible.
*/
static void MD5_memset (POINTER output, int value, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		((char *)output)[i] = (char)value;
}

void MD5::MD5Transform (UINT4 state[4], const unsigned char block[64])
{
	UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

	Decode (x, block, 64);

	/* Round 1 */
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
	FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
	FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
	FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
	FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

	/* Round 2 */
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
	GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
	GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
	GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
	GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
	GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
	HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
	HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

	/* Round 4 */
	II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
	II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
	II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
	II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
	II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
	II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

	/* Zeroize sensitive information.
	*/
	MD5_memset ((POINTER)x, 0, sizeof (x));
}

/* Encodes input (UINT4) into output (unsigned char). Assumes len is
a multiple of 4.
*/
void MD5::GenerateDigest(const unsigned char *string, size_t len, unsigned char *digest)
{
	assert(string != NULL);
	assert(digest != NULL);

	MD5_CTX context;

	memset(digest,0,sizeof(digest));
	MDInit (&context);
	MDUpdate (&context, string, len);
	MDFinal (digest, &context);
}

void MD5::ToHex(uchar * Out,uchar * lpbuffer,DWORD dwSize)
{
	char TMP[4096]={0};
	char t[10]={0};
	for(int i=0;i<(long)dwSize;i++)
	{
		sprintf(t,"%02x",lpbuffer[i]);
		strcat(TMP,t);
	}
	memcpy(Out,TMP,strlen(TMP));
}

void MD5::ToHex2(char *Out,const unsigned char *lpbuffer,int dwSize)
{
        char TMP[32]={0};
        char t1[2]={0};
        int j=0;
        for(int i=0;i<(long)dwSize;i++)
        {
                sprintf(t1,"%c%c",lpbuffer[i*2],lpbuffer[i*2+1]);
                int h=strtol(t1,NULL,16);
                TMP[j]=h;
                j=j+1;
        }
        memcpy(Out,TMP,dwSize);
}

void MD5::Copy(unsigned char *pDst, const unsigned char *pSrc)
{
	memcpy(pDst, pSrc, MD5::LENGTH);
}
