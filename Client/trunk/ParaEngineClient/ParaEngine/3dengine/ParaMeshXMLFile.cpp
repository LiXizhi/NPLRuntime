//-----------------------------------------------------------------------------
// Class:	CParaMeshXMLFile
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Corporation
// Date:	2007.12.16
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FileManager.h"
#include "BaseObject.h"
#ifdef PARAENGINE_MOBILE
#include <tinyxml2.h>
#else
#include <tinyxml.h>
#include <xpath_processor.h>
#endif
#include "ParaMeshXMLFile.h"

#include "memdebug.h"

using namespace ParaEngine;

/** @def larger than which mesh file can not be loaded. */
#define SUPPORTED_MESH_FILE_VERSION		0

CParaMeshXMLFile::CParaMeshXMLFile(void)
{
	m_nType = TYPE_MESH_LOD;
	m_nPrimaryShader = TECH_SIMPLE_MESH_NORMAL;
	m_nVersion = 0;
	m_bHasBoundingBox = false;
	m_vMinPos = Vector3(0,0,0);
	m_vMaxPos = Vector3(0,0,0);
}

CParaMeshXMLFile::~CParaMeshXMLFile(void)
{
}

bool ParaEngine::CParaMeshXMLFile::LoadFromFile( const string& filename )
{
	// auto set parent directory to be the parent directory of the input file.
	return LoadFromFile(filename, CParaFile::GetParentDirectoryFromPath(filename));
}

bool ParaEngine::CParaMeshXMLFile::LoadFromFile(const string& filename, const string& parentDir)
{
	CParaFile file(filename.c_str());
	if(!file.isEof())
	{
		SetParentDirectory(parentDir);

		// load 
		return LoadFromBuffer(file.getBuffer(), (int)file.getSize());
	}
	return false;
}

