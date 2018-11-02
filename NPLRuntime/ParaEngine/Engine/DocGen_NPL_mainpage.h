#pragma once

/** @mainpage The NPL Scripting Language Reference in ParaEngine
* 
* @section intro_sec Introduction
* 
@par
ParaEngine is a full-fledged 3D computer game engine. The key feature of it is to be agile for both application 
development and deployment. It is suitable to create 3D online games where the client and server logics changes 
regularly or there are lots of UGC(user generated content) elements during game play. 

@par
ParaEngine has a scalable and extensible application development framework for both client and server side. 
The client side scripting uses NPL, our proprietory scripting language, whose syntax is based on LUA. The server 
side scripting supports NPL as well as C# via mono. 

@par
As of 2009, the biggest game created by ParaEngine contains over 300000 lines of NPL code, hundreds of html files 
for custom game UI, gigabytes of art content; deployed on the web as a less than 10MB installer 
(all other content are streamed on demand during game play); launched after 6 months of development; 
updated weekly for additional 3D scenes and game logics; generating tens of thousands of user created 3d worlds per day; 
served by over 20 linux servers and expanding; and having over 10000 average concurrent users and growing. 

\n
* @section Modules Modules
*
@par
The NPL and ParaEngine functions and objects are organized inside several modules, such as ParaScene, ParaUI, etc. 
Please browse the module list to quickly locate the function you need. All NPL functions are implemented in the ParaScripting namespace.
Some examples are provided to explain some commonly used functionalities. 

* @section copyright Copyright 
@par
All work is copyrighted by ParaEngine Corporation. \n
URL: http://www.paraengine.com  \n
We are planning to release NPL standard and code to the open source community.\n

*
* @section changs Changes
*
- All major ParaEngine modules are exposed via NPL API.
- Dozens of useful NPL script modules are implemented in the script/ide folder

* @section developer Developers
- LiXizhi: Designer and developer of ParaEngine and NPL (for all its major modules) 
- WangTian(Andy): Core developer of NPL libraries and ParaEngine application framework
*/

/** @defgroup NPL			NPL*/
/** @defgroup ParaNetwork	ParaNetwork*/
/** @defgroup ParaScene		ParaScene*/
/** @defgroup ParaUI		ParaUI*/
/** @defgroup ParaGlobal	ParaGlobal*/
/** @defgroup ParaAsset		ParaAsset*/
/** @defgroup ParaEngine	ParaEngine*/
/** @defgroup ParaWorld		ParaWorld*/
/** @defgroup ParaTerrain	ParaTerrain*/
/** @defgroup ParaCamera	ParaCamera*/
/** @defgroup ParaMovie		ParaMovie*/
/** @defgroup ParaIO		ParaIO*/
/** @defgroup ParaAudio		ParaAudio*/
/** @defgroup ParaMisc		ParaMisc*/
/** @defgroup ParaBootStrapper		ParaBootStrapper*/
/** @defgroup global		global functions*/
/** @defgroup Config		Config*/
/** @defgroup JabberClient	JabberClient*/
/** @defgroup ObjectAttributes		Object Attributes*/


/** @example NPL_samplecode.lua */
/** @example gameinterface.lua */
/** @example helloworld.lua */
/** @example AI_script.lua */
