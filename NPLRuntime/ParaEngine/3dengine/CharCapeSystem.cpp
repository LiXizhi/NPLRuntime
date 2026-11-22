//-----------------------------------------------------------------------------
// Class:	CharCapeSystem
// Desc:	Sky-style DIY cape physics + legacy Geoset/Bone cape takeover
//			with distance-budgeted static attachment fallback.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "CharCapeSystem.h"
#include "CustomCharCommon.h"
#include "CustomCharModelInstance.h"
#include "ParaXModelCanvas.h"
#include "ParaXEntity.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/ParaXBone.h"
#include "ParaWorldAsset.h"
#include "ParaImage.h"
#include "SceneObject.h"
#include "SceneState.h"
#include "BipedObject.h"
#include "EffectManager.h"
#include "effect_file.h"
#include "json/json.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <sstream>

using namespace ParaEngine;

namespace
{
	inline CapeVec3 ToCapeVec(const Vector3& v)
	{
		return { v.x, v.y, v.z };
	}

	inline Vector3 ToParaVec(const CapeVec3& v)
	{
		return Vector3(v.x, v.y, v.z);
	}

	std::string ToLowerCopy(const std::string& s)
	{
		std::string out = s;
		for (char& ch : out)
			ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
		return out;
	}

	DWORD ParseColorValue(const Json::Value& val, DWORD fallback)
	{
		if (val.isNull())
			return fallback;
		if (val.isUInt())
			return static_cast<DWORD>(val.asUInt());
		if (val.isInt())
			return static_cast<DWORD>(val.asInt());
		if (val.isDouble())
			return static_cast<DWORD>(static_cast<uint64_t>(val.asDouble()));
		if (val.isString())
		{
			std::string s = val.asString();
			if (s.empty())
				return fallback;
			if (s[0] == '#')
				s.erase(0, 1);
			else if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
				s.erase(0, 2);

			char* endPtr = nullptr;
			const unsigned long parsed = std::strtoul(s.c_str(), &endPtr, 16);
			if (endPtr == s.c_str())
				return fallback;
			if (s.size() <= 6)
				return 0xff000000u | static_cast<DWORD>(parsed & 0x00ffffffu);
			return static_cast<DWORD>(parsed & 0xffffffffu);
		}
		return fallback;
	}

	std::string FormatColorHex(DWORD argb)
	{
		char buf[16];
		std::snprintf(buf, sizeof(buf), "#%08x", static_cast<unsigned int>(argb));
		return std::string(buf);
	}

	float NormalizeAnglePi(float a)
	{
		static constexpr float kPi = 3.14159265358979323846f;
		static constexpr float kTwoPi = 6.28318530717958647692f;
		while (a > kPi) a -= kTwoPi;
		while (a < -kPi) a += kTwoPi;
		return a;
	}

	void BuildUncompressedBmp32(int width, int height, const std::vector<uint8_t>& rgba, std::vector<uint8_t>& outBmp)
	{
		const uint32_t pixelBytes = static_cast<uint32_t>(width * height * 4);
		const uint32_t fileSize = 54u + pixelBytes;
		outBmp.assign(fileSize, 0);
		auto write16 = [&](size_t off, uint16_t v) {
			outBmp[off + 0] = static_cast<uint8_t>(v & 0xffu);
			outBmp[off + 1] = static_cast<uint8_t>((v >> 8) & 0xffu);
		};
		auto write32 = [&](size_t off, uint32_t v) {
			outBmp[off + 0] = static_cast<uint8_t>(v & 0xffu);
			outBmp[off + 1] = static_cast<uint8_t>((v >> 8) & 0xffu);
			outBmp[off + 2] = static_cast<uint8_t>((v >> 16) & 0xffu);
			outBmp[off + 3] = static_cast<uint8_t>((v >> 24) & 0xffu);
		};
		outBmp[0] = 'B';
		outBmp[1] = 'M';
		write32(2, fileSize);
		write32(10, 54u);
		write32(14, 40u);
		write32(18, static_cast<uint32_t>(width));
		// Negative height = top-down DIB so row order matches UV v in [0..1]
		write32(22, static_cast<uint32_t>(-height));
		write16(26, 1u);
		write16(28, 32u);
		write32(34, pixelBytes);

		uint8_t* dst = outBmp.data() + 54;
		for (size_t i = 0; i + 3 < rgba.size(); i += 4)
		{
			dst[i + 0] = rgba[i + 2]; // B
			dst[i + 1] = rgba[i + 1]; // G
			dst[i + 2] = rgba[i + 0]; // R
			dst[i + 3] = rgba[i + 3]; // A
		}
	}

	CapeVec3 ComputeSkinnedModelVertex(const CParaXModel* pModel, uint16_t vertexIndex)
	{
		if (!pModel || !pModel->m_origVertices || vertexIndex >= pModel->GetObjectNum().nVertices)
			return { 0.0f, 0.0f, 0.0f };
		const ModelVertex& ov = pModel->m_origVertices[vertexIndex];
		const uint32_t nBones = pModel->GetObjectNum().nBones;
		if (!pModel->bones || nBones == 0 || ov.weights[0] == 0)
			return ToCapeVec(ov.pos);

		Vector3 accum(0.0f, 0.0f, 0.0f);
		float weightSum = 0.0f;
		for (int b = 0; b < 4 && ov.weights[b] > 0; ++b)
		{
			const uint8_t boneIdx = ov.bones[b];
			if (boneIdx < nBones)
			{
				const float w = static_cast<float>(ov.weights[b]) * (1.0f / 255.0f);
				accum += (pModel->bones[boneIdx].mat * ov.pos) * w;
				weightSum += w;
			}
		}
		return (weightSum > 1e-4f) ? ToCapeVec(accum) : ToCapeVec(ov.pos);
	}

	TextureEntity* GetOrCreateSynthesizedCapeTexture(
		const std::string& cacheKey,
		DWORD baseColor,
		DWORD gradColor,
		DWORD trimColor,
		float trimWidth,
		bool drawStar,
		DWORD starColor,
		float starScale)
	{
		auto* assetMgr = CGlobals::GetAssetManager();
		if (!assetMgr || !CGlobals::GetRenderDevice())
			return nullptr;

		// Use unique cacheKey as both identifier and asset map key; check IsInitialized() directly without triggering GetTexture()
		TextureEntity* pTex = assetMgr->LoadTexture(cacheKey, cacheKey, TextureEntity::StaticTexture);
		if (!pTex)
			return nullptr;

		if (!pTex->IsInitialized())
		{
			pTex->SetState(AssetEntity::ASSET_STATE_CACHE);
			pTex->SetLocalFileName("");
			std::vector<uint8_t> rgba;
			SynthesizeCapePatternRGBA(
				128, 128,
				baseColor, gradColor, trimColor, trimWidth,
				drawStar, starColor, starScale,
				rgba);

			ParaImage img;
			bool uploaded = false;
			if (img.initWithRawData(rgba.data(), rgba.size(), 128, 128, 32, false))
			{
				uploaded = pTex->LoadFromImage(&img, 1, PixelFormat::A8R8G8B8);
			}
			if (!uploaded)
			{
				std::vector<uint8_t> bmpBytes;
				BuildUncompressedBmp32(128, 128, rgba, bmpBytes);
				pTex->LoadFromMemory(reinterpret_cast<const char*>(bmpBytes.data()), bmpBytes.size(), 1, PixelFormat::A8R8G8B8);
			}
		}
		return pTex;
	}
}

