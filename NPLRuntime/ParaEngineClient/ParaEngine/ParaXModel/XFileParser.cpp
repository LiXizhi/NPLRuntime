//----------------------------------------------------------------------
// Class:	XFile Parser
// Authors:	LiXizhi
// Date:	2014.10.3
// desc: some code is based on Open Asset Import Library (assimp) and WINE directX source code(dxdxof.c  dxfile.h xfile.c parsing.c)
// References: directx file format spec: http://www.cs.ru.ac.za/courses/CSc303/GameDev/Resources/Other/DX113Spec.pdf
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "zlib.h"
#include "util/ByteSwap.h"
#include "util/fast_atof.h"
#include "util/StringHelper.h"
#include "modelheaders.h"
#include "XFileHelper.h"
#include "XFileParser.h"

using namespace ParaEngine;
using namespace ParaEngine::XFile;

// Magic identifier for MSZIP compressed data
#define MSZIP_MAGIC 0x4B43
#define MSZIP_BLOCK 32786
// ------------------------------------------------------------------------------------------------
// Dummy memory wrappers for use with zlib
static void* dummy_alloc(void* /*opaque*/, unsigned int items, unsigned int size)	{
	return ::operator new(items*size);
}

static void  dummy_free(void* /*opaque*/, void* address)	{
	return ::operator delete(address);
}


ParaEngine::XFileParser::XFileParser(const char* pBuffer, int32 nSize)
{
	// set up memory pointers
	P = pBuffer;	End = P + nSize - 1;
	ParseHeader();
}

ParaEngine::XFileParser::~XFileParser()
{
	// kill everything we created
	SAFE_DELETE(mScene);
}

XFile::Scene* ParaEngine::XFileParser::GetImportedData(bool bGiveupDataOwnerhip)
{
	if (!bGiveupDataOwnerhip)
		return mScene;
	else
	{
		XFile::Scene* pTmp = mScene;
		mScene = nullptr;
		return pTmp;
	}
}

// ------------------------------------------------------------------------------------------------
void XFileParser::ParseFile()
{
	bool running = true;
	while (running)
	{
		// read name of next object
		std::string objectName = GetNextToken();
		if (objectName.length() == 0)
			break;

		// parse specific object
		if (objectName == "template")
			ParseDataObjectTemplate();
		else if (objectName == "Frame")
			ParseDataObjectFrame(NULL);
		else if (objectName == "Mesh")
		{
			// some meshes have no frames at all
			Mesh* mesh = new Mesh;
			ParseDataObjectMesh(mesh);
			mScene->mGlobalMeshes.push_back(mesh);
		}
		else if (objectName == "ParaXHeader")
		{
			ParseDataObjectParaXHeader(mScene->m_header);
		}
		else if (objectName == "Material")
		{
			// Material outside of a mesh or node
			Material material;
			ParseDataObjectMaterial(&material);
			mScene->mGlobalMaterials.push_back(material);
		}
		else if (objectName == "}")
		{
			// whatever?
			// OUTPUT_DEBUG("} found in dataObject");
		}
		else
		{
			// unknown format
			// OUTPUT_DEBUG("Unknown data object in animation of .x file");
			ParseUnknownDataObject();
		}
	}
}

/*
template ParaXHeader {
<10000000-0000-0000-0000-123456789000>
array CHAR id[4];
array UCHAR version[4];
DWORD type;
DWORD AnimationBitwise;# boolean animBones,animTextures
Vector minExtent;
Vector maxExtent;
DWORD nReserved;
}
*/
void ParaEngine::XFileParser::ParseDataObjectParaXHeader(ParaXHeaderDef& header)
{
	std::string name;
	readHeadOfDataObject(&name);
	
	ReadCharArray(header.id, 4);
	ReadCharArray((char*)(header.version), 4);

	header.type = ReadInt();
	header.IsAnimated = ReadInt();
	header.minExtent = ReadVector3();
	header.maxExtent = ReadVector3();
	ReadToEndOfDataObject();
}

void ParaEngine::XFileParser::ReadCharArray(char* sOutput, int nCount)
{
	if (mIsBinaryFormat)
	{
		for (int i = 0; i < nCount; ++i)
		{
			sOutput[i] = (char)ReadInt();
		}
	}
	else
	{
		std::string str;
		for (int i = 0; i < nCount; ++i)
		{
			str = GetNextToken();
			sOutput[i] = (char)atoi(str.c_str());
			CheckForSeparator();
		}
	}
}

