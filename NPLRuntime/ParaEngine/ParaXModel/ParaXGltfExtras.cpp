//-----------------------------------------------------------------------------
// Class:	ParaXGltfExtras
// Desc:	glTF 2.0 / GLB extras.parax & node.extras particle and ribbon support
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "ParaXBone.h"
#include "particle.h"
#include "GltfModel.h"
#include "ParaXGltfExtras.h"
#include <algorithm>
#include <cstring>
#include "memdebug.h"

using namespace ParaEngine;

namespace
{
	uint32_t ParseHexARGB(const std::string& s)
	{
		std::string hex = s;
		if (!hex.empty() && hex[0] == '#')
			hex = hex.substr(1);
		if (hex.size() == 6)
			hex = "FF" + hex; // #RRGGBB -> #FFRRGGBB
		uint32_t val = 0xFFFFFFFFu;
		try {
			val = static_cast<uint32_t>(std::stoul(hex, nullptr, 16));
		} catch (...) {
			val = 0xFFFFFFFFu;
		}
		return val;
	}

	Vector4 ParseColorValue(const Json::Value& v, const Vector4& fallback = Vector4(1, 1, 1, 1))
	{
		if (v.isString())
		{
			LinearColor lc{Color(ParseHexARGB(v.asString()))};
			return reinterpret_cast<const Vector4&>(lc);
		}
		if (v.isArray() && v.size() >= 3)
		{
			float a = (v.size() >= 4) ? static_cast<float>(v[3u].asDouble()) : 1.0f;
			return Vector4(static_cast<float>(v[0u].asDouble()), static_cast<float>(v[1u].asDouble()), static_cast<float>(v[2u].asDouble()), a);
		}
		if (v.isIntegral() || v.isUInt())
		{
			LinearColor lc{Color(v.asUInt())};
			return reinterpret_cast<const Vector4&>(lc);
		}
		return fallback;
	}

	Vector3 ParseVec3(const Json::Value& v, const Vector3& fallback = Vector3(0, 0, 0))
	{
		if (v.isArray() && v.size() >= 3)
			return Vector3(static_cast<float>(v[0u].asDouble()), static_cast<float>(v[1u].asDouble()), static_cast<float>(v[2u].asDouble()));
		return fallback;
	}

	Quaternion ParseQuat(const Json::Value& v)
	{
		if (v.isArray() && v.size() >= 4)
			return Quaternion(static_cast<float>(v[0u].asDouble()), static_cast<float>(v[1u].asDouble()), static_cast<float>(v[2u].asDouble()), static_cast<float>(v[3u].asDouble()));
		return Quaternion::IDENTITY;
	}

	void SetConstantAnimFloat(Animated<float>& anim, float val, size_t nAnims)
	{
		anim.used = false;
		anim.type = INTERPOLATION_NONE;
		anim.seq = -1;
		anim.times.assign(1, 0);
		anim.data.assign(1, val);
		anim.ranges.assign(std::max<size_t>(1, nAnims), AnimRange(0, 0));
	}

	void ParseAnimatedFloat(const Json::Value& jv, Animated<float>& anim, float defaultVal, size_t nAnims)
	{
		if (jv.isNull())
		{
			SetConstantAnimFloat(anim, defaultVal, nAnims);
			return;
		}
		if (jv.isNumeric())
		{
			SetConstantAnimFloat(anim, static_cast<float>(jv.asDouble()), nAnims);
			return;
		}
		if (!jv.isNull() && jv.isObject() && !jv["times"].empty() && !jv["values"].empty())
		{
			size_t n = std::min(jv["times"].size(), jv["values"].size());
			anim.used = true;
			anim.type = jv.get("interpolation", INTERPOLATION_LINEAR).asInt();
			anim.seq = jv.get("seq", -1).asInt();
			anim.times.resize(n);
			anim.data.resize(n);
			for (Json::Value::UInt k = 0; k < (Json::Value::UInt)n; ++k)
			{
				anim.times[k] = jv["times"][k].asInt();
				anim.data[k] = static_cast<float>(jv["values"][k].asDouble());
			}
			anim.ranges.clear();
			if (jv["ranges"].isArray() && !jv["ranges"].empty())
			{
				for (Json::Value::UInt r = 0; r < jv["ranges"].size(); ++r)
				{
					const auto& rv = jv["ranges"][r];
					if (rv.isArray() && rv.size() >= 2)
						anim.ranges.push_back(AnimRange(rv[0u].asUInt(), rv[1u].asUInt()));
				}
			}
			if (anim.ranges.empty())
				anim.ranges.assign(std::max<size_t>(1, nAnims), AnimRange(0, (uint32)(n - 1)));
			return;
		}
		SetConstantAnimFloat(anim, defaultVal, nAnims);
	}

