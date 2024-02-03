//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2004.5
// Description:	API for 3D scene objects. 
//-----------------------------------------------------------------------------
#pragma once

#include "ParaScriptingCommon.h"
#include "ParaScriptingCharacter.h"
#include "ParaScriptingGlobal.h"
#include "MiniSceneGraph.h"

namespace ParaEngine{
	class CTerrainTile;
	class CGUIBase;
	class CBaseObject;
	class CGUIResource;
	struct GUILAYER;
	struct CGUIPosition;
	class IGameObject;
	class CMiniSceneGraph;
}

namespace luabind
{
	namespace adl{
		class object;
	}
	using adl::object;	
}

/**
* ParaScripting contains all classes and functions used for communication between the game engine and scripts.
*/
namespace ParaScripting
{
	using namespace std;
	using namespace ParaEngine;
	using namespace luabind;

	/**
	* @ingroup ParaScene
	* ParaObject class:
	* it is used to control game scene objects from scripts.
	@par Class Properties

	- ("name",&ParaObject::GetName,&ParaObject::SetName)
	- ("onclick",&ParaObject::GetOnClick,&ParaObject::SetOnClick)
	- ("onentersentientarea",&ParaObject::GetOnEnterSentientArea,&ParaObject::SetOnEnterSentientArea)
	- ("onleavesentientarea",&ParaObject::GetOnLeaveSentientArea,&ParaObject::SetOnLeaveSentientArea)
	- ("onperceived",&ParaObject::GetOnPerceived,&ParaObject::SetOnPerceived)
	- ("onframemove",&ParaObject::GetOnFrameMove,&ParaObject::SetOnFrameMove)
	- ("onnetreceive",&ParaObject::GetOnNetReceive,&ParaObject::SetOnNetReceive)
	- ("onnetsend",&ParaObject::GetOnNetSend,&ParaObject::SetOnNetSend)
	*/
	class PE_CORE_DECL ParaObject
	{
	public:
		CBaseObject::WeakPtr_type m_pObj;		// a pointer to the object
		
		ParaObject();
		ParaObject(CBaseObject* pObj);
		~ParaObject();

		CBaseObject* get() const {
			return m_pObj.get();
		};

		/** get the Runtime class information of the object. */
		const char* GetType();
		/** get paraengine defined object type name. 
		// TODO: This function is not implemented
		*/
		int GetMyType() const;

		/** the ID of the object. The ID of the object is only generated when the first time this function is called. 
		* One can then easily get the object, by calling ParaScene.GetObject(nID). When an object is released, its ID is not longer used. 
		* Please note that we can ParaScene.GetObject(nID) even if the object is never attached before.
		* Please note that the ID is neither unique outside the world, nor persistent in the same world.
		* @return: return 0 if object is invalid. 
		*/
		int GetID();

		/**
		* check if the object is valid
		*/
		bool IsValid() const;
		/** whether the object has been attached to the scene. */
		bool IsAttached() const;

		/** get the main asset object associated with this object. the object may be invalid.*/
		ParaAssetObject GetPrimaryAsset();
		/** this function shall never be called from the scripting interface. this is solely for exporting API. and should not be used from the scripting interface.*/
		void GetPrimaryAsset_(ParaAssetObject* pOut);

		/** get the parameter block of the effect (shader) associated with this object.*/
		ParaParamBlock GetEffectParamBlock();

		/** whether the object is persistent in the world. If an object is persistent, it will be saved to the world's database.
		if it is not persistent it will not be saved when the world closes. Player, OPC, some temporary movie actors may 
		by non-persistent; whereas NPC are usually persistent to the world that it belongs.*/
		bool IsPersistent();

		/**@see IsPersistent() */
		void SetPersistent(bool bPersistent);

		/** save the current character to database according to the persistent property.
		if the object is persistent, the action is to update or insert the character to db
		if the object is non-persistent, the action is to delete it from the database. */
		bool SaveToDB();

		/**
		* return true, if this object is the same as the given object.
		*/
		bool equals(const ParaObject obj) const;

		/** convert the object to object creation string.
		* @return : "" if not valid */
		const char* ToString() const;
		/** convert the object to string.
		* @param sMethod: it can be one of the following strings
		*	"create": generate script to create the object
		*	"update": generate script to update the object, useful for updating static physics object
		*	"delete": generate script to delete the object
		*	"loader": generate script to create the object in managed loader
		* @return : "" if not valid
		*/
		const char* ToString1(const char* sMethod) const;

		/** get the attribute object associated with an object. */
		ParaAttributeObject GetAttributeObject();
		/** for API exportation*/
		void GetAttributeObject_(ParaAttributeObject& output);
		/**
		* when this function is called, it ensures that the physics object around this object is properly loaded.
		* It increases the hit count of these physics objects by 1. The garbage collector in the physics world
		* may use the hit count to move out unused static physics object from the physics scene (Novodex).
		* This function might be called for the current player, each active mobile object in the scene and the camera eye position.
		* vCenter: the center of the object in world coordinates 
		* fRadius: the radius of the object within which all physics object must be active.
		*/
		void CheckLoadPhysics();

		/** only load physics of this object if it has not loaded. */
		void LoadPhysics();

		/** get field by name.
		e.g. suppose att is the attribute object.
			local bGloble = att:GetField("global", true);
			local facing = att:GetField("facing", 0);
			local pos = att:GetField("position", {0,0,0});
			pos[1] = pos[1]+100;pos[2] = 0;pos[3] = 10;

		@param sFieldname: field name
		@param output: default value. if field type is vectorN, output is a table with N items.
		@return: return the field result. If field not found, output will be returned. 
			if field type is vectorN, return a table with N items.Please note table index start from 1
		*/
		object GetField(const char*  sFieldname, const object& output);

		/** set field by name 
		e.g. suppose att is the attribute object.
			att:SetField("facing", 3.14);
			att:SetField("position", {100,0,0});
		@param sFieldname: field name
		@param input: input value. if field type is vectorN, input is a table with N items.*/
		void SetField(const char*  sFieldname, const object& input);

		/** call field by name. This function is only valid when The field type is void.
		It simply calls the function associated with the field name. */
		void CallField(const char*  sFieldname);

		/** get field by name.
		e.g. suppose att is the attribute object.
		local bGloble = att:GetField("URL", nil);
		local facing = att:GetField("Title", "default one");
		
		@param sFieldname: field name
		@param output: default value. if field type is vectorN, output is a table with N items.
		@return: return the field result. If field not found, output will be returned. 
		if field type is vectorN, return a table with N items.Please note table index start from 1
		*/
		object GetDynamicField(const char*  sFieldname, const object& output);

		/** set field by name 
		e.g. suppose att is the attribute object.
		att:SetDynamicField("URL", 3.14);
		att:SetDynamicField("Title", {100,0,0});
		@param sFieldname: field name
		@param input: input value. can be value or string type*/
		void SetDynamicField(const char*  sFieldname, const object& input);

		/** get object by name, if there are multiple objects with the same name, the last added one is inserted.
		* @note: This function will traverse the scene to search the object. So there might be some performance penalty.
		* @param name:
		*/
		ParaObject GetObject(const char* name);
	public:

		/** whether the object has 0 speed.*/
		bool IsStanding();

		/** invisible object will not be drawn. e.g. one can turn off the visibility of physics object. */
		bool IsVisible();

		/** set the visibility of this object. The visibility will recursively affect all its child objects. */
		void SetVisible(bool bVisible);

		/**
		* whether an object attribute is enabled. 
		* @param attribute 
		* object volume bit fields
		enum OBJECT_ATTRIBUTE
		{
			/// two solid objects with sensor volume will cause environment simulator to
			/// to generate sensor/collision event when they come in to contact.
			OBJ_VOLUMN_SENSOR = 1,
			/// all child objects are in this object's volume 
			OBJ_VOLUMN_CONTAINER = 0x1<<1,
			/// solid objects(like biped) can be placed on its volume, provided 
			/// it's not already occupied by any solid objects from its children
			/// when we solve two solid object collision, this is the field we check first.
			OBJ_VOLUMN_FREESPACE = 0x1<<2,
			/// whether the object is isolated from its siblings. An isolated object
			/// can overlap in physical space with all its siblings regardless of their solidity.
			/// multiple scenes or terrains can be declared as ISOLATED object. Note, the object
			/// is not isolated from its parent, though.
			OBJ_VOLUMN_ISOLATED = 0x1<<3,
			/// the object has a perceptive radius that may be larger than the object's 
			/// collision radius. Currently only biped object might has this volume type
			OBJ_VOLUMN_PERCEPTIVE_RADIUS = 0x1<<4,
			/// objects with this VIP volume type will trigger the plot of the scene in its view-culling radius.
			OBJ_VOLUMN_VIP = 0x1<<5,
			/// Object invisible, the object is not drawn.but its physics may load. added by lxz 2006.3.5
			OBJ_VOLUMN_INVISIBLE = 0x1<<6,
			/// mask of the above bits. this field is never used externally.
			VOLUMN_MASK = 0x7f,
			/// whether lights have effects on this object.
			MESH_USE_LIGHT = 0x1<<7,
			/// whether to rotate the object around Y axis to let the object always facing the camera.
			MESH_BILLBOARDED= 0x1<<8,
			/// whether it is a shadow receiver.
			MESH_SHADOW_RECEIVER= 0x1<<9,
			/// whether it is a vegetation.
			MESH_VEGETATION= 0x1<<10,
		};
		* @return true if enabled
		*/
		bool CheckAttribute(DWORD attribute);

		/**
		* enable or disable a given attribute.

		Below are some attributes. For more information please see BaseObject.h

		/// two solid objects with sensor volume will cause environment simulator to
		/// to generate sensor/collision event when they come in to contact.
		OBJ_VOLUMN_SENSOR = 1,
		/// all child objects are in this object's volume 
		OBJ_VOLUMN_CONTAINER = 0x1<<1,
		/// solid objects(like biped) can be placed on its volume, provided 
		/// it's not already occupied by any solid objects from its children
		/// when we solve two solid object collision, this is the field we check first.
		OBJ_VOLUMN_FREESPACE = 0x1<<2,
		/// whether the object is isolated from its siblings. An isolated object
		/// can overlap in physical space with all its siblings regardless of their solidity.
		/// multiple scenes or terrains can be declared as ISOLATED object. Note, the object
		/// is not isolated from its parent, though.
		OBJ_VOLUMN_ISOLATED = 0x1<<3,
		/// the object has a perceptive radius that may be larger than the object's 
		/// collision radius. Currently only biped object might has this volume type
		OBJ_VOLUMN_PERCEPTIVE_RADIUS = 0x1<<4,
		/// objects with this VIP volume type will trigger the plot of the scene in its view-culling radius.
		OBJ_VOLUMN_VIP = 0x1<<5,
		/// Object invisible, the object is not drawn.but its physics may load. added by lxz 2006.3.5
		OBJ_VOLUMN_INVISIBLE = 0x1<<6,

		* @param dwAtt 
		* @param bTurnOn: true to turn on, false to turn off. 
		*/
		void SetAttribute(DWORD dwAtt, bool bTurnOn);