void ParaEngine::XFileParser::ReadToEndOfDataObject()
{
	// read and ignore data members
	bool running = true;
	while (running)
	{
		std::string s = GetNextToken();

		if (s == "}" || s.length() == 0)
			break;
	}
}
void ParaEngine::XFileParser::ParseHeader()
{
	mMajorVersion = mMinorVersion = 0;
	mIsBinaryFormat = false;
	mBinaryNumCount = 0;
	mLineNumber = 0;
	mScene = NULL;

	// check header
	if (strncmp(P, "xof ", 4) != 0)
		throw DeadlyImportError("Header mismatch, file is not an XFile.");

	// read version. It comes in a four byte format such as "0302"
	mMajorVersion = (uint32)(P[4] - 48) * 10 + (uint32)(P[5] - 48);
	mMinorVersion = (uint32)(P[6] - 48) * 10 + (uint32)(P[7] - 48);

	bool compressed = false;

	// txt - pure ASCII text format
	if (strncmp(P + 8, "txt ", 4) == 0)
		mIsBinaryFormat = false;

	// bin - Binary format
	else if (strncmp(P + 8, "bin ", 4) == 0)
		mIsBinaryFormat = true;

	// tzip - Inflate compressed text format
	else if (strncmp(P + 8, "tzip", 4) == 0)
	{
		mIsBinaryFormat = false;
		compressed = true;
	}
	// bzip - Inflate compressed binary format
	else if (strncmp(P + 8, "bzip", 4) == 0)
	{
		mIsBinaryFormat = true;
		compressed = true;
	}
	else
		ThrowException("Unsupported xfile format ");

	// float size
	mBinaryFloatSize = (uint32)(P[12] - 48) * 1000
		+ (uint32)(P[13] - 48) * 100
		+ (uint32)(P[14] - 48) * 10
		+ (uint32)(P[15] - 48);

	if (mBinaryFloatSize != 32 && mBinaryFloatSize != 64)
		ThrowException("Unknown float size specified in xfile header.");

	// The x format specifies size in bits, but we work in bytes
	mBinaryFloatSize /= 8;

	P += 16;

	// If this is a compressed X file, apply the inflate algorithm to it
	if (compressed)
	{
		/* ///////////////////////////////////////////////////////////////////////
		* COMPRESSED X FILE FORMAT
		* ///////////////////////////////////////////////////////////////////////
		*    [xhead]
		*    2 major
		*    2 minor
		*    4 type    // bzip,tzip
		*    [mszip_master_head]
		*    4 unkn    // checksum?
		*    2 unkn    // flags? (seems to be constant)
		*    [mszip_head]
		*    2 ofs     // offset to next section
		*    2 magic   // 'CK'
		*    ... ofs bytes of data
		*    ... next mszip_head
		*
		*  http://www.kdedevelopers.org/node/3181 has been very helpful.
		* ///////////////////////////////////////////////////////////////////////
		*/

		// build a zlib stream
		z_stream stream;
		stream.opaque = NULL;
		stream.zalloc = &dummy_alloc;
		stream.zfree = &dummy_free;
		stream.data_type = (mIsBinaryFormat ? Z_BINARY : Z_ASCII);

		// initialize the inflation algorithm
		::inflateInit2(&stream, -MAX_WBITS);

		// skip unknown data (checksum, flags?)
		P += 6;

		// First find out how much storage we'll need. Count sections.
		const char* P1 = P;
		uint32 est_out = 0;

		while (P1 + 3 < End)
		{
			// read next offset
			uint16_t ofs = *((uint16_t*)P1);
			PE_SWAP2(ofs); P1 += 2;

			if (ofs >= MSZIP_BLOCK)
				throw DeadlyImportError("X: Invalid offset to next MSZIP compressed block");

			// check magic word
			uint16_t magic = *((uint16_t*)P1);
			PE_SWAP2(magic); P1 += 2;

			if (magic != MSZIP_MAGIC)
				throw DeadlyImportError("X: Unsupported compressed format, expected MSZIP header");

			// and advance to the next offset
			P1 += ofs;
			est_out += MSZIP_BLOCK; // one decompressed block is 32786 in size
		}

		// vector to store uncompressed file for INFLATE'd X files
		std::vector<char> uncompressed;

		// Allocate storage and terminating zero and do the actual uncompressing
		uncompressed.resize(est_out + 1);
		char* out = &uncompressed.front();
		while (P + 3 < End)
		{
			uint16_t ofs = *((uint16_t*)P);
			PE_SWAP2(ofs);
			P += 4;

			// push data to the stream
			stream.next_in = (Bytef*)P;
			stream.avail_in = ofs;
			stream.next_out = (Bytef*)out;
			stream.avail_out = MSZIP_BLOCK;

			// and decompress the data ....
			int ret = ::inflate(&stream, Z_SYNC_FLUSH);
			if (ret != Z_OK && ret != Z_STREAM_END)
				throw DeadlyImportError("X: Failed to decompress MSZIP-compressed data");

			::inflateReset(&stream);
			::inflateSetDictionary(&stream, (const Bytef*)out, MSZIP_BLOCK - stream.avail_out);

			// and advance to the next offset
			out += MSZIP_BLOCK - stream.avail_out;
			P += ofs;
		}

		// terminate zlib
		::inflateEnd(&stream);

		// ok, update pointers to point to the uncompressed file data
		P = &uncompressed[0];
		End = out;

		// FIXME: we don't need the compressed data anymore, could release
		// it already for better memory usage. Consider breaking const-co.
		OUTPUT_DEBUG("Successfully decompressed MSZIP-compressed file\n");
	}
	else
	{
		// start reading here
		ReadUntilEndOfLine();
	}
}

