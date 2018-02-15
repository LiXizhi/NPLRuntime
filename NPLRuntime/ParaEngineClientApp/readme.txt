Author: LiXizhi
Company: ParaEngine Co.
Date: 2010.3.2

---++ Overview
This can be used as a sample project for how to create standalone ParaEngine applications using the ParaEngineClient libraries. 
We also show how to load ParaEngine library dynamically at runtime. 

ParaEngineClient also support interprocess-communication. In this case, it is started by another process and render into the window specified by another process. 
The following command line is supported in additional to standard command line
| usage | "webplayer", if so it will do the auto update and use local app directory as the current working directory. |
| app_dir | set the application directory relative to local app data folder, it must be in the format "company_name/product_name", such as "ParaEngine/Demo" or slash can be in other form like "\\TaoMee/Haqi/". For security reasons, company name can only be "ParaEngine" or "TaoMee" |
| updateurl | url for the autoupdater to fetch the core game engine and game files. The string is three urls separated by ;. The first is asset file server, the second and third are version check url. |
| appid | application interprocess queue name. must be unique. Some times it is good to generate it using HWND. |
| apphost | host interprocess queue name. must be unique. Some times it is good to generate it using HWND. |
| minwidth | "960"  minimum resolution width|
| minheight | "560"  minimum resolution height|
| bootstrapper | this is ignored when calling from web plugin for security reasons. |

---++ Deploy Web Redist
More information, please see readme.txt in ParaWebPlayer project. 
   * modify the version.txt to increase the version number
   * modify ParaEngineClient.js and related *.html files to update the redst version and redist url accordingly. 
   * run MakeParaEngineClientRedist.bat to create the installer. It is taomee, where the third is a number like a april fool story.

---+++ upgrading the redist version
If we change the installer name (which is recommended since CDN server will cache by URL), we need to use the formal update, 
otherwise we can use the informal which only requires changing two small strings. 

Formal update with URL changes:
   * edit nsis file in ./ParaWorld/Aries_RedistParaEngineClient.nsi and increase the version, such as from 1002 to 1003, this will change the final installer executable name as well. 
   * edit MakeParaEngineClientRedist.bat to change redist name, such as from 1002 to 1003
   * == the following steps can be done via CI server == 
   * edit ParaEngineClientApp/js/ParaEngineClient.js and look for "redistVersion : " and change the string accordingly. 
   * edit ParaEngineClientApp/js/ParaEngineClient.js and look for "redistInstallUrl" and change accordingly. 
   * edit ParaEngineClientApp/version.txt and change the string accordingly. 
   * edit all related HTML haqi_play.html and change accordingly. 
Informal update without URL changes:
   * edit ParaEngineClientApp/js/ParaEngineClient.js and look for "redistVersion : " and change the string accordingly. 
   * edit ParaEngineClientApp/version.txt and change the string accordingly. 

---+++ upgrading the plugin itself
Normally, there is no need to update the plugin via cab file, just reinstall it together with a new redist installer. But it is important to change the install dir as directed below, otherwise installer will fail to copy the file because it is still in use. 
In addition to upgrade redist, we also need to upgrade the plugin itself. Plugin should rarely update itself, since it is most complicated. 
TODO: I may need to create some script to automate this process. 
More info, see http://msdn.microsoft.com/en-us/library/aa751974%28VS.85%29.aspx and readme.txt in ParaEngineWebPlugin

For IE-only cab files, we can use codebase attribute to specify the plugin addresss. 
<verbatim>
	<object id="paraplugin" type="application/x-paraenginewebplugin" codebase="http://pedn.paraengine.com/webplayer/ParaEngineWebPlugin.cab#version=1,0,0,1" width="20" height="20">
