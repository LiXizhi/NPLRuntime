#pragma once

#include "ParaScriptingCommon.h"
#include "ParaScriptingGlobal.h"
#include "BipedObject.h"

namespace ParaEngine{
	class CBaseObject;
	class CMovieCtrler;
	class CSequenceCtler;
	class CFaceTrackingCtrler;
}
namespace luabind
{
	namespace adl{
		class object;
	}
	using adl::object;	
}
namespace ParaScripting
{
	using namespace std;
	using namespace luabind;
	using namespace ParaEngine;
	class ParaObject;

	/**
	* @ingroup ParaScene
	* ParaMovieCtrler object:
	* it is used to control time based movie of character object.
	*/
	class PE_CORE_DECL ParaMovieCtrler
	{
	public:
		CMovieCtrler*	m_pMovieCtrler;		// a pointer to the object

		/**
		* check if the object is valid
		*/
		bool IsValid(){return m_pMovieCtrler!=NULL;}
	public:
		ParaMovieCtrler():m_pMovieCtrler(NULL){};
		ParaMovieCtrler(CMovieCtrler* pObj):m_pMovieCtrler(pObj){};
		~ParaMovieCtrler(){};
	public:
		/**
		* get the local time of a module of the character
		* @param sFormat: the format of the time in the returning string
		*	"": in seconds
		*   Formated: The format argument consists of one or more codes; as in printf, 
		*	the formatting codes are preceded by a percent sign (%). Characters that do not 
		*	begin with % are copied unchanged. 
		*  %H : Hour in 24-hour format (00 ¨C 23) 
		*  %M : Minute as decimal number (00 ¨C 59) 
		*  %S : Second as decimal number (00 ¨C 59) 
		* e.g.: GetTime("movie", "time elapsed: %H:%M:%S");
		*/
		string GetTime(const char* sFormat);
		/** set the current time of the movie
		* @param fTime: in seconds. 0 is the beginning of the movie
		*/
		void SetTime(float fTime);
		/** suspend the movie*/
		void Suspend();
		/** resume the movie*/
		void Resume();
		/** play the movie*/
		void Play();
		/** record the movie from its current position. 
		* all movies behind will be erased.
		*/
		void Record();

		/** add a new dialog record to the current time. All trailing dialog will be removed.*/
		void RecordNewDialog(const char* sDialog);
		/** add a new action record to the current time. All trailing dialog will be removed.*/
		bool RecordNewAction(const char* sActionName);
		/** add a new action record to the current time. All trailing dialog will be removed.*/
		bool RecordNewAction_(const char* sActionName, float fTime);
		/** add a new effect record to the current time. All trailing dialog will be removed.*/
		void RecordNewEffect(int effectID, const char* sTarget);

		/** get the offset position of the movie. All movie's position key are relative to it.
		* if there is no position key, 0,0,0 is returned
		* e.g. x,y,z = movieCtrler:GetOffsetPosition();
		*/
		void GetOffsetPosition(float *x, float *y, float *z);
		/** set the offset position of the movie. All movie's position key are relative to it.*/
		void SetOffsetPosition(float x, float y, float z);

		/** save recorded movie to file.
		* @param filename:file path to save the movie
		*/
		void SaveMovie(const char* filename);
		/** load a recorded movie from  file.
		* @param filename:file path to load the movie 
		*/
		void LoadMovie(const char* filename);
	};
	/**
	* @ingroup ParaScene
	* ParaSeqCtrler object:
	* A sequence controller is a biped controller which moves the biped according to some predefined sequence.
	* it is used to control sequence based commands of character object.
	*/
	class PE_CORE_DECL ParaSeqCtrler
	{
	public:
		CSequenceCtler*	m_pSequenceCtrler;		// a pointer to the object

		/**
		* check if the object is valid
		*/
		bool IsValid(){return m_pSequenceCtrler!=NULL;}
	public:
		ParaSeqCtrler():m_pSequenceCtrler(NULL){};
		ParaSeqCtrler(CSequenceCtler* pObj):m_pSequenceCtrler(pObj){};
		~ParaSeqCtrler(){};
	public:
		bool Save(bool bOverride);
		bool LoadByID(int nSequenceID);
		bool LoadFromFile(const char* fileName);
		int  Create(const char* name, const char* description, const char* pData,bool bInMemory);
		string ToString();