ParaEngine::XFileDataObjectPtr ParaEngine::XFileParser::CreateEnumObject()
{
	XFileDataObjectPtr root(new XFileDataObject());

	bool running = true;
	while (running)
	{
		// read name of next object
		std::string objectName = GetNextToken();
		if (objectName.length() == 0 && IsEndOfFile())
			break;
		if (objectName == "ParaXBody" || objectName == "XDWORDArray" || objectName == "ParaXRefSection" || objectName == "ParaXHeader")
		{
			XFileDataObjectPtr node(new XFileDataObject());
			node->Init(*this, objectName);
			root->AddChild(node);
		}
		else if (objectName == "template")
		{
			ParseUnknownDataObject();
		}
		else
		{
			ParseUnknownDataObject();
		}
	}
	return root;
}

bool ParaEngine::XFileParser::IsEndOfFile()
{
	if (mIsBinaryFormat)
		return (End - P < 2);
	else
		return P >= End;
}


// ------------------------------------------------------------------------------------------------
void XFileParser::ParseDataObjectTemplate()
{
	// parse a template data object. Currently not stored.
	std::string name;
	readHeadOfDataObject(&name);

	// read GUID
	std::string guid = GetNextToken();

	ReadToEndOfDataObject();
}

// ------------------------------------------------------------------------------------------------
void XFileParser::ParseDataObjectFrame(Node* pParent)
{
	// A coordinate frame, or "frame of reference." The Frame template
	// is open and can contain any object. The Direct3D extensions (D3DX)
	// mesh-loading functions recognize Mesh, FrameTransformMatrix, and
	// Frame template instances as child objects when loading a Frame
	// instance.
	std::string name;
	readHeadOfDataObject(&name);

	// create a named node and place it at its parent, if given
	Node* node = new Node(pParent);
	node->mName = name;
	if (pParent)
	{
		pParent->mChildren.push_back(node);
	}
	else
	{
		// there might be multiple root nodes
		if (mScene->mRootNode != NULL)
		{
			// place a dummy root if not there
			if (mScene->mRootNode->mName != "$dummy_root")
			{
				Node* exroot = mScene->mRootNode;
				mScene->mRootNode = new Node(NULL);
				mScene->mRootNode->mName = "$dummy_root";
				mScene->mRootNode->mChildren.push_back(exroot);
				exroot->mParent = mScene->mRootNode;
			}
			// put the new node as its child instead
			mScene->mRootNode->mChildren.push_back(node);
			node->mParent = mScene->mRootNode;
		}
		else
		{
			// it's the first node imported. place it as root
			mScene->mRootNode = node;
		}
	}

	// Now inside a frame.
	// read tokens until closing brace is reached.
	bool running = true;
	while (running)
	{
		std::string objectName = GetNextToken();
		if (objectName.size() == 0)
			ThrowException("Unexpected end of file reached while parsing frame");

		if (objectName == "}")
			break; // frame finished
		else if (objectName == "Frame")
			ParseDataObjectFrame(node); // child frame
		else if (objectName == "FrameTransformMatrix")
			ParseDataObjectTransformationMatrix(node->mTrafoMatrix);
		else if (objectName == "Mesh")
		{
			Mesh* mesh = new Mesh;
			node->mMeshes.push_back(mesh);
			ParseDataObjectMesh(mesh);
		}
		else {
			// OUTPUT_DEBUG("Unknown data object in frame in x file");
			ParseUnknownDataObject();
		}
	}
}