		/**
		* set world position. Please note, for static object, it may make the quad tree terrain in which the object is located invalid. 
		* it may also make the physics engine panic.In such cases, one should call ParaScene.Attach() after chancing the position or rotation 
		* of a static mesh or physics object. 
		* If any of the following rule matches, the function is safe to use.
		* -  Use this function for global biped if (x,y,z) changes.
		* -  Use this function for all objects if it has not been attached to the global terrain
		* -  Use this function for static mesh, with (0,y,0) only.i.e. only changing height.
		* -  Never use this function for physics object, unless you are building the world. If you do use it 
		*    it with physics or static mesh object, make sure that you follow the following rules:
		*		-- create the physics object and save it in a global variable called physicsObj.
		*		physicsObj = ParaScene.Attach(physicsObj); -- attach the physics object to the scene
		*		-- modification of the position, orientation, scaling of the object occurred. 
		*		local x,y,z = physicsObj:GetPosition(); physicsObj:SetPosition(x+2,y,z);
		*		-- immediately call Attach again with physicsObj, so that the physical scene will be updated and the object be re-inserted properly in to the scene.
		*		physicsObj = ParaScene.Attach(physicsObj); 
		* @param x: global x
		* @param y: global y
		* @param z: global z
		*/
		void SetPosition(double x, double y, double z);
		/**
		* get the world position of the object. This function takes no parameters.
		* x,y,z are not input, but pure output. In the script, we can call it as below
		*	x,y,z = biped:GetPosition(); -- get the biped's position
		* in Luabind, it is defined as 
		*	.def("GetPosition", &ParaObject::GetPosition, pure_out_value(_2) + pure_out_value(_3) + pure_out_value(_4))
		* please note, y is the absolute position in world coordinate
		* @see SetPosition(double x, double y, double z)
		*/
		void GetPosition(double *x, double *y, double *z);
		/**
		* get the world position of the center of the view object. This function takes no parameters.
		* x,y,z are not input, but pure output. In the script, we can call it as below
		*	x,y,z = biped:GetViewCenter(); -- get the biped's center
		*/
		void GetViewCenter(double *x, double *y, double *z);
		
		/**
		* offset the current object position by (dx,dy,dz)
		* @see SetPosition(float x, float y, float z) for precautions of using this function
		*/
		void OffsetPosition (float dx, float dy, float dz);
		/** set object facing around the Y axis.
		* this function is safe to call for all kind of objects except the physics mesh object. 
		* for physics mesh object, one must call ParaScene.Attach() immediately after this function.
		* for more information, please see SetPostion();
		* @see: SetPostion();
		*/
		void SetFacing(float fFacing);
		/** get object facing around the Y axis	*/
		float GetFacing();

		/** Rotate the object.This only takes effects on objects having 3D orientation, such as
		* static mesh and physics mesh. The orientation is computed in the following way: first rotate around x axis, 
		* then around y, finally z axis.
		* Note: this function is safe to call for all kind of objects except the physics mesh object. 
		* for physics mesh object, one must call ParaScene.Attach() immediately after this function.
		* for more information, please see SetPostion();
		* @param x: rotation around the x axis.
		* @param y: rotation around the y axis.
		* @param z: rotation around the z axis.
		* @see: SetPostion();
		*/
		void Rotate(float x, float y, float z);

		/** set the scale of the object. This function takes effects on both character object and mesh object. 
		* Note: this function is safe to call for all kind of objects except the physics mesh object. 
		* for physics mesh object, one must call ParaScene.Attach() immediately after this function.
		* for more information, please see SetPostion();
		* @param s: This is a relative scale to its current size. Scaling applied to all axis.1.0 means original size. 
		* @see: SetPostion();
		*/
		void Scale(float s);
		void SetScaling(float s);

		/** set the scale of the object. This function takes effects on both character object and mesh object. 
		* Note: this function is safe to call for all kind of objects except the physics mesh object. 
		* for physics mesh object, one must call ParaScene.Attach() immediately after this function.
		* for more information, please see SetPostion();
		* @param s: this is the absolute scale on the original mesh model. Scaling applied to all axis.1.0 means original size. 
		*/
		float GetScale();
		/**
		* set scale @see GetScale();
		* @param s 
		*/
		void SetScale(float s);

		/**
		* this usually applies only to mesh object.
		* get the rotation as quaternion. e.g. local mat3x3 = obj:GetRotation({});
		* @return the rotational matrix is of the following format: {x,y,z,w,}
		*/
		object GetRotation(const object& quat);

		/**
		* set the rotation as quaternion.
		* @param sRot the rotational matrix is of the following format: {x,y,z,w,}
		*/
		void SetRotation(const object& quat);

		/** reset the object to its default settings.*/
		void Reset();

		/** set the physics group ID to which this object belongs to
		default to 0, must be smaller than 32. 
		please see groups Mask used to filter shape objects. See #NxShape::setGroup
		- group 0 means physics object that will block the camera and player, such as building walls, big tree trunks, etc. 
		- group 1 means physics object that will block the player, but not the camera, such as small stones, thin posts, trees, etc. 
		*/
		void SetPhysicsGroup(int nGroup);

		/** Get the physics group ID to which this object belongs to
		default to 0, must be smaller than 32. 
		please see groups Mask used to filter shape objects. See #NxShape::setGroup
		- group 0 means physics object that will block the camera and player, such as building walls, big tree trunks, etc. 
		- group 1 means physics object that will block the player, but not the camera, such as small stones, thin posts, trees, etc. 
		*/
		int GetPhysicsGroup();


		/** set the selection group index. if -1, it means that it was not selected. 
		* this function is equivalent to calling ParaSelection.AddObject(obj, nGroupID);
		* @param nGroupIndex: selection group index. 
		*/
		void SetSelectGroupIndex(int nGroupIndex);

		/** get the selection group index. if -1, it means that it was not selected. */
		int GetSelectGroupIndex();

		/** body density. The water density is always 1.0 in the game engine. 
		So if it is above 1.0, it will sink in water; otherwise it will float on water surface. 
		So if it is below 0.5, it will fly or glind in air falling down with little gravity; otherwise it will fall down with full gravity. 
		A density of 0 or negative, means that the character can fly. The default value is 1.2. the following are some examples
		- character:	body density: 1.2		
		- car:			body density: 2.0	mount target
		- ship:			body density: 0.8	mount target
		- plane:		body density: 0.4	mount target
		* @param fDensity 
		*/
		void SetDensity(float fDensity);
		/** get body density */
		float GetDensity();

		/** the biped is modeled as a cylinder or sphere during rough physics calculation.
		* this function returns the radius of the cylinder or sphere.*/
		float		GetPhysicsRadius();
		/** the biped is modeled as a cylinder or sphere during rough physics calculation.
		* this function set the radius of the cylinder or sphere.*/
		void		SetPhysicsRadius(float fR);

		/** the biped is modeled as a cylinder or sphere during rough physics calculation.
		* this function returns the height of the cylinder or sphere.
		* this value will also restrict the biped's vertical movement.
		* if there is no head attachment, the GetPhysicsHeight is used for character head on text position instead. 
		* If PhysicsHeight is never set, it is always 4*PhysicsRadius. However, if it is set, its value are maintained. 
		*/
		float		GetPhysicsHeight();

		/** the biped is modeled as a cylinder or sphere during rough physics calculation.
		* this function set the height of the cylinder or sphere.
		* this value will also restrict the biped's vertical movement.
		* if there is no head attachment, the GetPhysicsHeight is used for character head on text position instead. 
		* If PhysicsHeight is never set, it is always 4*PhysicsRadius. However, if it is set, its value are maintained. 
		*/
		void		SetPhysicsHeight(float fHeight);

		/** get the object name */
		string GetName() const;
		/** for .NET API use only.not thread safe. */
		const char* GetName_() const;

		/** set the object name 
		* @remark: currently, renaming an object after attaching it to the scene is dangerous, because when people use the ParaScene.GetObject(), it may return a renamed and deleted object. 
		* The good practice is that never change an object's name when it is attached to the scene. 
		* @remark: In version 0.9 or above, renaming an object will detach the object from the scene and then reattach it to the scene using the new name; 
		* so it works fine when changing object name after attachment.
		* @remark: this function will not take effect, if the object can not be renamed, such as due to another global object with the same name.
		* @param sName: new name of the object
		*/
		void SetName(const char* sName);

		/**
		* snap to terrain surface. Such as landscape trees, stones, etc. 
		* set bUseNorm to 1, if you want the norm to be aligned.
		*/
		void SnapToTerrainSurface(int bUseNorm);
		/**
		* @return return true if object is a character(biped) object
		*/
		bool IsCharacter() const;
		/**
		* @return return true if object is a network player
		*/
		bool IsOPC() const;

		/**
		* return the ParaCharacter object, if this object is a biped typed scene object. 
		* the ParaCharacter interface offers more specific functions to control the behavior 
		* and appearance of the character object.
		* @see ParaCharacter
		*/
		ParaCharacter ToCharacter();

		/**
		* convert to game object. This function may return NULL. 
		*/
		IGameObject* ToGameObject();

		/**
		* add events to object to control the object, please refer to HLE for object 
		* event specifications. Generally, one can use event to tell a biped to walk to a new position
		* play a certain animation, speak some words, follow another biped, attack, etc.
		* @param strEvent: currently a biped object accepts the following events.
		A list of events (more will be coming in future release):\n
		- "stop"\n
			Stop the biped

		- "walk x y"\n
			Walk to (x,y) using the default animation

		- "colr r, g, b"\n	
			Set model color[0, 1]\n
			e.g. colr 1.0 0 0    =  to set red color

		- "anim string | number"\n	
			Play animation 

		- "asai type parameters"\n
			Assign AI module of type type to the biped. Old modules are discarded Currently only support 
			creature type AI module, with one parameter which can be ranged or melee unit.\n
			type:\n
			AIModuleCreatures=1, // respawning creatures that may be neural,or aggressive. They will not attack bipeds of any type, but may attack any team bipeds.\n
			AIModuleTeam=2,		 // Player Controllable biped AI module. it recognize creatures and members in other teams by their IDs.\n
			"asai 1 1"		Assign creature AI with ranged unit attribute\n
			"asai 1 0"		Assign creature AI with melee unit attribute\n

		- "ClearAll"\n
			Clear all tasks that this biped is assigned.

		- "task  taskType parameters"\n
			Assign task to the AImodule associated with the biped.\n
			taskType: \n
				Currently taskType should be string, not their number\n
				- DieAndReborn = 0, 
				- WanderNearby = 1,	
				- Evade=2, 	"task Evade name"\n
							name: which biped to evade\n
							e.g. "task Evade enemy1"
				- Follow=3, 	"task Follow name"\n
							name: which biped to follow\n
							e.g. "task Follow LiXizhi"
				- Movie=4,	"task Movie string"\n
							string: list of key, no spaces in the str are allowed. Format is given below\n
							[<const char* anim, float x, float y, float z, float facing, float duration>]+ \n
							If y=0, then the height of the position is ignored. Duration is in seconds.\n
							e.g. "task Movie <attack-1,50,0,60,0,10><Die-1,50,0,60,0,0>"
		* @param nEventType: default to 0
		* @param bIsUnique: is it unique by string
		*/
		void AddEvent(const char* strEvent, int nEventType, bool bIsUnique);

		/**
		* attach the object as its child object.
		* @param obj: the child object to attach 
		*/
		void AddChild(const ParaObject obj);

		/** if this is a physics mesh object, this function will turn on or off the physics of the object. */
		void EnablePhysics(bool bEnable);

		/** whether this is a physics mesh object */
		bool IsPhysicsEnabled();

		/** get the distance with another object */
		float DistanceTo(ParaObject obj);
		/** get the distance square with another object */
		float DistanceToSq(ParaObject obj);
		/** get the distance square with the current player object */
		float DistanceToPlayerSq();
		/** get the distance square with the camera object */
		float DistanceToCameraSq();
		
		/** get the view box. 
		* e.g. local box = obj:GetViewBox({});
		* log(box.pos_x..box.pos_y..box.pos_z);
		* return a table containing the following field:{pos_x, pos_y,pos_z,obb_x,obb_y,obb_z,}
		* pos_x, pos_y,pos_z: is the point at the bottom center of the box. 
		* obb_x,obb_y,obb_z: is the size of the box.
		*/
		object GetViewBox(const object& output);
		
		/** return whether this model has a given attachment point 
		* @param nAttachmentID: see ATTACHMENT_ID. default to 0, which is general mount point. ATT_ID_MOUNT1-9(20-28) is another mount points
		*/
		bool HasAttachmentPoint(int nAttachmentID);

		/** return this object's attachment point 
		* e.g. local x,y,z = obj:GetAttachmentPosition(0);
		* @param nAttachmentID: see ATTACHMENT_ID. default to 0, which is general mount point. ATT_ID_MOUNT1-9(20-28) is another mount points
		* @return x,y,z: in world coordinates
		*/
		void GetAttachmentPosition(int nAttachmentID, float * x, float * y, float *z);

