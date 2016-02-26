/*
Code is mostly from Open Asset Import Library (assimp)
----------------------------------------------------------------------
Copyright (c) 2006-2012, assimp team
All rights reserved.
----------------------------------------------------------------------
*/

/** @file Helper class tp perform various byte oder swappings 
   (e.g. little to big endian) */
#pragma once
#include "ParaEngine.h"

#if _MSC_VER >= 1400 
#include <stdlib.h>
#endif

namespace ParaEngine	{
// --------------------------------------------------------------------------------------
/** Defines some useful byte order swap routines.
 * 
 * This is required to read big-endian model formats on little-endian machines,
 * and vice versa. Direct use of this class is DEPRECATED. Use #StreamReader instead. */
// --------------------------------------------------------------------------------------
class ByteSwap
{
	ByteSwap() {}

public:

	// ----------------------------------------------------------------------
	/** Swap two bytes of data
	 *  @param[inout] _szOut A void* to save the reintcasts for the caller. */
	static inline void Swap2(void* _szOut)
	{
		PE_ASSERT(_szOut);

#if _MSC_VER >= 1400
		uint16_t* const szOut = reinterpret_cast<uint16_t*>(_szOut);
		*szOut = _byteswap_ushort(*szOut);
#else
		uint8_t* const szOut = reinterpret_cast<uint8_t*>(_szOut);
		std::swap(szOut[0],szOut[1]);
#endif
	}

	// ----------------------------------------------------------------------
	/** Swap four bytes of data
	 *  @param[inout] _szOut A void* to save the reintcasts for the caller. */
	static inline void Swap4(void* _szOut)
	{
		PE_ASSERT(_szOut);

#if _MSC_VER >= 1400
		uint32_t* const szOut = reinterpret_cast<uint32_t*>(_szOut);
		*szOut = _byteswap_ulong(*szOut);
#else
		uint8_t* const szOut = reinterpret_cast<uint8_t*>(_szOut);
		std::swap(szOut[0],szOut[3]);
		std::swap(szOut[1],szOut[2]);
#endif
	}

	// ----------------------------------------------------------------------
	/** Swap eight bytes of data
	 *  @param[inout] _szOut A void* to save the reintcasts for the caller. */
	static inline void Swap8(void* _szOut)
	{
	PE_ASSERT(_szOut);

#if _MSC_VER >= 1400
		uint64_t* const szOut = reinterpret_cast<uint64_t*>(_szOut);
		*szOut = _byteswap_uint64(*szOut);
#else
		uint8_t* const szOut = reinterpret_cast<uint8_t*>(_szOut);
		std::swap(szOut[0],szOut[7]);
		std::swap(szOut[1],szOut[6]);
		std::swap(szOut[2],szOut[5]);
		std::swap(szOut[3],szOut[4]);
#endif
	}

	// ----------------------------------------------------------------------
	/** ByteSwap a float. Not a joke.
	 *  @param[inout] fOut ehm. .. */
	static inline void Swap(float* fOut) {
		Swap4(fOut);
	}

	// ----------------------------------------------------------------------
	/** ByteSwap a double. Not a joke.
	 *  @param[inout] fOut ehm. .. */
	static inline void Swap(double* fOut) {
		Swap8(fOut);
	}


	// ----------------------------------------------------------------------
	/** ByteSwap an int16t. Not a joke.
	 *  @param[inout] fOut ehm. .. */
	static inline void Swap(int16_t* fOut) {
		Swap2(fOut);
	}

	static inline void Swap(uint16_t* fOut) {
		Swap2(fOut);
	}

	// ----------------------------------------------------------------------
	/** ByteSwap an int32t. Not a joke.
	 *  @param[inout] fOut ehm. .. */
	static inline void Swap(int32_t* fOut){
		Swap4(fOut);
	}

	static inline void Swap(uint32_t* fOut){
		Swap4(fOut);
	}

	// ----------------------------------------------------------------------
	/** ByteSwap an int64t. Not a joke.
	 *  @param[inout] fOut ehm. .. */
	static inline void Swap(int64_t* fOut) {
		Swap8(fOut);
	}

