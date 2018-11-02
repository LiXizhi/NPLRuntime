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

---+++ Changes
   - 2014.7.29: mono is upgraded to latest version in win32.