#pragma once
#include <vector>
#include <string>
#include "ParameterBlock.h"
namespace ParaEngine
{
	using namespace std;
	
	/**
	* Para mesh XML file is a reference file for properly loading static mesh or parax mesh file. 
	* It may contain LOD information, shader information (shader index is shader handle, params are all per asset effect parameters.), bounding box info, etc.
	sample ParaEngine mesh xml file: 
	<mesh version=1 type=0>
		<boundingbox minx=0 miny=0 minz=0 maxx=1 maxy=1 maxz=1/>
		<shader index=3>
			<param name="0" type="texture">Texture/whitedot.dds</param>
			<param name="a" type="int">10</param>
			<param name="b" type="boolean">1</param>
			<param name="d" type="float">0.1</param>
			<param name="e" type="vector2">1,1</param>
			<param name="f" type="vector3">1,1,1</param>
			<param name="g" type="vector4">1,1,1</param>
			<param name="h" type="matrix43">1,0,0,0,1,0,0,0,1,0,0,0,</param>
		</shader>
		<submesh loddist=10 filename="LOD_10.x"/>
		<submesh loddist=50 filename="LOD_50.x"/>
	</mesh>
	*/
	class CParaMeshXMLFile
	{
	public:
		CParaMeshXMLFile(void);
		~CParaMeshXMLFile(void);

		/** type of the mesh that this file represents. */
		enum ParaMeshXMLType
		{	
			TYPE_MESH_LOD,
			TYPE_CHARACTER_LOD,
			TYPE_MESH_BSP,
			TYPE_MESH_GROUP,
		};

		/** sub LOD meshes. 
		*/
		class CSubMesh
		{
		public:
			CSubMesh():m_fToCameraDist(0.f){};
			CSubMesh(float toCameraDist, const string& filename):m_fToCameraDist(toCameraDist), m_sFileName(filename){};

			/** distance to camera.when this LOD mesh should be drawn. */
			float m_fToCameraDist;

			/** absolute file name of the mesh*/
			string m_sFileName;
		};

	public:
		/**
		* load data from file to class data structure. Parent directory is automatically set to parent directory of filename. 
		*/
		bool LoadFromFile(const string& filename);

		/** same as above, except that parent directory is specified. */
		bool LoadFromFile(const string& filename, const string& parentDir);

		/**
		* load data from file to class data structure. 
		*/
		bool LoadFromBuffer(const char* pData, int nSize);


		/** set such as "model/", relative path will be appended with this one.  */
		void SetParentDirectory(const string& parentDir){m_sParentDirectory = parentDir;}

		/** get such as "model/", relative path will be appended with this one.  */
		const string& GetParentDirectory() { return m_sParentDirectory;}
		
		/**
		* save the content of this file to a given file. 
		* @return false if failed. 
		*/
		bool SaveToFile(const string& filename);

		/** get the type of the mesh that this file represents. */
		ParaMeshXMLType GetType(){return m_nType;};

		/** see ParaEngine::TechniqueHandle. some common values are:
		* -1: UNSPECIFIED. default from mesh. 
		* TECH_SIMPLE_MESH_NORMAL = 3
		* TECH_CHARACTER = 12
		*/
		int GetPrimaryShaderIndex() { return m_nPrimaryShader; }
		
		/** get the number of sub meshes*/
		int GetSubMeshCount(){return (int)m_SubMeshes.size();}

		/** get the sub mesh info at the given index. */
		CSubMesh* GetSubMesh(int nIndex){
			return (nIndex<(int)m_SubMeshes.size()) ? (&(m_SubMeshes[nIndex])) : NULL;
		}

	public:

		/** type of the mesh file. */
		ParaMeshXMLType m_nType;

		/** see ParaEngine::TechniqueHandle. some common values are:
		* -1: UNSPECIFIED. default from mesh. 
		* TECH_SIMPLE_MESH_NORMAL = 3
		* TECH_CHARACTER = 12
		*/
		int m_nPrimaryShader;

		/** file version*/
		int m_nVersion;

		/** sub meshes info. Usually for different LOD information. */
		vector <CSubMesh> m_SubMeshes;

		/** such as "model/", relative path will be appended with this one.  */
		string m_sParentDirectory;

		/** bounding box of the mesh: whether contains bounding box info */
		bool m_bHasBoundingBox;
		/** bounding box of the mesh: min point */
		Vector3 m_vMinPos;
		/** bounding box of the mesh: max point */
		Vector3 m_vMaxPos;

		/** effect parameter block with this asset. */
		CParameterBlock m_paramBlock;
	};

}
