#pragma once
#include "ParaNamespaces.h"

namespace ParaEngine
{
	template <class T>
	inline const T* constVectorData(const typename std::vector<T>& data){
		return !data.empty() ? &(data[0]) : NULL;
	}
}
