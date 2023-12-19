#pragma once
#include "IAttributeFields.h"
#include "util/NetworkAdapter.h"

namespace ParaEngine
{
	using namespace luabind;

	/**
	@brief Enum the language type supported now
	*/
	enum class LanguageType
	{
		ENGLISH = 0,
		CHINESE,
		FRENCH,
		ITALIAN,
		GERMAN,
		SPANISH,
		DUTCH,
		RUSSIAN,
		KOREAN,
		JAPANESE,
		HUNGARIAN,
		PORTUGUESE,
		ARABIC,
		NORWEGIAN,
		POLISH
	};

	/**
	* @ingroup ParaEngine
	* global settings for ParaEngine.
	* such as rendering technique, window setting, etc. Most settings can be set from the script interface
	*/
	class ParaEngineSettings : public IAttributeFields
	{
	public:
		ParaEngineSettings(void);
		virtual ~ParaEngineSettings(void);
		/** get an instance of this class.*/
		static ParaEngineSettings& GetSingleton();
	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields
		
		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_ParaEngineSetting;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "ParaEngineSettings"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const char * sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetScriptEditor_s, const char**)	{*p1 = cls->GetScriptEditor().c_str(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetScriptEditor_s, const char*)	{cls->SetScriptEditor(p1); return S_OK;}
		
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetCtorColor_s, Vector3*)		{*p1 = cls->GetCtorColor().ToVector3(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetCtorColor_s, Vector3)		{LinearColor c(p1.x, p1.y,p1.z,1); cls->SetCtorColor(c); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetCtorHeight_s, float*)	{*p1 = cls->GetCtorHeight(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetCtorHeight_s, float)	{cls->SetCtorHeight(p1); return S_OK;}
		
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetCtorSpeed_s, float*)	{*p1 = cls->GetCtorSpeed(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetCtorSpeed_s, float)	{cls->SetCtorSpeed(p1); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetSelectionColor_s, Vector3*)		{*p1 = cls->GetSelectionColor().ToVector3(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetSelectionColor_s, Vector3)		{LinearColor c(p1.x, p1.y,p1.z,1); cls->SetSelectionColor(c); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, IsDebugging_s, bool*)	{*p1 = cls->IsDebugging(); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, IsTouchInputting_s, bool*)	{ *p1 = cls->IsTouchInputting(); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, IsSlateMode_s, bool*)	{ *p1 = cls->IsSlateMode(); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, IsServerMode_s, bool*) { *p1 = cls->IsServerMode(); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, IsEditing_s, bool*)	{*p1 = cls->IsEditing(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetEditingMode_s, bool)	{cls->SetEditingMode(p1); return S_OK;}
		
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetLockWindowSize_s, bool)	{ cls->SetLockWindowSize(p1); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, IsShowWindowTitleBar_s, bool*)	{ *p1 = cls->IsShowWindowTitleBar(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetShowWindowTitleBar_s, bool)	{ cls->SetShowWindowTitleBar(p1); return S_OK; }


		ATTRIBUTE_METHOD1(ParaEngineSettings, GetGameEffectSet_s, int*)	{*p1 = cls->GetGameEffectSet(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, LoadGameEffectSet_s, int)	{cls->LoadGameEffectSet(p1); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetTextureLOD_s, int*)	{*p1 = cls->GetTextureLOD(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetTextureLOD_s, int)	{cls->SetTextureLOD(p1); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetLocale_s, const char**)	{*p1 = cls->GetLocale(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetLocale_s, const char*)	{cls->SetLocale(p1); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetCurrentLanguage_s, int*)	{ *p1 = (int)(cls->GetCurrentLanguage()); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetMouseInverse_s1, bool*)	{*p1 = cls->GetMouseInverse(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetMouseInverse_s1, bool)	{cls->SetMouseInverse(p1); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetIgnoreWindowSizeChange_s, bool*)	{*p1 = cls->GetIgnoreWindowSizeChange(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetIgnoreWindowSizeChange_s, bool)	{cls->SetIgnoreWindowSizeChange(p1); return S_OK;}
			

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetWindowText1_s, const char**)	{*p1 = cls->GetWindowText1(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetWindowText1_s, const char*)	{cls->SetWindowText1(p1); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, IsWindowClosingAllowed_s, bool*)	{*p1 = cls->IsWindowClosingAllowed(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetAllowWindowClosing_s, bool)	{cls->SetAllowWindowClosing(p1); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, HasClosingRequest_s, bool*)	{ *p1 = cls->HasClosingRequest(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetHasClosingRequest_s, bool)	{ cls->SetHasClosingRequest(p1); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, IsFullScreenMode_s, bool*)	{*p1 = cls->IsFullScreenMode(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetFullScreenMode_s, bool)	{cls->SetFullScreenMode(p1); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, IsWindowMaximized_s, bool*)	{*p1 = cls->IsWindowMaximized(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetWindowMaximized_s, bool)	{cls->SetWindowMaximized(p1); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetVisibleSize_s, Vector2*) { *p1 = cls->GetVisibleSize(); return S_OK; }

		ATTRIBUTE_METHOD(ParaEngineSettings, BringWindowToTop_s)	{cls->BringWindowToTop(); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, HasNewConfig_s, bool*)	{*p1 = cls->HasNewConfig(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetHasNewConfig_s, bool)	{cls->SetHasNewConfig(p1); return S_OK;}
		
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetScreenResolution_s, Vector2*)		{*p1 = cls->GetScreenResolution(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetScreenResolution_s, Vector2)		{cls->SetScreenResolution(p1); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetMultiSampleType_s, int*)	{*p1 = cls->GetMultiSampleType(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetMultiSampleType_s, int)	{cls->SetMultiSampleType(p1); return S_OK;}
		
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetMultiSampleQuality_s, int*)	{*p1 = cls->GetMultiSampleQuality(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetMultiSampleQuality_s, int)	{cls->SetMultiSampleQuality(p1); return S_OK;}

		ATTRIBUTE_METHOD(ParaEngineSettings, UpdateScreenMode_s)	{cls->UpdateScreenMode(); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, SetShowMenu_s, bool)	{cls->ShowMenu(p1); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, IsProfilingEnabled_s, bool*)	{*p1 = cls->IsProfilingEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, EnableProfiling_s, bool)	{cls->EnableProfiling(p1); return S_OK;}
		
		ATTRIBUTE_METHOD1(ParaEngineSettings, Is3DRenderingEnabled_s, bool*)	{*p1 = cls->Is3DRenderingEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, Enable3DRendering_s, bool)	{cls->Enable3DRendering(p1); return S_OK;}
		
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetAsyncLoaderItemsLeft_s, int*)	{*p1 = cls->GetAsyncLoaderItemsLeft(-2); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetAsyncLoaderBytesReceived_s, int*)	{*p1 = cls->GetAsyncLoaderBytesReceived(-2); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetAsyncLoaderRemainingBytes_s, int*)	{*p1 = cls->GetAsyncLoaderRemainingBytes(); return S_OK;}
		
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetVertexShaderVersion_s, int*)	{*p1 = cls->GetVertexShaderVersion(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetPixelShaderVersion_s, int*)	{*p1 = cls->GetPixelShaderVersion(); return S_OK;}
		
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetRefreshTimer_s, float*)	{*p1 = cls->GetRefreshTimer(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetRefreshTimer_s, float)	{cls->SetRefreshTimer(p1); return S_OK;}
		
		ATTRIBUTE_METHOD(ParaEngineSettings, AllocConsole_s)	{cls->AllocConsole(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetConsoleTextAttribute_s, int)	{cls->SetConsoleTextAttribute(p1); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetAppCount_s, int*)	{ *p1 = cls->GetAppCount(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetProcessName_s, const char**) { *p1 = cls->GetProcessName(); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetCoreUsage_s, int*)	{*p1 = cls->GetCoreUsage(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetCoreUsage_s, int)	{cls->SetCoreUsage(p1); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetProcessId_s, int*)	{ *p1 = cls->GetProcessId(); return S_OK; }
		
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetSystemInfoString_s, const char**)	{*p1 = cls->GetSystemInfoString().c_str(); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetMaxMacAddress_s, const char**)	{*p1 = cls->GetMaxMacAddress().c_str(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetMaxIPAddress_s, const char**)	{ *p1 = cls->GetMaxIPAddress().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetMachineID_s, const char**) { *p1 = cls->GetMachineID().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetMachineID_old_s, const char**) { *p1 = cls->GetMachineID_old().c_str(); return S_OK; }

		
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetAutoLowerFrameRateWhenNotFocused_s, bool*)	{*p1 = cls->GetAutoLowerFrameRateWhenNotFocused(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetAutoLowerFrameRateWhenNotFocused_s, bool)	{cls->SetAutoLowerFrameRateWhenNotFocused(p1); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetToggleSoundWhenNotFocused_s, bool*)	{*p1 = cls->GetToggleSoundWhenNotFocused(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetToggleSoundWhenNotFocused_s, bool)	{cls->SetToggleSoundWhenNotFocused(p1); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, Is32bitsTextureEnabled_s, bool*)	{*p1 = cls->Is32bitsTextureEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, Enable32bitsTexture_s, bool)	{cls->Enable32bitsTexture(p1); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetAppHasFocus_s, bool*)	{*p1 = cls->GetAppHasFocus(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetAppHasFocus_s, bool)	{cls->SetAppHasFocus(p1); return S_OK;}

		ATTRIBUTE_METHOD1(ParaEngineSettings, IsMouseCaptured_s, bool*)	{ *p1 = cls->IsMouseCaptured(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetCaptureMouse_s, bool)	{ cls->SetCaptureMouse(p1); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, IsSandboxMode_s, bool*) { *p1 = cls->IsSandboxMode(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetSandboxMode_s, bool) { cls->SetSandboxMode(p1); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetDisplayMode_s, const char**)	{*p1 = cls->GetDispalyMode().c_str(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetMonitorResolution_s, Vector2*)	{*p1 = cls->GetMonitorResolution(); return S_OK;}
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetWindowResolution_s, Vector2*)	{ *p1 = cls->GetWindowResolution(); return S_OK; }
		
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetDefaultOpenFileFolder_s, const char*)	{ cls->SetDefaultOpenFileFolder(p1); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetOpenFolder_s, const char**)	{ *p1 = cls->GetOpenFolder(); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, SetIcon_s, const char*)	{ cls->SetIcon(p1); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetPlatform_s, int*)	{ *p1 = cls->GetPlatform(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, IsMobilePlatform_s, bool*)	{ *p1 = cls->IsMobilePlatform(); return S_OK; }
		
		ATTRIBUTE_METHOD1(ParaEngineSettings, Is64BitsSystem_s, bool*)	{ *p1 = cls->Is64BitsSystem(); return S_OK; }

		ATTRIBUTE_METHOD(ParaEngineSettings, RecreateRenderer_s)	{ cls->RecreateRenderer(); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetFPS_s, float*)	{ *p1 = (float)(cls->GetFPS()); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetDrawCallCount_s, int*)	{ *p1 = cls->GetDrawCallCount(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetTriangleCount_s, int*)	{ *p1 = cls->GetTriangleCount(); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetPeakMemoryUse_s, int*)	{ *p1 = cls->GetPeakMemoryUse(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetCurrentMemoryUse_s, int*)	{ *p1 = cls->GetCurrentMemoryUse(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetVertexBufferPoolTotalBytes_s, int*)	{ *p1 = (int)cls->GetVertexBufferPoolTotalBytes(); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetWritablePath_s, const char**) { *p1 = cls->GetWritablePath(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetWritablePath_s, const char*) { cls->SetWritablePath(p1); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetAppHWND_s, double*) { *p1 = (double)cls->GetAppHWND(); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, ResetAudioDevice_s, const char*) { cls->ResetAudioDevice(p1); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetAudioDeviceName_s, const char**) { *p1 = cls->GetAudioDeviceName(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, GetDefaultFileAPIEncoding_s, const char**) { *p1 = cls->GetDefaultFileAPIEncoding().c_str(); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetWorldDirectory_s, const char**) { *p1 = cls->GetWorldDirectory(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetWorldDirectory_s, const char*) { cls->SetWorldDirectory(p1); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetPythonToLua_s, const char**) { *p1 = cls->GetPythonToLua()->c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetPythonToLua_s, const char*) { cls->SetPythonToLua(p1); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetLandscapeMode_s, const char**) { *p1 = cls->GetLandscapeMode().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineSettings, SetLandscapeMode_s, const char*) { cls->SetLandscapeMode(p1); return S_OK; }

		ATTRIBUTE_METHOD1(ParaEngineSettings, GetRendererName_s, const char**) { *p1 = cls->GetRendererName(); return S_OK; }

	public:
		/** if true, IO is restricted to current working directory and writable directory.  Under win32, this is true by default. */
		static bool IsSandboxMode();

		/** if true, IO is restricted to current working directory and writable directory.  Under win32, this is true by default. */
		static void SetSandboxMode(bool val);

		/** 
		* @param dwTechnique: 0 for basic, the higher the more advanced. up to 3. */
		static void SetOceanTechnique(DWORD dwTechnique);

		/** set the default script editor file path for ParaEngine.The windows notepad.exe is used if not specified.  */
		void SetScriptEditor(const string& sEditorFilePath);
		/** Get the default script editor for ParaEngine.*/
		const string& GetScriptEditor();
	
		/** the default open file folder. */
		void SetDefaultOpenFileFolder(const char* sDefaultOpenFileFolder);

		/** display a dialog to let the user select a folder. the default folder can be set by "SetDefaultOpenFileFolder" */
		const char* GetOpenFolder();

		void RecreateRenderer();
		//////////////////////////////////////////////////////////////////////////
		//
		// mesh construction settings
		//
		//////////////////////////////////////////////////////////////////////////
		/** get the construction animation color. */
		LinearColor GetCtorColor();
		static LinearColor GetCtorColorS(){ return GetSingleton().GetCtorColor();};
		/** set the construction animation color. */
		void SetCtorColor(const LinearColor& color);
		static void SetCtorColorS(const LinearColor& color){ GetSingleton().SetCtorColor(color);};

		/** allocate a console with redirected stdin/stdout/stderr, mostly for script debugging. */
		void AllocConsole();

		/** Sets the attributes of characters written to the console screen buffer by the WriteFile  or WriteConsole  function, or echoed by the ReadFile  or ReadConsole  function. 
		* This function affects text written after the function call. 
		* @param wAttributes: please see SetConsoleTextAttribute for a list of font color and styles. 
		*/
		void SetConsoleTextAttribute(int wAttributes);
	
		/** Get the current ParaEngine app usage. 
		* [main thread only]
		* @return see PE_USAGE
		*/
		DWORD GetCoreUsage();

		/** Set the current ParaEngine app usage. 
		* [main thread only]
		* @param dwUsage: bitwise of PE_USAGE
		*/
		void SetCoreUsage(DWORD dwUsage);

		/** get current process id*/
		int GetProcessId();

		/** get the selection color. */
		LinearColor GetSelectionColor(int nGroupID=0);
		static LinearColor GetSelectionColorS(){ return GetSingleton().GetSelectionColor();};
		/** set the construction animation color. */
		void SetSelectionColor(const LinearColor& color,int nGroupID=0);
		static void SetSelectionColorS(const LinearColor& color){ GetSingleton().SetSelectionColor(color);};


		/** Get the construction animation region's height in meters. */
		float  GetCtorHeight();
		static float  GetCtorHeightS(){ return GetSingleton().GetCtorHeight();};
		/** set the construction height. */
		void SetCtorHeight(float fHeight);
		static void SetCtorHeightS(float fHeight){ GetSingleton().SetCtorHeight(fHeight);};

		/** Get the construction Speed. in meters per second*/
		float  GetCtorSpeed();
		static float  GetCtorSpeedS(){ return GetSingleton().GetCtorSpeed();};

		/** set the construction Speed. in meters per second*/
		void SetCtorSpeed(float fSpeed);
		static void SetCtorSpeedS(float fSpeed){ GetSingleton().SetCtorSpeed(fSpeed);};
		
		/**
		 * get client width and height
		 **/
		static void GetClientSize(int* width, int* height);

		/** start/stop all profilers. anything using Start_S does not take effect. */
		static void EnableProfiling(bool bEnable);
		static bool IsProfilingEnabled();

		/** get the render engine stats to output. 
		* @param output: the output buffer. 
		* @param dwFields: current it is 0, which just collect graphics card settings. 
		*/
		static string GetStats(DWORD dwFields);

		/**
		* Returns the peak (maximum so far) resident set size (physical
		* memory use) measured in bytes, or zero if the value cannot be
		* determined on this OS.
		*/
		int GetPeakMemoryUse();

		/**
		* Returns the current resident set size (physical memory use) measured
		* in bytes, or zero if the value cannot be determined on this OS.
		*/
		int GetCurrentMemoryUse();

		/** render refresh/io time interval. */
		static void SetRefreshTimer(float fTimerInterval);

		/** render refresh/io time interval. */
		static float GetRefreshTimer();

		/**get platform id:  
		#define PARA_PLATFORM_UNKNOWN            0
		#define PARA_PLATFORM_IOS                1
		#define PARA_PLATFORM_ANDROID            2
		#define PARA_PLATFORM_WIN32              3
		#define PARA_PLATFORM_MARMALADE          4
		#define PARA_PLATFORM_LINUX              5
		*/
		static int GetPlatform();

		static bool IsMobilePlatform();

		/** whether the current system is compiled with 64bits version. */
		static bool Is64BitsSystem();
		//////////////////////////////////////////////////////////////////////////
		//
		// global effect and light settings
		//
		//////////////////////////////////////////////////////////////////////////

		/** Load the game effect set. This function can be used to load a predefined set of ParaEngine's rendering choices,
		such as best performance, maximum speed, etc. Please note that ParaEngine may change the set content as computer hardware evolves
		Hence, for more granular control over the rendering settings, please use respective functions in the ParaScene and ParaTerrain namespace,etc.
		Please note if a computer does not support a shader in an effect set, the game engine will automatically degrade to a previous level.
		@param nSetID: The following set id are internally supported.
			1024: force using fixed function pipeline. It will run on the oldest possible hardware without using any shaders. 
			0: recommended setting at the time the game engine is released. 
			1: use more expensive settings than the set id=0.
			2: use more expensive settings than the set id=1.
			-1: use less expensive settings than the set id=0.
			-2: use less expensive settings than the set id=-1.
		*/
		void LoadGameEffectSet(int nSetID);
		int GetGameEffectSet();
		static void LoadGameEffectSetS(int nSetID){ GetSingleton().LoadGameEffectSet(nSetID);};
		static int GetGameEffectSetS(){ return GetSingleton().GetGameEffectSet();};

		
		
		/** Most detailed level-of-detail value to set for the mipmap chain. Default value is 0
		More specifically, if the texture was created with the dimensions of 256x256, setting the most detailed level to 0 
		indicates that 256 x 256 is the largest mipmap available, setting the most detailed level to 1 indicates that 128 x 128 
		is the largest mipmap available, and so on, up to the most detailed mip level (the smallest texture size) for the chain.
		*/
		void SetTextureLOD(int nLOD);
		int GetTextureLOD();

		/* get the current number of ParaEngine app running. */
		int GetAppCount();
		/** this usually returns "ParaEngineClient.exe" under win32 */
		const char* GetProcessName();

		/** get total number of draw calls in last rendered frame */
		int GetDrawCallCount();
		/** get total number of triangles in last rendered frame */
		int GetTriangleCount();
		/** get most recent frame per second. */
		int GetFPS();

		/** whether we will load files whose name ends with _32bits as 32 bits textures. 
		* disabling this will reduce memory usage by 6 times, but the rendered image will not be as sharp. since DXT5 or DXT3 compression is used instead. */
		void Enable32bitsTexture(bool bEnable);

		/** whether we will load files whose name ends with _32bits as 32 bits textures. 
		* disabling this will reduce memory usage by 6 times, but the rendered image will not be as sharp. since DXT5 or DXT3 compression is used instead. */
		bool Is32bitsTextureEnabled();

		/** whether ParaEngine is compiled as debug build.*/
		static bool IsDebugging();

		/** whether touch input is in action*/
		static bool IsTouchInputting();

		/** whether in slate mode*/
		static bool IsSlateMode();

		/** whether ParaEngine is loaded from config/config.new.txt. if this is true, we need to ask the user to save to config/config.txt. This is usually done at start up. */
		bool HasNewConfig();
		void SetHasNewConfig(bool bHasNewConfig);

		/** whether the game engine is in developing mode. */
		static bool IsEditing();
		/** set editing mode. 
		@param bEnable: true if enable. */
		static void SetEditingMode(bool bEnable);

		/** whether we are running in server mode without GUI support */
		static bool IsServerMode();

		/**The locale is what makes it possible to know what language the interface is using 
		* and thus determine which specific code is going to be executed
		* @return: 
		"frFR": French
		"deDE": German
		"enUS": American english
		"enGB": British english
		"koKR": Korean
		"zhCN": Chinese (simplified)
		"zhTW": Chinese (traditional)
		"ruRU": Russian (UI AddOn)
		"esES": Spanish
		*/
		const char* GetLocale();
		static const char* GetLocaleS(){ return GetSingleton().GetLocale();};

		/** @see GetLocale().*/
		void SetLocale(const char* sLocale);
		static void SetLocaleS(const char* sLocale){ GetSingleton().SetLocale(sLocale);};

		/** set the application icon at runtime.
		* @param sIconFile: one of the ico file for win32. or it can be "IDI_APPLICATION", "IDI_ERROR", etc for system defined icon. 
		*/
		void SetIcon(const char* sIconFile);

		/** unlike GetLocale(), which may be changed by the user. The current language can not be changed by user. */
		LanguageType GetCurrentLanguage();
		/** this function should not be called by the user. we will always use the default system language. Use SetLocale() for users. */
		void SetCurrentLanguage(LanguageType lang);
		
		/**
		* If ActivateProduct() is called and returns true before this function is called when the application starts, 
		* then this function will return true. Otherwise it will return false. 
		* @note: IsProductActivated() and ActivateProduct() provides a very simple method to ship product with shareware
		* One can use license code generation tools in ParaIDE to generate, say 2000, valid license codes at a time into a text file. 
		* @return 
		*/
		static bool IsProductActivated();
		/**
		* activate the product using a activation code. This function must be called every time the application start with a valid license code,
		* otherwise, IsProductActivated() will always be false.
		* @note: IsProductActivated() and ActivateProduct() provides a very simple method to ship product with shareware
		* One can use license code generation tools in ParaIDE to generate, say 2000, valid license codes at a time into a text file. 
		*/
		static bool ActivateProduct(const char* sActivationCode);

		/**
		* Get the product key for the current application
		* @param sProductName: this can be NULL, it is reserved for future use
		* @return product key is returned.
		*/
		static const char* GetProductKey(const char* sProductName);

		/**
		* generate product activation codes in large numbers in to a given file. 
		* this function is only available in debug build. 
		* @param sOutputFileName the text file that store the generated keys: each key is on a separate line
		*  if this is NULL, a automatically generated file name will be used, which is of the following format.
		*   temp/ParaEngine Licenses 1.0.1.0_enUS_1-2000.txt, where v1.0.1.0 is the software version number, enUS is the language, 0-2000 is nFrom-nTo
		*	This file is usually submitted to shareware website so that they can sell your software. 
		* @param nFrom copy number
		* @param nTo copy number
		* @return 
		*/
		static bool GenerateActivationCodes(const char* sOutputFileName, int nFrom, int nTo);

		/** whether to use full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		void SetFullScreenMode(bool bFullscreen);
		bool IsFullScreenMode();

		/** The BringWindowToTop current window to the top of the Z order. This function only works if IsFullScreenMode() is false.
		*/
		void BringWindowToTop();

		/** change the full screen mode, it does not immediately change the device, call UpdateScreenMode() to update the device. */
		Vector2 GetScreenResolution();
		void SetScreenResolution(const Vector2& vSize);

		/** get current render window resolution. */
		Vector2 GetWindowResolution();

		/** anti-aliasing for both windowed and full screen mode. it does not immediately change the device, call UpdateScreenMode() to update the device.*/
		int GetMultiSampleType();
		void SetMultiSampleType(int nType);

		/** anti-aliasing for both windowed and full screen mode. it does not immediately change the device, call UpdateScreenMode() to update the device.*/
		int GetMultiSampleQuality();
		void SetMultiSampleQuality(int nType);

		/** get the vertex shader version. This can be roughly used to auto-adjust graphics settings. 
		* @return 0,1,2,3,4
		*/
		int GetVertexShaderVersion();

		/** get the pixel shader version. This can be roughly used to auto-adjust graphics settings.  
		* @return 0,1,2,3,4
		*/
		int GetPixelShaderVersion();
		
		/**get all supported display mode(width,height,refresh rate)
		*/
		const std::string& GetDispalyMode();

		Vector2 GetMonitorResolution();


		/** fetch OS information string. 
		* @param bRefresh: true to force all parameters to be refreshed. 
		* @return: the output string. each attribute is printed on a single line, such as "Processor type: 586"
		*/
		const std::string& GetSystemInfoString(bool bRefresh = false);

		/** call this function to update changes of FullScreen Mode and Screen Resolution. */
		void UpdateScreenMode();

		/** switch to either windowed mode or full screen mode. */
		static bool SetWindowedMode(bool bWindowed);

		/** return true if it is currently under windowed mode. */
		static bool IsWindowedMode();

		void SetWindowMaximized(bool bWindowed);
		bool IsWindowMaximized();
		Vector2 GetVisibleSize();

		/** disable 3D rendering, do not present the scene. 
		* This is usually called before and after we show a standard win32 window during full screen mode */
		void Enable3DRendering(bool bPause);

		/** whether 3D rendering is enabled, do not present the scene. 
		* This is usually called before and after we show a standard win32 window during full screen mode, such as a flash window */
		bool Is3DRenderingEnabled();

		/** set whether the user can close the window (sending WM_CLOSE message). Default to true. 
		* When set to false, the scripting interface will receive WM_CLOSE message via system event. And can use ParaEngine.Exit() to quit the application after user confirmation, etc. 
		*/
		static void SetAllowWindowClosing(bool bAllowClosing);

		/** get whether the user can close the window (sending WM_CLOSE message). Default to true. 
		* When set to false, the scripting interface will receive WM_CLOSE message via system event. And can use ParaEngine.Exit() to quit the application after user confirmation, etc. 
		*/
		static bool IsWindowClosingAllowed();

		/** turn on/off menu */
		static void ShowMenu(bool bShow);

		/** switch to ignore windows size change. default to false. 
		* if false, the user is allowed to adjust window size in windowed mode. */
		static void SetIgnoreWindowSizeChange(bool bIgnoreSizeChange);

		/** return true if it is currently under windowed mode. */
		static bool GetIgnoreWindowSizeChange();


		/**  passive rendering, it will not render the scene, but simulation and time remains the same. Default is false
		* this function is usually used in server mode, when the application is run in silent mode
		*/
		static void    EnablePassiveRendering(bool bEnable);
		/**  passive rendering, it will not render the scene, but simulation and time remains the same. Default is false
		* this function is usually used in server mode, when the application is run in silent mode
		*/
		static bool	IsPassiveRenderingEnabled();

		/** set the window title when at windowed mode */
		static void SetWindowText(const char* pChar);
		void SetWindowText1(const char* pChar){SetWindowText(pChar);};
		/** get the window title when at windowed mode */
		static const char* GetWindowText();
		const char* GetWindowText1(){return GetWindowText();};

		/** write the current setting to config file. Such as graphics mode and whether full screen, etc. 
		* config file at ./config.txt will be automatically loaded when the game engine starts. 
		* @param sFileName: if this is "", it will be the default config file at ./config.txt
		*/
		static void WriteConfigFile(const char* sFileName);

		/** set whether the mouse is inversed. */
		void SetMouseInverse(bool bInverse);
		static void SetMouseInverse_s(bool bInverse){GetSingleton().SetMouseInverse(bInverse);};
		bool GetMouseInverse();
		static bool GetMouseInverse_s(){return GetSingleton().GetMouseInverse();};

		/** get string specifying the command line for the application, excluding the program name.*/
		static const char* GetAppCommandLine();

		/**
		* return a specified parameter value in the command line of the application. If the parameter does not exist, the return value is NULL. 
		* @param pParam: key to get
		* @param defaultValue: if the key does not exist, this value will be added and returned. This can be NULL.
		*/
		static const char* GetAppCommandLineByParam(const char* pParam, const object& defaultValue);

		/** set string specifying the command line for the application, excluding the program name.
		* calling this function multiple times with different command line is permitted. The latter settings will merge and override the previous ones.
		* @param pCommandLine: such as   key="value" key2="value2"
		*/
		static void SetAppCommandLine(const char* pCommandLine);

		/**
		* Get the number of items left in the asynchronous content loader queue. 
		* @param nItemType: 
		*  -2 : all remote assets(excluding HTTP web requests) in the queue. 
		*  -1 : all outstanding resources in the queue. 
		*	0 : all local requests in the queue. 
		*	1 : Smaller remote requests in the queue.
		*	2 : HTTP web requests in the queue.
		*	3 : big remote requests in the queue.
		*/
		int GetAsyncLoaderItemsLeft(int nItemType);
		/** same as above, except it is meant to be called from the scripting interface */
		static int GetAsyncLoaderItemsLeftS(int nItemType) {return GetSingleton().GetAsyncLoaderItemsLeft(nItemType);} ;

		/** get the total number of bytes processed. 
		* @param nItemType: 
		*  -2 : all remote assets(excluding HTTP web requests) in the queue. 
		*  -1 : all url requests(include web request) in the queue. 
		*	0 : all local requests in the queue. 
		*	1 : Smaller remote requests in the queue.
		*	2 : HTTP web requests in the queue.
		*	3 : big remote requests in the queue.
		*/
		int GetAsyncLoaderBytesReceived(int nItemType);
		/** same as above, except it is meant to be called from the scripting interface */
		static int GetAsyncLoaderBytesReceivedS(int nItemType) {return GetSingleton().GetAsyncLoaderBytesReceived(nItemType);};

		/** get total estimated size in bytes in the async loader 
		*/
		int GetAsyncLoaderRemainingBytes();

		/// default to false. if true, we will lower frame rate when the window is not focused. 
		void SetAutoLowerFrameRateWhenNotFocused(bool bEnabled); 
		bool GetAutoLowerFrameRateWhenNotFocused(); 

		/// if true, sound volume will be set to 0, when not focused. 
		void SetToggleSoundWhenNotFocused(bool bEnabled); 
		bool GetToggleSoundWhenNotFocused();

		/** app has focus */
		void SetAppHasFocus(bool bEnabled); 
		/** get app focus */
		bool GetAppHasFocus();

		void SetCaptureMouse(bool bCapture);
		bool IsMouseCaptured();

		/** lock window size, so that window size is totally controlled by application instead of user. both min/max/resize window is ignored.*/
		void SetLockWindowSize(bool bEnabled);

		/** show/hide title bar */
		void SetShowWindowTitleBar(bool bEnabled);
		bool IsShowWindowTitleBar();

		const std::string& GetMaxMacAddress();
		const std::string& GetMaxIPAddress();
		const std::string& GetMachineID();
		const std::string& GetMachineID_old();

		/** get total number of bytes in vertex buffer pool for quick stats. */
		size_t GetVertexBufferPoolTotalBytes();

		/** whether there is closing request in the message queue. In windows, the user clicks the close button.
		* For long running task in the main thread, it is recommended to periodically check this value to end the task prematurely if necessary.
		*/
		bool HasClosingRequest();
		void SetHasClosingRequest(bool val);

		/** get the current writable path */
		const char* GetWritablePath();

		/** set current writable path. 
		* TODO: This can be a security problem. use with care. 
		*/
		void SetWritablePath(const char* sPath);

		static intptr_t GetAppHWND();

		void ResetAudioDevice(const char* deviceName);
		const char* GetAudioDeviceName();

		const std::string& GetDefaultFileAPIEncoding();

		/** get current world directory */
		static const char* GetWorldDirectory();

		/** this allows us to reset world directory without unloading the world. 
		* This is useful when saving as the world.
		*/
		static void SetWorldDirectory(const char* sWorldDirectory);

		static std::string* GetPythonToLua() { static std::string s_lua_code; return &s_lua_code; }
		static void SetPythonToLua(const char* python_code);

		const std::string& GetLandscapeMode();
        void SetLandscapeMode(const char* mode);

		/** @return "OpenGL" or "DirectX" */
		const char* GetRendererName();

	protected:
		void LoadNameIndex();
	private:
		/** get the construction animation color. */
		LinearColor m_ctorColor;
		/** get the selection color. */
		LinearColor m_selectionColor;

		float m_ctorHeight;
		float m_ctorSpeed;
		bool m_bEditingMode;
		/** if true, IO is restricted to current working directory and writable directory.  Under win32, this is true by default. */
		static bool m_bSandboxMode;
		/**default script editor executable file path*/
		string m_sScriptEditorPath;
		/** whether to inverse mouse*/
		bool m_bInverseMouse;
		/* current system language. */
		int m_currentLanguage;

		/** 
		"frFR": French
		"deDE": German
		"enUS": American english
		"enGB": British english
		"koKR": Korean
		"zhCN": Chinese (simplified)
		"zhTW": Chinese (traditional)
		"ruRU": Russian (UI AddOn)
		"esES": Spanish
		*/
		string m_sLocale;
		string m_displayModes;

		/** mapping from child name to index */
		std::map<std::string, int> m_name_to_index;
	};
}