		/** Get the current absolute playing cursor position*/
		int GetKeyPos();
		/** set the current absolute playing cursor position*/
		void SetKeyPos(int nPos);

		/** get total key count*/
		int GetTotalKeys();
		/** offset the key index according to the current play mode. i.e. it will automatically wrap to the beginning if looping. 
		@param nOffset: number of keys to advance. 
		@return: the number of keys that have been successfully offseted. Usually if the returned value is not equal to the input value, it means
		that the sequence should be paused. */
		int AdvanceKey(int nOffset);
		/** call the command functions(RunTo, MoveTo, etc) only between the matching pair of BeginAddKeys() and EndAddKeys()*/
		void BeginAddKeys();
		/** call the command functions(RunTo, MoveTo, etc) only between the matching pair of BeginAddKeys() and EndAddKeys()*/
		void EndAddKeys();

		int GetSequenceID();
		/** delete keys range 
		@param nFrom: 0 based index. 
		@param nTo: 0 based index, if -1, it means the last one. */
		bool DeleteKeysRange(int nFrom, int nTo);

		/** get the play direction. */
		bool GetPlayDirection();
		/** set the play direction. */
		void SetPlayDirection(bool bForward);

		/** the minimum time between two successive calls. */
		float GetInterval();
		/** the minimum time between two successive calls. */
		void SetInterval(float fInterval);

		/** get the starting position. */
		void GetStartPos(float& x, float& y, float&z);
		/** set the starting position. */
		void SetStartPos(float x, float y, float z);

		/** get the start facing. usually default to 0. */
		float GetStartFacing();
		/** Set the start facing. usually default to 0. */
		void SetStartFacing(float facing);

		/** get the current play mode */
		int GetPlayMode();
		/** set the current play mode */
		void SetPlayMode(int mode);

		/** get the number of seconds after which all move commands will be treated as finished. 
		default value is 30 seconds. */
		float GetMovingTimeout();
		/** set the number of seconds after which all move commands will be treated as finished. 
		default value is 30 seconds. */
		void SetMovingTimeout(float fTimeout);

		/** suspend */
		void Suspend();
		/** resume */
		void Resume();
		//////////////////////////////////////////////////////////////////////////
		// commands:
	public:

		void RunTo(float x,float y,float z);
		void WalkTo(float x,float y,float z);
		void MoveTo(float x,float y,float z);
		void MoveAndTurn(float x,float y,float z,float facing);
		void PlayAnim(const char* sAnim);
		void PlayAnim1(int nAnimID);
		void Wait(float fSeconds);
		void Exec(const char* sCmd);
		void Pause();
		void Turn(float fAngleAbsolute);
		void MoveForward(float fDistance);
		void MoveBack(float fDistance);
		void MoveLeft(float fDistance);
		void MoveRight(float fDistance);
		void Jump();
		void Goto(int nOffset);
		void Goto1(const char* sLable);
		void Lable(const char* sLable);
	};
	
	/**
	* @ingroup ParaScene
	* ParaFaceTrackingCtrler object:
	* it will allow the biped to always face to a given target or another biped.
	*/
	class PE_CORE_DECL ParaFaceTrackingCtrler
	{
	public:
		CFaceTrackingCtrler*	m_pFaceTrackingCtrler;		// a pointer to the object

		/**
		* check if the object is valid
		*/
		bool IsValid(){return m_pFaceTrackingCtrler!=NULL;}
	public:
		ParaFaceTrackingCtrler():m_pFaceTrackingCtrler(NULL){};
		ParaFaceTrackingCtrler(CFaceTrackingCtrler* pObj):m_pFaceTrackingCtrler(pObj){};
		~ParaFaceTrackingCtrler(){};
	public:
		/**
		* instruct a character to face a target for a certain duration. It will smoothly rotate the character neck to face it
		@param x,y,z: which point in world space to face to
		@param fDuration: how many seconds the character should keeps facing the target. Set zero or a negative value to cancel facing previous point. 
		*/
		void FaceTarget(float x, float y, float z, float fDuration);
	};

