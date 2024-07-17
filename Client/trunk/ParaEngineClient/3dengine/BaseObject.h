#pragma once
#include "IAttributeFields.h"
#include "IViewClippingObject.h"
#include "IRefObject.h"
#include "IHeadOn3D.h"
#include "IObjectScriptingInterface.h"
#include <list>
#include <vector>
#include "unordered_ref_array.h"
#include "EffectFileHandles.h"
#include "ObjectEvent.h"

namespace ParaEngine
{
	class IParaDebugDraw;

	/** object volume bit fields
	* here is some example:\n
	* scene root:		OBJ_VOLUMN_ISOLATED | OBJ_VOLUMN_FREESPACE
	* floor/terrain:	OBJ_VOLUMN_ISOLATED | OBJ_VOLUMN_FREESPACE | OBJ_VOLUMN_CONTAINER
	* biped:			OBJ_VOLUMN_SENSOR
	* dummy:			[OBJ_VOLUMN_SENSOR]		e.g. Like walls, chairs, gate, monitors/sensors
	* skybox:			OBJ_VOLUMN_ISOLATED | OBJ_VOLUMN_FREESPACE
	* house:			OBJ_VOLUMN_CONTAINER | OBJ_VOLUMN_FREESPACE
	* houseWalls:		see dummy.

	View culling principles:\n
	1.draw obj only when it's in the view sphere
	2.do not draw child nodes of a container if the container is out of the view
	3.When object is in view, we will not draw the _House object itself, if the player is
	in this _House.  However, we will draw the inside content of this _House. If it's not
	a house container, both this object and its child nodes are drawn.
	4.When object is in view, we will draw the container object, but not its child
	nodes, if the player is not in the container.

	Collision detection principles:\n
	1. add to collision pair only if it's a sensor.
	2. child nodes are also tested if the player is in OBJ_VOLUMN_FREESPACE | OBJ_VOLUMN_CONTAINER
	3. do not need to check the child of contain if the container does not contains the object
	*/
	enum OBJECT_ATTRIBUTE
	{
		/// two solid objects with sensor volume will cause environment simulator to
		/// to generate sensor/collision event when they come in to contact.
		OBJ_VOLUMN_SENSOR = 1,
		/// all child objects are in this object's volume 
		OBJ_VOLUMN_CONTAINER = 0x1 << 1,
		/// solid objects(like biped) can be placed on its volume, provided 
		/// it's not already occupied by any solid objects from its children
		/// when we solve two solid object collision, this is the field we check first.
		OBJ_VOLUMN_FREESPACE = 0x1 << 2,
		/// whether the object is isolated from its siblings. An isolated object
		/// can overlap in physical space with all its siblings regardless of their solidity.
		/// multiple scenes or terrains can be declared as ISOLATED object. Note, the object
		/// is not isolated from its parent, though.
		OBJ_VOLUMN_ISOLATED = 0x1 << 3,
		/// the object has a perceptive radius that may be larger than the object's 
		/// collision radius. Currently only IGameObject(biped) object might has this volume type
		OBJ_VOLUMN_TILE_VISITOR = 0x1 << 4,
		/// objects with this VIP volume type will trigger the plot of the scene in its view-culling radius.
		OBJ_VOLUMN_VIP = 0x1 << 5,
		/// Object invisible, the object is not drawn.but its physics may load. added by lxz 2006.3.5
		OBJ_VOLUMN_INVISIBLE = 0x1 << 6,
		/** mask of the above bits. this field is never used externally. */
		VOLUMN_MASK = 0x7f,
		/** whether lights have effects on this object.*/
		MESH_USE_LIGHT = 0x1 << 7,
		/** whether to rotate the object around Y axis to let the object always facing the camera.*/
		MESH_BILLBOARDED = 0x1 << 8,
		/** whether it is a shadow receiver.*/
		MESH_SHADOW_RECEIVER = 0x1 << 9,
		/** whether it is not a shadow caster.*/
		MESH_SHADOW_NOT_CASTER = 0x1 << 10,
		/** whether it is a vegetation.*/
		MESH_VEGETATION = 0x1 << 11,
		/** whether show bounding box.*/
		OBJ_SHOW_BOUNDINGBOX = 0x1 << 12,
		/** this is a very big and non-movable object. For big objects, we will attach them to the terrain tile that best contains them;
		and for everything else(small) we will attach to the root terrain tile. */
		OBJ_BIG_STATIC_OBJECT = 0x1 << 13,
		/** this object is should be skipped early during render queue building. But it is included during scene picking.
		* usually OBJ_SKIP_RENDER and OBJ_VOLUMN_INVISIBLE determines whether or not an object should be put to a render queue.
		*/
		OBJ_SKIP_RENDER = 0x1 << 14,
		/** this object should be skipped in all picking functions. this is not enabled by default
		*/
		OBJ_SKIP_PICKING = 0x1 << 15,
		/** if specified, this character will always be perpendicular to the ground, regardless to the terrain normal. this is usually true for tall thin biped players.
		*/
		OBJ_SKIP_TERRAIN_NORMAL = 0x1 << 16,
		/* whether the object will be rendered with a custom renderer. The default draw function will do nothing. */
		OBJ_CUSTOM_RENDERER = 0x1 << 17,
	};

	/** how physics is implemented in this object. */
	enum PHYSICS_METHOD {
		/** only load physics when a dynamic physics object enters its bounding box.
		* and automatically unload during a garbage collection.it is mutually exclusive with PHYSICS_ALWAYS_LOAD */
		PHYSICS_LAZY_LOAD = 1,
		/** load physics when this object is created. Never unload physics. it is mutually exclusive with PHYSICS_LAZY_LOAD*/
		PHYSICS_ALWAYS_LOAD = 0x1 << 1,
		/** this object has no physics. if this bit is on, it will override all other methods. */
		PHYSICS_FORCE_NO_PHYSICS = 0x1 << 2,
		/** if this bit is on, the object's physics data is read from the mesh entity (i.e. mesh faces with no textured material).
		* if not on, the entire mesh entity is used as the physics data.*/
		PHYSICS_LOAD_FROM_ASSET = 0x1 << 3,
		// 动态物理标识 
		PHYSICS_FORCE_DYNAMIC = 0x1 << 4
	};

	/**
	* type of the scene objects
	*/
	enum ObjectType
	{
		_undefined = 0,
		/// CBaseObject
		_Floor = 2,
		_Sky = 3,
		/// extensions for HAPI creation, main terrain mesh(can be very large)
		_MeshTerrain = 4,
		_Scene = 5,
		_LocalLight = 6,

		/// portal rendering object
		_PC_Zone = 7,
		_PC_Portal = 8,
		_PC_AntiPortal = 9,

		/** above this value must be drawn */
		_Tile = 20,

		_Sensor = 22,
		_House = 23,
		/// extensions for HAPI creation, added 2004-4-30 by LiXizhi
		_SolidHouse = 24,
		/// extensions for HAPI creation, added 2004-5-20 by LiXizhi
		_SolidLightedHouse = 25,
		/// solid mesh such as simple houses, stone, etc
		_SolidMesh = 26,
		/// Dummy mesh that does not collide with any object, such as grass,etc
		_DummyMesh = 27,
		/// mini scene graph
		_MiniSceneGraph = 28,
		// voxel mesh from a 3d grid.
		_VoxelMesh = 29,