		/** set the home zone of this object if any. it may return NULL, if zone is not visible.
		* @param sHomeZone: if this is NULL, it will remove the zone. 
		*/
		void SetHomeZone(const char* sHomeZone);

		/** get the home zone of this object if any. it may return NULL or "", if zone is not visible.*/
		const char* GetHomeZone();

		//////////////////////////////////////////////////////////////////////////
		//
		// IHeadOn3D object
		//
		//////////////////////////////////////////////////////////////////////////

		/** set the text to be displayed on head on display*/
		void SetHeadOnText(const char* sText, int nIndex);

		/** Get the text to be displayed on head on display*/
		const char* GetHeadOnText(int nIndex);


		/** set which UI control the head on display will be used as a template for drawing the text
		* it can be a single CGUIText Object or it can be a container with a direct children called "text"
		* if this is "" or empty, the default UI template will be used. The default UI template is an invisible CGUIText control called "_HeadOnDisplayText_"
		* By default, "_HeadOnDisplayText_" uses horizontal text alignment and system font.
		*/
		void SetHeadOnUITemplateName(const char* sUIName, int nIndex);

		/** get which UI control the head on display will be used as a template for drawing the text
		* it can be a single CGUIText Object or it can be a container with a direct children called "text"
		* if this is "" or empty, the default UI template will be used. The default UI template is an invisible CGUIText control called "_HeadOnDisplayText_"
		* By default, "_HeadOnDisplayText_" uses horizontal text alignment and system font.
		* @return: it returns NULL if no UI head on display.
		*/
		const char* GetHeadOnUITemplateName(int nIndex);

		/** set the text to be displayed on head on display
		* @param color: "r g b" or "r g b a",  such as "0 255 0", "0 255 0 255"
		*/
		void SetHeadOnTextColor(const char* color, int nIndex);
		
		/** set the offset where head on display should be rendered relative to the origin or head of the host 3d object*/
		void SetHeadOnOffest(float x, float y, float z, int nIndex);

		/** Get the offset where head on display should be rendered relative to the origin or head of the host 3d object*/
		void GetHeadOnOffset(int nIndex, float* x, float* y, float* z);

		/** show or hide object's head on display*/
		void ShowHeadOnDisplay(bool bShow, int nIndex);

		/** whether the object head on display shall be visible*/
		bool IsHeadOnDisplayShown(int nIndex);

		/** whether the object contains head on display */
		bool HasHeadOnDisplay(int nIndex);

		//////////////////////////////////////////////////////////////////////////
		//
		// x reference script objects
		//
		//////////////////////////////////////////////////////////////////////////

		/** get the number of the script X reference instances*/
		int GetXRefScriptCount();

		/** return xref script file path by index */
		const char* GetXRefScript(int nIndex);

		/**
		* get the 3D position in world space of the script object's origin
		*/
		void GetXRefScriptPosition(int nIndex, float *x, float *y, float *z);

		/**
		* get the scaling of the object in both x,y,z directions
		*/
		void GetXRefScriptScaling(int nIndex, float *x, float *y, float *z);

		/**
		* get the facing of the object in xz plane
		*/
		float GetXRefScriptFacing(int nIndex);

		/**
		* get the local transform of the script object. It contains scale and rotation only
		* the string returned by this function must be used at once and should not be saved. This function is also not thread-safe. 
		* @return: "11,12,13, 21,22,23, 31,32,33,41,42,43" ,where 41,42,43 are always 0. It may return NULL, if the local matrix does not exist.
		*/
		const char* GetXRefScriptLocalMatrix(int nIndex);

	public:
		//////////////////////////////////////////////////////////////////////////
		// game object functions
		//////////////////////////////////////////////////////////////////////////
		/** whether the biped is sentient or not*/
		bool IsSentient();

		/** get the sentient radius. usually this is much larger than the perceptive radius.*/
		float GetSentientRadius();

		/** get the perceptive radius. */
		float GetPerceptiveRadius();
		/** Set the perceptive radius. */
		void SetPerceptiveRadius(float fNewRaduis);

		/** return the total number of perceived objects. */
		int GetNumOfPerceivedObject();

		/** get the perceived object by index. This function may return NULL.*/
		ParaObject GetPerceivedObject(int nIndex);
		
		/** whether the object is always sentient. The current player is always sentient */
		bool IsAlwaysSentient();

		/** set whether sentient. */
		void SetAlwaysSentient(bool bAlways);

		/** set the object to sentient. 
		* @param bSentient: true to make sentient. if the object's sentient count is larger than 0, this function has no effect 
		* false, to remove the object from the sentient list.
		*/
		void MakeSentient(bool bSentient);

		/** update the tile container according to the current position of the game object. 
		* This function is automatically called when a global object is attached. */
		void UpdateTileContainer();

		/** make the biped global if it is not and vice versa.*/
		void MakeGlobal(bool bGlobal);

		/** whether the object is global or not.*/
		bool IsGlobal();

		/** set the group ID to which this object belongs to. In order to be detected by other game object. 
		* Object needs to be in group 0 to 31. default value is 0*/
		void SetGroupID(int nGroup);

		/** set the sentient field.  A bit field of sentient object. from lower bit to higher bits, it matches to the 0-31 groups. @see SetGroupID()
		* if this is 0x0000, it will detect no objects. If this is 0xffff, it will detects all objects in any of the 32 groups. 
		* if this is 0x0001, it will only detect group 0. 
		* @param dwFieldOrGroup: this is either treated as field or group,depending on the bIsGroup parameter. 
		* @param bIsGroup: if this is true, dwFieldOrGroup is treated as a group number of which will object will detect. 
		* if this is false, dwFieldOrGroup is treated as a bitwise field of which will object will detect. 
		*/
		void SetSentientField(DWORD dwFieldOrGroup, bool bIsGroup);

		/** return true if the current object is sentient to the specified object. If the object is always sentient, this function will always return true.*/
		bool IsSentientWith(const ParaObject& pObj);

		/** Set the region within which the object can move.
		This function is not fully implemented on a per object basis. 
		@note: currently it sets the global movable region of the character. */
		void SetMovableRegion(float center_x, float center_y, float center_z, float extent_x, float extent_y, float extent_z);
		/**
		* get the region within which the object can move.. This function takes no parameters.
		* input are not input, but pure output. In the script, we can call it as below
		*	cx,cy,cz,ex,ey,ez = biped:GetMovableRegion(); -- get the biped's position
		* please note, y is the absolute position in world coordinate
		* @see SetMovableRegion()
		*/
		void GetMovableRegion(float* center_x, float* center_y, float* center_z, float* extent_x, float* extent_y, float* extent_z);

		/** Set the current animation id
		* @param nAnimID: 0 is default standing animation. 4 is walking, 5 is running. more information, please see AnimationID */
		void SetAnimation(int nAnimID);

		/** get the scaling. */
		int GetAnimation();

		//////////////////////////////////////////////////////////////////////////
		// call back script functions 
		//////////////////////////////////////////////////////////////////////////
		/** when other game objects of a different type entered the sentient area of this object. 
		This function will be automatically called by the environment simulator. */
		string GetOnEnterSentientArea() const;
		void SetOnEnterSentientArea(const char* script);
		
		/** when no other game objects of different type is in the sentient area of this object. 
		This function will be automatically called by the environment simulator. */
		string GetOnLeaveSentientArea() const;
		void SetOnLeaveSentientArea(const char* script);

		/** when the player clicked on this object.
		This function will be automatically called by the environment simulator. */
		string GetOnClick() const;
		void SetOnClick(const char* script);
		/** activate the OnClick */
		void On_Click(DWORD nMouseKey, DWORD dwParam1,  DWORD dwParam2);
		
		/** when other game objects of a different type entered the perceptive area of this object. 
		This function will be automatically called by the environment simulator. */
		string GetOnPerceived() const;
		void SetOnPerceived(const char* script);

		/** called every frame move when this character is sentient.
		* This is most likely used by active AI controllers, such as  movie controller. */
		string GetOnFrameMove() const;
		void SetOnFrameMove(const char* script);

		/** during the execution of this object, it may send various network commands to the server or client. 
		* the network module will decide when to group these commands and send them over the network in one package. 
		* this function will be called when such network package is being prepared. */
		string GetOnNetSend() const;
		void SetOnNetSend(const char* script);

		/** when the network module receives packages from the network and it is about a certain game object. Then this 
		* function will be automatically called. In this function, the game object may read the network packages and act 
		* accordingly. */
		string GetOnNetReceive() const;
		void SetOnNetReceive(const char* script);

		//////////////////////////////////////////////////////////////////////////
		// 
		// some effect parameters
		// 
		//////////////////////////////////////////////////////////////////////////

		/** Get the current shader handle used to render the object
		* @see TechniqueHandle
		*/
		int GetEffectHandle();

		/**
		* Set the shader handle used to render the object. Please note, object will be immediately rendered using the newly assigned shader in the next frame.
		* shade handle in the range [0,2048] is reserved for ParaEngine's internal usage.
		* CAUTION: setting a shader whose input is incompatible with the object's internal data presentation will cause the application to close.
		* @param nHandle: @see TechniqueHandle
		*/
		void SetEffectHandle(int nHandle);

		//////////////////////////////////////////////////////////////////////////
		//
		// Reference object
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		* add a new reference. 
		* @param maker 
		* @return 
		*/
		int AddReference(const ParaObject& maker, int nTag);

		/**
		* delete a reference.
		* @param ref 
		* @return return REF_FAIL if reference not found. otherwise REF_SUCCEED
		*/
		int DeleteReference(const ParaObject&  ref);

		/** Deletes all references of this object.*/
		int DeleteAllRefs();

		/** get the total number of references */
		int GetRefObjNum();

		/** get the referenced object at the given index */
		ParaObject GetRefObject(int nIndex);

		/** get primary texture object */
		ParaAssetObject GetTexture();

		bool Export(const char* filepath, const char* typ = "gltf");

		//////////////////////////////////////////////////////////////////////////
		//
		// mesh and physics mesh function object
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		* get the total number of replaceable textures, which is the largest replaceable texture ID. 
		* but it does not mean that all ID contains valid replaceable textures.  
		* This function can be used to quickly decide whether the model contains replaceable textures. 
		* Generally we allow 32 replaceable textures per model.
		* @note: This function will cause the mesh entity to be initialized.
		* @return 0 may be returned if no replaceable texture is used by the model.
		*/
		int GetNumReplaceableTextures();

		/**
		* get the default replaceable texture by its ID. 
		* The default replaceable texture is the main texture exported from the 3dsmax exporter.
		* @note: This function will cause the mesh entity to be initialized.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @return this may return NULL, if replaceable texture is not set before or ID is invalid.
		*/
		ParaAssetObject GetDefaultReplaceableTexture(int ReplaceableTextureID); 

		/**
		* get the current replaceable texture by its ID.
		* if no replaceable textures is set before, this will return the same result as GetNumReplaceableTextures(). 
		* @note: This function will cause the mesh entity to be initialized.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @return this may return NULL, if replaceable texture is not set before or ID is invalid.
		*/
		ParaAssetObject GetReplaceableTexture(int ReplaceableTextureID); 

		/**
		* set the replaceable texture at the given index with a new texture. 
		* this function will succeed regardless whether the mesh is initialized. Hence it can be used at loading time. 
		* because default instance of the mesh may use different replaceable texture set. 
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @param pTextureEntity The reference account of the texture entity will be automatically increased by one. 
		* @return true if succeed. if ReplaceableTextureID exceed the total number of replaceable textures, this function will return false.
		*/
		bool  SetReplaceableTexture(int ReplaceableTextureID, ParaAssetObject pTextureEntity);
	};

	
	/**
	* @ingroup ParaScene
	* ParaMiniSceneGraph class:
	*/
	class PE_CORE_DECL ParaMiniSceneGraph
	{
	public:
		CMiniSceneGraph::WeakPtr_type m_pSceneGraph;
		ParaMiniSceneGraph(){};
		ParaMiniSceneGraph(CMiniSceneGraph* pGraph):m_pSceneGraph(pGraph){};
		~ParaMiniSceneGraph();
	public:
		/** get name */
		const char* GetName();