</verbatim>
For both IE and non-IE browsers, we need check plugin().version to see if the user is running an up-to-date version. 
Therefore, to update the plugin, we need to rebuild the plugin with the new version string. 
   * edit /FireBreath-XXX/Projects/ParaEngineWebPlugin/PluginConfig.cmake and change FBSTRING_PLUGIN_VERSION string accordingly, such as from "1.0.0" to "1.0.1"
	To distringuish with the redist version, I use 3 digits with dots. Redist version is 4 digits without dot. Then rebuild the plugin. 
   * edit ParaEngineClientApp/js/ParaEngineClient.js and look for "pluginVersion" and change it accordingly. 
   * edit nsis file in ./ParaWorld/Aries_RedistParaEngineClient.nsi and modify "PluginVersion" accordingly. 
		The reason to do so, is that we need to copy the plugin to a different directory in case the old one is still in use. This is important, otherwise install will not succeed. 
		The installer will check for file existence before doing a copy. 
   * NOTE: file version of activeX plugin is always "1,0,0,1", we do not rely on it for plugin version, instead we update the plugin by redist. 
		However, in case one wants to change the file version, one need to update the object's codebase attributes's #version in HTML, as well as edit firebreath_1.0.0\gen_templates\firebreathWin.rc and change FILEVERSION resource accordingly. 
   * rebuild using MakeParaEngineClientRedist.bat and deploy.
   
---+++ Interprocess Communication
Host App: is the parent process who owns the main window
ParaEngineClientApp: refers to this ParaEngineClient.exe process, which will load the ParaEngineClient.dll in order to render in to the window. 

In order to establish communication, the host App first create a new interprocess message queue with the name "MyAppClient"(it should be any unique name).
Then it write an app connection request message to it {method="app", type=0, param1=0, param2=0, from="AppHostName"}
Next, the host app spawns a ParaEngineClientApp process passing appid="MyAppClient" in the command line. 
When the ParaEngineClientApp process is started with appid in the command line, it knows that it should act as a background 3d app client. 
So it will open an interprocess message queue called "MyAppClient", and begins reading app commands from it. Of course it will receive the app connection request command that is just sent by the host app. 
All app command message has "app" as the method name. The following are minimum hand-shake messages. See InterprocessMsg.h for more details.
   1. PEAPP_SetParentWindow (HWND)
   1. PEAPP_Start ()
   1. PEAPP_Stop ()

---++ Debuggging Web Plugin
	The debug version of ParaEngineClient will set working directory to "D:\\lxzsrc\\ParaEngine\\ParaWorld\\"(I may move this to a registry in future) even from a web plugin. 
So one can copy ParaEngineClient_d.exe to redist dir and rename it. And start debugging from the dev working directory. 
Use "haqi_play.html?debug=on" to show all logs in html page.

---++ Page paramters
haqi_guide.html will forward all HTML request parameters to haqi_play. 
| debug | "on", whether to display log |
| raw | "on", whether to hide all page elements, so that the page can be embedded nicely in other page |
| nid | user nid that is forwarded to plugin control. |

---++ Changes

2013.4.12
	- config.h added, which allow to build with different icon in rc file. 

2012.5.8
	- commandline noupdate="true" will disable autoupdater.

2011.4.11
	- ParaEngineClient.js is modified, so that it will register using simple event instead of AddEventListener if fbVersion is valid. 
	- IE9 will work with simple onpluginmsg, but not AddEvent. I haved submitted the bug to FB official jira bug track site. 
	  TODO: change this back when FB fixed the event handler in IE9. 

2010.11.16
	- WM_NO_PARENTNOTIFY is added for d3d window style
	- haqi_play support debug=on paramter. 

2010.5.25
	- two way communications between game engine and web plugin are implemented. see app_ipc.lua. We can change plugin resolution, restart the web page, exit web page, or anything from within the game engine. 
	- web plugin resolutions will be automatically adjusted according to the user's graphics card quality. 
	
2010.4.26
	- IETester is used to debug HTML view with IE 6/7/8
2010.4.22
	- InterprocessAppClient is implemented. A host process can lanch this process and render into the host's process' window. 
		It is actually a child window created by the client inside the host's window. 
2010.3.3
	- only a single instance is allowed to run unless command line contains: single="false"
	
---++ Know Issues
	* FrameRate using boost is lower than in a standalone process. Perhaps the process's thread priority is not high enough.  e.g. 1/30 only get 22FPS and 1/45 only get 32 FPS. 
	* IME does not work for chinese input method. 