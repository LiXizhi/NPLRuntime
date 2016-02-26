#pragma once

namespace ParaEngine
{
	/**
	* loading collada model to internal format. 
	* Collada model will be converted to MeshEntity or ParaXEntity. The latter are the original format used by ParaEngine
	*/
	class ColladaModelLoader
	{
	public:
		ColladaModelLoader();
		~ColladaModelLoader();

	public:
		/** load the collada file as a single ParaXEntity. the scene is exported to x file as filename.x and all textures are converted to dds. 
		* @return: the ParaXEntity is created. 
		*/
		ParaXEntity* LoadSingleEntity(const char* filename);

		/** load the collada scene instances to the current scene. each mesh instance in the file is treated as a single 
		* object, and the mesh library is exported to parax file as filename#meshname.x  and all textures are converted to dds. 
		* @return: the number of instances created is returned. 
		*/
		int LoadMultipleInstances(const char* filename);

	private:
		// some intermediary data structures. 
		/** whether we will create parax or x file when loading th collada file. */
		bool m_bCreateXFile;

	};
}
