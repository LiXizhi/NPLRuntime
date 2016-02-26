//-----------------------------------------------------------------------------
// Class:	
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.8.6
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "CharacterDB.h"
#include "ContentLoaders.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModelCanvas.h"
#include "SceneState.h"
#include "AsyncLoader.h"
#include <fstream>
#include <algorithm>
#include "CustomCharModelInstance.h"

using namespace ParaEngine;



namespace ParaEngine
{
	std::list <CharModelInstance*> CharModelInstance::g_listPendingCartoonFaces;
	std::list <CharModelInstance*> CharModelInstance::g_listPendingBodyTextures;
}

CharModelInstance::CharModelInstance()
	:m_bIsCustomModel(false), m_bHasCustomGeosets(false), m_skinIndex(-1), m_cartoonFace(NULL), m_bodyTexture(NULL), m_bNeedRefreshModel(true), m_bNeedComposeBodyTexture(false), m_bNeedRefreshEquipment(true), m_dwSkinColorMask(0xffffffff)
{
	m_pModelCanvas.reset(new ParaXModelCanvas());
	/// this model is an automatic character model
	m_pModelCanvas->SetAutoCharacterModel(true);
	reset();
}

CharModelInstance::~CharModelInstance()
{
	{
		std::list <CharModelInstance*>::iterator itCur, itEnd = g_listPendingCartoonFaces.end();
		for (itCur = g_listPendingCartoonFaces.begin(); itCur != itEnd; ++itCur)
		{
			if ((*itCur) == this)
			{
				// if a pending one already exist, just return. 
				g_listPendingCartoonFaces.erase(itCur);
				break;
			}
		}
	}
	{
		std::list <CharModelInstance*>::iterator itCur, itEnd = g_listPendingBodyTextures.end();
		for (itCur = g_listPendingBodyTextures.begin(); itCur != itEnd; ++itCur)
		{
			if ((*itCur) == this)
			{
				// if a pending one already exist, just return. 
				g_listPendingBodyTextures.erase(itCur);
				break;
			}
		}
	}

	// just delete the temp file,if any
	/*if(!m_sSkinTextureName.empty())
	DeleteFile(m_sSkinTextureName.c_str());

	if(!m_sFaceTextureName.empty())
	DeleteFile(m_sFaceTextureName.c_str());*/

	for (size_t i = 0; i < NUM_TEX; i++)
		m_textures[i].reset();
	SAFE_DELETE(m_cartoonFace);
	SAFE_DELETE(m_bodyTexture);
}

void CharModelInstance::SetModified()
{
	SetModifiedModel();
	SetModifiedEquipment();
}

void CharModelInstance::SetDisplayOptions(int bShowUnderwear, int bShowEars, int bShowHair)
{
	if (bShowUnderwear >= 0)
		showUnderwear = (bShowUnderwear == 1);
	if (bShowEars >= 0)
		showEars = (bShowEars == 1);
	if (bShowHair >= 0)
		showHair = (bShowHair == 1);
}

bool CharModelInstance::GetDisplayOptions(int type)
{
	switch (type)
	{
	case DO_SHOWUNDERWEAR:
		return showUnderwear;
		break;
	case DO_SHOWEARS:
		return showEars;
		break;
	case DO_SHOWHAIR:
		return showHair;
		break;
	default:
		return true;
		break;
	}
}

/** return true if filename ends with *.anim.x */
bool CharModelInstance::IsStaticMeshAsset(const std::string& filename)
{
	int nSize = (int)(filename.size());
	return !(nSize > 7 && (filename[nSize - 7] == '.') &&
		(filename[nSize - 6] == 'a') && (filename[nSize - 5] == 'n') && (filename[nSize - 4] == 'i') && (filename[nSize - 3] == 'm'));
}

ParaXEntity* CharModelInstance::GetBaseModel()
{
	if (m_pModelCanvas)
		return m_pModelCanvas->GetBaseModel();
	else
		return NULL;
}

ParaXEntity* CharModelInstance::GetAnimModel()
{
	if (m_pModelCanvas)
		return m_pModelCanvas->GetAnimModel();
	else
		return NULL;
}

bool CharModelInstance::InitBaseModel(ParaXEntity * pModel)
{
	bool bSuc = true;
	m_pModelCanvas->InitBaseModel(pModel);

	m_bIsCustomModel = false;

	/** this is a character model.*/
	/** Set race and gender of the model. */
	try {
		const string& name = pModel->GetKey();

		if (name.find("character/CC/") != std::string::npos)
			m_bHasCustomGeosets = true;
		else
			m_bHasCustomGeosets = false;

		size_t p4 = name.find_last_of('.');
		if (p4 != std::string::npos)
		{
			size_t p3 = name.find_last_of('/', p4 - 1);
			if (p3 != std::string::npos)
			{
				size_t p2 = name.find_last_of('/', p3 - 1);
				if (p2 != std::string::npos)
				{
					size_t p1 = name.find_last_of('/', p2 - 1);
					if (p1 != std::string::npos)
					{
						std::string raceName = name.substr(p1 + 1, p2 - p1 - 1);
						std::string genderName = name.substr(p2 + 1, p3 - p2 - 1);
						std::string modelname = name.substr(p3 + 1, p4 - p3 - 1);

						/// it is a custom model if and only if the asset file path meets the following criterion
						/// [any directory]/[RaceName]/[GenderName]/[RaceName][GenderName].x
						if (modelname.find(raceName + genderName) == 0)
						{
							/** get gender */
							gender = (genderName == "Female") ? 1 : 0;

							/** get race */
							if (CCharacterDB::GetInstance().GetRaceIDbyName(raceName.c_str(), race))
							{
								m_bIsCustomModel = true;
								m_bHasCustomGeosets = true;
							}
						}
					}
				}
			}
		}
	}
	catch (...) { bSuc = false; }

	if (m_bIsCustomModel == false)
	{
		/** this is not a fully customizable character model.*/
		m_bIsCustomModel = false;

		/** a custom model is definitely not an automatic character model.*/
		m_pModelCanvas->SetAutoCharacterModel(false);
		// TODO: just for testing. Set the default skin.
		// add an interface to change the skin by script.
		SetSkin(0);
	}

	return bSuc;
}

int CharModelInstance::GetSkin()
{
	return m_skinIndex >= 0 ? m_skinIndex : 0;
}

