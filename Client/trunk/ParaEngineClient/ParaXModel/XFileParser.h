#pragma once
#include "XFileDataObject.h"

namespace ParaEngine
{
	struct ParaXHeaderDef;
	
	namespace XFile
	{
		struct Node;
		struct Mesh;
		struct Scene;
		struct Material;
	}

	/** The XFileParser reads a XFile either in text or binary form and builds a temporary
	* data structure out of it.
	*/
	class XFileParser
	{
	public:
		friend class XFileDataObject;

		/** Constructor. Creates a data structure out of the XFile given in the memory block.
		* @param pBuffer Null-terminated memory buffer containing the XFile
		*/
		XFileParser(const char* pBuffer, int32 nSize);

		/** Destructor. Destroys all imported data along with it */
		~XFileParser();

		/** Returns the temporary representation of the imported data 
		* @param bGiveupDataOwnerhip: if true, the parse will given up buffer ownership. 
		*/
		XFile::Scene* GetImportedData(bool bGiveupDataOwnerhip=false);

		XFileDataObjectPtr CreateEnumObject();
	protected:
		void ParseHeader();
		void ParseFile();
		void ParseDataObjectTemplate();

		void ParseDataObjectFrame(XFile::Node *pParent);
		void ParseDataObjectTransformationMatrix(Matrix4& pMatrix);
		void ParseDataObjectMesh(XFile::Mesh* pMesh);
		void ParseDataObjectMeshNormals(XFile::Mesh* pMesh);
		void ParseDataObjectMeshTextureCoords(XFile::Mesh* pMesh);
		void ParseDataObjectMeshVertexColors(XFile::Mesh* pMesh);
		void ParseDataObjectMeshMaterialList(XFile::Mesh* pMesh);
		void ParseDataObjectMaterial(XFile::Material* pMaterial);
		void ParseDataObjectTextureFilename(std::string& pName);
		void ParseDataObjectParaXHeader(ParaXHeaderDef& pName);
		bool ParseUnknownDataObject();

		//! places pointer to next begin of a token, and ignores comments
		void FindNextNoneWhiteSpace();

		//! returns next parseable token. Returns empty string if no token there
		std::string GetNextToken();
		bool IsEndOfFile();
		//! reads header of dataobject including the opening brace.
		//! returns false if error happened, and writes name of object
		//! if there is one
		void readHeadOfDataObject(std::string* poName = NULL);

		//! checks for closing curly brace, throws exception if not there
		void CheckForClosingBrace();

		//! checks for one following semicolon, throws exception if not there
		void CheckForSemicolon();

		//! checks for a separator char, either a ',' or a ';'
		void CheckForSeparator();

		/// tests and possibly consumes a separator char, but does nothing if there was no separator
		void TestForSeparator();

		//! reads a x file style string
		void GetNextTokenAsString(std::string& poString);

		void ReadUntilEndOfLine();
		void ReadToEndOfDataObject();

		uint16 ReadBinWord();
		uint32 ReadBinDWord();
		uint32 ReadInt();
		
		float ReadFloat();
		Vector2 ReadVector2();
		Vector3 ReadVector3();
		Vector3 ReadRGB();
		LinearColor ReadRGBA();

		void ReadCharArray(char* sOutput, int nCount);

		/** Throws an exception with a line number and the given text. */
		void ThrowException(const std::string& pText);

		/** Filters the imported hierarchy for some degenerated cases that some exporters produce.
		* @param pData The sub-hierarchy to filter
		*/
		void FilterHierarchy(XFile::Node* pNode);
	
	protected:
		uint32 mMajorVersion, mMinorVersion; ///< version numbers
		bool mIsBinaryFormat; ///< true if the file is in binary, false if it's in text form
		uint32 mBinaryFloatSize; ///< float size in bytes, either 4 or 8
		// counter for number arrays in binary format
		uint32 mBinaryNumCount;

		const char* P;
		const char* End;

		/// Line number when reading in text format
		uint32 mLineNumber;

		/// Imported data
		XFile::Scene* mScene;
	};
}


