#pragma once
#include "XFileParser.h"

namespace ParaEngine
{
	struct ParaXRefObject;

	/** parsing static model: both binary and text format supported */
	class XFileStaticModelParser : public XFileParser
	{
	public:
		/** Constructor. Creates a data structure out of the XFile given in the memory block.
		* @param pBuffer Null-terminated memory buffer containing the XFile
		*/
		XFileStaticModelParser(const char* pBuffer, int32 nSize);
		~XFileStaticModelParser();
	public:
		XFile::Scene* ParseParaXStaticModel();

		bool ParseXRefSection();

		bool ParseDataObjectXRefObject(ParaXRefObject& xref);
	};
}