	/**
	* @ingroup ParaScene
	* ParaObject class:
	* it is used to control game scene objects from scripts.
	*/
	class PE_CORE_DECL ParaCharacter
	{
	public:
		CBipedObject::WeakPtr_type m_pCharacter;		// a pointer to the object

		/**
		* check if the object is valid
		*/
		bool IsValid();
	public:
		ParaCharacter();
		ParaCharacter(CBaseObject* pObj);
		~ParaCharacter();

		/** 
		* the default value of speed scale is 1.0f, which will use the character's default speed.
		*/
		void SetSpeedScale(float fSpeedScale);
		float GetSpeedScale();

		/** if the biped is in air, it will fall down. In case a biped is put to stop and the terrain below it changes. 
		* one should manually call this function to let the biped fall down. Internally it just set the vertical speed to 
		* a small value*/
		void FallDown();

		/** 
		* the default value of size scale is 1.0f, which will use the character's default size.
		* increasing this value will enlarge the character as well as its physical radius.
		*/
		void SetSizeScale(float fScale);
		float GetSizeScale();

		/** 
		* ask the camera to follow this character.The camera will be immediately focused on this character without translation.
		*/
		void SetFocus();

		/** add an action symbol, and let the character state manager determine which states it should go to.
		* this function will not perform any concrete actions on the biped objects.
		* @param nAction: please see script/ide/action_table.lua for a list of valid values.
		* @param param: the param specifying more details about the action. This value default to nil
		*	if nAct is S_ACTIONKEY, then this is const ActionKey* 
		*	if nAct is S_WALK_POINT, then this is nil or 1, specifying whether to use angle.*/
		void AddAction2(int nAction, const object& param);
		void AddAction(int nAction);

		/** return true if character uses walk as the default moving animation.otherwise it uses running.*/
		bool WalkingOrRunning();

		/** whether an animation id exist. this function may have different return value when asset is async loaded. 
		* @param nAnimID: predefined id.
		*/
		bool HasAnimation(int nAnimID);

		/**
		* check whether this character is customizable.
		*/
		bool IsCustomModel();

		/** 
		* Reset base model. Base model is the frame model, to which other sub-models like 
		* weapons and clothing  are attached.
		* NOTE: the current version will delete the entire model instance, so that any sub-models
		* attached to this model will be deleted and that the default appearance of the base model will 
		* show up, if one does not update its equipment after this call.
		* @param assetCharBaseModel: It is the new base model asset;it should be a valid ParaX model asset. 
		*/
		void ResetBaseModel(ParaAssetObject assetCharBaseModel);

		/** set the replaceable skin according to the skin database.
		* this only applies to non-customizable characters.if the index exceeds, it will use the default one.
		* the default skin is at index 0.
		* @param nIndex: the skin index.
		*/
		void SetSkin(int nIndex);

		/** return the current skin index. the default skin is at index 0.*/
		int GetSkin();

		/**
		* Load a stored model in data base by the model set ID. A stored model usually contain 
		* the attachments and equipments, but not the base model. This function is only valid when
		* the base model has already been set.
		* @param nModelSetID: the ID of the model set in the database. Some ID may be reserved for 
		* user-specified model
		*/
		void LoadStoredModel(int nModelSetID);

		/** play a specified animation.
		* @param anims: 
		* - it can be int of animation ID(external animation id is supported)
		*		see local nAnimID = ParaAsset.CreateBoneAnimProvider(-1, filename, filename, false);
		* - it can also be string of animation file name
		* - it can also be a table of {animID, animID}: currently only two are supported. The first one is usually a non-loop, and second one can be loop or non-loop. 
		*/
		void PlayAnimation(const object& anims);
		

		/** get the current animation ID of the character. Usually  0-46 is for normal animation like idle and walking; 0-1000 are reserved for internally animation. 1000-2000 are game specific; 
		* 2000 plus are automatically generated. One should call GetAnimFileName() for id above 2000. 
		* @return: it may return -1 if invalid. 
		*/
		int GetAnimID();