		/** set the object name. this function can be used to rename this object */
		void SetName(const char* sName);

		/**
		* check if the object is valid
		*/
		bool IsValid() const {return m_pSceneGraph;};

		/** get the attribute object associated with the global scene. */
		ParaAttributeObject GetAttributeObject();
		/** get the attribute object associated with the global scene. 
		* @param name: "" for global scene, "sky" for sky, "camera" for camera, "sun" for sun. 
		*/
		ParaAttributeObject GetAttributeObject1(const char* name);

		/** used for API exportation. */
		void GetAttributeObject_(ParaAttributeObject& output);
		
		/** get the attribute object associated with the current camera object. */
		ParaAttributeObject GetAttributeObjectCamera();

		/**
		* create a sky box and add it to the current list. sky box with the same name 
		* will not be recreated,but will be selected as the current sky box. It may be a sky box/dome/plane or whatever.
		* The associated mesh will be scaled by the specified amount along x,y,z axis and then 
		* translate up or down along the y axis. 
		* in many cases, the mesh data in the mesh asset is of unit size. 
		* @param strObjectName: sky name
		* @param strMeshAssetName: mesh asset name. this is not the file name.
		* @param fScaleX: the static mesh local transform scale along the x axis
		* @param fScaleY: the static mesh local transform scale along the y axis
		* @param fScaleZ: the static mesh local transform scale along the z axis
		* @param fHeightOffset: the translation along the y axis. 
		*/
		bool CreateSkyBox(const char* strObjectName, const char* strMeshAssetName, float fScaleX, float fScaleY,float fScaleZ, float fHeightOffset);
		bool CreateSkyBox_(const char* strObjectName, ParaAssetObject& asset, float fScaleX, float fScaleY,float fScaleZ, float fHeightOffset);
		/** delete a name sky box. 
		* @param strObjectName: if this is "", all sky boxes will be deleted.*/
		void DeleteSkyBox(const char* strObjectName);

		/** Enable both global and local lighting. Turn off lighting will greatly improve performance, such as on slower computers*/
		void EnableLighting(bool bEnable);
		bool IsLightingEnabled();

		/** set standard time. see SetTimeOfDay()
		*@param time: always in the range [-1,1], 0 means at noon, -1 is morning. 1 is night. */
		void SetTimeOfDaySTD(float time);

		/** get standard time. see GetTimeOfDay()
		*@return: always in the range [-1,1], 0 means at noon, -1 is morning. 1 is night. */
		float GetTimeOfDaySTD();

		/**
		* set the global fog effect
		* @param bRenderFog: 1 to enable fog.
		* @param strFogColor: a string of RGB value in the format "%f %f %f", such as "1.0 1.0 1.0", value must be in the range [0, 1.0].
		* @param fFogDensity: between (0,1)
		* @param fFogStart: unit in meters. 
		* @param fFogEnd: unit in meters. 
		*/
		void SetFog(bool bRenderFog, const char* strFogColor, float fFogStart,float fFogEnd, float fFogDensity);

		/** invisible object will not be drawn. e.g. one can turn off the visibility of physics object. */
		bool IsVisible();


		/** set the visibility of this object. The visibility will recursively affect all its child objects. */
		void SetVisible(bool bVisible);

		/** enable or disable a given camera */
		void EnableCamera(bool bEnable);

		/** whether camera is enabled. 
		* it is disabled by default. it is much faster to disable camera, because it will use the main scene's render pipeline and effects. 
		* Otherwise it will be rendered after the main scene is rendered, since the camera is different.
		* TODO: currently mini scene graph is only rendered when its camera is disabled. local camera is not supported at the moment
		*/
		bool IsCameraEnabled();

		/** update the camera parameter by providing the lookat and eye position */
		//void UpdateCameraParam(float lookat_x, float lookat_y, float lookat_z, float eyeat_x, float eyeat_y, float eyeat_z);

		/** get the camera parameter of the lookat and eye position 
		* in scripting interface all inputs are outputs.
		*/
		//void GetCameraParam(float* lookat_x, float* lookat_y, float* lookat_z, float* eyeat_x, float* eyeat_y, float* eyeat_z);

		/** get object by name, if there are multiple objects with the same name, the last added one is inserted. 
		* @note: This function will traverse the scene to search the object. So there might be some performance penalty. 
		* @param name: 
		*/
		ParaObject GetObject(const char* name);

		/** get the first object that matches the position. EPSILON is 0.01f*/
		ParaObject GetObject3(float x, float y, float z);
		
		/** get the first object that matches the position within fEpsilon, which is usually 0.01f*/
		ParaObject GetObject4(float x, float y, float z, float fEpsilon);

		/** remove an object from this scene graph but do not destroy it.
		* this function can be used to move a node from one scene graph to another */
		int RemoveObject(const char* name);

		/** remove an object from this scene graph but do not destroy it.
		* this function can be used to move a node from one scene graph to another */
		int RemoveObject_(const ParaObject& pObj);

		/**
		* attach the object as its child object.
		* @param obj: the child object to attach 
		*/
		void AddChild(const ParaObject obj);

		/** destroy all objects with the given name. 
		* the current version will only destroy the first met child with the given name. 
		* @return the number of objects deleted. 
		*/
		int DestroyObject(const char* name);

		/** delete an object from this scene graph but do not destroy it.This function will search the scene recursively
		* this function can be used to move a node from one scene graph to another 
		* Note: this is like calling RemoveObject(pObj) and then delete the object. 
		* @param pObj: object to delete. 
		*/
		int DestroyObject_(const ParaObject& pObj);

		/** destroy all children of this mini-scenegraph. but still preserving other settings like camera and render target. */
		void DestroyChildren();

		/** clear the entire scene graph */
		void Reset();


		/**
		* set the actor: The camera always focuses on actor, so 
		* this actor can be used to control the current camera position.
		* @param pActor: it must be a valid object. 
		*/
		void SetActor(const ParaObject pActor);

		/**get the current actor */
		ParaObject GetActor();

		/**
		* the canvas texture, which can be used as any other ordinary texture on 3D or 2D object.
		* @return 
		*/
		ParaAssetObject GetTexture();


		/**
		* Zoom the camera
		* @param fAmount 
		*/
		void CameraZoom(float fAmount);

		/**
		* reset the camera parameters to view the entire sphere at best (default) distance
		* @param center_x: sphere center x
		* @param center_y: sphere center y
		* @param center_z: sphere center z
		* @param raduis: sphere raduis
		*/
		void CameraZoomSphere(float center_x, float center_y, float center_z, float raduis);

		/**
		* rotate the camera round the object on canvas
		* @param dx 
		* @param dy relative amount in radian. 
		* @param dz 
		*/
		void CameraRotate(float dx, float dy, float dz);

		/**
		* pan the camera
		* @param dx relative amount in pixels
		* @param dy relative amount in pixels
		*/
		void CameraPan(float dx, float dy);

		/**
		* set the camera look at position
		*/
		void CameraSetLookAtPos(float x, float y, float z);
		void CameraGetLookAtPos(float* x, float* y, float* z);

		/**
		* set the camera eye position
		* @param fRotY rotation of the camera around the Y axis, in the world coordinate. 
		* @param fLiftupAngle lift up angle of the camera.
		* @param fCameraObjectDist the distance from the camera eye to the object being followed.
		*/
		void CameraSetEyePosByAngle(float fRotY, float fLiftupAngle, float fCameraObjectDist);
		void CameraGetEyePosByAngle(float* fRotY, float* fLiftupAngle, float* fCameraObjectDist);
		void CameraSetEyePos(double x, double y, double z);
		void CameraGetEyePos(double* x, double* y, double* z);



		/** draw the content of the scene graph to the current render target. If EnableActiveRendering is enabled, this function will be called each render frame after the main scene graph. 
		* however, if EnableActiveRendering is disabled, one can call this function to render a single frame to the render target on demand. 
		*/
		void Draw(float fDeltaTime);

		/**
		* save to file. 
		* @param sFileName a texture file path to save the file to. 
		*	we support ".dds", ".jpg", ".png" files. If the file extension is not recognized, ".png" file is used. 
		* @param nImageSize: if this is zero, the original size is used. If it is dds, all mip map levels are saved.
		*/
		void SaveToFile(const char* sFileName, int nImageSize = 0);
		void SaveToFileEx(const char* sFileName, int width = 0, int height = 0, DWORD dwFormat = 3, UINT MipLevels = 0, int srcLeft = 0, int srcTop = 0, int srcWidth = 0, int srcHeight = 0);

		/**
		* set the canvas size in pixels
		* @param nWidth default to 512
		* @param nHeight default to 512 
		*/
		void SetRenderTargetSize(int nWidth, int nHeight);

		/** set the color of the scene ground when it is not enabled.When scene is enabled, the background color is always the fog color. 
		* @param rgba: it can contain alpha channel, such as "255 255 255 128". If no alpha is specified like "255 255 255", alpha will be 1.0
		*/
		void SetBackGroundColor(const char* rgba);

		/** this is an optional 2D mask, which is drawn over the entire canvas after scene is rendered in to it. 
		* @param pTexture 
		*/
		void SetMaskTexture(ParaAssetObject pTexture);

		/** if true, contents will be drawn to the current render target each frame. Default value is false. where the user can call DrawToTexture() on demand.*/
		void EnableActiveRendering(bool bEnable);

		/** if true, contents will be drawn to the current render target each frame. Default value is false. where the user can call DrawToTexture() on demand.*/
		bool IsActiveRenderingEnabled();

		/**
		* Pick scene object at the current mouse cursor position. 
		* pick the smallest intersected object which is un-occluded by any objects. Object A is considered occluded by object B only if 
		* (1) both A and B intersect with the hit ray. 
		* (2) both A and B do not intersect with each other. 
		* (3) B is in front of A, with regard to the ray origin.
		* 
		* this function will ray-pick any loaded scene object(biped & mesh, but excluding the terrain) using their oriented bounding box. 
		* a filter function may be provided to further filter selected object. 
		* this function will transform all objects to near-camera coordinate system. 
		* This will remove some floating point inaccuracy near the camera position.Hence this function is most suitable for 
		* testing object near the camera eye position. This function does not rely on the physics engine to perform ray-picking. 
		* @param x: screen position relative to the render target. 
		* @param y: screen position relative to the render target. 
		* @params fMaxDistance: the longest distance from the ray origin to check for collision. If the value is 0 or negative, the view culling radius is used 
		*	as the fMaxDistance.
		* @param sFnctFilter: it can be any of the following string or a number string
		* "mesh": mesh any mesh object in the scene. Usually for selection during scene editing.
		* "cmesh": mesh object that is clickable (associated with scripts). Usually for game playing.
		* "notplayer": any object in the scene except for the current player. Usually for selection during scene editing.
		* "": any object in the scene except. Usually for selection during scene editing.
		* "light": only pick light objects
		* "biped": any character objects :local or global.
		* "anyobject": any objects, including mesh and characters. but not including helper objects, such as light.
		* "global": all global objects, such as global character and mesh. This is usually for game mode.
		* "point": the returned object is invalid if there no collision with any physics faces. otherwise, one can use GetPosition function of the returned object to retrieve the intersection point.
		* "terrain":  pick a point on the global terrain only.
		* "walkpoint":  pick a point on the global terrain or any physical object with camera obstruction attribute set to true. 
		* "actionmesh": mesh with action script. 
		* number: if it is a number, it is treated as a 32 bitwise DWORD filter code. see SetPickingFilter() for more example. 
		* @return :the scene object. if the object is invalid, it means that the ray has hit nothing.
		*/
		ParaObject MousePick(float x, float y, float fMaxDistance, const char* sFilterFunc);

		/** show or hide all scene's objects' head on display*/
		void ShowHeadOnDisplay(bool bShow);

