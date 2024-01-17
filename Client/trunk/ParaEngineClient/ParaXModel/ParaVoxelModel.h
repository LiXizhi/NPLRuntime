#pragma once
#include "IAttributeFields.h"

namespace ParaEngine
{
	struct SceneState;

	/** a octree based sparse voxel model, with 8 bits color per node. 
	* The AABB of the model is always 1*1*1, one needs to transform the model when used. 
	* It uses extremely small memory, and can be dynamically changed. 
	*/
	class ParaVoxelModel : public IAttributeFields
	{
	public:
		ParaVoxelModel();
		virtual ~ParaVoxelModel();

		ATTRIBUTE_DEFINE_CLASS(ParaVoxelModel);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);
			
	public:
		/** load the model from a binary buffer. */
		bool Load(const char* pBuffer, int nCount = -1);

		/** save the model to a binary buffer. */
		bool Save(std::vector<char>& output);

		/** set the block at the given position. 
		* @param x, y, z : the position of the block relative to level.
		* @param level: the level or model width. this should be a power of 2, like 1, 2, 4, 8, 16, ..., 1024, ...
		* @param color: the 8 bits color of the block. -1 if the block is empty.
		*/
		void SetBlock(int x, int y, int z, int level, int color);

		/** get the block color at the given position and level
		* @return -1 if the block is empty. or return 8 bits color of the block.
		*/
		int GetBlock(int x, int y, int z, int level);

		/** ray picking at given level
		* @param level: the model level at which to pick. this should be a power of 2, like 1, 2, 4, 8, 16, ..., 1024, ...
		*/
		bool RayPicking(const Vector3& origin, const Vector3& dir, Vector3& hitPos, int& hitColor, int level = -1);

		/** optimize the model for faster rendering. */
		void Optimize();

		/** draw blocks with dynamic level of detail with reference to the camera position in scene state.
		*/
		void Draw(SceneState* pSceneState);
	};
}