void CharModelInstance::SetSkin(int nSkinIndex)
{
	if (nSkinIndex < 0)
		nSkinIndex = 0;
	if (m_skinIndex == nSkinIndex)
		return;
	// set skin is only for non-customizable character.
	if (m_bIsCustomModel)
		return;

	ParaXEntity * pModelAsset = GetBaseModel();
	if (pModelAsset == NULL)
		return;
	CParaXModel* pModel = pModelAsset->GetModel();
	if (pModel == NULL)
		return;

	ReplaceableTextureGroup grp;

	{
		bool bHasReplaceableTextures = false;
		bool bCanDeduceFromFileName = false;
		// try a non database method first. 
		int nTotalNum = pModel->GetObjectNum().nTextures;
		int k = 0;
		for (int i = 0; i < CParaXModel::MAX_MODEL_TEXTURES; ++i)
		{
			if (pModel->useReplaceTextures[i])
				bHasReplaceableTextures = true;

			if (i < nTotalNum && pModel->textures[i] != 0 && pModel->specialTextures[i] >= 0)
			{
				// if there is both a standard texture and a replaceable texture, we can deduce the replaceable 
				// one from the file name of the standard one.
				bCanDeduceFromFileName = true;
				if (nSkinIndex == 0)
				{
					// use the default. 
					m_textures[k] = pModel->textures[i];
					m_skinIndex = nSkinIndex;
				}
				else
				{
					TextureEntity* pEntity = pModel->textures[i].get();
					string sTextureFileName = pEntity->GetLocalFileName();
					int nSize = (int)sTextureFileName.size();
					if (nSize > 5)
					{
						char nID = sTextureFileName[nSize - 5];
						int nNumberCount = (nID >= '0' && nID <= '9') ? 1 : 0;

						char number[100];
						sTextureFileName.replace(nSize - 4 - nNumberCount, nNumberCount, itoa(nSkinIndex, number, 10));
						if (CParaFile::DoesFileExist(sTextureFileName.c_str(), true))
						{
							m_textures[k] = CGlobals::GetAssetManager()->LoadTexture("", sTextureFileName.c_str(), TextureEntity::StaticTexture);
							m_skinIndex = nSkinIndex;
						}
					}
				}
				k++;
			}
		}
		if (bCanDeduceFromFileName || !bHasReplaceableTextures)
		{
			return;
		}
	}
	// TODO: get replaceable texture group for the num from the database
	{
		// see if this model has skins
		try {
			std::string fn;
			CParaFile::ToCanonicalFilePath(fn, pModelAsset->GetFileName(), true);
			int nLen = (int)fn.length();
			if (fn[nLen - 2] == '.'&& fn[nLen - 1] == 'x'){
				// for .x file. 
				fn[nLen - 1] = 'm';
				fn.append("dx");
			}
			else if (fn[nLen - 1] == '2') {
				// for .m2 file. 
				fn[nLen - 1] = 'd';
				fn.append("x");
			}
			// for .mdx file.

			// get the model ID from model asset file name
			int modelid = 0;
			int modeltype = 0;
			bool bFound = false;
			if (CCharacterDB::GetInstance().GetModelIDfromModelFile(fn, modelid, modeltype))
			{
				bFound = true;
			}
			if (!bFound)
				return;
			// get the replaceable texture group for the specified index.
			if (CCharacterDB::GetInstance().GetReplaceTexturesByModelIDAndSkinID(modelid, nSkinIndex, grp.tex[0], grp.tex[1], grp.tex[2], bFound))
			{
				if (!grp.tex[0].empty())
					++ (grp.count);
				if (!grp.tex[1].empty())
					++ (grp.count);
				if (!grp.tex[2].empty())
					++ (grp.count);
			}
		}
		catch (...) {
			return;
		}
	}

	// reset replaceable textures
	if (grp.count > 0)
	{
		m_skinIndex = nSkinIndex;
		for (size_t i = 0; i < NUM_TEX; i++)
			m_textures[i].reset();
		for (int i = 0; i < grp.count; i++) {
			if (pModel->useReplaceTextures[grp.base + i]) {
				TextureEntity* def = CGlobals::GetAssetManager()->LoadTexture("", makeSkinTexture(pModelAsset->GetFileName().c_str(), grp.tex[i].c_str()).c_str(), TextureEntity::StaticTexture);
				m_textures[i] = def;
			}
		}
	}
}

float CharModelInstance::GetCurrrentUpperBodyTurningAngle()
{
	return m_fUpperBodyTurningAngle;
}
void CharModelInstance::SetUpperBodyTurningAngle(float fAngle)
{
	if (m_fUpperBodyTurningAngle != fAngle)
	{
		if (fAngle > MATH_PI*0.5f)
			fAngle = MATH_PI*0.5f;
		if (fAngle< -MATH_PI*0.5f)
			fAngle = -MATH_PI*0.5f;

		m_fUpperBodyTurningAngle = fAngle;
	}
}


void CharModelInstance::SetUpperBodyUpdownAngle(float fUpDownAngle)
{
	if (fUpDownAngle != m_fUpperBodyUpDownAngle)
	{
		if (fUpDownAngle> MATH_PI*0.5f)
			fUpDownAngle = MATH_PI*0.5f;
		if (fUpDownAngle < -MATH_PI*0.5f)
			fUpDownAngle = -MATH_PI*0.5f;
		m_fUpperBodyUpDownAngle = fUpDownAngle;
	}
}

float CharModelInstance::GetCurrrentUpperBodyUpdownAngle()
{
	return m_fUpperBodyUpDownAngle;
}

void CharModelInstance::AdvanceTime(float fDeltaTime)
{

}

bool CharModelInstance::HasCustomGeosets(CParaXModel* pModel)
{
	//if (m_bNeedRefreshModel && pModel)
	//{
	//	m_bNeedRefreshModel = false;

	//	m_bHasCustomGeosets = false;

	//	int nCount = (int)pModel->geosets.size();
	//	for (int j = 0; j < nCount; j++)
	//	{
	//		int id = pModel->geosets[j].id;
	//		if (id != 0)
	//		{
	//			/*int nBasePart = ((int)floorf((float)id / 100.f));
	//			if (geosets[nBasePart] == 0 && (nBasePart == CSET_ARM_SLEEVES))
	//			{
	//				geosets[nBasePart] = id - nBasePart*100;
	//			}*/
	//			m_bHasCustomGeosets = true;
	//			break;
	//		}
	//	}
	//}
	return m_bHasCustomGeosets;
}
void CharModelInstance::UpdateGeosetsToModel(CParaXModel* pModel)
{
	if (!pModel)
		return;
	// copy geosets visibilities to the model
	if (!showHair)
		geosets[CSET_HAIR] = 0; // hide the hair
	if (!showEars)
		geosets[CSET_EARS] = 0; // hide the ears
	int nCount = (int)pModel->geosets.size();
	for (int j = 0; j < nCount; j++)
	{
		int id = pModel->geosets[j].id;

		//////////////////////////////////////////////////////////////////////////
		// 2007.7
		// id is of format "ccdd", where "cc" is the geoset category CharModelInstance::CharGeosets 
		// and "dd" is individual geoset style starting from 1 of the given category. 
		// if id == 0, it means an always displayed mesh.
		if (id != 0)
		{
			int nBasePart = ((int)floorf((float)id / 100.f));
			if (nBasePart < CharModelInstance::NUM_CHAR_GEOSETS)
				pModel->showGeosets[j] = (id == (nBasePart * 100 + geosets[nBasePart]));
			else
				pModel->showGeosets[j] = false; // this line should never be executed.
		}
		else
		{
			// show all geoset whose id ==0, there may be multiple such geosets.
			pModel->showGeosets[j] = true;
		}
	}
}

void CharModelInstance::UpdateTexturesToModel(CParaXModel* pModel)
{
	if (m_bIsCustomModel)
	{
		// set replaceable textures
		pModel->replaceTextures[1] = m_textures[CHAR_TEX].get();

		pModel->replaceTextures[2] = m_textures[CAPE_TEX].get();
		pModel->replaceTextures[3] = m_textures[WING_TEX].get();
		pModel->replaceTextures[6] = m_textures[HAIR_TEX].get();

		if (m_cartoonFace && m_cartoonFace->m_bNeedUpdate)
		{
			m_cartoonFace->GetComponent(CFS_FACE).SetColor(GetSkinColorMask());
			m_cartoonFace->compose(m_textures[CARTOON_FACE_TEX], this);
		}

		pModel->replaceTextures[7] = m_textures[CARTOON_FACE_TEX].get();
		pModel->replaceTextures[8] = m_textures[FUR_TEX].get();
	}
	else
	{
		for (int i = 0, k = 0; i < CParaXModel::MAX_MODEL_TEXTURES; ++i)
		{
			int nIndex = pModel->specialTextures[i];
			if (nIndex >= 0)
			{
				if (m_textures[k].get() != 0)
					pModel->replaceTextures[nIndex] = m_textures[k].get();
				else
					pModel->replaceTextures[nIndex] = pModel->textures[i].get();
				k++;
			}
		}
	}
}

bool CharModelInstance::AnimateModel(SceneState * sceneState, const AnimIndex& CurrentAnim, const AnimIndex& NextAnim, const AnimIndex& BlendingAnim, float blendingFactor, IAttributeFields* pAnimInstance)
{
	ParaXEntity * pModelAsset = GetBaseModel();
	if (pModelAsset == NULL)
		return false;
	int nIndex = sceneState ? pModelAsset->GetLodIndex(sceneState->GetCameraToCurObjectDistance()) : 0;
	CParaXModel* pModel = pModelAsset->GetModel(nIndex);

	if (pModel == NULL)
		return false;
	if (!pModel->HasAnimation())
		return true;
	if (!CurrentAnim.IsValid())
		return false;
	/**
	* copy the current instance states to ParaX model
	*/
	pModel->m_CurrentAnim = CurrentAnim;
	pModel->m_NextAnim = NextAnim;
	pModel->m_BlendingAnim = BlendingAnim;
	pModel->blendingFactor = blendingFactor;


	if (m_bIsCustomModel)
	{
		// refresh the model and geosets settings based on character parameters. 
		RefreshEquipment_imp();
		RefreshModel_imp();

		UpdateGeosetsToModel(pModel);
		UpdateTexturesToModel(pModel);
	}
	else
	{
		if (HasCustomGeosets(pModel))
			UpdateGeosetsToModel(pModel);
		UpdateTexturesToModel(pModel);
	}
	static CharacterPose charPose;
	charPose.m_fUpperBodyFacingAngle = m_fUpperBodyTurningAngle;
	charPose.m_fUpperBodyUpDownAngle = m_fUpperBodyUpDownAngle;
	// calculate bones
	bool bRes = m_pModelCanvas->Animate(sceneState, &charPose, CurrentAnim.nAnimID, pAnimInstance);
	return bRes;
}