		/**
		* below are types of animation object that at least derived from CBipedObject
		* and is animatable. Currently MDX file is the underlying asset for those anim mesh
		* if can simply test ObjectType > _Biped, to see whether it is animated mesh.
		*/
		/// below this value are all object derived from CBipedObject
		/// it might be static mesh, NPC, OPC, creatures, players,...
		_Biped = 40,
		/// animated mesh object that does not collide with any object, such as bird and grass
		_DummyAnimMesh = 41,
		/// animated mesh object that DOES collide with other object, such as wall and trees.
		_SolidAnimMesh = 42,
		/** below are all Neuron objects */
		/// static passive NPC object that is distributed in the world
		_NPC = 50,

		/// above this value are all global bipeds, as apposed to regional biped
		_GlobalBiped = 60,
		/// Controllable only through LLE(commands) from the network. 
		/// may or may not be in the collision detection list. 
		_OPC,
		_Team,
		_Creature,
		_Player,
		_RPG, // typical RPG object
		_UnknownGlobal,

		/// CCameraObject
		_CameraFollowPerson = 100,
		_CameraThirdPerson = 101,
		_CameraFirstPerson = 102,
	};

	/** how to render a selected object */
	enum RenderSelectionStyle
	{
		/// just appear brighter without applying light effect and diffuse color is 1. 
		RenderSelectionStyle_unlit = 0,
		/// a yellow border around the biped. 
		RenderSelectionStyle_border = 1,
		/// flashing the biped. Not implemented. 
		RenderSelectionStyle_flash = 2,
	};


	struct AssetEntity;
	/**
	* Defines the base class of all scene elements:CBaseObject for Parallel World
	*		 Engine. It is generally an abstract class, which means that it doe snot contain
	*		 data structure. However, it provides many virtual functions that may be called
	*		 many times by other modules(such as Environment Simulator and AI Simulator ),
	*       so that inherited class could has their own implementations.
	*
	*		 The virtual functions falls into several class:
	*       (1) object property query: like position, facing, radius,height, volume type, etc.
	*       (2) object collision detection: like TestCollisionObject, TestCollisionSphere, etc.
	*           it only contains functions that carry out the actual collision detection, NOT
	*           implementing any collision detection algorithm in large scale.
	*       (3) frame move: Animate(). Note this is only implemented by [meta]biped and sceneRoot objects
	*           which will actually handle events. All other scene objects' frame move will
	*           be handled by classes other than their own. Those classes may be
	*           environment simulator and game Interface and Rule controller.
	*       (4) Rendering: Almost all object use Draw() method to render itself. Some objects
	*           like Biped needs an additional AdvanceTime(). Since most of the time draw()
	*           method only draws a static scene, while AdvanceTime() will advance the animation
	*           frame used by an animation based object. Some animated object like Sprite does not
	*           have this AdvanceTime(); this is either because the frame advance needs very little
	*           computation time or its animation frame doe snot need to be precise. So Sprite object
	*           is treated like a static object, and it doe snot have AdvanceTime() like other animated
	*           ones.
	*
	*       Base object class also kept common and shared data structures and functions
	*       (1) Object Type and Identifier
	*       (2) tree nodes management: m_children
	*       (3) Event management
	*
	* Note: It is not suggested to have too many inherited class of this class in depth,
	*		 because it will induce too many data space waste. As you may see, I have used
	*		 as few data words as possible in this set of classes.
	* Note[2004/5/3]:  All inherited object should deallocate anything in the Cleanup() which is
	*    called by the destructor of the BaseObject. Because the destruction of any inherited class
	*    are not guaranteed to be called when it's deleted. Pay attention to any member object like(string)
	*    which may depends on the destructor to release its resources.
	*/
	class CBaseObject : public IViewClippingObject, public IHeadOn3D, public IObjectScriptingInterface
	{
	public:
		typedef ParaEngine::weak_ptr<IObject, CBaseObject> WeakPtr_type;

		// typedef std::list<CBaseObject*> CChildObjectList_Type;
		typedef unordered_ref_array<CBaseObject*> CChildObjectList_Type;
		typedef std::list<ObjectEvent> ObjectEventList_Type;

		enum _SceneObjectType
		{
			BaseObject = 0,
			ContainerObject = 0x1,
			MeshObject = 0x1 << 1,
			SkyMesh = 0x1 << 2,
			BipedObject = 0x1 << 3,
			SpriteObject = 0x1 << 4,
			SceneRoot = 0x1 << 5,
			OPCBiped = 0x1 << 6,
			RPGBiped = 0x1 << 7,
			MDXObject = 0x1 << 8,
			MeshPhysicsObject = 0x1 << 9,
			DynamicObject = 0x1 << 10,
			ViewCullingObject = 0x1 << 11,
			MissileObject = 0x1 << 12,
			GameObject = 0x1 << 13,
			LightObject = 0x1 << 14,
			MiniSceneGraph = 0x1 << 15,
			VoxelMesh = 0x1 << 16,
			ManagedLoader = 0x1 << 17,
			ZoneNode = 0x1 << 18,
			PortalNode = 0x1 << 19,
			AntiPortalNode = 0x1 << 20,
			CadObject = 0x1 << 21,
			BMaxObject = 0x1 << 22,
		};
		virtual CBaseObject::_SceneObjectType GetType() { return CBaseObject::BaseObject; };

		/** call back type */
		enum CallBackType {
			Type_EnterSentientArea = 0,
			Type_LeaveSentientArea,
			Type_Click,
			Type_Event,
			Type_Perception,
			Type_FrameMove,
			Type_Net_Send,
			Type_Net_Receive,
			Type_OnLoadScript,
			Type_OnAssetLoaded,
			Type_Paint,
			Type_AddRiggedFile,
			Type_Unknown,
		};

		/** by which method an object is converted to NPL string. */
		enum NPL_STRING {
			/// create the object in NPL
			NPL_CREATE = 0,
			/// create the object in managed loader in NPL
			NPL_CREATE_IN_LOADER = 0x1 << 1,
			/// Update the object in NPL, which is delete the object first, then create it again. this is useful to update the static physics if any. 
			NPL_UPDATE = 0x1 << 2,
			/// delete the object in NPL
			NPL_DELETE = 0x1 << 3,
			/// whether to suppress the asset creation code.
			NPL_DONOT_OUTPUT_ASSET = 0x1 << 4,
		};
		CBaseObject(void);
		virtual ~CBaseObject(void);

		/**
		* Clone the object's contains to a pointer.
		* The caller should allocate the memory and pass the pointer to this function..
		* Inheritance should implement this function
		* void Clone(IObject*) and IObject* Clone() should have the same behavior
		* In some cases, if you want to avoid Dead Reference( one object is being referred
		* to by many objects and you can't update all the references). It is possible to use
		* this function to replace the contains at a given pointer. But this may result in
		* memory leak if you do not implement this function carefully or misuse it.
		**/
		virtual void Clone(CBaseObject* obj);

		/**
		* Clone the object's contains and return a pointer to the newly created object.
		* The caller should free the memory of the return object.
		* Inheritance should implement this function
		* void Clone(IObject*) and IObject* Clone() should have the same behavior
		**/
		virtual CBaseObject* Clone();