// ------------------------------------------------------------------------------------------------
void XFileParser::ParseDataObjectTransformationMatrix(Matrix4& pMatrix)
{
	// read header, we're not interested if it has a name
	readHeadOfDataObject();

	// read its components
	pMatrix._11 = ReadFloat(); pMatrix._21 = ReadFloat();
	pMatrix._31 = ReadFloat(); pMatrix._41 = ReadFloat();
	pMatrix._12 = ReadFloat(); pMatrix._22 = ReadFloat();
	pMatrix._32 = ReadFloat(); pMatrix._42 = ReadFloat();
	pMatrix._13 = ReadFloat(); pMatrix._23 = ReadFloat();
	pMatrix._33 = ReadFloat(); pMatrix._43 = ReadFloat();
	pMatrix._14 = ReadFloat(); pMatrix._24 = ReadFloat();
	pMatrix._34 = ReadFloat(); pMatrix._44 = ReadFloat();

	// trailing symbols
	CheckForSemicolon();
	CheckForClosingBrace();
}

// ------------------------------------------------------------------------------------------------
void XFileParser::ParseDataObjectMesh(Mesh* pMesh)
{
	std::string name;
	readHeadOfDataObject(&name);

	// read vertex count
	uint32 numVertices = ReadInt();
	pMesh->mPositions.resize(numVertices);

	// read vertices
	for (uint32 a = 0; a < numVertices; a++)
		pMesh->mPositions[a] = ReadVector3();

	// read position faces
	uint32 numPosFaces = ReadInt();
	pMesh->mPosFaces.resize(numPosFaces);
	for (uint32 a = 0; a < numPosFaces; a++)
	{
		uint32 numIndices = ReadInt();
		if (numIndices == 3)
		{
			// read indices
			Face& face = pMesh->mPosFaces[a];
			face.mIndices[0] = (uint16)ReadInt();
			face.mIndices[1] = (uint16)ReadInt();
			face.mIndices[2] = (uint16)ReadInt();
			TestForSeparator();
		}
		else
		{
			ThrowException("Invalid index count for face");
		}
	}

	// here, other data objects may follow
	bool running = true;
	while (running)
	{
		std::string objectName = GetNextToken();

		if (objectName.size() == 0)
			ThrowException("Unexpected end of file while parsing mesh structure");
		else if (objectName == "}")
			break; // mesh finished
		else if (objectName == "MeshNormals")
			ParseDataObjectMeshNormals(pMesh);
		else if (objectName == "MeshTextureCoords")
			ParseDataObjectMeshTextureCoords(pMesh);
		else if (objectName == "MeshVertexColors")
			ParseDataObjectMeshVertexColors(pMesh);
		else if (objectName == "MeshMaterialList")
			ParseDataObjectMeshMaterialList(pMesh);
		else if (objectName == "VertexDuplicationIndices")
			ParseUnknownDataObject(); // we'll ignore vertex duplication indices
		else
		{
			// OUTPUT_DEBUG("Unknown data object in mesh in x file");
			ParseUnknownDataObject();
		}
	}
}

// ------------------------------------------------------------------------------------------------
void XFileParser::ParseDataObjectMeshNormals(Mesh* pMesh)
{
	readHeadOfDataObject();

	// read count
	uint32 numNormals = ReadInt();
	pMesh->mNormals.resize(numNormals);

	// read normal vectors
	for (uint32 a = 0; a < numNormals; a++)
		pMesh->mNormals[a] = ReadVector3();

	// since normal indices always matches position indices, no need to parse it. 
#ifdef READ_NORMAL_INDICES
	// read normal indices
	uint32 numFaces = ReadInt();
	if (numFaces != pMesh->mPosFaces.size())
		ThrowException("Normal face count does not match vertex face count.");

	pMesh->mNormFaces.resize(numFaces);
	for (uint32 a = 0; a < numFaces; a++)
	{
		uint32 numIndices = ReadInt();
		if (numIndices == 3)
		{
			Face& face = pMesh->mNormFaces[a];
			face.mIndices[0] = ReadInt();
			face.mIndices[1] = ReadInt();
			face.mIndices[2] = ReadInt();
			TestForSeparator();
		}
	}
	CheckForClosingBrace();
#else
	ReadToEndOfDataObject();
#endif
}

// ------------------------------------------------------------------------------------------------
void XFileParser::ParseDataObjectMeshTextureCoords(Mesh* pMesh)
{
	readHeadOfDataObject();
	if (pMesh->mNumTextures + 1 > PARAX_MAX_NUMBER_OF_TEXTURECOORDS)
		ThrowException("Too many sets of texture coordinates");

	std::vector<Vector2>& coords = pMesh->mTexCoords[pMesh->mNumTextures++];

	uint32 numCoords = ReadInt();
	if (numCoords != pMesh->mPositions.size())
		ThrowException("Texture coord count does not match vertex count");

	coords.resize(numCoords);
	for (uint32 a = 0; a < numCoords; a++)
		coords[a] = ReadVector2();

	CheckForClosingBrace();
}

