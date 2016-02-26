#pragma once
#include <vector>
#include <stack>

namespace ParaEngine
{
	class IParaEngineApp;
	class TransformStack;
	/** a table of global strings. */
	enum GLOBAL_STRING_ID
	{
		G_STR_EMPTY = 0,
		G_STR_OK,
		G_STR_ERROR,
	};
	/** frame rate controller type */
	enum FRCType
	{
		FRC_RENDER = 0,
		FRC_SIM,
		FRC_IO,
		FRC_GAME,
		FRC_LAST,
	};

	/**
	* a class consists of static functions to access global objects in ParaEngine, 
	* such as Current Root Scene, physical world object, terrain, asset manager, 
	* environment simulator, 2d GUI root, etc.
	*/
	class CGlobals
	{
	public:
		CGlobals(void);
		~CGlobals(void);
	public:
		/** get the application interface. */
		static IParaEngineApp* GetApp();
		static CAISimulator* GetAISim();
		static CReport* GetReport();
		static CFileManager* GetFileManager();
		static CPluginManager* GetPluginManager();
		static ParaInfoCenter::CICConfigManager* GetICConfigManager();
		static CEventsCenter* GetEventsCenter();
		/** get the current NPL runtime environment. */
		static NPL::CNPLRuntime* GetNPLRuntime();
		static NPL::INPLStimulationPipe* GetNetworkLayer();
		static ParaEngineSettings& GetSettings();
		static CGUIRoot* GetGUI();
		static CParaWorldAsset* GetAssetManager();
		static CSceneObject* GetScene();
		static ParaTerrain::CGlobalTerrain* GetGlobalTerrain();
		/** get the scene state information. The scene state contains information about the current simulation
		* and render states.Such as which scene object is being animated or rendered, etc*/
		static SceneState*GetSceneState();
		static CWorldInfo* GetWorldInfo();

		/** get static string used in the game engine. 
		@param nStringID: if this is 0, "" is returned. see GLOBAL_STRING_ID */
		static const string& GetString(int nStringID=0);
		/** return whether frame based report will be generated for a specified item,
		* @params itemName: name of the report item to check, this can be NULL.
		* @return: return true if report should be generated.
		*/
		static bool WillGenReport(const char* itemName = NULL);
		
		/** get the game time in seconds*/
		static double GetGameTime();
		
		/** get a given frame rate controller used by the game engine.*/
		static CFrameRateController* GetFrameRateController(FRCType nType);

		/** get an identity matrix. */
		static const Matrix4* GetIdentityMatrix();

		/** world matrix stack. One can use it to keep track of matrices. 
		* It will be automatically reset at the beginning of each frame move*/
		static TransformStack& GetWorldMatrixStack();
		static TransformStack& GetViewMatrixStack();
		static TransformStack& GetProjectionMatrixStack();
		static CPhysicsWorld* GetPhysicsWorld();
		static CViewportManager* GetViewportManager();
		static CSelectionManager* GetSelectionManager();
		static COceanManager* GetOceanManager();
		static IEnvironmentSim* GetEnvSim();
		static CLightManager* GetLightManager();

		/** check whether the scene is loading. We may display some progress bar to inform the user */
		static bool IsLoading();
		/** set whether the scene is loading. */
		static void SetLoading(bool bIsLoading);

		static RenderDevicePtr GetRenderDevice();
		static EffectManager* GetEffectManager();

		static CAttributesManager* GetAttributesManager();
		static CDataProviderManager* GetDataProviderManager();

		static CMoviePlatform* GetMoviePlatform();

		/**
		* @return application handle
		*/
		static HWND GetAppHWND();
#ifdef PLATFORM_WINDOWS
		static CAudioEngine* GetAudioEngine();
	public:
		/** get directX engine parameters */
		static DirectXEngine& GetDirectXEngine();
#endif
	};


	/** for world view projection transformation stack */
	class TransformStack : public std::stack<Matrix4, std::vector<Matrix4> >
	{
	public:
		TransformStack(){};

		/** get the top() matrix, if the stack is empty, it will return identity matrix */
		inline const Matrix4& SafeGetTop(){ return !empty() ? top() : Matrix4::IDENTITY; }

		/** it will only set if top is not empty */
		inline void SafeSetTop(const Matrix4& mat){ 	if(!empty()) top() = mat;}
	};

}