void CharModelInstance::BuildShadowVolume(SceneState * sceneState, ShadowVolume * pShadowVolume, LightParams* pLight, Matrix4* mxWorld)
{
	if (m_pModelCanvas)
		m_pModelCanvas->BuildShadowVolume(sceneState, pShadowVolume, pLight, mxWorld);
}

void CharModelInstance::Draw(SceneState * sceneState, CParameterBlock* materialParams)
{
	// draw model
	if (m_pModelCanvas)
		m_pModelCanvas->Draw(sceneState, materialParams);
}
//
//DatabaseEntity* CharModelInstance::GetCharDB()
//{
//	if(m_pDbCharacters == NULL)
//	{
//		LoadCharacterDatabase();
//		if(m_pDbCharacters == NULL)
//		{
//			// TODO: load the default one. maybe the simplest model.
//			return NULL;
//		}
//	}
//	return m_pDbCharacters;
//}
//
//void CharModelInstance::LoadCharacterDatabase()
//{
//	if(m_pDbCharacters == NULL)
//	{
//		m_pDbCharacters = CGlobals::GetAssetManager()->GetDatabase("cs");
//		if(m_pDbCharacters == NULL)
//		{
//			m_pDbCharacters = CGlobals::GetAssetManager()->LoadDatabase("cs", "database/characters.db");
//		}
//	}
//}

void CharModelInstance::SetModifiedModel()
{
	m_bNeedRefreshModel = true;
}

void CharModelInstance::RefreshModel_imp()
{
	if (!m_bNeedRefreshModel)
		return;
	else
		m_bNeedRefreshModel = false;

	if (!m_bIsCustomModel)
		return;
	for (int i = 0; i < NUM_CHAR_GEOSETS; i++)
		geosets[i] = 1;
	// facial hair/decorations
	geosets[CSET_FACIAL_HAIR1] = geosets[CSET_FACIAL_HAIR2] = 0;
	// hide skirt
	geosets[CSET_SKIRT] = 0;
	// hide cape
	geosets[CSET_CAPE] = 0;
	// hide wings
	geosets[CSET_WINGS] = 0;

	//// ears
	//if (showEars) 
	//	geosets[CSET_EARS] = 2;

	if (m_bodyTexture == NULL)
	{
		m_bodyTexture = new CharTexture();
	}
	else
	{
		m_bodyTexture->Reset();
	}
	CharTexture& tex = *m_bodyTexture;

	string sTex0, sTex1, sTex2;

	/**
	* character base texture(skin)
	*/
	if (CCharacterDB::GetInstance().GetCharacterSkins(race, gender, SkinType, 0, skinColor, sTex0, sTex1, sTex2))
	{
		/// character base texture
		tex.addLayer(sTex0.c_str(), CR_BASE, 0, GetSkinColorMask());

		/// fur texture
		if (!sTex1.empty())
		{
			m_textures[FUR_TEX] = CGlobals::GetAssetManager()->LoadTexture("", sTex1, TextureEntity::StaticTexture);
		}
		else
			m_textures[FUR_TEX].reset();
	}

	/**
	* character base texture (underwear)
	*/
	if (showUnderwear) {
		if (CCharacterDB::GetInstance().GetCharacterSkins(race, gender, UnderwearType, 0, skinColor, sTex0, sTex1, sTex2))
		{
			/// character base texture
			// If we're wearing pants, hide the panties
			if (equipment[CS_PANTS] == 0)
				tex.addLayer(sTex0.c_str(), CR_LEG_UPPER, 1); // panties
			// If we're wearing a shirt/chest armor, hide the bra
			if (equipment[CS_CHEST] == 0 && equipment[CS_SHIRT] == 0)
				tex.addLayer(sTex1.c_str(), CR_TORSO_UPPER, 1);  // bra
		}
	}

	/**
	* character base texture (face)
	*/
	if (CCharacterDB::GetInstance().GetCharacterSkins(race, gender, FaceType, faceType, skinColor, sTex0, sTex1, sTex2))
	{
		/// character base texture
		tex.addLayer(sTex0.c_str(), CR_FACE_LOWER, 1);
		tex.addLayer(sTex1.c_str(), CR_FACE_UPPER, 1);
	}

	/**
	* character base texture (facial hair)
	*/
	if (CCharacterDB::GetInstance().GetCharacterSkins(race, gender, FacialHairType, facialHair, skinColor, sTex0, sTex1, sTex2))
	{
		/// character base texture
		tex.addLayer(sTex0.c_str(), CR_FACE_LOWER, 2);
		tex.addLayer(sTex1.c_str(), CR_FACE_UPPER, 2);
	}

	/**
	* character base texture (hair + hair on face)
	*/
	if (CCharacterDB::GetInstance().GetCharacterSkins(race, gender, HairType, hairStyle, hairColor, sTex0, sTex1, sTex2))
	{
		if (!sTex0.empty())
		{
			m_textures[HAIR_TEX] = CGlobals::GetAssetManager()->LoadTexture("", sTex0, TextureEntity::StaticTexture);
		}
		else
		{
			// TODO: we're missing a hair texture. try with hair style 1.
			m_textures[HAIR_TEX].reset();
		}
		tex.addLayer(sTex1.c_str(), CR_FACE_LOWER, 3);
		tex.addLayer(sTex2.c_str(), CR_FACE_UPPER, 3);
	}

	/**
	* facial hair geosets
	*/
	if (!CCharacterDB::GetInstance().GetFacialHairGeosets(race, gender, facialHair, geosets[CSET_FACIAL_HAIR1], geosets[CSET_FACIAL_HAIR2], geosets[CSET_FACIAL_HAIR3]))
	{
		// if not found
		geosets[CSET_FACIAL_HAIR1] = 0;
		geosets[CSET_FACIAL_HAIR2] = 0;
	}
	else
	{
		// this is head, the index begins by 1 instead of 0.
		geosets[CSET_FACIAL_HAIR3] ++;
	}

	/**
	* select hairstyle geoset(s)
	*/
	if (CCharacterDB::GetInstance().GetHairGeosets(race, gender, hairStyle, geosets[CSET_HAIR]))
	{
		if (geosets[CSET_HAIR] == 0)
			showHair = false;
		else
			showHair = true;
	}
	else
	{
		// if not found
		geosets[CSET_HAIR] = 2;
		showHair = true;
	}

	// dress ups
	hadRobe = false;
	for (int i = 0; i < NUM_CHAR_SLOTS; i++)
	{
		int sn = CCharCustomizeSysSetting::GetSlotOrder(i, hadRobe);
		if (equipment[sn] != 0)
			UpdateBaseModelByEquipment(sn, equipment[sn], 10 + i, tex);
	}

	tex.compose(m_textures[CHAR_TEX], this);

	// regenerate cartoon face texture, if and only if the user has used it at least once.
	// here we will regenerate if the head is not the default geoset.
	if (geosets[CSET_FACIAL_HAIR3]>1)
	{
		if (m_cartoonFace == 0)
		{
			m_cartoonFace = new CartoonFace();
		}
		// used to update and compose cartoon face texture here. but it is moved to the animate function to be composed on demand 
		if (m_textures[CARTOON_FACE_TEX].get() == 0)
		{
			m_cartoonFace->SetModified(true);
		}
	}
	else
	{
		if (m_cartoonFace != 0)
		{
			m_cartoonFace->SetModified(false);
		}
	}
}

