#pragma once
#include "TileObject.h"
#include <vector>
namespace ParaEngine
{
	class CAIBase;
	class CShapeAABB;

	/** this is an interface class for game objects, such as NPC, OPC and players. 
	* It contains a list of call back functions that the game engine will automatically call during the simulation.
	* The CBipedObject implements this interface. This class is in fact, not a pure interface. It contains some over-writable
	* implementations such as the script event registration management. 
	* For each callback functions in game object, there can be (optionally) a NPL script function, 
	* which will be called after the C++ function is called. */
	class IGameObject:public CTileObject
	{
	public:
		typedef ParaEngine::weak_ptr<IObject, IGameObject> WeakPtr_type;

		IGameObject();
		virtual ~IGameObject();
		virtual CBaseObject::_SceneObjectType GetType(){return CBaseObject::GameObject;};
		
	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_IGameObject;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "IGameObject"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/**
		* Reset the field to its initial or default value. 
		* @param nFieldID : field ID
		* @return true if value is set; false if value not set. 
		*/
		virtual bool ResetField(int nFieldID);

		/**
		* Invoke an (external) editor for a given field. This is usually for NPL script field
		* @param nFieldID : field ID
		* @param sParameters : the parameter passed to the editor
		* @return true if editor is invoked, false if failed or field has no editor. 
		*/
		virtual bool InvokeEditor(int nFieldID, const string& sParameters);

		
		ATTRIBUTE_METHOD1(IGameObject, IsModified_s, bool*)		{*p1 = cls->IsModified(); return S_OK;}
		ATTRIBUTE_METHOD1(IGameObject, SetModified_s, bool)		{cls->SetModified(p1); return S_OK;}
		ATTRIBUTE_METHOD(IGameObject, SaveToDB_s)		{return cls->SaveToDB()?S_OK:E_FAIL;}

		ATTRIBUTE_METHOD1(IGameObject, GetOnLoadScript_s, const char**)	{*p1 = cls->GetOnLoadScript().c_str(); return S_OK;}
		ATTRIBUTE_METHOD1(IGameObject, SetOnLoadScript_s, const char*)	{cls->SetOnLoadScript(p1); return S_OK;}
		
		ATTRIBUTE_METHOD1(IGameObject, IsLoaded_s, bool*)	{*p1 = cls->IsLoaded(); return S_OK;}
		ATTRIBUTE_METHOD1(IGameObject, SetLoaded_s, bool)	{cls->SetLoaded(p1); return S_OK;}

		DEFINE_SCRIPT_EVENT(IGameObject, EnterSentientArea);
		DEFINE_SCRIPT_EVENT(IGameObject, LeaveSentientArea);
		DEFINE_SCRIPT_EVENT(IGameObject, Click);
		DEFINE_SCRIPT_EVENT(IGameObject, Event);
		DEFINE_SCRIPT_EVENT(IGameObject, Perception);
		DEFINE_SCRIPT_EVENT(IGameObject, FrameMove);
		DEFINE_SCRIPT_EVENT(IGameObject, Net_Send);
		DEFINE_SCRIPT_EVENT(IGameObject, Net_Receive);


		ATTRIBUTE_METHOD1(IGameObject, IsGlobal_s, bool*)		{*p1 = cls->IsGlobal(); return S_OK;}
		ATTRIBUTE_METHOD1(IGameObject, MakeGlobal_s, bool)		{cls->MakeGlobal(p1); return S_OK;}

		ATTRIBUTE_METHOD1(IGameObject, IsSentient_s, bool*)			{*p1 = cls->IsSentient(); return S_OK;}
		ATTRIBUTE_METHOD1(IGameObject, MakeSentient_s, bool)		{cls->MakeSentient(p1); return S_OK;}

		ATTRIBUTE_METHOD1(IGameObject, IsAlwaysSentient_s, bool*)		{*p1 = cls->IsAlwaysSentient(); return S_OK;}
		ATTRIBUTE_METHOD1(IGameObject, SetAlwaysSentient_s, bool)		{cls->SetAlwaysSentient(p1); return S_OK;}

		ATTRIBUTE_METHOD1(IGameObject, GetSentientRadius_s, float*)		{*p1 = cls->GetSentientRadius(); return S_OK;}
		ATTRIBUTE_METHOD1(IGameObject, SetSentientRadius_s, float)		{cls->SetSentientRadius(p1); return S_OK;}
		
		ATTRIBUTE_METHOD1(IGameObject, GetPerceptiveRadius_s, float*)			{*p1 = cls->GetPerceptiveRadius(); return S_OK;}
		ATTRIBUTE_METHOD1(IGameObject, SetPerceptiveRadius_s, float)		{cls->SetPerceptiveRadius(p1); return S_OK;}

