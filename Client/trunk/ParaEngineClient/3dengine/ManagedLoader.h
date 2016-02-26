#pragma once
#include "BaseObject.h"

namespace ParaEngine
{
class CSceneObject;
/**
* a managed loader is a kind of global scene object for dynamic scene object loading and unloading. 
* the content of a managed loader can no longer be changed once the loader has been attached to the scene.
* once objects are attached to the scene graph, the ownership of these objects transfered to the scene manager,
* otherwise the object ownership is the loader object. The owner of an object is responsible to clean up
* the object when it is no longer needed.
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
*	- it is good practice to put all objects which are physically close to each other in a single managed loader. 
*
*/
class CManagedLoader : 	public CBaseObject
{
public:
	CManagedLoader(void);
	virtual ~CManagedLoader(void);
	virtual CBaseObject::_SceneObjectType GetType(){return CBaseObject::ManagedLoader;};

	ATTRIBUTE_DEFINE_CLASS(CManagedLoader);
	
private:
	/// whether all managed objects have been attached to the scene.
	bool m_bHasAttached;
public:
	/**
	* if child objects are not attached to the scene, they will be destroyed together with the loader.
	* Otherwise, this function does nothing. The scene graph is responsible to clean up all scene objects.
	*/
	virtual void Cleanup();
	/**
	* add a new scene object to this managed loader. No two objects in the managed loader can be of the same position and
	* sharing the same primary asset and of the same object type.
	*/
	virtual void AddChild(CBaseObject * pObject);

	/** Attach all scene objects managed by this loader to the current scene manager. 
	* This function can be called as many times as one likes. The code however will ensure that objects managed 
	* by the loader will only be created and attached once in the game engine.There may be a setting in ParaEngine to 
	* do automatic garbage collection with managed loaders, so one may need to call the function often enough for the 
	* managed objects to stay active during the game play. 
	*/
	void AttachObjectsToScene();

	/** Detach all scene objects managed by this loader from the current scene manager. 
	* Please be very careful with this function. Because when the function is called, the ownership
	* of all objects are in the scene graph. If the scene graph has moved or deleted any of the loader's 
	* child objects, the loader will not be informed and will function unexpectedly. So one would
	* only call this function, if he or she was sure that all child objects are static or global in the scene graph
	* and have not been changed by the scene graph from the time they were attached to the scene to this time. 
	*/
	void DetachObjectsFromScene();

	/** 
	* Pre-load all resource and physics objects of all managed scene objects.
	* all scene object resources will be automatically loaded by the game engine. So this function is
	* not really needed to be called, unless it is what you wanted. 
	*/
	void PreLoad();

	/** 
	* Unload all resources and physics objects of all managed scene objects.
	* the scene has a automatic garbage collector to do the job on a per-object level. If one is sure all objects
	* managed by this loader will not be used for a long time, one can call this function to explicitly unload them.
	* This function is only valid after AttachObjectsToScene() has been called.
	*/
	virtual void CompressObject(CBaseObject::CompressOption option = (CBaseObject::CompressOption)0xff);
};
}