void CharModelInstance::UpdateBaseModelByEquipment(int slot, int itemid, int layer, CharTexture &tex)
{
	if (!m_bIsCustomModel)
		return;
	if (slot == CS_PANTS && geosets[CSET_ROBE] == 2)
		return; // if we are wearing a robe, no pants will be drawn

	if (slot == CS_ARIES_CHAR_PANT && showAriesPantByItem == false) {
		//geosets[CSET_PANTS] = 0;
		return; // if we are wearing an aries shirt with pants mask, no pants will be drawn
	}
	if (slot == CS_ARIES_CHAR_HAND && showAriesHandByItem == false) {
		//geosets[CSET_GLOVES] = 0;
		return; // if we are wearing an aries shirt with gloves mask, no gloves will be drawn
	}
	if (slot == CS_ARIES_CHAR_FOOT && showAriesFootByItem == false) {
		//geosets[CSET_BOOTS] = 0;
		return; // if we are wearing an aries shirt with boots mask, no boots will be drawn
	}
	if (slot == CS_ARIES_CHAR_GLASS && showAriesGlassByItem == false) {
		//geosets[CSET_FACIAL_HAIR2] = 0;
		return; // if we are wearing an aries shirt with glasses mask, no glasses will be drawn
	}

	try {
		/**
		* get item's model type and model id by its item id
		*/
		int nItemType = -1;
		int nItemModelID = -1;
		if (CCharacterDB::GetInstance().GetModelIDfromItemID(itemid, nItemType, nItemModelID))
		{
		}
		/** get the model information from the model display database by the model id*/
		int GeosetA = 0, GeosetB = 0, GeosetC = 0, GeosetD = 0, GeosetE = 0;
		int flags = 0, GeosetVisID1 = 0, GeosetVisID2 = 0;
		string skin, skin2, TexArmUpper, TexArmLower, TexHands, TexChestUpper, TexChestLower, TexLegUpper, TexLegLower, TexFeet;
		if (nItemModelID >= 0)
		{
			if (CCharacterDB::GetInstance().GetModelDisplayInfo(nItemModelID, GeosetA, GeosetB, GeosetC, GeosetD, GeosetE, flags, GeosetVisID1, GeosetVisID2,
				skin, skin2, TexArmUpper, TexArmLower, TexHands, TexChestUpper, TexChestLower, TexLegUpper, TexLegLower, TexFeet))
			{
			}
		}
		// shirt sleeves
		if (nItemType == IT_SHIRT || nItemType == IT_CHEST || nItemType == IT_ROBE) {
			geosets[CSET_ARM_SLEEVES] = 1 + GeosetA;
		}

		// pants sleeves
		if (nItemType == IT_PANTS) {
			geosets[CSET_PANTS] = 1 + GeosetB;
		}

		// boots
		if (nItemType == IT_BOOTS) {
			geosets[CSET_BOOTS] = 1 + GeosetA;
		}

		// gloves
		if (nItemType == IT_GLOVES) {
			geosets[CSET_GLOVES] = 1 + GeosetA;
		}

		// capes
		if (nItemType == IT_CAPE) {
			geosets[CSET_CAPE] = 1 + GeosetA;
			// load the cape texture
			const char *tex = skin.c_str();
			if (skin.length() > 0)
				m_textures[CAPE_TEX] = CGlobals::GetAssetManager()->LoadTexture("", makeItemTexture(CR_CAPE, tex).c_str(), TextureEntity::StaticTexture);
		}

		// robe
		if (geosets[CSET_ROBE] == 1)
			geosets[CSET_ROBE] = 1 + GeosetC;
		if (geosets[CSET_ROBE] == 2) {
			geosets[CSET_BOOTS] = 0;		// hide the boots
			geosets[CSET_PANTS] = 0;		// hide the pants
			geosets[CSET_TABARD] = 0;	// also hide the tabard.
			hadRobe = true;
		}

		// wings
		if (nItemType == IT_ARIES_CHAR_WING) {
			geosets[CSET_WINGS] = 1 + GeosetA;
			// load the wing texture
			const char *tex = skin.c_str();
			if (skin.length() > 0)
				m_textures[WING_TEX] = CGlobals::GetAssetManager()->LoadTexture("", makeItemTexture(CR_ARIES_CHAR_WING, tex).c_str(), TextureEntity::StaticTexture);
		}


		if (nItemType == IT_ARIES_CHAR_SHIRT || nItemType == IT_ARIES_CHAR_SHIRT_TEEN)
		{
			geosets[CSET_ARM_SLEEVES] = 1 + GeosetA;
			if (nItemType == IT_ARIES_CHAR_SHIRT)
			{
				tex.addLayer(makeItemTexture(CR_ARIES_CHAR_SHIRT, skin.c_str()).c_str(), CR_ARIES_CHAR_SHIRT, layer);
				tex.addLayer(makeItemTexture(CR_ARIES_CHAR_SHIRT_OVERLAYER, skin2.c_str()).c_str(), CR_ARIES_CHAR_SHIRT_OVERLAYER, layer);
			}
			else if (nItemType == IT_ARIES_CHAR_SHIRT_TEEN)
			{
				tex.addLayer(makeItemTexture(CR_ARIES_CHAR_SHIRT_TEEN, skin.c_str()).c_str(), CR_ARIES_CHAR_SHIRT_TEEN, layer);
			}
			if (flags == 1) { // set item use mask item

				if (GeosetD >= 0){
					showAriesGlassByItem = false;
					geosets[CSET_FACIAL_HAIR2] = GeosetD; // show or hide the character glasses geometry set according to the GeosetD, if 0 hide
				}
				else
					showAriesGlassByItem = true;

				if (GeosetE >= 0){
					showAriesPantByItem = false;
					geosets[CSET_PANTS] = GeosetE; // show or hide the character pants geometry set according to the GeosetE, if 0 hide
				}
				else
					showAriesPantByItem = true;

				if (GeosetVisID1 >= 0){
					showAriesHandByItem = false;
					geosets[CSET_GLOVES] = GeosetVisID1; // show or hide the character gloves geometry set according to the GeosetVisID1, if 0 hide
				}
				else
					showAriesHandByItem = true;

				if (GeosetVisID2 >= 0){
					showAriesFootByItem = false;
					geosets[CSET_BOOTS] = GeosetVisID2; // show or hide the character foot geometry set according to the GeosetVisID2, if 0 hide
				}
				else
					showAriesFootByItem = true;
			}
			else{
				showAriesGlassByItem = true;
				showAriesPantByItem = true;
				showAriesHandByItem = true;
				showAriesFootByItem = true;
			}
		}
		else if (nItemType == IT_ARIES_CHAR_PANT)
		{
			geosets[CSET_PANTS] = 1 + GeosetB;
			tex.addLayer(makeItemTexture(CR_ARIES_CHAR_PANT, skin.c_str()).c_str(), CR_ARIES_CHAR_PANT, layer);
		}
		else if (nItemType == IT_ARIES_CHAR_HAND)
		{
			geosets[CSET_GLOVES] = 1 + GeosetA;
			tex.addLayer(makeItemTexture(CR_ARIES_CHAR_HAND, skin.c_str()).c_str(), CR_ARIES_CHAR_HAND, layer);
		}
		else if (nItemType == IT_ARIES_CHAR_FOOT)
		{
			geosets[CSET_BOOTS] = 1 + GeosetA;
			tex.addLayer(makeItemTexture(CR_ARIES_CHAR_FOOT, skin.c_str()).c_str(), CR_ARIES_CHAR_FOOT, layer);
		}
		else if (nItemType == IT_ARIES_CHAR_GLASS)
		{
			geosets[CSET_FACIAL_HAIR2] = 1 + GeosetA;
			tex.addLayer(makeItemTexture(CR_ARIES_CHAR_GLASS, skin.c_str()).c_str(), CR_ARIES_CHAR_GLASS, layer);
		}
		else if (nItemType == IT_ARIES_PET_HEAD)
		{
			geosets[CSET_ARM_SLEEVES] = 1 + GeosetA;
			tex.addLayer(makeItemTexture(CR_ARIES_PET_HEAD, skin.c_str()).c_str(), CR_ARIES_PET_HEAD, layer);
		}
		else if (nItemType == IT_ARIES_PET_BODY)
		{
			geosets[CSET_PANTS] = 1 + GeosetB;
			tex.addLayer(makeItemTexture(CR_ARIES_PET_BODY, skin.c_str()).c_str(), CR_ARIES_PET_BODY, layer);
		}
		else if (nItemType == IT_ARIES_PET_TAIL)
		{
			geosets[CSET_BOOTS] = 1 + GeosetA;
			tex.addLayer(makeItemTexture(CR_ARIES_PET_TAIL, skin.c_str()).c_str(), CR_ARIES_PET_TAIL, layer);
		}
		else if (nItemType == IT_ARIES_PET_WING)
		{
			geosets[CSET_GLOVES] = 1 + GeosetA;
			tex.addLayer(makeItemTexture(CR_ARIES_PET_WING, skin.c_str()).c_str(), CR_ARIES_PET_WING, layer);
		}

		tex.addLayer(makeItemTexture(CR_ARM_UPPER, TexArmUpper.c_str()).c_str(), CR_ARM_UPPER, layer);
		tex.addLayer(makeItemTexture(CR_ARM_LOWER, TexArmLower.c_str()).c_str(), CR_ARM_LOWER, layer);
		tex.addLayer(makeItemTexture(CR_TORSO_UPPER, TexChestUpper.c_str()).c_str(), CR_TORSO_UPPER, layer);
		tex.addLayer(makeItemTexture(CR_TORSO_LOWER, TexChestLower.c_str()).c_str(), CR_TORSO_LOWER, layer);
		tex.addLayer(makeItemTexture(CR_LEG_UPPER, TexLegUpper.c_str()).c_str(), CR_LEG_UPPER, layer);
		tex.addLayer(makeItemTexture(CR_LEG_LOWER, TexLegLower.c_str()).c_str(), CR_LEG_LOWER, layer);
		tex.addLayer(makeItemTexture(CR_HAND, TexHands.c_str()).c_str(), CR_HAND, layer);
		tex.addLayer(makeItemTexture(CR_FOOT, TexFeet.c_str()).c_str(), CR_FOOT, layer);

		if (nItemType == IT_WINGS)
		{
			// we use the face lower texture for wings in Aries project
			tex.addLayer(makeItemTexture(CR_WINGS, skin.c_str()).c_str(), CR_WINGS, layer);
		}
	}
	catch (...) {
	}
}