		/** whether all scene's objects' head on display*/
		bool IsHeadOnDisplayShown();
	};

	
	/**
	* @ingroup ParaCamera
	* The camera controller.
	* First call FollowObject() method to set the character to follow.
	* set call any of the camera transition functions to set the camera follow mode
	*/
	class PE_CORE_DECL ParaCamera
	{
	public:
		
		/**
		* set the character to follow.
		* the object is also set as the current player
		* @param obj: the ParaObject to follow, one can call 
		*/
		static void FollowObject(ParaObject obj);
		/**
		* set the character to follow.
		* the object must be global in order to be found by its name. It is also set as the current player
		* currently,
		* @param strObjectName: the name of the object to follow 
		*/
		static void FollowObject(const char* strObjectName);
	public:
		/**
		* Set the current camera to follow a certain object, using first person view of the object.
		* @param nTransitionMode: 1 (default) smooth move or 0 immediate move
		* @param fRadius: the distance from the person to the camera
		* @param fAngle: camera lift up angle, this is usually Pi/4: 45 degrees
		*/
		static void FirstPerson(int nTransitionMode, float fHeight, float fAngle);

		/**
		* Set the current camera to follow a certain object, using Third Person view of the object, where character is always centered
		*		while allowing rotation around it from 360 degree angles, 
		* @param nTransitionMode: 1 (default) smooth move or 0 immediate move
		* @param fRadius: the distance from the person to the camera
		* @param fFacing: camera facing, around the y axis, which is the world height axis.
		* @param fAngle: camera lift up angle, this is usually Pi/4: 45 degrees
		*/
		static void ThirdPerson(int nTransitionMode, float fHeight, float fFacing, float fAngle);

		/**
		* @obsolete: 
		* Set the current camera to follow a certain object, using the default view of the object.
		*		character restricted to move within a rectangular region, while camera is facing a fixed direction.
		* @param nTransitionMode: 1 (default) smooth move or 0 immediate move
		* @param fHeight: relative camera height above the object.
		* @param fAngle: camera lift up angle, this is usually Pi/4: 45 degrees
		*/
		static void Default(int nTransitionMode, float fHeight, float fAngle);

		/**
		* Set the current camera to look at a certain object, from a fixed camera location
		*		while allowing rotation around it from 360 degree angles, 
		* @param nTransitionMode: 1 (default) smooth move or 0 immediate move
		* @param fX: camera location: x
		* @param fY: camera location: y
		* @param fZ: camera location: z
		*/
		static void Fixed(int nTransitionMode, float fX, float fY, float fZ);
		/**
		* get the world position of the camera eye. This function takes no parameters.
		* x,y,z are not input, but pure output. In the script, we can call it as below
		*	x,y,z = ParaCamera.GetPosition(); -- get the camera's eye position
		* in Luabind, it is defined as 
		*	.def("GetPosition", &ParaCamera::GetPosition, pure_out_value(_1) + pure_out_value(_2) + pure_out_value(_3))
		* please note, y is the absolute position in world coordinate
		* @see SetPosition(float x, float y, float z)
		*/
		static void GetPosition(float* x, float* y, float* z);

		/** get the position that the camera is currently looking at.*/
		static void GetLookAtPosition(float* x, float* y, float* z);

		/** we can alter key map at runtime
		* @param key: CharacterAndCameraKeys . [0-10]
		* @param scancode:  DIK_A, DIK_D,DIK_W,DIK_S,DIK_Q,DIK_E,DIK_SPACE,0,0,DIK_INSERT,DIK_DELETE*/
		static void SetKeyMap(int key, int scancode);

		/** get scancode from key id */
		static int GetKeyMap(int key);

		/** get the attribute object associated with the current camera object. */
		static ParaAttributeObject GetAttributeObject();
		/** used for API exportation. */
		static void GetAttributeObject_(ParaAttributeObject& output);
	};
	/** 
	* @ingroup ParaScene
	* ParaScene namespace contains a list of HAPI functions to create and modify scene objects in paraworld. 
	* The following are basic steps to create scene object:
	* - Use Create*() functions to create scene object. The created object is return as a ParaObject instance.
	* - Use the ParaObject instance to modify the position and orientation of the object.
	* - Use Attach method to insert the scene object to the scene.
	*
	* Please note: if an object is created without attaching to the scene, it may result in memory leak.
	* although, we may automatically clear unattached scene object, when the application quit.
	*/
	class PE_CORE_DECL ParaScene
	{
	public:
		/** get the attribute object associated with the global scene. */
		static ParaAttributeObject GetAttributeObject();
		static ParaAttributeObject GetAttributeObject1(const char* name);
		/** used for API exportation. */
		static void GetAttributeObject_(ParaAttributeObject& output);
		/** get the attribute object associated with the current sky object. */
		static ParaAttributeObject GetAttributeObjectSky();
		/** used for API exportation. */
		static void GetAttributeObjectSky_(ParaAttributeObject& output);
		/** get the attribute object associated with the current player. */
		static ParaAttributeObject GetAttributeObjectPlayer();
		/** used for API exportation. */
		static void GetAttributeObjectPlayer_(ParaAttributeObject& output);
		/** get the attribute object associated with the global ocean manager. */
		static ParaAttributeObject GetAttributeObjectOcean();
		/** used for API exportation. */
		static void GetAttributeObjectOcean_(ParaAttributeObject & output);
		/** get the attribute object associated with the sun light . */
		static ParaAttributeObject GetAttributeObjectSunLight();
		/** used for API exportation. */
		static void GetAttributeObjectSunLight_(ParaAttributeObject & output);
		
		/**
		* get the scene object by name. 
		* currently, the object must be global, in order to be found by its name.
		* @remark: if local mesh's name begins with "g_", it can also be retrieved by calling this function.
		* however, if a global object has the same name, the global object is always returned instead of the local mesh.
		* @param strObjName: the format of the name is as below: 
		*	strObjName := [<_type>]string 
		*	_type := managed_loader | OPC | NPC | player | zone | portal | CRenderTarget 
		* e.g. strObjName = "creatures1" or "<managed_loader>sceneloader1" or "<player>" or "<CRenderTarget>name".
		*/
		static ParaObject GetObject(const char * strObjName);

		/** get an object by its ID */
		static ParaObject GetObject5(int nID);

		/* check whether a given object exist. */
		static bool CheckExist(int nID);

		/**
		* get the first local object,whose position is very close to vPos. This function will search for the first (local mesh) object throughout the hierachy of the scene.
		* this function is kind of slow, please do not call on a per frame basis. Use GetObjectByViewBox() to get an object faster. 
		* @param vPos: world position of the local mesh object
		* @param fEpsilon: if a mesh is close enough to vPos within this value. 
		* @return: NULL if not found
		*/
		static ParaObject GetObject3(float x, float y, float z);
		
		/** get the first object that matches the position within fEpsilon, which is usually 0.01f*/
		static ParaObject GetObject4(float x, float y, float z, float fEpsilon);

		/** this function shall never be called from the scripting interface. this is solely for exporting API. and should not be used from the scripting interface.*/
		static void GetObject_(ParaObject* pOut, const char * strObjName);

		/** Get the current player. same as ParaScene.GetObject("<player>"). */
		static ParaObject GetPlayer();
		/** this function shall never be called from the scripting interface. this is solely for exporting API. and should not be used from the scripting interface.*/
		static void GetPlayer_(ParaObject* pOut);

		/**
		* get the next scene object. 
		* @param obj: the object whose next object is retrieved.
		* @return: return the next object. the returned object is invalid if there is only one object left.
		*/
		static ParaObject GetNextObject(ParaObject& obj);

		/**
		* Create a new parallel world of a given size. When this function is called, it will replace
		* previously created world of the same name. Currently only a single world can be created at
		* any given time. In future, we will support hosting several world simultaneously.
		* @param sWorldName: name of the world to be created. 
		* @param fWorldSize: the size of the world in meters.
		* @param sConfigFile: the file name of the configuration file. Currently it is the same as the terrain configuration file
		*/
		static void CreateWorld(const char * sWorldName, float fWorldSize, const char* sConfigFile);
		
		/** reset the scene to blank. 
		* this function is NOT automatically called when a new isolated world is created. so one need
		* to call Reset() when it wants to change the world, otherwise the new world will be merged into the 
		* previous world.
		* @see CreateWorld()
		*/
		static void Reset();
		/**
		* Create a managed loader for dynamic scene object loading and unloading. 
		* The behavior of a managed loader is below: 
		*
		*	- The child objects of a managed loader will be automatically loaded and unloaded as a single entity.
		*	- Generally only static objects are attached to a managed loader.
		*	- Different managed loaders in a ParaScene must have different names.
		*	- if one create a manager loader with the same name several times, the same managed loader will be returned.
		*	- the bounding box of a managed loader will be automatically calculated as new child objects are attached to it. 
		*	- The current loader algorithm will linearly transverse all managed loaders in the scene to decide which scene objects
		*		to load or unload. Although the routine is executed when the CPU is free, it is good practice to keep the total number of
		*		managed loaders in a single scene low. I think a couple of thousand loaders will be fine for current hardware. 
		*	- it is good practice to use managed loaders to group all static scene objects that appears in a scene. Because, it will 
		*		keep the scene graph to a moderate size automatically and accelerate physics calculation, etc.
		*
		* The following NPL code shows typical usage of the managed loader. Generally a managed loader and its children are
		* written in a single script file. Then, any other script can call dofile() or NPL.ActivateCopy() to run the script
		* as many times as they like. The code however will ensure that objects managed by the loader will only be created
		* and attached once in the game engine.There may be a setting in ParaEngine to do automatic garbage collection with 
		* managed loaders, so one may need to call the following script file often enough for the managed objects to stay active
		* in the game scene.\n
		*		
		local sceneLoader = ParaScene.GetObject("<managed_loader>scene2"); \n
		if (sceneLoader:IsValid() == true) then \n
		-- if the scene loader already exists, just attach it to the scene. \n
		ParaScene.Attach(sceneLoader); \n
		else \n
		-- if the scene loader is not created before, we will create it now \n
		sceneLoader = ParaScene.CreateManagedLoader("scene2"); \n
		ParaAsset.ParaXModel("tiny", "Units/Human/Peasant/peasant.x"); \n
		
		-- create scene objects and add them to managed loader object \n
		local player; \n
		player = ParaScene.CreateCharacter ("LiXizhi2", "tiny", "", false, 0.5, 0, 1.0); \n
		player:SetPosition(21758, 0, 16221); \n
		player:SnapToTerrainSurface(0); \n
		sceneLoader:AddChild(player); \n
		
		-- attach all objects in the loader to the scene graph \n
		ParaScene.Attach(sceneLoader); \n
		end \n
		* \n	
		* @param sLoaderName: the name of the loader.
		* @see CManagedLoaderObject in ParaEngine reference.
		*/
		static ParaObject CreateManagedLoader(const char * sLoaderName);/*, float vMinX, float vMinY, float vMinZ, float vMaxX, float vMaxY, float vMaxZ*/

		/**
		* update the terrain tile at a given tile location.
		* In case a world is extremely large, it can be divided into a matrix of square tiles of a given size.
		* there is no limitation on the size of this matrix, hence the (nRow, nCol) parameter can be any integer pair.
		* but typically, the matrix is rarely larger than 64*64. We use indexed set to save the matrix, so the memory
		* consumption of the matrix is linearly proportionally to the number of terrain tiles created.
		* @param nRow: the row number in the terrain tile matrix.nRow>=0.
		* @param nCol: the row number in the terrain tile matrix.nCol>=0.
		* @param sConfigFile: the terrain tile configuration file to be used to create the terrain at (nRow, nCol).
		* if sConfigFile == "", then the terrain at (nRow, nCol) will be removed. 
		* @see CreateWorld(const char * sWorldName, float fWorldSize, float fTerrainTileSize);
		*/
		//static void UpdateTerrainTile(int nRow, int nCol, const char* sConfigFile);

