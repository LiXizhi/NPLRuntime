
//////////////////////////////////////////////////////////////////////
//
// CyoDecode.h
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
// - CyoDecode.cpp
// - CyoEncode.h
// - CyoEncode.cpp
//
// History
// - September 22, 2004 - Created
//
//////////////////////////////////////////////////////////////////////

#ifndef __CYODECODE_H
#define __CYODECODE_H

namespace CyoDecode
{
    // Base16 Decoding
    unsigned long Base16DecodeGetLength( unsigned long size );
    unsigned long Base16Decode( void* dest, const void* src, unsigned long size );

    // Base32 Decoding
    unsigned long Base32DecodeGetLength( unsigned long size );
    unsigned long Base32Decode( void* dest, const void* src, unsigned long size );

    // Base64 Decoding
    unsigned long Base64DecodeGetLength( unsigned long size );
    unsigned long Base64Decode( void* dest, const void* src, unsigned long size );
}

#endif //__CYODECODE_H