bool CharModelInstance::slotHasModel(int i)
{
	return (i == CS_HEAD || i == CS_SHOULDER || i == CS_HAND_LEFT || i == CS_HAND_RIGHT || i == CS_FACE_ADDON || i == CS_ARIES_CHAR_BACK || i == CS_ARIES_CHAR_GROUND_EFFECT || i == CS_ARIES_CHAR_GEM_BUFF_EFFECT || i == CS_ARIES_CHAR_RING_BUFF_EFFECT);
}

void CharModelInstance::RefreshEquipment_imp()
{
	if (!m_bNeedRefreshEquipment)
		return;
	else
		m_bNeedRefreshEquipment = false;

	if (!m_bIsCustomModel)
		return;
	for (int i = 0; i < NUM_CHAR_SLOTS; i++) {
		if (slotHasModel(i))
			RefreshItem(i);
	}
}

void CharModelInstance::SetModifiedEquipment()
{
	m_bNeedRefreshEquipment = true;
}

void CharModelInstance::RefreshItem(int slot)
{
	if (!m_bIsCustomModel)
		return;
	// delete all attachments in that slot
	m_pModelCanvas->deleteSlot(slot);

	int itemid = equipment[slot];
	if (itemid != 0) {

		// load new model(s)
		int id1 = -1, id2 = -1;
		int regionpath = -1;
		float sc = 1.0f;

		if (slot == CS_HEAD) {
			id1 = ATT_ID_HEAD;
			regionpath = CR_HEAD;
		}
		else if (slot == CS_SHOULDER) {
			id1 = ATT_ID_SHOULDER_LEFT;
			id2 = ATT_ID_SHOULDER_RIGHT;
			regionpath = CR_SHOULDER;
			// sc = shoulderscales[race][gender];
		}
		else if (slot == CS_HAND_LEFT)
			id1 = ATT_ID_HAND_LEFT;
		else if (slot == CS_HAND_RIGHT)
			id1 = ATT_ID_HAND_RIGHT;
		else if (slot == CS_FACE_ADDON) {
			id1 = ATT_ID_FACE_ADDON;
			regionpath = CR_FACE_ADDON;
		}
		else if (slot == CS_ARIES_CHAR_BACK)
			id1 = ATT_ID_BACK_ADDON;
		else if (slot == CS_ARIES_CHAR_GROUND_EFFECT)
			id1 = ATT_ID_GROUND;
		else if (slot == CS_ARIES_CHAR_GEM_BUFF_EFFECT)
			id1 = ATT_ID_GROUND;
		else if (slot == CS_ARIES_CHAR_RING_BUFF_EFFECT)
			id1 = ATT_ID_GROUND;
		else
			return;


		/**
		* get item's model type and model id by its item id
		*/
		int nItemType = -1;
		int nItemModelID = -1;
		if (CCharacterDB::GetInstance().GetModelIDfromItemID(itemid, nItemType, nItemModelID))
		{
		}

		if (slot == CS_HAND_LEFT || slot == CS_HAND_RIGHT) {
			if (nItemType == IT_SHIELD) {
				regionpath = CR_SHIELD;
				id1 = ATT_ID_SHIELD;
			}
			else
				regionpath = CR_WEAPON;
		}
		else if (slot == CS_ARIES_CHAR_BACK) {
			regionpath = CR_ARIES_CHAR_BACK;
		}
		else if (slot == CS_ARIES_CHAR_GROUND_EFFECT) {
			regionpath = CR_ARIES_CHAR_GROUND_EFFECT;
		}
		else if (slot == CS_ARIES_CHAR_GEM_BUFF_EFFECT) {
			regionpath = CR_ARIES_CHAR_GEM_BUFF_EFFECT;
		}
		else if (slot == CS_ARIES_CHAR_RING_BUFF_EFFECT) {
			regionpath = CR_ARIES_CHAR_GEM_BUFF_EFFECT;
		}


		/** get the model information from the model display database by the model id*/
		string Model, Model2, Skin, Skin2;
		int visualid = -1;
		int GeosetVisID1 = -1;
		int GeosetVisID2 = -1;
		if (nItemModelID >= 0)
		{
			if (CCharacterDB::GetInstance().GetModelInfoFromModelID(nItemModelID, Model, Model2, Skin, Skin2, visualid, GeosetVisID1, GeosetVisID2))
			{
			}
		}
		/// adjust hair visibility
		if (nItemType == IT_HEAD && autoHair)
		{
			//TODO: Work out what exactly these geosets mean and act accordingly.
			// These values point to records in HelmetGeosetVisData.dbc
			// Still not sure if the 2 columns are for male / female or
			// for facial hair / normal hair
			if (GeosetVisID1 > 245)
				showHair = false;

			if (GeosetVisID2 > 265)
				showFacialHair = false;
		}
		if (nItemType == IT_HEAD)
		{
			if (GeosetVisID1 == 1)
			{
				//geosets[CSET_EARS] = 0; // hide the character ear
				showEars = false;
			}
			else
			{
				//geosets[CSET_EARS] = 1; // show the default character ear
				showEars = true;
			}
		}

		{
			string modelpath;
			bool succ = false;
			CanvasAttachment *att = NULL;
			union{
				MeshEntity* AttachedModel;
				ParaXEntity* AttachedParaXModel;
			};
			AttachedModel = NULL;

			if (id1 >= 0)
			{
				/*
				// TODO:shall we use some special naming convention for head models?
				if(nItemType==IT_HEAD)
				{
				}
				*/

				modelpath = makeItemModel(regionpath, Model.c_str());

				if (!modelpath.empty())
				{
					bool bIsStaticMeshModel = IsStaticMeshAsset(modelpath);
					if (bIsStaticMeshModel)
					{
						AttachedModel = CGlobals::GetAssetManager()->LoadMesh("", modelpath.c_str());
					}
					else
					{
						AttachedParaXModel = CGlobals::GetAssetManager()->LoadParaX("", modelpath.c_str());
					}

					if (AttachedModel != 0 && (bIsStaticMeshModel ? AttachedModel->IsValid() : AttachedParaXModel->IsValid()))
					{
						if (bIsStaticMeshModel){
							att = m_pModelCanvas->addAttachment(AttachedModel, id1, slot, sc);
						}
						else{
							att = m_pModelCanvas->addAttachment(AttachedParaXModel, id1, slot, sc);
						}

						if (att)
						{
							string replaceableTex = makeItemModel(regionpath, Skin.c_str());
							if (!replaceableTex.empty())
							{
								TextureEntity* tex = CGlobals::GetAssetManager()->LoadTexture("", replaceableTex.c_str(), TextureEntity::StaticTexture);
								att->SetReplaceableTexture(tex);
							}
							succ = true;
						}
					}
				}
			}
			if (id2 >= 0)
			{
				modelpath = makeItemModel(regionpath, Model2.c_str());
				if (!modelpath.empty())
				{
					bool bIsStaticMeshModel = IsStaticMeshAsset(modelpath);
					if (bIsStaticMeshModel)
					{
						AttachedModel = CGlobals::GetAssetManager()->LoadMesh("", modelpath.c_str());
					}
					else
					{
						AttachedParaXModel = CGlobals::GetAssetManager()->LoadParaX("", modelpath.c_str());
					}

					if (AttachedModel != 0 && (bIsStaticMeshModel ? AttachedModel->IsValid() : AttachedParaXModel->IsValid()))
					{
						if (bIsStaticMeshModel){
							att = m_pModelCanvas->addAttachment(AttachedModel, id2, slot, sc);
						}
						else{
							att = m_pModelCanvas->addAttachment(AttachedParaXModel, id2, slot, sc);
						}

						if (att)
						{
							string replaceableTex = makeItemModel(regionpath, Skin2.c_str());
							if (!replaceableTex.empty())
							{
								TextureEntity* tex = CGlobals::GetAssetManager()->LoadTexture("", replaceableTex.c_str(), TextureEntity::StaticTexture);
								att->SetReplaceableTexture(tex);
							}
							succ = true;
						}
					}
				}
			}

			if (succ) {
#ifdef MODEL_EFFECT
				// okay, see if we have any glowy effects
				if (visualid > 0) {
					try {
						ItemVisualDB::Record vis = visualdb.getById(visualid);
						for (int i = 0; i<5; i++) {
							// try all five visual slots
							int effectid = vis.getInt(ItemVisualDB::Effect1 + i);
							if (effectid == 0) continue;

							try {
								ItemVisualEffectDB::Record eff = effectdb.getById(effectid);
								const char *filename = eff.getString(ItemVisualEffectDB::Model);

								att->addChild(filename, i, -1);

							}
							catch (...) {}
						}
					}
					catch (...) {}
				}
#endif

			}
			else {
				equipment[slot] = 0; // no such model
			}
		}
	}
	else
	{
		if ((slot == CS_HEAD) && autoHair)
		{/// if there is no head attachments, enable hair
			showHair = true;
			showFacialHair = true;
		}
		if (slot == CS_HEAD)
		{
			//geosets[CSET_EARS] = 1; // show the default character ear
			showEars = true;
		}
	}
}

