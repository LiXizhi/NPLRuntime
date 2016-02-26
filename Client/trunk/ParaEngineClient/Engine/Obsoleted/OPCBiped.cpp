//-----------------------------------------------------------------------------
// Class:	COPCBiped
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2004.7.23
// Revised: 2004.7.23
// description: For player daemon on the network.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "OPCBiped.h"
using namespace ParaEngine;
using namespace std;



/**@def the default sentient radius. This is usually smaller than the smallest quad-tree terrain tile radius.
The default tile radius is usually around 128 meters depending on the game settings.  */
#define DEFAULT_OPC_SENTIENT_RADIUS			50.f
/**@def the default perceptive radius. */
#define DEFAULT_OPC_PERCEPTIVE_RADIUS		50.f

//---------------------------------------------------------
/// desc: hash function for strings used by the Biped object
//---------------------------------------------------------
size_t COPCBiped::GetStringHashValue(const char* str)
{
	char cChar;
	size_t hashcode = 0;
	int i=0;
	while((cChar = str[i++])!='\0')
	{
		hashcode += (int)cChar;
	}
	return hashcode;
}

COPCBiped::COPCBiped(void)
{
	hash_code = 0;
	m_bActive = true;
	// OPC are created as sentient and could detect all sorts of players. 
	SetAlwaysSentient(true);
	//SetGroupID(0);
	SetSentientField(0xffff);
	// OPC need not be saved, so it is not persistent. 
	SetPersistent(false);
	SetPerceptiveRadius(DEFAULT_OPC_SENTIENT_RADIUS);
	SetSentientRadius(DEFAULT_OPC_PERCEPTIVE_RADIUS);
	SetMyType( _OPC );
}

COPCBiped::~COPCBiped(void)
{
}

void COPCBiped::SetHashCode(size_t hashvalue)
{
	hash_code = hashvalue; /* note that POL module name string is not set */
};

void COPCBiped::ReComputeHashcode()
{
	hash_code = GetStringHashValue(GetIdentifier().c_str());
};

//void COPCBiped::SetIdentifier(const char * sID)
//{
//	CBaseObject::SetIdentifier(sID);
//	ReComputeHashcode();
//}

