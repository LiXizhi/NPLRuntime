---++ ParaEngine Source Code Overview
| author	| LiXizhi	 |
| date		| 2011.1.31  |
| company	| ParaEngine |

---++ How to Get Source and Compile
Install TortoiseSVN, CMake 2.8.3, AnkhSVN for Visual Studio, DirectX SDK 2007 April(or latest). 

Create Directory: D:/lxzsrc/ParaEngine/ and checkout source code from ParaEngine SVN servers.
Please note that different projects subject to different access rights. 
One needs to have dev accounts on different SVN servers to compile some of the core projects

---++ Directory
All libs are in sub-directory of trunk with their version number included. 

/Client/trunk/ 	:client projects, that depend on some server projects. only win32 is supported.
            /CMakeList.txt:   the all-in-one cmake build file, read the file for details.
            /Client.sln:      cmake auto generated all-in-one solution files
            /ParaEngineClient : Core paraengine client project files
            /ParaWorld/ : Release Output Files
            /ParaWorld/ParaWorld.exe: the main client output file 
            /ParaWorld/ParaWorld.sln: Open this file if only wants to edit ParaEngineClient project, but not the solutions.
            
/Server/trunk/ 	:server projects, normally one should build under linux, code is fully cross platform.
             /CMakeList.txt:   the all-in-one cmake build file, read the file for details.
             /Server.sln:      cmake auto generated all-in-one solution files
             /ParaEngineServer/:  main server project files
             /boost_XXX/:  the boost src
             /ParaEngineServer/ParaEngineServer:  Final executable of server
             /bin/: "sudo make install" will generate files in this directory.
             
/ParaWorld/ : this should be the final product dev directory with all script files, etc

---++ Build Procedure
---+++ Prerequesites 
One needs to install the latest version of CMake. Checkout both client and server in your working folder.
Extract the latest version of boost to the /server/trunk/boost_XXXX/. Install DirectX SDK.

./client/trunk/
./server/trunk/
./server/trunk/boost_XXXX/

---+++ Building boost
run bootstrap in the boost folder to build bjam, and then building the following libraries using the commands below. 
	bjam --build-type=complete msvc stage --with-date_time
	bjam --build-type=complete msvc stage --with-thread 
	bjam --build-type=complete msvc stage --with-filesystem 
	bjam --build-type=complete msvc stage --with-system 
	bjam --build-type=complete msvc stage --with-regex 
	bjam --build-type=complete msvc stage --with-signals
	bjam --build-type=complete msvc stage --with-serialization
	
boost version 1.44. Applying MSVC10 Bug Fix for boost 1.44:
	  - boost 1.44 will generate compiler error mostly because of a vc10 bug. To fix it, one needs to declare BOOST_NO_RVALUE_REFERENCES in boost/config/compiler/visualc.hpp. 
      // https://connect.microsoft.com/VisualStudio/feedback/details/525239/bind
      // http://lists.boost.org/boost-users/2010/09/62542.php
      #define BOOST_NO_RVALUE_REFERENCES
