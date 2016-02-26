
//////////////////////////////////////////////////////////////////////
//
// CyoDecode.cpp
//
// Developed by Graham Bull for Cyotec Systems Limited.
// http://www.cyotec.com
//
// Copyright (c) 2004 by Cyotec Systems Limited. All Rights Reserved.
//
// This source code may be used, modified, compiled, duplicated, and/or
// distributed without restriction provided this copyright notice remains intact.
// Cyotec Systems Limited and/or its employees cannot be held responsible for any
// direct or indirect damage or loss of any kind that may result from using this
// code, and provides no warranty, guarantee, or support.
//
// Associated Files
// - CyoDecode.h
// - CyoEncode.h
// - CyoEncode.cpp
//
// History
// - September 22, 2004 - Created
// - November 04, 2004 - Not strictly ANSI compatible!
//
//////////////////////////////////////////////////////////////////////
#include "ParaEngine.h"
#include "CyoDecode.h"

#include <stdexcept> //for std::runtime_error
#include <assert.h> //for assert()

typedef unsigned char BYTE, *LPBYTE;
typedef const char* LPCSTR;

////////////////////////////////////////
// Constants

namespace CyoDecode
{
    DWORD BASE16_INPUT = 2;
    DWORD BASE16_OUTPUT = 1;
    BYTE BASE16_TABLE[ 0x80 ] = {
        /*00-07*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*08-0f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*10-17*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*18-1f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*20-27*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*28-2f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*30-37*/ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, //8 = '0'-'7'
        /*38-3f*/ 0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //2 = '8'-'9'
        /*40-47*/ 0xFF, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xFF, //6 = 'A'-'F'
        /*48-4f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*50-57*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*58-5f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*60-67*/ 0xFF, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xFF, //6 = 'a'-'f' (same as 'A'-'F')
        /*68-6f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*70-77*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*78-7f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
    };

    DWORD BASE32_INPUT = 8;
    DWORD BASE32_OUTPUT = 5;
    BYTE BASE32_TABLE[ 0x80 ] = {
        /*00-07*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*08-0f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*10-17*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*18-1f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*20-27*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*28-2f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*30-37*/ 0xFF, 0xFF, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, //6 = '2'-'7'
        /*38-3f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0xFF, 0xFF, //1 = '='
        /*40-47*/ 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //7 = 'A'-'G'
        /*48-4f*/ 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, //8 = 'H'-'O'
        /*50-57*/ 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, //8 = 'P'-'W'
        /*58-5f*/ 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //3 = 'X'-'Z'
        /*60-67*/ 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //7 = 'a'-'g' (same as 'A'-'G')
        /*68-6f*/ 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, //8 = 'h'-'o' (same as 'H'-'O')
        /*70-77*/ 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, //8 = 'p'-'w' (same as 'P'-'W')
        /*78-7f*/ 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  //3 = 'x'-'z' (same as 'X'-'Z')
    };

    DWORD BASE64_INPUT = 4;
    DWORD BASE64_OUTPUT = 3;
    BYTE BASE64_TABLE[ 0x80 ] = {
        /*00-07*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*08-0f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*10-17*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*18-1f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*20-27*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        /*28-2f*/ 0xFF, 0xFF, 0xFF, 0x3e, 0xFF, 0xFF, 0xFF, 0x3f, //2 = '+' and '/'
        /*30-37*/ 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, //8 = '0'-'7'
        /*38-3f*/ 0x3c, 0x3d, 0xFF, 0xFF, 0xFF, 0x40, 0xFF, 0xFF, //2 = '8'-'9' and '='
        /*40-47*/ 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //7 = 'A'-'G'
        /*48-4f*/ 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, //8 = 'H'-'O'
        /*50-57*/ 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, //8 = 'P'-'W'
        /*58-5f*/ 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //3 = 'X'-'Z'
        /*60-67*/ 0xFF, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, //7 = 'a'-'g'
        /*68-6f*/ 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, //8 = 'h'-'o'
        /*70-77*/ 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, //8 = 'p'-'w'
        /*78-7f*/ 0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  //3 = 'x'-'z'
    };

	const char* c_pszErrorInvalidParameter = "Invalid parameter";
	const char* c_pszErrorNotMultipleOf2 = "Invalid source, size is not a multiple of 2";
	const char* c_pszErrorNotMultipleOf4 = "Invalid source, size is not a multiple of 4";
	const char* c_pszErrorNotMultipleOf8 = "Invalid source, size is not a multiple of 8";
}

////////////////////////////////////////
// Base16 Decoding

unsigned long CyoDecode::Base16DecodeGetLength( unsigned long size )
{
    // output 1 byte for every 2 input:
    //                1
    // 1 = ----1111 = 1111----
    // 2 = ----1111 = ----1111

    if (size % BASE16_INPUT != 0)
        throw std::runtime_error( c_pszErrorNotMultipleOf2 );

    return (((size + BASE16_INPUT - 1) / BASE16_INPUT) * BASE16_OUTPUT);
}