		/** get the current animation's filename. If it is an internal animation, it will return nil. 
		* If it is from bone animation provider, it returns the file name from which the animation is loaded. */
		const char* GetAnimFileName();

		/**
		* set whether the m_mapAnimIDs will be used. Disabled by default
		*/
		void EnableAnimIDMap(bool bEnable);

		/**
		* get whether the m_mapAnimIDs will be used.Disabled by default
		*/
		bool IsAnimIDMapEnabled();

		/**
		* get whether the m_mapAnimIDs will be used.Disabled by default
		* animation ID mapping is a mapping from one animation ID to another ID. This mapping is usually not used (empty). However, it is used when we can 
		* want to secretly replace animation used by this animation instance, by adding a ID to ID map.
		* e.g. we can replace female walk animation ID with an external female walk animation ID. 
		* @note: remember to EnableAnimIDMap() in order for the mapping to take effect. 
		* @param nFromID: 
		* @param nToID: if it is negative, the mapping nFromID is removed. 
		*/
		bool AddAnimIDMap(int nFromID, int nToID);

		/** remove all mapping.*/
		void ClearAllAnimIDMap();

		
		/** whether the character has a mount point at the given ID. Such characters are usually cars, horses, planes, etc. 
		* @param nMountPointID: this is usually 0.
		*/
		bool HasMountPoint(int nMountPointID);

		/** whether the object is mounted on another object. */
		bool IsMounted();

		/** mount the current object on another object. 
		* @param sTarget: the name of the global target object on which the current character is mounted on.
		*/
		void MountOn_(const char* sTarget);

		/**
		* the target must contain at least one mount point. if there are multiple mount point, it will mount to the closet one. 
		* @param target: the name of the target object on which the current character is mounted on.
		*/
		void MountOn(ParaObject& target);

		/**
		* the target must contain at least one mount point. if there are multiple mount point, it will mount to the closet one. 
		* @param target: the name of the target object on which the current character is mounted on.
		* @param nMountID: the attachment ID of the model. if -1 (default), we will attach to the nearest mount position. 
		*/
		void MountOn2(ParaObject& target, int nMountID);

		/** this will force unmount the characer. However, it is usually not natural to do this explicitly, 
		since we do not know how the character should behave after mount. 
		Instead, one can call player_obj:ToCharacter():AddAction(action_table.ActionSymbols.S_JUMP_START) to unmount by jumping off. 
		*/
		void UnMount();

		/**
		* Set the character body parameters. 
		* Need to call RefreshModel() after finished with the settings.
		* All body parameters are integer ranging from 0 to maxType, where maxType is the maximum number of types of
		* of a certain body parameter. For each body parameter, one can specify -1 to retain its current value. This 
		* is useful, when the caller only wants to change one or several of the body parameters. The default body parameters
		* is (0,0,0,0,0).
		*/
		void SetBodyParams(int skinColor, int faceType, int hairColor, int hairStyle, int facialHair);

		/** 
		* Get the character body parameters. 
		* @ param type: the body parameter type of the character
			BP_SKINCOLOR =0,
			BP_FACETYPE = 1,
			BP_HAIRCOLOR = 2,
			BP_HAIRSTYLE = 3,
			BP_FACIALHAIR = 4
		*/
		int GetBodyParams(int type);

		/** Set the display options for the character.
		* in case of boolean input: 1 stands for true; 0 stands for false, -1 stands for retaining its current value. 
		* Need to call RefreshModel() after finished with the settings.
		*/
		void SetDisplayOptions(int bShowUnderwear, int bShowEars, int bShowHair);

		/** 
		* Get the display option parameters.
		* @ param type: the display option parameter of the character
			DO_SHOWUNDERWEAR =0,
			DO_SHOWEARS = 1,
			DO_SHOWHAIR = 2
		*/
		bool GetDisplayOptions(int type);

