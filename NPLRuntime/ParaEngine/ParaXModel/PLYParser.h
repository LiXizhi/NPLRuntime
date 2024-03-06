#pragma once

namespace ParaEngine
{
	class CParaXModel;

	/** ply point cloud file format parser. For *.ply
	*/
	class PLYParser
	{
	public:
		PLYParser();
		PLYParser(const std::string& filename);
		~PLYParser();

		const std::string& GetFilename() const;
		void SetFilename(const std::string& val);

		/* Note the caller is responsible for deletion of the returned object.
		* parse as animated model
		*/
		CParaXModel* ParseParaXModel(const char* buffer, int nSize, const char* pHint = "point");
		/** use current filename if any. */
		CParaXModel* ParseParaXModel();

	private:
		std::string m_sFilename;
	};
}