bool CapeConfig::ParseFromJson(const std::string& jsonStr)
{
	if (jsonStr.empty())
	{
		*this = CapeConfig();
		mode = CapeMode::None;
		return true;
	}

	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(jsonStr, root, false) || !root.isObject())
		return false;

	CapeConfig cfg;
	cfg.mode = CapeMode::ProceduralMesh;

	if (root.isMember("mode"))
	{
		if (root["mode"].isInt())
		{
			const int m = root["mode"].asInt();
			cfg.mode = (m == 2) ? CapeMode::GeosetTakeover : ((m == 1) ? CapeMode::ProceduralMesh : CapeMode::None);
		}
		else if (root["mode"].isString())
		{
			const std::string m = ToLowerCopy(root["mode"].asString());
			if (m == "none" || m == "off" || m == "disabled")
				cfg.mode = CapeMode::None;
			else if (m == "geoset" || m == "geoset_takeover" || m == "bone")
				cfg.mode = CapeMode::GeosetTakeover;
			else
				cfg.mode = CapeMode::ProceduralMesh;
		}
	}

	if (root.isMember("collarStyle"))
	{
		if (root["collarStyle"].isInt())
		{
			const int cs = root["collarStyle"].asInt();
			cfg.style.collarStyle = (cs == 1) ? CapeCollarStyle::ClassicBack
				: ((cs == 2) ? CapeCollarStyle::Scarf : CapeCollarStyle::WrapPoncho);
		}
		else if (root["collarStyle"].isString())
		{
			const std::string cs = ToLowerCopy(root["collarStyle"].asString());
			if (cs == "classic" || cs == "classic_back" || cs == "back")
				cfg.style.collarStyle = CapeCollarStyle::ClassicBack;
			else if (cs == "scarf" || cs == "ribbon")
				cfg.style.collarStyle = CapeCollarStyle::Scarf;
			else
				cfg.style.collarStyle = CapeCollarStyle::WrapPoncho;
		}
	}

	if (root.isMember("hemStyle"))
	{
		if (root["hemStyle"].isInt())
		{
			const int hs = (std::max)(0, (std::min)(5, root["hemStyle"].asInt()));
			cfg.style.hemStyle = static_cast<CapeHemStyle>(hs);
		}
		else if (root["hemStyle"].isString())
		{
			const std::string hs = ToLowerCopy(root["hemStyle"].asString());
			if (hs == "scalloped" || hs == "petal" || hs == "wave")
				cfg.style.hemStyle = CapeHemStyle::Scalloped;
			else if (hs == "star" || hs == "star_pointed" || hs == "starred")
				cfg.style.hemStyle = CapeHemStyle::StarPointed;
			else if (hs == "swallowtail" || hs == "split")
				cfg.style.hemStyle = CapeHemStyle::Swallowtail;
			else if (hs == "pointed" || hs == "diamond")
				cfg.style.hemStyle = CapeHemStyle::Pointed;
			else if (hs == "asymmetric" || hs == "slant")
				cfg.style.hemStyle = CapeHemStyle::Asymmetric;
			else
				cfg.style.hemStyle = CapeHemStyle::Rounded;
		}
	}

	auto readFloat = [&](const char* key, float& target, float minVal, float maxVal)
	{
		if (root.isMember(key) && root[key].isNumeric())
		{
			const float v = static_cast<float>(root[key].asDouble());
			target = (std::max)(minVal, (std::min)(maxVal, v));
		}
	};

	if (root["gridCols"].isNumeric()) cfg.style.gridCols = (std::max)(4, (std::min)(15, root["gridCols"].asInt()));
	if (root["gridRows"].isNumeric()) cfg.style.gridRows = (std::max)(4, (std::min)(12, root["gridRows"].asInt()));
	readFloat("length", cfg.style.length, 0.15f, 2.5f);
	readFloat("topWidth", cfg.style.topWidth, 0.08f, 1.5f);
	readFloat("bottomWidth", cfg.style.bottomWidth, 0.10f, 2.5f);
	readFloat("wrapAngleDeg", cfg.style.wrapAngleDeg, 60.0f, 280.0f);
	readFloat("shoulderDrop", cfg.style.shoulderDrop, -0.2f, 0.4f);
	readFloat("backOffset", cfg.style.backOffset, -0.2f, 0.5f);
	readFloat("bellCurvature", cfg.style.bellCurvature, 0.0f, 0.5f);
	if (root["hemCount"].isNumeric()) cfg.style.hemCount = (std::max)(2, (std::min)(16, root["hemCount"].asInt()));
	readFloat("hemAmplitude", cfg.style.hemAmplitude, 0.0f, 0.45f);
	readFloat("splitDepth", cfg.style.splitDepth, 0.0f, 0.75f);

	if (root["attachOffset"].isArray() && root["attachOffset"].size() >= 3u)
	{
		cfg.style.attachOffset = {
			static_cast<float>(root["attachOffset"][0u].asDouble()),
			static_cast<float>(root["attachOffset"][1u].asDouble()),
			static_cast<float>(root["attachOffset"][2u].asDouble())
		};
	}

	if (root["autoScale"].isBool()) cfg.autoScale = root["autoScale"].asBool();
	if (root["replaceBackGeoset"].isBool()) cfg.replaceBackGeoset = root["replaceBackGeoset"].asBool();
	if (root["doubleSided"].isBool()) cfg.doubleSided = root["doubleSided"].asBool();
	if (root["drawStarEmblem"].isBool()) cfg.drawStarEmblem = root["drawStarEmblem"].asBool();

	cfg.outerColor = ParseColorValue(root["outerColor"], cfg.outerColor);
	cfg.innerColor = ParseColorValue(root["innerColor"], cfg.innerColor);
	cfg.gradientColor = ParseColorValue(root["gradientColor"], cfg.gradientColor);
	cfg.trimColor = ParseColorValue(root["trimColor"], cfg.trimColor);
	cfg.emissiveColor = ParseColorValue(root["emissiveColor"], cfg.emissiveColor);

	readFloat("trimWidth", cfg.trimWidth, 0.0f, 0.3f);
	readFloat("emissiveIntensity", cfg.emissiveIntensity, 0.0f, 3.0f);
	readFloat("emblemScale", cfg.emblemScale, 0.05f, 0.5f);

	if (root["outerTexture"].isString()) cfg.outerTexture = root["outerTexture"].asString();
	if (root["innerTexture"].isString()) cfg.innerTexture = root["innerTexture"].asString();
	if (root["emblemTexture"].isString()) cfg.emblemTexture = root["emblemTexture"].asString();

	if (root["geosetId"].isNumeric()) cfg.targetGeosetId = root["geosetId"].asInt();
	if (root["targetGeosetId"].isNumeric()) cfg.targetGeosetId = root["targetGeosetId"].asInt();
	if (root["geosetCategory"].isNumeric()) cfg.targetGeosetCategory = root["geosetCategory"].asInt();
	if (root["targetGeosetCategory"].isNumeric()) cfg.targetGeosetCategory = root["targetGeosetCategory"].asInt();
	if (root["attachmentId"].isNumeric()) cfg.targetAttachmentId = root["attachmentId"].asInt();
	if (root["bonePrefix"].isString()) cfg.bonePrefix = root["bonePrefix"].asString();
	if (root["preferBoneDrive"].isBool()) cfg.preferBoneDrive = root["preferBoneDrive"].asBool();

	if (root["boneIndices"].isArray())
	{
		for (Json::UInt i = 0; i < root["boneIndices"].size(); ++i)
		{
			if (root["boneIndices"][i].isInt())
				cfg.boneIndices.push_back(root["boneIndices"][i].asInt());
		}
	}

	readFloat("gravity", cfg.tuning.gravity, 0.0f, 30.0f);
	readFloat("damping", cfg.tuning.damping, 0.005f, 0.35f);
	readFloat("bendStiffness", cfg.tuning.bendStiffness, 0.05f, 1.0f);
	readFloat("stretchLimit", cfg.tuning.stretchLimit, 1.01f, 1.35f);
	readFloat("windResponse", cfg.tuning.windResponse, 0.0f, 4.0f);
	readFloat("liftFactor", cfg.tuning.liftFactor, 0.0f, 3.0f);

	*this = std::move(cfg);
	return true;
}

std::string CapeConfig::ToJson() const
{
	if (mode == CapeMode::None)
		return "";

	Json::Value root(Json::objectValue);
	root["mode"] = (mode == CapeMode::GeosetTakeover) ? "geoset" : "procedural";
	if (mode == CapeMode::ProceduralMesh)
	{
		const char* collarNames[] = { "wrap_poncho", "classic_back", "scarf" };
		const char* hemNames[] = { "rounded", "scalloped", "star", "swallowtail", "pointed", "asymmetric" };
		root["collarStyle"] = collarNames[static_cast<int>(style.collarStyle) % 3];
		root["hemStyle"] = hemNames[static_cast<int>(style.hemStyle) % 6];
		root["gridCols"] = style.gridCols;
		root["gridRows"] = style.gridRows;
		root["length"] = style.length;
		root["topWidth"] = style.topWidth;
		root["bottomWidth"] = style.bottomWidth;
		root["wrapAngleDeg"] = style.wrapAngleDeg;
		root["shoulderDrop"] = style.shoulderDrop;
		root["backOffset"] = style.backOffset;
		root["bellCurvature"] = style.bellCurvature;
		root["hemCount"] = style.hemCount;
		root["hemAmplitude"] = style.hemAmplitude;
		root["splitDepth"] = style.splitDepth;
		root["autoScale"] = autoScale;
		root["doubleSided"] = doubleSided;
		root["drawStarEmblem"] = drawStarEmblem;
		root["outerColor"] = FormatColorHex(outerColor);
		root["innerColor"] = FormatColorHex(innerColor);
		if ((gradientColor >> 24) != 0)
			root["gradientColor"] = FormatColorHex(gradientColor);
		root["trimColor"] = FormatColorHex(trimColor);
		root["trimWidth"] = trimWidth;
		root["emissiveColor"] = FormatColorHex(emissiveColor);
		root["emissiveIntensity"] = emissiveIntensity;
		root["emblemScale"] = emblemScale;
		if (!outerTexture.empty()) root["outerTexture"] = outerTexture;
		if (!innerTexture.empty()) root["innerTexture"] = innerTexture;
		if (!emblemTexture.empty()) root["emblemTexture"] = emblemTexture;
	}
	else
	{
		if (targetGeosetId > 0) root["geosetId"] = targetGeosetId;
		if (targetGeosetCategory >= 0) root["geosetCategory"] = targetGeosetCategory;
		if (targetAttachmentId >= 0) root["attachmentId"] = targetAttachmentId;
		if (!bonePrefix.empty()) root["bonePrefix"] = bonePrefix;
		root["preferBoneDrive"] = preferBoneDrive;
	}
	root["bendStiffness"] = tuning.bendStiffness;
	root["windResponse"] = tuning.windResponse;
	root["liftFactor"] = tuning.liftFactor;

	Json::FastWriter writer;
	std::string out = writer.write(root);
	while (!out.empty() && (out.back() == '\n' || out.back() == '\r'))
		out.pop_back();
	return out;
}

CharCapeInstance::CharCapeInstance() = default;
CharCapeInstance::~CharCapeInstance() = default;

void CharCapeInstance::InvalidateGeometry()
{
	m_bGeometryBuilt = false;
	m_pBuiltForModel = nullptr;
	m_boundGeosetId = 0;
	m_bUseBoneChainDrive = false;
	m_boneLinks.clear();
	m_proxyTriVertices.clear();
	m_proxyPasses.clear();
	m_cachedStaticOuterTris.clear();
	m_cachedStaticInnerTris.clear();
	m_dynamicOuterTris.clear();
	m_dynamicInnerTris.clear();
	m_bHasPrevMotion = false;
}