		/** convert the object to an NPL string by which this object may be created or deleted.
		* @param nMethod: bit combination of CBaseObject::NPL_STRING
		*/
		virtual std::string ToString(DWORD nMethod);

		/** the ID of the object. The ID of the object is only generated when the first time this function is called.
		* One can then easily get the object, by calling GetObjectByID. When an object is released, its ID is not longer used.
		* Please note that the ID is neither unique outside the world, nor persistent in the same world.
		*/
		int GetID();

		/** GetObject By ID. it does not matter whether this object is attached or not. It will return NULL, if not found */
		static CBaseObject* GetObjectByID(int nID);

		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID() { return ATTRIBUTE_CLASSID_CBaseObject; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "CBaseObject"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** add child object. */
		virtual bool AddChildAttributeObject(IAttributeFields* pChild, int nRowIndex = -1, int nColumnIndex = 0);

		/** convert to object of a given type.
		* @param sObjectType:
		*/
		virtual void* QueryObjectByName(const std::string& sObjectType);

		/** convert to object of a given type.
		* @param nObjectType: such as values in ATTRIBUTE_CLASSID_TABLE
		*/
		virtual void* QueryObject(int nObjectType);

		ATTRIBUTE_METHOD1(CBaseObject, GetID_s, int*) { *p1 = cls->GetID(); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsGlobal_s, bool*) { *p1 = cls->IsGlobal(); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetFacing_s, float*) { *p1 = cls->GetFacing(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetFacing_s, float) { cls->SetFacing(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetYaw_s, float*) { *p1 = cls->GetYaw(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetYaw_s, float) { cls->SetYaw(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetPitch_s, float*) { *p1 = cls->GetPitch(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetPitch_s, float) { cls->SetPitch(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetRoll_s, float*) { *p1 = cls->GetRoll(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetRoll_s, float) { cls->SetRoll(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetHeight_s, float*) { *p1 = cls->GetHeight(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetHeight_s, float) { cls->SetHeight(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetWidth_s, float*) { *p1 = cls->GetWidth(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetWidth_s, float) { cls->SetWidth(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetDepth_s, float*) { *p1 = cls->GetDepth(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetDepth_s, float) { cls->SetDepth(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetRadius_s, float*) { *p1 = cls->GetRadius(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetRadius_s, float) { cls->SetRadius(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetPosition_s, DVector3*) { *p1 = cls->GetPosition(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetPosition_s, DVector3) { cls->SetPosition(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetAssetFileName_s, const char**) { *p1 = cls->GetAssetFileName().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetAssetFileName_s, const char*) { cls->SetAssetFileName(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetTechHandle_s, int*) { *p1 = cls->GetPrimaryTechniqueHandle(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetTechHandle_s, int) { cls->SetPrimaryTechniqueHandle(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetCtorPercentage_s, float*) { *p1 = cls->GetCtorPercentage(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetCtorPercentage_s, float) { cls->SetCtorPercentage(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetHomeZone_s, const char**) { *p1 = cls->GetHomeZoneName(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetHomeZone_s, const char*) { cls->SetHomeZoneName(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetShowBoundingBox_s, bool*) { *p1 = cls->IsShowBoundingBox(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetShowBoundingBox_s, bool) { cls->ShowBoundingBox(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetPhysicsGroup_s, int*) { *p1 = cls->GetPhysicsGroup(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetPhysicsGroup_s, int) { cls->SetPhysicsGroup(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetPhysicsGroupMask_s, DWORD*) { *p1 = cls->GetPhysicsGroupMask(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetPhysicsGroupMask_s, DWORD) { cls->SetPhysicsGroupMask(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetSelectGroupIndex_s, int*) { *p1 = cls->GetSelectGroupIndex(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetSelectGroupIndex_s, int) { cls->SetSelectGroupIndex(p1); return S_OK; }

		ATTRIBUTE_METHOD(CBaseObject, Reset_s) { cls->Reset(); return S_OK; }

		DEFINE_SCRIPT_EVENT_GET(CBaseObject, OnAssetLoaded);
		ATTRIBUTE_METHOD1(CBaseObject, SetOnAssetLoaded_s, const char*) { cls->SetOnAssetLoaded(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetViewTouch_s, bool*) { *p1 = cls->ViewTouch(); return S_OK; }
		ATTRIBUTE_METHOD(CBaseObject, UpdateGeometry_s) { cls->UpdateGeometry();  return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetRenderOrder_s, float*) { *p1 = cls->GetRenderOrder(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetRenderOrder_s, float) { cls->SetRenderOrder(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetObjectToCameraDistance_s, float*) { *p1 = cls->GetObjectToCameraDistance(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetObjectToCameraDistance_s, float) { cls->SetObjectToCameraDistance(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetRenderImportance_s, int*) { *p1 = cls->GetRenderImportance(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetRenderImportance_s, int) { cls->SetRenderImportance(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetRenderDistance_s, float*) { *p1 = cls->GetRenderDistance(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetRenderDistance_s, float) { cls->SetRenderDistance(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetAnimation_s, int*) { *p1 = cls->GetAnimation(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetAnimation_s, int) { cls->SetAnimation(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetUpperAnimation_s, int*) { *p1 = cls->GetUpperAnimation(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetUpperAnimation_s, int) { cls->SetUpperAnimation(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetAnimFrame_s, int*) { *p1 = cls->GetAnimFrame(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetAnimFrame_s, int) { cls->SetAnimFrame(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsAnimEnabled_s, bool*) { *p1 = cls->IsAnimEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, EnableAnim_s, bool) { cls->EnableAnim(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsUseGlobalTime_s, bool*) { *p1 = cls->IsUseGlobalTime(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetUseGlobalTime_s, bool) { cls->SetUseGlobalTime(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetChildCount_s, int*) { *p1 = (int)(cls->GetChildren().size()); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetNormal_s, Vector3*) { *p1 = cls->GetNormal(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetNormal_s, Vector3) { cls->SetNormal(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsTransparent_s, bool*) { *p1 = cls->IsTransparent(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetTransparent_s, bool) { cls->SetTransparent(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsVisible_s, bool*) { *p1 = cls->IsVisible(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetVisibility_s, bool) { cls->SetVisibility(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsShadowCaster_s, bool*) { *p1 = cls->IsShadowCaster(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetShadowCaster_s, bool) { cls->SetShadowCaster(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsShadowReceiver_s, bool*) { *p1 = cls->IsShadowReceiver(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetShadowReceiver_s, bool) { cls->SetShadowReceiver(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsBillboarded_s, bool*) { *p1 = cls->IsBillboarded(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetBillboarded_s, bool) { cls->SetBillboarded(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsSkipRender_s, bool*) { *p1 = cls->IsSkipRender(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetSkipRender_s, bool) { cls->SetSkipRender(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsSkipPicking_s, bool*) { *p1 = cls->IsSkipPicking(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetSkipPicking_s, bool) { cls->SetSkipPicking(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsSkipTerrainNormal_s, bool*) { *p1 = cls->IsSkipTerrainNormal(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetSkipTerrainNormal_s, bool) { cls->SetSkipTerrainNormal(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetSelectionEffect_s, int*) { *p1 = cls->GetSelectionEffect(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetSelectionEffect_s, int) { cls->SetSelectionEffect(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsHeadOnZEnabled_s, bool*) { *p1 = cls->IsHeadOnZEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetHeadOnZEnabled_s, bool) { cls->SetHeadOnZEnabled(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsHeadOnSolid_s, bool*) { *p1 = cls->IsHeadOnSolid(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetHeadOnSolid_s, bool) { cls->SetHeadOnSolid(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsHeadOn3DScalingEnabled_s, bool*) { *p1 = cls->IsHeadOn3DScalingEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetHeadOn3DScalingEnabled_s, bool) { cls->SetHeadOn3DScalingEnabled(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsHeadOnUseGlobal3DScaling_s, bool*) { *p1 = cls->IsHeadOnUseGlobal3DScaling(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetHeadOnUseGlobal3DScaling_s, bool) { cls->SetHeadOnUseGlobal3DScaling(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetHeadOnNearZoomDist_s, float*) { *p1 = cls->GetHeadOnNearZoomDist(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetHeadOnNearZoomDist_s, float) { cls->SetHeadOnNearZoomDist(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetHeadOnFarZoomDist_s, float*) { *p1 = cls->GetHeadOnFarZoomDist(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetHeadOnFarZoomDist_s, float) { cls->SetHeadOnFarZoomDist(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetHeadOnMinUIScaling_s, float*) { *p1 = cls->GetHeadOnMinUIScaling(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetHeadOnMinUIScaling_s, float) { cls->SetHeadOnMinUIScaling(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetHeadOnMaxUIScaling_s, float*) { *p1 = cls->GetHeadOnMaxUIScaling(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetHeadOnMaxUIScaling_s, float) { cls->SetHeadOnMaxUIScaling(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetHeadOnAlphaFadePercentage_s, float*) { *p1 = cls->GetHeadOnAlphaFadePercentage(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetHeadOnAlphaFadePercentage_s, float) { cls->SetHeadOnAlphaFadePercentage(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetHeadOn3DFacing_s, float*) { *p1 = cls->GetHeadOn3DFacing(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetHeadOn3DFacing_s, float) { cls->SetHeadOn3DFacing(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsPersistent_s, bool*) { *p1 = cls->IsPersistent(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetPersistent_s, bool) { cls->SetPersistent(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsDead_s, bool*) { *p1 = cls->IsDead(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetDead_s, bool) { cls->SetDead(); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsTileObject_s, bool*) { *p1 = cls->IsTileObject(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetTileObject_s, bool) { cls->SetTileObject(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetFrameNumber_s, int*) { *p1 = cls->GetFrameNumber(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetFrameNumber_s, int) { cls->SetFrameNumber(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetOpacity_s, float*) { *p1 = cls->GetOpacity(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetOpacity_s, float) { cls->SetOpacity(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsLastFrameRendered_s, bool*) { *p1 = cls->IsLastFrameRendered(); return S_OK; }
		ATTRIBUTE_METHOD(CBaseObject, DestroyChildren_s) { cls->DestroyChildren(); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetRenderMatrix_s, Matrix4*) { cls->GetRenderMatrix(*p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, SetLocalTransform_s, const Matrix4&) { cls->SetLocalTransform(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, GetLocalTransform_s, Matrix4*) { cls->GetLocalTransform(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsPhysicsEnabled_s, bool*) { *p1 = cls->IsPhysicsEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, EnablePhysics_s, bool) { cls->EnablePhysics(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsDynamicPhysicsEnabled_s, bool*) { *p1 = cls->IsDynamicPhysicsEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, EnableDynamicPhysics_s, bool) { cls->EnableDynamicPhysics(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetPhysicsShape_s, const char**) { *p1 = cls->GetPhysicsShape(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetPhysicsShape_s, const char*) { cls->SetPhysicsShape(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetPhysicsProperty_s, const char**) { *p1 = cls->GetPhysicsProperty(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetPhysicsProperty_s, const char*) { cls->SetPhysicsProperty(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, ApplyCentralImpulse_s, Vector3) { cls->ApplyCentralImpulse(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, IsLODEnabled_s, bool*) { *p1 = cls->IsLODEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, EnableLOD_s, bool) { cls->EnableLOD(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBaseObject, GetMaterialId_s, int*) { *p1 = cls->GetMaterialId(); return S_OK; }
		ATTRIBUTE_METHOD1(CBaseObject, SetMaterialId_s, int) { cls->SetMaterialId(p1); return S_OK; }

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const char* sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);
	public:

		/** whether this object is global. By default,the type info is used to determine this.*/
		virtual bool IsGlobal();
		inline bool IsBiped() { return m_objType >= _Biped; };

		/** this may return NULL, if the object does not implement the IGameObject interface. */
		virtual IGameObject* QueryIGameObject();
		virtual IAttributeFields* GetAttributeObject();

		/** @obsolete Use CheckAttribute() */
		inline bool CheckVolumnField(DWORD volType) { return CheckAttribute(volType); };

		virtual TextureEntity* GetTexture();

		/**
		* whether an object attribute is enabled.
		* @param attribute
		* @return true if enabled
		*/
		inline bool CheckAttribute(DWORD attribute) {
			return (m_dwAttribute & attribute) > 0;
		}
		/**
		* enable or disable a given attribute.
		* @param dwAtt
		* @param bTurnOn: true to turn on, false to turn off.
		*/
		inline void SetAttribute(DWORD dwAtt, bool bTurnOn = true) {
			if (bTurnOn)
				m_dwAttribute |= dwAtt;
			else
				m_dwAttribute &= (~dwAtt);
		}

		/**
		* @param bBillboarded if true it will be billboarded. default value is false.
		*/
		inline void SetBillboarded(bool bBillboarded) {
			SetAttribute(MESH_BILLBOARDED, bBillboarded);
		}
		/**
		* if the model asset file name ended with "_b" it will be treated as billboarded. all billboarded mesh are not physical mesh.
		* @return true if billboarded.
		*/
		inline bool IsBillboarded() {
			return CheckAttribute(MESH_BILLBOARDED);
		}

		/**
		* @param true it is shadow caster. default value is false.
		*/
		inline void SetShadowReceiver(bool bEnable) {
			SetAttribute(MESH_SHADOW_RECEIVER, bEnable);
		}

		/**
		* if the model asset file name ended with "_r" it will be treated as shadow receiver.
		* @return true if it is shadow receiver
		*/
		inline bool IsShadowReceiver() {
			return CheckAttribute(MESH_SHADOW_RECEIVER);
		}

		/**
		* set whether it is shadow caster. default value is true.
		* if the model asset file name ended with "_e" it will be treated as not a shadow caster.
		*/
		inline void SetShadowCaster(bool bEnable) {
			SetAttribute(MESH_SHADOW_NOT_CASTER, !bEnable);
		}

		/**
		* @return true if it is shadow caster. default value is true.
		*/
		inline bool IsShadowCaster() {
			return !CheckAttribute(MESH_SHADOW_NOT_CASTER);
		}

		/** whether to render this mesh using the vegetation shader*/
		inline void SetVegetation(bool bIsVegetation)
		{
			SetAttribute(MESH_VEGETATION, bIsVegetation);
		}

		/** whether to render this mesh using the vegetation shader*/
		inline bool IsVegetation() {
			return CheckAttribute(MESH_VEGETATION);
		}

		/**
		* set whether to show bounding box for this object.
		*/
		inline void ShowBoundingBox(bool bShowBoundingBox) {
			SetAttribute(OBJ_SHOW_BOUNDINGBOX, bShowBoundingBox);
		}
		/**
		* get whether to show bounding box for this object.
		*/
		inline bool IsShowBoundingBox() {
			return CheckAttribute(OBJ_SHOW_BOUNDINGBOX);
		}

		/** get the home zone of this object if any. it may return NULL, if zone is not visible.*/
		CZoneNode* GetHomeZone();

		/** set the home zone of this object if any. it may return NULL, if zone is not visible.
		* @param pZone: if this is NULL, it will remove the zone.
		*/
		void SetHomeZone(CZoneNode* pZone);

		/** invisible object will not be drawn. e.g. one can turn off the visibility of physics object. */
		bool IsVisible();
		/** set the visibility of this object. The visibility will recursively affect all its child objects. */
		void SetVisibility(bool bVisible);

		/** whether this object can be rendered. It should be both visible and no skip render. */
		inline bool IsRenderable()
		{
			return !CheckAttribute(OBJ_SKIP_RENDER | OBJ_VOLUMN_INVISIBLE);
		}

		/** set whether object should be excluded from the skip render */
		inline void SetSkipRender(bool bSkipRender) {
			SetAttribute(OBJ_SKIP_RENDER, bSkipRender);
		}

		/** get whether object should be excluded from the render queue. */
		inline bool IsSkipRender() {
			return CheckAttribute(OBJ_SKIP_RENDER);
		}

		/** set whether object should be excluded from picking */
		inline void SetSkipPicking(bool bSkipPicking) {
			SetAttribute(OBJ_SKIP_PICKING, bSkipPicking);
		}

		/** get whether object should be excluded from picking. */
		inline bool IsSkipPicking() {
			return CheckAttribute(OBJ_SKIP_PICKING);
		}

		/** if specified, this character will always be perpendicular to the ground, regardless to the terrain normal.
		this is usually true for tall thin biped players.
		*/
		void SetSkipTerrainNormal(bool bSkip);

		/** if specified, this character will always be perpendicular to the ground, regardless to the terrain normal.
		this is usually true for tall thin biped players.
		*/
		bool IsSkipTerrainNormal();

		/** get whether object can be picked by mouse picking. */
		bool CanPick();

		/** if the object may contain physics*/
		virtual bool CanHasPhysics();
		/**
		* load the physics objects.
		*/
		virtual void LoadPhysics();

		/**
		* load the physics object
		*/
		virtual void UnloadPhysics();

		/** this function will turn on or off the physics of the object. */
		virtual void EnablePhysics(bool bEnable);

		/** by default physics is lazy-load when player walk into its bounding box, setting this to false will always load the physics.
		* Please note, one must EnablePhysics(true) before this one takes effect.
		*/
		virtual void SetAlwaysLoadPhysics(bool bEnable);

		virtual bool IsPhysicsEnabled();

		// 设置物理基本形状 默认为box(AABB)
		virtual void SetPhysicsShape(const char* shape) {}
		virtual const char* GetPhysicsShape() { return "box"; }
		virtual void SetPhysicsProperty(const char* property) {}
		virtual const char* GetPhysicsProperty() { return "{}"; }
		// 是否启用动态物理
		virtual void EnableDynamicPhysics(bool bEnable) {}
		virtual bool IsDynamicPhysicsEnabled() { return false; }
		// 设置物理速度
		virtual void ApplyCentralImpulse(const Vector3& impulse) {}

		/** this function is called, when the object is in view range. we may need to load the primary asset to update the bounding box, etc.
		* @return true if the object is ready to be rendered.
		*/
		virtual bool ViewTouch();

		/** the larger, the more important. default to 0. all objects with 0 or positive number will always be rendered.
		* objects with negative numbers will not be rendered if there are too many objects in the scene.
		* @param nRenderImportance: for example,we can set player to 2, NPC to 1, friend OPC to -1, non-friend OPC to -2, etc
		*/
		inline void SetRenderImportance(int nRenderImportance) { m_nRenderImportance = nRenderImportance; }
		inline int GetRenderImportance() { return m_nRenderImportance; }

		/** 0 if automatic, larger number renders after smaller numbered object. The following numbers are fixed in the pipeline and may subject to changes in later versions.
		[1.0-2.0): solid big objects
		[2.0-3.0): solid small objects
		[3.0-3.0): sprites
		[4.0-4.0): characters
		[5.0-5.0): selection
		[6.0-7.0): transparent object
		[100.0, ...): rendered last and sorted by m_fRenderOrder
		*/
		float GetRenderOrder() const { return m_fRenderOrder; }
		void SetRenderOrder(float val);

		/** force camera to object distance, when sorting the object. if 0.f (default value), we will use the actual camera to object distance instead. */
		virtual float GetObjectToCameraDistance();
		virtual void SetObjectToCameraDistance(float val);

		/// -- Base object functions
		CChildObjectList_Type& GetChildren() { return m_children; };

		/** get child by name: it may return NULL if the child does not exist.
		* @param bRecursive: if true, it will be recursive (depth first search), otherwise only the direct children is searched.
		*/
		CBaseObject* GetChildByName(const string& name, bool bRecursive = false);

		/**
		* Completely destroy child nodes from memory recursively. This is often called
		* by the root scene at application clean up
		*/
		virtual void					DestroyChildren();

		/**
		* destroy a child by name. It returns the number of nodes that are destroyed.
		* the current version will only destroy the first child with the given name.
		*/
		int DestroyChildByName(const string& name, bool bRecursive = false);

		/**
		* remove a child by name but it does not destroy it. It returns the number of nodes that are removed.
		* the current version will only remove the first child with the given name.
		*/
		int RemoveChildByName(const string& name, bool bRecursive = false);

		/**
		* remove a child by pointer but it does not destroy it. It returns the number of nodes that are removed.
		* the current version will only remove the first child with the given name.
		*/
		int RemoveChild(const CBaseObject* pObj, bool bRecursive = false);

		/** set the callback script whenever the primary asset is successfully loaded. */
		virtual void SetOnAssetLoaded(const char* sCallbackScript);


		/** Fire script callback
		* global variable sensor_name and sensor_id will be filled with GetName and GetID,
		* so that caller knows who is firing the callback script.
		* @param func_type: it should be type CallBackType
		* @param precode: script to be called before the callback script
		* @param postcode: script to be called after the callback script
		*/
		bool ActivateScript(int func_type, const string& precode, const string& postcode);
		bool ActivateScript(int func_type, const string& precode);
		bool ActivateScript(int func_type);

		/**
		* Set the object shape to box and bounding box parameters
		* if fFacing is 0, it is a axis aligned bounding box (AABB). The bounding sphere is set to
		* radius of bounding box's the smallest spherical container
		* @param fOBB_X: object bounding box.x
		* @param fOBB_Y: object bounding box.y
		* @param fOBB_Z: object bounding box.z
		* @param fFacing: rotation of the bounding box around the y axis.
		*/
		virtual void SetBoundingBox(float fOBB_X, float fOBB_Y, float fOBB_Z, float fFacing);

		/**
		* @see SetBoundingBox(float fOBB_X, float fOBB_Y, float fOBB_Z, float fFacing);
		*/
		virtual void GetBoundingBox(float* fOBB_X, float* fOBB_Y, float* fOBB_Z, float* fFacing);

		/**
		* Set the object shape to rect (a plane) and rect parameters
		* if fFacing is 0, it is a axis aligned bounding box (AABB).
		* @param fWidth: plane width or x
		* @param fHeight: plane height or y.
		* @param fFacing: rotation of the plane around the y axis.
		*/
		virtual void SetBoundRect(float fWidth, float fHeight, float fFacing);
		/**
		* @see SetBoundRect(float fWidth ,float fHeight, float fFacing);
		*/
		virtual void GetBoundRect(float* fWidth, float* fHeight, float* fFacing);

		/**
		* Set the object shape to Sphere and sphere parameters
		* @param fRadius: radius
		*/
		virtual void SetRadius(float fRadius);

		/**
		* @see SetRadius(float fRadius);
		*/
		virtual float GetRadius();

		/** get the render effect when character is in selected state.
		* @return: 1 for yellow border style, 0 for unlit style. default to 0.
		*/
		virtual int GetSelectionEffect();

		/** get the render effect when character is in selected state.
		* @param nStyle: 1 for yellow border style, 0 for unlit style. default to 0.
		*/
		virtual void SetSelectionEffect(int nStyle);

		/** whether the object is persistent in the world. If an object is persistent, it will be saved to the world's database.
		if it is not persistent it will not be saved when the world closes. Player, OPC, some temporary movie actors may
		by non-persistent; whereas NPC are usually persistent to the world that it belongs.*/
		virtual bool IsPersistent();
		/** whenever a persistent object is made non-persistent, the SaveToDB() function will actually removed it from the database and the action can not be recovered.
		* so special caution must be given when using this function to prevent accidentally losing information.
		@see IsPersistent() */
		virtual void SetPersistent(bool bPersistent);

		virtual void SetOpacity(float fOpacity);
		virtual float GetOpacity();

		virtual HRESULT InitDeviceObjects();	// device independent
		virtual HRESULT RestoreDeviceObjects(); // device dependent
		virtual HRESULT InvalidateDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();
		virtual HRESULT RendererRecreated();
	public:
		/** we will not render this object if the object's position to camera eye position is further than this value.
		If this is 0 of negative, the global view clipping rule is applied. */
		virtual float GetRenderDistance();
		virtual void SetRenderDistance(float fDist);

		/** when batch-rendering a group of objects, objects are usually sorted by their render techniques and then by their primary asset.
		* One can access the effect file currently associated with the handle in the asset manager. Please note that, the game engine may change the mapping
		* from technique handle to the effect file, secretly at runtime. Effect files capable to be assigned to the same technique handle must
		* also share the same (vertex) input declaration in order to be swapped secretly at runtime.
		* Note: the Draw() function of object will dynamically retrieve the effect file for each call. If the effect file is not valid, it will try to render
		* using directx 9's fixed programming pipeline.
		* @see TechniqueHandle
		*/
		virtual int GetPrimaryTechniqueHandle();

		/**
		* Set a new render technique handle. If the effect file associated with the handle is invalid or do not share the same input declaration as
		* the object at the time of drawing, the object will not be drawn or will be drawn improperly.
		* This function rarely needs to be called by users. Technique handles are usually automatically assigned by the game engine when the asset file is loaded.
		* Of course, users can use special effect files for the rendering of special objects; then this function needs to be called.
		* @param nHandle: @see TechniqueHandle
		*/
		virtual void SetPrimaryTechniqueHandle(int nHandle);

		/** get effect parameter block with this object.
		* @param bCreateIfNotExist:
		*/
		virtual CParameterBlock* GetEffectParamBlock(bool bCreateIfNotExist = false);

		/**
		* whether the object contains transparent material. Ideally, we should perform polygon-level sorting for transparent faces;
		* however, at the moment, we just draw them last, after all solid objects are drawn.
		* @return true if it is partially or completely transparent.
		*/
		virtual bool IsTransparent() { return false; };

		/**
		* Set whether the object is transparent.
		* @param bIsTransparent
		*/
		virtual void SetTransparent(bool bIsTransparent) {};

		/**
		* get the total number of replaceable textures, which is the largest replaceable texture ID.
		* but it does not mean that all ID contains valid replaceable textures.
		* This function can be used to quickly decide whether the model contains replaceable textures.
		* Generally we allow 32 replaceable textures per model.
		* @return 0 may be returned if no replaceable texture is used by the model.
		*/
		virtual int GetNumReplaceableTextures();

		/**
		* get the default replaceable texture by its ID. The default replaceable texture is the main texture exported from the 3dsmax exporter.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @return this may return NULL, if replaceable texture is not set before or ID is invalid.
		*/
		virtual TextureEntity* GetDefaultReplaceableTexture(int ReplaceableTextureID);

		/**
		* get the current replaceable texture by its ID.
		* if no replaceable textures is set before, this will return the same result as GetNumReplaceableTextures().
		* @note: This function will cause the mesh entity to be initialized.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @return this may return NULL, if replaceable texture is not set before or ID is invalid.
		*/
		virtual TextureEntity* GetReplaceableTexture(int ReplaceableTextureID);

		/**
		* set the replaceable texture at the given index with a new texture.
		* this function will succeed regardless whether the mesh is initialized. Hence it can be used at loading time.
		* because default instance of the mesh may use different replaceable texture set.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @param pTextureEntity The reference account of the texture entity will be automatically increased by one.
		* @return true if succeed. if ReplaceableTextureID exceed the total number of replaceable textures, this function will return false.
		*/
		virtual bool  SetReplaceableTexture(int ReplaceableTextureID, TextureEntity* pTextureEntity);

		/**
		* called when an object is selected.
		* @param nGroupID the group ID in to which the object is selected.
		*/
		virtual void OnSelect(int nGroupID);
		/** called whenever an object is un-selected.*/
		virtual void OnDeSelect();

		/** add another object as the child of this object */
		virtual void AddChild(CBaseObject* pObject);
		/** this function is called by parent class to set the child's parent.
		* in most cases, the child does not need to save parent, but some child class
		* can override this function to keep a weak reference of parent.
		*/
		virtual void SetParent(CBaseObject* pParent) {};
		virtual CBaseObject* GetParent() { return NULL; };

		/** whether this object should be removed some time in the future.
		* we usually call AddToDeadObjectPool(), and let the root scene to remove it safely in the next render frame move.
		* After all, it is up to the individual class to decide which action to take when setting dead.
		* For example, some implementation may remove it during prepare render.
		*/
		virtual bool IsDead();
		virtual void SetDead();

		/** When a child object wants to remove itself, it is usually not safe to do so immediately, instead
		* the object add itself to dead object pool for the root scene to safely remove it at the end of the frame. */
		virtual void AddToDeadObjectPool();

		/** clean up the object. So that the object is ready to be deleted */
		virtual void Cleanup() {};
		/** animate the model by a given delta time.
		* @param dTimeDelta: delta time in seconds
		* @param nRenderNumber: if 0 it means that it is always animated, otherwise we should only animate
		* if previous call of this function has a different render frame number than this one.
		*/
		virtual void Animate(double dTimeDelta, int nRenderNumber = 0);
		/// only for drawable objects
		virtual HRESULT Draw(SceneState* sceneState);
		/** derived class can override this function to place the object in to the render pipeline.
		* if this function return -1 by default, the SceneObject will automatically place the object into the render pipeline.
		* if return 0, it means the object has already placed the object and the scene object should skip this object.
		*/
		virtual int PrepareRender(CBaseCamera* pCamera, SceneState* sceneState);

		/** Get the specified attachment matrix of the current model.
		* this is usually for getting the mount point on a certain model, such as horses. It also works for static mesh with xrefed mountable objects.
		* @param pOut: output result
		* @param nAttachmentID: see ATTACHMENT_ID. default to 0, which is general mount point. ATT_ID_MOUNT1-9(20-28) is another mount points
		* @param nRenderNumber: if it is bigger than current calculated render number, the value will be recalculated. If 0, it will not recalculate
		* @return: NULL if not attachment found, otherwise it is pOut.
		*/
		virtual Matrix4* GetAttachmentMatrix(Matrix4& pOut, int nAttachmentID = 0, int nRenderNumber = 0);

		/** whether it has an attachment point
		* @param nAttachmentID: see ATTACHMENT_ID. default to 0, which is general mount point. ATT_ID_MOUNT1-9(20-28) is another mount points
		*/
		virtual bool HasAttachmentPoint(int nAttachmentID = 0);

		/** whether it has an attachment point
		* @param pOut: in world coordinate
		* @param nAttachmentID: see ATTACHMENT_ID. default to 0, which is general mount point. ATT_ID_MOUNT1-9(20-28) is another mount points
		* @param nRenderNumber: if it is bigger than current calculated render number, the value will be recalculated. If 0, it will not recalculate
		* @return if it does not exist, NULL is returned.
		*/
		virtual Vector3* GetAttachmentPosition(Vector3& pOut, int nAttachmentID = 0, int nRenderNumber = 0);

		/**
		* get biped state manager
		* @param bCreateOnDemand if true, we will attempt to create the manager if it is not created before.
		* @return it may return NULL if bCreateOnDemand is false.
		*/
		virtual CBipedStateManager* GetBipedStateManager(bool bCreateOnDemand = true) { return NULL; };

		/** set the physics group ID to which this object belongs to
		default to 0, must be smaller than 32.
		please see groups Mask used to filter shape objects. See #NxShape::setGroup
		- group 0 means physics object that will block the camera and player, such as building walls, big tree trunks, etc.
		- group 1 means physics object that will block the player, but not the camera, such as small stones, thin posts, trees, etc.
		*/
		virtual void SetPhysicsGroup(int nGroup) {};

		/** Get the physics group ID to which this object belongs to
		default to 0, must be smaller than 32.
		please see groups Mask used to filter shape objects. See #NxShape::setGroup
		- group 0 means physics object that will block the camera and player, such as building walls, big tree trunks, etc.
		- group 1 means physics object that will block the player and standard mouse picking, but not the camera, such as small stones, thin posts, trees, etc.
		- group 2 means physics object that will block the player, but not the camera or mouse picking, such as transparent wall to prevent user to walk on.
		*/
		virtual int GetPhysicsGroup() { return 0; };

		/** Set the current animation id
		* @param nAnimID: 0 is default standing animation. 4 is walking, 5 is running. more information, please see AnimationID */
		virtual void SetAnimation(int nAnimID) {};

		virtual void SetUpperAnimation(int nAnimID) {}

		/** set groups Mask used to filter physics objects, default to 0xffffffff*/
		virtual void SetPhysicsGroupMask(DWORD dwValue);

		/** get groups Mask used to filter physics objects, default to 0xffffffff*/
		virtual DWORD GetPhysicsGroupMask();

		/** get the scaling. */
		virtual int GetAnimation() { return 0; };

		virtual int GetUpperAnimation() { return -1; }

		/** set the current animation frame number relative to the beginning of current animation.
		* @param nFrame: 0 means beginning. if nFrame is longer than the current animation length, it will wrap (modulate the length).
		*/
		virtual void SetAnimFrame(int nFrame) {};

		/** get the current animation frame number relative to the beginning of current animation.  */
		virtual int GetAnimFrame() { return 0; };

		/** whether animation is enabled. by default this is true. During movie editing, we may disable animation, set animation frame explicitly by editor logics. */
		virtual void EnableAnim(bool bAnimated) {};
		virtual bool IsAnimEnabled() { return true; };

		/** whether to use global time to sync animation. Default to false.
		if true, all characters plays exactly the same frames if they are using the same animation file at all times,
		if false, each character advances its time frame separately according to their visibility in the scene.
		@note: animation is always looped once use global time
		*/
		virtual void SetUseGlobalTime(bool bUseGlobalTime) {};

		/** whether to use global time to sync animation. Default to false.
		if true, all characters plays exactly the same frames if they are using the same animation file at all times,
		if false, each character advances its time frame separately according to their visibility in the scene.
		@note: animation is always looped once use global time
		*/
		virtual bool IsUseGlobalTime() { return false; };

		/** get the radius for physics simulation. */
		virtual float GetPhysicsRadius();

		/** reset the object to its default settings.*/
		virtual void Reset() {};

		/** always bottom center of the bounding shape*/
		virtual DVector3 GetPosition();

		/** always bottom center of the bounding shape*/
		virtual void SetPosition(const DVector3& v);

		/** get object height y axis: only for object that has a Height property, usually is height of the bounding box */
		virtual float GetHeight();
		virtual void SetHeight(float fHeight);
		/** get asset height with scaling applied. */
		virtual float GetAssetHeight();

		/** get object width: x axis */
		virtual float GetWidth();
		virtual void SetWidth(float fWidth);

		/** get object depth: z axis */
		virtual float GetDepth();
		virtual void SetDepth(float fDepth);

		/* return the normal at the terrain position where the object is situated at its x,z plane.*/
		virtual Vector3 GetNormal();
		virtual void SetNormal(const Vector3& pNorm) {};

		/** Usually, this is used in canvas drawing. So that the same object can be used in different scene manager.
		Parameter includes: position, facing, facing target, speed, etc.
		* @note: must be paired with PopParam().
		* currently, it does not use a stack for storing parameters, but a simple set of globals. So only one level push/pop operation is supported.
		*/
		virtual void PushParam();

		/** Usually, this is used in canvas drawing. So that the same object can be used in different scene manager.
		Parameter includes: position, facing, facing target, speed, etc.
		* @note: must be paired with PushParam().
		* currently, it does not use a stack for storing parameters, but a simple set of globals. So only one level push/pop operation is supported.
		*/
		virtual void PopParam();

		/**
		* Set scaling. By default an object will be first scaled, then rotated and finally translated.
		* Not many objects support scaling. Only the mesh and mesh physics object supports full-dimensional object positioning.
		* I.e. rotation, norm, position, scaling.
		*/
		/*virtual void SetScaling(float x, float y, float z){};
		virtual void GetScaling(float& x, float& y, float& z){x=1.f;y=1.f;z=1.f;};*/

		/// used as KEY for batch rendering
		virtual AssetEntity* GetPrimaryAsset() { return NULL; };

		/** get asset file name */
		virtual const std::string& GetAssetFileName();

		/** set asset file name */
		virtual void SetAssetFileName(const std::string& sFilename);

		virtual bool IsStanding() { return true; };

		/** Get the object construction percentage (progress) in the range[0,1]. 0 means that the object has not been constructed.
		* 1 means that the object is fully constructed. This value can be used to inform user that how much an object has been loaded. */
		virtual float GetCtorPercentage() { return 1.0f; }
		/** Set the object construction percentage (progress) in the range[0,1]. 0 means that the object has not been constructed.
		* 1 means that the object is fully constructed. This value can be used to inform user that how much an object has been loaded. */
		virtual void SetCtorPercentage(float fPercentage) {}

		/** automatically select the proper technique for the rendering of the object.
		* This method is automatically called by the render pipeline before rendering any potentially visible object.
		* Please note that: When rendering some special pass, the render pipeline might select a different technique and then select back.
		*/
		virtual void AutoSelectTechnique();

		virtual void Report(vector < string >& v_sReport);
		virtual HRESULT ResetTime();

		/** return true if the object contains alpha blended render pass. This function determines which render pipeline stage the object is rendered.
		* generally speaking, if deferred shading is used, we will render alpha blended objects last.
		*/
		virtual bool HasAlphaBlendedObjects();

		enum CompressOption {
			ReleaseDevices = 1,
			ReleasePhysics = 2,
			ReleaseEvents = 4
		};
		/**
		* Compress the object to save memory.
		* Only compress an object if you believe if will not be needed for a while.
		* a compressed object can always be recovered automatically.
		*/
		virtual void CompressObject(CompressOption option = (CompressOption)0xffff);

		/**
		* return the view clipping object used for object-level clipping when rendering this object.
		*/
		virtual IViewClippingObject* GetViewClippingObject();

		/** this function is called to update the render frame number of this object.
		* please note that, the frame number increases by 1 every frame. This function is called at the earliest scene culling stage.
		* the object may be culled in later rendering stages. so one should not do much computation in this place.
		* in most cases, we only need to cache some derived data such as render coordinates for use in later rendering processes.
		*/
		virtual void UpdateFrameNumber(int nFrameNumber);
		void SetFrameNumber(int nFrameNumber);

		/** get the frame number that this object is last accessed.*/
		int GetFrameNumber();

		/** call this function whenever a render may render this object. it will call UpdateFrameNumber virtual function.
		* @return return true if nFrameNumber is different from the current frame number and set it to nFrameNumber before return..*/
		bool CheckFrameNumber(int nFrameNumber);

		/** check to see if the last frame is rendered. Internally it will check if FrameNumber of this object and the main scene are equal. */
		bool IsLastFrameRendered();

		/**
		* set the type of the object, it tells what this object is used for.
		* it will automatically set its volume field according to its type.
		*/
		void SetMyType(ObjectType t);
		ObjectType GetMyType();

		/**
		* call this function to reset the y component and norm of the object to the global terrain surface at (x,0,z),
		* where (x,y,z) is the object's current position.  if bUseNorm is true, the norm value is also changed.
		*/
		void SnapToTerrainSurface(bool bUseNorm = true);

		/** get the object shape */
		virtual ObjectShape GetObjectShape();

		/** set the object shape */
		virtual void SetObjectShape(ObjectShape shape);

		/** get the quad-tree terrain tile that this game object is currently visiting. */
		CTerrainTile* GetTileContainer();
		virtual void SetTileContainer(CTerrainTile* val);
		/** update the tile container according to the current position of the game object.
		* This function is automatically called when a global object is attached. */
		virtual void UpdateTileContainer();

		/** set the home zone name. if the zone does not exist, it will be created. */
		void SetHomeZoneName(const char* sName);
		const char* GetHomeZoneName();

		/** set the selection group index. if -1, it means that it was not selected.
		* @param nGroupIndex: selection group index.
		*/
		void SetSelectGroupIndex(int nGroupIndex = -1);

		/** get the selection group index. if -1, it means that it was not selected. */
		int GetSelectGroupIndex();

		/** if true, we will attach this object to quad-tree terrain tile according to its attributes when adding to the scene
		* if false, we will automatically attach it to a container object named after its class name.
		*/
		virtual bool IsTileObject();
		virtual void SetTileObject(bool bIsTileObject);


		/** whether the shape of the object is dirty, such as model, size, facing, local transform is changed. */
		inline bool IsGeometryDirty() const { return m_bGeometryDirty; }
		void SetGeometryDirty(bool bDirty = true);

		/** whether to enable lod if there is lod. Default to true. */
		bool IsLODEnabled() const;
		void EnableLOD(bool val);

		/** this function is usually called after asset file has changed. and bounding box needs to be recalculated. */
		virtual void UpdateGeometry();

		/** set local transform directly */
		virtual void SetLocalTransform(const Matrix4& mXForm);
		/** get local transform*/
		virtual void GetLocalTransform(Matrix4* localTransform);

		/** return triangle list */
		virtual int GetMeshTriangleList(std::vector<Vector3>& output, int nOption = 0);

		virtual void SetMaterialId(int materialId);
		virtual int GetMaterialId();
		virtual void ApplyMaterial();
	protected:
		/** the ID of the object. default to 0. it is regenerated automatically when GetID() is called and that the id is 0. */
		int m_nID;

		int m_nMaterialId;

		/** volume type and attribute of this object. @see OBJECT_ATTRIBUTE */
		DWORD					m_dwAttribute;

		/** child objects array: we keep strong references (ref counted) to child objects */
		CChildObjectList_Type		m_children;

		/** type of the object */
		ObjectType				m_objType;


		// the quad-tree terrain tile that this game object is currently visiting. This may be NULL, if object has not been attached.
		CTerrainTile* m_tileContainer;

		/** the primary technique handle*/
		int32 m_nTechniqueHandle;

		/** the frame number that this object is last accessed.*/
		int32 m_nFrameNumber;

		/** this is the selection group index. if -1, it means that it was not selected. */
		int32 m_nSelectGroupIndex;

		/** the larger, the more important. default to 0. only positive numbers are allowed.
		* During scene sorting, objects of the same group with larger render importance is rendered first, regardless of its camera to object distance
		* we can limit the max number of objects drawn of a given render importance by calling SceneObject::SetMaxRenderCount() function
		*/
		int32 m_nRenderImportance;

		/** 0 if automatic, larger number renders after smaller numbered object. The following numbers are fixed in the pipeline and may subject to changes in later versions.
		[1.0-2.0): solid big objects
		[2.0-3.0): solid small objects
		[3.0-3.0): sprites
		[4.0-4.0): characters
		[5.0-5.0): selection
		[6.0-7.0): transparent object
		[100.0, ...): rendered last and sorted by m_fRenderOrder
		*/
		float m_fRenderOrder;

		/** we will not render this object if the object's position to camera eye position is further than this value.
		If this is 0 of negative, the global view clipping rule is applied. default to 0.f */
		float m_fRenderDistance;

		/** effect param block that should be applied to m_nTechniqueHandle */
		CParameterBlock* m_pEffectParamBlock;

		/** whether the shape of the object is dirty, such as model, size, facing, local transform is changed. */
		bool m_bGeometryDirty : 1;
		/** whether to enable lod if there is lod. Default to true. */
		bool m_bEnableLOD : 1;
		/** enum of RenderSelectionStyle */
		static int g_nObjectSelectionEffect;
	};
}