		/** set the model ID of a specified character slot. 
		* Need to call RefreshModel() after finished with the settings.
		* @param nSlotID: the ID of the slot to be set for the character. Normally there are 16 slots on the character. 
			CS_HEAD =0,
			CS_NECK = 1,
			CS_SHOULDER = 2,
			CS_BOOTS = 3,
			CS_BELT = 4,
			CS_SHIRT = 5,
			CS_PANTS = 6,
			CS_CHEST = 7,
			CS_BRACERS = 8,
			CS_GLOVES = 9,
			CS_HAND_RIGHT = 10,
			CS_HAND_LEFT = 11,
			CS_CAPE = 12,
			CS_TABARD = 13,
			CS_FACE_ADDON = 14, // newly added by andy -- 2009.5.10, Item type: IT_MASK 26
			CS_WINGS = 15, // newly added by andy -- 2009.5.11, Item type: IT_WINGS 27
			CS_ARIES_CHAR_SHIRT = 16, // newly added by andy -- 2009.6.16, Item type: IT_WINGS 28
			CS_ARIES_CHAR_PANT = 17,
			CS_ARIES_CHAR_HAND = 18,
			CS_ARIES_CHAR_FOOT = 19,
			CS_ARIES_CHAR_GLASS = 20,
			CS_ARIES_CHAR_WING = 21,
			CS_ARIES_PET_HEAD = 22,
			CS_ARIES_PET_BODY = 23,
			CS_ARIES_PET_TAIL = 24,
			CS_ARIES_PET_WING = 25,
			CS_ARIES_CHAR_SHIRT_TEEN = 28, // newly added by andy -- 2011.8.13, Item type: IT_ARIES_CHAR_SHIRT_TEEN 40
		* @param nItemID: the ID of the item to be put into the character slot. The default value for all slots is 0. One 
		* may empty a certain slots by setting its nItemID to 0.*/
		void SetCharacterSlot(int nSlotID, int nItemID);

		/** Get the model ID of a specified character slot. 
		* @param nSlotID: the ID of the slot to be set for the character. Normally there are 16 slots on the character. 
		* @return: the item ID on the given slot. 0 stands for empty. -1 if invalid slot ID
		*/
		int GetCharacterSlotItemID(int nSlotID);

		
		/** load all settings of the model to file.
		* Need to call RefreshModel() after loading.
		*/
		void LoadFromFile(const char* filename);
		/** save all settings of the model to file.*/
		void SaveToFile(const char* filename);

		/** update the entire model from its characters settings, such as body parameters and equipments. This will 
		* rebuild the composed character texture. */
		void RefreshModel();

		/** use a specified AI object. 
		* @param sAIType: valid AI object is:
		*  "NPC"|""|"NULL"
		*  "" is the same as "NPC"
		*  "NULL" means no AI module.
		*/
		void UseAIModule(const char* sAIType );

		/** assign a new controller to the current AI object. 
		* if there is no AI object, we will create a default one to use.
		* @param sAICtrlerName:valid AI controller name is:
		*	"sequence"|"movie"|"face"|"follow"|"avoid"
		* @param sParam1: this format of this parameter is dependent on the sAICtrlerName. see below:
		*
		* sAICtrlerName = "face": Face tracking controller:
		* sParam1 = "true"|"false": "true" to enable face tracking.
		* e.g. Char:AssignAIController("face", true);
		*
		* sAICtrlerName = "follow": follow another named biped:
		* sParam1 = "" | "sName" | "sName radius angle" : "" to disable following, or follow a biped called sName.
		*	- sName: the name of the biped to follow,
		*	- radius: [optional, default to 2.5f] it is the default radius around the target biped. it will control the biped to try it best to stand on this circle.
		*	- angle: [optional, default to Pi] it will control the biped to stand beside the target with the target facing shifted by this value. 
		* note that +-Pi means behind the biped;0 means in front of the character.  e.g. "playername", "playername 2.5 3.14", "playername 3.0 0", "playername 3.0 1.57", "playername 3.0 -1.57"
		* e.g. Char:AssignAIController("follow", "player1");
		* e.g. Char:AssignAIController("follow", "player1 2.5 3.14");
		*
		* sAICtrlerName = "movie": enable a movie controller.
		* sParam1 = ""|"true"|"false": "" or "true" to enable a movie, or "false" to disable it.
		* e.g. Char:AssignAIController("movie", "true");Char:AssignAIController("movie", "false");
		* use GetMovieController() to get the controller
		*
		* sAICtrlerName = "sequence": enable a sequence controller.
		* sParam1 = ""|"true"|"false": "" or "true" to enable a sequence, or "false" to disable it.
		* e.g. Char:AssignAIController("sequence", "true");Char:AssignAIController("sequence", "false");
		* use GetSeqController() to get the controller
		*
		* @see: UseAIModule() */
		void AssignAIController(const char* sAICtrlerName, const char* sParam1);
		/** whether a certain controller is enabled. 
		* @param sAICtrlerName: "sequence"|"movie"|"face"|"follow"|"avoid" 
		* see also AssignAIController();
		*/
		bool IsAIControllerEnabled(const char* sAICtrlerName);

