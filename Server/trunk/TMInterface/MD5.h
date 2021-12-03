#pragma once
#include <string>
using std::string;

#define uchar unsigned char
typedef unsigned char *POINTER;
typedef unsigned char BYTE;
//typedef unsigned short int UINT2;
//typedef unsigned long int UINT4;
//typedef unsigned long DWORD;
typedef uint16 UINT2;
typedef uint32 UINT4;
typedef uint32 DWORD;
#define PROTO_LIST(list) list

#define MAX_HASH_BUFFER_LENGTH 8192

//MD5 context.
typedef struct md5_ctx {
	UINT4 state[4];                                   // state (ABCD)
	UINT4 count[2];        // number of bits, modulo 2^64 (lsb first)
	unsigned char buffer[64];                         // input buffer
} MD5_CTX;

void MD5Init PROTO_LIST ((MD5_CTX *));
void MD5Update PROTO_LIST
((MD5_CTX *, unsigned char *, unsigned int));
void MD5Final PROTO_LIST ((unsigned char [16], MD5_CTX *));

/* Constants for MD5Transform routine.
*/

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static void MD5Transform PROTO_LIST ((UINT4 [4], unsigned char [64]));
static void Encode PROTO_LIST
((unsigned char *, UINT4 *, unsigned int));
static void Decode PROTO_LIST
((UINT4 *, const unsigned char *, unsigned int));
static void MD5_memcpy PROTO_LIST ((POINTER, POINTER, size_t));
static void MD5_memset PROTO_LIST ((POINTER, int, unsigned int));

static unsigned char PADDING[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions.
*/
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
*/
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
*/
#define FF(a, b, c, d, x, s, ac) { \
	(a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
	}
#define GG(a, b, c, d, x, s, ac) { \
	(a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
	}
#define HH(a, b, c, d, x, s, ac) { \
	(a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
	}
#define II(a, b, c, d, x, s, ac) { \
	(a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
	(a) = ROTATE_LEFT ((a), (s)); \
	(a) += (b); \
	}

class MD5
{
public:
	static const int LENGTH = 16;
private:
	static void MD5Update (MD5_CTX *context, const unsigned char *input, size_t inputLen);
	static void MD5Final (unsigned char digest[LENGTH], MD5_CTX *context);
	static void MD5Transform (UINT4 state[4], const unsigned char block[64]);
	void ToHex(BYTE * Out,BYTE * lpbuffer,DWORD dwSize);
public:
	MD5(void);
	~MD5(void);
	static void GenerateDigest(const unsigned char *input, size_t len, unsigned char *digest);
	static void GenerateDigest(const char *input, size_t len, char *digest)
	{
		GenerateDigest((const unsigned char*)input, len, (unsigned char*)digest);
	}
	static string GenerateMD5String(const unsigned char *input, size_t len)
	{
		unsigned char bufMD5[16] = {0};
		GenerateDigest(input,len,bufMD5);
		return ToHexString(bufMD5);
	}

	/**
	* @brief 16位md5码转32位md5码
	*
	* @param p_md5_buffer_16 指向16位md5码缓冲区
	* @param p_md5_buffer_32 指向32位md5码缓冲区
	* @param is_upper        是否大写(默认0,小写)
	*/
	static void md5_16to32(const unsigned char *p_md5_buffer_16, unsigned char *p_md5_buffer_32, int is_upper = 0);

	static int Compare(const unsigned char* pLeft, const unsigned char* pRight);
	static void FromString(char* stringGuid, unsigned char* pDstMD5);
	static string ToString(const unsigned char* pMd5);
	static string ToGuidString(const unsigned char* pMd5);
	static string ToHexString(const unsigned char* pMD5);
	static void Copy(unsigned char* pDst, const unsigned char* pSrc);
	//static string ToHexFormat2(unsigned char* pBuf, size_t len);
	static string ToHexFormat(const char* pBuf, size_t len);
	static string Base64Encode(const unsigned char* in_str, size_t in_len);
	static const std::string Base64EncodeString(  const unsigned char* input,  std::string::size_type length);
	static void ToHex2(char *Out,const unsigned char *lpbuffer,int dwSize);
	static int gen_chnlhash32(int chnl_id, const char *p_chnl_key, const char *p_data, int data_length, unsigned char *vfy_code);
};

//运行所需参数
#define TEST_BLOCK_LEN 1000
#define TEST_BLOCK_COUNT 1000
static void MDString PROTO_LIST ((char *));
static void MDTimeTrial PROTO_LIST ((void));
static void MDTestSuite PROTO_LIST ((void));
static void MDFile PROTO_LIST ((char *));
static void MDFilter PROTO_LIST ((void));
static void MDPrint PROTO_LIST ((unsigned char [16]));
#define MD5_CTX MD5_CTX
#define MDInit MD5Init
#define MDUpdate MD5Update
#define MDFinal MD5Final
/* Digests a string and prints the result.
*/
