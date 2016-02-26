
//////////////////////////////////////////////////////////////////////
//
// CyoEncode.cpp
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
// - CyoEncode.h
// - CyoDecode.h
// - CyoDecode.cpp
//
// History
// - September 22, 2004 - Created
// - November 04, 2004 - Not strictly ANSI compatible!
//
//////////////////////////////////////////////////////////////////////

#include "ParaEngine.h"
#include "CyoEncode.h"

#include <stdexcept> //for std::runtime_error

typedef unsigned char BYTE, *LPBYTE;
typedef const char* LPCSTR;

////////////////////////////////////////
// Constants

namespace CyoEncode
{
    DWORD BASE16_INPUT = 1;
    DWORD BASE16_OUTPUT = 2;
    LPCSTR BASE16_TABLE = "0123456789ABCDEF";

    DWORD BASE32_INPUT = 5;
    DWORD BASE32_OUTPUT = 8;
    LPCSTR BASE32_TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567=";

    DWORD BASE64_INPUT = 3;
    DWORD BASE64_OUTPUT = 4;
    LPCSTR BASE64_TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

    const char* c_pszErrorInvalidParameter = "Invalid parameter";
}

////////////////////////////////////////
// Base16 Encoding

unsigned long CyoEncode::Base16EncodeGetLength( unsigned long size )
{
    // output 2 bytes for every 1 input:
    //                1
    // 1 = ----1111 = 1111----
    // 2 = ----1111 = ----1111

    return (((size + BASE16_INPUT - 1) / BASE16_INPUT) * BASE16_OUTPUT);
}

unsigned long CyoEncode::Base16Encode( void* dest, const void* src, unsigned long size )
{
    if (!dest || !src)
        throw std::runtime_error( c_pszErrorInvalidParameter );

    ///

    LPBYTE pSrc = (LPBYTE)src;
    LPBYTE pDest = (LPBYTE)dest;
    DWORD dwSrcSize = size;
    DWORD dwDestSize = 0;

    while (dwSrcSize >= 1)
    {
        // 1 input...
        BYTE ch = *pSrc++;
        dwSrcSize -= BASE16_INPUT; //1

        // 2 outputs...
        *pDest++ = BASE16_TABLE[ (ch & 0xf0) >> 4 ];
        *pDest++ = BASE16_TABLE[ (ch & 0x0f)      ];
        dwDestSize += BASE16_OUTPUT; //2
    }

    return dwDestSize;
}

////////////////////////////////////////
// Base32 Encoding

unsigned long CyoEncode::Base32EncodeGetLength( unsigned long size )
{
    // output 8 bytes for every 5 input:
    //                1        2        3        4        5
    // 1 = ---11111 = 11111---
    // 2 = ---111XX = -----111 XX------
    // 3 = ---11111 =          --11111-
    // 4 = ---1XXXX =          -------1 XXXX----
    // 5 = ---1111X =                   ----1111 X-------
    // 6 = ---11111 =                            -11111--
    // 7 = ---11XXX =                            ------11 XXX-----
    // 8 = ---11111 =                                     ---11111

    return (((size + BASE32_INPUT - 1) / BASE32_INPUT) * BASE32_OUTPUT);
}