bool CharCapeInstance::SetConfigJson(const std::string& jsonStr)
{
	CapeConfig parsed;
	if (!parsed.ParseFromJson(jsonStr))
		return false;

	m_config = std::move(parsed);
	m_rawJson = (m_config.mode == CapeMode::None) ? std::string() : m_config.ToJson();
	m_bAutoManaged = false;
	m_bTexturesDirty = true;
	InvalidateGeometry();
	if (m_config.mode == CapeMode::None)
	{
		m_bTargetActive = false;
		m_fSimBlend = 0.0f;
	}
	return true;
}

std::string CharCapeInstance::GetConfigJson() const
{
	return m_config.ToJson();
}

void CharCapeInstance::SetAutoGeosetTakeover(int geosetCategory, int geosetItemValue)
{
	const int desiredId = geosetCategory * 100 + geosetItemValue;
	if (m_bAutoManaged && m_config.mode == CapeMode::GeosetTakeover && m_config.targetGeosetId == desiredId)
		return;

	m_config = CapeConfig();
	m_config.mode = CapeMode::GeosetTakeover;
	m_config.targetGeosetCategory = geosetCategory;
	m_config.targetGeosetId = desiredId;
	m_config.preferBoneDrive = true;
	m_rawJson.clear();
	m_bAutoManaged = true;
	InvalidateGeometry();
}

void CharCapeInstance::ClearAutoGeosetTakeover()
{
	if (m_bAutoManaged)
		Clear();
}

void CharCapeInstance::Clear()
{
	m_config = CapeConfig();
	m_config.mode = CapeMode::None;
	m_rawJson.clear();
	m_bAutoManaged = false;
	m_bTargetActive = false;
	m_fSimBlend = 0.0f;
	m_outerTex.reset();
	m_innerTex.reset();
	m_emblemTex.reset();
	InvalidateGeometry();
}

bool CharCapeInstance::ShouldSuppressModelGeoset(int geosetId) const
{
	if (geosetId <= 0 || !IsActive())
		return false;

	if (m_config.mode == CapeMode::ProceduralMesh && m_config.replaceBackGeoset)
	{
		const int cat = geosetId / 100;
		const int sub = geosetId % 100;
		// Hide legacy cape geoset or Haqi cloth-cape geosets (1002+) when wearing a DIY Sky cape
		if (cat == CharModelInstance::CSET_CAPE || (cat == CharModelInstance::CSET_WINGS && sub >= 2))
			return true;
	}
	else if (m_config.mode == CapeMode::GeosetTakeover)
	{
		if (!m_bUseBoneChainDrive && m_bGeometryBuilt && m_fSimBlend > 0.001f && geosetId == m_boundGeosetId)
			return true;
	}
	return false;
}

CParaXModel* CharCapeInstance::ResolveTargetModel(CParaXModel* pBaseModel, ParaXModelCanvas* pCanvas) const
{
	if (m_config.mode == CapeMode::GeosetTakeover && m_config.targetAttachmentId >= 0 && pCanvas)
	{
		CanvasAttachment* pAtt = pCanvas->GetChild(m_config.targetAttachmentId);
		if (pAtt && pAtt->model)
		{
			CParaXModel* pAttModel = pAtt->model->GetModel();
			if (pAttModel)
				return pAttModel;
		}
	}
	return pBaseModel;
}

void CharCapeInstance::ExtractRigAnchorsAndCapsules(
	CParaXModel* pModel,
	CapeVec3& outNeck,
	CapeVec3& outLeftShoulder,
	CapeVec3& outRightShoulder,
	CapeCapsule* outCapsules,
	size_t& outCapsuleCount) const
{
	outCapsuleCount = 0;
	if (!pModel)
		return;

	const uint32_t nBones = pModel->GetObjectNum().nBones;
	auto getBonePos = [&](int knownBoneId, CapeVec3& outPos) -> bool
	{
		if (knownBoneId <= 0 || knownBoneId >= MAX_KNOWN_BONE_NODE)
			return false;
		const int idx = pModel->m_boneLookup[knownBoneId];
		if (idx >= 0 && static_cast<uint32_t>(idx) < nBones && pModel->bones)
		{
			outPos = ToCapeVec(pModel->bones[idx].GetAnimatedPivotPoint());
			return true;
		}
		return false;
	};

	auto getAttachPos = [&](int attId, CapeVec3& outPos) -> bool
	{
		if (attId < 0 || attId >= CParaXModel::MAX_MODEL_ATTACHMENTS)
			return false;
		const int attIdx = pModel->m_attLookup[attId];
		if (attIdx >= 0 && static_cast<size_t>(attIdx) < pModel->m_atts.size())
		{
			const ModelAttachment& att = pModel->m_atts[static_cast<size_t>(attIdx)];
			if (att.bone >= 0 && static_cast<uint32_t>(att.bone) < nBones && pModel->bones)
			{
				const Vector3 p = pModel->bones[att.bone].mat * att.pos;
				outPos = ToCapeVec(p);
				return true;
			}
			outPos = ToCapeVec(att.pos);
			return true;
		}
		return false;
	};

	const Vector3& minE = pModel->GetHeader().minExtent;
	const Vector3& maxE = pModel->GetHeader().maxExtent;
	float boxHeight = maxE.y - minE.y;
	if (boxHeight < 0.35f || boxHeight > 6.0f)
		boxHeight = 1.65f;
	const float baseY = (std::fabs(minE.y) < 1.0f) ? minE.y : 0.0f;

	CapeVec3 spinePos = { 0.0f, baseY + boxHeight * 0.58f, 0.0f };
	CapeVec3 pelvisPos = { 0.0f, baseY + boxHeight * 0.46f, 0.0f };
	const bool hasSpine = getBonePos(Bone_Spine1, spinePos) || getBonePos(Bone_Spine, spinePos);
	const bool hasPelvis = getBonePos(Bone_Pelvis, pelvisPos);

	if (!getBonePos(Bone_Neck, outNeck))
	{
		if (!getAttachPos(ATT_ID_NECK, outNeck))
		{
			CapeVec3 backAtt;
			if (getAttachPos(ATT_ID_BACK_ADDON, backAtt))
				outNeck = backAtt + m_upDir * (0.12f * m_modelScale);
			else if (hasSpine)
				outNeck = spinePos + m_upDir * (0.20f * m_modelScale);
			else
				outNeck = { 0.0f, baseY + boxHeight * 0.76f, 0.0f };
		}
	}

	if (!getBonePos(Bone_L_Clavicle, outLeftShoulder) && !getBonePos(Bone_L_UpperArm, outLeftShoulder))
	{
		if (!getAttachPos(ATT_ID_SHOULDER_LEFT, outLeftShoulder))
			outLeftShoulder = outNeck - m_rightDir * (0.16f * m_modelScale) - m_upDir * (0.04f * m_modelScale);
	}
	if (!getBonePos(Bone_R_Clavicle, outRightShoulder) && !getBonePos(Bone_R_UpperArm, outRightShoulder))
	{
		if (!getAttachPos(ATT_ID_SHOULDER_RIGHT, outRightShoulder))
			outRightShoulder = outNeck + m_rightDir * (0.16f * m_modelScale) - m_upDir * (0.04f * m_modelScale);
	}
	// Ensure left/right shoulder order matches canonical rightDir (-Z) even if a rig swaps L/R bone IDs
	if ((outRightShoulder - outLeftShoulder).Dot(m_rightDir) < 0.0f)
	{
		std::swap(outLeftShoulder, outRightShoulder);
	}

	if (!outCapsules)
		return;

	// Build up to 5 fast body/leg collision capsules in character local space
	const CapeVec3 chestShift = m_forwardDir * (0.02f * m_modelScale);
	outCapsules[outCapsuleCount++] = {
		(hasSpine ? spinePos : (outNeck - m_upDir * (0.24f * m_modelScale))) + chestShift,
		outNeck - m_upDir * (0.04f * m_modelScale) + chestShift,
		0.155f * m_modelScale
	};

	outCapsules[outCapsuleCount++] = {
		hasPelvis ? pelvisPos : (outNeck - m_upDir * (0.44f * m_modelScale)),
		hasSpine ? spinePos : (outNeck - m_upDir * (0.24f * m_modelScale)),
		0.150f * m_modelScale
	};

	CapeVec3 lThigh, lCalf, rThigh, rCalf;
	if (getBonePos(Bone_L_Thigh, lThigh) && getBonePos(Bone_L_Calf, lCalf))
	{
		outCapsules[outCapsuleCount++] = { lThigh, lCalf, 0.105f * m_modelScale };
	}
	else
	{
		const CapeVec3 hipL = (hasPelvis ? pelvisPos : (outNeck - m_upDir * (0.44f * m_modelScale))) - m_rightDir * (0.09f * m_modelScale);
		outCapsules[outCapsuleCount++] = { hipL, hipL - m_upDir * (0.38f * m_modelScale), 0.105f * m_modelScale };
	}

	if (getBonePos(Bone_R_Thigh, rThigh) && getBonePos(Bone_R_Calf, rCalf))
	{
		outCapsules[outCapsuleCount++] = { rThigh, rCalf, 0.105f * m_modelScale };
	}
	else
	{
		const CapeVec3 hipR = (hasPelvis ? pelvisPos : (outNeck - m_upDir * (0.44f * m_modelScale))) + m_rightDir * (0.09f * m_modelScale);
		outCapsules[outCapsuleCount++] = { hipR, hipR - m_upDir * (0.38f * m_modelScale), 0.105f * m_modelScale };
	}

	CapeVec3 lArm, rArm;
	if (getBonePos(Bone_L_UpperArm, lArm) && getBonePos(Bone_R_UpperArm, rArm))
	{
		outCapsules[outCapsuleCount++] = { lArm, rArm, 0.095f * m_modelScale };
	}
}