unsigned long CyoDecode::Base16Decode( void* dest, const void* src, unsigned long size )
{
    if ((dest == NULL) || (src == NULL))
        throw std::runtime_error( c_pszErrorInvalidParameter );

    if (size % BASE16_INPUT != 0)
        throw std::runtime_error( c_pszErrorNotMultipleOf2 );

    ///

    LPBYTE pSrc = (LPBYTE)src;
    LPBYTE pDest = (LPBYTE)dest;
    DWORD dwSrcSize = size;
    DWORD dwDestSize = 0;

    while (dwSrcSize >= 1)
    {
        // 2 inputs...
        BYTE in1 = *pSrc++;
        BYTE in2 = *pSrc++;
        dwSrcSize -= BASE16_INPUT; //2

        // Validate ascii...
        assert( 0 <= in1 && in1 <= 0x7f );
        assert( 0 <= in2 && in2 <= 0x7f );

        // Convert ascii to base16...
        in1 = BASE16_TABLE[ in1 ];
        in2 = BASE16_TABLE[ in2 ];

        // Validate base16...
        assert( in1 != 0xff );
        assert( 0 <= in1 && in1 <= 15 );
        assert( 0 <= in2 && in2 <= 15 );

        // 1 output...
        *pDest++ = ((in1 << 4) | in2);
        dwDestSize += BASE16_OUTPUT; //1
    }

    return dwDestSize;
}


////////////////////////////////////////
// Base32 Decoding

unsigned long CyoDecode::Base32DecodeGetLength( unsigned long size )
{
    // output 5 bytes for every 8 input:
    //                1        2        3        4        5
    // 1 = ---11111 = 11111---
    // 2 = ---111XX = -----111 XX------
    // 3 = ---11111 =          --11111-
    // 4 = ---1XXXX =          -------1 XXXX----
    // 5 = ---1111X =                   ----1111 X-------
    // 6 = ---11111 =                            -11111--
    // 7 = ---11XXX =                            ------11 XXX-----
    // 8 = ---11111 =                                     ---11111

    if (size % BASE32_INPUT != 0)
        throw std::runtime_error( c_pszErrorNotMultipleOf8 );

    return (((size + BASE32_INPUT - 1) / BASE32_INPUT) * BASE32_OUTPUT);
}

unsigned long CyoDecode::Base32Decode( void* dest, const void* src, unsigned long size )
{
    if ((dest == NULL) || (src == NULL))
        throw std::runtime_error( c_pszErrorInvalidParameter );

    if (size % BASE32_INPUT != 0)
        throw std::runtime_error( c_pszErrorNotMultipleOf8 );

    ///

    LPBYTE pSrc = (LPBYTE)src;
    LPBYTE pDest = (LPBYTE)dest;
    DWORD dwSrcSize = size;
    DWORD dwDestSize = 0;

    while (dwSrcSize >= 1)
    {
        // 8 inputs...
        BYTE in1 = *pSrc++;
        BYTE in2 = *pSrc++;
        BYTE in3 = *pSrc++;
        BYTE in4 = *pSrc++;
        BYTE in5 = *pSrc++;
        BYTE in6 = *pSrc++;
        BYTE in7 = *pSrc++;
        BYTE in8 = *pSrc++;
        dwSrcSize -= BASE32_INPUT; //8

        // Validate ascii...
        assert( 0 <= in1 && in1 <= 0x7f );
        assert( 0 <= in2 && in2 <= 0x7f );
        assert( 0 <= in3 && in3 <= 0x7f );
        assert( 0 <= in4 && in4 <= 0x7f );
        assert( 0 <= in5 && in5 <= 0x7f );
        assert( 0 <= in6 && in6 <= 0x7f );
        assert( 0 <= in7 && in7 <= 0x7f );
        assert( 0 <= in8 && in8 <= 0x7f );

        // Convert ascii to base16...
        in1 = BASE32_TABLE[ in1 ];
        in2 = BASE32_TABLE[ in2 ];
        in3 = BASE32_TABLE[ in3 ];
        in4 = BASE32_TABLE[ in4 ];
        in5 = BASE32_TABLE[ in5 ];
        in6 = BASE32_TABLE[ in6 ];
        in7 = BASE32_TABLE[ in7 ];
        in8 = BASE32_TABLE[ in8 ];

        // Validate base32...
        assert( in1 != 0xff );
        assert( in2 != 0xff );
        assert( in3 != 0xff );
        assert( in4 != 0xff );
        assert( in5 != 0xff );
        assert( in6 != 0xff );
        assert( in7 != 0xff );
        assert( in8 != 0xff );
        assert( 0 <= in1 && in1 <= 31 );
        assert( 0 <= in2 && in2 <= 31 );
        assert( 0 <= in3 && in3 <= 32 ); //possible padding
        assert( 0 <= in4 && in4 <= 32 ); //possible padding
        assert( 0 <= in5 && in5 <= 32 ); //possible padding
        assert( 0 <= in6 && in6 <= 32 ); //possible padding
        assert( 0 <= in7 && in7 <= 32 ); //possible padding
        assert( 0 <= in8 && in8 <= 32 ); //possible padding

        // 5 outputs...
        *pDest++ = ((in1 & 0x1f) << 3) | ((in2 & 0x1c) >> 2);
        *pDest++ = ((in2 & 0x03) << 6) | ((in3 & 0x1f) << 1) | ((in4 & 0x10) >> 4);
        *pDest++ = ((in4 & 0x0f) << 4) | ((in5 & 0x1e) >> 1);
        *pDest++ = ((in5 & 0x01) << 7) | ((in6 & 0x1f) << 2) | ((in7 & 0x18) >> 3);
        *pDest++ = ((in7 & 0x07) << 5) | (in8 & 0x1f);
        dwDestSize += BASE32_OUTPUT; //5

        //6 4 3 1

        // Padding...
        if (in8 == 32)
        {
            --dwDestSize;
            assert( (in7 == 32 && in6 == 32) || (in7 != 32) );
            if (in6 == 32)
            {
                --dwDestSize;
                if (in5 == 32)
                {
                    --dwDestSize;
                    assert( (in4 == 32 && in3 == 32) || (in4 != 32) );
                    if (in3 == 32)
                    {
                        --dwDestSize;
                    }
                }
            }
        }
    }

    return dwDestSize;
}