		ATTRIBUTE_METHOD1(IGameObject, GetGroupID_s, int*)			{*p1 = cls->GetGroupID(); return S_OK;}
		ATTRIBUTE_METHOD1(IGameObject, SetGroupID_s, int)		{cls->SetGroupID(p1); return S_OK;}

		ATTRIBUTE_METHOD1(IGameObject, GetSentientField_s, int*)			{*p1 = cls->GetSentientField(); return S_OK;}
		ATTRIBUTE_METHOD1(IGameObject, SetSentientField_s, int)		{cls->SetSentientField(p1, false); return S_OK;}
		
		ATTRIBUTE_METHOD1(IGameObject, GetFrameMoveInterval_s, int*)			{*p1 = cls->GetFrameMoveInterval(); return S_OK;}
		ATTRIBUTE_METHOD1(IGameObject, SetFrameMoveInterval_s, int)		{cls->SetFrameMoveInterval(p1); return S_OK;}

	public:
		/** save the current character to database according to the persistent property.
		if the object is persistent, the action is to update or insert the character to db
		if the object is non-persistent, the action is to delete it from the database. */
		virtual bool SaveToDB();

		/** automatically generate way points according to its perceptions */
		virtual void PathFinding(double dTimeDelta);

		/** animate biped according to its current way point lists and speed.
		* assuming that no obstacles are in the way it moves.*/
		virtual void AnimateBiped( double dTimeDelta, bool bSharpTurning = false /*reserved*/);		


		/** Get the AI module that is dynamically associated with this object */
		virtual CAIBase*		 GetAIModule();

		//////////////////////////////////////////////////////////////////////////
		// the following must be implemented. The IGameObject will automatically activate
		// the script. Hence, object which implements these functions need to call the base class
		// to ensure that the script is also activated. 
		//////////////////////////////////////////////////////////////////////////

		/** called when this object is attached to the scene. */
		virtual int On_Attached();
		/** called when this object is detached from the scene */
		virtual int On_Detached();

		/** when other game objects of a different type entered the sentient area of this object. 
		This function will be automatically called by the environment simulator. */
		virtual int On_EnterSentientArea();
		/** when no other game objects of different type is in the sentient area of this object. 
		This function will be automatically called by the environment simulator. */
		virtual int On_LeaveSentientArea();

		/** when the player clicked on this object.
		This function will be automatically called by the environment simulator. 
		* @param nMouseKey if this is 0, the mouse parameters will not be generated in the script handler.
		* @param dwParam1 
		* @param dwParam2 
		* @return 
		*/
		virtual int On_Click(DWORD nMouseKey, DWORD dwParam1,  DWORD dwParam2);

		/** TODO: Some game defined events, such as user attack, etc. */
		virtual int On_Event(DWORD nEventType, DWORD dwParam1,  DWORD dwParam2);

		/** when other game objects of a different type entered the perceptive area of this object. 
		This function will be automatically called by the environment simulator. */
		virtual int On_Perception();
		/** called every frame move when this character is sentient.
		* This is most likely used by active AI controllers, such as  movie controller. */
		virtual int On_FrameMove();
		/** during the execution of this object, it may send various network commands to the server or client. 
		* the network module will decide when to group these commands and send them over the network in one package. 
		* this function will be called when such network package is being prepared. */
		virtual int On_Net_Send(DWORD dwNetType, DWORD dwParam1, DWORD dwParam2);
		/** when the network module receives packages from the network and it is about a certain game object. Then this 
		* function will be automatically called. In this function, the game object may read the network packages and act 
		* accordingly. */
		virtual int On_Net_Receive(DWORD dwNetType, DWORD dwParam1, DWORD dwParam2);
	public:
		//////////////////////////////////////////////////////////////////////////
		// the following is  implemented.
		//////////////////////////////////////////////////////////////////////////

		/** load the on load script if it is not loaded. after calling this function the object will be set as loaded.
		@see SetOnLoadScript and IsLoaded() */
		void LoadOnLoadScriptIfNot();

		/** set the on load script. 
		On Load script logic: the onload script is moved from the biped object to the IGameObject. an attribute called m_bIsLoaded is added 
		as a member of the IGameObject. Besides, providing a manual function, there are several different schemes to automatically execute the on load script. 
		(1) execute it whenever the IGameObject is attached to the scene, m_bIsGlobal==true and that m_bIsLoaded == false. 
		(2) execute it whenever either onsentient, onperceive, onclick event is raised and that m_bIsLoaded == false. 
		I think the best way to do is to execute both schemes. and use the m_bIsLoaded field to choose which scheme to use in script. 
		for example, if one does not want to load a script simple set m_bIsLoaded to true, then attach it to the scene, finally set m_bIsLoaded back to false, 
		so that the on load script will still be loaded when onsentient,... event is raised. 
		@param str: it may has the same format as in the AddScriptCallback() 
		*/
		void SetOnLoadScript(const string& str);
		/** get the on load script. */
		const string& GetOnLoadScript() {return m_sOnLoadScript;};