bool CharCapeInstance::BuildProceduralCape(CParaXModel* pBaseModel)
{
	if (!pBaseModel)
		return false;

	const Vector3& minE = pBaseModel->GetHeader().minExtent;
	const Vector3& maxE = pBaseModel->GetHeader().maxExtent;
	float charHeight = maxE.y - minE.y;
	if (charHeight < 0.4f || charHeight > 5.0f)
	{
		// Fallback from head/neck pivot height when model header bounds are unpopulated
		const int neckIdx = pBaseModel->m_boneLookup[Bone_Neck];
		if (neckIdx >= 0 && static_cast<uint32_t>(neckIdx) < pBaseModel->GetObjectNum().nBones && pBaseModel->bones)
		{
			const float ny = pBaseModel->bones[neckIdx].GetAnimatedPivotPoint().y;
			if (ny > 0.3f && ny < 4.0f)
				charHeight = ny / 0.78f;
		}
		if (charHeight < 0.4f || charHeight > 5.0f)
			charHeight = 1.72f;
	}
	m_modelScale = m_config.autoScale ? (std::max)(0.45f, (std::min)(2.2f, charHeight / 1.72f)) : 1.0f;

	// Canonical ParaEngine CBipedObject / CParaXModel basis: +X forward, +Y up, -Z right
	m_upDir = { 0.0f, 1.0f, 0.0f };
	m_forwardDir = { 1.0f, 0.0f, 0.0f };
	m_rightDir = { 0.0f, 0.0f, -1.0f };

	CapeVec3 neck, leftSh, rightSh;
	size_t capCount = 0;
	ExtractRigAnchorsAndCapsules(pBaseModel, neck, leftSh, rightSh, nullptr, capCount);
	m_initialNeckRef = neck;
	m_currentNeckAnchor = neck;

	m_cloth.BuildProcedural(
		m_config.style,
		neck, leftSh, rightSh,
		m_forwardDir, m_upDir, m_rightDir,
		m_modelScale);

	// Precompute zero-physics static fallback triangles so distant characters do 0 math per frame
	m_cloth.BuildSurfaceTriangles(
		0.0f,
		m_initialNeckRef,
		m_initialNeckRef,
		m_cachedStaticOuterTris,
		m_config.doubleSided ? &m_cachedStaticInnerTris : nullptr);

	m_pBuiltForModel = pBaseModel;
	m_bGeometryBuilt = !m_cachedStaticOuterTris.empty();
	return m_bGeometryBuilt;
}

