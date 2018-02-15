#pragma once


namespace ParaEngine
{
	class BlockModel;
	namespace XFile{
		struct Scene;
	};

	class BlockModelManager
	{
	public:

		BlockModelManager();
		~BlockModelManager();
		static BlockModelManager& GetInstance();

		XFile::Scene* GetXFile(const std::string& filename);

		void Cleanup();
	protected:
		/** optimize model, so that co-planer faces are packed as face1(0,1,3)  face2(1,2,3) 
		* if a model can be optimized in this way, we will set scene.m_header.nModelFormat to 1. 
		*/
		bool OptimizeRectFaceModelInPlace(XFile::Scene& scene);
		void RemoveUntexturedFaces(XFile::Scene& scene);
	protected:

		// static model files 
		std::map<std::string, XFile::Scene*> m_Xfiles;
	};
}
	

