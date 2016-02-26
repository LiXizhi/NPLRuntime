#pragma once
#include <string>
namespace ParaEngine
{
	class CParaXModel;
	class ParaXModelExporter
	{
	public:
		static bool BmaxExportToSTL(const std::string& input_bmax_filename,const std::string& out_filename, bool binary = false);
	private:
		static bool ExportSTL_Internal(const std::string& out_filename, CParaXModel* mesh, bool binary = false);

	};
}