bool CharCapeInstance::BuildGeosetTakeover(CParaXModel* pBaseModel, ParaXModelCanvas* pCanvas, int resolvedGeosetId)
{
	CParaXModel* pTargetModel = ResolveTargetModel(pBaseModel, pCanvas);
	if (!pTargetModel || !pTargetModel->m_origVertices || !pTargetModel->m_indices)
		return false;

	// Establish basis & model scale from base model
	BuildProceduralCape(pBaseModel ? pBaseModel : pTargetModel);
	m_cachedStaticOuterTris.clear();
	m_cachedStaticInnerTris.clear();

	const uint32_t nBones = pTargetModel->GetObjectNum().nBones;
	const uint32_t nVerts = pTargetModel->GetObjectNum().nVertices;
	const uint32_t nIndices = pTargetModel->GetObjectNum().nIndices;

	// Collect matching passes for resolvedGeosetId (or all passes if attachment model takeover)
	std::vector<int> matchingPasses;
	for (size_t pi = 0; pi < pTargetModel->passes.size(); ++pi)
	{
		const ModelRenderPass& pass = pTargetModel->passes[pi];
		if (pass.indexCount == 0 || pass.geoset < 0 || static_cast<size_t>(pass.geoset) >= pTargetModel->geosets.size())
			continue;
		if (m_config.targetAttachmentId >= 0 && pTargetModel != pBaseModel)
		{
			matchingPasses.push_back(static_cast<int>(pi));
		}
		else if (pTargetModel->geosets[static_cast<size_t>(pass.geoset)].id == resolvedGeosetId)
		{
			matchingPasses.push_back(static_cast<int>(pi));
		}
	}

	if (matchingPasses.empty() && m_config.boneIndices.empty() && m_config.bonePrefix.empty())
		return false;

	// Check for explicit or dedicated cape bones
	std::vector<bool> isBodyKnownBone(nBones, false);
	for (int kb = 1; kb < MAX_KNOWN_BONE_NODE; ++kb)
	{
		const int bi = pTargetModel->m_boneLookup[kb];
		if (bi >= 0 && static_cast<uint32_t>(bi) < nBones)
			isBodyKnownBone[static_cast<uint32_t>(bi)] = true;
	}

	std::vector<bool> usedByCapeGeoset(nBones, false);
	std::vector<bool> usedByOtherGeoset(nBones, false);

	for (size_t pi = 0; pi < pTargetModel->passes.size(); ++pi)
	{
		ModelRenderPass& pass = pTargetModel->passes[pi];
		if (pass.indexCount == 0 || pass.geoset < 0 || static_cast<size_t>(pass.geoset) >= pTargetModel->geosets.size())
			continue;
		const bool isTargetPass = std::find(matchingPasses.begin(), matchingPasses.end(), static_cast<int>(pi)) != matchingPasses.end();
		const int vOffset = pass.GetVertexStart(pTargetModel);
		const uint32_t iStart = pass.GetStartIndex();
		for (uint32_t k = 0; k < pass.indexCount && (iStart + k) < nIndices; ++k)
		{
			const int vIdx = static_cast<int>(pTargetModel->m_indices[iStart + k]) + vOffset;
			if (vIdx < 0 || static_cast<uint32_t>(vIdx) >= nVerts)
				continue;
			const ModelVertex& mv = pTargetModel->m_origVertices[vIdx];
			for (int b = 0; b < 4; ++b)
			{
				if (mv.weights[b] > 0 && mv.bones[b] < nBones)
				{
					if (isTargetPass)
						usedByCapeGeoset[mv.bones[b]] = true;
					else
						usedByOtherGeoset[mv.bones[b]] = true;
				}
			}
		}
	}

	std::vector<int> candidateBones;
	if (!m_config.boneIndices.empty())
	{
		for (int bi : m_config.boneIndices)
		{
			if (bi >= 0 && static_cast<uint32_t>(bi) < nBones)
				candidateBones.push_back(bi);
		}
	}
	else if (!m_config.bonePrefix.empty() && pTargetModel->bones)
	{
		const std::string pref = ToLowerCopy(m_config.bonePrefix);
		for (uint32_t bi = 0; bi < nBones; ++bi)
		{
			const std::string bname = ToLowerCopy(pTargetModel->bones[bi].GetName());
			if (!bname.empty() && bname.find(pref) != std::string::npos && !pTargetModel->bones[bi].IsAttachment())
				candidateBones.push_back(static_cast<int>(bi));
		}
	}
	else if (m_config.preferBoneDrive && pTargetModel->bones)
	{
		for (uint32_t bi = 0; bi < nBones; ++bi)
		{
			if (usedByCapeGeoset[bi] && !usedByOtherGeoset[bi] && !isBodyKnownBone[bi] && !pTargetModel->bones[bi].IsAttachment())
				candidateBones.push_back(static_cast<int>(bi));
		}
	}

	m_boneLinks.clear();
	if (!candidateBones.empty() && m_config.preferBoneDrive && pTargetModel->bones)
	{
		// Order candidate bones parent-before-child and map onto simulated cloth grid
		std::sort(candidateBones.begin(), candidateBones.end());
		candidateBones.erase(std::unique(candidateBones.begin(), candidateBones.end()), candidateBones.end());

		// Compute bounding span of candidate bone pivots to map onto cloth (u, v)
		float minU = 1e9f, maxU = -1e9f, minY = 1e9f, maxY = -1e9f;
		std::vector<CapeVec3> pivots(candidateBones.size());
		for (size_t i = 0; i < candidateBones.size(); ++i)
		{
			pivots[i] = ToCapeVec(pTargetModel->bones[candidateBones[i]].GetAnimatedPivotPoint());
			const float uProj = pivots[i].Dot(m_rightDir);
			minU = (std::min)(minU, uProj);
			maxU = (std::max)(maxU, uProj);
			minY = (std::min)(minY, pivots[i].y);
			maxY = (std::max)(maxY, pivots[i].y);
		}

		const float spanW = (std::max)(0.24f * m_modelScale, (maxU - minU) + 0.12f * m_modelScale);
		const float spanH = (std::max)(0.35f * m_modelScale, (maxY - minY) + 0.20f * m_modelScale);
		CapeStyleParams proxyStyle = m_config.style;
		proxyStyle.collarStyle = CapeCollarStyle::ClassicBack;
		proxyStyle.hemStyle = CapeHemStyle::Rounded;
		proxyStyle.gridCols = 7;
		proxyStyle.gridRows = 6;
		proxyStyle.topWidth = spanW / m_modelScale;
		proxyStyle.bottomWidth = (spanW * 1.15f) / m_modelScale;
		proxyStyle.length = spanH / m_modelScale;

		CapeVec3 neck, leftSh, rightSh;
		size_t capCount = 0;
		ExtractRigAnchorsAndCapsules(pBaseModel ? pBaseModel : pTargetModel, neck, leftSh, rightSh, nullptr, capCount);
		m_initialNeckRef = neck;
		m_cloth.BuildProcedural(proxyStyle, neck, leftSh, rightSh, m_forwardDir, m_upDir, m_rightDir, m_modelScale);

		for (size_t i = 0; i < candidateBones.size(); ++i)
		{
			const int bi = candidateBones[i];
			const int parentBone = pTargetModel->bones[bi].GetParentIndex();
			int parentLink = -1;
			for (size_t j = 0; j < m_boneLinks.size(); ++j)
			{
				if (m_boneLinks[j].boneIndex == parentBone)
				{
					parentLink = static_cast<int>(j);
					break;
				}
			}

			const float uNorm = (maxU > minU + 1e-3f) ? ((pivots[i].Dot(m_rightDir) - minU) / (maxU - minU)) : 0.5f;
			const float vNorm = (maxY > minY + 1e-3f) ? ((maxY - pivots[i].y) / (maxY - minY)) : 0.5f;
			const int col = (std::max)(0, (std::min)(m_cloth.cols - 1, static_cast<int>(std::round(uNorm * (m_cloth.cols - 1)))));
			const int row = (std::max)(1, (std::min)(m_cloth.rows - 2, 1 + static_cast<int>(std::round(vNorm * (m_cloth.rows - 3)))));

			// Find first child bone in candidateBones to define bone aim vector; otherwise aim downward
			CapeVec3 aimDir = m_upDir * -1.0f;
			float segLen = 0.18f * m_modelScale;
			for (size_t k = 0; k < candidateBones.size(); ++k)
			{
				if (pTargetModel->bones[candidateBones[k]].GetParentIndex() == bi)
				{
					const CapeVec3 d = pivots[k] - pivots[i];
					if (d.LengthSq() > 1e-5f)
					{
						segLen = d.Length();
						aimDir = d * (1.0f / segLen);
						break;
					}
				}
			}

			m_boneLinks.push_back({ bi, parentLink, row, col, aimDir, segLen });
		}

		m_bUseBoneChainDrive = !m_boneLinks.empty();
	}

	if (!m_bUseBoneChainDrive)
	{
		// Proxy Cloth Lattice mode: works for ALL Haqi cape geosets even when vertices have no dedicated cape bones!
		if (matchingPasses.empty())
			return false;

		float minSide = 1e9f, maxSide = -1e9f;
		float minY = 1e9f, maxY = -1e9f;
		float avgBack = 0.0f;
		size_t vSampleCount = 0;

		for (int pi : matchingPasses)
		{
			ModelRenderPass& pass = pTargetModel->passes[static_cast<size_t>(pi)];
			const int vOffset = pass.GetVertexStart(pTargetModel);
			const uint32_t iStart = pass.GetStartIndex();
			for (uint32_t k = 0; k < pass.indexCount && (iStart + k) < nIndices; ++k)
			{
				const int vIdx = static_cast<int>(pTargetModel->m_indices[iStart + k]) + vOffset;
				if (vIdx < 0 || static_cast<uint32_t>(vIdx) >= nVerts)
					continue;
				const CapeVec3 p = ToCapeVec(pTargetModel->m_origVertices[vIdx].pos);
				const float side = p.Dot(m_rightDir);
				minSide = (std::min)(minSide, side);
				maxSide = (std::max)(maxSide, side);
				minY = (std::min)(minY, p.y);
				maxY = (std::max)(maxY, p.y);
				avgBack += p.Dot(m_forwardDir * -1.0f);
				++vSampleCount;
			}
		}

		if (vSampleCount == 0 || maxY <= minY + 0.02f)
			return false;

		avgBack /= static_cast<float>(vSampleCount);
		const float widthSpan = (std::max)(0.18f, maxSide - minSide);
		const float heightSpan = (std::max)(0.18f, maxY - minY);
		const float centerSide = 0.5f * (minSide + maxSide);

		CapeVec3 neck, leftSh, rightSh;
		size_t capCount = 0;
		ExtractRigAnchorsAndCapsules(pTargetModel, neck, leftSh, rightSh, nullptr, capCount);

		// Align cloth top row directly to the geoset's own collar top so u,v=(*,0) matches the cape seam
		const CapeVec3 geosetTopCenter = m_rightDir * centerSide + m_upDir * maxY + (m_forwardDir * -1.0f) * (std::max)(0.02f, avgBack * 0.65f);
		CapeStyleParams proxyStyle;
		proxyStyle.collarStyle = CapeCollarStyle::ClassicBack;
		proxyStyle.hemStyle = CapeHemStyle::Rounded;
		proxyStyle.gridCols = 7;
		proxyStyle.gridRows = 6;
		proxyStyle.topWidth = widthSpan;
		proxyStyle.bottomWidth = widthSpan * 1.08f;
		proxyStyle.length = heightSpan;
		proxyStyle.backOffset = 0.0f;
		proxyStyle.bellCurvature = 0.03f * m_modelScale;
		proxyStyle.shoulderDrop = 0.0f;
		proxyStyle.attachOffset = geosetTopCenter - neck;

		m_initialNeckRef = neck;
		m_currentNeckAnchor = neck;
		m_cloth.BuildProcedural(proxyStyle, neck, leftSh, rightSh, m_forwardDir, m_upDir, m_rightDir, 1.0f);

		m_proxyPasses.clear();
		m_proxyTriVertices.clear();
		const CapeVec3 backDir = m_forwardDir * -1.0f;

		for (int pi : matchingPasses)
		{
			ModelRenderPass& pass = pTargetModel->passes[static_cast<size_t>(pi)];
			const int vOffset = pass.GetVertexStart(pTargetModel);
			const uint32_t iStart = pass.GetStartIndex();
			ProxyPassSlice slice;
			slice.passIndex = pi;
			slice.firstTriVertex = m_proxyTriVertices.size();

			for (uint32_t k = 0; k < pass.indexCount && (iStart + k) < nIndices; ++k)
			{
				const int vIdx = static_cast<int>(pTargetModel->m_indices[iStart + k]) + vOffset;
				if (vIdx < 0 || static_cast<uint32_t>(vIdx) >= nVerts)
					continue;
				const ModelVertex& mv = pTargetModel->m_origVertices[vIdx];
				const CapeVec3 p = ToCapeVec(mv.pos);
				const float u = (std::max)(0.0f, (std::min)(1.0f, (p.Dot(m_rightDir) - minSide) / widthSpan));
				const float v = (std::max)(0.0f, (std::min)(1.0f, (maxY - p.y) / heightSpan));

				// Bilinearly sample rest cloth surface to preserve exact thickness/relief offset of the original cape mesh
				const float gfC = u * static_cast<float>(m_cloth.cols - 1);
				const float gfR = v * static_cast<float>(m_cloth.rows - 1);
				const int c0 = (std::min)(m_cloth.cols - 2, static_cast<int>(gfC));
				const int r0 = (std::min)(m_cloth.rows - 2, static_cast<int>(gfR));
				const float tx = gfC - static_cast<float>(c0);
				const float ty = gfR - static_cast<float>(r0);
				const CapeVec3& p00 = m_cloth.particles[static_cast<size_t>(r0 * m_cloth.cols + c0)].restLocalPos;
				const CapeVec3& p10 = m_cloth.particles[static_cast<size_t>(r0 * m_cloth.cols + c0 + 1)].restLocalPos;
				const CapeVec3& p01 = m_cloth.particles[static_cast<size_t>((r0 + 1) * m_cloth.cols + c0)].restLocalPos;
				const CapeVec3& p11 = m_cloth.particles[static_cast<size_t>((r0 + 1) * m_cloth.cols + c0 + 1)].restLocalPos;
				const CapeVec3 gridRest = CapeLerp(CapeLerp(p00, p10, tx), CapeLerp(p01, p11, tx), ty);
				const float nOff = (p - gridRest).Dot(backDir);

				m_proxyTriVertices.push_back({
					static_cast<uint16_t>(vIdx),
					u,
					v,
					nOff,
					{ mv.texcoords.x, mv.texcoords.y }
				});
			}

			slice.triVertexCount = m_proxyTriVertices.size() - slice.firstTriVertex;
			if (slice.triVertexCount >= 3)
				m_proxyPasses.push_back(slice);
		}

		if (m_proxyPasses.empty())
			return false;
	}

	m_boundGeosetId = resolvedGeosetId;
	m_pBuiltForModel = pBaseModel ? pBaseModel : pTargetModel;
	m_bGeometryBuilt = true;
	return true;
}

bool CharCapeInstance::EnsureBuilt(CParaXModel* pBaseModel, ParaXModelCanvas* pCanvas, const int* charGeosets)
{
	if (!pBaseModel || !IsActive())
		return false;

	if (m_config.mode == CapeMode::ProceduralMesh)
	{
		if (m_bGeometryBuilt && m_pBuiltForModel == pBaseModel)
			return true;
		return BuildProceduralCape(pBaseModel);
	}
	else if (m_config.mode == CapeMode::GeosetTakeover)
	{
		int resolvedId = m_config.targetGeosetId;
		if (resolvedId <= 0 && charGeosets)
		{
			if (m_config.targetGeosetCategory >= 0 && m_config.targetGeosetCategory < CharModelInstance::NUM_CHAR_GEOSETS)
			{
				const int sub = charGeosets[m_config.targetGeosetCategory];
				if (sub > 0)
					resolvedId = m_config.targetGeosetCategory * 100 + sub;
			}
			else
			{
				if (charGeosets[CharModelInstance::CSET_CAPE] > 0)
					resolvedId = CharModelInstance::CSET_CAPE * 100 + charGeosets[CharModelInstance::CSET_CAPE];
				else if (charGeosets[CharModelInstance::CSET_WINGS] >= 2)
					resolvedId = CharModelInstance::CSET_WINGS * 100 + charGeosets[CharModelInstance::CSET_WINGS];
			}
		}

		if (resolvedId <= 0 && m_config.targetAttachmentId < 0)
			return false;

		if (m_bGeometryBuilt && m_pBuiltForModel == pBaseModel && m_boundGeosetId == resolvedId)
			return true;

		return BuildGeosetTakeover(pBaseModel, pCanvas, resolvedId);
	}
	return false;
}