	static inline void Swap(uint64_t* fOut) {
		Swap8(fOut);
	}

	// ----------------------------------------------------------------------
	//! Templatized ByteSwap
	//! \returns param tOut as swapped
	template<typename Type> 
	static inline Type Swapped(Type tOut)
	{
		return _swapper<Type,sizeof(Type)>()(tOut);
	}

private:

	template <typename T, size_t size> struct _swapper;
};

template <typename T> struct ByteSwap::_swapper<T,2> {
	T operator() (T tOut) {
		Swap2(&tOut); 
		return tOut;
	}
};

template <typename T> struct ByteSwap::_swapper<T,4> {
	T operator() (T tOut) {
		Swap4(&tOut); 
		return tOut;
	}
};

template <typename T> struct ByteSwap::_swapper<T,8> {
	T operator() (T tOut) {
		Swap8(&tOut); 
		return tOut;
	}
};


// --------------------------------------------------------------------------------------
// ByteSwap macros for BigEndian/LittleEndian support 
// --------------------------------------------------------------------------------------
#if ! PLATFORM_LITTLE_ENDIAN
#	define PE_LE(t)	(t)
#	define PE_BE(t) ByteSwap::Swapped(t)
#	define PE_LSWAP2(p)
#	define PE_LSWAP4(p)
#	define PE_LSWAP8(p)
#	define PE_LSWAP2P(p)
#	define PE_LSWAP4P(p)
#	define PE_LSWAP8P(p)
#	define LE_NCONST const
#	define PE_SWAP2(p) ByteSwap::Swap2(&(p))
#	define PE_SWAP4(p) ByteSwap::Swap4(&(p))
#	define PE_SWAP8(p) ByteSwap::Swap8(&(p))
#	define PE_SWAP2P(p) ByteSwap::Swap2((p))
#	define PE_SWAP4P(p) ByteSwap::Swap4((p))
#	define PE_SWAP8P(p) ByteSwap::Swap8((p))
#	define BE_NCONST
#else
#	define PE_BE(t)	(t)
#	define PE_LE(t) ByteSwap::Swapped(t)
#	define PE_SWAP2(p)
#	define PE_SWAP4(p)
#	define PE_SWAP8(p)
#	define PE_SWAP2P(p)
#	define PE_SWAP4P(p)
#	define PE_SWAP8P(p)
#	define BE_NCONST const
#	define PE_LSWAP2(p)		ByteSwap::Swap2(&(p))
#	define PE_LSWAP4(p)		ByteSwap::Swap4(&(p))
#	define PE_LSWAP8(p)		ByteSwap::Swap8(&(p))
#	define PE_LSWAP2P(p)	ByteSwap::Swap2((p))
#	define PE_LSWAP4P(p)	ByteSwap::Swap4((p))
#	define PE_LSWAP8P(p)	ByteSwap::Swap8((p))
#	define LE_NCONST
#endif


namespace Intern {

// --------------------------------------------------------------------------------------------
template <typename T, bool doit>
struct ByteSwapper	{
	void operator() (T* inout) {
		ByteSwap::Swap(inout);
	}
};

template <typename T> 
struct ByteSwapper<T,false>	{
	void operator() (T*) {
	}
};

// --------------------------------------------------------------------------------------------
template <bool SwapEndianess, typename T, bool RuntimeSwitch>
struct Getter {
	void operator() (T* inout, bool le) {
#ifdef AI_BUILD_BIG_ENDIAN
		le =  le;
#else
		le =  !le;
#endif
		if (le) {
			ByteSwapper<T,(sizeof(T)>1?true:false)> () (inout);
		}
		else ByteSwapper<T,false> () (inout);
	}
};

template <bool SwapEndianess, typename T> 
struct Getter<SwapEndianess,T,false> {

	void operator() (T* inout, bool /*le*/) {
		// static branch
		ByteSwapper<T,(SwapEndianess && sizeof(T)>1)> () (inout);
	}
};
} // end Intern
}

