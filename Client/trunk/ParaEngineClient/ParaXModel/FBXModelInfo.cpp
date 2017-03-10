//-----------------------------------------------------------------------------
// Class:	AnimationSplitter
// Authors:	LiPeng
// Company: ParaEngine Corporation
// Date:	2015.07.17
// Desc:    read split the animtion info from xml file;
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_TINYXML2
#include <tinyxml2.h>
#else
#include <tinyxml.h>
#endif

#include "FBXModelInfo.h"

using namespace ParaEngine;

/** @def larger than which mesh file can not be loaded. */
#define SUPPORTED_MESH_FILE_VERSION		0

FBXModelInfo::FBXModelInfo(void)
	:m_vNeckYawAxis(Vector3::ZERO), m_vNeckPitchAxis(Vector3::ZERO)
{
	
}

FBXModelInfo::~FBXModelInfo(void)
{
}

bool ParaEngine::FBXModelInfo::LoadFromFile(const string& filename)
{
	m_sFilename = filename;
	ParaEngine::CParaFile file(filename.c_str());
	if (!file.isEof())
	{
		// load 
		return LoadFromBuffer(file.getBuffer(), (int)file.getSize());
	}
	return false;
}

Vector3 GetAxisFromString(const char* sAxis)
{
	if (sAxis)
	{
		char axis = sAxis[0];
		float fSign = 1.0;
		if (axis == '-'){
			fSign = -1.0;
			axis = sAxis[1];
		}
		else if (axis == '+'){
			fSign = 1.0;
			axis = sAxis[2];
		}
		if (axis == 'x')
			return Vector3::UNIT_X * fSign;
		else if (axis == 'y')
			return Vector3::UNIT_Y * fSign;
		else if (axis == 'z')
			return Vector3::UNIT_Z * fSign;
	}
	return Vector3::ZERO;
}

bool ParaEngine::FBXModelInfo::LoadFromBuffer(const char* pData, int nSize)
{
	try
	{
#ifdef USE_TINYXML2
		namespace TXML = tinyxml2;
		TXML::XMLDocument doc(true, TXML::COLLAPSE_WHITESPACE);
		doc.Parse(pData, nSize);
#else
		TiXmlDocument doc;
		doc.Parse(pData, 0, TIXML_DEFAULT_ENCODING);
#endif
		auto pRoot = doc.RootElement();
		if (pRoot)
		{
			const char* sNeckYawAxis = pRoot->Attribute("neck_yaw_axis");
			if (sNeckYawAxis)
				m_vNeckYawAxis = GetAxisFromString(sNeckYawAxis);
			const char* sNeckPitchAxis = pRoot->Attribute("neck_pitch_axis");
			if (sNeckPitchAxis)
				m_vNeckPitchAxis = GetAxisFromString(sNeckPitchAxis);

			for (auto pChild = pRoot->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
			{
				auto pElement = pChild->ToElement();
				if (pElement)
				{
					std::string tagName = pElement->Value();
					if (tagName == "anim")
					{
						AnimInfo anim;
						pElement->QueryIntAttribute("id", &anim.id);
						pElement->QueryIntAttribute("starttick", &anim.startTick);
						pElement->QueryIntAttribute("endtick", &anim.endTick);
						pElement->QueryIntAttribute("looptype", &anim.loopType);
						float fSpeed = 0;
						pElement->QueryFloatAttribute("speed", &fSpeed);
						anim.fSpeed = fSpeed;
						m_Anims.push_back(anim);
					}
				}
			}
		}
	}
	catch (...)
	{
		OUTPUT_LOG("error parsing xml file %s \n", m_sFilename.c_str());
		return false;
	}

	// Shall we sort in ascending order?
	// std::sort(m_Anims.begin(), m_Anims.end());

	return m_Anims.size() > 0;
}