	void ParseAnimatedVec3(const Json::Value& jv, Animated<Vector3>& anim, const Vector3& defaultVal, size_t nAnims)
	{
		if (!jv.isNull() && jv.isObject() && !jv["times"].empty() && !jv["values"].empty())
		{
			size_t n = std::min(jv["times"].size(), jv["values"].size());
			anim.used = true;
			anim.type = jv.get("interpolation", INTERPOLATION_LINEAR).asInt();
			anim.seq = jv.get("seq", -1).asInt();
			anim.times.resize(n);
			anim.data.resize(n);
			for (Json::Value::UInt k = 0; k < (Json::Value::UInt)n; ++k)
			{
				anim.times[k] = jv["times"][k].asInt();
				anim.data[k] = ParseVec3(jv["values"][k], defaultVal);
			}
			anim.ranges.clear();
			if (jv["ranges"].isArray() && !jv["ranges"].empty())
			{
				for (Json::Value::UInt r = 0; r < jv["ranges"].size(); ++r)
				{
					const auto& rv = jv["ranges"][r];
					if (rv.isArray() && rv.size() >= 2)
						anim.ranges.push_back(AnimRange(rv[0u].asUInt(), rv[1u].asUInt()));
				}
			}
			if (anim.ranges.empty())
				anim.ranges.assign(std::max<size_t>(1, nAnims), AnimRange(0, (uint32)(n - 1)));
			return;
		}
		Vector3 c = jv.isArray() ? ParseVec3(jv, defaultVal) : defaultVal;
		anim.used = false;
		anim.type = INTERPOLATION_NONE;
		anim.seq = -1;
		anim.times.assign(1, 0);
		anim.data.assign(1, c);
		anim.ranges.assign(std::max<size_t>(1, nAnims), AnimRange(0, 0));
	}

	void ParseAnimatedShortOpacity(const Json::Value& jv, AnimatedShort& anim, float defaultVal, size_t nAnims)
	{
		// AnimatedShort stores float data in [0..1] internally after ShortToFloat conversion
		if (!jv.isNull() && jv.isObject() && !jv["times"].empty() && !jv["values"].empty())
		{
			size_t n = std::min(jv["times"].size(), jv["values"].size());
			anim.used = true;
			anim.type = jv.get("interpolation", INTERPOLATION_LINEAR).asInt();
			anim.seq = jv.get("seq", -1).asInt();
			anim.times.resize(n);
			anim.data.resize(n);
			for (Json::Value::UInt k = 0; k < (Json::Value::UInt)n; ++k)
			{
				anim.times[k] = jv["times"][k].asInt();
				anim.data[k] = static_cast<float>(jv["values"][k].asDouble());
			}
			anim.ranges.clear();
			if (jv["ranges"].isArray() && !jv["ranges"].empty())
			{
				for (Json::Value::UInt r = 0; r < jv["ranges"].size(); ++r)
				{
					const auto& rv = jv["ranges"][r];
					if (rv.isArray() && rv.size() >= 2)
						anim.ranges.push_back(AnimRange(rv[0u].asUInt(), rv[1u].asUInt()));
				}
			}
			if (anim.ranges.empty())
				anim.ranges.assign(std::max<size_t>(1, nAnims), AnimRange(0, (uint32)(n - 1)));
			return;
		}
		float v = jv.isNumeric() ? static_cast<float>(jv.asDouble()) : defaultVal;
		anim.used = false;
		anim.type = INTERPOLATION_NONE;
		anim.seq = -1;
		anim.times.assign(1, 0);
		anim.data.assign(1, v);
		anim.ranges.assign(std::max<size_t>(1, nAnims), AnimRange(0, 0));
	}

	int ParseEmitterType(const Json::Value& v)
	{
		if (v.isIntegral())
			return v.asInt();
		if (v.isString())
		{
			std::string s = v.asString();
			if (s == "plane") return ParticleEmitter::TYPE_PLANE_PARTICLE_EMITTER;
			if (s == "sphere" || s == "cone_ring") return ParticleEmitter::TYPE_SPHERE_PARTICLE_EMITTER;
			if (s == "sphere_shell" || s == "shell") return ParticleEmitter::TYPE_SPHERE_SHELL_EMITTER;
			if (s == "pattern") return ParticleEmitter::TYPE_PATTERN_PARTICLE_EMITTER;
			if (s == "cone_jet" || s == "cone") return ParticleEmitter::TYPE_CONE_JET_EMITTER;
		}
		return ParticleEmitter::TYPE_SPHERE_SHELL_EMITTER;
	}

	int16 ParsePatternSubType(const Json::Value& v)
	{
		if (v.isIntegral())
			return (int16)v.asInt();
		if (v.isString())
		{
			std::string s = v.asString();
			if (s == "ring") return ParticleEmitter::PATTERN_RING;
			if (s == "heart") return ParticleEmitter::PATTERN_HEART;
			if (s == "star" || s == "star5") return ParticleEmitter::PATTERN_STAR5;
			if (s == "double_ring" || s == "saturn") return ParticleEmitter::PATTERN_DOUBLE_RING;
			if (s == "spiral") return ParticleEmitter::PATTERN_SPIRAL;
		}
		return 0;
	}

	int ParseBlendMode(const Json::Value& v, int fallback = 4)
	{
		if (v.isNull())
			return fallback;
		if (v.isIntegral())
			return v.asInt();
		if (v.isString())
		{
			std::string s = v.asString();
			if (s == "opaque") return 0;
			if (s == "additive_one") return 1;
			if (s == "alpha" || s == "alpha_blend") return 2;
			if (s == "transparent") return 3;
			if (s == "additive" || s == "add" || s == "additive_alpha" || s == "add_alpha") return 4;
		}
		return fallback;
	}