/** sample ParaEngine mesh xml file
<mesh version=1 type=0>
	<boundingbox minx=0 miny=0 minz=0 maxx=1 maxy=1 maxz=1/>
	<shader index=3/>
	<submesh loddist=10 filename="LOD_10.x"/>
	<submesh loddist=50 filename="LOD_50.x"/>
</mesh>
*/
bool ParaEngine::CParaMeshXMLFile::LoadFromBuffer(const char* pData, int nSize)
{
#ifdef PARAENGINE_MOBILE
	namespace TXML = tinyxml2;
	try
	{
		TXML::XMLDocument doc(true, TXML::COLLAPSE_WHITESPACE);
		doc.Parse(pData, nSize);
		TXML::XMLElement* pRoot = doc.RootElement();
		if (pRoot)
		{
			pRoot->QueryIntAttribute("version", &m_nVersion);
			if (m_nVersion < SUPPORTED_MESH_FILE_VERSION)
			{
				OUTPUT_LOG("can not load para mesh xml file. because of a lower file version.\n");
			}
			int nType = 0;
			pRoot->QueryIntAttribute("type", &nType);
			m_nType = (ParaMeshXMLType)nType;

			for (TXML::XMLNode* pChild = pRoot->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
			{
				TXML::XMLElement* pElement = pChild->ToElement();
				if (pElement)
				{
					std::string tagName = pElement->Name();
					if (tagName == "boundingbox")
					{
						pElement->QueryFloatAttribute("minx", &m_vMinPos.x);
						pElement->QueryFloatAttribute("miny", &m_vMinPos.y);
						pElement->QueryFloatAttribute("minz", &m_vMinPos.z);
						pElement->QueryFloatAttribute("maxx", &m_vMaxPos.x);
						pElement->QueryFloatAttribute("maxy", &m_vMaxPos.y);
						pElement->QueryFloatAttribute("maxz", &m_vMaxPos.z);
					}
					else if (tagName == "shader")
					{
						pElement->QueryIntAttribute("index", &m_nPrimaryShader);
						// TODO: for shader parameters
						for (TXML::XMLNode* pSubChild = pElement->FirstChild(); pSubChild != 0; pSubChild = pSubChild->NextSibling())
						{
							TXML::XMLElement* pParamElement = pSubChild->ToElement();
							if (pParamElement)
							{
								std::string tagName = pParamElement->Name();
								if (tagName == "params")
								{
									CParameter p;
									// param name
									p.SetName(pParamElement->Attribute("name", ""));
									p.SetTypeByString(pParamElement->Attribute("type", ""));
									p.SetValueByString(pParamElement->GetText());
									m_paramBlock.AddParameter(p);
								}
							}
						}
					}
					else if (tagName == "submesh")
					{
						CSubMesh meshInfo;
						pElement->QueryFloatAttribute("loddist", &meshInfo.m_fToCameraDist);
						std::string filepath = pElement->Attribute("filename");
						// check if it is relative path or absolute path
						if (filepath.find('/') != string::npos || filepath.find('\\') != string::npos)
							meshInfo.m_sFileName = filepath;
						else
							meshInfo.m_sFileName = m_sParentDirectory + filepath;
						m_SubMeshes.push_back(meshInfo);
					}
				}
			}
		}
	}
	catch (...)
	{
		OUTPUT_LOG("error parsing xml file %s**.xml \n", m_sParentDirectory.c_str());
		return false;
	}
	return m_SubMeshes.size() > 0;
#else
	TiXmlDocument doc;
	try
	{
		doc.Parse(pData, 0, TIXML_DEFAULT_ENCODING);
		TiXmlElement* pRoot =  doc.RootElement();
		{
			// get mesh file version
			TinyXPath::xpath_processor xpathProc(pRoot, "/mesh/@version");
			TinyXPath::expression_result res = xpathProc.er_compute_xpath();
			TinyXPath::node_set* pNodeSet = res.nsp_get_node_set();
			if(pNodeSet!=0 && pNodeSet->u_get_nb_node_in_set()>0)
			{
				const TiXmlAttribute* att = pNodeSet->XAp_get_attribute_in_set(0);
				m_nVersion = att->IntValue();
			}
		}
		if(m_nVersion < SUPPORTED_MESH_FILE_VERSION)
		{
			OUTPUT_LOG("can not load para mesh xml file. because of a lower file version.\n");
		}

		{
			// get mesh type 
			TinyXPath::xpath_processor xpathProc(pRoot, "/mesh/@type");
			TinyXPath::expression_result res = xpathProc.er_compute_xpath();
			TinyXPath::node_set* pNodeSet = res.nsp_get_node_set();
			if(pNodeSet!=0 && pNodeSet->u_get_nb_node_in_set()>0)
			{
				const TiXmlAttribute* att = pNodeSet->XAp_get_attribute_in_set(0);
				m_nType = (ParaMeshXMLType)att->IntValue();
			}
		}

		{
			// get mesh bounding box
			TinyXPath::xpath_processor xpathProc(pRoot, "/mesh/boundingbox");
			TinyXPath::expression_result res = xpathProc.er_compute_xpath();
			TinyXPath::node_set* pNodeSet = res.nsp_get_node_set();
			if(pNodeSet!=0 && pNodeSet->u_get_nb_node_in_set()>0)
			{
				const TiXmlElement* pElem = pNodeSet->XNp_get_node_in_set(0)->ToElement();
				// parse attributes
				const TiXmlAttribute* pAttr = pElem->FirstAttribute();
				if (pAttr) {
					m_bHasBoundingBox = true;
					for (;pAttr;pAttr = pAttr->Next()) 
					{
						string sName = pAttr->Name();
						if(sName == "minx")
						{
							m_vMinPos.x = (float)(pAttr->DoubleValue());
						}
						else if(sName == "miny")
						{
							m_vMinPos.y = (float)(pAttr->DoubleValue());
						}
						else if(sName == "minz")
						{
							m_vMinPos.z = (float)(pAttr->DoubleValue());
						}
						else if(sName == "maxx")
						{
							m_vMaxPos.x = (float)(pAttr->DoubleValue());
						}
						else if(sName == "maxy")
						{
							m_vMaxPos.y = (float)(pAttr->DoubleValue());
						}
						else if(sName == "maxz")
						{
							m_vMaxPos.z = (float)(pAttr->DoubleValue());
						}
					}
				}
			}
		}

		{
			// get shader index.
			TinyXPath::xpath_processor xpathProc(pRoot, "/mesh/shader/@index");
			TinyXPath::expression_result res = xpathProc.er_compute_xpath();
			TinyXPath::node_set* pNodeSet = res.nsp_get_node_set();
			if(pNodeSet!=0 && pNodeSet->u_get_nb_node_in_set()>0)
			{
				const TiXmlAttribute* att = pNodeSet->XAp_get_attribute_in_set(0);
				m_nPrimaryShader = att->IntValue();
			}
		}
		{
			// get shader parameters. 
			TinyXPath::xpath_processor xpathProc(pRoot, "/mesh/shader/param");
			TinyXPath::expression_result res = xpathProc.er_compute_xpath();
			TinyXPath::node_set* pNodeSet = res.nsp_get_node_set();
			if(pNodeSet!=0 && pNodeSet->u_get_nb_node_in_set()>0)
			{
				int nCount = pNodeSet->u_get_nb_node_in_set();

				for(int i=0;i<nCount;++i)
				{
					// for each parameter
					const TiXmlNode* node = pNodeSet->XNp_get_node_in_set(i);

					CParameter p;
					{
						// param name
						TinyXPath::xpath_processor xpathProc1(node->ToElement(), "@name");
						TinyXPath::expression_result res1 = xpathProc1.er_compute_xpath();
						TinyXPath::node_set* pNodeSet1 = res1.nsp_get_node_set();
						if(pNodeSet1!=0 && pNodeSet1->u_get_nb_node_in_set()>0)
						{
							const TiXmlAttribute* att = pNodeSet1->XAp_get_attribute_in_set(0);
							p.SetName(att->Value());
						}
					}

					{
						// get type
						TinyXPath::xpath_processor xpathProc1(node->ToElement(), "@type");
						TinyXPath::expression_result res1 = xpathProc1.er_compute_xpath();
						TinyXPath::node_set* pNodeSet1 = res1.nsp_get_node_set();
						if(pNodeSet1!=0 && pNodeSet1->u_get_nb_node_in_set()>0)
						{
							const TiXmlAttribute* att = pNodeSet1->XAp_get_attribute_in_set(0);
							p.SetTypeByString(att->Value());
						}
					}
					const TiXmlElement * pParamElement =  node->ToElement();
					if(pParamElement)
					{
						// get text value
						p.SetValueByString(pParamElement->GetText());
						m_paramBlock.AddParameter(p);
					}
				}
			}
		}
		{
			// get mesh info.
			TinyXPath::xpath_processor xpathProc(pRoot, "/mesh/submesh");
			TinyXPath::expression_result res = xpathProc.er_compute_xpath();
			TinyXPath::node_set* pNodeSet = res.nsp_get_node_set();
			if(pNodeSet!=0 && pNodeSet->u_get_nb_node_in_set()>0)
			{
				int nCount = pNodeSet->u_get_nb_node_in_set();
				m_SubMeshes.resize(nCount);

				for(int i=0;i<nCount;++i)
				{
					// for each sub meshes
					const TiXmlNode* node = pNodeSet->XNp_get_node_in_set(i);

					CSubMesh& meshInfo = m_SubMeshes[i];

					{
						// LOD to camera distance
						TinyXPath::xpath_processor xpathProc1(node->ToElement(), "@loddist");
						TinyXPath::expression_result res1 = xpathProc1.er_compute_xpath();
						TinyXPath::node_set* pNodeSet1 = res1.nsp_get_node_set();
						if(pNodeSet1!=0 && pNodeSet1->u_get_nb_node_in_set()>0)
						{
							const TiXmlAttribute* att = pNodeSet1->XAp_get_attribute_in_set(0);
							meshInfo.m_fToCameraDist = (float)(att->DoubleValue());
						}
					}

					{
						// file name of the mesh
						TinyXPath::xpath_processor xpathProc1(node->ToElement(), "@filename");
						TinyXPath::expression_result res1 = xpathProc1.er_compute_xpath();
						TinyXPath::node_set* pNodeSet1 = res1.nsp_get_node_set();
						if(pNodeSet1!=0 && pNodeSet1->u_get_nb_node_in_set()>0)
						{
							const TiXmlAttribute* att = pNodeSet1->XAp_get_attribute_in_set(0);
							string filepath = att->Value();
							// check if it is relative path or absolute path
							if(filepath.find('/')!=string::npos || filepath.find('\\')!=string::npos)
								meshInfo.m_sFileName = filepath;
							else
								meshInfo.m_sFileName = m_sParentDirectory + filepath;
						}
					}
				}
			}
		}
	}
	catch (...)
	{
		OUTPUT_LOG("error parsing xml file %s**.xml \n", m_sParentDirectory.c_str());
		return false;
	}
	return true;
#endif
}

bool ParaEngine::CParaMeshXMLFile::SaveToFile( const string& filename )
{

	return false;
}