		/**
		On Load script logic: the onload script is moved from the biped object to the IGameObject. an attribute called m_bIsLoaded is added 
		as a member of the IGameObject. Besides, providing a manual function, there are several different schemes to automatically execute the on load script. 
		(1) execute it whenever the IGameObject is attached to the scene, m_bIsGlobal==true and that m_bIsLoaded == false. 
		(2) execute it whenever either onsentient, onperceive, onclick event is raised and that m_bIsLoaded == false. 
		I think the best way to do is to execute both schemes. and use the m_bIsLoaded field to choose which scheme to use in script. 
		for example, if one does not want to load a script simple set m_bIsLoaded to true, then attach it to the scene, finally set m_bIsLoaded back to false, 
		so that the on load script will still be loaded when onsentient,... event is raised. 
		* @return true if on load script is loaded. 
		*/
		bool IsLoaded() { return m_bIsLoaded; };
		/**
		* see IsLoaded()
		* @param bLoaded 
		*/
		void SetLoaded(bool bLoaded) { m_bIsLoaded =bLoaded; };

		/** whether the biped is sentient or not*/
		virtual bool IsSentient();
		/** set the object to sentient according to whether the object is currently sentient.  It will check for duplicates in case the same sentient object is attached multiple times. 
		* this function will not work as expected when setting a always-sentient object to false and then back to true, use ForceSentient() instead.
		* @param bSentient: true to make sentient. if the object's sentient count is larger than 0, this function has no effect 
		* false, to remove the object from the sentient list.
		*/
		virtual void MakeSentient(bool bSentient=true);

		/** Force the object to be sentient or not. It will just move the character in and out of the sentient list of the scene. It also checks for duplicates in the list. 
		* so it is safe to call multiple times with the same parameters. However, this function does not modify the "always sentient" attribute of the character. 
		* @param bSentient: true to make sentient. if the object's sentient count is larger than 0, this function has no effect 
		* false, to remove the object from the sentient list.
		*/
		void ForceSentient(bool bSentient=true);

		/** get the sentient radius. usually this is much larger than the perceptive radius.*/
		float GetSentientRadius();
		/** set the sentient radius. usually this is much larger than the perceptive radius.*/
		void SetSentientRadius(float fR);

		/** get the perceptive radius. */
		float GetPerceptiveRadius();
		/** Set the perceptive radius. */
		void SetPerceptiveRadius(float fNewRaduis);

		/** return the total number of perceived objects. */
		int GetNumOfPerceivedObject();

		/** get the perceived object by index. This function may return NULL.*/
		IGameObject* GetPerceivedObject(int nIndex);

		/** get the distance square between this object and another game object*/
		float GetDistanceSq2D(IGameObject* pObj);

		/** whether the object is always sentient. The current player is always sentient */
		bool IsAlwaysSentient() const;
		/** set whether sentient. */
		void SetAlwaysSentient(bool bAlways);

		/** update the tile container according to the current position of the game object. 
		* This function is automatically called when a global object is attached. */
		virtual void UpdateTileContainer();

		virtual void SetTileContainer(CTerrainTile * pTile);

		/** is global object */
		virtual bool IsGlobal(){return m_bIsGlobal;}
		
		/** make the biped global if it is not and vice versa.*/
		virtual void MakeGlobal(bool bGlobal);

		/** Set the region within which the object can move.
		This function is not fully implemented on a per object basis. 
		@note: currently it sets the global movable region of the character. */
		virtual void SetMovableRegion(const CShapeAABB* aabb);
		/** get the region within which the object can move.*/
		virtual const CShapeAABB* GetMovableRegion();
		
		/** set the group ID to which this object belongs to. In order to be detected by other game object. 
		* Object needs to be in group 0 to 31. default value is 0*/
		void SetGroupID(int nGroup);
		/** Get the group ID to which this object belongs to. In order to be detected by other game object.*/
		int GetGroupID();

		/** set the sentient field.  A bit field of sentient object. from lower bit to higher bits, it matches to the 0-31 groups. @see SetGroupID()
		* if this is 0x0000, it will detect no objects. If this is 0xffff, it will detects all objects in any of the 32 groups. 
		* if this is 0x0001, it will only detect group 0. 
		* @param dwFieldOrGroup: this is either treated as field or group,depending on the bIsGroup parameter. 
		* @param bIsGroup: if this is true, dwFieldOrGroup is treated as a group number of which object will detect. 
		* if this is false, dwFieldOrGroup is treated as a bitwise field of which will object will detect. 
		*/
		void SetSentientField(DWORD dwFieldOrGroup, bool bIsGroup=false);
		/** @see SetSentientField*/
		DWORD GetSentientField();