// ------------------------------------------------------------------------------------------------
void XFileParser::ParseDataObjectMeshVertexColors(Mesh* pMesh)
{
	readHeadOfDataObject();
	if (pMesh->mNumColorSets + 1 > PARAX_MAX_NUMBER_OF_COLOR_SETS)
		ThrowException("Too many colorsets");
	std::vector<LinearColor>& colors = pMesh->mColors[pMesh->mNumColorSets++];

	uint32 numColors = ReadInt();
	if (numColors != pMesh->mPositions.size())
		ThrowException("Vertex color count does not match vertex count");

	colors.resize(numColors, LinearColor(0, 0, 0, 1.f));
	for (uint32 a = 0; a < numColors; a++)
	{
		uint32 index = ReadInt();
		if (index >= pMesh->mPositions.size())
			ThrowException("Vertex color index out of bounds");

		colors[index] = ReadRGBA();
		// HACK: (thom) Maxon Cinema XPort plugin puts a third separator here, kwxPort puts a comma.
		// Ignore gracefully.
		if (!mIsBinaryFormat)
		{
			FindNextNoneWhiteSpace();
			if (*P == ';' || *P == ',')
				P++;
		}
	}

	CheckForClosingBrace();
}

// ------------------------------------------------------------------------------------------------
void XFileParser::ParseDataObjectMeshMaterialList(Mesh* pMesh)
{
	readHeadOfDataObject();

	// read material count
	/*uint32 numMaterials =*/ ReadInt();
	// read non triangulated face material index count
	uint32 numMatIndices = ReadInt();

	// some models have a material index count of 1... to be able to read them we
	// replicate this single material index on every face
	if (numMatIndices != pMesh->mPosFaces.size() && numMatIndices != 1)
		ThrowException("Per-Face material index count does not match face count.");

	// read per-face material indices
	for (uint32 a = 0; a < numMatIndices; a++)
		pMesh->mFaceMaterials.push_back(ReadInt());

	// in version 03.02, the face indices end with two semicolons.
	// commented out version check, as version 03.03 exported from blender also has 2 semicolons
	if (!mIsBinaryFormat) // && MajorVersion == 3 && MinorVersion <= 2)
	{
		if (P < End && *P == ';')
			++P;
	}

	// if there was only a single material index, replicate it on all faces
	while (pMesh->mFaceMaterials.size() < pMesh->mPosFaces.size())
		pMesh->mFaceMaterials.push_back(pMesh->mFaceMaterials.front());

	// read following data objects
	bool running = true;
	while (running)
	{
		std::string objectName = GetNextToken();
		if (objectName.size() == 0)
			ThrowException("Unexpected end of file while parsing mesh material list.");
		else if (objectName == "}")
			break; // material list finished
		else if (objectName == "{")
		{
			// template materials 
			std::string matName = GetNextToken();
			Material material;
			material.mIsReference = true;
			material.mName = matName;
			pMesh->mMaterials.push_back(material);

			CheckForClosingBrace(); // skip }
		}
		else if (objectName == "Material")
		{
			pMesh->mMaterials.push_back(Material());
			ParseDataObjectMaterial(&pMesh->mMaterials.back());
		}
		else if (objectName == ";")
		{
			// ignore
		}
		else
		{
			// OUTPUT_DEBUG("Unknown data object in material list in x file");
			ParseUnknownDataObject();
		}
	}
}

// ------------------------------------------------------------------------------------------------
void XFileParser::ParseDataObjectMaterial(Material* pMaterial)
{
	std::string matName;
	readHeadOfDataObject(&matName);
	if (matName.empty())
	{
		char temp[16];
		ParaEngine::StringHelper::fast_itoa(mLineNumber, temp, 10);
		matName = std::string("material") + temp;
	}
		
	pMaterial->mName = matName;
	pMaterial->mIsReference = false;

	// read material values
	pMaterial->mDiffuse = ReadRGBA();
	pMaterial->mSpecularExponent = ReadFloat();
	pMaterial->mSpecular = ReadRGB();
	pMaterial->mEmissive = ReadRGB();

	// read other data objects
	bool running = true;
	while (running)
	{
		std::string objectName = GetNextToken();
		if (objectName.size() == 0)
			ThrowException("Unexpected end of file while parsing mesh material");
		else if (objectName == "}")
				break; // material finished
		else if (objectName == "TextureFilename" || objectName == "TextureFileName")
		{
			// some exporters write "TextureFileName" instead.
			std::string texname;
			ParseDataObjectTextureFilename(texname);
			pMaterial->mTextures.push_back(TexEntry(texname));
		}
		else if (objectName == "NormalmapFilename" || objectName == "NormalmapFileName")
		{
			// one exporter writes out the normal map in a separate filename tag
			std::string texname;
			ParseDataObjectTextureFilename(texname);
			pMaterial->mTextures.push_back(TexEntry(texname, true));
		}
		else
		{
			// OUTPUT_DEBUG("Unknown data object in material in x file");
			ParseUnknownDataObject();
		}
	}
}

