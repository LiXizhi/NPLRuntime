#pragma once

#include <vector>
#include <list>
#include "ParaXModel/ParaXModel.h"

namespace ParaEngine
{
	struct TextureEntity;

	using namespace std;
	

	/** face group is similar to render pass in ParaXModel, or sub mesh in StaticMesh. 
	* it contains static data. both ParaXModel and StaticMesh stores face group (for sorting) in this format. 
	* Each face group contains one or several triangle strips, one center of mass(the average position of all vertex points in object space), 
	* one texture, a set of rendering parameters(such as z-write, z-test, blending, etc)
	*/
	class CFaceGroup
	{
	public:
		CFaceGroup();

		//////////////////////////////////////////////////////////////////////////
		// render parameters
		bool m_alphaBlending;
		bool m_alphaTesting;
		bool m_disableZWrite;
		bool m_bHasLighting;
		bool m_bAdditive;
		bool m_bSkinningAni;

		TextureEntity* m_pTexture;
		// materials.
		ParaMaterial m_material;
		// the larger, the later to to be rendered. In most cases, this is always 0.
		int m_order;

		// center of mass position in object space. This value multiplied by the instance transform matrix is used to determine the camera to face distance. 
		Vector3 m_vCenterPos;

		//////////////////////////////////////////////////////////////////////////
		// triangle list: vertices, normals, texture coordinate set 1. 
		int m_nNumTriangles;
		vector <Vector3> m_vertices;
		vector <Vector3> m_normals;
		vector <Vector2> m_UVs;
		vector <uint32> m_vertexWeights;
		vector <uint32> m_boneIndices;

		float m_stripLength;
	public:
		/** update the center position based on the vertices in the groups. */
		void UpdateCenterPos();
	};

	/**
	* an instance of the face group: 
	* it contains the face group static data, and per-instance data, such as group to camera distance, group transform. 
	* please note that only a single transform matrix is supported. For skinned ParaXModel, it will be the first bone.
	*/
	class CFaceGroupInstance
	{
	public:
		CFaceGroupInstance();
		CFaceGroupInstance(const Matrix4* pMat, CFaceGroup* faceGroup);

		/** distance square to camera. this value is calculated on the fly*/
		float m_fToCameraDistSq;

		/** transform of the face group instance. */
		Matrix4 m_transform;

		/** in case there is an UV animation applied to this face group. such as transparent texture based water pools, etc. 
		* in most cases, this is (0,0).*/
		Vector2 m_vUVOffset;

		/** pointer to the data of the face group. */
		CFaceGroup* m_facegroup;

		/** the alpha value in [0,1]*/
		float m_fAlpha;

		bool m_UVRgbAnim;

		Bone *m_bones;
	};

	/**
	* This class provides sorting and rendering of translucent face groups. Sorting is performed by face group, instead of individual face.
	* Each face group contains one or several triangle strips, one texture, a set of rendering parameters(such as z-write, z-test, blending, etc)
	* By default, the SceneState will disable rendering sorted face group, unless it is explicitly enabled. This prevents face group to be rendered during 
	* shadow pass or in canvas or mini scene graph. 
	* 
	* Both StaticMesh and ParaXModel may contain translucent face group that need to be sorted very accurately before rendering. 
	* however, it is advised that we use translucent faces rarely, as it degrades rendering performance. 
	*/
	class CSortedFaceGroups
	{
	public:
		typedef std::vector<CFaceGroupInstance> FaceGroups_Type;
		CSortedFaceGroups(void);
		~CSortedFaceGroups(void);

	public:
		/** add a new face group */
		void AddFaceGroup(const CFaceGroupInstance& facegroup);

		/** sorting from back to front relative to a given camera position in world space. */
		void Sort(const Vector3& vCameraPos);

		/** whether faces is sorted from back to front. */
		bool IsSorted(){return m_sorted;}

		/** render the face group from back to front. Be sure to call Sort() before calling this function. */
		void Render();

		/** clear all face group. this function should be called at the beginning of each frame. */
		void Clear();

		/** check if empty. */
		bool IsEmpty() {return m_sortedFaceGroups.empty();};
	private:
		/** sorted face groups*/
		FaceGroups_Type m_sortedFaceGroups;
		
		/** whether sorted. when a new face group is added, this parameter is automatically set to false. */
		bool m_sorted;
	};

}