unsigned long CyoEncode::Base32Encode( void* dest, const void* src, unsigned long size )
{
    if (!dest || !src)
        throw std::runtime_error( c_pszErrorInvalidParameter );

    ///

    LPBYTE pSrc = (LPBYTE)src;
    LPBYTE pDest = (LPBYTE)dest;
    DWORD dwSrcSize = size;
    DWORD dwDestSize = 0;

    while (dwSrcSize >= 1)
    {
        unsigned long dwBlockSize = min( dwSrcSize, BASE32_INPUT );
        PE_ASSERT( 1 <= dwBlockSize && dwBlockSize <= BASE32_INPUT );

        // Encode inputs...
        BYTE n1, n2 = 0, n3 = 0, n4 = 0, n5 = 0, n6 = 0, n7 = 0, n8 = 0;
        switch (dwBlockSize)
        {
        case 5:
            n8  =  (pSrc[ 4 ] & 0x1f);
            n7  = ((pSrc[ 4 ] & 0xe0) >> 5);
        case 4:
            n7 |= ((pSrc[ 3 ] & 0x03) << 3);
            n6  = ((pSrc[ 3 ] & 0x7c) >> 2);
            n5  = ((pSrc[ 3 ] & 0x80) >> 7);
        case 3:
            n5 |= ((pSrc[ 2 ] & 0x0f) << 1);
            n4  = ((pSrc[ 2 ] & 0xf0) >> 4);
        case 2:
            n4 |= ((pSrc[ 1 ] & 0x01) << 4);
            n3  = ((pSrc[ 1 ] & 0x3e) >> 1);
            n2  = ((pSrc[ 1 ] & 0xc0) >> 6);
        case 1:
            n2 |= ((pSrc[ 0 ] & 0x07) << 2);
            n1  = ((pSrc[ 0 ] & 0xf8) >> 3);
            break;

        default:
            PE_ASSERT( false );
        }
        pSrc += dwBlockSize;
        dwSrcSize -= dwBlockSize;

        // Validate...
        PE_ASSERT( 0 <= n1 && n1 <= 31 );
        PE_ASSERT( 0 <= n2 && n2 <= 31 );
        PE_ASSERT( 0 <= n3 && n3 <= 31 );
        PE_ASSERT( 0 <= n4 && n4 <= 31 );
        PE_ASSERT( 0 <= n5 && n5 <= 31 );
        PE_ASSERT( 0 <= n6 && n6 <= 31 );
        PE_ASSERT( 0 <= n7 && n7 <= 31 );
        PE_ASSERT( 0 <= n8 && n8 <= 31 );

        // Padding...
        switch (dwBlockSize)
        {
        case 1: n3 = n4 = 32;
        case 2: n5 = 32;
        case 3: n6 = n7 = 32;
        case 4: n8 = 32;
        case 5:
            break;

        default:
            PE_ASSERT( false );
        }

        // 8 outputs...
        *pDest++ = BASE32_TABLE[ n1 ];
        *pDest++ = BASE32_TABLE[ n2 ];
        *pDest++ = BASE32_TABLE[ n3 ];
        *pDest++ = BASE32_TABLE[ n4 ];
        *pDest++ = BASE32_TABLE[ n5 ];
        *pDest++ = BASE32_TABLE[ n6 ];
        *pDest++ = BASE32_TABLE[ n7 ];
        *pDest++ = BASE32_TABLE[ n8 ];
        dwDestSize += BASE32_OUTPUT; //8
    }

    return dwDestSize;
}

////////////////////////////////////////
// Base64 Encoding

unsigned long CyoEncode::Base64EncodeGetLength( unsigned long size )
{
    // output 4 bytes for every 3 input:
    //                1        2        3
    // 1 = --111111 = 111111--
    // 2 = --11XXXX = ------11 XXXX----
    // 3 = --1111XX =          ----1111 XX------
    // 4 = --111111 =                   --111111

    return (((size + BASE64_INPUT - 1) / BASE64_INPUT) * BASE64_OUTPUT);
}

unsigned long CyoEncode::Base64Encode( void* dest, const void* src, unsigned long size )
{
    if (!dest || !src)
        throw std::runtime_error( c_pszErrorInvalidParameter );

    ///

    LPBYTE pSrc = (LPBYTE)src;
    LPBYTE pDest = (LPBYTE)dest;
    DWORD dwSrcSize = size;
    DWORD dwDestSize = 0;

    while (dwSrcSize >= 1)
    {
        unsigned long dwBlockSize = min( dwSrcSize, BASE64_INPUT );
        PE_ASSERT( 1 <= dwBlockSize && dwBlockSize <= BASE64_INPUT );

        // Encode inputs...
        BYTE n1, n2 = 0, n3 = 0, n4 = 0;
        switch (dwBlockSize)
        {
        case 3:
            n4  =  (pSrc[ 2 ] & 0x3f);
            n3  = ((pSrc[ 2 ] & 0xc0) >> 6);
        case 2:
            n3 |= ((pSrc[ 1 ] & 0x0f) << 2);
            n2  = ((pSrc[ 1 ] & 0xf0) >> 4);
        case 1:
            n2 |= ((pSrc[ 0 ] & 0x03) << 4);
            n1  = ((pSrc[ 0 ] & 0xfc) >> 2);
            break;

        default:
            PE_ASSERT( false );
        }
        pSrc += dwBlockSize;
        dwSrcSize -= dwBlockSize;

        // Validate...
        PE_ASSERT( 0 <= n1 && n1 <= 63 );
        PE_ASSERT( 0 <= n2 && n2 <= 63 );
        PE_ASSERT( 0 <= n3 && n3 <= 63 );
        PE_ASSERT( 0 <= n4 && n4 <= 63 );

        // Padding...
        switch (dwBlockSize)
        {
        case 1: n3 = 64;
        case 2: n4 = 64;
        case 3:
            break;

        default:
            PE_ASSERT( false );
        }

        // 4 outputs...
        *pDest++ = BASE64_TABLE[ n1 ];
        *pDest++ = BASE64_TABLE[ n2 ];
        *pDest++ = BASE64_TABLE[ n3 ];
        *pDest++ = BASE64_TABLE[ n4 ];
        dwDestSize += BASE64_OUTPUT; //4
    }

    return dwDestSize;
}