		/**
		* Create and set the global terrain from height map and texture files.
		* this function can be called multiple times, in which cases previously loaded terrain will be discarded
		* example: ParaScene.CreateGlobalTerrain(2048, 7, "LlanoElev.png", 5.0, 15.0, 1, "LlanoTex.jpg", "dirt2.jpg", 64, 10.0");
		* @param fRadius: entire terrain size, this has doing to do with the actual terrain map size, 
		*		it just prevent mobile characters from walking outside it.
		* @param nDepth: depth of the quad tree terrain hierarchy. objects created on the terrain will be organized in a quad tree.
		*		This is the depth of the quad tree. It should not be too big. usually 7 is enough.
		* the rest of the parameters specify the data to render the terrain. 
		* @param sHeightmapfile: the height map used to create the terrain. It must be sized to 2*2*...*2 pixels for both height and width. 
		*		so usually it is 1024*1024, 2048*2048, etc.
		* @param fTerrainSize: the actual terrain size in the game
		* @param bSwapvertical: if one want to swap the height map data vertically. 
		* @param sMainTextureFile: texture to be mapped to entire terrain
		* @param sCommonTextureFile: texture to be tiles to the entire terrain to add some details.
		* @param nMaxBlockSize: When doing LOD with the height map, the max block size must be smaller than this one. 
		*		This will be (nMaxBlockSize*nMaxBlockSize) sized region on the height map.
		* @param fDetailThreshold: we will use a LOD block to approximate the terrain at its location, if the 
		*		block is smaller than fDetailThreshold pixels when projected to the 2D screen.
		*/
		static void CreateGlobalTerrain(float fRadius, int nDepth, const char* sHeightmapfile, 
			float fTerrainSize, float fElevscale, int bSwapvertical, 
			const char*  sMainTextureFile, const char* sCommonTextureFile, 
			int nMaxBlockSize, float fDetailThreshold);

		/**
		* Automatically attach a scene object to the scene graph according to its type and position.
		* The object can be a manager loader, a global object or any ordinary scene object.
		*    - For tiled object, it is added to the smallest CTerrainTile in the quad-tree
		*    - For global tiled object, it is added to the root CTerrainTile
		*    - For non-tiled object, it is added to an automatically created CContainerObject whose name is the class identifier name of the object.
		*       hence objects are automatically grouped by class type on the root scene's child nodes.
		*       To explicitly add an object to a specified parent, use AddChild() method on the parent node.
		* @return: parent object is returned if successfully attached.
		* For tiled object, this is the smallest terrain tile that contains the object.
		* For non-tiled object, this is the automatically created CContainerObject that.
		* @note: If the object has already been attached to the scene, it will be removed and reattached.
		* In most cases, a strong reference of the object is kept by its parent.
		*/
		static void Attach(ParaObject& pObj);
		/**
		* delete the object. If the object is root scene object, then the entire scene is deleted.
		*/
		static void Delete(ParaObject& pObj);
		/**
		* detach the object. Be sure that the object is properly deleted after it is detached from the scene, because
		* the scene graph is not responsible to manage it any more. The only exception is the managed loader object. 
		* @see CreateManagedLoader(const char * sLoaderName)
		*/
		static void Detach(ParaObject& pObj);

		/** fire a missile from (fromX, fromY, fromZ) to (toX, toY, toZ) using the specified missile object and speed.*/
		static void FireMissile(int nMissileID, float fSpeed, double fromX, double fromY, double fromZ, double toX, double toY, double toZ);
		static void FireMissile1(const char* assetname, float fSpeed, double fromX, double fromY, double fromZ, double toX, double toY, double toZ);
		static void FireMissile2(ParaAssetObject& asset, float fSpeed, double fromX, double fromY, double fromZ, double toX, double toY, double toZ);

		/** set whether scene is modified */
		static void SetModified(bool bModified);
		/** Get whether scene is modified */
		static bool IsModified();

		/** when a scene is paused, all animation will be frozen.*/
		static bool IsScenePaused();
		/** pause the scene */
		static void PauseScene(bool bEnable);

		/** whether 3D scene is enabled or not. a disabled scene is not visible no matter what. 
		This function must be called at least once whenever a new scene is loaded, or 3D scene will not be displayed. 
		A scene is automatically disabled when cleaned up. */
		static bool IsSceneEnabled();
		/** enable the scene*/
		static void EnableScene(bool bEnable);

		/**
		* create a simple and static mesh object in the scene. Solid mesh will collide with mobile characters.
		* Simple mesh does not implement clipping optimization, hence should not be very large. It is good to use it
		* @param strMeshAssetName: for small houses, trees, stones, small hills, etc. 
		* when naming mesh file, one can combine "_a"(no physics), "_b"(billboard), "_t"(transparent), "_d"(dim or no lighting), "_r"(receive shadow) in the file name in any order, such as "xxx_b_t_d.x".
		* all such special file endings are listed below
		*  - "_a": no physics, it will have no physics, even bApplyPhysics is true. For example. "grass_a.x". 
		*  - "_b": billboarded and no physics
		*  - "_r": mesh is shadow receiver
		*  - "_e": mesh is not a shadow caster
		*  - "_t": mesh contains majority transparent objects. Please note that this is different from alpha testing. Transparency throguh alpha testing is not transparent. 
		*  - "_p": mesh is picture or particles. They are rendered with billboarding and Force No Light flag on.
		*  - "_d": mesh is dim and is rendered with force no light on. 
		*  - "_v": mesh is rendered with vegetation shader. the static model like palm trees, grasses, bamboos can be animated by this shader.
		* @param fOBB_X: object bounding box.x 
		* @param fOBB_Y: object bounding box.y 
		* @param fOBB_Z: object bounding box.z 
		* @param fFacing: rotation of the bounding box around the y axis.
		* @param bSolid: 1, if it is a solid mesh, otherwise it is passable.
		* @param localMatrix: the local transformation matrix of the mesh. It is a string of 4*3 number of float values
		* separated by comma (see below): \n
		*	"mat._11, mat._12, mat._13, mat._21, mat._22, mat._23,mat._31, mat._32, mat._33,mat._41, mat._42, mat._43"
		*	If a blank string("") is specified, the local matrix is set to identity matrix.
		*/
		static ParaObject CreateMeshObject(const char* strObjectName, const char* strMeshAssetName, 
			float fOBB_X, float fOBB_Y, float fOBB_Z,float fFacing, bool bSolid,const char* localMatrix); 
		static ParaObject CreateMeshObject(const char* strObjectName, ParaAssetObject& asset, 
			float fOBB_X, float fOBB_Y, float fOBB_Z,float fFacing, bool bSolid,const char* localMatrix); 

		/** create an object according to type. 
		* @param strType: as returned by GetAttributeClassName of IAttributeField, such as "CMeshPhysicsObject", "CMeshObject", "CBipedObject", etc.  
		* @param strObjectName: string identifier of the object
		* @param x,y,z: position of the object. 
		* @return ParaObject
		*/
		static ParaObject CreateObject(const char* strType, const char * strObjectName, double x, double y, double z);

		/**
		* create a portal zone object for portal rendering. 
		* @param sZoneName: it must be unique.
		* @param fRadius: the zone sphere radius is an approximation of the bounding volume. we will only further check if an object is inside a zone, if it is first inside this sphere.
		* @param sBoundingVolumes: if this is "", the zone will not be able to automatically determine which mobile objects are in it. 
		*  or it can be "x1,y1,z1;x2,y2,z2;x3,y3,z3;" each three value is a point in local space denoting a plane of the bounding volume. 
		*  because the convex bounding volume in local space always contains the origin, three values is enough to represent a plane in the bounding volume.
		* @param width, height, depth, facing: the bounding shape of the portal.
		*/
		static ParaObject CreateZone(const char* sZoneName, const char* sBoundingVolumes, float width, float height, float depth, float facing);

		/**
		* create a portal object for portal rendering
		* @param sPortalName: it must be a unique name.
		* @param sHomeZone: a zone name that this portal connects. this can be "", if a portal is not connected to any zone.  
		* @param sTargetZone: another zone name that this portal connects. this can be "", if the portal is connected to outer space. 
		* @param sQuadVertices: it is coordinates of the 4 quad vertices, "x1,y1,z1;x2,y2,z2;x3,y3,z3;x4,y4,z4;" 
		* the order of the first three vertices decides the direction of the quad plane. direction of quad plane is only useful when the portal's sTargetZone is empty(outer space), and it should always point from home zone to outer space. 
		* @param width, height, depth, facing: the bounding size of the portal.
		*/
		static ParaObject CreatePortal(const char* sPortalName, const char* sHomeZone, const char* sTargetZone, const char* sQuadVertices, float width, float height, float depth, float facing);

		/**
		* create a voxel mesh object. A voxel mesh is a 32*32*32 grid which can be editable at runtime, and rendered using the matching cube algorithm. 
		@param strObjectName: 
		@param sGridFileName: the file name from which to load.This can be "", where an empty one is created. 
		@param sTextureFileName: the texture to use, one can later change this by calling SetReplaceableTexture. 
		*/
		static ParaObject CreateVoxelMesh(const char* strObjectName, const char* sGridFileName, const char* sTextureFileName); 
		/**
		* create a sky box and add it to the current list. sky box with the same name 
		* will not be recreated,but will be selected as the current sky box. It may be a sky box/dome/plane or whatever.
		* The associated mesh will be scaled by the specified amount along x,y,z axis and then 
		* translate up or down along the y axis. 
		* in many cases, the mesh data in the mesh asset is of unit size. 
		* @param strObjectName: sky name
		* @param strMeshAssetName: mesh asset name. this is not the file name.
		* @param fScaleX: the static mesh local transform scale along the x axis
		* @param fScaleY: the static mesh local transform scale along the y axis
		* @param fScaleZ: the static mesh local transform scale along the z axis
		* @param fHeightOffset: the translation along the y axis. 
		*/
		static bool CreateSkyBox(const char* strObjectName, const char* strMeshAssetName, float fScaleX, float fScaleY,float fScaleZ, float fHeightOffset);
		static bool CreateSkyBox_(const char* strObjectName, ParaAssetObject& asset, float fScaleX, float fScaleY,float fScaleZ, float fHeightOffset);
		/** delete a name sky box. 
		* @param strObjectName: if this is "", all sky boxes will be deleted.*/
		static void DeleteSkyBox(const char* strObjectName);
		
