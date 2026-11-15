#pragma once

#include "FxShaderTypes.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iomanip>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "hlsl2glsl.h"
#include "intermediate.h"

namespace ParaEngine
{
	namespace AstTextGenDetail
	{
		enum class ETextLang
		{
			MSL_2_0,
			WGSL_1_0
		};

		enum class ETexDim
		{
			Dim2D = 0,
			Dim3D = 1,
			DimCube = 2
		};

		inline std::string ToUpperAscii(const std::string& s)
		{
			std::string r = s;
			for (char& c : r)
				c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
			return r;
		}

		inline uint32_t AlignUpU32(uint32_t val, uint32_t align)
		{
			return (val + align - 1u) & ~(align - 1u);
		}

		inline void ParseSemanticAndIdx(const std::string& rawSem, std::string& outBaseUpper, uint32_t& outIndex)
		{
			outBaseUpper.clear();
			outIndex = 0;
			if (rawSem.empty())
				return;

			std::string s = ToUpperAscii(rawSem);
			size_t colon = s.find(':');
			if (colon != std::string::npos)
				s = s.substr(0, colon);
			while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
				s.pop_back();
			size_t b = 0;
			while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b])))
				++b;
			s = s.substr(b);

			size_t dpos = s.size();
			while (dpos > 0 && std::isdigit(static_cast<unsigned char>(s[dpos - 1])))
				--dpos;
			if (dpos < s.size())
			{
				outIndex = static_cast<uint32_t>(std::stoul(s.substr(dpos)));
				s.resize(dpos);
			}
			outBaseUpper = s;
		}

		inline int32_t MapVaryingLoc(const std::string& baseUpper, uint32_t index)
		{
			if (baseUpper == "COLOR")
				return (index == 0) ? 0 : 1;
			if (baseUpper == "TEXCOORD")
				return (index <= 8) ? static_cast<int32_t>(2 + index) : 10;
			if (baseUpper == "NORMAL")
				return 11;
			if (baseUpper == "TANGENT")
				return 12;
			if (baseUpper == "BINORMAL")
				return 13;
			return static_cast<int32_t>(14 + (index & 1u));
		}

		// Sanitize HLSL identifiers for both MSL C++14 and WGSL 1.0 (avoid reserved words & leading '__')
		inline std::string SanitizeIdent(const std::string& raw, const char* prefix = "id_")
		{
			std::string out;
			out.reserve(raw.size() + 6);
			for (char c : raw)
			{
				if (std::isalnum(static_cast<unsigned char>(c)) || c == '_')
					out.push_back(c);
				else
					out.push_back('_');
			}
			while (out.size() >= 2 && out[0] == '_' && out[1] == '_')
				out.erase(out.begin());
			if (out.empty() || std::isdigit(static_cast<unsigned char>(out[0])) || out[0] == '_')
				out = std::string(prefix) + out;

			// Prefix with short tag so user identifiers never collide with WGSL/MSL keywords
			// (e.g. target, sample, filter, texture, sampler, fragment, vertex, metal, modf, etc.)
			return std::string(prefix) + out;
		}

		inline std::string FormatFloatLit(float v, ETextLang lang)
		{
			if (!std::isfinite(v))
				v = 0.0f;
			if (v == 0.0f)
				v = 0.0f;
			char buf[64];
			std::snprintf(buf, sizeof(buf), "%.8e", static_cast<double>(v));
			std::string s(buf);
			if (lang == ETextLang::MSL_2_0)
				s.push_back('f');
			return s;
		}

		inline std::string FormatIntLit(int32_t v, ETextLang lang)
		{
			if (v == (-2147483647 - 1))
				return (lang == ETextLang::WGSL_1_0) ? "(-2147483647i - 1i)" : "(-2147483647 - 1)";
			std::string s = std::to_string(v);
			if (lang == ETextLang::WGSL_1_0)
				s.push_back('i');
			return s;
		}

		struct SingleNodeClassifier : public TIntermTraverser
		{
			TIntermUnary* unaryNode = nullptr;
			TIntermLoop* loopNode = nullptr;
			TIntermBranch* branchNode = nullptr;

			SingleNodeClassifier()
			{
				preVisit = true;
				postVisit = false;
				visitUnary = [](bool, TIntermUnary* n, TIntermTraverser* it) -> bool {
					static_cast<SingleNodeClassifier*>(it)->unaryNode = n;
					return false;
				};
				visitLoop = [](bool, TIntermLoop* n, TIntermTraverser* it) -> bool {
					static_cast<SingleNodeClassifier*>(it)->loopNode = n;
					return false;
				};
				visitBranch = [](bool, TIntermBranch* n, TIntermTraverser* it) -> bool {
					static_cast<SingleNodeClassifier*>(it)->branchNode = n;
					return false;
				};
				visitBinary = [](bool, TIntermBinary*, TIntermTraverser*) -> bool { return false; };
				visitAggregate = [](bool, TIntermAggregate*, TIntermTraverser*) -> bool { return false; };
				visitSelection = [](bool, TIntermSelection*, TIntermTraverser*) -> bool { return false; };
				visitDeclaration = [](bool, TIntermDeclaration*, TIntermTraverser*) -> bool { return false; };
			}
		};

		struct StructuredTextModuleBuilder
		{
			ETextLang lang;
			bool isVertex = true;
			std::string entryPointName;
			const std::vector<FxSamplerDesc>& declaredSamplers;
			const std::vector<FxUniformParam>& declaredParams;
			std::string errorMsg;

			uint32_t nextTmpId = 1;

			struct StructInfo
			{
				std::string typeName;
				std::vector<std::string> fieldNames;
				std::vector<TType> fieldTypes;
				std::vector<uint32_t> uniformFieldByteOffsets;
				uint32_t uniformByteSize = 16;
			};
			std::unordered_map<const TTypeList*, StructInfo> structMap;
			std::vector<const TTypeList*> structOrder;

			struct UniformFieldInfo
			{
				int symId = -1;
				std::string rawName;
				std::string varName;
				std::string semantic;
				TType astType;
				uint32_t byteOffset = 0;
				uint32_t byteSize = 0;
				bool isMutableCopy = false;
			};
			uint32_t globalsByteSize = 0;
			uint32_t globalsVec4Count = 1;
			std::vector<UniformFieldInfo> uniformFields;
			std::unordered_map<int, size_t> symIdToUniformIdx;

			struct SamplerResourceInfo
			{
				int symId = -1;
				std::string rawName;
				uint32_t slot = 0;
				ETexDim dim = ETexDim::Dim2D;
				std::string texIdent;
				std::string smpIdent;
				bool isParamRef = false;
			};
			std::unordered_map<int, SamplerResourceInfo> samplerBySymId;
			std::map<uint32_t, SamplerResourceInfo> samplerBySlot;
			uint32_t nextAutoSamplerSlot = 0;

			struct GlobalVarInfo
			{
				int symId = -1;
				std::string varName;
				TType astType;
				TIntermTyped* initExpr = nullptr;
				bool containsArrayInit = false;
			};
			std::unordered_map<int, GlobalVarInfo> privateGlobals;
			std::vector<int> privateGlobalInitOrder;

			struct ParamSpec
			{
				int symId = -1;
				std::string rawName;
				std::string paramIdent;
				std::string localIdent;
				std::string smpParamIdent;
				std::string semantic;
				TType astType;
				TQualifier qualifier = EvqIn;
				bool passByPtr = false;
				bool isSampler = false;
				ETexDim samplerDim = ETexDim::Dim2D;
			};

			struct FuncRecord
			{
				TIntermAggregate* astNode = nullptr;
				std::string plainName;
				std::string mangledName;
				std::string emittedName;
				std::string returnSemantic;
				TType returnAstType;
				std::vector<ParamSpec> params;
			};
			std::unordered_map<std::string, FuncRecord> funcsByMangled;
			std::vector<std::string> orderedCalledFuncs;
			std::string mainTargetMangled;

			// Per-function active state
			std::ostringstream fnOut;
			int indentLevel = 1;
			std::unordered_map<int, std::pair<std::string, bool>> fnLocals; // symId -> (exprName, isWgslPtrDeref)
			std::unordered_map<int, SamplerResourceInfo> fnSamplerParams;
			TType currentFnRetAstType;
			bool blockTerminated = false;

			// Active stage interface tracking
			std::set<uint32_t> stageInputLocs;
			std::set<uint32_t> stageOutputLocs;
			bool usesVertexId = false;
			bool usesInstanceId = false;
			bool usesFragCoord = false;
			bool usesFrontFace = false;
			bool usesPosOut = false;
			bool usesPointSizeOut = false;
			bool usesFragDepthOut = false;

			StructuredTextModuleBuilder(
				ETextLang targetLang,
				bool vertex,
				const std::string& entry,
				const std::vector<FxSamplerDesc>& samps,
				const std::vector<FxUniformParam>& params)
				: lang(targetLang)
				, isVertex(vertex)
				, entryPointName(entry)
				, declaredSamplers(samps)
				, declaredParams(params)
			{
			}

			void WriteIndent()
			{
				for (int i = 0; i < indentLevel; ++i)
					fnOut << "    ";
			}

			std::string AllocTmp(const std::string& typeStr, const std::string& rhsExpr)
			{
				std::string name = "t_" + std::to_string(nextTmpId++);
				WriteIndent();
				if (lang == ETextLang::WGSL_1_0)
					fnOut << "let " << name << " : " << typeStr << " = " << rhsExpr << ";\n";
				else
					fnOut << "const " << typeStr << " " << name << " = " << rhsExpr << ";\n";
				return name;
			}

			std::string AllocMutableLocal(const std::string& typeStr, const std::string& initExpr)
			{
				std::string name = "v_" + std::to_string(nextTmpId++);
				WriteIndent();
				if (lang == ETextLang::WGSL_1_0)
					fnOut << "var " << name << " : " << typeStr << " = " << initExpr << ";\n";
				else
					fnOut << typeStr << " " << name << " = " << initExpr << ";\n";
				return name;
			}

			const StructInfo& RegisterStruct(const TType& stType)
			{
				const TTypeList* sl = stType.getStruct();
				auto it = structMap.find(sl);
				if (it != structMap.end())
					return it->second;

				StructInfo info;
				std::string rawName = stType.getTypeName().c_str();
				info.typeName = SanitizeIdent(rawName.empty() ? "Struct" : rawName, "S_") + "_" + std::to_string(structMap.size());

				uint32_t curOff = 0;
				if (sl)
				{
					for (size_t i = 0; i < sl->size(); ++i)
					{
						const TType& ft = *(*sl)[i].type;
						if (ft.getBasicType() == EbtStruct)
							RegisterStruct(ft);
						std::string fn = ft.getFieldName().c_str();
						info.fieldNames.push_back(SanitizeIdent(fn.empty() ? ("f" + std::to_string(i)) : fn, "m_"));
						info.fieldTypes.push_back(ft);

						uint32_t fAlign = 4, fSize = 4;
						ComputeUniformLayout(ft, fAlign, fSize);
						curOff = AlignUpU32(curOff, fAlign);
						info.uniformFieldByteOffsets.push_back(curOff);
						curOff += fSize;
					}
				}
				info.uniformByteSize = AlignUpU32((std::max)(curOff, 16u), 16u);
				auto inserted = structMap.emplace(sl, std::move(info));
				structOrder.push_back(sl);
				return inserted.first->second;
			}

			void ComputeUniformLayout(const TType& t, uint32_t& outAlign, uint32_t& outSize, bool ignoreArray = false)
			{
				if (!ignoreArray && t.isArray())
				{
					uint32_t elemAlign = 4, elemSize = 4;
					ComputeUniformLayout(t, elemAlign, elemSize, true);
					uint32_t stride = AlignUpU32((std::max)(elemSize, elemAlign), 16u);
					uint32_t count = static_cast<uint32_t>((std::max)(1, t.getArraySize()));
					outAlign = 16u;
					outSize = stride * count;
					return;
				}
				if (t.getBasicType() == EbtStruct)
				{
					const StructInfo& si = RegisterStruct(t);
					outAlign = 16u;
					outSize = si.uniformByteSize;
					return;
				}
				if (t.isMatrix())
				{
					uint32_t cols = static_cast<uint32_t>((std::max)(1, t.getColsCount()));
					outAlign = 16u;
					outSize = cols * 16u;
					return;
				}
				uint32_t rows = static_cast<uint32_t>((std::max)(1, t.getRowsCount()));
				if (rows == 1) { outAlign = 4u; outSize = 4u; }
				else if (rows == 2) { outAlign = 8u; outSize = 8u; }
				else if (rows == 3) { outAlign = 16u; outSize = 12u; }
				else { outAlign = 16u; outSize = 16u; }
			}

			std::string GetScalarTypeStr(TBasicType bt) const
			{
				if (lang == ETextLang::WGSL_1_0)
				{
					if (bt == EbtInt) return "i32";
					if (bt == EbtBool) return "bool";
					return "f32";
				}
				else
				{
					if (bt == EbtInt) return "int";
					if (bt == EbtBool) return "bool";
					return "float";
				}
			}

			std::string GetVecTypeStr(TBasicType bt, uint32_t rows) const
			{
				if (rows <= 1)
					return GetScalarTypeStr(bt);
				if (lang == ETextLang::WGSL_1_0)
					return "vec" + std::to_string(rows) + "<" + GetScalarTypeStr(bt) + ">";
				else
					return GetScalarTypeStr(bt) + std::to_string(rows);
			}

			std::string GetMatTypeStr(uint32_t cols, uint32_t rows) const
			{
				if (lang == ETextLang::WGSL_1_0)
					return "mat" + std::to_string(cols) + "x" + std::to_string(rows) + "<f32>";
				else
					return "float" + std::to_string(cols) + "x" + std::to_string(rows);
			}

			std::string GetValueTypeStr(const TType& t, bool ignoreArray = false)
			{
				if (!ignoreArray && t.isArray())
				{
					int count = (std::max)(1, t.getArraySize());
					std::string elemStr = GetValueTypeStr(t, true);
					return "array<" + elemStr + ", " + std::to_string(count) + ">";
				}
				if (t.getBasicType() == EbtVoid)
					return "void";
				if (t.getBasicType() == EbtStruct)
					return RegisterStruct(t).typeName;
				if (t.isMatrix())
					return GetMatTypeStr(static_cast<uint32_t>(t.getColsCount()), static_cast<uint32_t>(t.getRowsCount()));
				return GetVecTypeStr(t.getBasicType(), static_cast<uint32_t>((std::max)(1, t.getRowsCount())));
			}

			std::string GetZeroExpr(const TType& t, bool ignoreArray = false)
			{
				std::string tyStr = GetValueTypeStr(t, ignoreArray);
				if (lang == ETextLang::WGSL_1_0)
					return tyStr + "()";

				if (!ignoreArray && t.isArray())
					return tyStr + "{}";
				if (t.getBasicType() == EbtStruct)
					return tyStr + "{}";
				if (t.isMatrix())
					return tyStr + "(0.0f)";
				uint32_t rows = static_cast<uint32_t>((std::max)(1, t.getRowsCount()));
				if (rows == 1)
				{
					if (t.getBasicType() == EbtInt) return "0";
					if (t.getBasicType() == EbtBool) return "false";
					return "0.0f";
				}
				return tyStr + "(" + (t.getBasicType() == EbtInt ? "0" : (t.getBasicType() == EbtBool ? "false" : "0.0f")) + ")";
			}

			std::string ConstructComposite(const TType& dstType, const std::vector<std::string>& elems, bool ignoreArray = false)
			{
				std::string tyStr = GetValueTypeStr(dstType, ignoreArray);
				std::ostringstream oss;
				const bool useBraces = (lang == ETextLang::MSL_2_0) && ((!ignoreArray && dstType.isArray()) || dstType.getBasicType() == EbtStruct);
				oss << tyStr << (useBraces ? "{" : "(");
				for (size_t i = 0; i < elems.size(); ++i)
				{
					if (i > 0) oss << ", ";
					oss << elems[i];
				}
				oss << (useBraces ? "}" : ")");
				return AllocTmp(tyStr, oss.str());
			}

			std::string CastScalarExpr(const std::string& valExpr, TBasicType srcBt, TBasicType dstBt)
			{
				if (srcBt == dstBt)
					return valExpr;
				if (dstBt == EbtFloat)
				{
					if (srcBt == EbtInt)
						return AllocTmp(GetScalarTypeStr(EbtFloat), GetScalarTypeStr(EbtFloat) + "(" + valExpr + ")");
					if (srcBt == EbtBool)
					{
						std::string sel = (lang == ETextLang::WGSL_1_0)
							? ("select(0.0, 1.0, " + valExpr + ")")
							: ("(" + valExpr + " ? 1.0f : 0.0f)");
						return AllocTmp(GetScalarTypeStr(EbtFloat), sel);
					}
				}
				if (dstBt == EbtInt)
				{
					if (srcBt == EbtFloat)
						return AllocTmp(GetScalarTypeStr(EbtInt), GetScalarTypeStr(EbtInt) + "(" + valExpr + ")");
					if (srcBt == EbtBool)
					{
						std::string sel = (lang == ETextLang::WGSL_1_0)
							? ("select(0i, 1i, " + valExpr + ")")
							: ("(" + valExpr + " ? 1 : 0)");
						return AllocTmp(GetScalarTypeStr(EbtInt), sel);
					}
				}
				if (dstBt == EbtBool)
				{
					if (srcBt == EbtFloat)
						return AllocTmp("bool", "(" + valExpr + " != " + FormatFloatLit(0.0f, lang) + ")");
					if (srcBt == EbtInt)
						return AllocTmp("bool", "(" + valExpr + " != " + FormatIntLit(0, lang) + ")");
				}
				return valExpr;
			}

			std::string SplatScalar(const std::string& scalarExpr, TBasicType bt, uint32_t rows)
			{
				if (rows <= 1)
					return scalarExpr;
				std::string vt = GetVecTypeStr(bt, rows);
				return AllocTmp(vt, vt + "(" + scalarExpr + ")");
			}

			static const char* SwizzleChar(uint32_t c)
			{
				static const char* kSwiz[4] = { "x", "y", "z", "w" };
				return kSwiz[c & 3u];
			}

			std::string CoerceVectorOrScalar(const std::string& valExpr, const TType& srcType, TBasicType dstBt, uint32_t dstRows)
			{
				uint32_t srcRows = static_cast<uint32_t>((std::max)(1, srcType.getRowsCount()));
				TBasicType srcBt = srcType.getBasicType();
				std::string cur = valExpr;

				if (srcRows == 1 && dstRows > 1)
				{
					cur = SplatScalar(cur, srcBt, dstRows);
					srcRows = dstRows;
				}
				else if (srcRows > dstRows)
				{
					std::string swz = ".";
					for (uint32_t i = 0; i < dstRows; ++i)
						swz += SwizzleChar(i);
					cur = AllocTmp(GetVecTypeStr(srcBt, dstRows), cur + swz);
					srcRows = dstRows;
				}
				else if (srcRows < dstRows)
				{
					std::vector<std::string> comps;
					for (uint32_t i = 0; i < srcRows; ++i)
						comps.push_back(cur + "." + SwizzleChar(i));
					std::string zeroLit = (srcBt == EbtInt) ? FormatIntLit(0, lang) : (srcBt == EbtBool ? "false" : FormatFloatLit(0.0f, lang));
					for (uint32_t i = srcRows; i < dstRows; ++i)
						comps.push_back(zeroLit);
					std::string vt = GetVecTypeStr(srcBt, dstRows);
					std::ostringstream oss;
					oss << vt << "(";
					for (size_t i = 0; i < comps.size(); ++i)
					{
						if (i > 0) oss << ", ";
						oss << comps[i];
					}
					oss << ")";
					cur = AllocTmp(vt, oss.str());
					srcRows = dstRows;
				}

				if (srcBt == dstBt)
					return cur;
				if (dstRows == 1)
					return CastScalarExpr(cur, srcBt, dstBt);

				std::string dstVecT = GetVecTypeStr(dstBt, dstRows);
				if ((srcBt == EbtInt && dstBt == EbtFloat) || (srcBt == EbtFloat && dstBt == EbtInt))
					return AllocTmp(dstVecT, dstVecT + "(" + cur + ")");
				if (srcBt == EbtBool && (dstBt == EbtFloat || dstBt == EbtInt))
				{
					std::string z = SplatScalar(dstBt == EbtInt ? FormatIntLit(0, lang) : FormatFloatLit(0.0f, lang), dstBt, dstRows);
					std::string o = SplatScalar(dstBt == EbtInt ? FormatIntLit(1, lang) : FormatFloatLit(1.0f, lang), dstBt, dstRows);
					return AllocTmp(dstVecT, "select(" + z + ", " + o + ", " + cur + ")");
				}
				if ((srcBt == EbtFloat || srcBt == EbtInt) && dstBt == EbtBool)
				{
					std::string z = SplatScalar(srcBt == EbtInt ? FormatIntLit(0, lang) : FormatFloatLit(0.0f, lang), srcBt, dstRows);
					return AllocTmp(dstVecT, "(" + cur + " != " + z + ")");
				}
				return cur;
			}

			std::string CoerceToType(const std::string& valExpr, const TType& srcType, const TType& dstType)
			{
				if (srcType == dstType)
					return valExpr;
				if (!srcType.isArray() && !dstType.isArray() && !srcType.isMatrix() && !dstType.isMatrix() &&
					srcType.getBasicType() != EbtStruct && dstType.getBasicType() != EbtStruct)
				{
					return CoerceVectorOrScalar(valExpr, srcType, dstType.getBasicType(), static_cast<uint32_t>((std::max)(1, dstType.getRowsCount())));
				}
				return valExpr;
			}

			std::string UbVec4Expr(const std::string& vecIdxExpr) const
			{
				if (lang == ETextLang::WGSL_1_0)
					return "_para_ub.data[" + vecIdxExpr + "]";
				else
					return "_ctx._ub[" + vecIdxExpr + "]";
			}

			std::string ReinterpretFloatScalar(const std::string& fExpr, TBasicType dstBt)
			{
				if (dstBt == EbtFloat)
					return fExpr;
				if (lang == ETextLang::WGSL_1_0)
				{
					std::string iExpr = "bitcast<i32>(" + fExpr + ")";
					return (dstBt == EbtBool) ? ("(" + iExpr + " != 0i)") : iExpr;
				}
				else
				{
					std::string iExpr = "as_type<int>(" + fExpr + ")";
					return (dstBt == EbtBool) ? ("(" + iExpr + " != 0)") : iExpr;
				}
			}

			// Unpack an arbitrary TType starting at (dynamicVecBaseExpr * 16 + staticByteOffset) from _para_ub.data[]
			std::string UnpackUniformValue(const std::string& dynVecBaseExpr, uint32_t staticByteOff, const TType& t, bool ignoreArray = false)
			{
				auto makeVecIdx = [&](uint32_t addVecs) -> std::string {
					std::string addLit = FormatIntLit(static_cast<int32_t>(addVecs), lang);
					if (dynVecBaseExpr.empty())
						return addLit;
					if (addVecs == 0)
						return dynVecBaseExpr;
					return "(" + dynVecBaseExpr + " + " + addLit + ")";
				};

				if (!ignoreArray && t.isArray())
				{
					uint32_t eAlign = 4, eSize = 4;
					ComputeUniformLayout(t, eAlign, eSize, true);
					uint32_t strideBytes = AlignUpU32((std::max)(eSize, eAlign), 16u);
					int count = (std::max)(1, t.getArraySize());
					std::vector<std::string> elems;
					elems.reserve(static_cast<size_t>(count));
					for (int i = 0; i < count; ++i)
						elems.push_back(UnpackUniformValue(dynVecBaseExpr, staticByteOff + static_cast<uint32_t>(i) * strideBytes, t, true));
					return ConstructComposite(t, elems, false);
				}

				if (t.getBasicType() == EbtStruct)
				{
					const StructInfo& si = RegisterStruct(t);
					std::vector<std::string> fields;
					fields.reserve(si.fieldTypes.size());
					for (size_t i = 0; i < si.fieldTypes.size(); ++i)
						fields.push_back(UnpackUniformValue(dynVecBaseExpr, staticByteOff + si.uniformFieldByteOffsets[i], si.fieldTypes[i], false));
					return ConstructComposite(t, fields, true);
				}

				if (t.isMatrix())
				{
					uint32_t cols = static_cast<uint32_t>((std::max)(1, t.getColsCount()));
					uint32_t rows = static_cast<uint32_t>((std::max)(1, t.getRowsCount()));
					uint32_t baseVec = staticByteOff / 16u;
					std::vector<std::string> colExprs;
					for (uint32_t c = 0; c < cols; ++c)
					{
						std::string v4 = UbVec4Expr(makeVecIdx(baseVec + c));
						if (rows == 4) colExprs.push_back(v4);
						else if (rows == 3) colExprs.push_back(v4 + ".xyz");
						else if (rows == 2) colExprs.push_back(v4 + ".xy");
						else colExprs.push_back(v4 + ".x");
					}
					return ConstructComposite(t, colExprs, true);
				}

				uint32_t rows = static_cast<uint32_t>((std::max)(1, t.getRowsCount()));
				TBasicType bt = t.getBasicType();
				uint32_t baseVec = staticByteOff / 16u;
				uint32_t startComp = (staticByteOff / 4u) % 4u;

				if (bt == EbtFloat && startComp == 0)
				{
					std::string v4 = UbVec4Expr(makeVecIdx(baseVec));
					if (rows == 4) return AllocTmp(GetVecTypeStr(EbtFloat, 4), v4);
					if (rows == 3) return AllocTmp(GetVecTypeStr(EbtFloat, 3), v4 + ".xyz");
					if (rows == 2) return AllocTmp(GetVecTypeStr(EbtFloat, 2), v4 + ".xy");
					return AllocTmp(GetScalarTypeStr(EbtFloat), v4 + ".x");
				}

				std::vector<std::string> comps;
				for (uint32_t r = 0; r < rows; ++r)
				{
					uint32_t bytePos = staticByteOff + r * 4u;
					uint32_t vIdx = bytePos / 16u;
					uint32_t cIdx = (bytePos / 4u) % 4u;
					std::string rawComp = UbVec4Expr(makeVecIdx(vIdx)) + "." + SwizzleChar(cIdx);
					comps.push_back(ReinterpretFloatScalar(rawComp, bt));
				}
				if (rows == 1)
					return AllocTmp(GetScalarTypeStr(bt), comps[0]);
				return ConstructComposite(t, comps, true);
			}

			int ExtractImmediateInt(TIntermTyped* node)
			{
				if (!node) return 0;
				if (TIntermConstant* c = node->getAsConstant())
				{
					if (c->getBasicType() == EbtInt) return c->toInt(0);
					if (c->getBasicType() == EbtFloat) return static_cast<int>(c->toFloat(0));
					if (c->getBasicType() == EbtBool) return c->toBool(0) ? 1 : 0;
				}
				return 0;
			}

			void ExtractSwizzleIndices(TIntermTyped* rightNode, std::vector<uint32_t>& outIndices)
			{
				outIndices.clear();
				if (!rightNode)
					return;
				if (TIntermAggregate* agg = rightNode->getAsAggregate())
				{
					for (TIntermNode* child : agg->getNodes())
					{
						if (child && child->getAsTyped())
							outIndices.push_back(static_cast<uint32_t>(ExtractImmediateInt(child->getAsTyped())));
					}
				}
				else
				{
					outIndices.push_back(static_cast<uint32_t>(ExtractImmediateInt(rightNode)));
				}
			}

			std::string EmitConstantNode(TIntermConstant* c)
			{
				const TType& t = c->getType();
				if (t.getBasicType() == EbtStruct)
				{
					if (c->getCount() <= 1 && c->toFloat(0) == 0.0f && c->toInt(0) == 0)
						return AllocTmp(GetValueTypeStr(t), GetZeroExpr(t));

					unsigned cursor = 0;
					std::function<std::string(const TType&)> buildField = [&](const TType& ft) -> std::string {
						if (ft.isArray())
						{
							int n = (std::max)(1, ft.getArraySize());
							TType elemT = ft;
							elemT.clearArrayness();
							std::vector<std::string> elems;
							for (int i = 0; i < n; ++i)
								elems.push_back(buildField(elemT));
							return ConstructComposite(ft, elems, false);
						}
						if (ft.getBasicType() == EbtStruct)
						{
							const StructInfo& si = RegisterStruct(ft);
							std::vector<std::string> subFields;
							for (const TType& sft : si.fieldTypes)
								subFields.push_back(buildField(sft));
							return ConstructComposite(ft, subFields, true);
						}
						int count = ft.getInstanceSize();
						std::vector<std::string> scalars;
						for (int k = 0; k < count; ++k)
						{
							unsigned idx = (std::min)(cursor++, (std::max)(1u, c->getCount()) - 1u);
							const auto& val = c->getValue(idx);
							if (ft.getBasicType() == EbtInt)
								scalars.push_back(FormatIntLit(val.type == EbtFloat ? static_cast<int>(val.asFloat) : val.asInt, lang));
							else if (ft.getBasicType() == EbtBool)
								scalars.push_back((val.type == EbtFloat ? (val.asFloat != 0.0f) : val.asBool) ? "true" : "false");
							else
								scalars.push_back(FormatFloatLit(val.type == EbtInt ? static_cast<float>(val.asInt) : val.asFloat, lang));
						}
						if (ft.isMatrix())
						{
							int cols = ft.getColsCount();
							int rows = ft.getRowsCount();
							std::string colT = GetVecTypeStr(EbtFloat, static_cast<uint32_t>(rows));
							std::vector<std::string> colsVec;
							for (int col = 0; col < cols; ++col)
							{
								std::ostringstream cs;
								cs << colT << "(";
								for (int r = 0; r < rows; ++r)
								{
									if (r > 0) cs << ", ";
									cs << scalars[static_cast<size_t>(col * rows + r)];
								}
								cs << ")";
								colsVec.push_back(cs.str());
							}
							return ConstructComposite(ft, colsVec, true);
						}
						if (scalars.size() == 1)
							return scalars[0];
						return ConstructComposite(ft, scalars, true);
					};

					const StructInfo& si = RegisterStruct(t);
					std::vector<std::string> topFields;
					for (const TType& ft : si.fieldTypes)
						topFields.push_back(buildField(ft));
					return ConstructComposite(t, topFields, true);
				}

				int nElems = t.getInstanceSize();
				unsigned nConsts = (std::max)(1u, c->getCount());
				std::vector<std::string> scLits;
				scLits.reserve(static_cast<size_t>(nElems));
				for (int i = 0; i < nElems; ++i)
				{
					unsigned idx = (std::min)(static_cast<unsigned>(i), nConsts - 1u);
					if (t.getBasicType() == EbtInt)
						scLits.push_back(FormatIntLit(c->toInt(idx), lang));
					else if (t.getBasicType() == EbtBool)
						scLits.push_back(c->toBool(idx) ? "true" : "false");
					else
						scLits.push_back(FormatFloatLit(c->toFloat(idx), lang));
				}

				if (t.isMatrix())
				{
					int cols = t.getColsCount();
					int rows = t.getRowsCount();
					std::string colT = GetVecTypeStr(EbtFloat, static_cast<uint32_t>(rows));
					std::vector<std::string> colExprs;
					for (int col = 0; col < cols; ++col)
					{
						std::ostringstream cs;
						cs << colT << "(";
						for (int r = 0; r < rows; ++r)
						{
							if (r > 0) cs << ", ";
							cs << scLits[static_cast<size_t>(col * rows + r)];
						}
						cs << ")";
						colExprs.push_back(cs.str());
					}
					return ConstructComposite(t, colExprs, true);
				}

				if (nElems <= 1)
					return scLits[0];
				return ConstructComposite(t, scLits, true);
			}

			bool TryEmitUniformLoad(TIntermTyped* node, std::string& outExpr)
			{
				std::vector<TIntermBinary*> chain;
				TIntermTyped* cur = node;
				while (cur)
				{
					TIntermBinary* bin = cur->getAsBinaryNode();
					if (bin && (bin->getOp() == EOpIndexDirectStruct || bin->getOp() == EOpIndexDirect || bin->getOp() == EOpIndexIndirect))
					{
						TIntermTyped* lhs = bin->getLeft();
						if (lhs && !lhs->isArray() && (lhs->isVector() || lhs->isMatrix()))
							break;
						chain.push_back(bin);
						cur = lhs;
					}
					else
					{
						break;
					}
				}

				TIntermSymbol* baseSym = cur ? cur->getAsSymbolNode() : nullptr;
				if (!baseSym)
					return false;

				auto uit = symIdToUniformIdx.find(baseSym->getId());
				if (uit == symIdToUniformIdx.end())
					return false;

				const UniformFieldInfo& ufi = uniformFields[uit->second];
				if (ufi.isMutableCopy)
					return false;

				uint32_t staticByteOff = ufi.byteOffset;
				std::string dynVecBaseExpr;
				TType curType = baseSym->getType();

				for (auto it = chain.rbegin(); it != chain.rend(); ++it)
				{
					TIntermBinary* b = *it;
					if (b->getOp() == EOpIndexDirectStruct)
					{
						const StructInfo& si = RegisterStruct(curType);
						int fIdx = (std::max)(0, ExtractImmediateInt(b->getRight()));
						if (static_cast<size_t>(fIdx) < si.uniformFieldByteOffsets.size())
						{
							staticByteOff += si.uniformFieldByteOffsets[static_cast<size_t>(fIdx)];
							curType = si.fieldTypes[static_cast<size_t>(fIdx)];
						}
					}
					else if (curType.isArray())
					{
						uint32_t eAlign = 4, eSize = 4;
						ComputeUniformLayout(curType, eAlign, eSize, true);
						uint32_t strideBytes = AlignUpU32((std::max)(eSize, eAlign), 16u);
						uint32_t strideVecs = strideBytes / 16u;

						if (b->getOp() == EOpIndexDirect || b->getRight()->getAsConstant() != nullptr)
						{
							int idx = (std::max)(0, ExtractImmediateInt(b->getRight()));
							staticByteOff += static_cast<uint32_t>(idx) * strideBytes;
						}
						else
						{
							std::string dynIdx = CoerceVectorOrScalar(EmitRValue(b->getRight()), b->getRight()->getType(), EbtInt, 1);
							std::string term = (strideVecs == 1u)
								? dynIdx
								: ("(" + dynIdx + " * " + FormatIntLit(static_cast<int32_t>(strideVecs), lang) + ")");
							dynVecBaseExpr = dynVecBaseExpr.empty() ? term : ("(" + dynVecBaseExpr + " + " + term + ")");
						}
						curType.clearArrayness();
					}
				}

				const TType& resType = chain.empty() ? baseSym->getType() : chain.front()->getType();
				outExpr = UnpackUniformValue(dynVecBaseExpr, staticByteOff, resType, false);
				return true;
			}

			struct LValueRef
			{
				std::string baseExpr;
				TType baseType;
				bool hasSwizzle = false;
				std::vector<uint32_t> swizzleIndices;
				TType resultType;
			};

			bool EmitLValue(TIntermTyped* node, LValueRef& outLv)
			{
				if (!node)
					return false;

				if (TIntermBinary* bin = node->getAsBinaryNode())
				{
					if (bin->getOp() == EOpVectorSwizzle)
					{
						LValueRef sub;
						if (!EmitLValue(bin->getLeft(), sub))
							return false;
						std::vector<uint32_t> swiz;
						ExtractSwizzleIndices(bin->getRight(), swiz);
						if (sub.hasSwizzle)
						{
							std::vector<uint32_t> composed;
							for (uint32_t idx : swiz)
								composed.push_back(sub.swizzleIndices[(std::min)(static_cast<size_t>(idx), sub.swizzleIndices.size() - 1)]);
							outLv = sub;
							outLv.swizzleIndices = std::move(composed);
							outLv.resultType = bin->getType();
							return true;
						}
						outLv = sub;
						outLv.hasSwizzle = true;
						outLv.swizzleIndices = std::move(swiz);
						outLv.resultType = bin->getType();
						return true;
					}

					if (bin->getOp() == EOpIndexDirectStruct || bin->getOp() == EOpIndexDirect || bin->getOp() == EOpIndexIndirect)
					{
						std::vector<TIntermBinary*> steps;
						TIntermTyped* cur = node;
						while (cur)
						{
							TIntermBinary* b = cur->getAsBinaryNode();
							if (b && (b->getOp() == EOpIndexDirectStruct || b->getOp() == EOpIndexDirect || b->getOp() == EOpIndexIndirect))
							{
								steps.push_back(b);
								cur = b->getLeft();
							}
							else
							{
								break;
							}
						}
						LValueRef rootLv;
						if (!EmitLValue(cur, rootLv) || rootLv.hasSwizzle)
							return false;

						std::string expr = rootLv.baseExpr;
						TType stepType = rootLv.baseType;
						for (auto it = steps.rbegin(); it != steps.rend(); ++it)
						{
							TIntermBinary* b = *it;
							if (b->getOp() == EOpIndexDirectStruct)
							{
								const StructInfo& si = RegisterStruct(stepType);
								int fIdx = (std::max)(0, ExtractImmediateInt(b->getRight()));
								if (static_cast<size_t>(fIdx) < si.fieldNames.size())
								{
									expr += "." + si.fieldNames[static_cast<size_t>(fIdx)];
									stepType = si.fieldTypes[static_cast<size_t>(fIdx)];
								}
							}
							else
							{
								std::string idxStr;
								if (b->getOp() == EOpIndexDirect || b->getRight()->getAsConstant() != nullptr)
									idxStr = FormatIntLit((std::max)(0, ExtractImmediateInt(b->getRight())), lang);
								else
									idxStr = CoerceVectorOrScalar(EmitRValue(b->getRight()), b->getRight()->getType(), EbtInt, 1);
								expr += "[" + idxStr + "]";
								stepType = b->getType();
							}
						}

						outLv.baseExpr = expr;
						outLv.baseType = node->getType();
						outLv.hasSwizzle = false;
						outLv.resultType = node->getType();
						return true;
					}
				}

				if (TIntermSymbol* sym = node->getAsSymbolNode())
				{
					int sid = sym->getId();
					auto lit = fnLocals.find(sid);
					if (lit != fnLocals.end())
					{
						outLv.baseExpr = lit->second.second ? ("(*" + lit->second.first + ")") : lit->second.first;
						outLv.baseType = sym->getType();
						outLv.hasSwizzle = false;
						outLv.resultType = sym->getType();
						return true;
					}
					auto pit = privateGlobals.find(sid);
					if (pit != privateGlobals.end())
					{
						outLv.baseExpr = (lang == ETextLang::WGSL_1_0) ? pit->second.varName : ("_ctx." + pit->second.varName);
						outLv.baseType = sym->getType();
						outLv.hasSwizzle = false;
						outLv.resultType = sym->getType();
						return true;
					}
					auto uit = symIdToUniformIdx.find(sid);
					if (uit != symIdToUniformIdx.end() && uniformFields[uit->second].isMutableCopy)
					{
						outLv.baseExpr = (lang == ETextLang::WGSL_1_0) ? uniformFields[uit->second].varName : ("_ctx." + uniformFields[uit->second].varName);
						outLv.baseType = sym->getType();
						outLv.hasSwizzle = false;
						outLv.resultType = sym->getType();
						return true;
					}
				}
				return false;
			}

			std::string LoadFromLValue(const LValueRef& lv)
			{
				std::string baseVal = AllocTmp(GetValueTypeStr(lv.baseType), lv.baseExpr);
				if (!lv.hasSwizzle)
					return baseVal;

				std::string swz = ".";
				uint32_t maxComp = static_cast<uint32_t>((std::max)(1, lv.baseType.getRowsCount()) - 1);
				for (uint32_t idx : lv.swizzleIndices)
					swz += SwizzleChar((std::min)(idx, maxComp));
				return AllocTmp(GetValueTypeStr(lv.resultType), baseVal + swz);
			}

			void StoreToLValue(const LValueRef& lv, const std::string& rhsValExpr)
			{
				if (!lv.hasSwizzle)
				{
					WriteIndent();
					fnOut << lv.baseExpr << " = " << rhsValExpr << ";\n";
					return;
				}
				// Note: WGSL 1.0 forbids swizzle assignment on LHS (`v.xy = rhs` is illegal in WGSL!),
				// so component-by-component index assignment works universally in BOTH MSL and WGSL!
				if (lv.swizzleIndices.size() == 1)
				{
					WriteIndent();
					fnOut << lv.baseExpr << "." << SwizzleChar(lv.swizzleIndices[0]) << " = " << rhsValExpr << ";\n";
				}
				else
				{
					for (size_t i = 0; i < lv.swizzleIndices.size(); ++i)
					{
						WriteIndent();
						fnOut << lv.baseExpr << "." << SwizzleChar(lv.swizzleIndices[i]) << " = " << rhsValExpr << "." << SwizzleChar(static_cast<uint32_t>(i)) << ";\n";
					}
				}
			}

			std::string ApplyNumericBinaryOp(
				TOperator op,
				const std::string& lhsExpr,
				const TType& lhsType,
				const std::string& rhsExpr,
				const TType& rhsType,
				const TType& resType)
			{
				std::string resTyStr = GetValueTypeStr(resType);

				if (op == EOpVectorTimesScalar || (op == EOpMul && (lhsType.isVector() ^ rhsType.isVector()) && (lhsType.isScalar() || rhsType.isScalar())))
				{
					if (lhsType.isVector())
					{
						std::string s = CoerceVectorOrScalar(rhsExpr, rhsType, lhsType.getBasicType(), 1);
						return AllocTmp(resTyStr, "(" + lhsExpr + " * " + s + ")");
					}
					else
					{
						std::string s = CoerceVectorOrScalar(lhsExpr, lhsType, rhsType.getBasicType(), 1);
						return AllocTmp(resTyStr, "(" + s + " * " + rhsExpr + ")");
					}
				}
				if (op == EOpMatrixTimesScalar || (op == EOpMul && (lhsType.isMatrix() ^ rhsType.isMatrix()) && (lhsType.isScalar() || rhsType.isScalar())))
				{
					std::string m = lhsType.isMatrix() ? lhsExpr : rhsExpr;
					std::string s = lhsType.isMatrix()
						? CoerceVectorOrScalar(rhsExpr, rhsType, EbtFloat, 1)
						: CoerceVectorOrScalar(lhsExpr, lhsType, EbtFloat, 1);
					return AllocTmp(resTyStr, "(" + m + " * " + s + ")");
				}
				if (op == EOpVectorTimesMatrix)
				{
					// Transposed matrix storage convention: HLSL v * M -> M_col * v
					return AllocTmp(resTyStr, "(" + rhsExpr + " * " + lhsExpr + ")");
				}
				if (op == EOpMatrixTimesVector)
				{
					// Transposed matrix storage convention: HLSL M * v -> v * M_col
					return AllocTmp(resTyStr, "(" + rhsExpr + " * " + lhsExpr + ")");
				}
				if (op == EOpMatrixTimesMatrix)
				{
					// Component-wise HLSL M1 * M2
					int cols = resType.getColsCount();
					std::vector<std::string> colsOut;
					for (int c = 0; c < cols; ++c)
					{
						std::string ci = FormatIntLit(c, lang);
						colsOut.push_back("(" + lhsExpr + "[" + ci + "] * " + rhsExpr + "[" + ci + "])");
					}
					return ConstructComposite(resType, colsOut, true);
				}

				if (resType.isMatrix())
				{
					int cols = resType.getColsCount();
					int rows = resType.getRowsCount();
					const char* symOp = "*";
					if (op == EOpAdd || op == EOpAddAssign) symOp = "+";
					else if (op == EOpSub || op == EOpSubAssign) symOp = "-";
					else if (op == EOpDiv || op == EOpDivAssign) symOp = "/";

					std::vector<std::string> colsOut;
					for (int c = 0; c < cols; ++c)
					{
						std::string ci = FormatIntLit(c, lang);
						std::string c1 = lhsType.isMatrix()
							? (lhsExpr + "[" + ci + "]")
							: CoerceVectorOrScalar(lhsExpr, lhsType, EbtFloat, static_cast<uint32_t>(rows));
						std::string c2 = rhsType.isMatrix()
							? (rhsExpr + "[" + ci + "]")
							: CoerceVectorOrScalar(rhsExpr, rhsType, EbtFloat, static_cast<uint32_t>(rows));
						colsOut.push_back("(" + c1 + " " + symOp + " " + c2 + ")");
					}
					return ConstructComposite(resType, colsOut, true);
				}

				uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
				TBasicType bt = resType.getBasicType();
				std::string l = CoerceVectorOrScalar(lhsExpr, lhsType, bt, rows);
				std::string r = CoerceVectorOrScalar(rhsExpr, rhsType, bt, rows);
				bool isInt = (bt == EbtInt);

				if (op == EOpMod || op == EOpModAssign)
				{
					if (lang == ETextLang::MSL_2_0 && !isInt)
						return AllocTmp(resTyStr, "fmod(" + l + ", " + r + ")");
					return AllocTmp(resTyStr, "(" + l + " % " + r + ")");
				}

				if (op == EOpLeftShift || op == EOpLeftShiftAssign || op == EOpRightShift || op == EOpRightShiftAssign)
				{
					const char* shOp = (op == EOpLeftShift || op == EOpLeftShiftAssign) ? "<<" : ">>";
					if (lang == ETextLang::WGSL_1_0)
					{
						std::string uType = (rows <= 1) ? "u32" : ("vec" + std::to_string(rows) + "<u32>");
						r = uType + "(" + r + ")";
					}
					return AllocTmp(resTyStr, "(" + l + " " + shOp + " " + r + ")");
				}

				const char* binSym = "+";
				switch (op)
				{
				case EOpAdd:
				case EOpAddAssign: binSym = "+"; break;
				case EOpSub:
				case EOpSubAssign: binSym = "-"; break;
				case EOpMul:
				case EOpMulAssign:
				case EOpVectorTimesScalarAssign: binSym = "*"; break;
				case EOpDiv:
				case EOpDivAssign: binSym = "/"; break;
				case EOpAnd:
				case EOpAndAssign: binSym = "&"; break;
				case EOpInclusiveOr:
				case EOpInclusiveOrAssign: binSym = "|"; break;
				case EOpExclusiveOr:
				case EOpExclusiveOrAssign: binSym = "^"; break;
				default: binSym = "+"; break;
				}
				return AllocTmp(resTyStr, "(" + l + " " + binSym + " " + r + ")");
			}

			std::string EmitComparison(TOperator op, TIntermTyped* leftNode, TIntermTyped* rightNode, const TType& resType)
			{
				std::string l = EmitRValue(leftNode);
				std::string r = EmitRValue(rightNode);
				const TType& lt = leftNode->getType();
				const TType& rt = rightNode->getType();
				uint32_t rows = static_cast<uint32_t>((std::max)({ 1, lt.getRowsCount(), rt.getRowsCount() }));
				TBasicType operandBt = (lt.getBasicType() == EbtFloat || rt.getBasicType() == EbtFloat) ? EbtFloat
					: ((lt.getBasicType() == EbtInt || rt.getBasicType() == EbtInt) ? EbtInt : EbtBool);

				l = CoerceVectorOrScalar(l, lt, operandBt, rows);
				r = CoerceVectorOrScalar(r, rt, operandBt, rows);

				const char* cmpSym = "==";
				switch (op)
				{
				case EOpEqual:
				case EOpVectorEqual: cmpSym = "=="; break;
				case EOpNotEqual:
				case EOpVectorNotEqual: cmpSym = "!="; break;
				case EOpLessThan: cmpSym = "<"; break;
				case EOpGreaterThan: cmpSym = ">"; break;
				case EOpLessThanEqual: cmpSym = "<="; break;
				case EOpGreaterThanEqual: cmpSym = ">="; break;
				default: break;
				}

				std::string cmpVecT = GetVecTypeStr(EbtBool, rows);
				std::string cmpVal = AllocTmp(cmpVecT, "(" + l + " " + cmpSym + " " + r + ")");
				if (rows > 1 && resType.isScalar() && resType.getBasicType() == EbtBool)
				{
					return AllocTmp("bool", ((op == EOpNotEqual) ? "any(" : "all(") + cmpVal + ")");
				}
				TType cmpAstType(EbtBool, EbpUndefined, EvqTemporary, 1, static_cast<int>(rows));
				return CoerceToType(cmpVal, cmpAstType, resType);
			}

			std::string ConstructTypeFromScalar(const std::string& scalarExpr, const TType& scalarType, const TType& ft)
			{
				if (ft.isArray())
				{
					int n = (std::max)(1, ft.getArraySize());
					TType elemT = ft;
					elemT.clearArrayness();
					std::string elemVal = ConstructTypeFromScalar(scalarExpr, scalarType, elemT);
					return ConstructComposite(ft, std::vector<std::string>(static_cast<size_t>(n), elemVal), false);
				}
				if (ft.getBasicType() == EbtStruct)
				{
					const StructInfo& si = RegisterStruct(ft);
					if (si.fieldTypes.empty())
						return AllocTmp(GetValueTypeStr(ft), GetZeroExpr(ft));
					std::vector<std::string> subFields;
					subFields.reserve(si.fieldTypes.size());
					for (const TType& sft : si.fieldTypes)
						subFields.push_back(ConstructTypeFromScalar(scalarExpr, scalarType, sft));
					return ConstructComposite(ft, subFields, true);
				}
				if (ft.isMatrix())
				{
					int cols = ft.getColsCount();
					int rows = ft.getRowsCount();
					std::string colVal = CoerceVectorOrScalar(scalarExpr, scalarType, EbtFloat, static_cast<uint32_t>(rows));
					return ConstructComposite(ft, std::vector<std::string>(static_cast<size_t>(cols), colVal), true);
				}
				return CoerceVectorOrScalar(scalarExpr, scalarType, ft.getBasicType(), static_cast<uint32_t>((std::max)(1, ft.getRowsCount())));
			}

			void FlattenValueToScalars(const std::string& valExpr, const TType& t, std::vector<std::pair<std::string, TBasicType>>& outScalars)
			{
				if (t.isArray())
				{
					int n = (std::max)(1, t.getArraySize());
					TType elemT = t;
					elemT.clearArrayness();
					for (int i = 0; i < n; ++i)
						FlattenValueToScalars(valExpr + "[" + std::to_string(i) + "]", elemT, outScalars);
					return;
				}
				if (t.getBasicType() == EbtStruct)
				{
					const StructInfo& si = RegisterStruct(t);
					for (size_t i = 0; i < si.fieldTypes.size(); ++i)
						FlattenValueToScalars(valExpr + "." + si.fieldNames[i], si.fieldTypes[i], outScalars);
					return;
				}
				if (t.isMatrix())
				{
					int cols = t.getColsCount();
					int rows = t.getRowsCount();
					for (int c = 0; c < cols; ++c)
					{
						std::string colExpr = valExpr + "[" + std::to_string(c) + "]";
						for (int r = 0; r < rows; ++r)
							outScalars.emplace_back(colExpr + "." + SwizzleChar(static_cast<uint32_t>(r)), EbtFloat);
					}
					return;
				}
				int rows = (std::max)(1, t.getRowsCount());
				TBasicType bt = t.getBasicType();
				if (rows == 1)
				{
					outScalars.emplace_back(valExpr, bt);
					return;
				}
				for (int r = 0; r < rows; ++r)
					outScalars.emplace_back(valExpr + "." + SwizzleChar(static_cast<uint32_t>(r)), bt);
			}

			std::string ConsumeScalarsForType(const std::vector<std::pair<std::string, TBasicType>>& scalars, size_t& cursor, const TType& ft)
			{
				if (cursor >= scalars.size())
					return AllocTmp(GetValueTypeStr(ft), GetZeroExpr(ft));
				if (ft.isArray())
				{
					int n = (std::max)(1, ft.getArraySize());
					TType elemT = ft;
					elemT.clearArrayness();
					std::vector<std::string> elems;
					elems.reserve(static_cast<size_t>(n));
					for (int i = 0; i < n; ++i)
						elems.push_back(ConsumeScalarsForType(scalars, cursor, elemT));
					return ConstructComposite(ft, elems, false);
				}
				if (ft.getBasicType() == EbtStruct)
				{
					const StructInfo& si = RegisterStruct(ft);
					if (si.fieldTypes.empty())
						return AllocTmp(GetValueTypeStr(ft), GetZeroExpr(ft));
					std::vector<std::string> subFields;
					subFields.reserve(si.fieldTypes.size());
					for (const TType& sft : si.fieldTypes)
						subFields.push_back(ConsumeScalarsForType(scalars, cursor, sft));
					return ConstructComposite(ft, subFields, true);
				}
				if (ft.isMatrix())
				{
					int cols = ft.getColsCount();
					int rows = ft.getRowsCount();
					TType colT(EbtFloat, EbpUndefined, EvqTemporary, 1, rows);
					std::vector<std::string> colsVec;
					colsVec.reserve(static_cast<size_t>(cols));
					for (int c = 0; c < cols; ++c)
					{
						std::vector<std::string> comps;
						for (int r = 0; r < rows; ++r)
						{
							if (cursor < scalars.size())
							{
								auto sp = scalars[cursor++];
								comps.push_back(CastScalarExpr(sp.first, sp.second, EbtFloat));
							}
							else
							{
								comps.push_back(FormatFloatLit(0.0f, lang));
							}
						}
						colsVec.push_back(rows == 1 ? comps[0] : ConstructComposite(colT, comps, true));
					}
					return ConstructComposite(ft, colsVec, true);
				}
				int rows = (std::max)(1, ft.getRowsCount());
				TBasicType dstBt = ft.getBasicType();
				std::vector<std::string> comps;
				comps.reserve(static_cast<size_t>(rows));
				for (int r = 0; r < rows; ++r)
				{
					if (cursor < scalars.size())
					{
						auto sp = scalars[cursor++];
						comps.push_back(CastScalarExpr(sp.first, sp.second, dstBt));
					}
					else
					{
						if (dstBt == EbtInt) comps.push_back(FormatIntLit(0, lang));
						else if (dstBt == EbtBool) comps.push_back("false");
						else comps.push_back(FormatFloatLit(0.0f, lang));
					}
				}
				return rows == 1 ? comps[0] : ConstructComposite(ft, comps, true);
			}

			std::string EmitConstructor(TIntermAggregate* agg)
			{
				const TType& dstType = agg->getType();
				TNodeArray& args = agg->getNodes();

				if (agg->getOp() == EOpConstructStruct)
				{
					const StructInfo& si = RegisterStruct(dstType);
					if (si.fieldTypes.empty())
						return AllocTmp(GetValueTypeStr(dstType), GetZeroExpr(dstType));

					if (args.size() == 1)
					{
						TIntermTyped* a0 = args[0]->getAsTyped();
						if (TIntermConstant* c0 = a0 ? a0->getAsConstant() : nullptr)
						{
							if (c0->getCount() <= 1 && c0->toFloat(0) == 0.0f && c0->toInt(0) == 0 && !c0->toBool(0))
								return AllocTmp(GetValueTypeStr(dstType), GetZeroExpr(dstType));
						}
						if (a0 && a0->getType().isScalar() &&
							(si.fieldTypes.size() != 1 || si.fieldTypes[0].isArray() || si.fieldTypes[0].isMatrix() || si.fieldTypes[0].getBasicType() == EbtStruct))
						{
							return ConstructTypeFromScalar(EmitRValue(a0), a0->getType(), dstType);
						}
					}

					std::vector<std::string> fieldVals;
					fieldVals.reserve(si.fieldTypes.size());
					if (args.size() == si.fieldTypes.size())
					{
						for (size_t i = 0; i < si.fieldTypes.size(); ++i)
						{
							TIntermTyped* a = args[i]->getAsTyped();
							const TType& ft = si.fieldTypes[i];
							std::string v = EmitRValue(a);
							if (a->getType().isScalar() && (ft.isArray() || ft.isMatrix() || ft.getBasicType() == EbtStruct))
								fieldVals.push_back(ConstructTypeFromScalar(v, a->getType(), ft));
							else
								fieldVals.push_back(CoerceToType(v, a->getType(), ft));
						}
					}
					else
					{
						std::vector<std::pair<std::string, TBasicType>> flatScalars;
						for (size_t i = 0; i < args.size(); ++i)
						{
							if (TIntermTyped* a = args[i]->getAsTyped())
								FlattenValueToScalars(EmitRValue(a), a->getType(), flatScalars);
						}
						size_t cursor = 0;
						for (size_t i = 0; i < si.fieldTypes.size(); ++i)
							fieldVals.push_back(ConsumeScalarsForType(flatScalars, cursor, si.fieldTypes[i]));
					}
					return ConstructComposite(dstType, fieldVals, true);
				}

				if (agg->getOp() == EOpConstructArray)
				{
					TType elemT = dstType;
					elemT.clearArrayness();
					std::vector<std::string> elems;
					for (size_t i = 0; i < args.size(); ++i)
					{
						TIntermTyped* a = args[i]->getAsTyped();
						std::string v = EmitRValue(a);
						elems.push_back(CoerceToType(v, a->getType(), elemT));
					}
					return ConstructComposite(dstType, elems, false);
				}

				if (agg->getOp() == EOpConstructMat2x2FromMat || agg->getOp() == EOpConstructMat3x3FromMat ||
					(dstType.isMatrix() && args.size() == 1 && args[0]->getAsTyped() && args[0]->getAsTyped()->getType().isMatrix()))
				{
					int dstCols = dstType.getColsCount();
					int dstRows = dstType.getRowsCount();
					TIntermTyped* srcNode = args[0]->getAsTyped();
					std::string srcMat = EmitRValue(srcNode);
					int srcCols = srcNode->getType().getColsCount();
					int srcRows = srcNode->getType().getRowsCount();
					TType colType(EbtFloat, EbpUndefined, EvqTemporary, 1, dstRows);
					std::vector<std::string> colsOut;
					for (int c = 0; c < dstCols; ++c)
					{
						if (c < srcCols && dstRows <= srcRows)
						{
							std::string swz = ".";
							for (int r = 0; r < dstRows; ++r)
								swz += SwizzleChar(static_cast<uint32_t>(r));
							colsOut.push_back(srcMat + "[" + FormatIntLit(c, lang) + "]" + (dstRows == srcRows ? "" : swz));
						}
						else
						{
							std::vector<std::string> compExprs;
							for (int r = 0; r < dstRows; ++r)
							{
								if (c < srcCols && r < srcRows)
									compExprs.push_back(srcMat + "[" + FormatIntLit(c, lang) + "]." + SwizzleChar(static_cast<uint32_t>(r)));
								else
									compExprs.push_back(FormatFloatLit((c == r) ? 1.0f : 0.0f, lang));
							}
							colsOut.push_back(ConstructComposite(colType, compExprs, false));
						}
					}
					return ConstructComposite(dstType, colsOut, true);
				}

				TBasicType dstBt = dstType.getBasicType();
				std::vector<std::string> scalars;
				for (TIntermNode* child : args)
				{
					TIntermTyped* a = child->getAsTyped();
					if (!a) continue;
					std::string av = EmitRValue(a);
					const TType& at = a->getType();
					TBasicType abt = at.getBasicType();

					if (at.isMatrix())
					{
						int acols = at.getColsCount();
						int arows = at.getRowsCount();
						for (int c = 0; c < acols; ++c)
						{
							for (int r = 0; r < arows; ++r)
							{
								std::string sv = av + "[" + FormatIntLit(c, lang) + "]." + SwizzleChar(static_cast<uint32_t>(r));
								scalars.push_back(CastScalarExpr(sv, abt, dstBt));
							}
						}
					}
					else if (at.isVector())
					{
						int arows = at.getRowsCount();
						for (int r = 0; r < arows; ++r)
							scalars.push_back(CastScalarExpr(av + "." + SwizzleChar(static_cast<uint32_t>(r)), abt, dstBt));
					}
					else
					{
						scalars.push_back(CastScalarExpr(av, abt, dstBt));
					}
				}

				int needed = dstType.getInstanceSize();
				std::string zeroSc = (dstBt == EbtInt) ? FormatIntLit(0, lang) : (dstBt == EbtBool ? "false" : FormatFloatLit(0.0f, lang));
				if (scalars.empty())
					scalars.push_back(zeroSc);
				const bool singleScalarSplat = (scalars.size() == 1);
				while (static_cast<int>(scalars.size()) < needed)
					scalars.push_back(singleScalarSplat ? scalars[0] : zeroSc);

				if (dstType.isMatrix())
				{
					int cols = dstType.getColsCount();
					int rows = dstType.getRowsCount();
					std::string colT = GetVecTypeStr(EbtFloat, static_cast<uint32_t>(rows));
					std::vector<std::string> colsOut;
					for (int c = 0; c < cols; ++c)
					{
						std::ostringstream cs;
						cs << colT << "(";
						for (int r = 0; r < rows; ++r)
						{
							if (r > 0) cs << ", ";
							cs << scalars[static_cast<size_t>(c * rows + r)];
						}
						cs << ")";
						colsOut.push_back(cs.str());
					}
					return ConstructComposite(dstType, colsOut, true);
				}

				if (needed <= 1)
					return AllocTmp(GetScalarTypeStr(dstBt), scalars[0]);

				std::vector<std::string> take(scalars.begin(), scalars.begin() + needed);
				return ConstructComposite(dstType, take, true);
			}

			const SamplerResourceInfo* LookupSamplerSym(int symId) const
			{
				auto fit = fnSamplerParams.find(symId);
				if (fit != fnSamplerParams.end())
					return &fit->second;
				auto sit = samplerBySymId.find(symId);
				if (sit != samplerBySymId.end())
					return &sit->second;
				return nullptr;
			}

			std::string GetTextureParamTypeStr(ETexDim dim) const
			{
				if (lang == ETextLang::WGSL_1_0)
				{
					if (dim == ETexDim::DimCube) return "texture_cube<f32>";
					if (dim == ETexDim::Dim3D) return "texture_3d<f32>";
					return "texture_2d<f32>";
				}
				else
				{
					if (dim == ETexDim::DimCube) return "texturecube<float>";
					if (dim == ETexDim::Dim3D) return "texture3d<float>";
					return "texture2d<float>";
				}
			}

			std::string EmitTextureSample(TIntermAggregate* agg)
			{
				TNodeArray& args = agg->getNodes();
				TType v4Ast(EbtFloat, EbpUndefined, EvqTemporary, 1, 4);
				std::string v4Str = GetVecTypeStr(EbtFloat, 4);
				if (args.size() < 2)
					return AllocTmp(v4Str, GetZeroExpr(v4Ast));

				TIntermSymbol* sampSym = args[0]->getAsSymbolNode();
				if (!sampSym)
					return AllocTmp(v4Str, GetZeroExpr(v4Ast));

				const SamplerResourceInfo* psri = LookupSamplerSym(sampSym->getId());
				if (!psri)
					return AllocTmp(v4Str, GetZeroExpr(v4Ast));

				const SamplerResourceInfo& sri = *psri;
				std::string texRef = (lang == ETextLang::WGSL_1_0 || sri.isParamRef) ? sri.texIdent : ("_ctx." + sri.texIdent);
				std::string smpRef = (lang == ETextLang::WGSL_1_0 || sri.isParamRef) ? sri.smpIdent : ("_ctx." + sri.smpIdent);

				TIntermTyped* coordNode = args[1]->getAsTyped();
				std::string coordRaw = EmitRValue(coordNode);
				const TType& ct = coordNode->getType();

				TOperator op = agg->getOp();
				uint32_t coordDims = (sri.dim == ETexDim::Dim3D || sri.dim == ETexDim::DimCube) ? 3u : 2u;

				auto emitStandardSample = [&](const std::string& uvExpr) -> std::string {
					if (lang == ETextLang::WGSL_1_0)
					{
						if (isVertex)
							return AllocTmp(v4Str, "textureSampleLevel(" + texRef + ", " + smpRef + ", " + uvExpr + ", 0.0)");
						return AllocTmp(v4Str, "textureSample(" + texRef + ", " + smpRef + ", " + uvExpr + ")");
					}
					else
					{
						if (isVertex)
							return AllocTmp(v4Str, texRef + ".sample(" + smpRef + ", " + uvExpr + ", level(0.0f))");
						return AllocTmp(v4Str, texRef + ".sample(" + smpRef + ", " + uvExpr + ")");
					}
				};

				if (op == EOpTex2DProj || op == EOpTex3DProj || op == EOpTexCubeProj || op == EOpTex1DProj || op == EOpShadow2DProj)
				{
					std::string c4 = CoerceVectorOrScalar(coordRaw, ct, EbtFloat, 4);
					std::string num = CoerceVectorOrScalar(c4, v4Ast, EbtFloat, coordDims);
					std::string den = SplatScalar(c4 + ".w", EbtFloat, coordDims);
					std::string projUv = AllocTmp(GetVecTypeStr(EbtFloat, coordDims), "(" + num + " / " + den + ")");
					std::string sampled = emitStandardSample(projUv);
					return CoerceToType(sampled, v4Ast, agg->getType());
				}

				if (op == EOpTex2DLod || op == EOpTex3DLod || op == EOpTexCubeLod || op == EOpTex1DLod)
				{
					std::string c4 = CoerceVectorOrScalar(coordRaw, ct, EbtFloat, 4);
					std::string uv = CoerceVectorOrScalar(c4, v4Ast, EbtFloat, coordDims);
					std::string lod = c4 + ".w";
					std::string sampled = (lang == ETextLang::WGSL_1_0)
						? AllocTmp(v4Str, "textureSampleLevel(" + texRef + ", " + smpRef + ", " + uv + ", " + lod + ")")
						: AllocTmp(v4Str, texRef + ".sample(" + smpRef + ", " + uv + ", level(" + lod + "))");
					return CoerceToType(sampled, v4Ast, agg->getType());
				}

				if (op == EOpTex2DBias || op == EOpTex3DBias || op == EOpTexCubeBias || op == EOpTex1DBias)
				{
					std::string c4 = CoerceVectorOrScalar(coordRaw, ct, EbtFloat, 4);
					std::string uv = CoerceVectorOrScalar(c4, v4Ast, EbtFloat, coordDims);
					std::string b = c4 + ".w";
					if (isVertex)
						return CoerceToType(emitStandardSample(uv), v4Ast, agg->getType());
					std::string sampled = (lang == ETextLang::WGSL_1_0)
						? AllocTmp(v4Str, "textureSampleBias(" + texRef + ", " + smpRef + ", " + uv + ", " + b + ")")
						: AllocTmp(v4Str, texRef + ".sample(" + smpRef + ", " + uv + ", bias(" + b + "))");
					return CoerceToType(sampled, v4Ast, agg->getType());
				}

				if (args.size() >= 4 || op == EOpTex2DGrad || op == EOpTex3DGrad || op == EOpTexCubeGrad || op == EOpTex1DGrad)
				{
					std::string uv = CoerceVectorOrScalar(coordRaw, ct, EbtFloat, coordDims);
					TIntermTyped* ddxNode = args[2]->getAsTyped();
					TIntermTyped* ddyNode = args[3]->getAsTyped();
					std::string ddx = CoerceVectorOrScalar(EmitRValue(ddxNode), ddxNode->getType(), EbtFloat, coordDims);
					std::string ddy = CoerceVectorOrScalar(EmitRValue(ddyNode), ddyNode->getType(), EbtFloat, coordDims);
					std::string sampled;
					if (lang == ETextLang::WGSL_1_0)
					{
						sampled = AllocTmp(v4Str, "textureSampleGrad(" + texRef + ", " + smpRef + ", " + uv + ", " + ddx + ", " + ddy + ")");
					}
					else
					{
						const char* gradFn = (sri.dim == ETexDim::DimCube) ? "gradientcube" : (sri.dim == ETexDim::Dim3D ? "gradient3d" : "gradient2d");
						sampled = AllocTmp(v4Str, texRef + ".sample(" + smpRef + ", " + uv + ", " + gradFn + "(" + ddx + ", " + ddy + "))");
					}
					return CoerceToType(sampled, v4Ast, agg->getType());
				}

				std::string uv = CoerceVectorOrScalar(coordRaw, ct, EbtFloat, coordDims);
				return CoerceToType(emitStandardSample(uv), v4Ast, agg->getType());
			}

			std::string EmitUnaryExpr(TIntermUnary* un)
			{
				TOperator op = un->getOp();
				TIntermTyped* operand = un->getOperand();
				const TType& resType = un->getType();
				std::string resTyStr = GetValueTypeStr(resType);

				if (op == EOpPreIncrement || op == EOpPreDecrement || op == EOpPostIncrement || op == EOpPostDecrement)
				{
					LValueRef lv;
					if (EmitLValue(operand, lv))
					{
						std::string oldVal = LoadFromLValue(lv);
						uint32_t rows = static_cast<uint32_t>((std::max)(1, lv.resultType.getRowsCount()));
						bool isInt = (lv.resultType.getBasicType() == EbtInt);
						std::string step = isInt
							? SplatScalar(FormatIntLit(1, lang), EbtInt, rows)
							: SplatScalar(FormatFloatLit(1.0f, lang), EbtFloat, rows);
						const char* addSub = (op == EOpPreIncrement || op == EOpPostIncrement) ? "+" : "-";
						std::string newVal = AllocTmp(GetValueTypeStr(lv.resultType), "(" + oldVal + " " + addSub + " " + step + ")");
						StoreToLValue(lv, newVal);
						return (op == EOpPreIncrement || op == EOpPreDecrement) ? newVal : oldVal;
					}
				}

				if (op == EOpFclip)
				{
					std::string val = EmitRValue(operand);
					uint32_t rows = static_cast<uint32_t>((std::max)(1, operand->getType().getRowsCount()));
					val = CoerceVectorOrScalar(val, operand->getType(), EbtFloat, rows);
					std::string zero = SplatScalar(FormatFloatLit(0.0f, lang), EbtFloat, rows);
					std::string lt = AllocTmp(GetVecTypeStr(EbtBool, rows), "(" + val + " < " + zero + ")");
					std::string cond = (rows > 1) ? AllocTmp("bool", "any(" + lt + ")") : lt;
					if (!isVertex)
					{
						WriteIndent();
						fnOut << "if (" << cond << ") { " << (lang == ETextLang::WGSL_1_0 ? "discard;" : "discard_fragment();") << " }\n";
					}
					return "";
				}

				std::string v = EmitRValue(operand);
				const TType& ot = operand->getType();

				switch (op)
				{
				case EOpNegative:
				{
					if (resType.isMatrix())
					{
						int cols = resType.getColsCount();
						std::vector<std::string> negCols;
						for (int c = 0; c < cols; ++c)
							negCols.push_back("(-" + v + "[" + FormatIntLit(c, lang) + "])");
						return ConstructComposite(resType, negCols, true);
					}
					return AllocTmp(resTyStr, "(-" + v + ")");
				}
				case EOpLogicalNot:
				case EOpVectorLogicalNot:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					std::string bv = CoerceVectorOrScalar(v, ot, EbtBool, rows);
					return AllocTmp(GetVecTypeStr(EbtBool, rows), "(!" + bv + ")");
				}
				case EOpBitwiseNot:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					std::string iv = CoerceVectorOrScalar(v, ot, EbtInt, rows);
					return AllocTmp(GetVecTypeStr(EbtInt, rows), "(~" + iv + ")");
				}
				case EOpConvIntToBool:
				case EOpConvFloatToBool:
				case EOpConvBoolToFloat:
				case EOpConvIntToFloat:
				case EOpConvFloatToInt:
				case EOpConvBoolToInt:
					return CoerceVectorOrScalar(v, ot, resType.getBasicType(), static_cast<uint32_t>((std::max)(1, resType.getRowsCount())));

				case EOpAny:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, ot.getRowsCount()));
					std::string bv = CoerceVectorOrScalar(v, ot, EbtBool, rows);
					return (rows <= 1) ? bv : AllocTmp("bool", "any(" + bv + ")");
				}
				case EOpAll:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, ot.getRowsCount()));
					std::string bv = CoerceVectorOrScalar(v, ot, EbtBool, rows);
					return (rows <= 1) ? bv : AllocTmp("bool", "all(" + bv + ")");
				}
				case EOpSaturate:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					std::string fv = CoerceVectorOrScalar(v, ot, EbtFloat, rows);
					return AllocTmp(resTyStr, "saturate(" + fv + ")");
				}
				case EOpTranspose:
					return AllocTmp(resTyStr, "transpose(" + v + ")");
				case EOpDeterminant:
					return AllocTmp(GetScalarTypeStr(EbtFloat), "determinant(" + v + ")");
				case EOpLog10:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					std::string fv = CoerceVectorOrScalar(v, ot, EbtFloat, rows);
					std::string scale = SplatScalar(FormatFloatLit(0.30102999566f, lang), EbtFloat, rows);
					return AllocTmp(resTyStr, "(log2(" + fv + ") * " + scale + ")");
				}
				case EOpD3DCOLORtoUBYTE4:
				{
					std::string fv = CoerceVectorOrScalar(v, ot, EbtFloat, 4);
					std::string scale = SplatScalar(FormatFloatLit(255.001953f, lang), EbtFloat, 4);
					std::string scaled = AllocTmp(GetVecTypeStr(EbtFloat, 4), "(" + fv + ".zyxw * " + scale + ")");
					TType v4f(EbtFloat, EbpUndefined, EvqTemporary, 1, 4);
					return CoerceVectorOrScalar(scaled, v4f, resType.getBasicType(), 4);
				}
				case EOpDPdx:
				case EOpDPdy:
				case EOpFwidth:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					std::string fv = CoerceVectorOrScalar(v, ot, EbtFloat, rows);
					if (isVertex)
						return SplatScalar(FormatFloatLit(0.0f, lang), EbtFloat, rows);
					const char* fn = "fwidth";
					if (op == EOpDPdx) fn = (lang == ETextLang::WGSL_1_0) ? "dpdx" : "dfdx";
					else if (op == EOpDPdy) fn = (lang == ETextLang::WGSL_1_0) ? "dpdy" : "dfdy";
					return AllocTmp(resTyStr, std::string(fn) + "(" + fv + ")");
				}
				case EOpLength:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, ot.getRowsCount()));
					std::string fv = CoerceVectorOrScalar(v, ot, EbtFloat, rows);
					if (rows == 1)
						return AllocTmp(GetScalarTypeStr(EbtFloat), "abs(" + fv + ")");
					return AllocTmp(GetScalarTypeStr(EbtFloat), "length(" + fv + ")");
				}
				case EOpNormalize:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					std::string fv = CoerceVectorOrScalar(v, ot, EbtFloat, rows);
					if (rows == 1)
						return AllocTmp(resTyStr, "sign(" + fv + ")");
					return AllocTmp(resTyStr, "normalize(" + fv + ")");
				}
				default:
					break;
				}

				bool isInt = (resType.getBasicType() == EbtInt);
				const char* fnName = "abs";
				switch (op)
				{
				case EOpRadians:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					std::string fv = CoerceVectorOrScalar(v, ot, EbtFloat, rows);
					std::string k = SplatScalar(FormatFloatLit(0.01745329252f, lang), EbtFloat, rows);
					return AllocTmp(resTyStr, "(" + fv + " * " + k + ")");
				}
				case EOpDegrees:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					std::string fv = CoerceVectorOrScalar(v, ot, EbtFloat, rows);
					std::string k = SplatScalar(FormatFloatLit(57.295779513f, lang), EbtFloat, rows);
					return AllocTmp(resTyStr, "(" + fv + " * " + k + ")");
				}
				case EOpSin:         fnName = "sin"; break;
				case EOpCos:         fnName = "cos"; break;
				case EOpTan:         fnName = "tan"; break;
				case EOpAsin:        fnName = "asin"; break;
				case EOpAcos:        fnName = "acos"; break;
				case EOpAtan:        fnName = "atan"; break;
				case EOpExp:         fnName = "exp"; break;
				case EOpLog:         fnName = "log"; break;
				case EOpExp2:        fnName = "exp2"; break;
				case EOpLog2:        fnName = "log2"; break;
				case EOpSqrt:        fnName = "sqrt"; break;
				case EOpInverseSqrt: fnName = (lang == ETextLang::WGSL_1_0) ? "inverseSqrt" : "rsqrt"; break;
				case EOpAbs:         fnName = "abs"; break;
				case EOpSign:        fnName = "sign"; break;
				case EOpFloor:       fnName = "floor"; break;
				case EOpCeil:        fnName = "ceil"; break;
				case EOpFract:       fnName = "fract"; break;
				case EOpRound:       fnName = "round"; break;
				case EOpTrunc:       fnName = "trunc"; break;
				default:             fnName = "abs"; break;
				}

				uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
				std::string argVal = CoerceVectorOrScalar(v, ot, isInt ? EbtInt : EbtFloat, rows);
				return AllocTmp(resTyStr, std::string(fnName) + "(" + argVal + ")");
			}

			std::string EmitAggregateExpr(TIntermAggregate* agg)
			{
				TOperator op = agg->getOp();
				TNodeArray& args = agg->getNodes();
				const TType& resType = agg->getType();
				std::string resTyStr = GetValueTypeStr(resType);

				if (op >= EOpConstructInt && op <= EOpConstructMat3x3FromMat)
					return EmitConstructor(agg);

				if (op >= EOpTex1D && op <= EOpTex2DArrayBias)
					return EmitTextureSample(agg);

				if (op == EOpComma)
				{
					std::string last;
					for (TIntermNode* child : args)
					{
						if (child && child->getAsTyped())
							last = EmitRValue(child->getAsTyped());
					}
					return last;
				}

				if (op == EOpFunctionCall)
				{
					std::string mangled = agg->getName().c_str();
					auto fit = funcsByMangled.find(mangled);
					if (fit == funcsByMangled.end())
						return resType.getBasicType() == EbtVoid ? "" : AllocTmp(resTyStr, GetZeroExpr(resType));

					const FuncRecord& callee = fit->second;
					struct CopyBackItem
					{
						LValueRef lv;
						std::string tmpVar;
						TType paramType;
					};
					std::vector<CopyBackItem> copyBacks;
					std::vector<std::string> callArgs;
					if (lang == ETextLang::MSL_2_0)
						callArgs.push_back("_ctx");

					for (size_t i = 0; i < callee.params.size() && i < args.size(); ++i)
					{
						const ParamSpec& ps = callee.params[i];
						TIntermTyped* argNode = args[i]->getAsTyped();
						if (ps.isSampler)
						{
							TIntermSymbol* argSym = argNode ? argNode->getAsSymbolNode() : nullptr;
							const SamplerResourceInfo* argSri = argSym ? LookupSamplerSym(argSym->getId()) : nullptr;
							if (!argSri || argSri->dim != ps.samplerDim)
							{
								argSri = nullptr;
								for (const auto& kv : samplerBySymId)
								{
									if (kv.second.dim == ps.samplerDim) { argSri = &kv.second; break; }
								}
							}
							if (argSri)
							{
								std::string tArg = (lang == ETextLang::WGSL_1_0 || argSri->isParamRef) ? argSri->texIdent : ("_ctx." + argSri->texIdent);
								std::string sArg = (lang == ETextLang::WGSL_1_0 || argSri->isParamRef) ? argSri->smpIdent : ("_ctx." + argSri->smpIdent);
								callArgs.push_back(tArg);
								callArgs.push_back(sArg);
							}
						}
						else if (ps.passByPtr)
						{
							std::string initVal = (ps.qualifier == EvqInOut)
								? CoerceToType(EmitRValue(argNode), argNode->getType(), ps.astType)
								: GetZeroExpr(ps.astType);
							std::string tmpVar = AllocMutableLocal(GetValueTypeStr(ps.astType), initVal);
							LValueRef argLv;
							if (EmitLValue(argNode, argLv))
								copyBacks.push_back({ argLv, tmpVar, ps.astType });
							callArgs.push_back(lang == ETextLang::WGSL_1_0 ? ("&" + tmpVar) : tmpVar);
						}
						else
						{
							std::string val = EmitRValue(argNode);
							callArgs.push_back(CoerceToType(val, argNode->getType(), ps.astType));
						}
					}

					std::ostringstream callOss;
					callOss << callee.emittedName << "(";
					for (size_t i = 0; i < callArgs.size(); ++i)
					{
						if (i > 0) callOss << ", ";
						callOss << callArgs[i];
					}
					callOss << ")";

					std::string retVal;
					if (callee.returnAstType.getBasicType() == EbtVoid)
					{
						WriteIndent();
						fnOut << callOss.str() << ";\n";
					}
					else
					{
						retVal = AllocTmp(GetValueTypeStr(callee.returnAstType), callOss.str());
					}

					for (const auto& cb : copyBacks)
					{
						std::string outVal = CoerceToType(cb.tmpVar, cb.paramType, cb.lv.resultType);
						StoreToLValue(cb.lv, outVal);
					}
					return retVal;
				}

				if (op == EOpMul && args.size() == 2)
				{
					TIntermTyped* a0 = args[0]->getAsTyped();
					TIntermTyped* a1 = args[1]->getAsTyped();
					std::string v0 = EmitRValue(a0);
					std::string v1 = EmitRValue(a1);
					const TType& t0 = a0->getType();
					const TType& t1 = a1->getType();
					if ((t0.isMatrix() && t1.isMatrix()) || (t0.isVector() && t1.isMatrix()) || (t0.isMatrix() && t1.isVector()))
					{
						return AllocTmp(resTyStr, "(" + v1 + " * " + v0 + ")");
					}
					return ApplyNumericBinaryOp(EOpMul, v0, t0, v1, t1, resType);
				}

				if (op == EOpModf && args.size() == 2)
				{
					TIntermTyped* xNode = args[0]->getAsTyped();
					TIntermTyped* iNode = args[1]->getAsTyped();
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					std::string xVal = CoerceVectorOrScalar(EmitRValue(xNode), xNode->getType(), EbtFloat, rows);
					std::string trVal = AllocTmp(resTyStr, "trunc(" + xVal + ")");
					LValueRef outLv;
					if (EmitLValue(iNode, outLv))
					{
						std::string ci = CoerceVectorOrScalar(trVal, resType, outLv.resultType.getBasicType(), static_cast<uint32_t>((std::max)(1, outLv.resultType.getRowsCount())));
						StoreToLValue(outLv, ci);
					}
					return AllocTmp(resTyStr, "(" + xVal + " - " + trVal + ")");
				}

				if (op == EOpSinCos && args.size() == 3)
				{
					TIntermTyped* xNode = args[0]->getAsTyped();
					uint32_t rows = static_cast<uint32_t>((std::max)(1, xNode->getType().getRowsCount()));
					std::string vecF = GetVecTypeStr(EbtFloat, rows);
					std::string xVal = CoerceVectorOrScalar(EmitRValue(xNode), xNode->getType(), EbtFloat, rows);
					std::string sVal = AllocTmp(vecF, "sin(" + xVal + ")");
					std::string cVal = AllocTmp(vecF, "cos(" + xVal + ")");
					TType ft(EbtFloat, EbpUndefined, EvqTemporary, 1, static_cast<int>(rows));
					LValueRef slv, clv;
					if (EmitLValue(args[1]->getAsTyped(), slv))
						StoreToLValue(slv, CoerceToType(sVal, ft, slv.resultType));
					if (EmitLValue(args[2]->getAsTyped(), clv))
						StoreToLValue(clv, CoerceToType(cVal, ft, clv.resultType));
					return "";
				}

				if (op == EOpLit && args.size() >= 3)
				{
					TIntermTyped* a0 = args[0]->getAsTyped();
					TIntermTyped* a1 = args[1]->getAsTyped();
					TIntermTyped* a2 = args[2]->getAsTyped();
					std::string nl = CoerceVectorOrScalar(EmitRValue(a0), a0->getType(), EbtFloat, 1);
					std::string nh = CoerceVectorOrScalar(EmitRValue(a1), a1->getType(), EbtFloat, 1);
					std::string m  = CoerceVectorOrScalar(EmitRValue(a2), a2->getType(), EbtFloat, 1);
					std::string f0 = FormatFloatLit(0.0f, lang), f1 = FormatFloatLit(1.0f, lang);
					std::string fTy = GetScalarTypeStr(EbtFloat);
					std::string diff = AllocTmp(fTy, "max(" + nl + ", " + f0 + ")");
					std::string nhc  = AllocTmp(fTy, "max(" + nh + ", " + f0 + ")");
					std::string p    = AllocTmp(fTy, "pow(" + nhc + ", " + m + ")");
					std::string spec = AllocTmp(fTy, "select(" + f0 + ", " + p + ", (" + nl + " > " + f0 + "))");
					return AllocTmp(GetVecTypeStr(EbtFloat, 4), GetVecTypeStr(EbtFloat, 4) + "(" + f1 + ", " + diff + ", " + spec + ", " + f1 + ")");
				}

				if (op == EOpLessThan || op == EOpGreaterThan || op == EOpLessThanEqual ||
					op == EOpGreaterThanEqual || op == EOpVectorEqual || op == EOpVectorNotEqual)
				{
					return EmitComparison(op, args[0]->getAsTyped(), args[1]->getAsTyped(), resType);
				}

				if (op == EOpDot && args.size() == 2)
				{
					TIntermTyped* a0 = args[0]->getAsTyped();
					TIntermTyped* a1 = args[1]->getAsTyped();
					uint32_t rows = static_cast<uint32_t>((std::max)({ 1, a0->getType().getRowsCount(), a1->getType().getRowsCount() }));
					std::string v0 = CoerceVectorOrScalar(EmitRValue(a0), a0->getType(), EbtFloat, rows);
					std::string v1 = CoerceVectorOrScalar(EmitRValue(a1), a1->getType(), EbtFloat, rows);
					if (rows == 1)
						return AllocTmp(GetScalarTypeStr(EbtFloat), "(" + v0 + " * " + v1 + ")");
					return AllocTmp(GetScalarTypeStr(EbtFloat), "dot(" + v0 + ", " + v1 + ")");
				}

				if (op == EOpMod && args.size() == 2)
				{
					TIntermTyped* a0 = args[0]->getAsTyped();
					TIntermTyped* a1 = args[1]->getAsTyped();
					return ApplyNumericBinaryOp(EOpMod, EmitRValue(a0), a0->getType(), EmitRValue(a1), a1->getType(), resType);
				}

				if (op == EOpDistance && args.size() == 2)
				{
					TIntermTyped* a0 = args[0]->getAsTyped();
					TIntermTyped* a1 = args[1]->getAsTyped();
					uint32_t r = static_cast<uint32_t>((std::max)({ 1, a0->getType().getRowsCount(), a1->getType().getRowsCount() }));
					std::string v0 = CoerceVectorOrScalar(EmitRValue(a0), a0->getType(), EbtFloat, r);
					std::string v1 = CoerceVectorOrScalar(EmitRValue(a1), a1->getType(), EbtFloat, r);
					if (r == 1)
						return AllocTmp(GetScalarTypeStr(EbtFloat), "abs(" + v0 + " - " + v1 + ")");
					return AllocTmp(GetScalarTypeStr(EbtFloat), "distance(" + v0 + ", " + v1 + ")");
				}

				if (op == EOpRefract && args.size() == 3)
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					TIntermTyped* a0 = args[0]->getAsTyped();
					TIntermTyped* a1 = args[1]->getAsTyped();
					TIntermTyped* a2 = args[2]->getAsTyped();
					std::string v0 = CoerceVectorOrScalar(EmitRValue(a0), a0->getType(), EbtFloat, rows);
					std::string v1 = CoerceVectorOrScalar(EmitRValue(a1), a1->getType(), EbtFloat, rows);
					std::string eta = CoerceVectorOrScalar(EmitRValue(a2), a2->getType(), EbtFloat, 1);
					return AllocTmp(resTyStr, "refract(" + v0 + ", " + v1 + ", " + eta + ")");
				}

				if (op == EOpLdexp && args.size() == 2)
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					TIntermTyped* a0 = args[0]->getAsTyped();
					TIntermTyped* a1 = args[1]->getAsTyped();
					std::string v0 = CoerceVectorOrScalar(EmitRValue(a0), a0->getType(), EbtFloat, rows);
					std::string e  = CoerceVectorOrScalar(EmitRValue(a1), a1->getType(), EbtInt, rows);
					return AllocTmp(resTyStr, "ldexp(" + v0 + ", " + e + ")");
				}

				uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
				bool isInt = (resType.getBasicType() == EbtInt);
				std::vector<std::string> evalArgs;
				for (TIntermNode* child : args)
				{
					TIntermTyped* a = child->getAsTyped();
					evalArgs.push_back(CoerceVectorOrScalar(EmitRValue(a), a->getType(), isInt ? EbtInt : EbtFloat, rows));
				}

				const char* fn = "max";
				switch (op)
				{
				case EOpMin:         fn = "min"; break;
				case EOpMax:         fn = "max"; break;
				case EOpClamp:       fn = "clamp"; break;
				case EOpMix:         fn = "mix"; break;
				case EOpPow:         fn = "pow"; break;
				case EOpAtan2:       fn = "atan2"; break;
				case EOpStep:        fn = "step"; break;
				case EOpSmoothStep:  fn = "smoothstep"; break;
				case EOpCross:       fn = "cross"; break;
				case EOpReflect:     fn = "reflect"; break;
				case EOpFaceForward: fn = "faceforward"; break;
				default:             fn = "max"; break;
				}

				std::ostringstream oss;
				oss << fn << "(";
				for (size_t i = 0; i < evalArgs.size(); ++i)
				{
					if (i > 0) oss << ", ";
					oss << evalArgs[i];
				}
				oss << ")";
				return AllocTmp(resTyStr, oss.str());
			}

			std::string EmitBinaryExpr(TIntermBinary* bin)
			{
				TOperator op = bin->getOp();
				TIntermTyped* leftNode = bin->getLeft();
				TIntermTyped* rightNode = bin->getRight();
				const TType& resType = bin->getType();

				if (op == EOpAssign)
				{
					if (TIntermBinary* lhsBin = leftNode ? leftNode->getAsBinaryNode() : nullptr)
					{
						if (lhsBin->getOp() == EOpMatrixSwizzle)
						{
							LValueRef matLv;
							if (EmitLValue(lhsBin->getLeft(), matLv))
							{
								std::vector<uint32_t> swiz;
								ExtractSwizzleIndices(lhsBin->getRight(), swiz);
								std::string rhsVal = EmitRValue(rightNode);
								rhsVal = CoerceVectorOrScalar(rhsVal, rightNode->getType(), EbtFloat, static_cast<uint32_t>((std::max)(size_t(1), swiz.size())));
								uint32_t cols = static_cast<uint32_t>((std::max)(1, matLv.resultType.getColsCount()));
								uint32_t rows = static_cast<uint32_t>((std::max)(1, matLv.resultType.getRowsCount()));
								for (size_t i = 0; i < swiz.size(); ++i)
								{
									std::string compVal = (swiz.size() > 1) ? (rhsVal + "." + SwizzleChar(static_cast<uint32_t>(i))) : rhsVal;
									uint32_t cIdx = (std::min)(swiz[i] % 4u, cols - 1u);
									uint32_t rIdx = (std::min)(swiz[i] / 4u, rows - 1u);
									WriteIndent();
									fnOut << matLv.baseExpr << "[" << FormatIntLit(static_cast<int32_t>(cIdx), lang) << "]." << SwizzleChar(rIdx) << " = " << compVal << ";\n";
								}
								return rhsVal;
							}
						}
					}

					LValueRef lv;
					std::string rhsVal = EmitRValue(rightNode);
					if (EmitLValue(leftNode, lv))
					{
						rhsVal = CoerceToType(rhsVal, rightNode->getType(), lv.resultType);
						StoreToLValue(lv, rhsVal);
					}
					return rhsVal;
				}

				if (op >= EOpAddAssign && op <= EOpRightShiftAssign)
				{
					LValueRef lv;
					if (EmitLValue(leftNode, lv))
					{
						std::string oldVal = LoadFromLValue(lv);
						std::string rhsVal = EmitRValue(rightNode);
						std::string newVal;
						if (op == EOpVectorTimesMatrixAssign)
						{
							newVal = AllocTmp(GetValueTypeStr(lv.resultType), "(" + rhsVal + " * " + oldVal + ")");
						}
						else if (op == EOpMatrixTimesMatrixAssign)
						{
							newVal = ApplyNumericBinaryOp(EOpMatrixTimesMatrix, oldVal, lv.resultType, rhsVal, rightNode->getType(), lv.resultType);
						}
						else
						{
							newVal = ApplyNumericBinaryOp(op, oldVal, lv.resultType, rhsVal, rightNode->getType(), lv.resultType);
						}
						StoreToLValue(lv, newVal);
						return newVal;
					}
				}

				if (op == EOpVectorSwizzle)
				{
					std::string lhsVal = EmitRValue(leftNode);
					const TType& lt = leftNode->getType();
					std::vector<uint32_t> swiz;
					ExtractSwizzleIndices(rightNode, swiz);
					if (lt.isScalar())
					{
						return (swiz.size() <= 1) ? lhsVal : SplatScalar(lhsVal, lt.getBasicType(), static_cast<uint32_t>(swiz.size()));
					}
					uint32_t maxIdx = static_cast<uint32_t>((std::max)(1, lt.getRowsCount()) - 1);
					std::string swz = ".";
					for (uint32_t idx : swiz)
						swz += SwizzleChar((std::min)(idx, maxIdx));
					if (swiz.empty())
						swz += "x";
					return AllocTmp(GetValueTypeStr(resType), lhsVal + swz);
				}

				if (op == EOpMatrixSwizzle)
				{
					std::string matVal = EmitRValue(leftNode);
					const TType& lt = leftNode->getType();
					uint32_t cols = static_cast<uint32_t>((std::max)(1, lt.getColsCount()));
					uint32_t rows = static_cast<uint32_t>((std::max)(1, lt.getRowsCount()));
					std::vector<uint32_t> swiz;
					ExtractSwizzleIndices(rightNode, swiz);
					std::vector<std::string> comps;
					for (uint32_t s : swiz)
					{
						uint32_t cIdx = (std::min)(s % 4u, cols - 1u);
						uint32_t rIdx = (std::min)(s / 4u, rows - 1u);
						comps.push_back(matVal + "[" + FormatIntLit(static_cast<int32_t>(cIdx), lang) + "]." + SwizzleChar(rIdx));
					}
					if (comps.size() <= 1)
						return comps.empty() ? FormatFloatLit(0.0f, lang) : AllocTmp(GetScalarTypeStr(EbtFloat), comps[0]);
					return ConstructComposite(resType, comps, true);
				}

				if (op == EOpIndexDirectStruct || op == EOpIndexDirect || op == EOpIndexIndirect)
				{
					std::string uniExpr;
					if (TryEmitUniformLoad(bin, uniExpr))
						return uniExpr;

					LValueRef lv;
					if (EmitLValue(bin, lv))
						return LoadFromLValue(lv);

					std::string lhsVal = EmitRValue(leftNode);
					if (op == EOpIndexDirectStruct)
					{
						const StructInfo& si = RegisterStruct(leftNode->getType());
						int idx = (std::max)(0, ExtractImmediateInt(rightNode));
						if (static_cast<size_t>(idx) < si.fieldNames.size())
							return AllocTmp(GetValueTypeStr(resType), lhsVal + "." + si.fieldNames[static_cast<size_t>(idx)]);
						return AllocTmp(GetValueTypeStr(resType), GetZeroExpr(resType));
					}
					if (rightNode->getAsConstant() != nullptr)
					{
						int idx = (std::max)(0, ExtractImmediateInt(rightNode));
						return AllocTmp(GetValueTypeStr(resType), lhsVal + "[" + FormatIntLit(idx, lang) + "]");
					}
					std::string tmpArr = AllocMutableLocal(GetValueTypeStr(leftNode->getType()), lhsVal);
					std::string dynIdx = CoerceVectorOrScalar(EmitRValue(rightNode), rightNode->getType(), EbtInt, 1);
					return AllocTmp(GetValueTypeStr(resType), tmpArr + "[" + dynIdx + "]");
				}

				if (op == EOpLogicalAnd || op == EOpLogicalOr || op == EOpLogicalXor)
				{
					if (resType.isVector() || op == EOpLogicalXor)
					{
						uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
						std::string l = CoerceVectorOrScalar(EmitRValue(leftNode), leftNode->getType(), EbtBool, rows);
						std::string r = CoerceVectorOrScalar(EmitRValue(rightNode), rightNode->getType(), EbtBool, rows);
						const char* lop = "!=";
						if (op == EOpLogicalAnd)
							lop = (lang == ETextLang::WGSL_1_0 && rows > 1) ? "&" : "&&";
						else if (op == EOpLogicalOr)
							lop = (lang == ETextLang::WGSL_1_0 && rows > 1) ? "|" : "||";
						return AllocTmp(GetVecTypeStr(EbtBool, rows), "(" + l + " " + lop + " " + r + ")");
					}

					std::string resVar = AllocMutableLocal("bool", CoerceVectorOrScalar(EmitRValue(leftNode), leftNode->getType(), EbtBool, 1));
					WriteIndent();
					if (op == EOpLogicalAnd)
						fnOut << "if (" << resVar << ")\n";
					else
						fnOut << "if (!" << resVar << ")\n";
					WriteIndent();
					fnOut << "{\n";
					++indentLevel;
					std::string rhsCond = CoerceVectorOrScalar(EmitRValue(rightNode), rightNode->getType(), EbtBool, 1);
					WriteIndent();
					fnOut << resVar << " = " << rhsCond << ";\n";
					--indentLevel;
					WriteIndent();
					fnOut << "}\n";
					return resVar;
				}

				if (op == EOpEqual || op == EOpNotEqual || op == EOpVectorEqual || op == EOpVectorNotEqual ||
					op == EOpLessThan || op == EOpGreaterThan || op == EOpLessThanEqual || op == EOpGreaterThanEqual)
				{
					return EmitComparison(op, leftNode, rightNode, resType);
				}

				std::string l = EmitRValue(leftNode);
				std::string r = EmitRValue(rightNode);
				return ApplyNumericBinaryOp(op, l, leftNode->getType(), r, rightNode->getType(), resType);
			}

			std::string EmitSelection(TIntermSelection* sel)
			{
				const TType& resType = sel->getType();
				TIntermTyped* condTyped = sel->getCondition() ? sel->getCondition()->getAsTyped() : nullptr;

				if (resType.getBasicType() == EbtVoid)
				{
					std::string condVal = condTyped
						? CoerceVectorOrScalar(EmitRValue(condTyped), condTyped->getType(), EbtBool, 1)
						: "false";
					WriteIndent();
					fnOut << "if (" << condVal << ")\n";
					WriteIndent();
					fnOut << "{\n";
					++indentLevel;
					bool savedTerm = blockTerminated;
					blockTerminated = false;
					if (sel->getTrueBlock())
						EmitStatement(sel->getTrueBlock());
					bool thenTerm = blockTerminated;
					--indentLevel;
					WriteIndent();
					fnOut << "}\n";

					bool elseTerm = false;
					if (sel->getFalseBlock())
					{
						WriteIndent();
						fnOut << "else\n";
						WriteIndent();
						fnOut << "{\n";
						++indentLevel;
						blockTerminated = false;
						EmitStatement(sel->getFalseBlock());
						elseTerm = blockTerminated;
						--indentLevel;
						WriteIndent();
						fnOut << "}\n";
					}
					blockTerminated = savedTerm || (thenTerm && elseTerm);
					return "";
				}

				if (condTyped && condTyped->isVector())
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					std::string cVal = CoerceVectorOrScalar(EmitRValue(condTyped), condTyped->getType(), EbtBool, rows);
					TIntermTyped* tb = sel->getTrueBlock()->getAsTyped();
					TIntermTyped* fb = sel->getFalseBlock()->getAsTyped();
					std::string tVal = CoerceToType(EmitRValue(tb), tb->getType(), resType);
					std::string fVal = CoerceToType(EmitRValue(fb), fb->getType(), resType);
					return AllocTmp(GetValueTypeStr(resType), "select(" + fVal + ", " + tVal + ", " + cVal + ")");
				}

				std::string resVar = AllocMutableLocal(GetValueTypeStr(resType), GetZeroExpr(resType));
				std::string condVal = condTyped
					? CoerceVectorOrScalar(EmitRValue(condTyped), condTyped->getType(), EbtBool, 1)
					: "false";
				WriteIndent();
				fnOut << "if (" << condVal << ")\n";
				WriteIndent();
				fnOut << "{\n";
				++indentLevel;
				TIntermTyped* tb = sel->getTrueBlock()->getAsTyped();
				std::string tVal = CoerceToType(EmitRValue(tb), tb->getType(), resType);
				WriteIndent();
				fnOut << resVar << " = " << tVal << ";\n";
				--indentLevel;
				WriteIndent();
				fnOut << "}\n";
				WriteIndent();
				fnOut << "else\n";
				WriteIndent();
				fnOut << "{\n";
				++indentLevel;
				TIntermTyped* fb = sel->getFalseBlock()->getAsTyped();
				std::string fVal = CoerceToType(EmitRValue(fb), fb->getType(), resType);
				WriteIndent();
				fnOut << resVar << " = " << fVal << ";\n";
				--indentLevel;
				WriteIndent();
				fnOut << "}\n";
				return resVar;
			}

			std::string EmitRValue(TIntermTyped* node)
			{
				if (!node)
					return "";

				std::string uniVal;
				if (TryEmitUniformLoad(node, uniVal))
					return uniVal;

				if (TIntermConstant* c = node->getAsConstant())
					return EmitConstantNode(c);

				if (TIntermSymbol* s = node->getAsSymbolNode())
				{
					LValueRef lv;
					if (EmitLValue(s, lv))
						return LoadFromLValue(lv);
					return AllocTmp(GetValueTypeStr(s->getType()), GetZeroExpr(s->getType()));
				}

				if (TIntermBinary* b = node->getAsBinaryNode())
					return EmitBinaryExpr(b);

				if (TIntermAggregate* a = node->getAsAggregate())
					return EmitAggregateExpr(a);

				if (TIntermSelection* sel = node->getAsSelectionNode())
					return EmitSelection(sel);

				SingleNodeClassifier cls;
				node->traverse(&cls);
				if (cls.unaryNode)
					return EmitUnaryExpr(cls.unaryNode);

				return AllocTmp(GetValueTypeStr(node->getType()), GetZeroExpr(node->getType()));
			}

			void EmitDeclarationStmt(TIntermDeclaration* decl)
			{
				TIntermTyped* inner = decl ? decl->getDeclaration() : nullptr;
				if (!inner || decl->getType().getBasicType() == EbtTexture || IsSampler(decl->getType().getBasicType()))
					return;

				if (TIntermSymbol* sym = inner->getAsSymbolNode())
				{
					std::string localName = AllocMutableLocal(GetValueTypeStr(sym->getType()), GetZeroExpr(sym->getType()));
					fnLocals[sym->getId()] = { localName, false };
					return;
				}

				if (TIntermBinary* assign = inner->getAsBinaryNode())
				{
					TIntermSymbol* sym = assign->getLeft() ? assign->getLeft()->getAsSymbolNode() : nullptr;
					if (!sym)
						return;
					const TType& symType = sym->getType();
					TIntermTyped* rhs = assign->getRight();
					if (!rhs)
					{
						std::string localName = AllocMutableLocal(GetValueTypeStr(symType), GetZeroExpr(symType));
						fnLocals[sym->getId()] = { localName, false };
						return;
					}

					if (decl->containsArrayInitialization() && rhs->getAsAggregate() && rhs->getAsAggregate()->getOp() != EOpConstructArray)
					{
						TType elemT = symType;
						elemT.clearArrayness();
						int arrLen = (std::max)(1, symType.getArraySize());
						TNodeArray& initNodes = rhs->getAsAggregate()->getNodes();
						std::vector<std::string> elems;
						elems.reserve(static_cast<size_t>(arrLen));
						for (int i = 0; i < arrLen; ++i)
						{
							if (static_cast<size_t>(i) < initNodes.size() && initNodes[static_cast<size_t>(i)]->getAsTyped())
							{
								TIntermTyped* inNode = initNodes[static_cast<size_t>(i)]->getAsTyped();
								elems.push_back(CoerceToType(EmitRValue(inNode), inNode->getType(), elemT));
							}
							else
							{
								elems.push_back(GetZeroExpr(elemT));
							}
						}
						std::string arrVal = ConstructComposite(symType, elems, false);
						std::string localName = AllocMutableLocal(GetValueTypeStr(symType), arrVal);
						fnLocals[sym->getId()] = { localName, false };
					}
					else
					{
						std::string initVal = CoerceToType(EmitRValue(rhs), rhs->getType(), symType);
						std::string localName = AllocMutableLocal(GetValueTypeStr(symType), initVal);
						fnLocals[sym->getId()] = { localName, false };
					}
				}
			}

			void EmitLoopStmt(TIntermLoop* loop)
			{
				TLoopType lt = loop->getType();
				if (lang == ETextLang::WGSL_1_0)
				{
					// Canonical WGSL 1.0 `loop { ... continuing { ... } }` handles for, while, and do-while uniformly!
					WriteIndent();
					fnOut << "loop\n";
					WriteIndent();
					fnOut << "{\n";
					++indentLevel;

					if (lt != ELoopDoWhile && loop->getCondition())
					{
						std::string condVal = CoerceVectorOrScalar(EmitRValue(loop->getCondition()), loop->getCondition()->getType(), EbtBool, 1);
						WriteIndent();
						fnOut << "if (!(" << condVal << ")) { break; }\n";
					}

					bool savedTerm = blockTerminated;
					blockTerminated = false;
					if (loop->getBody())
						EmitStatement(loop->getBody());
					blockTerminated = false;

					if (loop->getExpression() || (lt == ELoopDoWhile && loop->getCondition()))
					{
						WriteIndent();
						fnOut << "continuing\n";
						WriteIndent();
						fnOut << "{\n";
						++indentLevel;
						if (loop->getExpression())
							EmitRValue(loop->getExpression());
						if (lt == ELoopDoWhile && loop->getCondition())
						{
							std::string condVal = CoerceVectorOrScalar(EmitRValue(loop->getCondition()), loop->getCondition()->getType(), EbtBool, 1);
							WriteIndent();
							fnOut << "break if (!(" << condVal << "));\n";
						}
						--indentLevel;
						WriteIndent();
						fnOut << "}\n";
					}

					--indentLevel;
					WriteIndent();
					fnOut << "}\n";
					blockTerminated = savedTerm;
					return;
				}

				// MSL 2.0 (C++14): unified `for (bool _first = true; ; _first = false)` handles for, while, and do-while
				// with continue-safe step expressions and multi-statement SSA condition temporaries.
				std::string firstFlag = "lp_" + std::to_string(nextTmpId++);
				WriteIndent();
				fnOut << "for (bool " << firstFlag << " = true; ; " << firstFlag << " = false)\n";
				WriteIndent();
				fnOut << "{\n";
				++indentLevel;

				if (lt == ELoopDoWhile)
				{
					if (loop->getCondition())
					{
						WriteIndent();
						fnOut << "if (!" << firstFlag << ")\n";
						WriteIndent();
						fnOut << "{\n";
						++indentLevel;
						std::string condVal = CoerceVectorOrScalar(EmitRValue(loop->getCondition()), loop->getCondition()->getType(), EbtBool, 1);
						WriteIndent();
						fnOut << "if (!(" << condVal << ")) { break; }\n";
						--indentLevel;
						WriteIndent();
						fnOut << "}\n";
					}
				}
				else
				{
					if (loop->getExpression())
					{
						WriteIndent();
						fnOut << "if (!" << firstFlag << ")\n";
						WriteIndent();
						fnOut << "{\n";
						++indentLevel;
						EmitRValue(loop->getExpression());
						--indentLevel;
						WriteIndent();
						fnOut << "}\n";
					}
					if (loop->getCondition())
					{
						std::string condVal = CoerceVectorOrScalar(EmitRValue(loop->getCondition()), loop->getCondition()->getType(), EbtBool, 1);
						WriteIndent();
						fnOut << "if (!(" << condVal << ")) { break; }\n";
					}
				}

				bool savedTerm = blockTerminated;
				blockTerminated = false;
				if (loop->getBody())
					EmitStatement(loop->getBody());
				blockTerminated = savedTerm;

				--indentLevel;
				WriteIndent();
				fnOut << "}\n";
			}

			void EmitBranchStmt(TIntermBranch* br)
			{
				if (blockTerminated)
					return;
				switch (br->getFlowOp())
				{
				case EOpKill:
					if (!isVertex)
					{
						WriteIndent();
						fnOut << (lang == ETextLang::WGSL_1_0 ? "discard;\n" : "discard_fragment();\n");
					}
					break;
				case EOpBreak:
					WriteIndent();
					fnOut << "break;\n";
					blockTerminated = true;
					break;
				case EOpContinue:
					WriteIndent();
					fnOut << "continue;\n";
					blockTerminated = true;
					break;
				case EOpReturn:
					if (br->getExpression() && currentFnRetAstType.getBasicType() != EbtVoid)
					{
						std::string rv = EmitRValue(br->getExpression());
						rv = CoerceToType(rv, br->getExpression()->getType(), currentFnRetAstType);
						WriteIndent();
						fnOut << "return " << rv << ";\n";
					}
					else
					{
						WriteIndent();
						fnOut << "return;\n";
					}
					blockTerminated = true;
					break;
				default:
					break;
				}
			}

			void EmitStatement(TIntermNode* node)
			{
				if (!node || blockTerminated)
					return;

				if (TIntermAggregate* agg = node->getAsAggregate())
				{
					if (agg->getOp() == EOpSequence)
					{
						for (TIntermNode* child : agg->getNodes())
						{
							if (blockTerminated)
								break;
							EmitStatement(child);
						}
						return;
					}
					if (agg->getOp() == EOpFunction || agg->getOp() == EOpParameters)
						return;
					EmitAggregateExpr(agg);
					return;
				}

				if (TIntermDeclaration* decl = node->getAsDeclaration())
				{
					EmitDeclarationStmt(decl);
					return;
				}

				if (TIntermSelection* sel = node->getAsSelectionNode())
				{
					EmitSelection(sel);
					return;
				}

				SingleNodeClassifier cls;
				node->traverse(&cls);
				if (cls.loopNode)
				{
					EmitLoopStmt(cls.loopNode);
					return;
				}
				if (cls.branchNode)
				{
					EmitBranchStmt(cls.branchNode);
					return;
				}
				if (TIntermTyped* typed = node->getAsTyped())
				{
					EmitRValue(typed);
				}
			}

			std::string EmitUserFunction(const FuncRecord& fn, const std::vector<TIntermNode*>& bodyNodes)
			{
				fnOut.str("");
				fnOut.clear();
				fnLocals.clear();
				fnSamplerParams.clear();
				indentLevel = 0;
				blockTerminated = false;
				currentFnRetAstType = fn.returnAstType;

				const bool isVoid = (fn.returnAstType.getBasicType() == EbtVoid);
				const std::string retTyStr = GetValueTypeStr(fn.returnAstType);

				if (lang == ETextLang::WGSL_1_0)
				{
					fnOut << "fn " << fn.emittedName << "(";
					for (size_t i = 0; i < fn.params.size(); ++i)
					{
						if (i > 0) fnOut << ", ";
						const ParamSpec& ps = fn.params[i];
						if (ps.isSampler)
						{
							fnOut << ps.paramIdent << " : " << GetTextureParamTypeStr(ps.samplerDim)
								<< ", " << ps.smpParamIdent << " : sampler";
						}
						else
						{
							std::string pTy = GetValueTypeStr(ps.astType);
							if (ps.passByPtr)
								fnOut << ps.paramIdent << " : ptr<function, " << pTy << ">";
							else
								fnOut << ps.paramIdent << " : " << pTy;
						}
					}
					fnOut << ")";
					if (!isVoid)
						fnOut << " -> " << retTyStr;
					fnOut << "\n{\n";
				}
				else
				{
					fnOut << "static inline " << retTyStr << " " << fn.emittedName << "(thread ParaShaderContext& _ctx";
					for (size_t i = 0; i < fn.params.size(); ++i)
					{
						const ParamSpec& ps = fn.params[i];
						fnOut << ", ";
						if (ps.isSampler)
						{
							fnOut << GetTextureParamTypeStr(ps.samplerDim) << " " << ps.paramIdent
								<< ", sampler " << ps.smpParamIdent;
						}
						else
						{
							std::string pTy = GetValueTypeStr(ps.astType);
							if (ps.passByPtr)
								fnOut << "thread " << pTy << "& " << ps.paramIdent;
							else
								fnOut << pTy << " " << ps.paramIdent;
						}
					}
					fnOut << ")\n{\n";
				}

				indentLevel = 1;
				for (const ParamSpec& ps : fn.params)
				{
					if (ps.isSampler)
					{
						fnSamplerParams[ps.symId] = { ps.symId, ps.rawName, 0u, ps.samplerDim, ps.paramIdent, ps.smpParamIdent, true };
					}
					else if (ps.passByPtr)
					{
						fnLocals[ps.symId] = { ps.paramIdent, (lang == ETextLang::WGSL_1_0) };
					}
					else
					{
						std::string pTy = GetValueTypeStr(ps.astType);
						WriteIndent();
						if (lang == ETextLang::WGSL_1_0)
							fnOut << "var " << ps.localIdent << " : " << pTy << " = " << ps.paramIdent << ";\n";
						else
							fnOut << pTy << " " << ps.localIdent << " = " << ps.paramIdent << ";\n";
						fnLocals[ps.symId] = { ps.localIdent, false };
					}
				}

				for (TIntermNode* bn : bodyNodes)
				{
					if (blockTerminated)
						break;
					EmitStatement(bn);
				}

				if (!blockTerminated)
				{
					WriteIndent();
					if (isVoid)
						fnOut << "return;\n";
					else
						fnOut << "return " << GetZeroExpr(fn.returnAstType) << ";\n";
					blockTerminated = true;
				}

				indentLevel = 0;
				fnOut << "}\n\n";
				return fnOut.str();
			}

			struct TopGlobalDecl
			{
				TIntermSymbol* sym = nullptr;
				TIntermTyped* initRhs = nullptr;
				bool containsArrayInit = false;
			};

			bool BuildModule(TIntermNode* rootNode, FxCompiledStage& outStage, std::vector<FxUniformParam>& ioDiscoveredParams)
			{
				TIntermAggregate* rootAgg = rootNode ? rootNode->getAsAggregate() : nullptr;
				if (!rootAgg)
				{
					errorMsg = "AST root is not an aggregate sequence";
					return false;
				}

				// 1. Scan top-level functions & global static/const declarations
				std::unordered_map<std::string, std::vector<TIntermNode*>> funcBodies;
				std::unordered_map<int, TopGlobalDecl> topDecls;

				for (TIntermNode* top : rootAgg->getNodes())
				{
					if (!top) continue;
					if (TIntermDeclaration* decl = top->getAsDeclaration())
					{
						TIntermTyped* inner = decl->getDeclaration();
						if (!inner) continue;
						if (TIntermSymbol* sym = inner->getAsSymbolNode())
						{
							topDecls[sym->getId()] = { sym, nullptr, false };
						}
						else if (TIntermBinary* assign = inner->getAsBinaryNode())
						{
							if (TIntermSymbol* sym = assign->getLeft() ? assign->getLeft()->getAsSymbolNode() : nullptr)
								topDecls[sym->getId()] = { sym, assign->getRight(), decl->containsArrayInitialization() };
						}
						continue;
					}

					TIntermAggregate* fnAgg = top->getAsAggregate();
					if (!fnAgg || fnAgg->getOp() != EOpFunction)
						continue;

					FuncRecord rec;
					rec.astNode = fnAgg;
					rec.plainName = fnAgg->getPlainName().c_str();
					rec.mangledName = fnAgg->getName().c_str();
					rec.returnSemantic = fnAgg->getSemantic().c_str();
					rec.returnAstType = fnAgg->getType();

					std::vector<TIntermNode*>& bodies = funcBodies[rec.mangledName];
					for (TIntermNode* child : fnAgg->getNodes())
					{
						if (!child) continue;
						TIntermAggregate* cagg = child->getAsAggregate();
						if (cagg && cagg->getOp() == EOpParameters)
						{
							size_t pIdx = 0;
							for (TIntermNode* pn : cagg->getNodes())
							{
								TIntermSymbol* psym = pn ? pn->getAsSymbolNode() : nullptr;
								if (!psym) continue;
								ParamSpec ps;
								ps.symId = psym->getId();
								ps.rawName = psym->getSymbol().c_str();
								ps.paramIdent = SanitizeIdent(ps.rawName, "p_") + "_" + std::to_string(pIdx);
								ps.localIdent = SanitizeIdent(ps.rawName, "lv_") + "_" + std::to_string(pIdx);
								ps.semantic = psym->getInfo() ? psym->getInfo()->getSemantic().c_str() : "";
								ps.astType = psym->getType();
								ps.qualifier = psym->getQualifier();
								TBasicType pbt = ps.astType.getBasicType();
								if (IsSampler(pbt))
								{
									ps.isSampler = true;
									ps.passByPtr = false;
									ps.samplerDim = (pbt == EbtSamplerCube) ? ETexDim::DimCube : ((pbt == EbtSampler3D) ? ETexDim::Dim3D : ETexDim::Dim2D);
									ps.smpParamIdent = SanitizeIdent(ps.rawName, "psmp_") + "_" + std::to_string(pIdx);
								}
								else
								{
									ps.passByPtr = (ps.qualifier == EvqOut || ps.qualifier == EvqInOut);
								}
								rec.params.push_back(std::move(ps));
								++pIdx;
							}
						}
						else
						{
							bodies.push_back(child);
						}
					}

					if (rec.plainName == entryPointName)
						mainTargetMangled = rec.mangledName;
					funcsByMangled[rec.mangledName] = std::move(rec);
				}

				if (mainTargetMangled.empty())
				{
					std::string epUpper = ToUpperAscii(entryPointName);
					for (const auto& kv : funcsByMangled)
					{
						if (ToUpperAscii(kv.second.plainName) == epUpper)
						{
							mainTargetMangled = kv.first;
							break;
						}
					}
				}
				if (mainTargetMangled.empty())
				{
					errorMsg = "Entry point function not found in HLSL AST: " + entryPointName;
					return false;
				}

				// 2. Reachability DFS from entry point
				std::unordered_set<std::string> visitedFuncs;
				std::unordered_set<int> localSymIds;
				std::vector<TIntermSymbol*> foundUniformSyms;
				std::unordered_map<int, bool> uniformSymMutable;
				std::vector<TIntermSymbol*> foundSamplerSyms;
				std::unordered_set<int> seenSamplerIds;
				std::vector<TIntermSymbol*> foundPrivateSyms;
				std::unordered_set<int> seenPrivateIds;

				struct ReachScanner : public TIntermTraverser
				{
					std::unordered_set<int>& locals;
					std::vector<std::string> callees;
					std::vector<TIntermSymbol*> extSyms;

					explicit ReachScanner(std::unordered_set<int>& l) : locals(l)
					{
						preVisit = true;
						postVisit = false;
						visitDeclaration = [](bool, TIntermDeclaration* d, TIntermTraverser* it) -> bool {
							auto* self = static_cast<ReachScanner*>(it);
							TIntermTyped* in = d->getDeclaration();
							if (!in) return true;
							if (TIntermSymbol* s = in->getAsSymbolNode())
								self->locals.insert(s->getId());
							else if (TIntermBinary* b = in->getAsBinaryNode())
							{
								if (b->getLeft() && b->getLeft()->getAsSymbolNode())
									self->locals.insert(b->getLeft()->getAsSymbolNode()->getId());
							}
							return true;
						};
						visitAggregate = [](bool, TIntermAggregate* a, TIntermTraverser* it) -> bool {
							if (a->getOp() == EOpFunctionCall)
								static_cast<ReachScanner*>(it)->callees.push_back(a->getName().c_str());
							return true;
						};
						visitSymbol = [](TIntermSymbol* s, TIntermTraverser* it) {
							auto* self = static_cast<ReachScanner*>(it);
							if (self->locals.find(s->getId()) == self->locals.end())
								self->extSyms.push_back(s);
						};
					}
				};

				std::function<void(TIntermSymbol*)> registerExtSym;
				registerExtSym = [&](TIntermSymbol* sym) {
					int sid = sym->getId();
					TBasicType bt = sym->getBasicType();
					TQualifier q = sym->getQualifier();
					if (bt == EbtTexture)
						return;
					if (IsSampler(bt))
					{
						if (seenSamplerIds.insert(sid).second)
							foundSamplerSyms.push_back(sym);
						return;
					}
					if (q == EvqUniform || q == EvqMutableUniform)
					{
						auto it = uniformSymMutable.find(sid);
						if (it == uniformSymMutable.end())
						{
							uniformSymMutable[sid] = (q == EvqMutableUniform);
							foundUniformSyms.push_back(sym);
						}
						else if (q == EvqMutableUniform)
						{
							it->second = true;
						}
						return;
					}
					if (seenPrivateIds.insert(sid).second)
					{
						foundPrivateSyms.push_back(sym);
						auto tdit = topDecls.find(sid);
						if (tdit != topDecls.end() && tdit->second.initRhs)
						{
							ReachScanner subScan(localSymIds);
							tdit->second.initRhs->traverse(&subScan);
							for (TIntermSymbol* subS : subScan.extSyms)
								registerExtSym(subS);
						}
					}
				};

				std::function<void(const std::string&)> visitFuncDfs = [&](const std::string& mangled) {
					if (!visitedFuncs.insert(mangled).second)
						return;
					auto fit = funcsByMangled.find(mangled);
					if (fit == funcsByMangled.end())
						return;
					for (const ParamSpec& ps : fit->second.params)
						localSymIds.insert(ps.symId);

					ReachScanner scanner(localSymIds);
					for (TIntermNode* bn : funcBodies[mangled])
					{
						if (bn) bn->traverse(&scanner);
					}
					for (TIntermSymbol* es : scanner.extSyms)
						registerExtSym(es);
					for (const std::string& cal : scanner.callees)
						visitFuncDfs(cal);

					orderedCalledFuncs.push_back(mangled); // post-order: callees before callers
				};

				visitFuncDfs(mainTargetMangled);

				// 3. Compute $Globals Uniform Buffer Layout (16-byte vec4 array compatible with Metal & WebGPU)
				if (!foundUniformSyms.empty())
				{
					std::sort(foundUniformSyms.begin(), foundUniformSyms.end(), [&](TIntermSymbol* a, TIntermSymbol* b) {
						auto findOrd = [&](const char* nm) -> size_t {
							for (size_t i = 0; i < declaredParams.size(); ++i)
								if (declaredParams[i].name == nm) return i;
							return declaredParams.size() + 1000u;
						};
						size_t oa = findOrd(a->getSymbol().c_str());
						size_t ob = findOrd(b->getSymbol().c_str());
						return (oa != ob) ? (oa < ob) : (a->getId() < b->getId());
					});

					uint32_t curOffset = 0;
					for (size_t i = 0; i < foundUniformSyms.size(); ++i)
					{
						TIntermSymbol* usym = foundUniformSyms[i];
						uint32_t mAlign = 4, mSize = 4;
						ComputeUniformLayout(usym->getType(), mAlign, mSize);
						curOffset = AlignUpU32(curOffset, mAlign);

						UniformFieldInfo ufi;
						ufi.symId = usym->getId();
						ufi.rawName = usym->getSymbol().c_str();
						ufi.varName = SanitizeIdent(ufi.rawName, "mu_") + "_" + std::to_string(i);
						ufi.semantic = usym->getInfo() ? usym->getInfo()->getSemantic().c_str() : "";
						ufi.astType = usym->getType();
						ufi.byteOffset = curOffset;
						ufi.byteSize = mSize;
						ufi.isMutableCopy = uniformSymMutable[ufi.symId];
						if (ufi.astType.getBasicType() == EbtStruct)
							RegisterStruct(ufi.astType);

						symIdToUniformIdx[ufi.symId] = uniformFields.size();
						uniformFields.push_back(ufi);
						curOffset += mSize;

						auto existing = std::find_if(ioDiscoveredParams.begin(), ioDiscoveredParams.end(),
							[&](const FxUniformParam& p) { return p.name == ufi.rawName; });
						if (existing == ioDiscoveredParams.end())
						{
							FxUniformParam p;
							p.name = ufi.rawName;
							p.semantic = ufi.semantic;
							p.elementCount = ufi.astType.isArray() ? (std::max)(1, ufi.astType.getArraySize()) : 1;
							p.isBool = (ufi.astType.getBasicType() == EbtBool);
							p.isInt = (ufi.astType.getBasicType() == EbtInt);
							ioDiscoveredParams.push_back(std::move(p));
						}
						else if (existing->semantic.empty() && !ufi.semantic.empty())
						{
							existing->semantic = ufi.semantic;
						}

						FxConstantMember cm;
						cm.name = ufi.rawName;
						cm.byteOffset = ufi.byteOffset;
						cm.byteSize = ufi.byteSize;
						cm.rows = static_cast<uint16_t>((std::max)(1, ufi.astType.getRowsCount()));
						cm.cols = static_cast<uint16_t>((std::max)(1, ufi.astType.getColsCount()));
						cm.elements = static_cast<uint16_t>(ufi.astType.isArray() ? (std::max)(1, ufi.astType.getArraySize()) : 1);
						cm.baseType = (ufi.astType.getBasicType() == EbtInt) ? 1 : (ufi.astType.getBasicType() == EbtBool ? 2 : 0);
						outStage.cbufferMembers.push_back(std::move(cm));
					}

					globalsByteSize = AlignUpU32((std::max)(curOffset, 16u), 16u);
					globalsVec4Count = (std::max)(1u, globalsByteSize / 16u);
					outStage.cbufferSize = globalsByteSize;
				}

				// 4. Assign Texture & Sampler slots (0..15)
				std::unordered_set<uint32_t> usedSamplerSlots;
				for (TIntermSymbol* ssym : foundSamplerSyms)
				{
					SamplerResourceInfo sri;
					sri.symId = ssym->getId();
					sri.rawName = ssym->getSymbol().c_str();
					TBasicType sbt = ssym->getBasicType();
					if (sbt == EbtSamplerCube) sri.dim = ETexDim::DimCube;
					else if (sbt == EbtSampler3D) sri.dim = ETexDim::Dim3D;
					else sri.dim = ETexDim::Dim2D;

					int assignedSlot = -1;
					for (size_t i = 0; i < declaredSamplers.size(); ++i)
					{
						if (declaredSamplers[i].samplerName == sri.rawName)
						{
							assignedSlot = (declaredSamplers[i].registerIndex >= 0) ? declaredSamplers[i].registerIndex : static_cast<int>(i);
							std::string stUpper = ToUpperAscii(declaredSamplers[i].samplerType);
							if (stUpper.find("CUBE") != std::string::npos) sri.dim = ETexDim::DimCube;
							else if (stUpper.find("3D") != std::string::npos) sri.dim = ETexDim::Dim3D;
							break;
						}
					}
					if (assignedSlot < 0 && ssym->getInfo() && !ssym->getInfo()->getRegister().empty())
					{
						const char* regStr = ssym->getInfo()->getRegister().c_str();
						while (*regStr && !std::isdigit(static_cast<unsigned char>(*regStr))) ++regStr;
						if (*regStr) assignedSlot = std::atoi(regStr);
					}
					if (assignedSlot < 0)
					{
						while (usedSamplerSlots.count(nextAutoSamplerSlot))
							++nextAutoSamplerSlot;
						assignedSlot = static_cast<int>(nextAutoSamplerSlot++);
					}
					sri.slot = static_cast<uint32_t>((std::max)(0, (std::min)(assignedSlot, 15)));
					usedSamplerSlots.insert(sri.slot);

					sri.texIdent = "para_tex_" + std::to_string(sri.slot);
					sri.smpIdent = "para_smp_" + std::to_string(sri.slot);

					samplerBySymId[sri.symId] = sri;
					if (samplerBySlot.find(sri.slot) == samplerBySlot.end())
						samplerBySlot[sri.slot] = sri;

					outStage.textures.push_back({ sri.rawName, sri.slot, 1u });
					outStage.samplers.push_back({ sri.rawName, sri.slot, 1u });
				}

				// 5. Register Private global variables
				std::sort(foundPrivateSyms.begin(), foundPrivateSyms.end(), [](TIntermSymbol* a, TIntermSymbol* b) {
					return a->getId() < b->getId();
				});
				for (size_t i = 0; i < foundPrivateSyms.size(); ++i)
				{
					TIntermSymbol* psym = foundPrivateSyms[i];
					GlobalVarInfo gvi;
					gvi.symId = psym->getId();
					gvi.varName = SanitizeIdent(psym->getSymbol().c_str(), "pg_") + "_" + std::to_string(i);
					gvi.astType = psym->getType();
					if (gvi.astType.getBasicType() == EbtStruct)
						RegisterStruct(gvi.astType);
					auto tdit = topDecls.find(gvi.symId);
					if (tdit != topDecls.end())
					{
						gvi.initExpr = tdit->second.initRhs;
						gvi.containsArrayInit = tdit->second.containsArrayInit;
					}
					privateGlobals[gvi.symId] = gvi;
					privateGlobalInitOrder.push_back(gvi.symId);
				}

				// 6. Emit all reachable HLSL functions in callee-first order
				for (size_t i = 0; i < orderedCalledFuncs.size(); ++i)
				{
					FuncRecord& fn = funcsByMangled[orderedCalledFuncs[i]];
					fn.emittedName = SanitizeIdent(fn.plainName, "fn_") + "_" + std::to_string(i);
				}

				std::ostringstream funcsStream;
				for (const std::string& mname : orderedCalledFuncs)
				{
					const FuncRecord& fn = funcsByMangled[mname];
					funcsStream << EmitUserFunction(fn, funcBodies[mname]);
				}
				std::string allUserFuncsCode = funcsStream.str();

				// 7. Emit Entry Point Body (`vs_main` / `fs_main`)
				const FuncRecord& entryTarget = funcsByMangled[mainTargetMangled];
				fnOut.str("");
				fnOut.clear();
				fnLocals.clear();
				fnSamplerParams.clear();
				indentLevel = 1;
				blockTerminated = false;
				currentFnRetAstType = TType(EbtVoid, EbpUndefined);

				// Initialize mutable uniform copies from `_para_ub`
				for (const UniformFieldInfo& ufi : uniformFields)
				{
					if (!ufi.isMutableCopy) continue;
					std::string unpacked = UnpackUniformValue("", ufi.byteOffset, ufi.astType, false);
					WriteIndent();
					if (lang == ETextLang::WGSL_1_0)
						fnOut << ufi.varName << " = " << unpacked << ";\n";
					else
						fnOut << "_ctx." << ufi.varName << " = " << unpacked << ";\n";
				}

				// Initialize private static/const globals
				for (int sid : privateGlobalInitOrder)
				{
					const GlobalVarInfo& gvi = privateGlobals[sid];
					if (!gvi.initExpr) continue;
					std::string lhsRef = (lang == ETextLang::WGSL_1_0) ? gvi.varName : ("_ctx." + gvi.varName);
					const TType& symType = gvi.astType;
					TIntermTyped* rhs = gvi.initExpr;
					if (gvi.containsArrayInit && rhs->getAsAggregate() && rhs->getAsAggregate()->getOp() != EOpConstructArray)
					{
						TType elemT = symType;
						elemT.clearArrayness();
						int arrLen = (std::max)(1, symType.getArraySize());
						TNodeArray& initNodes = rhs->getAsAggregate()->getNodes();
						std::vector<std::string> elems;
						for (int i = 0; i < arrLen; ++i)
						{
							if (static_cast<size_t>(i) < initNodes.size() && initNodes[static_cast<size_t>(i)]->getAsTyped())
							{
								TIntermTyped* inNode = initNodes[static_cast<size_t>(i)]->getAsTyped();
								elems.push_back(CoerceToType(EmitRValue(inNode), inNode->getType(), elemT));
							}
							else
							{
								elems.push_back(GetZeroExpr(elemT));
							}
						}
						std::string arrVal = ConstructComposite(symType, elems, false);
						WriteIndent();
						fnOut << lhsRef << " = " << arrVal << ";\n";
					}
					else
					{
						std::string initVal = CoerceToType(EmitRValue(rhs), rhs->getType(), symType);
						WriteIndent();
						fnOut << lhsRef << " = " << initVal << ";\n";
					}
				}

				uint32_t autoFallbackSemIdx = 0;
				TType v4FloatAst(EbtFloat, EbpUndefined, EvqTemporary, 1, 4);
				TType scIntAst(EbtInt, EbpUndefined, EvqTemporary, 1, 1);
				TType scFloatAst(EbtFloat, EbpUndefined, EvqTemporary, 1, 1);

				std::function<std::string(const TType&, const std::string&)> readStageInputValue =
					[&](const TType& t, const std::string& rawSem) -> std::string {
					if (t.getBasicType() == EbtStruct)
					{
						const StructInfo& si = RegisterStruct(t);
						const TTypeList* sl = t.getStruct();
						std::vector<std::string> fvals;
						if (sl)
						{
							for (size_t i = 0; i < sl->size(); ++i)
							{
								const TType& ft = *(*sl)[i].type;
								std::string fsem = ft.hasSemantic() ? ft.getSemantic().c_str() : "";
								fvals.push_back(readStageInputValue(ft, fsem));
							}
						}
						(void)si;
						return ConstructComposite(t, fvals, true);
					}

					std::string baseUpper;
					uint32_t semIdx = 0;
					ParseSemanticAndIdx(rawSem, baseUpper, semIdx);
					if (baseUpper.empty())
					{
						baseUpper = "TEXCOORD";
						semIdx = autoFallbackSemIdx++;
					}

					uint32_t dstRows = static_cast<uint32_t>((std::max)(1, t.getRowsCount()));
					TBasicType dstBt = t.getBasicType();

					if (isVertex)
					{
						if (baseUpper == "SV_VERTEXID" || baseUpper == "VERTEXID")
						{
							usesVertexId = true;
							std::string iv = (lang == ETextLang::WGSL_1_0)
								? AllocTmp("i32", "i32(_in.para_vid)")
								: AllocTmp("int", "int(_para_vid)");
							return CoerceVectorOrScalar(iv, scIntAst, dstBt, dstRows);
						}
						if (baseUpper == "SV_INSTANCEID" || baseUpper == "INSTANCEID")
						{
							usesInstanceId = true;
							std::string iv = (lang == ETextLang::WGSL_1_0)
								? AllocTmp("i32", "i32(_in.para_iid)")
								: AllocTmp("int", "int(_para_iid)");
							return CoerceVectorOrScalar(iv, scIntAst, dstBt, dstRows);
						}

						int32_t loc = MapSemanticToVSLocation(baseUpper.c_str(), semIdx);
						uint32_t uloc = (loc >= 0) ? static_cast<uint32_t>(loc) : 0u;
						stageInputLocs.insert(uloc);
						std::string attrExpr = "_in.attr_" + std::to_string(uloc);
						return CoerceVectorOrScalar(attrExpr, v4FloatAst, dstBt, dstRows);
					}
					else
					{
						if (baseUpper == "VPOS" || baseUpper == "SV_POSITION" || baseUpper == "POSITION")
						{
							usesFragCoord = true;
							return CoerceVectorOrScalar("_in.frag_pos", v4FloatAst, dstBt, dstRows);
						}
						if (baseUpper == "VFACE" || baseUpper == "SV_ISFRONTFACE")
						{
							usesFrontFace = true;
							std::string signedFace = (lang == ETextLang::WGSL_1_0)
								? AllocTmp("f32", "select(-1.0, 1.0, _in.front_face)")
								: AllocTmp("float", "(_para_front_face ? 1.0f : -1.0f)");
							return CoerceVectorOrScalar(signedFace, scFloatAst, dstBt, dstRows);
						}

						uint32_t loc = static_cast<uint32_t>(MapVaryingLoc(baseUpper, semIdx));
						stageInputLocs.insert(loc);
						std::string varyExpr = "_in.vary_" + std::to_string(loc);
						return CoerceVectorOrScalar(varyExpr, v4FloatAst, dstBt, dstRows);
					}
				};

				std::function<void(const std::string&, const TType&, const std::string&)> writeStageOutputValue =
					[&](const std::string& valExpr, const TType& t, const std::string& rawSem) {
					if (t.getBasicType() == EbtVoid)
						return;
					if (t.getBasicType() == EbtStruct)
					{
						const StructInfo& si = RegisterStruct(t);
						const TTypeList* sl = t.getStruct();
						if (sl)
						{
							for (size_t i = 0; i < sl->size() && i < si.fieldNames.size(); ++i)
							{
								const TType& ft = *(*sl)[i].type;
								std::string fExpr = AllocTmp(GetValueTypeStr(ft), valExpr + "." + si.fieldNames[i]);
								std::string fsem = ft.hasSemantic() ? ft.getSemantic().c_str() : "";
								writeStageOutputValue(fExpr, ft, fsem);
							}
						}
						return;
					}

					std::string baseUpper;
					uint32_t semIdx = 0;
					ParseSemanticAndIdx(rawSem, baseUpper, semIdx);

					if (isVertex)
					{
						if (baseUpper == "POSITION" || baseUpper == "SV_POSITION" || baseUpper.empty())
						{
							usesPosOut = true;
							std::string p4 = CoerceVectorOrScalar(valExpr, t, EbtFloat, 4);
							WriteIndent();
							fnOut << "_out.pos = " << p4 << ";\n";
							return;
						}
						if (baseUpper == "PSIZE")
						{
							std::string ps = CoerceVectorOrScalar(valExpr, t, EbtFloat, 1);
							if (lang == ETextLang::MSL_2_0)
							{
								usesPointSizeOut = true;
								WriteIndent();
								fnOut << "_out.psize = " << ps << ";\n";
							}
							else
							{
								(void)ps; // WebGPU 1.0 point-list primitives are fixed 1px (no point_size builtin)
							}
							return;
						}

						uint32_t loc = static_cast<uint32_t>(MapVaryingLoc(baseUpper, semIdx));
						stageOutputLocs.insert(loc);
						std::string v4 = CoerceVectorOrScalar(valExpr, t, EbtFloat, 4);
						WriteIndent();
						fnOut << "_out.vary_" << loc << " = " << v4 << ";\n";
					}
					else
					{
						if (baseUpper == "DEPTH" || baseUpper == "SV_DEPTH")
						{
							usesFragDepthOut = true;
							std::string d = CoerceVectorOrScalar(valExpr, t, EbtFloat, 1);
							WriteIndent();
							fnOut << "_out.frag_depth = " << d << ";\n";
							return;
						}

						uint32_t loc = (std::min)(semIdx, 3u);
						stageOutputLocs.insert(loc);
						std::string v4 = CoerceVectorOrScalar(valExpr, t, EbtFloat, 4);
						WriteIndent();
						fnOut << "_out.color_" << loc << " = " << v4 << ";\n";
					}
				};

				struct EntryOutParam
				{
					std::string tmpVar;
					TType astType;
					std::string semantic;
				};
				std::vector<EntryOutParam> entryOutParams;
				std::vector<std::string> entryCallArgs;
				if (lang == ETextLang::MSL_2_0)
					entryCallArgs.push_back("_ctx");

				for (const ParamSpec& ps : entryTarget.params)
				{
					if (ps.passByPtr)
					{
						std::string initVal = (ps.qualifier == EvqInOut)
							? readStageInputValue(ps.astType, ps.semantic)
							: GetZeroExpr(ps.astType);
						std::string tmpVar = AllocMutableLocal(GetValueTypeStr(ps.astType), initVal);
						entryCallArgs.push_back(lang == ETextLang::WGSL_1_0 ? ("&" + tmpVar) : tmpVar);
						entryOutParams.push_back({ tmpVar, ps.astType, ps.semantic });
					}
					else
					{
						entryCallArgs.push_back(readStageInputValue(ps.astType, ps.semantic));
					}
				}

				std::ostringstream entryCallOss;
				entryCallOss << entryTarget.emittedName << "(";
				for (size_t i = 0; i < entryCallArgs.size(); ++i)
				{
					if (i > 0) entryCallOss << ", ";
					entryCallOss << entryCallArgs[i];
				}
				entryCallOss << ")";

				if (entryTarget.returnAstType.getBasicType() != EbtVoid)
				{
					std::string retVal = AllocTmp(GetValueTypeStr(entryTarget.returnAstType), entryCallOss.str());
					writeStageOutputValue(retVal, entryTarget.returnAstType, entryTarget.returnSemantic);
				}
				else
				{
					WriteIndent();
					fnOut << entryCallOss.str() << ";\n";
				}

				for (const EntryOutParam& eop : entryOutParams)
					writeStageOutputValue(eop.tmpVar, eop.astType, eop.semantic);

				std::string entryBodyCode = fnOut.str();

				// 8. Assemble Final Module Text (`MSL_2_0` or `WGSL_1_0`)
				std::ostringstream mod;
				const bool hasUb = (globalsByteSize > 0);
				const bool hasStageOut = isVertex || !stageOutputLocs.empty() || usesFragDepthOut;

				if (lang == ETextLang::MSL_2_0)
				{
					mod << "#include <metal_stdlib>\n";
					mod << "#include <simd/simd.h>\n";
					mod << "using namespace metal;\n\n";

					for (const TTypeList* sl : structOrder)
					{
						const StructInfo& si = structMap[sl];
						mod << "struct " << si.typeName << "\n{\n";
						if (si.fieldTypes.empty())
						{
							mod << "    float _pad;\n";
						}
						else
						{
							for (size_t i = 0; i < si.fieldTypes.size(); ++i)
								mod << "    " << GetValueTypeStr(si.fieldTypes[i]) << " " << si.fieldNames[i] << ";\n";
						}
						mod << "};\n\n";
					}

					if (hasUb)
					{
						mod << "struct ParaGlobalsBuffer\n{\n";
						mod << "    float4 data[" << globalsVec4Count << "];\n";
						mod << "};\n\n";
					}

					mod << "struct ParaShaderContext\n{\n";
					mod << "    constant float4* _ub;\n";
					for (const UniformFieldInfo& ufi : uniformFields)
					{
						if (ufi.isMutableCopy)
							mod << "    " << GetValueTypeStr(ufi.astType) << " " << ufi.varName << ";\n";
					}
					for (int sid : privateGlobalInitOrder)
					{
						const GlobalVarInfo& gvi = privateGlobals[sid];
						mod << "    " << GetValueTypeStr(gvi.astType) << " " << gvi.varName << ";\n";
					}
					for (const auto& kv : samplerBySlot)
					{
						const SamplerResourceInfo& sri = kv.second;
						const char* texTy = (sri.dim == ETexDim::DimCube) ? "texturecube<float>" : (sri.dim == ETexDim::Dim3D ? "texture3d<float>" : "texture2d<float>");
						mod << "    " << texTy << " " << sri.texIdent << ";\n";
						mod << "    sampler " << sri.smpIdent << ";\n";
					}
					mod << "};\n\n";

					const bool hasStageIn = isVertex ? (!stageInputLocs.empty()) : (!stageInputLocs.empty() || usesFragCoord);
					if (hasStageIn)
					{
						mod << "struct ParaStageIn\n{\n";
						if (isVertex)
						{
							for (uint32_t loc : stageInputLocs)
								mod << "    float4 attr_" << loc << " [[attribute(" << loc << ")]];\n";
						}
						else
						{
							if (usesFragCoord)
								mod << "    float4 frag_pos [[position]];\n";
							for (uint32_t loc : stageInputLocs)
								mod << "    float4 vary_" << loc << " [[user(loc_" << loc << ")]];\n";
						}
						mod << "};\n\n";
					}

					if (hasStageOut)
					{
						mod << "struct ParaStageOut\n{\n";
						if (isVertex)
						{
							mod << "    float4 pos [[position]];\n";
							if (usesPointSizeOut)
								mod << "    float psize [[point_size]];\n";
							for (uint32_t loc : stageOutputLocs)
								mod << "    float4 vary_" << loc << " [[user(loc_" << loc << ")]];\n";
						}
						else
						{
							for (uint32_t loc : stageOutputLocs)
								mod << "    float4 color_" << loc << " [[color(" << loc << ")]];\n";
							if (usesFragDepthOut)
								mod << "    float frag_depth [[depth(any)]];\n";
						}
						mod << "};\n\n";
					}

					mod << allUserFuncsCode;

					mod << (isVertex ? "vertex " : "fragment ")
						<< (hasStageOut ? "ParaStageOut " : "void ")
						<< (isVertex ? "vs_main(" : "fs_main(");

					bool firstArg = true;
					auto comma = [&]() { if (!firstArg) mod << ", "; firstArg = false; };
					if (hasStageIn)
					{
						comma();
						mod << "ParaStageIn _in [[stage_in]]";
					}
					if (isVertex && usesVertexId)
					{
						comma();
						mod << "uint _para_vid [[vertex_id]]";
					}
					if (isVertex && usesInstanceId)
					{
						comma();
						mod << "uint _para_iid [[instance_id]]";
					}
					if (!isVertex && usesFrontFace)
					{
						comma();
						mod << "bool _para_front_face [[front_facing]]";
					}
					if (hasUb)
					{
						comma();
						mod << "constant ParaGlobalsBuffer& _para_ub [[buffer(" << (isVertex ? 16 : 17) << ")]]";
					}
					for (const auto& kv : samplerBySlot)
					{
						const SamplerResourceInfo& sri = kv.second;
						const char* texTy = (sri.dim == ETexDim::DimCube) ? "texturecube<float>" : (sri.dim == ETexDim::Dim3D ? "texture3d<float>" : "texture2d<float>");
						comma();
						mod << texTy << " " << sri.texIdent << " [[texture(" << sri.slot << ")]]";
						comma();
						mod << "sampler " << sri.smpIdent << " [[sampler(" << sri.slot << ")]]";
					}
					mod << ")\n{\n";
					mod << "    ParaShaderContext _ctx;\n";
					mod << "    _ctx._ub = " << (hasUb ? "_para_ub.data" : "nullptr") << ";\n";
					for (const auto& kv : samplerBySlot)
					{
						const SamplerResourceInfo& sri = kv.second;
						mod << "    _ctx." << sri.texIdent << " = " << sri.texIdent << ";\n";
						mod << "    _ctx." << sri.smpIdent << " = " << sri.smpIdent << ";\n";
					}
					if (hasStageOut)
					{
						mod << "    ParaStageOut _out = {};\n";
						if (isVertex)
							mod << "    _out.pos = float4(0.0f, 0.0f, 0.0f, 1.0f);\n";
					}
					mod << entryBodyCode;
					if (hasStageOut)
						mod << "    return _out;\n";
					mod << "}\n";

					outStage.target = EShaderTarget::MSL_2_0;
					outStage.profile = isVertex ? "msl_2_0_vs" : "msl_2_0_ps";
				}
				else // ETextLang::WGSL_1_0
				{
					if (!isVertex)
					{
						mod << "diagnostic(off, derivative_uniformity);\n\n";
					}

					for (const TTypeList* sl : structOrder)
					{
						const StructInfo& si = structMap[sl];
						mod << "struct " << si.typeName << "\n{\n";
						if (si.fieldTypes.empty())
						{
							mod << "    pad_0 : f32,\n";
						}
						else
						{
							for (size_t i = 0; i < si.fieldTypes.size(); ++i)
								mod << "    " << si.fieldNames[i] << " : " << GetValueTypeStr(si.fieldTypes[i]) << ",\n";
						}
						mod << "};\n\n";
					}

					if (hasUb)
					{
						mod << "struct ParaGlobalsBuffer\n{\n";
						mod << "    data : array<vec4<f32>, " << globalsVec4Count << ">,\n";
						mod << "};\n";
						mod << "@group(0) @binding(" << (isVertex ? 0 : 1) << ") var<uniform> _para_ub : ParaGlobalsBuffer;\n\n";
					}

					for (const auto& kv : samplerBySlot)
					{
						const SamplerResourceInfo& sri = kv.second;
						const char* texTy = (sri.dim == ETexDim::DimCube) ? "texture_cube<f32>" : (sri.dim == ETexDim::Dim3D ? "texture_3d<f32>" : "texture_2d<f32>");
						mod << "@group(0) @binding(" << (16u + sri.slot) << ") var " << sri.texIdent << " : " << texTy << ";\n";
						mod << "@group(0) @binding(" << (32u + sri.slot) << ") var " << sri.smpIdent << " : sampler;\n";
					}
					if (!samplerBySlot.empty())
						mod << "\n";

					for (const UniformFieldInfo& ufi : uniformFields)
					{
						if (ufi.isMutableCopy)
							mod << "var<private> " << ufi.varName << " : " << GetValueTypeStr(ufi.astType) << ";\n";
					}
					for (int sid : privateGlobalInitOrder)
					{
						const GlobalVarInfo& gvi = privateGlobals[sid];
						mod << "var<private> " << gvi.varName << " : " << GetValueTypeStr(gvi.astType) << ";\n";
					}

					const bool hasStageIn = isVertex
						? (!stageInputLocs.empty() || usesVertexId || usesInstanceId)
						: (!stageInputLocs.empty() || usesFragCoord || usesFrontFace);
					if (hasStageIn)
					{
						mod << "\nstruct ParaStageIn\n{\n";
						if (isVertex)
						{
							if (usesVertexId)
								mod << "    @builtin(vertex_index) para_vid : u32,\n";
							if (usesInstanceId)
								mod << "    @builtin(instance_index) para_iid : u32,\n";
							for (uint32_t loc : stageInputLocs)
								mod << "    @location(" << loc << ") attr_" << loc << " : vec4<f32>,\n";
						}
						else
						{
							if (usesFragCoord)
								mod << "    @builtin(position) frag_pos : vec4<f32>,\n";
							if (usesFrontFace)
								mod << "    @builtin(front_facing) front_face : bool,\n";
							for (uint32_t loc : stageInputLocs)
								mod << "    @location(" << loc << ") vary_" << loc << " : vec4<f32>,\n";
						}
						mod << "};\n";
					}

					if (hasStageOut)
					{
						mod << "\nstruct ParaStageOut\n{\n";
						if (isVertex)
						{
							mod << "    @builtin(position) pos : vec4<f32>,\n";
							for (uint32_t loc : stageOutputLocs)
								mod << "    @location(" << loc << ") vary_" << loc << " : vec4<f32>,\n";
						}
						else
						{
							for (uint32_t loc : stageOutputLocs)
								mod << "    @location(" << loc << ") color_" << loc << " : vec4<f32>,\n";
							if (usesFragDepthOut)
								mod << "    @builtin(frag_depth) frag_depth : f32,\n";
						}
						mod << "};\n\n";
					}
					else
					{
						mod << "\n";
					}

					mod << allUserFuncsCode;

					mod << (isVertex ? "@vertex\nfn vs_main(" : "@fragment\nfn fs_main(");
					if (hasStageIn)
						mod << "_in : ParaStageIn";
					mod << ")";
					if (hasStageOut)
						mod << " -> ParaStageOut";
					mod << "\n{\n";
					if (hasStageOut)
					{
						mod << "    var _out : ParaStageOut;\n";
						if (isVertex)
							mod << "    _out.pos = vec4<f32>(0.0, 0.0, 0.0, 1.0);\n";
					}
					mod << entryBodyCode;
					if (hasStageOut)
						mod << "    return _out;\n";
					mod << "}\n";

					outStage.target = EShaderTarget::WGSL_1_0;
					outStage.profile = isVertex ? "wgsl_1_0_vs" : "wgsl_1_0_ps";
				}

				std::string finalSrc = mod.str();
				outStage.valid = true;
				outStage.entryPoint = isVertex ? "vs_main" : "fs_main";
				outStage.bytecode.assign(finalSrc.begin(), finalSrc.end());
				return true;
			}
		};

		struct TextAstVisitorPayload
		{
			StructuredTextModuleBuilder* builder = nullptr;
			FxCompiledStage* outStage = nullptr;
			std::vector<FxUniformParam>* ioParams = nullptr;
		};

		inline bool C_DECL TextAstVisitorCallback(TIntermNode* rootNode, EShLanguage /*language*/, void* userData)
		{
			auto* payload = static_cast<TextAstVisitorPayload*>(userData);
			return payload->builder->BuildModule(rootNode, *payload->outStage, *payload->ioParams);
		}

		// Shared structural validator for generated text shaders (balanced delimiters, no embedded NULs, etc.)
		inline bool ValidateBalancedDelimiters(const std::string& src, std::string& outError)
		{
			int braces = 0, parens = 0, brackets = 0;
			bool inLineComment = false;
			bool inBlockComment = false;
			for (size_t i = 0; i < src.size(); ++i)
			{
				char c = src[i];
				char next = (i + 1 < src.size()) ? src[i + 1] : '\0';
				if (inLineComment)
				{
					if (c == '\n') inLineComment = false;
					continue;
				}
				if (inBlockComment)
				{
					if (c == '*' && next == '/') { inBlockComment = false; ++i; }
					continue;
				}
				if (c == '/' && next == '/') { inLineComment = true; ++i; continue; }
				if (c == '/' && next == '*') { inBlockComment = true; ++i; continue; }

				if (c == '{') ++braces;
				else if (c == '}') { if (--braces < 0) { outError = "Unmatched '}' in generated shader"; return false; } }
				else if (c == '(') ++parens;
				else if (c == ')') { if (--parens < 0) { outError = "Unmatched ')' in generated shader"; return false; } }
				else if (c == '[') ++brackets;
				else if (c == ']') { if (--brackets < 0) { outError = "Unmatched ']' in generated shader"; return false; } }
			}
			if (braces != 0 || parens != 0 || brackets != 0)
			{
				outError = "Unbalanced delimiters in generated shader";
				return false;
			}
			return true;
		}
	} // namespace AstTextGenDetail
} // namespace ParaEngine
