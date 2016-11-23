#pragma once
#include "IGameObject.h"
#include "ShadowVolume.h"
#include "BipedWayPoint.h"

namespace ParaEngine
{
	struct IParaPhysicsActor;

	/**
	*		It can be used to represent biped object(like human, re spawning monsters)
	*		 in the scene without inheriting and adding new functions.
	*
	*       This class and its inherited classes does some basic things to make the
	*		 element(or character) aware of its surroundings, as well as implements all actions
	*		 and movements.
	*
	*       CBipedObject provides many virtual functions that may be called many times by other
	*		 modules(such as Environment Simulator and AI Simulator ), so that inherited
	*		 class that override the parent implementation easily.
	*
	*       the biped can be associated with either MultiAnimationEntity or MDXEntity which will
	*       used to render the 3D animations.One of the AnimationInstanceBase derived class
	*		 should be used to control the animation of the biped object as well as the speed of
	*       the biped. AI controllers can also be associated with biped object, in which case
	*       the biped would act on their own.
	*		 The behavior of the biped is controlled through Events like all Base objects.
	*       High Level Event can be assigned to biped object to control almost everything about a
	*       biped object at run time, such as playing new animation, walking to a new position or
	*       even assigning AI tasks.
	*
	*
	* Note 1: It is not suggested to have too many inherited class of this class in depth,
	*		 because it will induce too many data space waste. As you may see, I have used
	*		 as few data words as possible in this set of classes.
	* Note 2: For mdx file animation entity. The Associated animation instance must contain
	*       animations "Stand" and "Walk"(if it ever moves).Some other animation name can be "Death", "Decay"
	*       These animation names are recognizable by the engine, and will be played when the user
	*       does not specifying them explicitly. For more information please see member functions.
	* Note 3: For x file animation entity: Biped has a set of predefined actions, but any single x file may not contain
	*		 all of them. So there is a tree like degradation structure. That is actions are
	*		 reorganized in a tree, with each action assigned to a tree node. Upon initializing
	*       those actions, we will start from the top of the tree. If a node(action) does not
	*		 find it's exact action in the x file, then its parent's action index is used. The
	*	     process is recursive. So that all actions will be assigned an action index in the
	*       index file. The worst case will be all nodes has index the same as the root node
	*       which is the loiter(idle) action.
	* The following is the predefined action.
	*       Stand-->Walk-->jog
	*                    -->swim
	*             -->jump-->jump forward/backward
	*                    -->sidestep right/left
	*             -->Speak-->say good bye
	*                     -->say hello
	*			   -->action0-->action1-->action2~action5
	*						 -->action6-->action7~action10
	* There is a constraint: if the parent is default, then all its descendants are set to default
	* regardless there is an exact animation in the file or not.
	*/
	class CBipedObject : public IGameObject, ShadowCaster
	{
	public:
		typedef ParaEngine::weak_ptr<IObject, CBipedObject> WeakPtr_type;

		virtual std::string ToString(DWORD nMethod);
		virtual CBaseObject::_SceneObjectType GetType(){ return CBaseObject::BipedObject; };

		typedef std::list<BipedWayPoint> WayPointList_type;

		/** how the character reacts to physics in the MoveTowards() functions*/
		enum PhysicsMoveStyle
		{
			/** character will slide along the wall infinitely. This is computationally most expensive.*/
			MOVESTYLE_SLIDINGWALL = 0,
			/** only care about the terrain(and model) height before its feet*/
			MOVESTYLE_HEIGHTONLY,
			/** no physics are applied, and make the character above terrain. It can stay higher than the terrain.*/
			MOVESTYLE_ABOVETERRAIN,
			/** just linear interpolate from current pos to target pos, without using any physics. */
			MOVESTYLE_LINEAR,
			/** OPC is the default for remote player agent. the behavior is simple. if it is too far away from destination, move linearly but with physics height.
			* when it is close to destination, only move according to MOVESTYLE_HEIGHTONLY
			* More over, if biped is away from the camera far plane, we will move the biped to the destination immediately.
			*/
			MOVESTYLE_OPC,
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


		/**@def default biped turning speed. */
		static const float SPEED_TURN;
		/**@def default biped walking speed. */
		static const float SPEED_WALK;
		/**@def default biped normal turning speed. */
		static const float SPEED_NORM_TURN;
		