		/**
		* Create static triangle mesh based actor for novodex physics engine. 
		* 	Some notes about meshes:
		* 	- Be sure that you define face normals as facing in the direction you intend.
		* 	Collision detection will only work correctly between shapes approaching the
		* 	mesh from the ��outside��, i.e. from the direction in which the face normals point.
		* 	- Do not duplicate identical vertices! If you have two triangles sharing a vertex,
		* 	this vertex should only occur once in the vertex list, and both triangles should
		* 	index it in the index list. If you create two copies of the vertex, the collision
		* 	detection code won't know that it is actually the same vertex, which leads to a
		* 	decreased performance and unreliable results.
		* 	- Also avoid t-joints and non-manifold edges for the same reason. (A t-joint is a
		* 	vertex of one triangle that is placed right on top of an edge of another triangle, but
		* 	this second triangle is not split into two triangles at the vertex, as it should. A non-manifold
		* 	edge is an edge (a pair of vertices) that is referenced by more than two
		* 	triangles.)
		* @param strMeshAssetName: the mesh asset name which contains the triangular mesh.
		* when naming mesh file, one can combine "_a"(no physics), "_b"(billboard), "_t"(transparent), "_d"(dim or no lighting), "_r"(receive shadow) in the file name in any order, such as "xxx_b_t_d.x".
		* all such special file endings are listed below
		*  - "_a": no physics, it will have no physics, even bApplyPhysics is true. For example. "grass_a.x". 
		*  - "_b": billboarded and no physics
		*  - "_r": mesh is shadow receiver
		*  - "_e": mesh is not a shadow caster
		*  - "_t": mesh contains majorily transparent objects. Please note that this is diferent from alpha testing. Transparency throguh alpha testing is not transparent. 
		*  - "_p": mesh is picture or particles. They are rendered with billboarding and Force No Light flag on.
		*  - "_d": mesh is dim and is rendered with force no light on. 
		*  - "_v": mesh is rendered with vegetation shader. the static model like palm trees, grasses, bamboos can be animated by this shader.
		* @param fOBB_X: object bounding box.x 
		* @param fOBB_Y: object bounding box.y 
		* @param fOBB_Z: object bounding box.z 
		* @param bApplyPhysics: whether to turn on the physics of the mesh
		* @param localMatrix: the local transformation matrix of the mesh. It is a string of 4*3 number of float values
		* separated by comma (see below): \n
		*	"mat._11, mat._12, mat._13, mat._21, mat._22, mat._23,mat._31, mat._32, mat._33,mat._41, mat._42, mat._43"
		*	If a blank string("") is specified, the local matrix is set to identity matrix.
		*/
		static ParaObject CreateMeshPhysicsObject(const char* strObjectName, const char* strMeshAssetName, float fOBB_X, float fOBB_Y, float fOBB_Z, bool bApplyPhysics,const char* localMatrix);
		static ParaObject CreateMeshPhysicsObject(const char* strObjectName, ParaAssetObject& asset, float fOBB_X, float fOBB_Y, float fOBB_Z, bool bApplyPhysics,const char* localMatrix);
		/** this function shall never be called from the scripting interface. this is solely for exporting API. and should not be used from the scripting interface.*/
		static void CreateMeshPhysicsObject__(ParaObject* pOut, const char* strObjectName, ParaAssetObject& asset, float fOBB_X, float fOBB_Y, float fOBB_Z, bool bApplyPhysics,const char* localMatrix);
		
		/**
		* Create a new light object. 
		* @param strObjectName Light Object's name
		* @param fPosX world position X
		* @param fPosY world position Y
		* @param fPosZ world position Z
		* @param sLightParams if this is "" a default light will be created. 
		* otherwise it is in the following format. 
		* format is "Type Range (r g b a) att0 att1 att2"
		* D3DLIGHTTYPE    Type;            Type of light source 
		* 						- D3DLIGHT_POINT          = 1,
		* 						- D3DLIGHT_SPOT           = 2,
		* 						- D3DLIGHT_DIRECTIONAL    = 3,
		* float           Range;           Cutoff range 
		* D3DCOLORVALUE   Diffuse;         Diffuse color of light
		* float           Attenuation0;    Constant attenuation
		* float           Attenuation1;    Linear attenuation
		* float           Attenuation2;    Quadratic attenuation
		* e.g. "1 7.0 (1 1 0 1) 0.3 0.1 1"
		* light intensity is calculated as 1/(Attenuation0+d*Attenuation1+d*d*Attenuation2), where d is the distance from the light to object center.
		*
		*	This is used to make light objects persistent. 
		* @param localMatrix: the local transformation matrix of the mesh. It is a string of 4*3 number of float values
		* separated by comma (see below): \n
		*	"mat._11, mat._12, mat._13, mat._21, mat._22, mat._23,mat._31, mat._32, mat._33,mat._41, mat._42, mat._43"
		*	If a blank string("") is specified, the local matrix is set to identity matrix.
		* @return light object is created and returned. One need to attach it to scene.
		*/
		static ParaObject CreateLightObject(const char* strObjectName, float fPosX, float fPosY, float fPosZ, const char* sLightParams, const char* localMatrix);

		/**
		* Create dynamic physics actor for novodex physics engine. 
		* dynamic objects are considered global object and is loaded to the physics engine immediately
		* currently, only Box and sphere shaped objects are supported.
		*/
		static ParaObject CreateDynamicPhysicsObject(const char* strObjectName, const char* strMeshAssetName, float fOBB_X, float fOBB_Y, float fOBB_Z, bool bRenderMesh);
		//static ParaObject CreateDynamicPhysicsObject(const char* strObjectName, ParaAssetObject asset, float fOBB_X, float fOBB_Y, float fOBB_Z, bool bRenderMesh);


		/**
		* Create Character. 
		* @param strObjectName: the name short cut for this object. If the character with the same name exists, it will be renamed to a random name. So that the character is always created.
		* @param strMultiAnimationAssetName: the asset name of the base model. It can be nil, in which one must specified it in other means.
		* if the asset file name ends with "_s", it will always be static and local , even IsGlobal is true. For example. "windmill_s.x". Some other special file endings are listed below
		*  - "_s": force static, solid and local.
		* @param strScript: The script file to be loaded when the object is loaded for the first time
		* @param bIsGlobal: a global character is not attached to the quad tree terrain, thus can move around the entire scene
		*		a non-global character is attached to the quad tree terrain, and is generally considered immobile. 
		*		although it is OK for non-global character to move only in very small region.
		* @param fRadius: radius of the character used in collision detection. if radius is 0, then the object is regarded as passable (not solid).
		* @param fFacing: rotation around the y axis
		* @param fScaling: typically this should be 1, however, one can scale the mesh and animation to render the character in a different size.
		*/
		static ParaObject CreateCharacter(const char* strObjectName, const char* strMultiAnimationAssetName, const char* strScript, bool bIsGlobal,
			float fRadius, float fFacing, float fScaling);
		static ParaObject CreateCharacter(const char* strObjectName, ParaAssetObject& asset, const char* strScript, bool bIsGlobal,
			float fRadius, float fFacing, float fScaling);
		/** this function shall never be called from the scripting interface. this is solely for exporting API. and should not be used from the scripting interface.*/
		static void CreateCharacter__(ParaObject* pOut, const char* strObjectName, ParaAssetObject& asset, const char* strScript, bool bIsGlobal,
			float fRadius, float fFacing, float fScaling);
		
		/**
		* play a 3D sound at world location (fx, fy, fz). 
		* Currently, the sound is played until it stopped(no looping). And the sound range is internally defined.
		* Please use the ParaUI.PlaySound() to play an ordinary sound or music with or without looping.
		*/
		static void Play3DSound(const char* strSoundAssetName, float fX, float fY, float fZ);
		//static void Play3DSound(ParaAssetObject asset, float fX, float fY, float fZ);

		/**
		* Set the global water drawing attribute.
		* the global water level is just a water surface at a given height, which is always drawn at current camera location
		* to fill the entire screen. Water surface will be drawn after terrain
		* @param bEnable: whether to draw global water 
		* @param fWaterLevel: water level in meters. Default value is 0.0f;
		*/
		static void SetGlobalWater(bool bEnable, float fWaterLevel);

		/** get the current global water level in meters. This function will return valid value even if the water is disabled. */
		static float GetGlobalWaterLevel();

		/** return true if global ocean water is enabled.*/
		static bool IsGlobalWaterEnabled();
		
		/** call this function, when the ocean has changed or the environment has changed. This will cause 
		* the reflection map of the ocean surface to redraw regardless of whether the camera moves or not. */
		static void UpdateOcean();

		/**
		* add water ripple to the ocean surface. 
		* @param x position of the ripple
		* @param y this is usually set to the current water level. @see GetGlobalWaterLevel()
		* @param z position of the ripple
		*/
		static void AddWaterRipple(float x, float y, float z);

		/**
		* execute the scene command string. Most commands are for debugging purposes. The list of command
		* is given below:
		* - "show OBB" display every scene object's bounding box 
		* - "hide OBB" hide every scene object's bounding box 
		* - "show report" display the per frame report, such as number of mesh drawn, number of terrain triangles, etc,..
		* - "hide report" hide the per frame report, such as number of mesh drawn, number of terrain triangles, etc,..
		* @params strCmd: the string command passed
		*/
		static void Execute(const char* strCmd);
		/**
		* Pick scene object at the current mouse cursor position. 
		* pick the smallest intersected object which is un-occluded by any objects. Object A is considered occluded by object B only if 
		* (1) both A and B intersect with the hit ray. 
		* (2) both A and B do not intersect with each other. 
		* (3) B is in front of A, with regard to the ray origin.
		* 
		* this function will ray-pick any loaded scene object(biped & mesh, but excluding the terrain) using their oriented bounding box. 
		* a filter function may be provided to further filter selected object. 
		* this function will transform all objects to near-camera coordinate system. 
		* This will remove some floating point inaccuracy near the camera position.Hence this function is most suitable for 
		* testing object near the camera eye position. This function does not rely on the physics engine to perform ray-picking. 
		* @params fMaxDistance: the longest distance from the ray origin to check for collision. If the value is 0 or negative, the view culling radius is used 
		*	as the fMaxDistance.
		* @param sFnctFilter: it can be any of the following string or a number string
		* "mesh": mesh any mesh object in the scene. Usually for selection during scene editing.
		* "cmesh": mesh object that is clickable (associated with scripts). Usually for game playing.
		* "notplayer": any object in the scene except for the current player. Usually for selection during scene editing.
		* "": any object in the scene except. Usually for selection during scene editing.
		* "light": only pick light objects
		* "biped": any character objects :local or global.
		* "anyobject": any objects, including mesh and characters. but not including helper objects, such as light.
		* "global": all global objects, such as global character and mesh. This is usually for game mode.
		* "point": the returned object is invalid if there no collision with any physics faces. otherwise, one can use GetPosition function of the returned object to retrieve the intersection point.
		* "actionmesh": mesh with action script. 
		* number: if it is a number, it is treated as a 32 bitwise DWORD filter code. see SetPickingFilter() for more example. 
		* @return :the scene object. if the object is invalid, it means that the ray has hit nothing.
		*/
		static ParaObject MousePick(float fMaxDistance, const char* sFilterFunc);

		static ParaObject Pick(float rayX, float rayY, float rayZ, float dirX, float dirY, float dirZ, float fMaxDistance, const char* sFilterFunc);

		/**
		* select select objects within a given region into a given group. 
		* @param nGroupIndex: which group to select to. One can get the result from ParaSelection. In most cases, select to group 1; since group 0 is reserved for current selection. 
		* @param x,y,z,radius: a sphere in world space. 
		* @param sFnctFilter: it can be any of the following string.
		* "mesh": mesh any mesh object in the scene. Usually for selection during scene editing.
		* "cmesh": mesh object that is clickable (associated with scripts). Usually for game playing.
		* "notplayer": any object in the scene except for the current player. Usually for selection during scene editing.
		* "": any object in the scene except. Usually for selection during scene editing.
		* "light": only pick light objects
		* "biped": any character objects :local or global.
		* "anyobject": any objects, including mesh and characters. but not including helper objects, such as light.
		* "global": all global objects, such as global character and mesh. This is usually for game mode.
		* @return: the total number of selected objects is returned. 
		*/
		static int SelectObject(int nGroupIndex, float x,float y, float z, float radius, const char* sFilterFunc );
		/**
		* @param x1,y1,z1,x2,y2,z2,fRotY: a bounding box: two diagonal points in world space and rotation around Y. 
		*/
		static int SelectObject1(int nGroupIndex, float x1,float y1, float z1,float x2,float y2, float z2, float fRotY, const char* sFilterFunc );


		/** register a mouse or key event handler 
		* @param sID: a string identifier of the event handler.
		*   if sID begins with "_m" it is treated as a mouse click event, except that
		*		if sID begins with "_mm" it is treated as a mouse move event.
		*		if sID begins with "_md" it is treated as a mouse down event.
		*		if sID begins with "_mu" it is treated as a mouse up event.
		*   if sID begins with "_k" it is treated as a key down event.
		*   if sID begins with "_ku" it is treated as a key up event.
		*   if sID begins with "_n" it is treated as a network event handler.
		* Note: mouse click is rarely used, since it can be produced in NPL via mouse down move and up. However, lazy NPL programmer can still use it if they do not like to write other mouse handlers in NPL.
		* @param sScript: the script to be executed when the event is triggered.This is usually a function call in NPL.
		*	sScript should be in the following format "{NPL filename};{sCode};". this is the same format in the UI event handler
		*/
		static void RegisterEvent(const char* sID, const char* sScript);
		/**
		* same as above RegisterEvent(), except that it allows caller to explicitly specify the event type, instead of deriving it from the event name.
		* @param nEventType any bit combination of EventHandler_type
		* @param sID any unique string identifier
		* @param sScript the NPL script. 
		*/
		static void RegisterEvent1(DWORD nEventType, const char* sID, const char* sScript);
		/** unregister a mouse or key event handler */
		static void UnregisterEvent(const char* sID);
		/** unregister all mouse or key event handler */
		static void UnregisterAllEvent();

