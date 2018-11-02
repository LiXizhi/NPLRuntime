#pragma once

/** add internal calls to mono. To add a new function in NPLMonoInterface.cpp.
	1. provide the function wrapper in NPLMonoInterface.cpp 
	1. add the binding in NPLMonoInterface_bind.cpp
	1. edit NPLMonoInterface.cs to tell the scripting interface 
*/
extern void Add_NPL_Internal_Call();