////////////////////////////////////////
// Base64 Decoding

unsigned long CyoDecode::Base64DecodeGetLength( unsigned long size )
{
    // output 3 bytes for every 4 input:
    //                1        2        3
    // 1 = --111111 = 111111--
    // 2 = --11XXXX = ------11 XXXX----
    // 3 = --1111XX =          ----1111 XX------
    // 4 = --111111 =                   --111111

    if (size % BASE64_INPUT != 0)
        throw std::runtime_error( c_pszErrorNotMultipleOf4 );

    return (((size + BASE64_INPUT - 1) / BASE64_INPUT) * BASE64_OUTPUT);
}

unsigned long CyoDecode::Base64Decode( void* dest, const void* src, unsigned long size )
{
    if ((dest == NULL) || (src == NULL))
        throw std::runtime_error( c_pszErrorInvalidParameter );

    if (size % BASE64_INPUT != 0)
        throw std::runtime_error( c_pszErrorNotMultipleOf4 );

    ///

    LPBYTE pSrc = (LPBYTE)src;
    LPBYTE pDest = (LPBYTE)dest;
    DWORD dwSrcSize = size;
    DWORD dwDestSize = 0;

    while (dwSrcSize >= 1)
    {
        // 4 inputs...
        BYTE in1 = *pSrc++;
        BYTE in2 = *pSrc++;
        BYTE in3 = *pSrc++;
        BYTE in4 = *pSrc++;
        dwSrcSize -= BASE64_INPUT; //4

        // Validate ascii...
        assert( 0 <= in1 && in1 <= 0x7f );
        assert( 0 <= in2 && in2 <= 0x7f );
        assert( 0 <= in1 && in3 <= 0x7f );
        assert( 0 <= in2 && in4 <= 0x7f );

        // Convert ascii to base16...
        in1 = BASE64_TABLE[ in1 ];
        in2 = BASE64_TABLE[ in2 ];
        in3 = BASE64_TABLE[ in3 ];
        in4 = BASE64_TABLE[ in4 ];

        // Validate base16...
        assert( in1 != 0xff );
        assert( in2 != 0xff );
        assert( in3 != 0xff );
        assert( in4 != 0xff );
        assert( 0 <= in1 && in1 <= 63 );
        assert( 0 <= in2 && in2 <= 63 );
        assert( 0 <= in3 && in3 <= 64 ); //possible padding
        assert( 0 <= in4 && in4 <= 64 ); //possible padding

        // 3 outputs...
        *pDest++ = ((in1 & 0x3f) << 2) | ((in2 & 0x30) >> 4);
        *pDest++ = ((in2 & 0x0f) << 4) | ((in3 & 0x3c) >> 2);
        *pDest++ = ((in3 & 0x03) << 6) | (in4 & 0x3f);
        dwDestSize += BASE64_OUTPUT; //3

        // Padding...
        if (in4 == 64)
        {
            --dwDestSize;
            if (in3 == 64)
            {
                --dwDestSize;
            }
        }
    }

    return dwDestSize;
}