// ------------------------------------------------------------------------------------------------
void XFileParser::ParseDataObjectTextureFilename(std::string& pName)
{
	readHeadOfDataObject();
	GetNextTokenAsString(pName);
	CheckForClosingBrace();

	// FIX: some files (e.g. AnimationTest.x) have "" as texture file name
	if (!pName.length())
	{
		OUTPUT_DEBUG("Length of texture file name is zero. Skipping this texture.\n");
	}

	// some exporters write double backslash paths out. We simply replace them if we find them
	while (pName.find("\\\\") != std::string::npos)
		pName.replace(pName.find("\\\\"), 2, "\\");
}

// ------------------------------------------------------------------------------------------------
bool XFileParser::ParseUnknownDataObject()
{
	// find opening delimiter
	bool running = true;
	while (running)
	{
		std::string t = GetNextToken();
		if (t.length() == 0)
			ThrowException("Unexpected end of file while parsing unknown segment.");

		if (t == "{")
			break;
	}

	uint32 counter = 1;

	// parse until closing delimiter
	while (counter > 0)
	{
		std::string t = GetNextToken();

		if (t.length() == 0)
			ThrowException("Unexpected end of file while parsing unknown segment.");

		if (t == "{")
			++counter;
		else if (t == "}")
			--counter;
	}
	return true;
}

// ------------------------------------------------------------------------------------------------
//! checks for closing curly brace
void XFileParser::CheckForClosingBrace()
{
	if (GetNextToken() != "}")
		ThrowException("Closing brace expected.");
}

// ------------------------------------------------------------------------------------------------
//! checks for one following semicolon
void XFileParser::CheckForSemicolon()
{
	if (mIsBinaryFormat)
		return;

	if (GetNextToken() != ";")
		ThrowException("Semicolon expected.");
}

// ------------------------------------------------------------------------------------------------
//! checks for a separator char, either a ',' or a ';'
void XFileParser::CheckForSeparator()
{
	if (mIsBinaryFormat)
		return;

	std::string token = GetNextToken();
	if (token != "," && token != ";")
		ThrowException("Separator character (';' or ',') expected.");
}

// ------------------------------------------------------------------------------------------------
// tests and possibly consumes a separator char, but does nothing if there was no separator
void XFileParser::TestForSeparator()
{
	if (mIsBinaryFormat)
		return;

	FindNextNoneWhiteSpace();
	if (P >= End)
		return;

	// test and skip
	if (*P == ';' || *P == ',')
		P++;
}

// ------------------------------------------------------------------------------------------------
void XFileParser::readHeadOfDataObject(std::string* poName)
{
	std::string nameOrBrace = GetNextToken();
	if (nameOrBrace != "{")
	{
		if (poName)
			*poName = nameOrBrace;

		if (GetNextToken() != "{")
			ThrowException("Opening brace expected.");
	}
}