TextureEntity* CharModelInstance::GetReplaceableTexture(int ReplaceableTextureID)
{
	if ((ReplaceableTextureID >= 0 && ReplaceableTextureID < CParaXModel::MAX_MODEL_TEXTURES))
	{
		if (m_bIsCustomModel)
		{
			// set replaceable textures
			if (ReplaceableTextureID == 1)
				return m_textures[CHAR_TEX].get();
			else if (ReplaceableTextureID == 1)
				return m_textures[CAPE_TEX].get();
			else if (ReplaceableTextureID == 6)
				return m_textures[HAIR_TEX].get();
			else if (ReplaceableTextureID == 7)
				return m_textures[CARTOON_FACE_TEX].get();
			else if (ReplaceableTextureID == 8)
				return m_textures[FUR_TEX].get();
		}
		else
		{
			if (GetBaseModel())
			{
				CParaXModel* pModel = GetBaseModel()->GetModel();
				if (pModel != NULL)
				{
					int k = 0;
					for (int i = 0; i < CParaXModel::MAX_MODEL_TEXTURES; ++i)
					{
						int nIndex = pModel->specialTextures[i];
						if (nIndex >= 0)
						{
							if (nIndex == ReplaceableTextureID)
							{
								return m_textures[k].get();
							}
							k++;
						}
					}
				}
			}
		}

	}
	return NULL;
}

bool CharModelInstance::SetReplaceableTexture(int ReplaceableTextureID, TextureEntity* pTextureEntity)
{
	if (ReplaceableTextureID >= 0 && ReplaceableTextureID < CParaXModel::MAX_MODEL_TEXTURES)
	{
		if (GetBaseModel())
		{
			if (m_bIsCustomModel)
			{
				// set replaceable textures
				if (ReplaceableTextureID == 1)
					m_textures[CHAR_TEX] = pTextureEntity;
				else if (ReplaceableTextureID == 1)
					m_textures[CAPE_TEX] = pTextureEntity;
				else if (ReplaceableTextureID == 6)
					m_textures[HAIR_TEX] = pTextureEntity;
				else if (ReplaceableTextureID == 7)
					m_textures[CARTOON_FACE_TEX] = pTextureEntity;
				else if (ReplaceableTextureID == 8)
					m_textures[FUR_TEX] = pTextureEntity;
			}
			else
			{
				CParaXModel* pModel = GetBaseModel()->GetModel();
				if (pModel != NULL)
				{
					int k = 0;
					for (int i = 0; i < CParaXModel::MAX_MODEL_TEXTURES; ++i)
					{
						int nIndex = pModel->specialTextures[i];
						if (nIndex >= 0)
						{
							if (nIndex == ReplaceableTextureID)
							{
								m_textures[k] = pTextureEntity;
								return true;
							}
							k++;
						}
					}
				}
				else
				{
					// model is NULL, this is mostly because model is asynchronously  loaded. we shall assume it is 0. 
					// this is correct, if the model has only one replaceable texture, however it can be wrong. 
					// TODO: is there a better way to preload. 
					m_textures[0] = pTextureEntity;
				}
			}
		}
	}
	return false;
}

void CharModelInstance::adjustHairVisibility()
{
}

std::string CharModelInstance::makeItemTexture(int region, const char *name)
{
	// just return an empty filename
	if (name == 0 || name[0] == '\0')
		return "";

	std::string sName = name;
	std::string fn;
	if (sName.find_last_of('/') == std::string::npos)
	{
		// if it does not contain a parent path, region is used
		std::string fullname = CCharCustomizeSysSetting::GetRegionPath(region);
		fullname += sName;
		if (CParaFile::GetFileExtension(sName) == "")
		{
			int nLen = (int)sName.length();
			if (nLen > 4)
			{
				char sex = sName[nLen - 1];
				if (sex >= 'a' && sex <= 'z')
					sex += ('A' - 'a');

				if (sName[nLen - 2] == '_' && (sex == 'U' || sex == 'F' || sex == 'M'))
				{
					fullname += ".dds";
				}
				else
				{
					if (CParaFile::DoesFileExist((fullname + ".dds").c_str(), true))
					{
						fullname += ".dds";
					}
					else
					{
						fullname += "_";
						fullname += gender ? "F" : "M";
						fullname += ".dds";
						if (CParaFile::DoesFileExist(fullname.c_str(), true) == false)
						{
							// try unisex version
							fullname[fullname.length() - 5] = 'U';
						}
					}
					return fullname;
				}
			}
			else
			{
				fullname += ".dds";
			}

		}
		return fullname;
	}
	else
	{
		if (CParaFile::GetFileExtension(sName) == "")
			sName += ".dds";
		return sName;
	}
}