		/** get the movie controller. 
		* the movie controller will be created if it does not exist.
		*/
		ParaMovieCtrler GetMovieController();

		/** get the sequence controller. 
		* the sequence controller will be created if it does not exist.
		*/
		ParaSeqCtrler GetSeqController();

		/** get the face tracking controller. 
		* the sequence controller will be created if it does not exist.
		*/
		ParaFaceTrackingCtrler GetFaceTrackingController();

		/** cast a magic effect by the effect ID. 
		* @param nEffectID: effect ID in the effect database
		* if effect id is negative, the effect will be removed from the object.
		*/
		void CastEffect(int nEffectID);
		/** cast a magic effect by the effect ID. 
		* @param nEffectID: effect ID in the effect database
		* if effect id is negative, the effect will be removed from the object.
		* @param sTarget: target of the effect, if there is any.
		*  it can either to the name of the target object, or an attachment ID on the current character.
		*  in case it is an attachment ID. It should be in the following format.
		*  <%d> %d is the attachment ID. For customizable characters, some IDs are:
				0	ATT_ID_SHIELD,
				1	ATT_ID_HAND_RIGHT,
				2	ATT_ID_HAND_LEFT,		-- default value
				5	ATT_ID_SHOULDER_RIGHT,
				6	ATT_ID_SHOULDER_LEFT,
				11	ATT_ID_HEAD,
		* e.g.	char:CastEffect(1, "NPC0"); -- fire missile 1 at NPC0.
		*		char:CastEffect(2, "<2>");	-- attach the effect 2 to the current character's hand.
		*/
		void CastEffect2(int nEffectID, const char* sTarget);

		/**
		* @param ModelAsset the model to be attached. This can be both ParaX model or static mesh model.
		* @param nAttachmentID to which part of the character, the effect model is attached.
			ATT_ID_SHIELD = 0,
			ATT_ID_HAND_RIGHT = 1,
			ATT_ID_HAND_LEFT = 2,
			ATT_ID_TEXT = 3,
			ATT_ID_GROUND = 4,
			ATT_ID_SHOULDER_RIGHT = 5,
			ATT_ID_SHOULDER_LEFT = 6,
			ATT_ID_HEAD = 11,
			ATT_ID_FACE_ADDON = 12,
			ATT_ID_EAR_LEFT_ADDON = 13,
			ATT_ID_EAR_RIGHT_ADDON = 14,
			ATT_ID_BACK_ADDON = 15,
			ATT_ID_WAIST = 16, 
			ATT_ID_NECK = 17, 
			ATT_ID_BOOTS = 18,
			ATT_ID_MOUTH = 19, 
			ATT_ID_MOUNT1 = 20,
		* @param nSlotID the slot id of the effect. default value is -1.  if there is already an effect with the same ID
		*	it will be replaced with this new one. 
		* @param scaling: scaling of the texture
		* @param ReplaceableTexture: replace the texture. 
		*/
		void AddAttachment(ParaAssetObject ModelAsset, int nAttachmentID);
		void AddAttachment3(ParaAssetObject ModelAsset, int nAttachmentID, int nSlotID);
		void AddAttachment4(ParaAssetObject ModelAsset, int nAttachmentID, int nSlotID, float fScaling);
		void AddAttachment5(ParaAssetObject ModelAsset, int nAttachmentID, int nSlotID, float fScaling, ParaAssetObject ReplaceableTexture);