// ------------------------------------------------------------------------------------------------
std::string XFileParser::GetNextToken()
{
	std::string s;

	// process binary-formatted file
	if (mIsBinaryFormat)
	{
		// in binary mode it will only return NAME and STRING token
		// and (correctly) skip over other tokens.

		if (End - P < 2) return s;
		uint32 tok = ReadBinWord();
		uint32 len;

		// standalone tokens
		switch (tok)
		{
		case 1:
			// name token
			if (End - P < 4) return s;
			len = ReadBinDWord();
			if (End - P < int(len)) return s;
			s = std::string(P, len);
			P += len;
			return s;
		case 2:
			// string token
			if (End - P < 4) return s;
			len = ReadBinDWord();
			if (End - P < int(len)) return s;
			s = std::string(P, len);
			P += (len + 2);
			return s;
		case 3:
			// integer token
			P += 4;
			return "<integer>";
		case 5:
			// GUID token
			P += 16;
			return "<guid>";
		case 6:
			if (End - P < 4) return s;
			len = ReadBinDWord();
			P += (len * 4);
			return "<int_list>";
		case 7:
			if (End - P < 4) return s;
			len = ReadBinDWord();
			P += (len * mBinaryFloatSize);
			return "<flt_list>";
		case 0x0a:
			return "{";
		case 0x0b:
			return "}";
		case 0x0c:
			return "(";
		case 0x0d:
			return ")";
		case 0x0e:
			return "[";
		case 0x0f:
			return "]";
		case 0x10:
			return "<";
		case 0x11:
			return ">";
		case 0x12:
			return ".";
		case 0x13:
			return ",";
		case 0x14:
			return ";";
		case 0x1f:
			return "template";
		case 0x28:
			return "WORD";
		case 0x29:
			return "DWORD";
		case 0x2a:
			return "FLOAT";
		case 0x2b:
			return "DOUBLE";
		case 0x2c:
			return "CHAR";
		case 0x2d:
			return "UCHAR";
		case 0x2e:
			return "SWORD";
		case 0x2f:
			return "SDWORD";
		case 0x30:
			return "void";
		case 0x31:
			return "string";
		case 0x32:
			return "unicode";
		case 0x33:
			return "cstring";
		case 0x34:
			return "array";
		}
	}
	// process text-formatted file
	else
	{
		FindNextNoneWhiteSpace();
		if (P >= End)
			return s;

		while ((P < End) && !isspace((unsigned char)*P))
		{
			// either keep token delimiters when already holding a token, or return if first valid char
			if (*P == ';' || *P == '}' || *P == '{' || *P == ',')
			{
				if (!s.size())
					s.append(P++, 1);
				break; // stop for delimiter
			}
			s.append(P++, 1);
		}
	}
	return s;
}

// ------------------------------------------------------------------------------------------------
void XFileParser::FindNextNoneWhiteSpace()
{
	if (mIsBinaryFormat)
		return;

	bool running = true;
	while (running)
	{
		while (P < End && isspace((unsigned char)*P))
		{
			if (*P == '\n')
				mLineNumber++;
			++P;
		}

		if (P >= End)
			return;

		// check if this is a comment
		if ((P[0] == '/' && P[1] == '/') || P[0] == '#')
			ReadUntilEndOfLine();
		else
			break;
	}
}

// ------------------------------------------------------------------------------------------------
void XFileParser::GetNextTokenAsString(std::string& poString)
{
	if (mIsBinaryFormat)
	{
		poString = GetNextToken();
		return;
	}

	FindNextNoneWhiteSpace();
	if (P >= End)
		ThrowException("Unexpected end of file while parsing string");

	if (*P != '"')
		ThrowException("Expected quotation mark.");
	++P;

	while (P < End && *P != '"')
		poString.append(P++, 1);

	if (P >= End - 1)
		ThrowException("Unexpected end of file while parsing string");

	if (P[1] != ';' || P[0] != '"')
		ThrowException("Expected quotation mark and semicolon at the end of a string.");
	P += 2;
}

// ------------------------------------------------------------------------------------------------
void XFileParser::ReadUntilEndOfLine()
{
	if (mIsBinaryFormat)
		return;

	while (P < End)
	{
		if (*P == '\n' || *P == '\r')
		{
			++P; mLineNumber++;
			return;
		}

		++P;
	}
}

// ------------------------------------------------------------------------------------------------
uint16 XFileParser::ReadBinWord()
{
	PE_ASSERT(End - P >= 2);
	const unsigned char* q = (const unsigned char*)P;
	uint16 tmp = q[0] | (q[1] << 8);
	P += 2;
	return tmp;
}

// ------------------------------------------------------------------------------------------------
uint32 XFileParser::ReadBinDWord()
{
	PE_ASSERT(End - P >= 4);
	const unsigned char* q = (const unsigned char*)P;
	uint32 tmp = q[0] | (q[1] << 8) | (q[2] << 16) | (q[3] << 24);
	P += 4;
	return tmp;
}

uint32 XFileParser::ReadInt()
{
	if (mIsBinaryFormat)
	{
		if (mBinaryNumCount == 0 && End - P >= 2)
		{
			uint16 tmp = ReadBinWord(); // 0x06 or 0x03
			if (tmp == 0x06 && End - P >= 4) // array of ints follows
				mBinaryNumCount = ReadBinDWord();
			else // single int follows
				mBinaryNumCount = 1;
		}

		--mBinaryNumCount;
		if (End - P >= 4) {
			return ReadBinDWord();
		}
		else {
			P = End;
			return 0;
		}
	}
	else
	{
		FindNextNoneWhiteSpace();

		// TODO: consider using strtol10 instead???

		// check preceeding minus sign
		bool isNegative = false;
		if (*P == '-')
		{
			isNegative = true;
			P++;
		}

		// at least one digit expected
		if (!isdigit(*P))
			ThrowException("Number expected.");

		// read digits
		uint32 number = 0;
		while (P < End)
		{
			if (!isdigit(*P))
				break;
			number = number * 10 + (*P - 48);
			P++;
		}

		CheckForSeparator();
		return isNegative ? ((uint32)-int(number)) : number;
	}
}