std::string CharModelInstance::makeItemModel(int region, const char *name)
{
	// just return an empty filename
	if (name == 0 || name[0] == '\0')
		return "";

	std::string sName = name;
	std::string fn;
	if (sName.find_last_of('/') == std::string::npos)
	{
		// if it does not contain a parent path, region is used
		std::string fullname = CCharCustomizeSysSetting::GetRegionPath(region);
		fullname += sName;
		if (CParaFile::GetFileExtension(sName) == "")
		{
			int nLen = (int)sName.length();
			if (nLen > 4)
			{
				char sex = sName[nLen - 1];
				if (sex >= 'a' && sex <= 'z')
					sex += ('A' - 'a');

				if (sName[nLen - 2] == '_' && (sex == 'U' || sex == 'F' || sex == 'M'))
				{
					fullname += ".x";
				}
				else
				{
					if (CParaFile::DoesFileExist((fullname + ".x").c_str(), true))
					{
						fullname += ".x";
					}
					else
					{
						fullname += "_";
						fullname += gender ? "F" : "M";
						fullname += ".x";
						if (CParaFile::DoesFileExist(fullname.c_str(), true) == false)
						{
							// try unisex version
							fullname[fullname.length() - 3] = 'U';
						}
					}
					return fullname;
				}
			}
			else
			{
				fullname += ".x";
			}
		}
		return fullname;
	}
	else
	{
		if (CParaFile::GetFileExtension(sName) == "")
			sName += ".x";
		return sName;
	}
}
std::string CharModelInstance::makeSkinTexture(const char *texfn, const char *skin)
{
	std::string res = texfn;
	size_t i = res.find_last_of('/');
	res = res.substr(0, i + 1);
	res.append(skin);
	res.append(".dds");
	return res;
}

void CharModelInstance::reset()
{
	m_fUpperBodyTurningAngle = 0;
	m_fUpperBodyUpDownAngle = 0;
	// display info

	skinColor = 0;
	m_skinIndex = -1;

	faceType = 0;
	hairColor = 0;
	hairStyle = 0;
	facialHair = 0;

	showUnderwear = true;
	showFacialHair = false;
	showHair = true;
	showEars = true;
	autoHair = true;
	showAriesPantByItem = true;
	showAriesHandByItem = true;
	showAriesFootByItem = true;
	showAriesGlassByItem = true;

	for (int i = 0; i < NUM_CHAR_SLOTS; i++) {
		equipment[i] = 0;
	}

	race = 1;
	gender = 1;
	nSetID = -1;

	SAFE_DELETE(m_cartoonFace);
	SAFE_DELETE(m_bodyTexture);
	// set to some default set. 
	memset(geosets, 0, sizeof(geosets));

	m_bNeedRefreshModel = true;
	m_bNeedRefreshEquipment = true;
	m_bNeedComposeBodyTexture = false;
}

bool CharModelInstance::CanItemInSlot(int type, int slot)
{
	if (type == IT_ALL) return true;
	switch (slot) {
	case CS_HEAD:		return (type == IT_HEAD);
	case CS_NECK:		return (type == IT_NECK);
	case CS_SHOULDER:	return (type == IT_SHOULDER);
	case CS_SHIRT:		return (type == IT_SHIRT);
	case CS_CHEST:		return (type == IT_CHEST || type == IT_ROBE);
	case CS_BELT:		return (type == IT_BELT);
	case CS_PANTS:		return (type == IT_PANTS);
	case CS_BOOTS:		return (type == IT_BOOTS);
	case CS_BRACERS:	return (type == IT_BRACERS);
	case CS_GLOVES:		return (type == IT_GLOVES);
	case CS_HAND_RIGHT:	return (type == IT_1HANDED || type == IT_GUN || type == IT_THROWN || type == IT_2HANDED || type == IT_CLAW || type == IT_DAGGER);
	case CS_HAND_LEFT:	return (type == IT_1HANDED || type == IT_BOW || type == IT_2HANDED || type == IT_SHIELD || type == IT_CLAW || type == IT_DAGGER);
	case CS_CAPE:		return (type == IT_CAPE);
	case CS_TABARD:		return (type == IT_TABARD);
	case CS_FACE_ADDON: return (type == IT_MASK);
	case CS_WINGS:		return (type == IT_WINGS);
	case CS_ARIES_CHAR_SHIRT:	return (type == IT_ARIES_CHAR_SHIRT);
	case CS_ARIES_CHAR_PANT:	return (type == IT_ARIES_CHAR_PANT);
	case CS_ARIES_CHAR_HAND:	return (type == IT_ARIES_CHAR_HAND);
	case CS_ARIES_CHAR_FOOT:	return (type == IT_ARIES_CHAR_FOOT);
	case CS_ARIES_CHAR_GLASS:	return (type == IT_ARIES_CHAR_GLASS);
	case CS_ARIES_CHAR_WING:	return (type == IT_ARIES_CHAR_WING);
	case CS_ARIES_PET_HEAD:		return (type == IT_ARIES_PET_HEAD);
	case CS_ARIES_PET_BODY:		return (type == IT_ARIES_PET_BODY);
	case CS_ARIES_PET_TAIL:		return (type == IT_ARIES_PET_TAIL);
	case CS_ARIES_PET_WING:		return (type == IT_ARIES_PET_WING);
	case CS_ARIES_CHAR_BACK:		return (type == IT_ARIES_CHAR_BACK);
	case CS_ARIES_CHAR_GROUND_EFFECT:		return (type == IT_ARIES_CHAR_GROUND_EFFECT);
	case CS_ARIES_CHAR_SHIRT_TEEN:	return (type == IT_ARIES_CHAR_SHIRT_TEEN);
	case CS_ARIES_CHAR_GEM_BUFF_EFFECT:		return (type == IT_ARIES_CHAR_GEM_BUFF_EFFECT);
	case CS_ARIES_CHAR_RING_BUFF_EFFECT:		return (type == IT_ARIES_CHAR_GEM_BUFF_EFFECT);
	}
	return false;
}

void CharModelInstance::CastEffect(int nEffectID, int nAttachmentID, int nSlotID)
{
	if (nEffectID < 0)
	{
		m_pModelCanvas->deleteSlot(nSlotID);
	}
	else
	{
		// the particle system is rendered with the character canvas.
		ParaXEntity* EffectModel = CGlobals::GetAssetManager()->LoadParaXByID(nEffectID);
		if (EffectModel)
		{
			// delete slot and attach this new one
			m_pModelCanvas->deleteSlot(nSlotID);
			m_pModelCanvas->addAttachment(EffectModel, nAttachmentID, nSlotID);
		}
	}
}

void CharModelInstance::AddAttachment(ParaXEntity* pModelEntity, int nAttachmentID, int nSlotID, float fScaling, TextureEntity* pReplaceableTexture)
{
	if (pModelEntity == 0)
	{
		m_pModelCanvas->deleteSlot(nSlotID);
	}
	else
	{
		// the particle system is rendered with the character canvas.
		// delete slot and attach this new one
		m_pModelCanvas->deleteSlot(nSlotID);
		CanvasAttachment* pAtt = m_pModelCanvas->addAttachment(pModelEntity, nAttachmentID, nSlotID, fScaling);
		if (pReplaceableTexture && pAtt)
		{
			pAtt->SetReplaceableTexture(pReplaceableTexture);
		}
	}
}

IAttributeFields * CharModelInstance::GetAttachmentAttObj(int nAttachmentID)
{
	CanvasAttachment* pAtt = m_pModelCanvas->GetChild(nAttachmentID);
	if (pAtt)
	{
		return pAtt->GetAttributeObject();
	}
	return NULL;
}


void CharModelInstance::AddAttachment(MeshEntity* pModelEntity, int nAttachmentID, int nSlotID, float fScaling, TextureEntity* pReplaceableTexture)
{
	if (pModelEntity == 0)
	{
		m_pModelCanvas->deleteSlot(nSlotID);
	}
	else
	{
		// the particle system is rendered with the character canvas.
		// delete slot and attach this new one
		m_pModelCanvas->deleteSlot(nSlotID);
		CanvasAttachment* pAtt = m_pModelCanvas->addAttachment(pModelEntity, nAttachmentID, nSlotID, fScaling);

		if (pReplaceableTexture && pAtt)
		{
			pAtt->SetReplaceableTexture(pReplaceableTexture);
		}
	}
}

CartoonFace* CharModelInstance::GetCartoonFace(bool bCreateIfNotExist)
{
	if (m_cartoonFace)
		return m_cartoonFace;
	else
	{
		if (bCreateIfNotExist)
		{
			m_cartoonFace = new CartoonFace();
			return m_cartoonFace;
		}
	}
	return NULL;
}

bool CharModelInstance::IsSupportCartoonFace()
{
	if (GetBaseModel() == NULL)
		return false;
	CParaXModel* pModel = GetBaseModel()->GetModel();
	if (pModel == NULL)
		return false;
	// we will assume that the model support cartoon face, if it contains the 7th replaceable texture.
	return (pModel->useReplaceTextures[7]);
}