	public:
		CBipedObject(void);
		virtual ~CBipedObject(void);

	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){ return ATTRIBUTE_CLASSID_CBipedObject; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){ static const char name[] = "CBipedObject"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){ static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const std::string& sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		ATTRIBUTE_METHOD1(CBipedObject, GetPhysicsRadius_s, float*)		{ *p1 = cls->GetPhysicsRadius(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetPhysicsRadius_s, float)		{ cls->SetPhysicsRadius(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetPhysicsHeight_s, float*)		{ *p1 = cls->GetPhysicsHeight(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetPhysicsHeight_s, float)		{ cls->SetPhysicsHeight(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetSizeScale_s, float*)		{ *p1 = cls->GetSizeScale(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetSizeScale_s, float)		{ cls->SetSizeScale(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetDensity_s, float*)		{ *p1 = cls->GetDensity(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetDensity_s, float)		{ cls->SetDensity(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetSpeedScale_s, float*)		{ *p1 = cls->GetSpeedScale(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetSpeedScale_s, float)		{ cls->SetSpeedScale(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetCurrentAnimation_s, int*)		{ *p1 = cls->GetCurrentAnimation(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, PlayAnimation_s, int)		{ cls->PlayAnimation(p1); return S_OK; }
		ATTRIBUTE_METHOD(CBipedObject, DumpBVHAnimations_s)		{ cls->DumpBVHAnimations(); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetHeadTurningAngle_s, float*)		{ *p1 = cls->GetHeadTurningAngle(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetHeadTurningAngle_s, float)		{ cls->SetHeadTurningAngle(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetHeadUpdownAngle_s, float*)		{ *p1 = cls->GetHeadUpdownAngle(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetHeadUpdownAngle_s, float)		{ cls->SetHeadUpdownAngle(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetBootHeight_s, float*)		{ *p1 = cls->GetBootHeight(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetBootHeight_s, float)		{ cls->SetBootHeight(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetMovementStyle_s, int*)		{ *p1 = cls->GetMovementStyle(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetMovementStyle_s, int)		{ cls->SetMovementStyle((PhysicsMoveStyle)p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetSpeed_s, float*)		{ *p1 = cls->GetSpeed(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetSpeed_s, float)		{ cls->SetSpeed(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetLastSpeed_s, float*)		{ *p1 = cls->GetLastSpeed(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetLastSpeed_s, float)		{ cls->SetLastSpeed(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetAccelerationDist_s, float*)		{ *p1 = cls->GetAccelerationDist(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetAccelerationDist_s, float)		{ cls->SetAccelerationDist(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetVerticalSpeed_s, float*)		{ *p1 = cls->GetVerticalSpeed(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetVerticalSpeed_s, float)		{ cls->SetVerticalSpeed(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetPhysicsGroupSensorMask_s, DWORD*)		{ *p1 = cls->GetPhysicsGroupMask(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetPhysicsGroupSensorMask_s, DWORD)		{ cls->SetPhysicsGroupMask(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetMaxSpeed_s, float*)		{ *p1 = cls->GetMaxSpeed(); return S_OK; }

		ATTRIBUTE_METHOD(CBipedObject, ForceStop_s)	{ cls->ForceStop(); return S_OK; }

		ATTRIBUTE_METHOD(CBipedObject, ForceMove_s)	{ cls->ForceMove(); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetIsAlwaysAboveTerrain_s, bool*)		{ *p1 = cls->GetIsAlwaysAboveTerrain(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetIsAlwaysAboveTerrain_s, bool)		{ cls->SetIsAlwaysAboveTerrain(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetIsAnimPaused_s, bool*)		{ *p1 = cls->GetIsAnimPaused(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetIsAnimPaused_s, bool)		{ cls->SetIsAnimPaused(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, SetBlendingFactor_s, float)		{ cls->SetBlendingFactor(p1); return S_OK; }


		ATTRIBUTE_METHOD1(CBipedObject, GetLastWayPointType_s, int*)		{ *p1 = cls->GetLastWayPoint().GetPointType(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, GetLastWayPointPos_s, Vector3*)		{ *p1 = cls->GetLastWayPoint().GetPosition(); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetGravity_s, float*)		{ *p1 = cls->GetGravity(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetGravity_s, float)		{ cls->SetGravity(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetIgnoreSlopeCollision_s, bool*)		{ *p1 = cls->GetIgnoreSlopeCollision(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetIgnoreSlopeCollision_s, bool)		{ cls->SetIgnoreSlopeCollision(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetCanFly_s, bool*)		{ *p1 = cls->GetCanFly(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetCanFly_s, bool)		{ cls->SetCanFly(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetAlwaysFlying_s, bool*)		{ *p1 = cls->GetAlwaysFlying(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetAlwaysFlying_s, bool)		{ cls->SetAlwaysFlying(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetIsFlying_s, bool*)		{ *p1 = cls->GetIsFlying(); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, GetFlyUsingCameraDir_s, bool*)		{ *p1 = cls->GetFlyUsingCameraDir(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetFlyUsingCameraDir_s, bool)		{ cls->SetFlyUsingCameraDir(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, IsAutoWalkupBlock_s, bool*)		{ *p1 = cls->IsAutoWalkupBlock(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetAutoWalkupBlock_s, bool)		{ cls->SetAutoWalkupBlock(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CBipedObject, IsControlledExternally_s, bool*)		{ *p1 = cls->IsControlledExternally(); return S_OK; }
		ATTRIBUTE_METHOD1(CBipedObject, SetIsControlledExternally_s, bool)		{ cls->SetIsControlledExternally(p1); return S_OK; }

	protected:
		/** Move the biped in the physical scene.move the biped towards the target using its current speed and facing
		* the biped may slide along a wall or be stopped.
		* @params dTimeDelta: time delta in seconds.
		* @params vPosTarget: destination point in world coordinate system.
		* @params fStopDistance: if the character is within this distance to the target, then it is stopped
		* @params pIsSlidingWall: [out] whether the object is sliding along a wall
		* @return: return true if the biped already reached the destination. */
		bool MoveTowards(double dTimeDelta, const DVector3& vPosTarget, float fStopDistance, bool * pIsSlidingWall = NULL);

		/** move as an OPC. see PhysicsMoveStyle::MOVESTYLE_OPC
		* @param fQuickMoveDistance: if destination is longer than this value, we will how many meters to quick move
		* @return: return true if the biped already reached the destination.
		*/
		bool MoveTowards_OPC(double dTimeDelta, const DVector3& vPosTarget, float fQuickMoveDistance = 15.f);

		/** move as an OPC. see PhysicsMoveStyle::MOVESTYLE_LINEAR
		* @return: return true if the biped already reached the destination.
		*/
		bool MoveTowards_Linear(double dTimeDelta, const DVector3& vPosTarget);

		/** vMovePos is new tentative position that the player is moving towards.
		* @param vMovePos [in/out]:
		* @param vMinPos [in/out]:
		* @param vMaxPos [in/out]:
		* @return: bUseMinMaxBox: if true, the position should be confined in vMinPos and vMaxPos.
		*/
		bool CheckBlockWorld(DVector3& vMovePos, Vector3& vMinPos, Vector3& vMaxPos, bool& bUseGlobalTerrainNorm, bool& bReachPos, Vector3& vBipedFacing, float fDeltaTime);

		bool FacingTowards(double dTimeDelta, float fTargetFacing);
		/** this function is only called by the AnimateBiped() to update states for active bipeds. */
		void UpdateState(float fTimeDelta);

		/** set params from asset when the asset is available (async asset loading) */
		bool SetParamsFromAsset();

		/** set whether character should always above terrain during movement. */
		void SetIsAlwaysAboveTerrain(bool bEnable) { m_bIsAlwaysAboveTerrain = bEnable; }
		/** get whether character should always above terrain during movement. */
		bool GetIsAlwaysAboveTerrain() { return m_bIsAlwaysAboveTerrain; }

		/** true to pause animation. default to false. this is usually to freeze the biped and use Frame Number to control its animation. */
		void SetIsAnimPaused(bool bPaused) { m_bPauseAnimation = bPaused; }
		/** true to pause animation. default to false. this is usually to freeze the biped and use Frame Number to control its animation. */
		bool GetIsAnimPaused() { return m_bPauseAnimation; }

		/** set the blending factor between the current and blend animation.
		* @param fBlendingFactor: from [0,1]. where 0 means no blending.
		*/
		void SetBlendingFactor(float fBlendingFactor);

		bool FlyTowards(double dTimeDelta, const DVector3& vPosTarget, float fStopDistance, bool * pIsSlidingWall);

	public:
		/** how the character reacts to physics in the MoveTowards() functions*/
		void SetMovementStyle(PhysicsMoveStyle nValue){ m_nMovementStyle = nValue; };

		/** how the character reacts to physics in the MoveTowards() functions*/
		PhysicsMoveStyle  GetMovementStyle(){ return m_nMovementStyle; };

		bool CanAnimOpacity() const;

		/** get the world transformation matrix for the current object.
		* @return: return pOut
		*/
		virtual Matrix4* GetRenderMatrix(Matrix4& pOut, int nRenderNumber = 0);

		/** Get the specified attachment matrix of the current model.
		* this is usually for getting the mount point on a certain model, such as horses. It also works for static mesh with xrefed mountable objects.
		* @param pOut: output result
		* @param nAttachmentID: see ATTACHMENT_ID. default to 0, which is general mount point. ATT_ID_MOUNT1-9(20-28) is another mount points
		* @return: NULL if not attachment found, otherwise it is pOut.
		*/
		virtual Matrix4* GetAttachmentMatrix(Matrix4& pOut, int nAttachmentID = 0, int nRenderNumber = 0);

		/** whether it has an attachment point
		* @param nAttachmentID: see ATTACHMENT_ID. default to 0, which is general mount point. ATT_ID_MOUNT1-9(20-28) is another mount points
		*/
		virtual bool HasAttachmentPoint(int nAttachmentID = 0);

		/** this function is called, when the object is in view range. we may need to load the primary asset to update the bounding box, etc.*/
		virtual bool ViewTouch();

		/** this function is only used to backward compatibility of ParaObject:AddEvent() function.  */
		virtual int ProcessObjectEvent(const ObjectEvent& event);
		/**
		* Set a new render technique handle. If the effect file associated with the handle is invalid or do not share the same input declaration as
		* the object at the time of drawing, the object will not be drawn or will be drawn improperly.
		* This function rarely needs to be called by users. Technique handles are usually automatically assigned by the game engine when the asset file is loaded.
		* Of course, users can use special effect files for the rendering of special objects; then this function needs to be called.
		* @param nHandle: @see TechniqueHandle
		*/
		virtual void SetPrimaryTechniqueHandle(int nHandle);

		virtual void SetPosition(const DVector3& v);
		/**
		* Set the base character model.
		*/
		HRESULT InitObject(AssetEntity* pMAE);
		/**
		* delete the animation instance associated with this model.
		*/
		void DeleteAnimInstance();
		virtual void Cleanup();
		/** get the animation instance.*/
		CAnimInstanceBase* GetAnimInstance();
		/** get the ParaX animation instance. This function will return NULL, if GetAnimInstance() is not a ParaX AnimInstance*/
		CParaXAnimInstance* GetParaXAnimInstance();
		/** get the character model instance. This function will return NULL, if there is no character
		* model instance associated with this node*/
		CharModelInstance* GetCharModelInstance();

		/** dump BVH animations of the main character asset*/
		bool DumpBVHAnimations();

		/** get a base object containing the bounding box of the physical biped.
		* this function is used to draw the bounding box of the physics of a biped object in debug mode.
		* @return: the pointer to the bounding box object is returned.
		* NOTE: this BB(bounding box) object pointer is only valid until the next call to this method.
		* internally, it just returns the pointer to a static base object, which is initialized according to the physics
		* of this biped instance.*/
		CBaseObject* GetPhysicsBBObj();

		//-- virtual function from baseObject
		virtual HRESULT Draw(SceneState * sceneState);
		virtual void Animate(double dTimeDelta, int nRenderNumber = 0);
		

		/// used as KEY for batch rendering
		virtual AssetEntity* GetPrimaryAsset();

		virtual void SetNormal(const Vector3 & pNorm);
		virtual Vector3 GetNormal();

		/** set the scale of the object. This function takes effects on both character object and mesh object.
		* @param s: scaling applied to all axis.1.0 means original size. */
		virtual void SetScaling(float s);

		/**get scaling */
		virtual float GetScaling();

		/** set local transform directly */
		virtual void SetLocalTransform(const Matrix4& mXForm);
		/** get local transform*/
		virtual void GetLocalTransform(Matrix4* localTransform);

		virtual void GetOBB(CShapeOBB* obb);

		/** reset the object to its default settings.*/
		virtual void Reset();

		/// environment simulator call these functions
		virtual void PathFinding(double dTimeDelta);

		/// animate biped according to its current waypoint lists and speed.
		/// assuming that no obstacles are in the way it moves.
		virtual void AnimateBiped(double dTimeDelta, bool bSharpTurning = false /*reserved*/);

		/** set asset file name */
		virtual void SetAssetFileName(const std::string& sFilename);

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

		void OneObstaclePathFinding(CBaseObject* pSolid);

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

		/** return true if the object contains alpha blended render pass. This function determines which render pipeline stage the object is rendered.
		* generally speaking, if deferred shading is used, we will render alpha blended objects last.
		*/
		virtual bool HasAlphaBlendedObjects();

		bool GetWayPoint(BipedWayPoint* pOut);
		/** get the last way point.
		* if the way point is empty, a INVALID_WAYPOINT type is returned.*/
		BipedWayPoint& GetLastWayPoint();

		void AddWayPoint(const BipedWayPoint& pt);
		void RemoveWayPoint();
		/** remove all way points that matches a given type*/
		void RemoveWayPointByType(BipedWayPoint::MyType nType = BipedWayPoint::COMMAND_POINT);


		virtual void SetUserControl();
		virtual void ChooseNewLocation(Vector3 *pV);

		/* Low level events(LLE) replacement to low level command*/

		/** replace the current AI module with a new one.*/
		void ReplaceAIModule(CAIBase* pNew);
		/** Get the AI module that is dynamically associated with this object */
		virtual CAIBase*		 GetAIModule();
		/** use a specified AI object.
		* @param sAIType: valid AI object is:
		*  "NPC"|""|"NULL"
		*  "" is the same as "NPC"
		*  "NULL" means no AI module.
		* @return: the current ai object is returned.
		*/
		CAIBase* UseAIModule(const string& sAIType);

		/// facing the target immediately.
		void FacingTargetImmediately();
		/// facing the target right away provided that the biped is not moving
		void FacingTarget(const Vector3 *pV);
		/// facing the target right away provided that the biped is not moving
		void FacingTarget(float fTargetFacing);


		/** get object roll: is facing the positive x axis, if yaw is 0.
		*/
		virtual float GetRoll();
		virtual void SetRoll(float fValue);

		/** get object pitch: is facing the positive x axis, if yaw is 0.
		*/
		virtual float GetPitch();
		virtual void SetPitch(float fValue);

		/** set model facing to the specified value. */
		virtual void SetYaw(float fFacing);

		/** update the local transform from size, roll, pitch, yaw, etc. */
		virtual void UpdateGeometry();
		
		/** set the facing of model's head. It is relative to the model facing.
		* Please note that if the head facing is not in the range [-Pi/2,Pi/2],
		* the head facing will be set to its edge value. */
		void SetHeadTurningAngle(float fFacing);
		/** get the head facing.It is relative to the model facing. */
		float GetHeadTurningAngle();

		/** set the facing of model's head. It is relative to the model facing.
		* Please note that if the head facing is not in the range [-Pi/2,Pi/2],
		* the head facing will be set to its edge value. */
		void SetHeadUpdownAngle(float fFacing);
		/** get the head facing.It is relative to the model facing. */
		float GetHeadUpdownAngle();

		/** this is the same as GetFacing()+GetHeadTurningAngle();*/
		float GetHeadFacing();

		virtual void SetOpacity(float fOpacity);

		/** get the direction vector according to speed angle of current biped.
		* if the speed is negative, the direction will be turned 180 degrees.
		* in other words, this direction is always the direction where the biped should progress.
		* it is used for physics calculation.
		*/
		void GetSpeedDirection(Vector3 *pV);
		/** set the current speed angle.
		* @param fSpeedAngle: the new speed direction of the biped. It will be immediately used for physics.
		* @param bSmoothRender: whether the biped will smoothly turn to that direction during rendering.
		*	the default value is true.*/
		void SetSpeedAngle(float fSpeedAngle);
		/** get the speed direction for physics. */
		float GetSpeedAngle();
		/** get the magnitude of the current speed of the character. Please note that this value may be negative. */
		float GetSpeed();

		/** this is not used, unless, speed acceleration is positive. */
		float GetLastSpeed();
		void SetLastSpeed(float fSpeed);

		/** when m_fSpeed is set, this acceleration is used on m_fLastSpeed, before it reach m_fSpeed.
		* default to 0.f
		*/
		float GetAccelerationDist();
		void SetAccelerationDist(float fAccelerationDist = 0.f);


		/** get the magnitude of the current speed of the character. this is always a non-negative value */
		float GetAbsoluteSpeed();
		/** set the magnitude of the current speed of the character. */
		void SetSpeed(float fSpeed);
		/** get the maximum speed of the character. Currently it is just the walk speed of the character. */
		float GetMaxSpeed();
		/** set the animation instance's speed to dScale times of its original speed */
		virtual void SetSpeedScale(float dScale);
		/** @see SetSpeedScale*/
		virtual float GetSpeedScale();
		/** set the size of the object to dScale times of its original size. usually this value is 1.0 */
		virtual void SetSizeScale(float dScale);
		/** @see SetSizeScale*/
		virtual float GetSizeScale();

		/** set boot height, default to 0.f meters. It only affects rendering. In case, the character is wearing a high heel boot, we may set this to 0.1-0.2 meters. */
		void SetBootHeight(float fBootHeight);
		/** get boot height, default to 0.f meters. It only affects rendering. In case, the character is wearing a high heel boot, we may set this to 0.1-0.2 meters. */
		float GetBootHeight();

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

		void SetGravity(float gravity);
		float GetGravity();

		void SetIgnoreSlopeCollision(bool ignoreSlope);
		bool GetIgnoreSlopeCollision();

		void SetCanFly(bool canFly);
		bool GetCanFly();

		bool GetAlwaysFlying() const;
		void SetAlwaysFlying(bool val);

		bool GetFlyUsingCameraDir() const;
		void SetFlyUsingCameraDir(bool val);

		bool GetIsFlying();

		void SetFlyingDirection(const Vector3 *dir);

		/**whether automatically walk up block that is 1 block high in front of us. */
		void SetAutoWalkupBlock(bool bAutoWalkup);
		/**whether automatically walk up block that is 1 block high in front of us. */
		bool IsAutoWalkupBlock();

		/** we will not simulate or changing the character's animation, etc, since the biped is controlled externally, such as a movie actor in the script*/
		bool IsControlledExternally() const;
		void SetIsControlledExternally(bool val);

		/** if the biped is in air, it will fall down. In case a biped is put to stop and the terrain below it changes.
		* one should manually call this function to let the biped fall down. Internally it just set the vertical speed to
		* a small value*/
		void FallDown();

		virtual bool IsStanding();
		bool IsStandingIntentioned();
		/** add the standing state action to its action manager */
		void SetStandingState();
		bool IsBipedBlocked();

		/** Walk using specified animation
		* @param sName: The name of the walking animation. if sName == NULL, the default walking animation will be loaded
		*/
		void SetWalkingState(const char* sName = NULL);

		/**
		* get biped state manager
		* @param bCreateOnDemand if true, we will attempt to create the manager if it is not created before.
		* @return it may return NULL if bCreateOnDemand is false.
		*/
		virtual CBipedStateManager*  GetBipedStateManager(bool bCreateOnDemand = true);

		/** set the callback scipt whenever the primary asset is successfully loaded. */
		virtual void SetOnAssetLoaded(const char* sCallbackScript);

		/** Plat a specified animation by its animation index.
		* @param nIndex: the animation index.
		* @param bUpdateSpeed: whether to update the biped's speed according to the one defined in the animation.
		*	the default value is true.
		* @param bAppend: if true, the new animation will be appended to the current animation according to the following rules:
		*		- if the current animation is non-looping, the new animation will be played after the end of the current animation.
		*		- if the current animation is looping, the new animation will be played immediately.
		*/
		void PlayAnimation(DWORD nIndex, bool bUpdateSpeed = true, bool bAppend = false);

		/** Plat a specified animation by its animation name.
		* @param sName: the animation name.Please see the ParaXAnimationInstance for some predefined animation names.
		* @param bUpdateSpeed: whether to update the biped's speed according to the one defined in the animation.
		*	the default value is true.
		* @param bAppend: if true, the new animation will be appended to the current animation according to the following rules:
		*		- if the current animation is non-looping, the new animation will be played after the end of the current animation.
		*		- if the current animation is looping, the new animation will be played immediately.
		*/
		void PlayAnimation(const char* sName, bool bUpdateSpeed = true, bool bAppend = false);
		/** get the ID of the current animation. */
		int GetCurrentAnimation();


		/** Set the current animation id
		* @param nAnimID: 0 is default standing animation. 4 is walking, 5 is running. more information, please see AnimationID */
		virtual void SetAnimation(int nAnimID);

		/** get the scaling. */
		virtual int GetAnimation();

		/** whether an animation id exist. this function may have different return value when asset is async loaded.
		* @param nAnimID: predefined id.
		*/
		virtual bool HasAnimation(int nAnimID);

		/** set the current animation frame number relative to the beginning of current animation.
		* @param nFrame: 0 means beginning. if nFrame is longer than the current animation length, it will wrap (modulate the length).
		*/
		virtual void SetAnimFrame(int nFrame);

		/** get the current animation frame number relative to the beginning of current animation.  */
		virtual int GetAnimFrame();

		/** whether animation is enabled. by default this is true. During movie editing, we may disable animation, set animation frame explicitly by editor logics. */
		virtual void EnableAnim(bool bAnimated);
		virtual bool IsAnimEnabled();

		/** get the current local time in case it is animated in milli seconds frames. */
		virtual int GetTime();
		virtual void SetTime(int nTime);

		/** whether to use global time to sync animation. Default to false.
		if true, all characters plays exactly the same frames if they are using the same animation file at all times,
		if false, each character advances its time frame separately according to their visibility in the scene.
		@note: animation is always looped once use global time
		*/
		virtual void SetUseGlobalTime(bool bUseGlobalTime);

		/** whether to use global time to sync animation. Default to false.
		if true, all characters plays exactly the same frames if they are using the same animation file at all times,
		if false, each character advances its time frame separately according to their visibility in the scene.
		@note: animation is always looped once use global time
		*/
		virtual bool IsUseGlobalTime();

		/** mount the current character on the target
		* @param pTarget: if NULL, it will unmount.
		* @param nMountID: the attachment ID of the model. if -1 (default), we will attach to the nearest mount position.
		*/
		bool MountOn(CBaseObject* pTarget, int nMountID = -1);

		/** update the biped's speed.
		* @param fSpeed: this is usually the input from the animation speed.
		*/
		void UpdateSpeed(float fSpeed);

		/** get the vertical speed of the character. 0.f is returned if it does not have a vertical speed.*/
		float GetVerticalSpeed();
		/** Set the vertical speed of the character. 0.f is returned if it does not have a vertical speed.*/
		void SetVerticalSpeed(float fSpeed);

		/** use the speed defined in a specified animation by its animation index.
		* @param nIndex: the animation index.
		*/
		void UseSpeedFromAnimation(int nIndex);
		/** use the speed defined in a specified animation by its animation name.
		* @param sName: the animation name.Please see the ParaXAnimationInstance for some predefined animation names.
		*/
		void UseSpeedFromAnimation(const char* sName);

		/** check to see if the underlying model has a given animation id. */
		bool HasAnimId(int nAnimID);

		void PlayDeathDecayAnim();

		void SetNextTargetPosition(const DVector3 &pV, bool bIgnoreHeight = true);
		/**
		* force the biped to walk to a position. The action is carried out immediately.
		* the biped might still be blocked after calling this function. In some cases,
		* one should check if a biped is already blocked before calling it to walk again.
		*/
		void WalkTo(const DVector3& pV, bool bIgnoreHeight = true);
		/**
		* Command the biped to move using its current facing (orientation).
		* the biped will continue walking until it is stopped or overridden by some other tasks.
		*/
		void ForceMove();
		/**
		* stop the biped by removing all of its targeting way points.
		*/
		void ForceStop();
		/**
		* jump up. it gives a character a vertical impulse. i.e. a vertical speed.
		*
		* @param fVerticalSpeed: the upward speed of the character after the jump
		* @param bJumpFromGround: if this is true, the impulse will only be given if the character is currently on
		*       the ground. i.e. its current vertical speed is 0.
		*/
		void JumpUpward(float fVerticalSpeed, bool bJumpFromGround);

		/** the biped is modeled as a cylinder or sphere during rough physics calculation.
		* this function returns the radius of the cylinder or sphere.*/
		virtual float		GetPhysicsRadius();
		/** the biped is modeled as a cylinder or sphere during rough physics calculation.
		* this function set the radius of the cylinder or sphere.*/
		void		SetPhysicsRadius(float fR);
		/** the biped is modeled as a cylinder or sphere during rough physics calculation.
		* this function returns the height of the cylinder or sphere.*/
		float		GetPhysicsHeight();
		/** the biped is modeled as a cylinder or sphere during rough physics calculation.
		* this function sets the height of the cylinder or sphere.*/
		void		SetPhysicsHeight(float fH);
		/** get asset height. */
		float GetAssetHeight();

		void ResetBipedPosition(const Vector3 *pV, bool bIgnoreHeight = true);

		virtual void Rotate(float x, float y, float z);

		/** set groups Mask used to filter physics objects, default to 0xffffffff.
		groups Mask used to specify which groups of physics object this biped will collide with.
		In most cases, only mesh object's physics group needs to be configured.
		*/
		virtual void SetPhysicsGroupMask(DWORD dwValue);

		/** get groups Mask used to filter physics objects, default to 0xffffffff.
		groups Mask used to specify which groups of physics object this biped will collide with.
		In most cases, only mesh object's physics group needs to be configured.
		*/
		virtual DWORD GetPhysicsGroupMask();

	public:
		/* advanced character control functions.*/

		/**
		* Reset base model. Base model is the frame model, to which other sub-models like
		* weapons and clothing  are attached.
		* NOTE: the current version will delete the entire model instance, so that any sub-models
		* attached to this model will be deleted and that the default appearance of the base model will
		* show up, if one does not update its equipment after this call.
		* @param assetCharBaseModel: It is the new base model asset;it should be a valid ParaX model asset.
		*/
		void ResetBaseModel(AssetEntity* assetCharBaseModel);
		/**
		* Load a stored model in data base by the model set ID. A stored model usually contain
		* the attachments and equipments, but not the base model. This function is only valid when
		* the base model has already been set.
		* @param nModelSetID: the ID of the model set in the database. Some ID may be reserved for
		* user-specified model
		*/
		void LoadStoredModel(int nModelSetID);

	public:
		virtual bool IsShadowEnabled(){ return m_bIsShadowEnabled; };
		virtual void BuildShadowVolume(SceneState * sceneState, ShadowVolume * pShadowVolume, LightParams* pLight);
		
		/** this may return NULL, if the object does not implement the IGameObject interface. */
		virtual IGameObject* QueryIGameObject();

		/** when other game objects of a different type entered the sentient area of this object.
		This function will be automatically called by the environment simulator. */
		virtual int On_EnterSentientArea();
		/** when no other game objects of different type is in the sentient area of this object.
		This function will be automatically called by the environment simulator. */
		virtual int On_LeaveSentientArea();

		/** when the player clicked on this object.
		This function will be automatically called by the environment simulator. */
		virtual int On_Click(DWORD nMouseKey, DWORD dwParam1, DWORD dwParam2);

		/** TODO: Some game defined events, such as user attack, etc. */
		virtual int On_Event(DWORD nEventType, DWORD dwParam1, DWORD dwParam2);

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

		/** if the object may contain physics*/
		virtual bool CanHasPhysics();
		virtual void LoadPhysics();
		virtual void UnloadPhysics();
		virtual void SetPhysicsGroup(int nGroup);
		virtual int GetPhysicsGroup();
		virtual void EnablePhysics(bool bEnable);
		virtual bool IsPhysicsEnabled();
		/** get the number of physics actors. If physics is not loaded, the returned value is 0. */
		int GetStaticActorCount();

	protected:
		void AnimateIdle(double dTimeDelta);
		void AnimateMoving(double dTimeDelta, bool bSharpTurning = false);

		void AnimateUserControl(double dTimeDelta);

		/** get parax asset entity if its type is ParaX model. */
		ParaXEntity* GetParaXEntity();
	private:
		// -- data structuring
		asset_ptr<AssetEntity>			m_pMultiAnimationEntity;
		/// animation instance
		ref_ptr<CAnimInstanceBase>		m_pAI;


		/** how biped move according to the physics of its environment. */
		PhysicsMoveStyle m_nMovementStyle;

		/// physical radius. this value will restrict the biped's movement and is only used for physics.
		float		m_fPhysicsRadius;

		/* physical height. this value will restrict the biped's vertical movement.
		if there is no head attachment, the GetPhysicsHeight is used to for character head on position instead.
		If m_fPhysicsHeight is never set, it is always 4*m_fPhysicsRadius. However, if it is set, its value are maintained.
		*/
		float		m_fPhysicsHeight;

		// the biped state manager
		CBipedStateManager*  m_pBipedStateManager;

		/// the AI module that is dynamically associated with this object
		CAIBase*		 m_pAIModule;

		/** character's movement speed -- in units/second */
		float		m_fSpeed;
		/** this is not used, unless, speed acceleration is positive. */
		float		m_fLastSpeed;
		/** when m_fSpeed is set, this acceleration is used on m_fLastSpeed, before it reach m_fSpeed.
		* default to 0.f
		*/
		float		m_fAccelerationDist;
		/** the direction of the speed in rads. 0 is the positive x. It is used for physics.*/
		float		m_fSpeedAngle;
		/** the current vertical speed of the character
		* a character will only have a vertical speed, when it is in the air,
		* such as falling from a high ground or jumping */
		float		m_fSpeedVertical;

		/// the norm of the terrain surface on which the character stands
		/// if vNorm.y is set to 0, then the norm is recalculated according to the terrain surface
		Vector3          m_vNorm;
		/// even when biped is stopped, we will animate its norm if m_vNormTarget is different from m_vNorm
		Vector3          m_vNormTarget;

		/** roll value */
		float m_fRoll;
		/** pitch value */
		float m_fPitch;

		/// This indicates where we are moving to, can be a list of way points
		WayPointList_type m_vPosTarget;

		/// current direction the character is facing -- in our sample, it's 2D only(Y axial)
		// float                m_fFacing;  // it is already defined in base object class
		/// The direction from the current position to the final destination
		float                m_fFacingTarget;

		/** boot height, default to 0.f meters. It only affects rendering. In case, the character is wearing a high heel boot, we may set this to 0.1-0.2 meters. */
		float m_fBootHeight;

		/** a value between [0,1). last block light. */
		float m_fLastBlockLight;
		/** a hash to detect if the containing block position of this biped changed. */
		DWORD m_dwLastBlockHash;

		/** body density. The water density is always 1.0 in the game engine.
		So if it is above 1.0, it will sink in water; otherwise it will float on water surface.
		So if it is below 0.5, it will fly or glind in air falling down with little gravity; otherwise it will fall down with full gravity.
		A density of 0 or negative, means that the character can fly. The default value is 1.2. the following are some examples
		- character:	body density: 1.2
		- car:			body density: 4.0	mount target
		- ship:			body density: 0.8	mount target
		- plane:		body density: 0.4	mount target
		*/
		float m_fDensity;

		//default to 9.18
		float m_gravity;

		/** size scale */
		float	m_fSizeScale;

		/** speed scale */
		float 	m_fSpeedScale;
		/** size scale multiple model height */
		float m_fAssetHeight;
		Vector3 m_flyingDir;

		/** groups Mask used to specify which groups of physics object this biped will collide with.
		Default to 0x3, which only collide with group 1 and 2 */
		DWORD m_dwPhysicsGroupSensorMask;

		/** groups Mask used to filter physics objects, default to 0xffffffff*/
		DWORD m_dwPhysicsGroupMask;

		/** all static physics actors in physics engine */
		vector<IParaPhysicsActor*> m_staticActors;
		// any bit wise combination of PHYSICS_METHOD
		DWORD m_dwPhysicsMethod;
		uint32 m_nPhysicsGroup;

		/// true:m_fFacingTarget movement is ignored.
		bool m_bIgnoreFacingTarget : 1;
		bool m_bIsShadowEnabled : 1;
		bool m_bIsAlwaysAboveTerrain : 1;
		/** true to pause animation. default to false. this is usually to freeze the biped and use Frame Number to control its animation. */
		bool m_bPauseAnimation : 1;

		bool m_ignoreSlopeCollision : 1;
		bool m_isFlying : 1;
		bool m_canFly : 1;
		bool m_isAlwaysFlying : 1;
		bool m_isFlyUsingCameraDir : 1;
		bool m_readyToLanding : 1;
		bool m_bAutoWalkupBlock : 1;
		/** we will not simulate or changing the character's animation, etc, since the biped is controlled externally, such as a movie actor in the script*/
		bool m_bIsControlledExternally : 1;
		bool m_bCanAnimOpacity : 1;
		
	};

}