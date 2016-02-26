
//////////////////////////////////////////////////////////////////////
//
// CyoEncode.h
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
// - CyoEncode.cpp
// - CyoDecode.h
// - CyoDecode.cpp
//
// History
// - September 22, 2004 - Created
//
//////////////////////////////////////////////////////////////////////

#ifndef __CYOENCODE_H
#define __CYOENCODE_H

namespace CyoEncode
{
    // Base16 Encoding
    unsigned long Base16EncodeGetLength( unsigned long size );
    unsigned long Base16Encode( void* dest, const void* src, unsigned long size );

    // Base32 Encoding
    unsigned long Base32EncodeGetLength( unsigned long size );
    unsigned long Base32Encode( void* dest, const void* src, unsigned long size );

    // Base64 Encoding
    unsigned long Base64EncodeGetLength( unsigned long size );
    unsigned long Base64Encode( void* dest, const void* src, unsigned long size );
}

#endif //__CYOENCODE_H