	int FindOrAppendTexture(CParaXModel* pMesh, const Json::Value& texVal, const std::vector<std::string>& extraTexList)
	{
		std::string texPath;
		if (texVal.isIntegral())
		{
			int idx = texVal.asInt();
			if (idx >= 0 && idx < (int)extraTexList.size())
				texPath = extraTexList[idx];
			else if (idx >= 0 && idx < (int)pMesh->m_objNum.nTextures)
				return idx;
		}
		else if (texVal.isString())
		{
			texPath = texVal.asString();
		}
		if (texPath.empty())
			texPath = "builtin://glow";

		for (uint32 i = 0; i < pMesh->m_objNum.nTextures; ++i)
		{
			if (pMesh->textures[i] && pMesh->textures[i]->GetKey() == texPath)
				return (int)i;
		}

		uint32 oldCount = pMesh->m_objNum.nTextures;
		auto* newArr = new asset_ptr<TextureEntity>[oldCount + 1];
		for (uint32 i = 0; i < oldCount; ++i)
			newArr[i] = pMesh->textures[i];
		if (IsBuiltinParticleTextureName(texPath))
			newArr[oldCount] = LoadOrCreateBuiltinParticleTexture(texPath);
		else if (CGlobals::GetAssetManager())
			newArr[oldCount] = CGlobals::GetAssetManager()->LoadTexture("", texPath.c_str(), TextureEntity::StaticTexture);

		SAFE_DELETE_ARRAY(pMesh->textures);
		pMesh->textures = newArr;
		pMesh->m_objNum.nTextures = oldCount + 1;
		return (int)oldCount;
	}

	int ResolveBoneIndex(CParaXModel* pMesh, const Json::Value& boneVal)
	{
		if (pMesh->m_objNum.nBones == 0 || !pMesh->bones)
			return 0;
		if (boneVal.isIntegral())
		{
			int idx = boneVal.asInt();
			if (idx >= 0 && idx < (int)pMesh->m_objNum.nBones)
				return idx;
		}
		else if (boneVal.isString())
		{
			std::string target = boneVal.asString();
			for (uint32 i = 0; i < pMesh->m_objNum.nBones; ++i)
			{
				if (pMesh->bones[i].GetName() == target)
					return (int)i;
			}
		}
		return 0;
	}

	Json::Value ExportAnimatedFloatJson(const Animated<float>& anim)
	{
		if (!anim.used && anim.data.size() <= 1)
			return Json::Value(anim.data.empty() ? 0.0f : anim.data[0]);

		Json::Value jv;
		jv["interpolation"] = anim.type;
		jv["seq"] = anim.seq;
		Json::Value times(Json::arrayValue), vals(Json::arrayValue), ranges(Json::arrayValue);
		for (size_t i = 0; i < anim.times.size(); ++i)
			times[(Json::Value::UInt)i] = anim.times[i];
		for (size_t i = 0; i < anim.data.size(); ++i)
			vals[(Json::Value::UInt)i] = anim.data[i];
		for (size_t i = 0; i < anim.ranges.size(); ++i)
		{
			Json::Value r(Json::arrayValue);
			r[0u] = static_cast<Json::Value::UInt>(anim.ranges[i].first);
			r[1u] = static_cast<Json::Value::UInt>(anim.ranges[i].second);
			ranges[(Json::Value::UInt)i] = r;
		}
		jv["times"] = times;
		jv["values"] = vals;
		jv["ranges"] = ranges;
		return jv;
	}

	Json::Value Vec3ToJson(const Vector3& v)
	{
		Json::Value a(Json::arrayValue);
		a[0u] = v.x; a[1u] = v.y; a[2u] = v.z;
		return a;
	}

	Json::Value Vec4ToJson(const Vector4& v)
	{
		Json::Value a(Json::arrayValue);
		a[0u] = v.x; a[1u] = v.y; a[2u] = v.z; a[3u] = v.w;
		return a;
	}
}

bool ParaXGltfExtras::ExtractGltfJson(const char* buffer, int nSize, Json::Value& outRoot)
{
	if (!buffer || nSize < 4)
		return false;

	const char* jsonPtr = buffer;
	int jsonLen = nSize;

	// Check binary GLB magic 0x46546C67 ("glTF")
	if (nSize >= 20)
	{
		uint32_t magic = 0;
		memcpy(&magic, buffer, sizeof(uint32_t));
		if (magic == 0x46546C67u)
		{
			uint32_t chunkLen = 0, chunkType = 0;
			memcpy(&chunkLen, buffer + 12, sizeof(uint32_t));
			memcpy(&chunkType, buffer + 16, sizeof(uint32_t));
			if (chunkType != GLTF_JSON || 20 + (int)chunkLen > nSize)
				return false;
			jsonPtr = buffer + 20;
			jsonLen = (int)chunkLen;
		}
	}

	Json::Reader reader;
	return reader.parse(jsonPtr, jsonPtr + jsonLen, outRoot, false);
}