void CharCapeInstance::ApplyBoneChainOverrides(CParaXModel* pTargetModel)
{
	if (!pTargetModel || !pTargetModel->bones || m_boneLinks.empty() || m_fSimBlend <= 0.001f)
		return;

	const uint32_t nBones = pTargetModel->GetObjectNum().nBones;
	std::vector<CapeVec3> linkDisp(m_boneLinks.size(), { 0.0f, 0.0f, 0.0f });

	for (size_t i = 0; i < m_boneLinks.size(); ++i)
	{
		const BoneChainLink& link = m_boneLinks[i];
		if (link.boneIndex < 0 || static_cast<uint32_t>(link.boneIndex) >= nBones)
			continue;

		const CapeVec3 parentShift = (link.parentLink >= 0) ? linkDisp[static_cast<size_t>(link.parentLink)] : CapeVec3{ 0.0f, 0.0f, 0.0f };
		const CapeVec3 animPivot = ToCapeVec(pTargetModel->bones[link.boneIndex].GetAnimatedPivotPoint());

		const CapeVec3& pTop = m_cloth.particles[static_cast<size_t>(link.particleRow * m_cloth.cols + link.particleCol)].pos;
		const CapeVec3& pBot = m_cloth.particles[static_cast<size_t>((link.particleRow + 1) * m_cloth.cols + link.particleCol)].pos;
		const CapeVec3 simDir = (pBot - pTop).Normalized(link.restDirFromParent);
		const CapeVec3 targetDir = CapeLerp(link.restDirFromParent, simDir, m_fSimBlend).Normalized(link.restDirFromParent);

		const CapeMat3 rot3 = CapeMat3::FromTwoVectors(link.restDirFromParent, targetDir);
		Matrix4 mToOrigin, mRot, mBack;
		mToOrigin.makeTrans(-animPivot.x, -animPivot.y, -animPivot.z);
		mRot.identity();
		for (int r = 0; r < 3; ++r)
			for (int c = 0; c < 3; ++c)
				mRot.m[r][c] = rot3.m[r][c];
		mBack.makeTrans(animPivot.x + parentShift.x, animPivot.y + parentShift.y, animPivot.z + parentShift.z);

		const Matrix4 mDelta = mToOrigin * mRot * mBack;
		pTargetModel->bones[link.boneIndex].mat = pTargetModel->bones[link.boneIndex].mat * mDelta;
		pTargetModel->bones[link.boneIndex].mrot = pTargetModel->bones[link.boneIndex].mrot * mRot;

		const CapeVec3 rotatedSeg = rot3.Transform(link.restDirFromParent * link.restLen);
		linkDisp[i] = parentShift + (rotatedSeg - link.restDirFromParent * link.restLen);
	}
}

void CharCapeInstance::UpdateSimulation(
	SceneState* sceneState,
	CParaXModel* pBaseModel,
	ParaXModelCanvas* pCanvas,
	const int* charGeosets,
	IAttributeFields* pAnimInstance)
{
	(void)pAnimInstance;
	if (!IsActive() || !sceneState || !pBaseModel)
		return;

	if (!EnsureBuilt(pBaseModel, pCanvas, charGeosets))
		return;

	// Deduplicate shadow/reflection/selection passes within the same render frame
	const int renderCount = sceneState->m_nRenderCount;
	if (renderCount > 0)
	{
		if (m_nLastSimRenderCount == renderCount)
		{
			if (m_config.mode == CapeMode::GeosetTakeover && m_bUseBoneChainDrive && m_fSimBlend > 0.001f)
				ApplyBoneChainOverrides(ResolveTargetModel(pBaseModel, pCanvas));
			return;
		}
		m_nLastSimRenderCount = renderCount;
	}
	else if (sceneState->IsShadowPass())
	{
		return;
	}

	CBaseObject* pSceneObj = sceneState->GetCurrentSceneObject();
	CBipedObject* pBiped = (pSceneObj && pSceneObj->IsBiped()) ? static_cast<CBipedObject*>(pSceneObj) : nullptr;
	const bool isMainPlayer = (pSceneObj != nullptr) && (CGlobals::GetScene() != nullptr) && (pSceneObj == CGlobals::GetScene()->GetCurrentPlayer());
	const bool isMiniScenePreview = (renderCount == 0);
	const bool isHighPriority = isMainPlayer || isMiniScenePreview;

	const float dt = (sceneState->dTimeDelta > 0.0 && sceneState->dTimeDelta < 0.5)
		? static_cast<float>(sceneState->dTimeDelta)
		: (1.0f / 60.0f);
	const float camDist = sceneState->GetCameraToCurObjectDistance();

	bool shouldSimulate = false;
	if (isMiniScenePreview)
	{
		// Offscreen MiniSceneGraph preview (avatar editor / shop): simulate independently of main-world crowd counts
		const auto& budget = CapeSimulationBudget::GetInstance();
		m_bTargetActive = budget.IsEnabled() && (camDist <= budget.GetDeactivateDistance());
		const float blendStep = (std::min)(1.0f, dt / 0.25f);
		m_fSimBlend = m_bTargetActive
			? (std::min)(1.0f, m_fSimBlend + blendStep)
			: (std::max)(0.0f, m_fSimBlend - blendStep);
		shouldSimulate = (m_fSimBlend > 0.001f);
	}
	else
	{
		shouldSimulate = CapeSimulationBudget::GetInstance().EvaluateInstance(
			reinterpret_cast<uintptr_t>(this),
			camDist,
			isMainPlayer,
			dt,
			m_bTargetActive,
			m_fSimBlend);
	}

	// Track current neck anchor even when static so distant procedural capes follow torso bobbing cleanly
	CapeCapsule capsules[6];
	size_t capsuleCount = 0;
	CapeVec3 neck, leftSh, rightSh;
	ExtractRigAnchorsAndCapsules(
		pBaseModel,
		neck, leftSh, rightSh,
		shouldSimulate ? capsules : nullptr,
		capsuleCount);
	m_currentNeckAnchor = neck;

	if (!shouldSimulate)
	{
		m_bHasPrevMotion = false;
		return;
	}

	m_accumTimeSec += dt;

	CapeMotionInput motion;
	motion.forwardDir = m_forwardDir;
	motion.upDir = m_upDir;
	motion.rightDir = m_rightDir;
	motion.timeSeconds = m_accumTimeSec;

	if (pBiped)
	{
		motion.isFlying = pBiped->GetIsFlying();
		const DVector3 curWorldPos = pBiped->GetPosition();
		const float curYaw = pBiped->GetYaw();

		if (!m_bHasPrevMotion)
		{
			m_prevWorldPos = curWorldPos;
			m_prevYaw = curYaw;
			m_bHasPrevMotion = true;
			motion.teleported = true;
		}
		else
		{
			const Vector3 worldDelta(
				static_cast<float>(curWorldPos.x - m_prevWorldPos.x),
				static_cast<float>(curWorldPos.y - m_prevWorldPos.y),
				static_cast<float>(curWorldPos.z - m_prevWorldPos.z));
			m_prevWorldPos = curWorldPos;

			const float yawDelta = NormalizeAnglePi(curYaw - m_prevYaw);
			m_prevYaw = curYaw;

			const float stepDistSq = worldDelta.squaredLength();
			if (stepDistSq > 9.0f) // > 3.0m single-frame jump -> teleport / respawn
			{
				motion.teleported = true;
			}
			else if (dt > 1e-4f)
			{
				const Vector3 worldVel = worldDelta * (1.0f / dt);
				// Rotate world velocity into character local space (inverse Yaw around +Y, compensating size scale)
				const float invScale = (pBiped->GetSizeScale() > 0.05f) ? (1.0f / pBiped->GetSizeScale()) : 1.0f;
				Matrix4 invRot;
				ParaMatrixRotationY(&invRot, -curYaw);
				Vector3 localVel;
				ParaVec3TransformCoord(&localVel, &worldVel, &invRot);
				motion.localVelocity = ToCapeVec(localVel * invScale);
				motion.yawAngularVel = yawDelta / dt;
			}
		}
	}
	else if (!m_bHasPrevMotion)
	{
		m_bHasPrevMotion = true;
		motion.teleported = true;
	}

	const int solverIters = isHighPriority ? 4 : 3;
	m_cloth.StepSimulation(
		dt,
		m_config.tuning,
		motion,
		neck, leftSh, rightSh,
		m_initialNeckRef,
		capsules, capsuleCount,
		solverIters);

	if (m_config.mode == CapeMode::GeosetTakeover && m_bUseBoneChainDrive)
	{
		ApplyBoneChainOverrides(ResolveTargetModel(pBaseModel, pCanvas));
	}
}