		/** return true if the current object is sentient to the specified object. If the object is always sentient, this function will always return true.*/
		bool IsSentientWith(const IGameObject * pObj);

		/** whether the object is persistent in the world. If an object is persistent, it will be saved to the world's database.
		if it is not persistent it will not be saved when the world closes. Player, OPC, some temporary movie actors may 
		by non-persistent; whereas NPC are usually persistent to the world that it belongs.*/
		virtual bool IsPersistent();
		/** whenever a persistent object is made non-persistent, the SaveToDB() function will actually removed it from the database and the action can not be recovered.
		* so special caution must be given when using this function to prevent accidentally losing information. 
		@see IsPersistent() */
		virtual void SetPersistent(bool bPersistent);
		
		/** whether some of the fields are modified.It is up to the implementation class to provide this functionality if necessary. */
		virtual bool IsModified(){return m_bModified;};
		/** set whether any field has been modified. */
		virtual void SetModified(bool bModified){m_bModified = bModified;};

		/** set the animation instance's speed to dScale times of its original speed */
		virtual void SetSpeedScale(float dScale){};
		/** @see SetSpeedScale*/
		virtual float GetSpeedScale(){return 1.0f;};
		/** set the size of the object to dScale times of its original size. usually this value is 1.0 */
		virtual void SetSizeScale(float dScale){};
		/** @see SetSizeScale*/
		virtual float GetSizeScale(){return 1.0f;};

		/** return the tag served for simulation. */
		inline int GetSimTag(){return m_nSimTag;}
		/** set the tag served for simulation. */
		inline void SetSimTag(int nTag){m_nSimTag = nTag;}
		/** get the names of the game objects in the object's perceptive radius.*/
		inline std::vector <std::string>& GetPerceiveList(){return m_PerceivedList;};

		
		/** the number of objects in its sentient area. This member will be set by the environment simulator.
		* this value is 0 or negative if the object is not sentient any more*/
		inline int GetSentientObjCount(){return m_nSentientObjCount;};
		inline void SetSentientObjCount(int nCount){m_nSentientObjCount = nCount;};
		
		/** milliseconds interval, that the on frame move method should be called on a sentient object. 
		* default is 0, which means as fast as the simulator. */
		void SetFrameMoveInterval(int nFrameMoveInterval) { m_nFrameMoveInterval = nFrameMoveInterval; };
		int GetFrameMoveInterval() {return m_nFrameMoveInterval; };
		

	protected:
		// this tag is served for simulation.
		int m_nSimTag;
	private:
		/** whether the m_sOnLoadScript has been loaded. */
		bool m_bIsLoaded;
		/** whether some of the fields are modified.It is up to the implementation class to provide this functionality if necessary. */
		bool m_bModified;
		/** whether it is global */
		bool m_bIsGlobal;
		/** whether it is persistent in the world. If an object is persistent, it will be saved to the world's database.
		if it is not persistent it will not be saved when the world closes. Player, OPC, some temporary movie actors may 
		by non-persistent; whereas NPC are usually persistent to the world that it belongs.*/
		bool m_bIsPersistent;
		/** if true, the object will never be removed from the sentient object list.*/
		bool m_bAlwaysSentient;

		/** the file to be loaded when biped is loaded. */
		string		 m_sOnLoadScript;

		/** the name of the game object in the object's perceptive radius. */
		std::vector <std::string> m_PerceivedList;

		/** the number of objects in its sentient area. This member will be set by the environment simulator.
		* this value is 0 or negative if the object is not sentient any more*/
		int m_nSentientObjCount;

		/** the sentient radius. usually this is much larger than the perceptive radius.*/
		float m_fSentientRadius;

		/** the radius within which this biped could see or perceive */
		float	m_fPeceptiveRadius;
		/** the group to which this object belongs to. In order to be detected by other game object. 
		* Object needs to be in group 0 to 31. default value is 0 */
		int m_nGroup;
		/** a bit field of sentient object. from lower bit to higher bits, it matches to the 0-31 groups. @see m_dwGroup
		* if this is 0x0000, it will detect no objects. If this is 0xffff, it will detects all objects in any of the 32 groups. 
		* if this is 0x0001, it will only detect group 0. */
		DWORD m_dwSentientField;

		/** milliseconds interval, that the on frame move method should be called on a sentient object. 
		* default is 0, which means as fast as the simulator. 
		*/
		unsigned int m_nFrameMoveInterval;
		
		friend class CSceneObject;
	};

}