bool ParaXGltfExtras::ApplyExtrasToModel(const Json::Value& gltfRoot, CParaXModel* pMesh)
{
	if (!pMesh || !gltfRoot.isObject())
		return false;

	const bool hasRootParax = gltfRoot.isMember("extras")
		&& !gltfRoot["extras"].isNull()
		&& gltfRoot["extras"].isObject()
		&& gltfRoot["extras"].isMember("parax")
		&& !gltfRoot["extras"]["parax"].isNull()
		&& gltfRoot["extras"]["parax"].isObject();
	const Json::Value& rootParax = hasRootParax ? gltfRoot["extras"]["parax"] : Json::Value::null;

	// Collect any per-node particleEmitter / ribbonEmitter in gltfRoot["nodes"]
	// Note: in vendored jsoncpp 0.5, nullValue::isObject() and nullValue::isArray() return true,
	// so always guard optional keys with isMember() / !isNull().
	Json::Value mergedPE(Json::arrayValue);
	Json::Value mergedRE(Json::arrayValue);
	if (hasRootParax)
	{
		if (rootParax.isMember("particleEmitters") && !rootParax["particleEmitters"].isNull() && rootParax["particleEmitters"].isArray())
			mergedPE = rootParax["particleEmitters"];
		if (rootParax.isMember("ribbonEmitters") && !rootParax["ribbonEmitters"].isNull() && rootParax["ribbonEmitters"].isArray())
			mergedRE = rootParax["ribbonEmitters"];
	}
	if (gltfRoot.isMember("nodes") && !gltfRoot["nodes"].isNull() && gltfRoot["nodes"].isArray())
	{
		for (Json::Value::UInt ni = 0; ni < gltfRoot["nodes"].size(); ++ni)
		{
			const auto& node = gltfRoot["nodes"][ni];
			if (!node.isMember("extras") || node["extras"].isNull() || !node["extras"].isObject())
				continue;
			const auto& nodeExtras = node["extras"];
			std::string nodeName = node.get("name", "").asString();
			if (nodeExtras.isMember("particleEmitter") && !nodeExtras["particleEmitter"].isNull() && nodeExtras["particleEmitter"].isObject())
			{
				Json::Value pe = nodeExtras["particleEmitter"];
				if (pe["bone"].isNull())
					pe["bone"] = nodeName.empty() ? Json::Value((int)ni) : Json::Value(nodeName);
				mergedPE.append(pe);
			}
			if (nodeExtras.isMember("ribbonEmitter") && !nodeExtras["ribbonEmitter"].isNull() && nodeExtras["ribbonEmitter"].isObject())
			{
				Json::Value re = nodeExtras["ribbonEmitter"];
				if (re["bone"].isNull())
					re["bone"] = nodeName.empty() ? Json::Value((int)ni) : Json::Value(nodeName);
				mergedRE.append(re);
			}
		}
	}

	if (!hasRootParax && mergedPE.empty() && mergedRE.empty())
		return false;

	// Optional bounding box override
	if (hasRootParax)
	{
		if (rootParax.isMember("minExtent") && !rootParax["minExtent"].empty())
			pMesh->m_header.minExtent = ParseVec3(rootParax["minExtent"], pMesh->m_header.minExtent);
		if (rootParax.isMember("maxExtent") && !rootParax["maxExtent"].empty())
			pMesh->m_header.maxExtent = ParseVec3(rootParax["maxExtent"], pMesh->m_header.maxExtent);
		pMesh->m_radius = (pMesh->m_header.maxExtent - pMesh->m_header.minExtent).length() * 0.5f;
		if (pMesh->m_radius < 1.0f)
			pMesh->m_radius = 1.0f;
	}

	// Ensure at least 1 root bone exists
	if (pMesh->m_objNum.nBones == 0 || pMesh->bones == nullptr)
	{
		if (hasRootParax && rootParax.isMember("bones") && !rootParax["bones"].empty())
		{
			uint32 nb = rootParax["bones"].size();
			pMesh->m_objNum.nBones = nb;
			pMesh->bones = new Bone[nb];
			size_t nAnimHint = (rootParax["animations"].isArray() && !rootParax["animations"].empty())
				? rootParax["animations"].size() : std::max<size_t>(1, pMesh->m_objNum.nAnimations);
			for (Json::Value::UInt bi = 0; bi < nb; ++bi)
			{
				const auto& jb = rootParax["bones"][bi];
				Bone& b = pMesh->bones[bi];
				b.nIndex = (int)bi;
				b.parent = jb.get("parent", -1).asInt();
				b.flags = 0;
				b.bUsePivot = true;
				b.pivot = ParseVec3(jb["pivot"], Vector3(0, 0, 0));
				if (jb["name"].isString())
					b.SetName(jb["name"].asString());
				ParseAnimatedVec3(jb["trans"], b.trans, Vector3(0, 0, 0), nAnimHint);
				if (jb["rot"].isObject() && jb["rot"]["times"].isArray() && jb["rot"]["values"].isArray() && !jb["rot"]["times"].empty())
				{
					size_t nk = std::min(jb["rot"]["times"].size(), jb["rot"]["values"].size());
					b.rot.used = true;
					b.rot.type = INTERPOLATION_LINEAR;
					b.rot.seq = -1;
					b.rot.times.resize(nk);
					b.rot.data.resize(nk);
					for (Json::Value::UInt k = 0; k < (Json::Value::UInt)nk; ++k)
					{
						b.rot.times[k] = jb["rot"]["times"][k].asInt();
						b.rot.data[k] = ParseQuat(jb["rot"]["values"][k]);
					}
					b.rot.ranges.assign(nAnimHint, AnimRange(0, (uint32)(nk - 1)));
				}
				ParseAnimatedVec3(jb["scale"], b.scale, Vector3(1, 1, 1), nAnimHint);
			}
			pMesh->animBones = true;
		}
		else
		{
			pMesh->m_objNum.nBones = 1;
			pMesh->bones = new Bone[1];
			pMesh->bones[0].nIndex = 0;
			pMesh->bones[0].parent = -1;
			pMesh->bones[0].pivot = Vector3(0, 0, 0);
			pMesh->bones[0].SetName("root");
		}
	}

	// Optional animations list in extras.parax (or ensure at least 1 animation exists)
	if (hasRootParax && rootParax.isMember("animations") && !rootParax["animations"].empty())
	{
		SAFE_DELETE_ARRAY(pMesh->anims);
		uint32 na = rootParax["animations"].size();
		pMesh->m_objNum.nAnimations = na;
		pMesh->anims = new ModelAnimation[na];
		memset(pMesh->anims, 0, sizeof(ModelAnimation) * na);
		for (Json::Value::UInt ai = 0; ai < na; ++ai)
		{
			const auto& ja = rootParax["animations"][ai];
			ModelAnimation& ma = pMesh->anims[ai];
			ma.animID = ja.get("id", (int)ai).asUInt();
			ma.timeStart = ja.get("timeStart", 0).asUInt();
			ma.timeEnd = ja.get("timeEnd", 3000).asUInt();
			ma.loopType = ja.get("loopType", 0).asUInt();
			ma.moveSpeed = static_cast<float>(ja.get("moveSpeed", 0.0).asDouble());
			ma.boxA = ParseVec3(ja["boxA"], pMesh->m_header.minExtent);
			ma.boxB = ParseVec3(ja["boxB"], pMesh->m_header.maxExtent);
			ma.rad = static_cast<float>(ja.get("rad", pMesh->m_radius).asDouble());
		}
	}
	else if (pMesh->m_objNum.nAnimations == 0 || pMesh->anims == nullptr)
	{
		pMesh->m_objNum.nAnimations = 1;
		pMesh->anims = new ModelAnimation[1];
		memset(pMesh->anims, 0, sizeof(ModelAnimation));
		pMesh->anims[0].animID = 0;
		pMesh->anims[0].timeStart = 0;
		pMesh->anims[0].timeEnd = 3000;
		pMesh->anims[0].loopType = 0;
		pMesh->anims[0].boxA = pMesh->m_header.minExtent;
		pMesh->anims[0].boxB = pMesh->m_header.maxExtent;
		pMesh->anims[0].rad = std::max(2.0f, pMesh->m_radius);
	}

	std::vector<std::string> extraTexList;
	if (hasRootParax && rootParax.isMember("textures") && !rootParax["textures"].empty())
	{
		for (Json::Value::UInt ti = 0; ti < rootParax["textures"].size(); ++ti)
			extraTexList.push_back(rootParax["textures"][ti].asString());
	}

	const size_t nAnims = std::max<size_t>(1, pMesh->m_objNum.nAnimations);

	if (!mergedPE.empty())
	{
		SAFE_DELETE_ARRAY(pMesh->particleSystems);
		uint32 nPE = mergedPE.size();
		pMesh->m_objNum.nParticleEmitters = nPE;
		pMesh->particleSystems = new ParticleSystem[nPE];

		for (Json::Value::UInt i = 0; i < nPE; ++i)
		{
			const auto& jpe = mergedPE[i];
			ParticleSystem& ps = pMesh->particleSystems[i];
			ps.model = pMesh;
			ps.m_bExtendedV2 = true;
			ps.pos = ParseVec3(jpe["pos"], Vector3(0, 0, 0));
			ps.blend = ParseBlendMode(jpe["blend"], 4);
			ps.type = jpe.get("renderType", 0).asInt();
			ps.patternType = ParsePatternSubType(jpe["patternType"]);
			ps.rows = std::max(1, jpe.get("rows", 1).asInt());
			ps.cols = std::max(1, jpe.get("cols", 1).asInt());
			ps.order = 0;
			ps.SetTextureRowsCols(ps.rows, ps.cols);

			ps.billboard = jpe.get("billboard", true).asBool();
			ps.useWorldGravity = jpe.get("useWorldGravity", true).asBool();
			ps.localSpace = jpe.get("localSpace", false).asBool();
			ps.rotate2SpeedDirection = jpe.get("rotate2SpeedDirection", false).asBool();

			ps.mid = static_cast<float>(jpe.get("mid", 0.5).asDouble());
			ps.slowdown = static_cast<float>(jpe.get("slowdown", 0.0).asDouble());
			ps.rotation = static_cast<float>(jpe.get("rotation", 0.0).asDouble());

			if (jpe["colors"].isArray() && jpe["colors"].size() >= 3)
			{
				for (int c = 0; c < 3; ++c)
					ps.colors[c] = ParseColorValue(jpe["colors"][(Json::Value::UInt)c], ps.colors[c]);
			}
			if (jpe["sizes"].isArray() && jpe["sizes"].size() >= 3)
			{
				for (int s = 0; s < 3; ++s)
					ps.sizes[s] = static_cast<float>(jpe["sizes"][(Json::Value::UInt)s].asDouble());
			}

			int boneIdx = ResolveBoneIndex(pMesh, jpe["bone"]);
			ps.parent = pMesh->bones + boneIdx;
			ps.m_texture_index = FindOrAppendTexture(pMesh, jpe["texture"], extraTexList);

			ParseAnimatedFloat(jpe["speed"], ps.speed, 4.0f, nAnims);
			ParseAnimatedFloat(jpe["variation"], ps.variation, 0.2f, nAnims);
			ParseAnimatedFloat(jpe["spread"], ps.spread, 0.0f, nAnims);
			ParseAnimatedFloat(jpe["lat"], ps.lat, 0.0f, nAnims);
			ParseAnimatedFloat(jpe["gravity"], ps.gravity, 4.0f, nAnims);
			ParseAnimatedFloat(jpe["lifespan"], ps.lifespan, 1.2f, nAnims);
			ParseAnimatedFloat(jpe["rate"], ps.rate, 0.0f, nAnims);
			ParseAnimatedFloat(jpe["areal"], ps.areal, 0.0f, nAnims);
			ParseAnimatedFloat(jpe["areaw"], ps.areaw, 0.0f, nAnims);
			ParseAnimatedFloat(jpe["grav2"], ps.grav2, 0.0f, nAnims);

			ps.emitDir = ParseVec3(jpe["emitDir"], Vector3(0, 1, 0));
			ps.minPolar = static_cast<float>(jpe.get("minPolar", 0.0).asDouble());
			ps.maxPolar = static_cast<float>(jpe.get("maxPolar", 3.14159265).asDouble());
			ps.shellThickness = static_cast<float>(jpe.get("shellThickness", 0.0).asDouble());
			ps.trailEmitterIndex = (int16)jpe.get("trailEmitterIndex", -1).asInt();
			ps.trailInterval = static_cast<float>(jpe.get("trailInterval", 0.03).asDouble());
			ps.deathEmitterIndex = (int16)jpe.get("deathEmitterIndex", -1).asInt();
			ps.deathCount = (int16)jpe.get("deathCount", 0).asInt();
			ps.deathCountVar = (int16)jpe.get("deathCountVar", 0).asInt();
			ps.inheritVelocity = static_cast<float>(jpe.get("inheritVelocity", 0.0).asDouble());
			ps.twinkleFreq = static_cast<float>(jpe.get("twinkleFreq", 0.0).asDouble());
			ps.twinkleAmount = static_cast<float>(jpe.get("twinkleAmount", 0.0).asDouble());

			if (!jpe["bursts"].empty() && jpe["bursts"].isArray())
			{
				for (Json::Value::UInt bi = 0; bi < jpe["bursts"].size(); ++bi)
				{
					const auto& jb = jpe["bursts"][bi];
					ParticleBurst b;
					b.animIndex = jb.get("animIndex", -1).asInt();
					b.timeMs = jb.get("timeMs", 0).asInt();
					b.count = jb.get("count", 0).asInt();
					b.countVar = jb.get("countVar", 0).asInt();
					ps.bursts.push_back(b);
				}
			}

			if (!jpe["palette"].empty() && jpe["palette"].isArray())
			{
				for (Json::Value::UInt pi = 0; pi < jpe["palette"].size(); ++pi)
					ps.palette.push_back(ParseColorValue(jpe["palette"][pi]));
			}

			int etype = ParseEmitterType(jpe["type"]);
			ps.emitter = ParticleSystem::CreateEmitterByType(&ps, etype);
			ps.tofs = frand();
		}
	}

	if (!mergedRE.empty())
	{
		SAFE_DELETE_ARRAY(pMesh->ribbons);
		uint32 nRE = mergedRE.size();
		pMesh->m_objNum.nRibbonEmitters = nRE;
		pMesh->ribbons = new RibbonEmitter[nRE];

		for (Json::Value::UInt i = 0; i < nRE; ++i)
		{
			const auto& jre = mergedRE[i];
			RibbonEmitter& re = pMesh->ribbons[i];
			re.model = pMesh;
			re.m_bExtendedV2 = true;
			re.pos = ParseVec3(jre["pos"], Vector3(0, 0, 0));
			re.numsegs = std::max(2, jre.get("numsegs", 20).asInt());
			re.seglen = std::max(0.01f, static_cast<float>(jre.get("seglen", 0.2).asDouble()));
			re.length = re.seglen * (float)re.numsegs;
			re.maxLife = static_cast<float>(jre.get("maxLife", 0.6).asDouble());
			re.blend = ParseBlendMode(jre["blend"], 4);
			re.orientCamera = jre.get("orientCamera", true).asBool();
			re.m_bUseAbsCord = !jre.get("localSpace", false).asBool();
			re.widthEndScale = static_cast<float>(jre.get("widthEndScale", 0.0).asDouble());
			re.maxStepDist = static_cast<float>(jre.get("maxStepDist", 8.0).asDouble());
			if (!jre["colorEnd"].isNull())
			{
				re.hasEndColor = true;
				re.colorEnd = ParseColorValue(jre["colorEnd"], Vector4(1, 1, 1, 0));
			}

			int boneIdx = ResolveBoneIndex(pMesh, jre["bone"]);
			re.parent = pMesh->bones + boneIdx;
			re.m_texture_index = FindOrAppendTexture(pMesh, jre["texture"], extraTexList);

			ParseAnimatedVec3(jre["color"], re.color, Vector3(1, 1, 1), nAnims);
			ParseAnimatedShortOpacity(jre["opacity"], re.opacity, 1.0f, nAnims);
			ParseAnimatedFloat(jre["above"], re.above, 0.12f, nAnims);
			ParseAnimatedFloat(jre["below"], re.below, 0.12f, nAnims);
		}
	}

	if (pMesh->m_objNum.nParticleEmitters > 0 || pMesh->m_objNum.nRibbonEmitters > 0)
	{
		pMesh->animated = true;
		pMesh->animBones = pMesh->m_objNum.nBones > 0;
		pMesh->m_RenderMethod = CParaXModel::SOFT_ANIM;
	}
	return true;
}