		/** whether the game engine will automatically generate mouse events for Game Objects.If true, The OnClick callback will 
		* be automatically called whenever there is a mouse click. */
		static void EnableMouseClick(bool bEnable);
		
		/** get the actor that is being processed by the AI module or a script call back. The validity of the pointer is not guaranteed. */
		static ParaObject GetCurrentActor();
		/** Set the actor that is being processed by the AI module or a script call back. The pointer can be NULL. */
		static void SetCurrentActor(ParaObject pActor);

		/**
		* It changes the current player to the next player in the scene. 
		* this function is mostly for testing and game purpose. 
		* if the object has a reference object, the reference object will become the current object. 
		* return : the new current player is returned. 
		*/
		static ParaObject TogglePlayer();

		/**
		* get an object(usually a static mesh object) by a given view box. 
		* @param viewbox: One can get the view box 
		* by calling ParaObject:GetViewBox(). Or one can construct it using a table with the following
		* field:{pos_x, pos_y,pos_z,obb_x,obb_y,obb_z,}
		* pos_x, pos_y,pos_z: is the point at the bottom center of the box. 
		* obb_x,obb_y,obb_z: is the size of the box.
		* @return: return the object with the closest match with the bounding box.
		*/
		static ParaObject GetObjectByViewBox(const object& viewbox);

		/**
		* get the action meshes within or intersect a sphere. 
		* same as GetObjectsBySphere(..., "actionmesh")
		*/
		static int GetActionMeshesBySphere(const object& inout, float x, float y, float z, float radius);

		/**
		* @param inout: input and output, it should be an empty table.
		* @param x: sphere center x
		* @param y: sphere center y
		* @param z: sphere center z
		* @param radius: sphere radius
		* @param sFnctFilter: it can be any of the following string.
		*	"mesh": mesh any mesh object in the scene. Usually for selection during scene editing.
		*	"cmesh": mesh object that is clickable (associated with scripts). Usually for game playing.
		*	"notplayer": any object in the scene except for the current player. Usually for selection during scene editing.
		*	"": any object in the scene except. Usually for selection during scene editing.
		*	"light": only pick light objects
		*	"biped": any character objects :local or global.
		*	"anyobject": any objects, including mesh and characters. but not including helper objects, such as light.
		*	"actionmesh": mesh with action script. 
		*	"global": all global objects, such as global character and mesh. This is usually for game mode.
		*/
		static int GetObjectsBySphere(const object& inout, float x, float y, float z, float radius, const char* sFilterFunc);

		/**
		* Get objects inside or intersect with a screen rect. screen rect is translated to a 3d cone from the camera eye position to a plane fMaxDistance away.
		* This function is usually used for finding other static mesh objects near a certain character. 
		* please note that: objects must be completely inside the near and far planes in order to pass the test. 
		* @param output: list to get the result
		* @param left,top, right, bottom: the rect in screen space coordinates
		* @param sFnctFilter: it can be any of the following string.
		*	"mesh": mesh any mesh object in the scene. Usually for selection during scene editing.
		*	"cmesh": mesh object that is clickable (associated with scripts). Usually for game playing.
		*	"notplayer": any object in the scene except for the current player. Usually for selection during scene editing.
		*	"": any object in the scene except. Usually for selection during scene editing.
		*	"light": only pick light objects
		*	"biped": any character objects :local or global.
		*	"anyobject": any objects, including mesh and characters. but not including helper objects, such as light.
		*	"actionmesh": mesh with action script. 
		*	"global": all global objects, such as global character and mesh. This is usually for game mode.
		* @param fMaxDistance: the world unit distance inside which we will test possible candidates. if negative, it will get all visible ones.
		* @return: return the number of objects in sphere.
		*/
		static int GetObjectsByScreenRect(const object& inout, int left, int top, int right, int bottom, const char* sFilterFunc, float fMaxDistance);
		
		/**
		* This function is called manually to update characters in a square region. So that when the terrain heightfield is changed,
		* the characters within the region will act accordingly, either falling down or snap to terrain surface. 
		* @param x: center of the terrain region being modified. 
		* @param y: center of the terrain region being modified. 
		* @param fRadius : radius of the terrain region being modified. 
		*/
		static void OnTerrainChanged(float x,float y, float fRadius);

		/**
		* save all local characters in the scene to the current NPC database regardless of whether they are modified or not
		* this function is usually called manually in some very rare cases. In most cases, call SaveLocalCharacters() instead.
		* @return the number of saved characters are returned.
		*/
		static int SaveAllCharacters();

		/** show or hide all scene's objects' head on display*/
		static void ShowHeadOnDisplay(bool bShow);

		/** whether all scene's objects' head on display*/
		static bool IsHeadOnDisplayShown();

		/**
		* Preload NPCs By Regions. By default NPCs are loaded on demand when the ParaTerrain.GetElevation is called.  
		* @param pOut : the list will be filled with data in the database that meat the criterion.
		* @param vMin min point in world coordinate system, y component is ignored
		* @param vMax max point in world coordinate system, y component is ignored 
		* @param bReload: if the character already exists in the scene and it is true, the character will be updated with the parameter in the database
		* @return the number of NPC is returned.
		*/
		static int LoadNPCsByRegion(float min_x, float min_y, float min_z, float max_x, float max_y, float max_z, bool bReload);
		
		/**
		* NOTE by andy: 2011.5.23
		* set character customization system region path setting
		* allow different projects share the same CCS implementation and separate region paths for asset management
		* @param slot: region slot id
		* @param path: new region path
		* NOTE: path string recorded in a static map <slot, path>
		*/
		static void SetCharacterRegionPath(int slot, const std::string& path);

		/** the default character skin and cartoon face texture size. 
		* @param nCharacterTexSize: the character texture size. default to 256. 512 is also fine. 
		* @param nCartoonFaceTexSize: the character texture size. default to 256. 128 is also fine.
		* @note: One must call SetCharRegionCoordinates afterwards to change the region accordingly. 
		*/
		static void SetCharTextureSize(int nCharacterTexSize, int nCartoonFaceTexSize);

		/** set char region coordinates. This function together with SetCharTextureSize makes CCS regions fully customizable via scripting interface.
		* however, the region index are predefined and can not be changed. 
		* @param nRegionIndex: see enumeration CharRegions. it must be smaller than NUM_REGIONS. 
		* @param xpos, ypos, xsize, ysize: see struct CharRegionCoords. defines a rect region within the skin or face texture. 
		*/
		static void SetCharRegionCoordinates(int nRegionIndex, int xpos, int ypos, int xsize, int ysize);

		//////////////////////////////////////////////////////////////////////////
		//
		// the following is for sun system
		//
		//////////////////////////////////////////////////////////////////////////

		/** set time of day in seconds. Use SetDayLength() to set the total number of minutes in a day.
		*@param time: time in seconds. This can be any non-negative value. (time%day_length) will be used in case time is longer than a day.
		*/
		static void SetTimeOfDay(float time);
		/** set standard time. see SetTimeOfDay()
		*@param time: always in the range [-1,1], 0 means at noon, -1 is morning. 1 is night. */
		static void SetTimeOfDaySTD(float time);

		/** get the current time of day in seconds. */
		static float GetTimeOfDay();

		/** get standard time. see GetTimeOfDay()
		*@return: always in the range [-1,1], 0 means at noon, -1 is morning. 1 is night. */
		static float GetTimeOfDaySTD();

		/** set the maximum sun angle with the Y (up) axis.*/
		static void SetMaximumAngle(float fMaxAngle);

		/** advance time is in seconds, this will also change the light direction, sun color and sun ambient color.
		* this function is automatically called by the environment simulator
		*/
		static float AdvanceTimeOfDay(float timeDelta);

		/** set how many minutes are there in a day, this is used in time simulation.
		* default value is 300. 
		*/
		static void SetDayLength(float fMinutes);

		/** return how many minutes are there in a day in minutes*/
		static float GetDayLength();
		//////////////////////////////////////////////////////////////////////////
		//
		// global effect and light settings
		//
		//////////////////////////////////////////////////////////////////////////
		/** the current shadow method:both shadow volume and shadow mapping could be set. 
		* @param nMethod:
		* 0: turn off all shadow rendering: this will greatly improve performance
		* 1: turn on shadow using shadow volume
		* 2: turn on shadow mapping
		* 3: turn on both shadow volume and shadow mapping
		*/
		static void SetShadowMethod(int nMethod);
		static int GetShadowMethod();

		/** Enable both global and local lighting. Turn off lighting will greatly improve performance, such as on slower computers*/
		static void EnableLighting(bool bEnable);
		static bool IsLightingEnabled();

		/**
		* set the global fog effect
		* @param bRenderFog: 1 to enable fog.
		* @param strFogColor: a string of RGB value in the format "%f %f %f", such as "1.0 1.0 1.0", value must be in the range [0, 1.0].
		* @param fFogDensity: between (0,1)
		* @param fFogStart: unit in meters. 
		* @param fFogEnd: unit in meters. 
		*/
		static void SetFog(bool bRenderFog, const char* strFogColor, float fFogStart,float fFogEnd, float fFogDensity);

		//////////////////////////////////////////////////////////////////////////
		//
		// mini scene graph
		//
		//////////////////////////////////////////////////////////////////////////

		/** If the mini scene graph does not exist, it will be created */
		static ParaMiniSceneGraph  GetMiniSceneGraph(const char* name);

		/** Delete mini scene graphs by name.  if name is "" or "*", all mini scene graphs will be deleted.
		* @return: the number of graphs deleted is returned. 
		*/
		static int DeleteMiniSceneGraph(const char* name);

		/** enable to render all mini scene graphs */
		static void EnableMiniSceneGraph(bool bEnable);

		/** enable to render all mini scene graphs */
		static bool IsMiniSceneGraphEnabled();

		/** get the post processing mini scene graph. 
		*@return: this is same as GetMiniSceneGraph("_ps")*/
		static ParaMiniSceneGraph GetPostProcessingScene();

		/** set whether post processing is enabled. 
		* @param bEnable: bool
		* @param sCallbackScript: if bEnabled is true, this is a callback script to be called per frame at which one can process scene after the main 3d scene is rendered. 
		* if this is NULL, the old callback script is used. 
		*/
		static void EnablePostProcessing(bool bEnable, const char* sCallbackScript);

		/** get whether post processing is enabled.*/
		static bool IsPostProcessingEnabled();

		/**
		* Get screen 2D position(x,y) from a 3d world point. 
		* @param x,y,z: in world coordinate system. 
		* @param output: output table, containing additional info, such as 
		* {x=screen position x,y=screen position y,z= depth in the 0,1 range,  visible=bool:is point is visible in camera frustum, distance=float:distance from eye to the object, }
		* @return true if the point is visible in screen. 
		*/
		static bool GetScreenPosFrom3DPoint(float x, float y, float z, const object& output);

		/** set the max number of objects to be rendered of a given render importance. 
		* One can set the render importance of any objects by setting its "RenderImportance" property
		* @param nRenderImportance: the render importance to set. All objects are by default set with render importance 0. 
		* @param nCount: the max number to set for the above render importance. 
		*/
		static void SetMaxRenderCount(int nRenderImportance, int nCount);

		/** Get the max number of objects to be rendered of a given render importance. 
		* One can set the render importance of any objects by setting its "RenderImportance" property
		* @param nRenderImportance: the render importance to set. All objects are by default set with render importance 0. 
		* @param nCount: the max number to set for the above render importance. 
		*/
		static int GetMaxRenderCount(int nRenderImportance);
	};
};
