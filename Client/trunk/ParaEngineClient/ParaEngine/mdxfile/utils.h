

#ifndef __UTILS_H__
#define __UTILS_H__
namespace ParaEngine
{

// Auxiliary union TypePointer
// Used for better readability



union TypePointer
{
	UINT*			dw;
	char*			c;
	void*			p;
	float*			f;
	TypePointer() :p(NULL){};
	TypePointer(void* in) :p(in){};
};

}

#endif//__UTILS_H__