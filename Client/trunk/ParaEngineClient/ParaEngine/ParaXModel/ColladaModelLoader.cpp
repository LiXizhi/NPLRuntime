//-----------------------------------------------------------------------------
// Class:	ColladaModelLoader
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2008.10.6
// Desc: 
/*
>COLLADA is an open Digital Asset Exchange Schema for
*      the interactive 3D industry. There are exporters and
*      importers for this format available for most of the
*      big 3d packagesat http://collada.org. Irrlicht can
*      import COLLADA files by using the
*      ISceneManager::getMesh() method. COLLADA files need
*      not contain only one single mesh but multiple meshes
*      and a whole scene setup with lights, cameras and mesh
*      instances, this loader can set up a scene as
*      described by the COLLADA file instead of loading and
*      returning one single mesh. By default, this loader
*      behaves like the other loaders and does not create
*      instances, but it can be switched into this mode by
*      using
*      SceneManager->getParameters()->setParameter(COLLADA_CREATE_SCENE_INSTANCES, true);
*      Created scene nodes will be named as the names of the
*      nodes in the COLLADA file. The returned mesh is just
*      a dummy object in this mode. Meshes included in the
*      scene will be added into the scene manager with the
*      following naming scheme:
*      path/to/file/file.dae#meshname. The loading of such
*      meshes is logged. Currently, this loader is able to
*      create meshes (made of only polygons), lights, and
*      cameras. Materials and animations are currently not
*      supported but this will change with future releases.


//! Name of the parameter specifying the COLLADA mesh loading mode
Specifies if the COLLADA loader should create instances of the models, lights and
cameras when loading COLLADA meshes. By default, this is set to false. If this is
set to true, the ISceneManager::getMesh() method will only return a pointer to a
dummy mesh and create instances of all meshes and lights and cameras in the collada
file by itself. Example:
\code
SceneManager->getParameters()->setAttribute(scene::COLLADA_CREATE_SCENE_INSTANCES, true);
\endcode

const c8* const COLLADA_CREATE_SCENE_INSTANCES = "COLLADA_CreateSceneInstances";
*/
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
// #include "FCollada/FCollada.h"
#include "ColladaModelLoader.h"

using namespace ParaEngine;

ColladaModelLoader::ColladaModelLoader()
:m_bCreateXFile(true)
{;
}

ColladaModelLoader::~ColladaModelLoader()
{
}

ParaXEntity* ParaEngine::ColladaModelLoader::LoadSingleEntity( const char* filename )
{
	return 0;
}

int ParaEngine::ColladaModelLoader::LoadMultipleInstances( const char* filename )
{
	return 0;
}
