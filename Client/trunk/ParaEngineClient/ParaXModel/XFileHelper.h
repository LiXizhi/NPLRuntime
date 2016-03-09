#pragma once
#include "modelheaders.h"
#include "ParaXRefObject.h"

#define PARAX_MAX_NUMBER_OF_TEXTURECOORDS 0x2
/** @def AI_MAX_NUMBER_OF_COLOR_SETS
*  Supported number of vertex color sets per mesh. */
#define PARAX_MAX_NUMBER_OF_COLOR_SETS 0x2

namespace ParaEngine
{
	// ---------------------------------------------------------------------------
	/** FOR IMPORTER PLUGINS ONLY: Simple exception class to be thrown if an
	*  unrecoverable error occurs while importing. Loading APIs return
	*  NULL instead of a valid aiScene then.  */
	class DeadlyImportError
		: public runtime_error
	{
	public:
		/** Constructor with arguments */
		explicit DeadlyImportError(const std::string& pErrorText)
			: runtime_error(pErrorText)
		{
		}
	private:
	};

	namespace XFile
	{

		/** Helper structure representing a XFile mesh face */
		struct Face
		{
			uint16 mIndices[3];
		};

		/** Helper structure representing a texture filename inside a material and its potential source */
		struct TexEntry
		{
			std::string mName;
			bool mIsNormalMap; // true if the texname was specified in a NormalmapFilename tag

			TexEntry() { mIsNormalMap = false; }
			TexEntry(const std::string& pName, bool pIsNormalMap = false)
				: mName(pName), mIsNormalMap(pIsNormalMap)
			{ /* done */
			}
		};

		/** Helper structure representing a XFile material */
		struct Material
		{
			std::string mName;
			bool mIsReference; // if true, mName holds a name by which the actual material can be found in the material list
			LinearColor mDiffuse;
			float mSpecularExponent;
			Vector3 mSpecular;
			Vector3 mEmissive;
			std::vector<TexEntry> mTextures;

			int32 sceneIndex; ///< the index under which it was stored in the scene's material list

			Material() { mIsReference = false; sceneIndex = -1; }
		};

		/** Helper structure to represent a bone weight */
		struct BoneWeight
		{
			unsigned int mVertex;
			float mWeight;
		};

		/** Helper structure to represent a bone in a mesh */
		struct Bone
		{
			std::string mName;
			std::vector<BoneWeight> mWeights;
			Matrix4 mOffsetMatrix;
		};

		/** Helper structure to represent an XFile mesh */
		struct Mesh
		{
			std::vector<Vector3> mPositions;
			std::vector<Face> mPosFaces;
			std::vector<Vector3> mNormals;
			std::vector<Face> mNormFaces;
			unsigned int mNumTextures;
			std::vector<Vector2> mTexCoords[PARAX_MAX_NUMBER_OF_TEXTURECOORDS];
			unsigned int mNumColorSets;
			std::vector<LinearColor> mColors[PARAX_MAX_NUMBER_OF_COLOR_SETS];

			std::vector<unsigned int> mFaceMaterials;
			std::vector<Material> mMaterials;

			std::vector<Bone> mBones;

			Mesh() { mNumTextures = 0; mNumColorSets = 0; }
		};

		/** Helper structure to represent a XFile frame */
		struct Node
		{
			std::string mName;
			Matrix4 mTrafoMatrix;
			Node* mParent;
			std::vector<Node*> mChildren;
			std::vector<Mesh*> mMeshes;

			Node() { mParent = NULL; }
			Node(Node* pParent) { mParent = pParent; }
			~Node()
			{
				for (unsigned int a = 0; a < mChildren.size(); a++)
					delete mChildren[a];
				for (unsigned int a = 0; a < mMeshes.size(); a++)
					delete mMeshes[a];
			}
		};


		/** Helper structure analogue to aiScene */
		struct Scene
		{
			Node* mRootNode;

			ParaXHeaderDef m_header;

			std::vector<Mesh*> mGlobalMeshes; // global meshes found outside of any frames
			std::vector<Material> mGlobalMaterials; // global materials found outside of any meshes.
			std::vector<ParaXRefObject> m_XRefObjects;
			Scene() { mRootNode = NULL; }
			~Scene()
			{
				delete mRootNode;
				for (unsigned int a = 0; a < mGlobalMeshes.size(); a++)
					delete mGlobalMeshes[a];
			}
		};

	} // end of namespace XFile
}


