---++ NPLMono plugin
| author| LiXizhi |
| company| ParaEngine |
| date | 2009.7 |

---+++ Overview

NPLMono now supports multiple thread on linux. However, multiple mono scripting state (Mono 2.4) does not work on windows. 
On windows, if multiple thread is used, GC will randomly throw fatal exception, saying Collecting from unknown thread. the problem doesnot exist on linux. 

NPLMono uses a single application domain, no matter how many MonoScriptingStates are created. 
All assemblies are loaded to the root application domain. Activation calls, however, are asynchronous and multithreaded. 


MonoScriptingState.cpp implements the IMonoScriptingState.h interface. 
NPLMonoInterface.cpp contains wrapper code for C++ API.  
test.cs contains forware declare. 

Please see CMakeLists.txt for how to upgrade monolib and mono.def under windows. 	

---+++ add internal calls
To add a new function in NPLMonoInterface.cpp.
   1. provide the function wrapper in NPLMonoInterface.cpp 
   1. add the binding in NPLMonoInterface_bind.cpp
   1. edit NPLMonoInterface.cs to tell the scripting interface 
---+++ building
	cygwin to build from source: 
	http://www.mono-project.com/Compiling_Mono_on_Windows
	http://www.mono-project.com/Generational_GC
	./configure --host=i686-pc-mingw32 --prefix=/opt/mymono/

---+++ Deployment
	copy following files to ParaEngineSDK root
	- NPLMono2.dll
	- NPLMonoInterface.dll(this is common C# file that one can put anywhere, usually at root)

	there are two libraries in the MONO_INSTALLATION_PATH\bin\ directory: libmonoboehm-2.0.dll and libmonosgen-2.0.dll 
	(you must choose either Boehm or SGen GC implementation and copy them to ParaEngine Folder and rename as mono-2.0.dll)
	the new SGen, which is multithreaded in win32 but crashes on special computers. so I decided to use old boehm which is single threaded in win32 (there can only be one NPL mono thread in win32). 
	- mono-2.0.dll (rename from MONOSDK/bin/libmonoboehm-2.0.dll)
	- libglib-2.0-0.dll
	- intl.dll
	- mono\lib\mono\2.0  <- copy used .net dll from MONO_ROOT\lib\mono\2.0\mscorlib.dll   and MONO_ROOT\lib\mono\2.0\gac\*.dll
	- (optional)mono\etc\mono\2.0  <- copy used files from MONO_ROOT\etc\mono\2.0\

---+++ Testing
	see TestNPLMono.lua
	NPL.activate("NPLMonoInterface.dll/NPLMonoInterface.cs", {data="test NPLMonoInterface"});

---+++ Creating your own Dll
	Andy new .net  dll's the target must be set to 2.0, which is the version inited with Mono. Otherwise it will throw exception and terminate program in win32. 
	One can include the NPLMonoInterface.cs file directly in the new dll or reference it via dll.
	
---+++ references:
	http://www.mono-project.com/Embedding_Mono
	http://www.mono-project.com/Interop_with_Native_Libraries

---+++ GC and MultiThreading
	Until mono 3.4.0 (2014.8), there is no multi-threaded GC in mono in Win32. For any secondary thread created outside the mono environment, one needs to register it with the GC by manually calling GC_Thread_New(). 
Unfortunately the GC_Thread_New() function(boehm GC) is not exposed unless you modify the source code, which is not worth the effort. So the finally solution is just be sure to use only one mono-thread in win32. 

---+++ Changes
	- 2014.8.1: Mono 3.4.0 is compiled from source using CYGWin (cost an hour and lots of dependency and fixes). And it is working with multithreaded GC and winform. 
	- 2014.7.31: Mono3.2.4 seems buggy, use a latest stable version 2.10.9 instead now. 
	- 2014.7.30: seperating debug and release build. 
	- 2014.7.29: mono is upgraded to latest version in win32. and renamed to NPLMono2