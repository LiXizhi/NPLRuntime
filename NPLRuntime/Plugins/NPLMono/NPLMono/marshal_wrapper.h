#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/assembly.h>

/** a mono string wrapper. Always allocate this object on the stack. 
e.g.
void ParaGlobal_wrapper::log(MonoString* str) 
{
	if(str!=0)
	{
		mono_string_utf8 str_(str);
		ParaScripting::ParaGlobal::WriteToLogFile(str_.c_str());
	}
}
*/
class mono_string_utf8
{
public:
	mono_string_utf8(MonoString* str);
	~mono_string_utf8();
	/** get const char */
	inline const char* c_str() const {return m_str;}

	/** get const string length in bytes. */
	int size();
private:
	char* m_str;
	int m_nLength;
};