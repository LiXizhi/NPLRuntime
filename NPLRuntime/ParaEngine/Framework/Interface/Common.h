#pragma once
#include <cstdint>
namespace IParaEngine
{

#define  PARA_INVALID_HANDLE UINT16_MAX

	///
#define PARA_HANDLE(_name)                                                           \
	struct _name { uint16_t idx; };                                                  \
	inline bool isValidHandle(_name _handle) { return PARA_INVALID_HANDLE != _handle.idx; }
}