		/** get the attachment object's attribute field. */
		ParaAttributeObject GetAttachmentAttObj(int nAttachmentID);

		/**
		* the model to be detached. @see AddAttachment();
		* @param nAttachmentID: this value is reserved and can be any value.  
		* @param nSlotID the slot id of the effect. default value is -1.  all attachments with the SlotID will be removed.  
		*/
		void RemoveAttachment(int nAttachmentID);
		void RemoveAttachment2(int nAttachmentID, int nSlotID);

		/** stop the biped if it is moving.*/
		void Stop();
		/** move (using the current style i.e. walk or run) to a position relative to the current position. */
		void MoveTo(double x, double y, double z);
		/** move (using the current style i.e. walk or run) to  a position relative to the current position and turn. */
		void MoveAndTurn(double x, double y, double z, float facing);

		//////////////////////////////////////////////////////////////////////////
		//
		// cartoon face functions
		//
		//////////////////////////////////////////////////////////////////////////

		/** get the cartoon face associated with this object. 
		* @param nComponentID: one of the following value.
			// cartoon face slots on the face of the character
			enum CartoonFaceSlots
			{
				CFS_FACE = 0,
				CFS_WRINKLE = 1,
				CFS_EYE = 2,
				CFS_EYEBROW = 3,
				CFS_MOUTH = 4,
				CFS_NOSE = 5,
				CFS_MARKS = 6,

				CFS_TOTAL_NUM, 
			};
		@Param SubType: 
			- 0: style: int [0,00]
			- 1: color: 32bits ARGB
			- 2: scale: float in [-1,1]
			- 3: rotation: float in (-3.14,3.14]
			- 4: x: (-128,128]
			- 5: y: (-128,128]
		* @note: check IsSupportCartoonFace() before using this function 
		*/
		double GetCartoonFaceComponent(int nComponentID, int SubType);
		/** same as GetCartoonFaceComponent, except that the returned value is a string code. 
		* @param nComponentID: enum CartoonFaceSlots, in most cases, this is CFS_FACE(0). 
		* @Param SubType: 
		- 0: style: int [0,00]: code is the string presentation of int.
		- 1: color: 32bits ARGB: code is hex presentation of RGB of the value, such as "ffffff"
		- 2: scale: float in [-1,1]: code is one hex number, where "0" is 0, "9" is 9/9, "a" is -1/7, "f" is -7/7.
		- 3: rotation: float in (-3.14,3.14]: code is at most 2 hex number, where "0" is 0. 
		- 4: x: (-128,128]: code is at most 2 hex number, where "0" is 0. 
		- 5: y: (-128,128]: code is at most 2 hex number, where "0" is 0. 
		* return the code usually hex format, such as "ffffff"
		*/
		string GetCartoonFaceComponentCode(int nComponentID, int SubType);

		/** see GetCartoonFaceComponent() 
		* @note: check IsSupportCartoonFace() before using this function */
		void SetCartoonFaceComponent(int nComponentID, int SubType, double value);
		
		/** see GetCartoonFaceComponentCode() 
		* @note: check IsSupportCartoonFace() before using this function */
		void SetCartoonFaceComponentCode(int nComponentID, int SubType, const char * color);

		/** check whether the associated cartoon model supports cartoon face. */
		bool IsSupportCartoonFace();

		/** the color mask to be applied to cartoon face and ccs base layer. It can be used to fine tune skin color 
		* on top of exiting base skin and face textures. Default value is "ffffff". setting this to a different value
		* will degrade the performance a little. 
		* @param strColor: the hex string of RGB, such as "RGB".
		*/
		void SetSkinColorMask(const char * strColor);

		/** the color mask to be applied to cartoon face and ccs base layer. It can be used to fine tune skin color 
		* on top of exiting base skin and face textures. Default value is "ffffff". setting this to a different value
		* will degrade the performance a little. 
		* @return the hex string of RGB, such as "fffff"
		*/
		string GetSkinColorMask();


		/** get the character gender.
		* @return: 0 if male, 1 if female, -1 if error */
		int GetGender();

		/** get the character race ID according to CharRacesDB table
		* @return: race ID, -1 if error*/
		int GetRaceID();

	};
}