bool ParaXGltfExtras::ApplyExtrasFromBuffer(const char* buffer, int nSize, CParaXModel* pMesh)
{
	Json::Value root;
	if (!ExtractGltfJson(buffer, nSize, root))
		return false;
	return ApplyExtrasToModel(root, pMesh);
}

CParaXModel* ParaXGltfExtras::ParseStandaloneModelFromBuffer(const char* buffer, int nSize)
{
	Json::Value root;
	if (!ExtractGltfJson(buffer, nSize, root))
		return nullptr;

	ParaXHeaderDef xheader;
	xheader.IsAnimated = (1 << 2) | (1 << 3);
	xheader.minExtent = Vector3(-4, 0, -4);
	xheader.maxExtent = Vector3(4, 16, 4);
	CParaXModel* pMesh = new CParaXModel(xheader);
	if (!ApplyExtrasToModel(root, pMesh))
	{
		SAFE_DELETE(pMesh);
		return nullptr;
	}
	return pMesh;
}

bool ParaXGltfExtras::ExportExtrasFromModel(CParaXModel* pMesh, Json::Value& outParaxExtras)
{
	if (!pMesh || (pMesh->m_objNum.nParticleEmitters == 0 && pMesh->m_objNum.nRibbonEmitters == 0))
		return false;

	outParaxExtras = Json::Value(Json::objectValue);
	outParaxExtras["minExtent"] = Vec3ToJson(pMesh->m_header.minExtent);
	outParaxExtras["maxExtent"] = Vec3ToJson(pMesh->m_header.maxExtent);

	Json::Value jTexs(Json::arrayValue);
	for (uint32 i = 0; i < pMesh->m_objNum.nTextures; ++i)
	{
		jTexs[(Json::Value::UInt)i] = (pMesh->textures && pMesh->textures[i]) ? pMesh->textures[i]->GetKey() : "builtin://glow";
	}
	outParaxExtras["textures"] = jTexs;

	Json::Value jAnims(Json::arrayValue);
	for (uint32 i = 0; i < pMesh->m_objNum.nAnimations; ++i)
	{
		const ModelAnimation& ma = pMesh->anims[i];
		Json::Value ja;
		ja["id"] = static_cast<Json::Value::UInt>(ma.animID);
		ja["timeStart"] = static_cast<Json::Value::UInt>(ma.timeStart);
		ja["timeEnd"] = static_cast<Json::Value::UInt>(ma.timeEnd);
		ja["loopType"] = static_cast<Json::Value::UInt>(ma.loopType);
		ja["moveSpeed"] = ma.moveSpeed;
		ja["boxA"] = Vec3ToJson(ma.boxA);
		ja["boxB"] = Vec3ToJson(ma.boxB);
		ja["rad"] = ma.rad;
		jAnims[(Json::Value::UInt)i] = ja;
	}
	outParaxExtras["animations"] = jAnims;

	if (pMesh->m_objNum.nBones > 0 && pMesh->bones)
	{
		Json::Value jBones(Json::arrayValue);
		for (uint32 i = 0; i < pMesh->m_objNum.nBones; ++i)
		{
			const Bone& b = pMesh->bones[i];
			Json::Value jb;
			jb["name"] = b.GetName().empty() ? ("bone_" + std::to_string(i)) : b.GetName();
			jb["parent"] = b.parent;
			jb["pivot"] = Vec3ToJson(b.pivot);
			if (b.trans.used && !b.trans.times.empty())
			{
				Json::Value jt, times(Json::arrayValue), vals(Json::arrayValue);
				for (size_t k = 0; k < b.trans.times.size(); ++k) times[(Json::Value::UInt)k] = b.trans.times[k];
				for (size_t k = 0; k < b.trans.data.size(); ++k) vals[(Json::Value::UInt)k] = Vec3ToJson(b.trans.data[k]);
				jt["times"] = times; jt["values"] = vals;
				jb["trans"] = jt;
			}
			if (b.rot.used && !b.rot.times.empty())
			{
				Json::Value jr, times(Json::arrayValue), vals(Json::arrayValue);
				for (size_t k = 0; k < b.rot.times.size(); ++k) times[(Json::Value::UInt)k] = b.rot.times[k];
				for (size_t k = 0; k < b.rot.data.size(); ++k)
				{
					Json::Value q(Json::arrayValue);
					q[0u] = b.rot.data[k].x; q[1u] = b.rot.data[k].y; q[2u] = b.rot.data[k].z; q[3u] = b.rot.data[k].w;
					vals[(Json::Value::UInt)k] = q;
				}
				jr["times"] = times; jr["values"] = vals;
				jb["rot"] = jr;
			}
			jBones[(Json::Value::UInt)i] = jb;
		}
		outParaxExtras["bones"] = jBones;
	}

	if (pMesh->m_objNum.nParticleEmitters > 0)
	{
		Json::Value jPEs(Json::arrayValue);
		for (uint32 i = 0; i < pMesh->m_objNum.nParticleEmitters; ++i)
		{
			const ParticleSystem& ps = pMesh->particleSystems[i];
			Json::Value jpe;
			int boneIdx = (ps.parent && pMesh->bones) ? (int)(ps.parent - pMesh->bones) : 0;
			jpe["bone"] = boneIdx;
			jpe["texture"] = ps.m_texture_index;
			jpe["pos"] = Vec3ToJson(ps.pos);
			jpe["blend"] = ps.blend;
			jpe["type"] = ps.emitter ? ps.emitter->GetEmitterType() : 1;
			jpe["renderType"] = ps.type;
			jpe["patternType"] = ps.patternType;
			jpe["rows"] = ps.rows;
			jpe["cols"] = ps.cols;
			jpe["billboard"] = ps.billboard;
			jpe["useWorldGravity"] = ps.useWorldGravity;
			jpe["localSpace"] = ps.localSpace;
			jpe["rotate2SpeedDirection"] = ps.rotate2SpeedDirection;
			jpe["mid"] = ps.mid;
			jpe["slowdown"] = ps.slowdown;
			jpe["rotation"] = ps.rotation;

			Json::Value cols(Json::arrayValue), szs(Json::arrayValue);
			for (int k = 0; k < 3; ++k)
			{
				cols[(Json::Value::UInt)k] = Vec4ToJson(ps.colors[k]);
				szs[(Json::Value::UInt)k] = ps.sizes[k];
			}
			jpe["colors"] = cols;
			jpe["sizes"] = szs;

			jpe["speed"] = ExportAnimatedFloatJson(ps.speed);
			jpe["variation"] = ExportAnimatedFloatJson(ps.variation);
			jpe["spread"] = ExportAnimatedFloatJson(ps.spread);
			jpe["lat"] = ExportAnimatedFloatJson(ps.lat);
			jpe["gravity"] = ExportAnimatedFloatJson(ps.gravity);
			jpe["lifespan"] = ExportAnimatedFloatJson(ps.lifespan);
			jpe["rate"] = ExportAnimatedFloatJson(ps.rate);
			jpe["areal"] = ExportAnimatedFloatJson(ps.areal);
			jpe["areaw"] = ExportAnimatedFloatJson(ps.areaw);
			jpe["grav2"] = ExportAnimatedFloatJson(ps.grav2);

			jpe["emitDir"] = Vec3ToJson(ps.emitDir);
			jpe["minPolar"] = ps.minPolar;
			jpe["maxPolar"] = ps.maxPolar;
			jpe["shellThickness"] = ps.shellThickness;
			jpe["trailEmitterIndex"] = ps.trailEmitterIndex;
			jpe["trailInterval"] = ps.trailInterval;
			jpe["deathEmitterIndex"] = ps.deathEmitterIndex;
			jpe["deathCount"] = ps.deathCount;
			jpe["deathCountVar"] = ps.deathCountVar;
			jpe["inheritVelocity"] = ps.inheritVelocity;
			jpe["twinkleFreq"] = ps.twinkleFreq;
			jpe["twinkleAmount"] = ps.twinkleAmount;

			if (!ps.bursts.empty())
			{
				Json::Value jbs(Json::arrayValue);
				for (size_t bi = 0; bi < ps.bursts.size(); ++bi)
				{
					Json::Value jb;
					jb["animIndex"] = ps.bursts[bi].animIndex;
					jb["timeMs"] = ps.bursts[bi].timeMs;
					jb["count"] = ps.bursts[bi].count;
					jb["countVar"] = ps.bursts[bi].countVar;
					jbs[(Json::Value::UInt)bi] = jb;
				}
				jpe["bursts"] = jbs;
			}
			if (!ps.palette.empty())
			{
				Json::Value jpal(Json::arrayValue);
				for (size_t pi = 0; pi < ps.palette.size(); ++pi)
					jpal[(Json::Value::UInt)pi] = Vec4ToJson(ps.palette[pi]);
				jpe["palette"] = jpal;
			}
			jPEs[(Json::Value::UInt)i] = jpe;
		}
		outParaxExtras["particleEmitters"] = jPEs;
	}

	if (pMesh->m_objNum.nRibbonEmitters > 0)
	{
		Json::Value jREs(Json::arrayValue);
		for (uint32 i = 0; i < pMesh->m_objNum.nRibbonEmitters; ++i)
		{
			const RibbonEmitter& re = pMesh->ribbons[i];
			Json::Value jre;
			int boneIdx = (re.parent && pMesh->bones) ? (int)(re.parent - pMesh->bones) : 0;
			jre["bone"] = boneIdx;
			jre["texture"] = re.m_texture_index;
			jre["pos"] = Vec3ToJson(re.pos);
			jre["numsegs"] = re.numsegs;
			jre["seglen"] = re.seglen;
			jre["maxLife"] = re.maxLife;
			jre["blend"] = re.blend;
			jre["orientCamera"] = re.orientCamera;
			jre["localSpace"] = !re.m_bUseAbsCord;
			jre["widthEndScale"] = re.widthEndScale;
			jre["maxStepDist"] = re.maxStepDist;
			if (re.hasEndColor)
				jre["colorEnd"] = Vec4ToJson(re.colorEnd);
			if (!re.color.data.empty())
				jre["color"] = Vec3ToJson(re.color.data[0]);
			jre["opacity"] = re.opacity.data.empty() ? 1.0f : re.opacity.data[0];
			jre["above"] = ExportAnimatedFloatJson(re.above);
			jre["below"] = ExportAnimatedFloatJson(re.below);
			jREs[(Json::Value::UInt)i] = jre;
		}
		outParaxExtras["ribbonEmitters"] = jREs;
	}
	return true;
}