void CharCapeInstance::BuildProxyDeformedTriangles(CParaXModel* pBaseModel)
{
	m_dynamicOuterTris.clear();
	if (!pBaseModel || m_proxyTriVertices.empty() || m_cloth.particles.empty())
		return;

	// Compute smooth normals on the simulated cloth grid first
	const size_t nGrid = m_cloth.particles.size();
	std::vector<CapeVec3> gridNormals(nGrid, { 0.0f, 0.0f, 0.0f });
	for (size_t t = 0; t + 2 < m_cloth.triIndices.size(); t += 3)
	{
		const uint16_t i0 = m_cloth.triIndices[t + 0];
		const uint16_t i1 = m_cloth.triIndices[t + 1];
		const uint16_t i2 = m_cloth.triIndices[t + 2];
		const CapeVec3 fn = (m_cloth.particles[i1].pos - m_cloth.particles[i0].pos).Cross(
			m_cloth.particles[i2].pos - m_cloth.particles[i0].pos);
		gridNormals[i0] += fn;
		gridNormals[i1] += fn;
		gridNormals[i2] += fn;
	}
	const CapeVec3 fallbackBack = m_forwardDir * -1.0f;
	for (CapeVec3& gn : gridNormals)
		gn = gn.Normalized(fallbackBack);

	m_dynamicOuterTris.reserve(m_proxyTriVertices.size());
	const float blend = (std::max)(0.0f, (std::min)(1.0f, m_fSimBlend));

	for (const ProxyVertexMap& pv : m_proxyTriVertices)
	{
		const float gfC = pv.u * static_cast<float>(m_cloth.cols - 1);
		const float gfR = pv.v * static_cast<float>(m_cloth.rows - 1);
		const int c0 = (std::min)(m_cloth.cols - 2, static_cast<int>(gfC));
		const int r0 = (std::min)(m_cloth.rows - 2, static_cast<int>(gfR));
		const float tx = gfC - static_cast<float>(c0);
		const float ty = gfR - static_cast<float>(r0);

		const size_t k00 = static_cast<size_t>(r0 * m_cloth.cols + c0);
		const size_t k10 = k00 + 1;
		const size_t k01 = static_cast<size_t>((r0 + 1) * m_cloth.cols + c0);
		const size_t k11 = k01 + 1;

		const CapeVec3 clothP = CapeLerp(
			CapeLerp(m_cloth.particles[k00].pos, m_cloth.particles[k10].pos, tx),
			CapeLerp(m_cloth.particles[k01].pos, m_cloth.particles[k11].pos, tx),
			ty);
		const CapeVec3 clothN = CapeLerp(
			CapeLerp(gridNormals[k00], gridNormals[k10], tx),
			CapeLerp(gridNormals[k01], gridNormals[k11], tx),
			ty).Normalized(fallbackBack);

		CapeVec3 finalPos = clothP + clothN * pv.normalOffset;
		if (blend < 0.999f)
		{
			const CapeVec3 skinnedPos = ComputeSkinnedModelVertex(pBaseModel, pv.origVertexIndex);
			finalPos = CapeLerp(skinnedPos, finalPos, blend);
		}
		m_dynamicOuterTris.push_back({ finalPos, clothN, pv.uv });
	}
}

TextureEntity* CharCapeInstance::ResolveOuterTexture()
{
	auto* assetMgr = CGlobals::GetAssetManager();
	if (!assetMgr)
		return nullptr;

	if (!m_config.outerTexture.empty())
	{
		if (m_bTexturesDirty || !m_outerTex)
			m_outerTex = assetMgr->LoadTexture("", m_config.outerTexture, TextureEntity::StaticTexture);
		return m_outerTex.get();
	}

	if (m_bTexturesDirty || !m_outerTex)
	{
		char keyBuf[160];
		std::snprintf(
			keyBuf, sizeof(keyBuf),
			"_cape_proc_outer_%08x_%08x_%08x_%d_%d_%08x_%d",
			static_cast<unsigned int>(m_config.outerColor),
			static_cast<unsigned int>(m_config.gradientColor),
			static_cast<unsigned int>(m_config.trimColor),
			static_cast<int>(m_config.trimWidth * 1000.0f),
			(m_config.drawStarEmblem && m_config.emblemTexture.empty()) ? 1 : 0,
			static_cast<unsigned int>(m_config.emissiveColor),
			static_cast<int>(m_config.emblemScale * 1000.0f));

		m_outerTex = GetOrCreateSynthesizedCapeTexture(
			keyBuf,
			m_config.outerColor,
			m_config.gradientColor,
			m_config.trimColor,
			m_config.trimWidth,
			m_config.drawStarEmblem && m_config.emblemTexture.empty(),
			m_config.emissiveColor,
			m_config.emblemScale);
	}
	return m_outerTex.get();
}

TextureEntity* CharCapeInstance::ResolveInnerTexture()
{
	auto* assetMgr = CGlobals::GetAssetManager();
	if (!assetMgr)
		return nullptr;

	if (!m_config.innerTexture.empty())
	{
		if (m_bTexturesDirty || !m_innerTex)
			m_innerTex = assetMgr->LoadTexture("", m_config.innerTexture, TextureEntity::StaticTexture);
		return m_innerTex.get();
	}

	if (m_bTexturesDirty || !m_innerTex)
	{
		char keyBuf[128];
		std::snprintf(
			keyBuf, sizeof(keyBuf),
			"_cape_proc_inner_%08x_%08x_%d",
			static_cast<unsigned int>(m_config.innerColor),
			static_cast<unsigned int>(m_config.trimColor),
			static_cast<int>(m_config.trimWidth * 1000.0f));

		m_innerTex = GetOrCreateSynthesizedCapeTexture(
			keyBuf,
			m_config.innerColor,
			0x00000000u,
			m_config.trimColor,
			m_config.trimWidth * 0.7f,
			false,
			0,
			0.0f);
	}
	return m_innerTex.get();
}

TextureEntity* CharCapeInstance::ResolveEmblemTexture()
{
	if (m_config.emblemTexture.empty())
		return nullptr;
	auto* assetMgr = CGlobals::GetAssetManager();
	if (!assetMgr)
		return nullptr;
	if (m_bTexturesDirty || !m_emblemTex)
		m_emblemTex = assetMgr->LoadTexture("", m_config.emblemTexture, TextureEntity::StaticTexture);
	return m_emblemTex.get();
}