// ------------------------------------------------------------------------------------------------
float XFileParser::ReadFloat()
{
	if (mIsBinaryFormat)
	{
		if (mBinaryNumCount == 0 && End - P >= 2)
		{
			uint16 tmp = ReadBinWord(); // 0x07 or 0x42
			if (tmp == 0x07 && End - P >= 4) // array of floats following
				mBinaryNumCount = ReadBinDWord();
			else // single float following
				mBinaryNumCount = 1;
		}

		--mBinaryNumCount;
		PE_ASSERT(mBinaryFloatSize == 4);
		if (End - P >= 4) {
#ifdef PARAENGINE_MOBILE
			const unsigned char* q = (const unsigned char*)P;
			DWORD dwValue = q[0] | (q[1] << 8) | (q[2] << 16) | (q[3] << 24);
			float result = reinterpret_cast<float&>(dwValue);
#else
			float result = *(float*)P;
#endif
			P += 4;
			return result;
		}
		else {
			P = End;
			return 0;
		}
	}

	// text version
	FindNextNoneWhiteSpace();
	// check for various special strings to allow reading files from faulty exporters
	// I mean you, Blender!
	// Reading is safe because of the terminating zero
	if (strncmp(P, "-1.#IND00", 9) == 0 || strncmp(P, "1.#IND00", 8) == 0)
	{
		P += 9;
		CheckForSeparator();
		return 0.0f;
	}
	else
		if (strncmp(P, "1.#QNAN0", 8) == 0)
		{
		P += 8;
		CheckForSeparator();
		return 0.0f;
		}

	float result = 0.0f;
	P = fast_atoreal_move<float>(P, result);

	CheckForSeparator();

	return result;
}

// ------------------------------------------------------------------------------------------------
Vector2 XFileParser::ReadVector2()
{
	Vector2 vector;
	vector.x = ReadFloat();
	vector.y = ReadFloat();
	TestForSeparator();

	return vector;
}

// ------------------------------------------------------------------------------------------------
Vector3 XFileParser::ReadVector3()
{
	Vector3 vector;
	vector.x = ReadFloat();
	vector.y = ReadFloat();
	vector.z = ReadFloat();
	TestForSeparator();

	return vector;
}

// ------------------------------------------------------------------------------------------------
LinearColor XFileParser::ReadRGBA()
{
	LinearColor color;
	color.r = ReadFloat();
	color.g = ReadFloat();
	color.b = ReadFloat();
	color.a = ReadFloat();
	TestForSeparator();

	return color;
}

// ------------------------------------------------------------------------------------------------
Vector3 XFileParser::ReadRGB()
{
	Vector3 color;
	color.x = ReadFloat();
	color.y = ReadFloat();
	color.z = ReadFloat();
	TestForSeparator();

	return color;
}

// ------------------------------------------------------------------------------------------------
// Throws an exception with a line number and the given text.
void XFileParser::ThrowException(const std::string& pText)
{
	if (mIsBinaryFormat){
		OUTPUT_LOG("XFileParser::ThrowException %s\n", pText.c_str());
	}
	else{
		OUTPUT_LOG("XFileParser::ThrowException Line %d: %s \n", mLineNumber, pText.c_str());
	}
	throw DeadlyImportError(pText);
}


// ------------------------------------------------------------------------------------------------
// Filters the imported hierarchy for some degenerated cases that some exporters produce.
void XFileParser::FilterHierarchy(XFile::Node* pNode)
{
	// if the node has just a single unnamed child containing a mesh, remove
	// the anonymous node inbetween. The 3DSMax kwXport plugin seems to produce this
	// mess in some cases
	if (pNode->mChildren.size() == 1 && pNode->mMeshes.empty())
	{
		XFile::Node* child = pNode->mChildren.front();
		if (child->mName.length() == 0 && child->mMeshes.size() > 0)
		{
			// transfer its meshes to us
			for (uint32 a = 0; a < child->mMeshes.size(); a++)
				pNode->mMeshes.push_back(child->mMeshes[a]);
			child->mMeshes.clear();

			// transfer the transform as well
			pNode->mTrafoMatrix = pNode->mTrafoMatrix * child->mTrafoMatrix;

			// then kill it
			delete child;
			pNode->mChildren.clear();
		}
	}

	// recurse
	for (uint32 a = 0; a < pNode->mChildren.size(); a++)
		FilterHierarchy(pNode->mChildren[a]);
}