void CharModelInstance::ProcessPendingTextures(int nMaxNumber)
{
	// OBSOLETED. This does nothing, because we use CAyncLoader for texture composition in another thread. 
}

bool CharModelInstance::HasPendingBodyTexture(CharModelInstance* pendingInstance)
{
	std::list <CharModelInstance*>::const_iterator itCur, itEnd = g_listPendingBodyTextures.end();
	for (itCur = g_listPendingBodyTextures.begin(); itCur != itEnd; ++itCur)
	{
		if ((*itCur) == pendingInstance)
		{
			// if a pending one already exist, just return true. 
			return true;
		}
	}
	return false;
}

bool CharModelInstance::RemovePendingBodyTexture(CharModelInstance* pendingInstance)
{
	bool bFound = false;
	std::list <CharModelInstance*>::iterator itCur, itEnd = CharModelInstance::g_listPendingBodyTextures.end();
	for (itCur = CharModelInstance::g_listPendingBodyTextures.begin(); itCur != itEnd; ++itCur)
	{
		if ((*itCur) == pendingInstance)
		{
			// find if the char model instance is still in the scene. 
			CharModelInstance::g_listPendingBodyTextures.erase(itCur);
			bFound = true;
			break;
		}
	}
	return bFound;
}

void CharModelInstance::AddPendingBodyTexture(CharModelInstance* pendingInstance)
{
	if (pendingInstance == 0 || pendingInstance->m_bodyTexture == 0)
		return;

	// add to pending texture queue. 
	bool bFound = false;
	std::list <CharModelInstance*>::const_iterator itCur, itEnd = g_listPendingBodyTextures.end();
	for (itCur = g_listPendingBodyTextures.begin(); itCur != itEnd; ++itCur)
	{
		if ((*itCur) == pendingInstance)
		{
			// if a pending one already exist, just return. 
			bFound = true;
			break;
		}
	}
	// add to the end of the pending list
	if (!bFound)
		g_listPendingBodyTextures.push_back(pendingInstance);

	// Add to the async loader queue. 
	CAsyncLoader* pAsyncLoader = &(CAsyncLoader::GetSingleton());
	if (pAsyncLoader)
	{
		CCCSSkinLoader* pLoader = new CCCSSkinLoader(pendingInstance, *(pendingInstance->m_bodyTexture), pendingInstance->m_bodyTexture->GetCacheFileName().c_str());
		CCCSSkinProcessor* pProcessor = new CCCSSkinProcessor(pendingInstance, pendingInstance->m_bodyTexture->GetCacheFileName().c_str());
		pAsyncLoader->AddWorkItem(pLoader, pProcessor, NULL, NULL);
	}

}

bool CharModelInstance::HasPendingCartoonFace(CharModelInstance* pendingInstance)
{
	std::list <CharModelInstance*>::const_iterator itCur, itEnd = g_listPendingCartoonFaces.end();
	for (itCur = g_listPendingCartoonFaces.begin(); itCur != itEnd; ++itCur)
	{
		if ((*itCur) == pendingInstance)
		{
			// if a pending one already exist, just return. 
			return true;
		}
	}
	return false;
}

bool CharModelInstance::RemovePendingCartoonFace(CharModelInstance* pendingInstance)
{
	bool bFound = false;
	std::list <CharModelInstance*>::iterator itCur, itEnd = CharModelInstance::g_listPendingCartoonFaces.end();
	for (itCur = CharModelInstance::g_listPendingCartoonFaces.begin(); itCur != itEnd; ++itCur)
	{
		if ((*itCur) == pendingInstance)
		{
			// find if the char model instance is still in the scene. 
			CharModelInstance::g_listPendingCartoonFaces.erase(itCur);
			bFound = true;
			break;
		}
	}
	return bFound;
}

void CharModelInstance::AddPendingCartoonFace(CharModelInstance* pendingInstance)
{
	if (pendingInstance == 0 || pendingInstance->m_cartoonFace == 0)
		return;

	bool bFound = false;
	std::list <CharModelInstance*>::const_iterator itCur, itEnd = g_listPendingCartoonFaces.end();
	for (itCur = g_listPendingCartoonFaces.begin(); itCur != itEnd; ++itCur)
	{
		if ((*itCur) == pendingInstance)
		{
			// if a pending one already exist, just return. 
			bFound = true;
			break;
		}
	}
	// add to the end of the pending list
	if (!bFound)
		g_listPendingCartoonFaces.push_back(pendingInstance);

	// Add to the async loader queue. 
	CAsyncLoader* pAsyncLoader = &(CAsyncLoader::GetSingleton());
	if (pAsyncLoader)
	{
		CCCSFaceLoader* pLoader = new CCCSFaceLoader(pendingInstance, *(pendingInstance->m_cartoonFace), pendingInstance->m_cartoonFace->GetCacheFileName().c_str());
		CCCSFaceProcessor* pProcessor = new CCCSFaceProcessor(pendingInstance, pendingInstance->m_cartoonFace->GetCacheFileName().c_str());

		pAsyncLoader->AddWorkItem(pLoader, pProcessor, NULL, NULL);
	}
}

DWORD CharModelInstance::GetSkinColorMask()
{
	return m_dwSkinColorMask;
}

void CharModelInstance::SetSkinColorMask(DWORD dwColor)
{
	if (m_dwSkinColorMask != dwColor)
	{
		m_dwSkinColorMask = dwColor;
		if (m_cartoonFace)
		{
			m_cartoonFace->SetModified(true);
		}
		SetModifiedModel();
	}
}


void CharModelInstance::SetCharacterSlot(int nSlotID, int nItemID)
{
	if (nSlotID >= 0 && nSlotID < NUM_CHAR_SLOTS && nItemID >= 0)
	{
		if (equipment[nSlotID] != nItemID)
		{
			equipment[nSlotID] = nItemID;

			if (m_bIsCustomModel)
			{
				SetModifiedEquipment();
				SetModifiedModel();
			}
			else
			{
				if (HasCustomGeosets(NULL))
				{
					geosets[nSlotID] = nItemID;
					SetModifiedModel();
				}
			}
		}
	}
}

int CharModelInstance::GetCharacterSlot(int nSlotID)
{
	if (nSlotID >= 0 && nSlotID < NUM_CHAR_SLOTS)
		return equipment[nSlotID];
	else
		return -1;
}

bool CharModelInstance::IsCustomModel()
{
	return m_bIsCustomModel;
}

void CharModelInstance::SetBodyParams(int skinColor_, int faceType_, int hairColor_, int hairStyle_, int facialHair_)
{
	if (skinColor_ >= 0 && skinColor != skinColor_)
	{
		skinColor = skinColor_;
		SetModifiedModel();
	}
	if (faceType_ >= 0 && faceType != faceType_)
	{
		faceType = faceType_;
		SetModifiedModel();
	}
	if (hairColor_ >= 0 && hairColor != hairColor_)
	{
		hairColor = hairColor_;
		SetModifiedModel();
	}
	if (hairStyle_ >= 0 && hairStyle != hairStyle_)
	{
		hairStyle = hairStyle_;
		SetModifiedModel();
	}
	if (facialHair_ >= 0 && facialHair != facialHair_)
	{
		facialHair = facialHair_;
		SetModifiedModel();
	}
}

int CharModelInstance::GetBodyParams(int type)
{
	switch (type)
	{
	case BP_SKINCOLOR:
		return skinColor;
		break;
	case BP_FACETYPE:
		return faceType;
		break;
	case BP_HAIRCOLOR:
		return hairColor;
		break;
	case BP_HAIRSTYLE:
		return hairStyle;
		break;
	case BP_FACIALHAIR:
		return facialHair;
		break;
	default:
		return 0;
		break;
	}
}

IAttributeFields* CharModelInstance::GetChildAttributeObject(const std::string& sName)
{
	return m_pModelCanvas.get();
}

IAttributeFields* CharModelInstance::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0 && nRowIndex == 0)
		return m_pModelCanvas.get();
	return 0;
}

int CharModelInstance::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
		return m_pModelCanvas ? 1 : 0;
	return 0;
}

int CharModelInstance::GetChildAttributeColumnCount()
{
	return 1;
}

int CharModelInstance::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);
	return S_OK;
}