void CharCapeInstance::DrawTriangleList(
	SceneState* sceneState,
	const std::vector<CapeSurfaceVertex>& tris,
	TextureEntity* pTexture,
	DWORD tintColorARGB,
	DWORD emissiveColorARGB,
	float emissiveIntensity,
	int blendMode,
	bool enableBackfaceCull,
	CParameterBlock* materialParams)
{
	if (tris.size() < 3 || !sceneState || !pTexture || pTexture->GetTexture() == 0)
		return;

	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	EffectManager* pEffectMgr = CGlobals::GetEffectManager();
	if (!pd3dDevice || !pEffectMgr)
		return;

	const int prevTech = pEffectMgr->GetCurrentTechHandle();
	bool switchedTech = false;
	if (prevTech == TECH_CHARACTER_SKINNED || prevTech == TECH_BMAX_SKINNED)
	{
		if (pEffectMgr->BeginEffect(TECH_CHARACTER, &(sceneState->m_pCurrentEffect)))
			switchedTech = true;
	}

	CEffectFile* pEffect = pEffectMgr->GetCurrentEffectFile();
	DynamicVertexBufferEntity* pBufEntity = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_NORM);
	if (!pEffect || !pBufEntity)
	{
		if (switchedTech)
			pEffectMgr->BeginEffect(prevTech, &(sceneState->m_pCurrentEffect));
		return;
	}

	float opacity = static_cast<float>((tintColorARGB >> 24) & 0xff) / 255.0f;
	if (materialParams)
	{
		CParameter* pOpacity = materialParams->GetParameter("g_opacity");
		if (pOpacity)
			opacity *= static_cast<float>(*pOpacity);
	}
	if (opacity <= 0.001f)
		return;
	if (sceneState->IsIgnoreTransparent() && (blendMode == BM_ALPHA_BLEND || blendMode == BM_ADDITIVE || blendMode == BM_ADDITIVE_ALPHA || opacity < 0.999f))
		return;

	pd3dDevice->SetStreamSource(0, pBufEntity->GetBuffer(), 0, sizeof(mesh_vertex_normal));

	const LinearColor tintLin(tintColorARGB);
	ParaMaterial savedMat = sceneState->GetCurrentMaterial();
	ParaMaterial capeMat = savedMat;
	if (!sceneState->HasLocalMaterial())
	{
		capeMat.Diffuse = LinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	capeMat.Diffuse.r *= tintLin.r;
	capeMat.Diffuse.g *= tintLin.g;
	capeMat.Diffuse.b *= tintLin.b;
	capeMat.Ambient.r *= (0.35f + 0.65f * tintLin.r);
	capeMat.Ambient.g *= (0.35f + 0.65f * tintLin.g);
	capeMat.Ambient.b *= (0.35f + 0.65f * tintLin.b);

	if (emissiveIntensity > 0.001f && !sceneState->IsShadowPass())
	{
		const LinearColor emLin(emissiveColorARGB);
		capeMat.Emissive = LinearColor(
			emLin.r * emissiveIntensity,
			emLin.g * emissiveIntensity,
			emLin.b * emissiveIntensity,
			1.0f);
	}
	else
	{
		capeMat.Emissive = LinearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	if (pEffect->begin())
	{
		if (pEffect->BeginPass(0))
		{
			pEffect->applySurfaceMaterial(&capeMat);
			pEffect->applyWorldMatrices();
			pEffect->setTexture(0, pTexture);
			pEffect->setFloat(CEffectFile::k_opacity, opacity);

			if (!sceneState->IsShadowPass())
			{
				if (blendMode == BM_TRANSPARENT)
				{
					pEffect->EnableAlphaTesting(true);
				}
				else if (blendMode == BM_ALPHA_BLEND || opacity < 0.999f)
				{
					pEffect->EnableAlphaBlending(true);
				}
				else if (blendMode == BM_ADDITIVE || blendMode == BM_ADDITIVE_ALPHA)
				{
					pEffect->EnableAlphaBlending(true);
					pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
					pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
					pEffectMgr->EnableZWrite(false);
				}
				pEffectMgr->SetCullingMode(enableBackfaceCull);
			}
			else
			{
				pEffectMgr->SetCullingMode(false);
			}

			pEffect->CommitChanges();

			const int totalVerts = static_cast<int>((tris.size() / 3) * 3);
			int finishedVerts = 0;
			mesh_vertex_normal* vbVertices = nullptr;
			while (finishedVerts < totalVerts)
			{
				const int lockedVerts = pBufEntity->Lock(totalVerts - finishedVerts, reinterpret_cast<void**>(&vbVertices));
				if (lockedVerts <= 0)
					break;
				const int triCount = lockedVerts / 3;
				const int copyVerts = triCount * 3;
				for (int i = 0; i < copyVerts; ++i)
				{
					const CapeSurfaceVertex& sv = tris[static_cast<size_t>(finishedVerts + i)];
					vbVertices[i].p = ToParaVec(sv.pos);
					vbVertices[i].n = ToParaVec(sv.normal);
					vbVertices[i].uv = Vector2(sv.uv.x, sv.uv.y);
				}
				pBufEntity->Unlock();

				if (triCount > 0)
				{
					if (pBufEntity->IsMemoryBuffer())
					{
						RenderDevice::DrawPrimitiveUP(
							pd3dDevice,
							RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER,
							EPrimitiveType::TRIANGLELIST,
							triCount,
							pBufEntity->GetBaseVertexPointer(),
							pBufEntity->m_nUnitSize);
					}
					else
					{
						RenderDevice::DrawPrimitive(
							pd3dDevice,
							RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER,
							EPrimitiveType::TRIANGLELIST,
							pBufEntity->GetBaseVertex(),
							triCount);
					}
				}

				if (copyVerts <= 0)
					break;
				finishedVerts += copyVerts;
			}

			// Restore render states cleanly
			if (!sceneState->IsShadowPass())
			{
				if (blendMode == BM_ADDITIVE || blendMode == BM_ADDITIVE_ALPHA)
				{
					pEffectMgr->EnableZWrite(true);
					pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
					pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
					pEffect->EnableAlphaBlending(false);
				}
				else if (blendMode == BM_ALPHA_BLEND || opacity < 0.999f)
				{
					pEffect->EnableAlphaBlending(false);
				}
				else if (blendMode == BM_TRANSPARENT)
				{
					pEffect->EnableAlphaTesting(false);
				}
				pEffect->setFloat(CEffectFile::k_opacity, 1.0f);
			}
			pEffectMgr->SetCullingMode(true);
			capeMat.Emissive = LinearColor(0.0f, 0.0f, 0.0f, 0.0f);
			pEffect->applySurfaceMaterial(&savedMat);

			pEffect->EndPass(0);
		}
		pEffect->end();
	}

	if (switchedTech)
	{
		pEffectMgr->BeginEffect(prevTech, &(sceneState->m_pCurrentEffect));
	}
}

void CharCapeInstance::Draw(
	SceneState* sceneState,
	CParaXModel* pBaseModel,
	ParaXModelCanvas* pCanvas,
	CParameterBlock* materialParams,
	PlayerReflectionMesh* reflection)
{
	(void)pCanvas;
	if (!IsActive() || !m_bGeometryBuilt || !sceneState || !pBaseModel || reflection != nullptr)
		return;

	if (m_config.mode == CapeMode::ProceduralMesh)
	{
		TextureEntity* pOuterTex = ResolveOuterTexture();
		TextureEntity* pInnerTex = m_config.doubleSided ? ResolveInnerTexture() : nullptr;
		TextureEntity* pEmblemTex = ResolveEmblemTexture();
		m_bTexturesDirty = false;

		const DWORD whiteIfSynthesized = m_config.outerTexture.empty() ? 0xffffffffu : m_config.outerColor;
		const DWORD innerTint = m_config.innerTexture.empty() ? 0xffffffffu : m_config.innerColor;
		const int outerBlend = (!m_config.outerTexture.empty()) ? BM_TRANSPARENT : BM_OPAQUE;

		if (m_fSimBlend > 0.001f)
		{
			// Nearby active / blending cape: build smooth dynamic mesh from XPBD particles
			m_cloth.BuildSurfaceTriangles(
				m_fSimBlend,
				m_currentNeckAnchor,
				m_initialNeckRef,
				m_dynamicOuterTris,
				m_config.doubleSided ? &m_dynamicInnerTris : nullptr);

			DrawTriangleList(
				sceneState,
				m_dynamicOuterTris,
				pOuterTex,
				whiteIfSynthesized,
				m_config.emissiveColor,
				m_config.emissiveIntensity * 0.28f,
				outerBlend,
				m_config.doubleSided,
				materialParams);

			if (m_config.doubleSided && pInnerTex)
			{
				DrawTriangleList(
					sceneState,
					m_dynamicInnerTris,
					pInnerTex,
					innerTint,
					m_config.emissiveColor,
					m_config.emissiveIntensity * 0.12f,
					BM_OPAQUE,
					true,
					materialParams);
			}

			if (pEmblemTex && m_config.emissiveIntensity > 0.01f && !sceneState->IsShadowPass())
			{
				DrawTriangleList(
					sceneState,
					m_dynamicOuterTris,
					pEmblemTex,
					m_config.emissiveColor,
					m_config.emissiveColor,
					m_config.emissiveIntensity,
					BM_ADDITIVE_ALPHA,
					true,
					materialParams);
			}
		}
		else
		{
			// Distant / over-budget static fallback: zero physics, push rigid neck offset matrix and draw cached rest mesh
			const CapeVec3 rigidOffset = m_currentNeckAnchor - m_initialNeckRef;
			Matrix4 matWorld = CGlobals::GetWorldMatrixStack().SafeGetTop();
			if (rigidOffset.LengthSq() > 1e-6f)
			{
				Matrix4 matOff;
				matOff.makeTrans(rigidOffset.x, rigidOffset.y, rigidOffset.z);
				matWorld = matOff * matWorld;
			}
			CGlobals::GetWorldMatrixStack().push(matWorld);

			DrawTriangleList(
				sceneState,
				m_cachedStaticOuterTris,
				pOuterTex,
				whiteIfSynthesized,
				m_config.emissiveColor,
				m_config.emissiveIntensity * 0.25f,
				outerBlend,
				m_config.doubleSided,
				materialParams);

			if (m_config.doubleSided && pInnerTex)
			{
				DrawTriangleList(
					sceneState,
					m_cachedStaticInnerTris,
					pInnerTex,
					innerTint,
					0,
					0.0f,
					BM_OPAQUE,
					true,
					materialParams);
			}

			CGlobals::GetWorldMatrixStack().pop();
		}
	}
	else if (m_config.mode == CapeMode::GeosetTakeover)
	{
		// When using bone chain drive OR when far away (m_fSimBlend == 0), native CParaXModel already rendered it!
		if (m_bUseBoneChainDrive || m_fSimBlend <= 0.001f || m_proxyPasses.empty())
			return;

		BuildProxyDeformedTriangles(pBaseModel);
		if (m_dynamicOuterTris.empty())
			return;

		std::vector<CapeSurfaceVertex> sliceVerts;
		for (const ProxyPassSlice& slice : m_proxyPasses)
		{
			if (slice.passIndex < 0 || static_cast<size_t>(slice.passIndex) >= pBaseModel->passes.size())
				continue;
			ModelRenderPass& pass = pBaseModel->passes[static_cast<size_t>(slice.passIndex)];

			TextureEntity* bindTex = nullptr;
			const int32_t texIdx = pass.GetTexture1();
			if (texIdx >= CParaXModel::MAX_MODEL_TEXTURES || pBaseModel->specialTextures[texIdx] == -1)
			{
				if (texIdx >= 0 && static_cast<uint32_t>(texIdx) < pBaseModel->GetObjectNum().nTextures && pBaseModel->textures)
					bindTex = pBaseModel->textures[texIdx].get();
			}
			else
			{
				bindTex = pBaseModel->replaceTextures[pBaseModel->specialTextures[texIdx]];
				if (!bindTex && texIdx >= 0 && static_cast<uint32_t>(texIdx) < pBaseModel->GetObjectNum().nTextures && pBaseModel->textures)
					bindTex = pBaseModel->textures[texIdx].get();
			}
			if (!bindTex)
				continue;

			sliceVerts.assign(
				m_dynamicOuterTris.begin() + static_cast<ptrdiff_t>(slice.firstTriVertex),
				m_dynamicOuterTris.begin() + static_cast<ptrdiff_t>(slice.firstTriVertex + slice.triVertexCount));

			DrawTriangleList(
				sceneState,
				sliceVerts,
				bindTex,
				0xffffffffu,
				0,
				0.0f,
				pass.blendmode,
				false, // Capes are double-sided
				materialParams);
		}
	}
}
