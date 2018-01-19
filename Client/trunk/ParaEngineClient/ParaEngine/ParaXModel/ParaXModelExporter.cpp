//-----------------------------------------------------------------------------
// Class:ParaXModel Exporter
// Authors:	leio
// Emails:	
// Date:	2015.11.18
// Desc: export ParaXModel to specific format e.g stl
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXModelExporter.h"
#include "ParaXModel.h"
#include "IO/ParaFile.h"
#include "BMaxModel/BMaxParser.h"
namespace ParaEngine
{
	bool ParaXModelExporter::BmaxExportToSTL(const std::string& input_bmax_filename, const std::string& out_filename, bool binary /*= false*/)
	{
		CParaFile file(input_bmax_filename.c_str());
		BMaxParser p(file.getBuffer(), file.getSize());
		std::string filename = out_filename.empty() ? input_bmax_filename + ".stl" : out_filename;
		return ExportSTL_Internal(filename, p.ParseParaXModel(), binary);
	}

	bool ParaXModelExporter::ExportSTL_Internal(const std::string& out_filename, CParaXModel* mesh, bool binary /*= false*/)
	{
		if (out_filename.empty() || mesh == nullptr)
		{
			return false;
		}
		CParaFile file;
		if (file.CreateNewFile(out_filename.c_str()))
		{
			if (binary)
			{
				char buf[80] = { 0 };
				buf[0] = 'P'; buf[1] = 'a'; buf[2] = 'r'; buf[3] = 'a'; buf[4] = 'E'; buf[5] = 'n';
				buf[6] = 'g'; buf[7] = 'i'; buf[8] = 'n'; buf[9] = 'e'; 
				file.write(buf, 80);
				int size = mesh->m_objNum.nIndices;
				int face_num = size / 3;
				file.WriteDWORD(face_num);
				for (int i = 0; i < size; i++)
				{
					int index_1 = mesh->m_indices[i];
					int index_2 = mesh->m_indices[i + 1];
					int index_3 = mesh->m_indices[i + 2];
					ModelVertex vertex_1 = mesh->m_origVertices[index_1];
					ModelVertex vertex_2 = mesh->m_origVertices[index_2];
					ModelVertex vertex_3 = mesh->m_origVertices[index_3];

					Vector3 a = vertex_3.pos - vertex_1.pos;
					Vector3 b = vertex_3.pos - vertex_2.pos;
					Vector3 normal = a.crossProduct(b);
					file.write(&normal.x, 4);
					file.write(&normal.y, 4);
					file.write(&normal.z, 4);

					file.write(&vertex_1.pos.x, 4);
					file.write(&vertex_1.pos.y, 4);
					file.write(&vertex_1.pos.z, 4);

					file.write(&vertex_2.pos.x, 4);
					file.write(&vertex_2.pos.y, 4);
					file.write(&vertex_2.pos.z, 4);

					file.write(&vertex_3.pos.x, 4);
					file.write(&vertex_3.pos.y, 4);
					file.write(&vertex_3.pos.z, 4);
					

					char dummy[2] = { 0 };
					file.write(dummy, 2);
					i += 2;
				}
				file.close();
			}
			else
			{
				const std::string& name = "ParaEngine";
				file.WriteFormated("solid %s\n", name.c_str());
				int size = mesh->m_objNum.nIndices;
				for (int i = 0; i < size; i++)
				{
					int index_1 = mesh->m_indices[i];
					int index_2 = mesh->m_indices[i + 1];
					int index_3 = mesh->m_indices[i + 2];
					ModelVertex vertex_1 = mesh->m_origVertices[index_1];
					ModelVertex vertex_2 = mesh->m_origVertices[index_2];
					ModelVertex vertex_3 = mesh->m_origVertices[index_3];

					Vector3 a = vertex_3.pos - vertex_1.pos;
					Vector3 b = vertex_3.pos - vertex_2.pos;
					Vector3 normal = a.crossProduct(b);
					file.WriteFormated(" facet normal %f %f %f\n", normal.x, normal.y, normal.z);
					file.WriteFormated("  outer loop\n");
					file.WriteFormated("  vertex %f %f %f\n", vertex_1.pos.x, vertex_1.pos.y, vertex_1.pos.z);
					file.WriteFormated("  vertex %f %f %f\n", vertex_2.pos.x, vertex_2.pos.y, vertex_2.pos.z);
					file.WriteFormated("  vertex %f %f %f\n", vertex_3.pos.x, vertex_3.pos.y, vertex_3.pos.z);
					file.WriteFormated("  endloop\n");
					file.WriteFormated(" endfacet\n");
					i += 2;
				}
				file.WriteFormated("endsolid %s\n", name.c_str());
				file.close();
			}
			return true;
		}
		return false;
	}

}