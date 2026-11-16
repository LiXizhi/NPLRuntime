#include "SpirvCodeGen.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <functional>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include "hlsl2glsl.h"
#include "intermediate.h"

namespace ParaEngine
{
	namespace
	{
		std::mutex g_spirvAstMutex;

		// =====================================================================
		// SPIR-V 1.0 Core Opcodes & Enumerants
		// =====================================================================
		constexpr uint32_t SPV_MAGIC = 0x07230203u;
		constexpr uint32_t SPV_VERSION_1_0 = 0x00010000u;

		enum SpvOp : uint16_t
		{
			OpNop = 0,
			OpName = 5,
			OpMemberName = 6,
			OpExtInstImport = 11,
			OpExtInst = 12,
			OpMemoryModel = 14,
			OpEntryPoint = 15,
			OpExecutionMode = 16,
			OpCapability = 17,
			OpTypeVoid = 19,
			OpTypeBool = 20,
			OpTypeInt = 21,
			OpTypeFloat = 22,
			OpTypeVector = 23,
			OpTypeMatrix = 24,
			OpTypeImage = 25,
			OpTypeSampler = 26,
			OpTypeSampledImage = 27,
			OpTypeArray = 28,
			OpTypeStruct = 30,
			OpTypePointer = 32,
			OpTypeFunction = 33,
			OpConstantTrue = 41,
			OpConstantFalse = 42,
			OpConstant = 43,
			OpConstantComposite = 44,
			OpConstantNull = 46,
			OpFunction = 54,
			OpFunctionParameter = 55,
			OpFunctionEnd = 56,
			OpFunctionCall = 57,
			OpVariable = 59,
			OpLoad = 61,
			OpStore = 62,
			OpAccessChain = 65,
			OpDecorate = 71,
			OpMemberDecorate = 72,
			OpVectorShuffle = 79,
			OpCompositeConstruct = 80,
			OpCompositeExtract = 81,
			OpCompositeInsert = 82,
			OpTranspose = 84,
			OpSampledImage = 86,
			OpImageSampleImplicitLod = 87,
			OpImageSampleExplicitLod = 88,
			OpImageSampleProjImplicitLod = 91,
			OpImageSampleProjExplicitLod = 92,
			OpConvertFToS = 110,
			OpConvertSToF = 111,
			OpSNegate = 126,
			OpFNegate = 127,
			OpIAdd = 128,
			OpFAdd = 129,
			OpISub = 130,
			OpFSub = 131,
			OpIMul = 132,
			OpFMul = 133,
			OpSDiv = 135,
			OpFDiv = 136,
			OpSMod = 139,
			OpFRem = 140,
			OpFMod = 141,
			OpVectorTimesScalar = 142,
			OpMatrixTimesScalar = 143,
			OpVectorTimesMatrix = 144,
			OpMatrixTimesVector = 145,
			OpMatrixTimesMatrix = 146,
			OpDot = 148,
			OpAny = 154,
			OpAll = 155,
			OpLogicalEqual = 164,
			OpLogicalNotEqual = 165,
			OpLogicalOr = 166,
			OpLogicalAnd = 167,
			OpLogicalNot = 168,
			OpSelect = 169,
			OpIEqual = 170,
			OpINotEqual = 171,
			OpSGreaterThan = 173,
			OpSGreaterThanEqual = 175,
			OpSLessThan = 177,
			OpSLessThanEqual = 179,
			OpFOrdEqual = 180,
			OpFUnordNotEqual = 183,
			OpFOrdLessThan = 184,
			OpFOrdGreaterThan = 186,
			OpFOrdLessThanEqual = 188,
			OpFOrdGreaterThanEqual = 190,
			OpShiftRightArithmetic = 195,
			OpShiftLeftLogical = 196,
			OpBitwiseOr = 197,
			OpBitwiseXor = 198,
			OpBitwiseAnd = 199,
			OpNot = 200,
			OpDPdx = 207,
			OpDPdy = 208,
			OpFwidth = 209,
			OpLoopMerge = 246,
			OpSelectionMerge = 247,
			OpLabel = 248,
			OpBranch = 249,
			OpBranchConditional = 250,
			OpKill = 252,
			OpReturn = 253,
			OpReturnValue = 254
		};

		enum GLSLstd450 : uint32_t
		{
			GLSL450_Round = 1,
			GLSL450_Trunc = 3,
			GLSL450_FAbs = 4,
			GLSL450_SAbs = 5,
			GLSL450_FSign = 6,
			GLSL450_SSign = 7,
			GLSL450_Floor = 8,
			GLSL450_Ceil = 9,
			GLSL450_Fract = 10,
			GLSL450_Radians = 11,
			GLSL450_Degrees = 12,
			GLSL450_Sin = 13,
			GLSL450_Cos = 14,
			GLSL450_Tan = 15,
			GLSL450_Asin = 16,
			GLSL450_Acos = 17,
			GLSL450_Atan = 18,
			GLSL450_Atan2 = 25,
			GLSL450_Pow = 26,
			GLSL450_Exp = 27,
			GLSL450_Log = 28,
			GLSL450_Exp2 = 29,
			GLSL450_Log2 = 30,
			GLSL450_Sqrt = 31,
			GLSL450_InverseSqrt = 32,
			GLSL450_Determinant = 33,
			GLSL450_FMin = 37,
			GLSL450_SMin = 39,
			GLSL450_FMax = 40,
			GLSL450_SMax = 42,
			GLSL450_FClamp = 43,
			GLSL450_SClamp = 45,
			GLSL450_FMix = 46,
			GLSL450_Step = 48,
			GLSL450_SmoothStep = 49,
			GLSL450_Ldexp = 53,
			GLSL450_Length = 66,
			GLSL450_Distance = 67,
			GLSL450_Cross = 68,
			GLSL450_Normalize = 69,
			GLSL450_FaceForward = 70,
			GLSL450_Reflect = 71,
			GLSL450_Refract = 72
		};

		enum SpvStorageClass : uint32_t
		{
			SC_UniformConstant = 0,
			SC_Input = 1,
			SC_Uniform = 2,
			SC_Output = 3,
			SC_Private = 6,
			SC_Function = 7
		};

		enum SpvDecoration : uint32_t
		{
			Dec_Block = 2,
			Dec_RowMajor = 4,
			Dec_ColMajor = 5,
			Dec_ArrayStride = 6,
			Dec_MatrixStride = 7,
			Dec_BuiltIn = 11,
			Dec_Location = 30,
			Dec_Binding = 33,
			Dec_DescriptorSet = 34,
			Dec_Offset = 35
		};

		enum SpvBuiltIn : uint32_t
		{
			BI_Position = 0,
			BI_PointSize = 1,
			BI_FragCoord = 15,
			BI_FrontFacing = 17,
			BI_FragDepth = 22,
			BI_VertexIndex = 42,
			BI_InstanceIndex = 43
		};

		enum SpvDim : uint32_t
		{
			Dim_1D = 0,
			Dim_2D = 1,
			Dim_3D = 2,
			Dim_Cube = 3
		};

		enum SpvImageOperands : uint32_t
		{
			ImgOp_Bias = 0x1,
			ImgOp_Lod = 0x2,
			ImgOp_Grad = 0x4
		};

		std::string ToUpperStr(const std::string& s)
		{
			std::string r = s;
			for (char& c : r)
				c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
			return r;
		}

		void ParseSemanticAndIndex(const std::string& rawSem, std::string& outBaseUpper, uint32_t& outIndex)
		{
			outBaseUpper.clear();
			outIndex = 0;
			if (rawSem.empty())
				return;

			std::string s = ToUpperStr(rawSem);
			// Strip optional centroid/linear/nointerpolation prefixes or suffixes
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

		// Canonical Varying Location mapping (0..15) between VS Output and PS Input
		int32_t MapVaryingLocation(const std::string& baseUpper, uint32_t index)
		{
			if (baseUpper == "COLOR")
			{
				if (index == 0) return 0;
				if (index == 1) return 1;
				return (index == 2) ? 14 : 15;
			}
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

		uint32_t AlignUp(uint32_t val, uint32_t align)
		{
			return (val + align - 1u) & ~(align - 1u);
		}

		void AppendWord(std::vector<uint32_t>& words, uint32_t w)
		{
			words.push_back(w);
		}

		void EmitInst(std::vector<uint32_t>& sec, SpvOp op, const std::vector<uint32_t>& operands)
		{
			uint32_t wc = static_cast<uint32_t>(1 + operands.size());
			sec.push_back((wc << 16) | static_cast<uint32_t>(op));
			sec.insert(sec.end(), operands.begin(), operands.end());
		}

		std::vector<uint32_t> EncodeStringWords(const std::string& s)
		{
			size_t byteLen = s.size() + 1;
			size_t wordLen = (byteLen + 3) / 4;
			std::vector<uint32_t> out(wordLen, 0);
			if (!s.empty())
				std::memcpy(out.data(), s.data(), s.size());
			return out;
		}

		void EmitInstWithStr(std::vector<uint32_t>& sec, SpvOp op, const std::vector<uint32_t>& prefixOps, const std::string& str, const std::vector<uint32_t>& suffixOps = {})
		{
			auto strWords = EncodeStringWords(str);
			uint32_t wc = static_cast<uint32_t>(1 + prefixOps.size() + strWords.size() + suffixOps.size());
			sec.push_back((wc << 16) | static_cast<uint32_t>(op));
			sec.insert(sec.end(), prefixOps.begin(), prefixOps.end());
			sec.insert(sec.end(), strWords.begin(), strWords.end());
			sec.insert(sec.end(), suffixOps.begin(), suffixOps.end());
		}

		// =====================================================================
		// SPIR-V AST Compiler Context
		// =====================================================================
		struct SpirvModuleBuilder
		{
			bool isVertex = true;
			std::string entryPointName;
			const std::vector<FxSamplerDesc>& declaredSamplers;
			const std::vector<FxUniformParam>& declaredParams;
			std::string errorMsg;

			uint32_t nextId = 1;
			uint32_t glsl450Id = 0;

			std::vector<uint32_t> secCapabilities;
			std::vector<uint32_t> secExtInstImports;
			std::vector<uint32_t> secMemoryModel;
			std::vector<uint32_t> secEntryPoints;
			std::vector<uint32_t> secExecutionModes;
			std::vector<uint32_t> secDebugNames;
			std::vector<uint32_t> secAnnotations;
			std::vector<uint32_t> secTypesConstsGlobals;
			std::vector<uint32_t> secFunctions;

			// Cached scalar/vector/matrix/pointer/array/struct types
			uint32_t tyVoid = 0;
			uint32_t tyBool = 0;
			uint32_t tyInt = 0;
			uint32_t tyFloat = 0;
			uint32_t tySampler = 0;
			std::map<std::pair<uint32_t, uint32_t>, uint32_t> vecTypeCache;
			std::map<std::pair<uint32_t, uint32_t>, uint32_t> matTypeCache;
			std::map<std::pair<uint32_t, uint32_t>, uint32_t> ptrTypeCache; // (storageClass, elemType)
			std::map<std::tuple<uint32_t, uint32_t, uint32_t>, uint32_t> arrTypeCache; // (elemType, count, stride)
			std::map<uint32_t, uint32_t> imageTypeCache; // SpvDim -> imageTypeId
			std::map<uint32_t, uint32_t> sampledImgTypeCache; // imageTypeId -> sampledImgTypeId
			std::unordered_map<const TTypeList*, uint32_t> valueStructCache;
			std::unordered_map<const TTypeList*, uint32_t> uniformStructCache;
			std::map<std::vector<uint32_t>, uint32_t> fnTypeCache; // [retType, arg0, arg1...] -> fnTypeId

			// Constants cache
			uint32_t constTrue = 0;
			uint32_t constFalse = 0;
			std::unordered_map<int32_t, uint32_t> intConstCache;
			std::unordered_map<uint32_t, uint32_t> floatBitsConstCache;
			std::unordered_map<uint32_t, uint32_t> nullConstCache;

			// Uniforms & Resources
			struct UniformFieldInfo
			{
				int symId = -1;
				std::string name;
				std::string semantic;
				TType astType;
				uint32_t memberIndex = 0;
				uint32_t byteOffset = 0;
				uint32_t byteSize = 0;
				bool isMutableCopy = false;
				uint32_t mutablePrivateVarId = 0;
			};
			uint32_t globalsBlockVarId = 0;
			uint32_t globalsStructTypeId = 0;
			uint32_t globalsByteSize = 0;
			std::vector<UniformFieldInfo> uniformFields;
			std::unordered_map<int, size_t> symIdToUniformIdx;

			struct SamplerResourceInfo
			{
				int symId = -1;
				std::string name;
				uint32_t slot = 0;
				SpvDim dim = Dim_2D;
				uint32_t imageVarId = 0;
				uint32_t samplerVarId = 0;
				uint32_t imageTypeId = 0;
				uint32_t sampledImgTypeId = 0;
			};
			std::unordered_map<int, SamplerResourceInfo> samplerBySymId;
			uint32_t nextAutoSamplerSlot = 0;

			// Global non-uniform variables (Private storage class)
			struct GlobalVarInfo
			{
				int symId = -1;
				uint32_t varId = 0;
				uint32_t valTypeId = 0;
				TIntermTyped* initExpr = nullptr;
			};
			std::unordered_map<int, GlobalVarInfo> privateGlobals;
			std::vector<int> privateGlobalInitOrder;

			// Helper functions
			struct ParamSpec
			{
				int symId = -1;
				std::string name;
				std::string semantic;
				TType astType;
				TQualifier qualifier = EvqIn;
				bool passByPtr = false;
				bool isSampler = false;
				SpvDim samplerDim = Dim_2D;
				uint32_t valTypeId = 0;
				uint32_t paramTypeId = 0;
				uint32_t smpParamTypeId = 0;
			};
			struct FuncRecord
			{
				TIntermAggregate* astNode = nullptr;
				std::string plainName;
				std::string mangledName;
				std::string returnSemantic;
				TType returnAstType;
				uint32_t funcId = 0;
				uint32_t retTypeId = 0;
				std::vector<ParamSpec> params;
				TIntermNode* bodyNode = nullptr;
			};
			std::unordered_map<std::string, FuncRecord> funcsByMangled;
			std::vector<std::string> orderedCalledFuncs;
			std::string mainTargetMangled;

			// Active function emission state
			struct LocalVarInfo
			{
				uint32_t ptrId = 0;
				uint32_t valTypeId = 0;
			};
			std::vector<uint32_t> fnLocalVarWords;
			std::vector<uint32_t> fnBodyWords;
			std::unordered_map<int, LocalVarInfo> fnLocals;
			std::unordered_map<int, SamplerResourceInfo> fnSamplerParams;
			std::map<uint32_t, SamplerResourceInfo> fallbackSamplerByDim;
			bool blockTerminated = false;
			std::vector<std::pair<uint32_t, uint32_t>> loopStack; // (mergeLabel, continueLabel)
			uint32_t currentFnRetTypeId = 0;

			SpirvModuleBuilder(
				bool vertex,
				const std::string& entry,
				const std::vector<FxSamplerDesc>& samps,
				const std::vector<FxUniformParam>& params)
				: isVertex(vertex)
				, entryPointName(entry)
				, declaredSamplers(samps)
				, declaredParams(params)
			{
			}

			uint32_t AllocId() { return nextId++; }

			uint32_t GetVoidType()
			{
				if (!tyVoid)
				{
					tyVoid = AllocId();
					EmitInst(secTypesConstsGlobals, OpTypeVoid, { tyVoid });
				}
				return tyVoid;
			}

			uint32_t GetBoolType()
			{
				if (!tyBool)
				{
					tyBool = AllocId();
					EmitInst(secTypesConstsGlobals, OpTypeBool, { tyBool });
				}
				return tyBool;
			}

			uint32_t GetIntType()
			{
				if (!tyInt)
				{
					tyInt = AllocId();
					EmitInst(secTypesConstsGlobals, OpTypeInt, { tyInt, 32u, 1u });
				}
				return tyInt;
			}

			uint32_t GetFloatType()
			{
				if (!tyFloat)
				{
					tyFloat = AllocId();
					EmitInst(secTypesConstsGlobals, OpTypeFloat, { tyFloat, 32u });
				}
				return tyFloat;
			}

			uint32_t GetVecType(uint32_t compType, uint32_t count)
			{
				if (count <= 1)
					return compType;
				auto key = std::make_pair(compType, count);
				auto it = vecTypeCache.find(key);
				if (it != vecTypeCache.end())
					return it->second;
				uint32_t id = AllocId();
				EmitInst(secTypesConstsGlobals, OpTypeVector, { id, compType, count });
				vecTypeCache[key] = id;
				return id;
			}

			uint32_t GetMatType(uint32_t cols, uint32_t rows)
			{
				uint32_t colVec = GetVecType(GetFloatType(), rows);
				auto key = std::make_pair(colVec, cols);
				auto it = matTypeCache.find(key);
				if (it != matTypeCache.end())
					return it->second;
				uint32_t id = AllocId();
				EmitInst(secTypesConstsGlobals, OpTypeMatrix, { id, colVec, cols });
				matTypeCache[key] = id;
				return id;
			}

			uint32_t GetPtrType(SpvStorageClass sc, uint32_t elemType)
			{
				auto key = std::make_pair(static_cast<uint32_t>(sc), elemType);
				auto it = ptrTypeCache.find(key);
				if (it != ptrTypeCache.end())
					return it->second;
				uint32_t id = AllocId();
				EmitInst(secTypesConstsGlobals, OpTypePointer, { id, static_cast<uint32_t>(sc), elemType });
				ptrTypeCache[key] = id;
				return id;
			}

			uint32_t GetConstBool(bool v)
			{
				uint32_t bt = GetBoolType();
				if (v)
				{
					if (!constTrue)
					{
						constTrue = AllocId();
						EmitInst(secTypesConstsGlobals, OpConstantTrue, { bt, constTrue });
					}
					return constTrue;
				}
				else
				{
					if (!constFalse)
					{
						constFalse = AllocId();
						EmitInst(secTypesConstsGlobals, OpConstantFalse, { bt, constFalse });
					}
					return constFalse;
				}
			}

			uint32_t GetConstInt(int32_t v)
			{
				auto it = intConstCache.find(v);
				if (it != intConstCache.end())
					return it->second;
				uint32_t itype = GetIntType();
				uint32_t id = AllocId();
				uint32_t bits = 0;
				std::memcpy(&bits, &v, 4);
				EmitInst(secTypesConstsGlobals, OpConstant, { itype, id, bits });
				intConstCache[v] = id;
				return id;
			}

			uint32_t GetConstFloat(float v)
			{
				if (v == 0.0f) v = 0.0f; // canonicalize -0.0f
				uint32_t bits = 0;
				std::memcpy(&bits, &v, 4);
				auto it = floatBitsConstCache.find(bits);
				if (it != floatBitsConstCache.end())
					return it->second;
				uint32_t ftype = GetFloatType();
				uint32_t id = AllocId();
				EmitInst(secTypesConstsGlobals, OpConstant, { ftype, id, bits });
				floatBitsConstCache[bits] = id;
				return id;
			}

			uint32_t GetConstNull(uint32_t typeId)
			{
				auto it = nullConstCache.find(typeId);
				if (it != nullConstCache.end())
					return it->second;
				uint32_t id = AllocId();
				EmitInst(secTypesConstsGlobals, OpConstantNull, { typeId, id });
				nullConstCache[typeId] = id;
				return id;
			}

			uint32_t GetSamplerType()
			{
				if (!tySampler)
				{
					tySampler = AllocId();
					EmitInst(secTypesConstsGlobals, OpTypeSampler, { tySampler });
				}
				return tySampler;
			}

			uint32_t GetImageType(SpvDim dim)
			{
				auto it = imageTypeCache.find(static_cast<uint32_t>(dim));
				if (it != imageTypeCache.end())
					return it->second;
				uint32_t ftype = GetFloatType();
				uint32_t id = AllocId();
				// OpTypeImage %float Dim Depth=0 Arrayed=0 MS=0 Sampled=1 Format=Unknown(0)
				EmitInst(secTypesConstsGlobals, OpTypeImage, { id, ftype, static_cast<uint32_t>(dim), 0u, 0u, 0u, 1u, 0u });
				imageTypeCache[static_cast<uint32_t>(dim)] = id;
				return id;
			}

			uint32_t GetSampledImageType(uint32_t imgTypeId)
			{
				auto it = sampledImgTypeCache.find(imgTypeId);
				if (it != sampledImgTypeCache.end())
					return it->second;
				uint32_t id = AllocId();
				EmitInst(secTypesConstsGlobals, OpTypeSampledImage, { id, imgTypeId });
				sampledImgTypeCache[imgTypeId] = id;
				return id;
			}

			uint32_t GetFunctionType(uint32_t retType, const std::vector<uint32_t>& paramTypes)
			{
				std::vector<uint32_t> key;
				key.reserve(1 + paramTypes.size());
				key.push_back(retType);
				key.insert(key.end(), paramTypes.begin(), paramTypes.end());
				auto it = fnTypeCache.find(key);
				if (it != fnTypeCache.end())
					return it->second;
				uint32_t id = AllocId();
				std::vector<uint32_t> ops;
				ops.reserve(1 + key.size());
				ops.push_back(id);
				ops.insert(ops.end(), key.begin(), key.end());
				EmitInst(secTypesConstsGlobals, OpTypeFunction, ops);
				fnTypeCache[key] = id;
				return id;
			}

			// Value SPIR-V type (used for locals, parameters, expressions, return values)
			uint32_t GetValueType(const TType& t, bool ignoreArray = false)
			{
				if (!ignoreArray && t.isArray())
				{
					uint32_t elemId = GetValueType(t, true);
					int count = (std::max)(1, t.getArraySize());
					auto key = std::make_tuple(elemId, static_cast<uint32_t>(count), 0u);
					auto it = arrTypeCache.find(key);
					if (it != arrTypeCache.end())
						return it->second;
					uint32_t cntId = GetConstInt(count);
					uint32_t id = AllocId();
					EmitInst(secTypesConstsGlobals, OpTypeArray, { id, elemId, cntId });
					arrTypeCache[key] = id;
					return id;
				}

				if (t.getBasicType() == EbtVoid)
					return GetVoidType();
				if (t.getBasicType() == EbtStruct)
				{
					const TTypeList* sl = t.getStruct();
					auto it = valueStructCache.find(sl);
					if (it != valueStructCache.end())
						return it->second;
					std::vector<uint32_t> memberTypes;
					if (sl)
					{
						for (size_t i = 0; i < sl->size(); ++i)
							memberTypes.push_back(GetValueType(*(*sl)[i].type));
					}
					uint32_t id = AllocId();
					std::vector<uint32_t> ops;
					ops.push_back(id);
					ops.insert(ops.end(), memberTypes.begin(), memberTypes.end());
					EmitInst(secTypesConstsGlobals, OpTypeStruct, ops);
					valueStructCache[sl] = id;
					return id;
				}

				if (t.isMatrix())
					return GetMatType(static_cast<uint32_t>(t.getColsCount()), static_cast<uint32_t>(t.getRowsCount()));

				uint32_t baseId = GetFloatType();
				if (t.getBasicType() == EbtInt) baseId = GetIntType();
				else if (t.getBasicType() == EbtBool) baseId = GetBoolType();
				return GetVecType(baseId, static_cast<uint32_t>((std::max)(1, t.getRowsCount())));
			}

			// Uniform Block member SPIR-V type (std140 layout, bool stored as int32!)
			uint32_t GetUniformMemberType(const TType& t, uint32_t& outAlign, uint32_t& outSize, bool ignoreArray = false)
			{
				if (!ignoreArray && t.isArray())
				{
					uint32_t elemAlign = 4, elemSize = 4;
					uint32_t elemId = GetUniformMemberType(t, elemAlign, elemSize, true);
					uint32_t stride = AlignUp((std::max)(elemSize, elemAlign), 16u);
					uint32_t count = static_cast<uint32_t>((std::max)(1, t.getArraySize()));
					outAlign = 16u;
					outSize = stride * count;

					auto key = std::make_tuple(elemId, count, stride);
					auto it = arrTypeCache.find(key);
					if (it != arrTypeCache.end())
						return it->second;
					uint32_t cntId = GetConstInt(static_cast<int32_t>(count));
					uint32_t id = AllocId();
					EmitInst(secTypesConstsGlobals, OpTypeArray, { id, elemId, cntId });
					EmitInst(secAnnotations, OpDecorate, { id, Dec_ArrayStride, stride });
					arrTypeCache[key] = id;
					return id;
				}

				if (t.getBasicType() == EbtStruct)
				{
					const TTypeList* sl = t.getStruct();
					uint32_t curOff = 0;
					std::vector<uint32_t> memberTypes;
					std::vector<uint32_t> memberOffsets;
					std::vector<bool> memberIsMat;
					if (sl)
					{
						for (size_t i = 0; i < sl->size(); ++i)
						{
							const TType& ft = *(*sl)[i].type;
							uint32_t fAlign = 4, fSize = 4;
							uint32_t ftId = GetUniformMemberType(ft, fAlign, fSize);
							curOff = AlignUp(curOff, fAlign);
							memberTypes.push_back(ftId);
							memberOffsets.push_back(curOff);
							memberIsMat.push_back(ft.isMatrix());
							curOff += fSize;
						}
					}
					outAlign = 16u;
					outSize = AlignUp((std::max)(curOff, 16u), 16u);

					auto it = uniformStructCache.find(sl);
					if (it != uniformStructCache.end())
						return it->second;

					uint32_t id = AllocId();
					std::vector<uint32_t> ops;
					ops.push_back(id);
					ops.insert(ops.end(), memberTypes.begin(), memberTypes.end());
					EmitInst(secTypesConstsGlobals, OpTypeStruct, ops);
					for (uint32_t m = 0; m < static_cast<uint32_t>(memberOffsets.size()); ++m)
					{
						EmitInst(secAnnotations, OpMemberDecorate, { id, m, Dec_Offset, memberOffsets[m] });
						if (memberIsMat[m])
						{
							EmitInst(secAnnotations, OpMemberDecorate, { id, m, Dec_ColMajor });
							EmitInst(secAnnotations, OpMemberDecorate, { id, m, Dec_MatrixStride, 16u });
						}
					}
					uniformStructCache[sl] = id;
					return id;
				}

				if (t.isMatrix())
				{
					uint32_t cols = static_cast<uint32_t>(t.getColsCount());
					uint32_t rows = static_cast<uint32_t>(t.getRowsCount());
					outAlign = 16u;
					outSize = cols * 16u;
					return GetMatType(cols, rows);
				}

				uint32_t rows = static_cast<uint32_t>((std::max)(1, t.getRowsCount()));
				// Note: bool in SPIR-V Uniform block is stored as 32-bit int
				uint32_t baseId = (t.getBasicType() == EbtInt || t.getBasicType() == EbtBool) ? GetIntType() : GetFloatType();
				if (rows == 1) { outAlign = 4u; outSize = 4u; }
				else if (rows == 2) { outAlign = 8u; outSize = 8u; }
				else if (rows == 3) { outAlign = 16u; outSize = 12u; }
				else { outAlign = 16u; outSize = 16u; }
				return GetVecType(baseId, rows);
			}

			uint32_t AllocLocalVar(uint32_t valTypeId)
			{
				uint32_t ptrType = GetPtrType(SC_Function, valTypeId);
				uint32_t id = AllocId();
				EmitInst(fnLocalVarWords, OpVariable, { ptrType, id, SC_Function });
				return id;
			}

			void StartBlock(uint32_t labelId)
			{
				EmitInst(fnBodyWords, OpLabel, { labelId });
				blockTerminated = false;
			}

			void EmitBranchIfLive(uint32_t targetLabel)
			{
				if (!blockTerminated)
				{
					EmitInst(fnBodyWords, OpBranch, { targetLabel });
					blockTerminated = true;
				}
			}

			// Convert loaded Uniform representation (e.g. int32 bool, std140 array/struct) into shader Value representation
			uint32_t ConvertUniformLoadedToValue(uint32_t loadedId, const TType& astType, bool ignoreArray = false)
			{
				if (!ignoreArray && astType.isArray())
				{
					uint32_t dstArrType = GetValueType(astType, false);
					int count = (std::max)(1, astType.getArraySize());
					uint32_t uAlign = 4, uSize = 4;
					uint32_t uElemType = GetUniformMemberType(astType, uAlign, uSize, true);
					std::vector<uint32_t> elems;
					elems.reserve(static_cast<size_t>(count));
					for (int i = 0; i < count; ++i)
					{
						uint32_t extId = AllocId();
						EmitInst(fnBodyWords, OpCompositeExtract, { uElemType, extId, loadedId, static_cast<uint32_t>(i) });
						elems.push_back(ConvertUniformLoadedToValue(extId, astType, true));
					}
					uint32_t resId = AllocId();
					std::vector<uint32_t> ops = { dstArrType, resId };
					ops.insert(ops.end(), elems.begin(), elems.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, ops);
					return resId;
				}

				if (astType.getBasicType() == EbtStruct)
				{
					const TTypeList* sl = astType.getStruct();
					uint32_t dstStructType = GetValueType(astType, true);
					std::vector<uint32_t> fields;
					if (sl)
					{
						for (size_t i = 0; i < sl->size(); ++i)
						{
							const TType& ft = *(*sl)[i].type;
							uint32_t uAlign = 4, uSize = 4;
							uint32_t uFieldType = GetUniformMemberType(ft, uAlign, uSize);
							uint32_t extId = AllocId();
							EmitInst(fnBodyWords, OpCompositeExtract, { uFieldType, extId, loadedId, static_cast<uint32_t>(i) });
							fields.push_back(ConvertUniformLoadedToValue(extId, ft));
						}
					}
					uint32_t resId = AllocId();
					std::vector<uint32_t> ops = { dstStructType, resId };
					ops.insert(ops.end(), fields.begin(), fields.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, ops);
					return resId;
				}

				if (astType.getBasicType() == EbtBool)
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, astType.getRowsCount()));
					uint32_t bType = GetVecType(GetBoolType(), rows);
					uint32_t zeroInt = GetConstInt(0);
					uint32_t zeroId = (rows > 1) ? SplatScalar(zeroInt, GetIntType(), rows) : zeroInt;
					uint32_t resId = AllocId();
					EmitInst(fnBodyWords, OpINotEqual, { bType, resId, loadedId, zeroId });
					return resId;
				}

				return loadedId;
			}

			uint32_t SplatScalar(uint32_t scalarId, uint32_t compType, uint32_t count)
			{
				if (count <= 1)
					return scalarId;
				uint32_t vecType = GetVecType(compType, count);
				uint32_t id = AllocId();
				std::vector<uint32_t> ops = { vecType, id };
				for (uint32_t i = 0; i < count; ++i)
					ops.push_back(scalarId);
				EmitInst(fnBodyWords, OpCompositeConstruct, ops);
				return id;
			}

			uint32_t CastScalar(uint32_t valId, TBasicType srcBt, TBasicType dstBt)
			{
				if (srcBt == dstBt)
					return valId;
				if (dstBt == EbtFloat)
				{
					uint32_t ft = GetFloatType();
					if (srcBt == EbtInt)
					{
						uint32_t id = AllocId();
						EmitInst(fnBodyWords, OpConvertSToF, { ft, id, valId });
						return id;
					}
					if (srcBt == EbtBool)
					{
						uint32_t id = AllocId();
						EmitInst(fnBodyWords, OpSelect, { ft, id, valId, GetConstFloat(1.0f), GetConstFloat(0.0f) });
						return id;
					}
				}
				else if (dstBt == EbtInt)
				{
					uint32_t it = GetIntType();
					if (srcBt == EbtFloat)
					{
						uint32_t id = AllocId();
						EmitInst(fnBodyWords, OpConvertFToS, { it, id, valId });
						return id;
					}
					if (srcBt == EbtBool)
					{
						uint32_t id = AllocId();
						EmitInst(fnBodyWords, OpSelect, { it, id, valId, GetConstInt(1), GetConstInt(0) });
						return id;
					}
				}
				else if (dstBt == EbtBool)
				{
					uint32_t bt = GetBoolType();
					if (srcBt == EbtFloat)
					{
						uint32_t id = AllocId();
						EmitInst(fnBodyWords, OpFUnordNotEqual, { bt, id, valId, GetConstFloat(0.0f) });
						return id;
					}
					if (srcBt == EbtInt)
					{
						uint32_t id = AllocId();
						EmitInst(fnBodyWords, OpINotEqual, { bt, id, valId, GetConstInt(0) });
						return id;
					}
				}
				return valId;
			}

			// Coerce vector/scalar value to target basicType and component count
			uint32_t CoerceVectorOrScalar(uint32_t valId, const TType& srcType, TBasicType dstBt, uint32_t dstRows)
			{
				uint32_t srcRows = static_cast<uint32_t>((std::max)(1, srcType.getRowsCount()));
				TBasicType srcBt = srcType.getBasicType();

				// 1. Adjust component count first
				uint32_t curId = valId;
				if (srcRows == 1 && dstRows > 1)
				{
					uint32_t srcCompType = (srcBt == EbtInt) ? GetIntType() : (srcBt == EbtBool ? GetBoolType() : GetFloatType());
					curId = SplatScalar(curId, srcCompType, dstRows);
					srcRows = dstRows;
				}
				else if (srcRows > dstRows)
				{
					uint32_t srcCompType = (srcBt == EbtInt) ? GetIntType() : (srcBt == EbtBool ? GetBoolType() : GetFloatType());
					if (dstRows == 1)
					{
						uint32_t extId = AllocId();
						EmitInst(fnBodyWords, OpCompositeExtract, { srcCompType, extId, curId, 0u });
						curId = extId;
					}
					else
					{
						uint32_t dstVecSrcBt = GetVecType(srcCompType, dstRows);
						uint32_t shufId = AllocId();
						std::vector<uint32_t> ops = { dstVecSrcBt, shufId, curId, curId };
						for (uint32_t i = 0; i < dstRows; ++i)
							ops.push_back(i);
						EmitInst(fnBodyWords, OpVectorShuffle, ops);
						curId = shufId;
					}
					srcRows = dstRows;
				}
				else if (srcRows < dstRows)
				{
					// Pad missing components with 0 (and 1 for w when padding to vec4)
					uint32_t srcCompType = (srcBt == EbtInt) ? GetIntType() : (srcBt == EbtBool ? GetBoolType() : GetFloatType());
					std::vector<uint32_t> comps;
					for (uint32_t i = 0; i < srcRows; ++i)
					{
						uint32_t ext = AllocId();
						EmitInst(fnBodyWords, OpCompositeExtract, { srcCompType, ext, curId, i });
						comps.push_back(ext);
					}
					for (uint32_t i = srcRows; i < dstRows; ++i)
					{
						if (srcBt == EbtInt) comps.push_back(GetConstInt(0));
						else if (srcBt == EbtBool) comps.push_back(GetConstBool(false));
						else comps.push_back(GetConstFloat(0.0f));
					}
					uint32_t vecT = GetVecType(srcCompType, dstRows);
					uint32_t cid = AllocId();
					std::vector<uint32_t> ops = { vecT, cid };
					ops.insert(ops.end(), comps.begin(), comps.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, ops);
					curId = cid;
					srcRows = dstRows;
				}

				// 2. Convert basic type if needed
				if (srcBt == dstBt)
					return curId;

				uint32_t dstCompType = (dstBt == EbtInt) ? GetIntType() : (dstBt == EbtBool ? GetBoolType() : GetFloatType());
				uint32_t dstType = GetVecType(dstCompType, dstRows);

				if (dstRows == 1)
					return CastScalar(curId, srcBt, dstBt);

				if (srcBt == EbtInt && dstBt == EbtFloat)
				{
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpConvertSToF, { dstType, id, curId });
					return id;
				}
				if (srcBt == EbtFloat && dstBt == EbtInt)
				{
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpConvertFToS, { dstType, id, curId });
					return id;
				}
				if (srcBt == EbtBool && dstBt == EbtFloat)
				{
					uint32_t oneVec = SplatScalar(GetConstFloat(1.0f), GetFloatType(), dstRows);
					uint32_t zeroVec = SplatScalar(GetConstFloat(0.0f), GetFloatType(), dstRows);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpSelect, { dstType, id, curId, oneVec, zeroVec });
					return id;
				}
				if (srcBt == EbtBool && dstBt == EbtInt)
				{
					uint32_t oneVec = SplatScalar(GetConstInt(1), GetIntType(), dstRows);
					uint32_t zeroVec = SplatScalar(GetConstInt(0), GetIntType(), dstRows);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpSelect, { dstType, id, curId, oneVec, zeroVec });
					return id;
				}
				if (srcBt == EbtFloat && dstBt == EbtBool)
				{
					uint32_t zeroVec = SplatScalar(GetConstFloat(0.0f), GetFloatType(), dstRows);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpFUnordNotEqual, { dstType, id, curId, zeroVec });
					return id;
				}
				if (srcBt == EbtInt && dstBt == EbtBool)
				{
					uint32_t zeroVec = SplatScalar(GetConstInt(0), GetIntType(), dstRows);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpINotEqual, { dstType, id, curId, zeroVec });
					return id;
				}
				return curId;
			}

			uint32_t CoerceToType(uint32_t valId, const TType& srcType, const TType& dstType)
			{
				if (srcType == dstType)
					return valId;
				if (!srcType.isArray() && !dstType.isArray() && !srcType.isMatrix() && !dstType.isMatrix() &&
					srcType.getBasicType() != EbtStruct && dstType.getBasicType() != EbtStruct)
				{
					return CoerceVectorOrScalar(valId, srcType, dstType.getBasicType(), static_cast<uint32_t>((std::max)(1, dstType.getRowsCount())));
				}
				return valId;
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

			// Emit constant or zero-initializer value
			uint32_t EmitConstantNode(TIntermConstant* c)
			{
				const TType& t = c->getType();
				if (t.getBasicType() == EbtStruct)
				{
					const TTypeList* sl = t.getStruct();
					uint32_t stId = GetValueType(t);
					if (c->getCount() <= 1 && c->toFloat(0) == 0.0f && c->toInt(0) == 0)
						return GetConstNull(stId);

					// Build struct field by field
					unsigned cursor = 0;
					std::function<uint32_t(const TType&)> buildField = [&](const TType& ft) -> uint32_t {
						if (ft.isArray())
						{
							int n = (std::max)(1, ft.getArraySize());
							TType elemT = ft;
							elemT.clearArrayness();
							std::vector<uint32_t> elems;
							for (int i = 0; i < n; ++i)
								elems.push_back(buildField(elemT));
							uint32_t arrT = GetValueType(ft);
							uint32_t id = AllocId();
							std::vector<uint32_t> ops = { arrT, id };
							ops.insert(ops.end(), elems.begin(), elems.end());
							EmitInst(fnBodyWords, OpCompositeConstruct, ops);
							return id;
						}
						if (ft.getBasicType() == EbtStruct)
						{
							const TTypeList* sub = ft.getStruct();
							std::vector<uint32_t> subFields;
							if (sub)
							{
								for (size_t k = 0; k < sub->size(); ++k)
									subFields.push_back(buildField(*(*sub)[k].type));
							}
							uint32_t subT = GetValueType(ft);
							uint32_t id = AllocId();
							std::vector<uint32_t> ops = { subT, id };
							ops.insert(ops.end(), subFields.begin(), subFields.end());
							EmitInst(fnBodyWords, OpCompositeConstruct, ops);
							return id;
						}
						int count = ft.getInstanceSize();
						std::vector<uint32_t> scalars;
						for (int k = 0; k < count; ++k)
						{
							unsigned idx = (std::min)(cursor++, (std::max)(1u, c->getCount()) - 1u);
							const auto& val = c->getValue(idx);
							if (ft.getBasicType() == EbtInt)
								scalars.push_back(GetConstInt(val.type == EbtFloat ? static_cast<int>(val.asFloat) : val.asInt));
							else if (ft.getBasicType() == EbtBool)
								scalars.push_back(GetConstBool(val.type == EbtFloat ? (val.asFloat != 0.0f) : (val.asBool)));
							else
								scalars.push_back(GetConstFloat(val.type == EbtInt ? static_cast<float>(val.asInt) : val.asFloat));
						}
						if (ft.isMatrix())
						{
							int cols = ft.getColsCount();
							int rows = ft.getRowsCount();
							uint32_t colVecT = GetVecType(GetFloatType(), static_cast<uint32_t>(rows));
							std::vector<uint32_t> colsVec;
							for (int col = 0; col < cols; ++col)
							{
								uint32_t cv = AllocId();
								std::vector<uint32_t> cops = { colVecT, cv };
								for (int r = 0; r < rows; ++r)
									cops.push_back(scalars[static_cast<size_t>(col * rows + r)]);
								EmitInst(fnBodyWords, OpCompositeConstruct, cops);
								colsVec.push_back(cv);
							}
							uint32_t mt = GetValueType(ft);
							uint32_t mid = AllocId();
							std::vector<uint32_t> mops = { mt, mid };
							mops.insert(mops.end(), colsVec.begin(), colsVec.end());
							EmitInst(fnBodyWords, OpCompositeConstruct, mops);
							return mid;
						}
						if (scalars.size() == 1)
							return scalars[0];
						uint32_t vt = GetValueType(ft);
						uint32_t vid = AllocId();
						std::vector<uint32_t> vops = { vt, vid };
						vops.insert(vops.end(), scalars.begin(), scalars.end());
						EmitInst(fnBodyWords, OpCompositeConstruct, vops);
						return vid;
					};

					std::vector<uint32_t> topFields;
					if (sl)
					{
						for (size_t i = 0; i < sl->size(); ++i)
							topFields.push_back(buildField(*(*sl)[i].type));
					}
					uint32_t resId = AllocId();
					std::vector<uint32_t> ops = { stId, resId };
					ops.insert(ops.end(), topFields.begin(), topFields.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, ops);
					return resId;
				}

				int nElems = t.getInstanceSize();
				unsigned nConsts = (std::max)(1u, c->getCount());
				std::vector<uint32_t> scIds;
				scIds.reserve(static_cast<size_t>(nElems));
				for (int i = 0; i < nElems; ++i)
				{
					unsigned idx = (std::min)(static_cast<unsigned>(i), nConsts - 1u);
					if (t.getBasicType() == EbtInt)
						scIds.push_back(GetConstInt(c->toInt(idx)));
					else if (t.getBasicType() == EbtBool)
						scIds.push_back(GetConstBool(c->toBool(idx)));
					else
						scIds.push_back(GetConstFloat(c->toFloat(idx)));
				}

				if (t.isMatrix())
				{
					int cols = t.getColsCount();
					int rows = t.getRowsCount();
					uint32_t colVecT = GetVecType(GetFloatType(), static_cast<uint32_t>(rows));
					std::vector<uint32_t> colIds;
					for (int col = 0; col < cols; ++col)
					{
						uint32_t cid = AllocId();
						std::vector<uint32_t> cops = { colVecT, cid };
						for (int r = 0; r < rows; ++r)
							cops.push_back(scIds[static_cast<size_t>(col * rows + r)]);
						EmitInst(fnBodyWords, OpCompositeConstruct, cops);
						colIds.push_back(cid);
					}
					uint32_t matT = GetValueType(t);
					uint32_t mid = AllocId();
					std::vector<uint32_t> mops = { matT, mid };
					mops.insert(mops.end(), colIds.begin(), colIds.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, mops);
					return mid;
				}

				if (nElems <= 1)
					return scIds[0];

				uint32_t vecT = GetValueType(t);
				uint32_t vid = AllocId();
				std::vector<uint32_t> ops = { vecT, vid };
				ops.insert(ops.end(), scIds.begin(), scIds.end());
				EmitInst(fnBodyWords, OpCompositeConstruct, ops);
				return vid;
			}

			// Try to emit direct OpAccessChain into $Globals Uniform block for chained field/array indexing
			bool TryEmitUniformLoad(TIntermTyped* node, uint32_t& outValId)
			{
				std::vector<TIntermBinary*> chain;
				TIntermTyped* cur = node;
				while (cur)
				{
					TIntermBinary* bin = cur->getAsBinaryNode();
					if (bin && (bin->getOp() == EOpIndexDirectStruct || bin->getOp() == EOpIndexDirect || bin->getOp() == EOpIndexIndirect))
					{
						// Stop before vector/matrix component subscript so we load the vector/matrix and extract cleanly
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
					return false; // Mutable uniform lives in Private storage class

				std::vector<uint32_t> indices;
				indices.push_back(GetConstInt(static_cast<int32_t>(ufi.memberIndex)));
				for (auto it = chain.rbegin(); it != chain.rend(); ++it)
				{
					TIntermBinary* b = *it;
					if (b->getOp() == EOpIndexDirectStruct)
					{
						indices.push_back(GetConstInt(ExtractImmediateInt(b->getRight())));
					}
					else
					{
						uint32_t idxVal = EmitRValue(b->getRight());
						idxVal = CoerceVectorOrScalar(idxVal, b->getRight()->getType(), EbtInt, 1);
						indices.push_back(idxVal);
					}
				}

				const TType& resType = chain.empty() ? baseSym->getType() : chain.front()->getType();
				uint32_t uAlign = 4, uSize = 4;
				uint32_t uElemType = GetUniformMemberType(resType, uAlign, uSize);
				uint32_t uPtrType = GetPtrType(SC_Uniform, uElemType);
				uint32_t ptrId = AllocId();
				std::vector<uint32_t> acOps = { uPtrType, ptrId, globalsBlockVarId };
				acOps.insert(acOps.end(), indices.begin(), indices.end());
				EmitInst(fnBodyWords, OpAccessChain, acOps);

				uint32_t loadedId = AllocId();
				EmitInst(fnBodyWords, OpLoad, { uElemType, loadedId, ptrId });
				outValId = ConvertUniformLoadedToValue(loadedId, resType);
				return true;
			}

			// L-value descriptor supporting variables, struct/array/matrix/vector indexing, and vector swizzles!
			struct LValueRef
			{
				uint32_t basePtrId = 0;
				SpvStorageClass storageClass = SC_Function;
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
						// Collect access chain down to Function/Private base symbol
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

						std::vector<uint32_t> indices;
						for (auto it = steps.rbegin(); it != steps.rend(); ++it)
						{
							TIntermBinary* b = *it;
							if (b->getOp() == EOpIndexDirectStruct)
							{
								indices.push_back(GetConstInt(ExtractImmediateInt(b->getRight())));
							}
							else
							{
								uint32_t idxVal = EmitRValue(b->getRight());
								idxVal = CoerceVectorOrScalar(idxVal, b->getRight()->getType(), EbtInt, 1);
								indices.push_back(idxVal);
							}
						}

						uint32_t leafTypeId = GetValueType(node->getType());
						uint32_t leafPtrType = GetPtrType(rootLv.storageClass, leafTypeId);
						uint32_t acId = AllocId();
						std::vector<uint32_t> ops = { leafPtrType, acId, rootLv.basePtrId };
						ops.insert(ops.end(), indices.begin(), indices.end());
						EmitInst(fnBodyWords, OpAccessChain, ops);

						outLv.basePtrId = acId;
						outLv.storageClass = rootLv.storageClass;
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
						outLv.basePtrId = lit->second.ptrId;
						outLv.storageClass = SC_Function;
						outLv.baseType = sym->getType();
						outLv.hasSwizzle = false;
						outLv.resultType = sym->getType();
						return true;
					}
					auto pit = privateGlobals.find(sid);
					if (pit != privateGlobals.end())
					{
						outLv.basePtrId = pit->second.varId;
						outLv.storageClass = SC_Private;
						outLv.baseType = sym->getType();
						outLv.hasSwizzle = false;
						outLv.resultType = sym->getType();
						return true;
					}
					auto uit = symIdToUniformIdx.find(sid);
					if (uit != symIdToUniformIdx.end() && uniformFields[uit->second].isMutableCopy)
					{
						outLv.basePtrId = uniformFields[uit->second].mutablePrivateVarId;
						outLv.storageClass = SC_Private;
						outLv.baseType = sym->getType();
						outLv.hasSwizzle = false;
						outLv.resultType = sym->getType();
						return true;
					}
				}
				return false;
			}

			uint32_t LoadFromLValue(const LValueRef& lv)
			{
				uint32_t baseValType = GetValueType(lv.baseType);
				uint32_t baseVal = AllocId();
				EmitInst(fnBodyWords, OpLoad, { baseValType, baseVal, lv.basePtrId });
				if (!lv.hasSwizzle)
					return baseVal;

				uint32_t compType = (lv.baseType.getBasicType() == EbtInt) ? GetIntType()
					: (lv.baseType.getBasicType() == EbtBool ? GetBoolType() : GetFloatType());
				if (lv.swizzleIndices.size() == 1)
				{
					uint32_t extId = AllocId();
					EmitInst(fnBodyWords, OpCompositeExtract, { compType, extId, baseVal, lv.swizzleIndices[0] });
					return extId;
				}
				uint32_t vecType = GetVecType(compType, static_cast<uint32_t>(lv.swizzleIndices.size()));
				uint32_t shufId = AllocId();
				std::vector<uint32_t> ops = { vecType, shufId, baseVal, baseVal };
				ops.insert(ops.end(), lv.swizzleIndices.begin(), lv.swizzleIndices.end());
				EmitInst(fnBodyWords, OpVectorShuffle, ops);
				return shufId;
			}

			void StoreToLValue(const LValueRef& lv, uint32_t rhsValId)
			{
				if (!lv.hasSwizzle)
				{
					EmitInst(fnBodyWords, OpStore, { lv.basePtrId, rhsValId });
					return;
				}

				uint32_t baseValType = GetValueType(lv.baseType);
				uint32_t curVec = AllocId();
				EmitInst(fnBodyWords, OpLoad, { baseValType, curVec, lv.basePtrId });

				uint32_t compType = (lv.baseType.getBasicType() == EbtInt) ? GetIntType()
					: (lv.baseType.getBasicType() == EbtBool ? GetBoolType() : GetFloatType());

				if (lv.swizzleIndices.size() == 1)
				{
					uint32_t updated = AllocId();
					EmitInst(fnBodyWords, OpCompositeInsert, { baseValType, updated, rhsValId, curVec, lv.swizzleIndices[0] });
					EmitInst(fnBodyWords, OpStore, { lv.basePtrId, updated });
				}
				else
				{
					for (size_t i = 0; i < lv.swizzleIndices.size(); ++i)
					{
						uint32_t compVal = AllocId();
						EmitInst(fnBodyWords, OpCompositeExtract, { compType, compVal, rhsValId, static_cast<uint32_t>(i) });
						uint32_t nextVec = AllocId();
						EmitInst(fnBodyWords, OpCompositeInsert, { baseValType, nextVec, compVal, curVec, lv.swizzleIndices[i] });
						curVec = nextVec;
					}
					EmitInst(fnBodyWords, OpStore, { lv.basePtrId, curVec });
				}
			}

			uint32_t ApplyNumericBinaryOp(TOperator op, uint32_t lhsId, const TType& lhsType, uint32_t rhsId, const TType& rhsType, const TType& resType)
			{
				// Handle matrix/vector scaling & products first
				if (op == EOpVectorTimesScalar || (op == EOpMul && lhsType.isVector() && rhsType.isScalar()))
				{
					uint32_t v = (lhsType.isVector()) ? lhsId : rhsId;
					uint32_t s = (lhsType.isVector()) ? rhsId : lhsId;
					const TType& st = (lhsType.isVector()) ? rhsType : lhsType;
					if (resType.getBasicType() == EbtFloat)
					{
						s = CoerceVectorOrScalar(s, st, EbtFloat, 1);
						uint32_t id = AllocId();
						EmitInst(fnBodyWords, OpVectorTimesScalar, { GetValueType(resType), id, v, s });
						return id;
					}
				}
				if (op == EOpMul && lhsType.isScalar() && rhsType.isVector() && resType.getBasicType() == EbtFloat)
				{
					uint32_t s = CoerceVectorOrScalar(lhsId, lhsType, EbtFloat, 1);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpVectorTimesScalar, { GetValueType(resType), id, rhsId, s });
					return id;
				}
				if (op == EOpMatrixTimesScalar || (op == EOpMul && (lhsType.isMatrix() ^ rhsType.isMatrix()) && (lhsType.isScalar() || rhsType.isScalar())))
				{
					uint32_t m = lhsType.isMatrix() ? lhsId : rhsId;
					uint32_t s = lhsType.isMatrix() ? rhsId : lhsId;
					const TType& st = lhsType.isMatrix() ? rhsType : lhsType;
					s = CoerceVectorOrScalar(s, st, EbtFloat, 1);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpMatrixTimesScalar, { GetValueType(resType), id, m, s });
					return id;
				}
				if (op == EOpVectorTimesMatrix)
				{
					// Remember: matrices are stored transposed (M^T) matching OpenGL GL_FALSE & DirectX memcpy!
					// HLSL v * M (EOpVectorTimesMatrix) -> M_col * v -> OpMatrixTimesVector(rhs, lhs)
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpMatrixTimesVector, { GetValueType(resType), id, rhsId, lhsId });
					return id;
				}
				if (op == EOpMatrixTimesVector)
				{
					// HLSL M * v -> v * M_col -> OpVectorTimesMatrix(rhs, lhs)
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpVectorTimesMatrix, { GetValueType(resType), id, rhsId, lhsId });
					return id;
				}
				if (op == EOpMatrixTimesMatrix)
				{
					// Component-wise HLSL M1 * M2
					int cols = resType.getColsCount();
					int rows = resType.getRowsCount();
					uint32_t colVecT = GetVecType(GetFloatType(), static_cast<uint32_t>(rows));
					std::vector<uint32_t> colsOut;
					for (int c = 0; c < cols; ++c)
					{
						uint32_t c1 = AllocId(), c2 = AllocId(), cm = AllocId();
						EmitInst(fnBodyWords, OpCompositeExtract, { colVecT, c1, lhsId, static_cast<uint32_t>(c) });
						EmitInst(fnBodyWords, OpCompositeExtract, { colVecT, c2, rhsId, static_cast<uint32_t>(c) });
						EmitInst(fnBodyWords, OpFMul, { colVecT, cm, c1, c2 });
						colsOut.push_back(cm);
					}
					uint32_t id = AllocId();
					std::vector<uint32_t> ops = { GetValueType(resType), id };
					ops.insert(ops.end(), colsOut.begin(), colsOut.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, ops);
					return id;
				}

				// Component-wise matrix +/- scalar or matrix
				if (resType.isMatrix())
				{
					int cols = resType.getColsCount();
					int rows = resType.getRowsCount();
					uint32_t colVecT = GetVecType(GetFloatType(), static_cast<uint32_t>(rows));
					SpvOp fop = OpFMul;
					if (op == EOpAdd || op == EOpAddAssign) fop = OpFAdd;
					else if (op == EOpSub || op == EOpSubAssign) fop = OpFSub;
					else if (op == EOpDiv || op == EOpDivAssign) fop = OpFDiv;

					std::vector<uint32_t> colsOut;
					for (int c = 0; c < cols; ++c)
					{
						uint32_t c1 = 0, c2 = 0;
						if (lhsType.isMatrix())
						{
							c1 = AllocId();
							EmitInst(fnBodyWords, OpCompositeExtract, { colVecT, c1, lhsId, static_cast<uint32_t>(c) });
						}
						else
						{
							c1 = CoerceVectorOrScalar(lhsId, lhsType, EbtFloat, static_cast<uint32_t>(rows));
						}
						if (rhsType.isMatrix())
						{
							c2 = AllocId();
							EmitInst(fnBodyWords, OpCompositeExtract, { colVecT, c2, rhsId, static_cast<uint32_t>(c) });
						}
						else
						{
							c2 = CoerceVectorOrScalar(rhsId, rhsType, EbtFloat, static_cast<uint32_t>(rows));
						}
						uint32_t cr = AllocId();
						EmitInst(fnBodyWords, OpFMul == fop ? OpFMul : fop, { colVecT, cr, c1, c2 });
						colsOut.push_back(cr);
					}
					uint32_t id = AllocId();
					std::vector<uint32_t> ops = { GetValueType(resType), id };
					ops.insert(ops.end(), colsOut.begin(), colsOut.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, ops);
					return id;
				}

				// Standard vector/scalar arithmetic & bitwise
				uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
				TBasicType bt = resType.getBasicType();
				uint32_t l = CoerceVectorOrScalar(lhsId, lhsType, bt, rows);
				uint32_t r = CoerceVectorOrScalar(rhsId, rhsType, bt, rows);
				uint32_t rtId = GetValueType(resType);
				uint32_t id = AllocId();

				SpvOp spvOp = OpFAdd;
				bool isInt = (bt == EbtInt);
				switch (op)
				{
				case EOpAdd:
				case EOpAddAssign: spvOp = isInt ? OpIAdd : OpFAdd; break;
				case EOpSub:
				case EOpSubAssign: spvOp = isInt ? OpISub : OpFSub; break;
				case EOpMul:
				case EOpMulAssign:
				case EOpVectorTimesScalarAssign: spvOp = isInt ? OpIMul : OpFMul; break;
				case EOpDiv:
				case EOpDivAssign: spvOp = isInt ? OpSDiv : OpFDiv; break;
				case EOpMod:
				case EOpModAssign: spvOp = isInt ? OpSMod : OpFRem; break;
				case EOpRightShift:
				case EOpRightShiftAssign: spvOp = OpShiftRightArithmetic; break;
				case EOpLeftShift:
				case EOpLeftShiftAssign: spvOp = OpShiftLeftLogical; break;
				case EOpAnd:
				case EOpAndAssign: spvOp = OpBitwiseAnd; break;
				case EOpInclusiveOr:
				case EOpInclusiveOrAssign: spvOp = OpBitwiseOr; break;
				case EOpExclusiveOr:
				case EOpExclusiveOrAssign: spvOp = OpBitwiseXor; break;
				default: spvOp = isInt ? OpIAdd : OpFAdd; break;
				}
				EmitInst(fnBodyWords, spvOp, { rtId, id, l, r });
				return id;
			}

			uint32_t EmitComparison(TOperator op, TIntermTyped* leftNode, TIntermTyped* rightNode, const TType& resType)
			{
				uint32_t l = EmitRValue(leftNode);
				uint32_t r = EmitRValue(rightNode);
				const TType& lt = leftNode->getType();
				const TType& rt = rightNode->getType();
				uint32_t rows = static_cast<uint32_t>((std::max)({ 1, lt.getRowsCount(), rt.getRowsCount() }));
				TBasicType operandBt = (lt.getBasicType() == EbtFloat || rt.getBasicType() == EbtFloat) ? EbtFloat
					: ((lt.getBasicType() == EbtInt || rt.getBasicType() == EbtInt) ? EbtInt : EbtBool);

				l = CoerceVectorOrScalar(l, lt, operandBt, rows);
				r = CoerceVectorOrScalar(r, rt, operandBt, rows);

				uint32_t boolVecType = GetVecType(GetBoolType(), rows);
				SpvOp cmpOp = OpFOrdEqual;
				if (operandBt == EbtFloat)
				{
					switch (op)
					{
					case EOpEqual:
					case EOpVectorEqual: cmpOp = OpFOrdEqual; break;
					case EOpNotEqual:
					case EOpVectorNotEqual: cmpOp = OpFUnordNotEqual; break;
					case EOpLessThan: cmpOp = OpFOrdLessThan; break;
					case EOpGreaterThan: cmpOp = OpFOrdGreaterThan; break;
					case EOpLessThanEqual: cmpOp = OpFOrdLessThanEqual; break;
					case EOpGreaterThanEqual: cmpOp = OpFOrdGreaterThanEqual; break;
					default: break;
					}
				}
				else if (operandBt == EbtInt)
				{
					switch (op)
					{
					case EOpEqual:
					case EOpVectorEqual: cmpOp = OpIEqual; break;
					case EOpNotEqual:
					case EOpVectorNotEqual: cmpOp = OpINotEqual; break;
					case EOpLessThan: cmpOp = OpSLessThan; break;
					case EOpGreaterThan: cmpOp = OpSGreaterThan; break;
					case EOpLessThanEqual: cmpOp = OpSLessThanEqual; break;
					case EOpGreaterThanEqual: cmpOp = OpSGreaterThanEqual; break;
					default: break;
					}
				}
				else
				{
					cmpOp = (op == EOpNotEqual || op == EOpVectorNotEqual) ? OpLogicalNotEqual : OpLogicalEqual;
				}

				uint32_t cmpId = AllocId();
				EmitInst(fnBodyWords, cmpOp, { boolVecType, cmpId, l, r });

				// If AST result type is scalar bool (e.g. vec4 == vec4 -> all(equal(...)), vec4 != vec4 -> any(notEqual(...)))
				if (rows > 1 && resType.isScalar() && resType.getBasicType() == EbtBool)
				{
					uint32_t redId = AllocId();
					EmitInst(fnBodyWords, (op == EOpNotEqual) ? OpAny : OpAll, { GetBoolType(), redId, cmpId });
					return redId;
				}
				TType cmpAstType(EbtBool, EbpUndefined, EvqTemporary, 1, static_cast<int>(rows));
				return CoerceToType(cmpId, cmpAstType, resType);
			}

			uint32_t ConstructTypeFromScalar(uint32_t scalarVal, const TType& scalarType, const TType& ft)
			{
				if (ft.isArray())
				{
					int n = (std::max)(1, ft.getArraySize());
					TType elemT = ft;
					elemT.clearArrayness();
					uint32_t elemVal = ConstructTypeFromScalar(scalarVal, scalarType, elemT);
					uint32_t arrT = GetValueType(ft);
					uint32_t id = AllocId();
					std::vector<uint32_t> ops = { arrT, id };
					ops.insert(ops.end(), static_cast<size_t>(n), elemVal);
					EmitInst(fnBodyWords, OpCompositeConstruct, ops);
					return id;
				}
				if (ft.getBasicType() == EbtStruct)
				{
					uint32_t subT = GetValueType(ft);
					const TTypeList* sub = ft.getStruct();
					if (!sub || sub->empty())
						return GetConstNull(subT);
					std::vector<uint32_t> subFields;
					subFields.reserve(sub->size());
					for (size_t k = 0; k < sub->size(); ++k)
						subFields.push_back(ConstructTypeFromScalar(scalarVal, scalarType, *(*sub)[k].type));
					uint32_t id = AllocId();
					std::vector<uint32_t> ops = { subT, id };
					ops.insert(ops.end(), subFields.begin(), subFields.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, ops);
					return id;
				}
				if (ft.isMatrix())
				{
					int cols = ft.getColsCount();
					int rows = ft.getRowsCount();
					uint32_t colVal = CoerceVectorOrScalar(scalarVal, scalarType, EbtFloat, static_cast<uint32_t>(rows));
					uint32_t mt = GetValueType(ft);
					uint32_t id = AllocId();
					std::vector<uint32_t> ops = { mt, id };
					ops.insert(ops.end(), static_cast<size_t>(cols), colVal);
					EmitInst(fnBodyWords, OpCompositeConstruct, ops);
					return id;
				}
				return CoerceVectorOrScalar(scalarVal, scalarType, ft.getBasicType(), static_cast<uint32_t>((std::max)(1, ft.getRowsCount())));
			}

			void FlattenValueToScalars(uint32_t valId, const TType& t, std::vector<std::pair<uint32_t, TBasicType>>& outScalars)
			{
				if (t.isArray())
				{
					int n = (std::max)(1, t.getArraySize());
					TType elemT = t;
					elemT.clearArrayness();
					uint32_t elemTypeId = GetValueType(elemT);
					for (int i = 0; i < n; ++i)
					{
						uint32_t ext = AllocId();
						EmitInst(fnBodyWords, OpCompositeExtract, { elemTypeId, ext, valId, static_cast<uint32_t>(i) });
						FlattenValueToScalars(ext, elemT, outScalars);
					}
					return;
				}
				if (t.getBasicType() == EbtStruct)
				{
					const TTypeList* sl = t.getStruct();
					if (sl)
					{
						for (size_t i = 0; i < sl->size(); ++i)
						{
							const TType& ft = *(*sl)[i].type;
							uint32_t ftId = GetValueType(ft);
							uint32_t ext = AllocId();
							EmitInst(fnBodyWords, OpCompositeExtract, { ftId, ext, valId, static_cast<uint32_t>(i) });
							FlattenValueToScalars(ext, ft, outScalars);
						}
					}
					return;
				}
				if (t.isMatrix())
				{
					int cols = t.getColsCount();
					int rows = t.getRowsCount();
					uint32_t colT = GetVecType(GetFloatType(), static_cast<uint32_t>(rows));
					uint32_t fT = GetFloatType();
					for (int c = 0; c < cols; ++c)
					{
						uint32_t colId = AllocId();
						EmitInst(fnBodyWords, OpCompositeExtract, { colT, colId, valId, static_cast<uint32_t>(c) });
						for (int r = 0; r < rows; ++r)
						{
							uint32_t sc = AllocId();
							EmitInst(fnBodyWords, OpCompositeExtract, { fT, sc, colId, static_cast<uint32_t>(r) });
							outScalars.emplace_back(sc, EbtFloat);
						}
					}
					return;
				}
				int rows = (std::max)(1, t.getRowsCount());
				TBasicType bt = t.getBasicType();
				if (rows == 1)
				{
					outScalars.emplace_back(valId, bt);
					return;
				}
				uint32_t compT = (bt == EbtInt) ? GetIntType() : (bt == EbtBool ? GetBoolType() : GetFloatType());
				for (int r = 0; r < rows; ++r)
				{
					uint32_t sc = AllocId();
					EmitInst(fnBodyWords, OpCompositeExtract, { compT, sc, valId, static_cast<uint32_t>(r) });
					outScalars.emplace_back(sc, bt);
				}
			}

			uint32_t ConsumeScalarsForType(const std::vector<std::pair<uint32_t, TBasicType>>& scalars, size_t& cursor, const TType& ft)
			{
				if (cursor >= scalars.size())
					return GetConstNull(GetValueType(ft));
				if (ft.isArray())
				{
					int n = (std::max)(1, ft.getArraySize());
					TType elemT = ft;
					elemT.clearArrayness();
					std::vector<uint32_t> elems;
					elems.reserve(static_cast<size_t>(n));
					for (int i = 0; i < n; ++i)
						elems.push_back(ConsumeScalarsForType(scalars, cursor, elemT));
					uint32_t arrT = GetValueType(ft);
					uint32_t id = AllocId();
					std::vector<uint32_t> ops = { arrT, id };
					ops.insert(ops.end(), elems.begin(), elems.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, ops);
					return id;
				}
				if (ft.getBasicType() == EbtStruct)
				{
					uint32_t subT = GetValueType(ft);
					const TTypeList* sub = ft.getStruct();
					if (!sub || sub->empty())
						return GetConstNull(subT);
					std::vector<uint32_t> subFields;
					subFields.reserve(sub->size());
					for (size_t k = 0; k < sub->size(); ++k)
						subFields.push_back(ConsumeScalarsForType(scalars, cursor, *(*sub)[k].type));
					uint32_t id = AllocId();
					std::vector<uint32_t> ops = { subT, id };
					ops.insert(ops.end(), subFields.begin(), subFields.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, ops);
					return id;
				}
				if (ft.isMatrix())
				{
					int cols = ft.getColsCount();
					int rows = ft.getRowsCount();
					uint32_t colVecT = GetVecType(GetFloatType(), static_cast<uint32_t>(rows));
					std::vector<uint32_t> colsVec;
					colsVec.reserve(static_cast<size_t>(cols));
					for (int c = 0; c < cols; ++c)
					{
						std::vector<uint32_t> cops = { colVecT, AllocId() };
						for (int r = 0; r < rows; ++r)
						{
							if (cursor < scalars.size())
							{
								auto sp = scalars[cursor++];
								cops.push_back(CastScalar(sp.first, sp.second, EbtFloat));
							}
							else
							{
								cops.push_back(GetConstFloat(0.0f));
							}
						}
						uint32_t cv = cops[1];
						EmitInst(fnBodyWords, OpCompositeConstruct, cops);
						colsVec.push_back(cv);
					}
					uint32_t mt = GetValueType(ft);
					uint32_t mid = AllocId();
					std::vector<uint32_t> mops = { mt, mid };
					mops.insert(mops.end(), colsVec.begin(), colsVec.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, mops);
					return mid;
				}
				int rows = (std::max)(1, ft.getRowsCount());
				TBasicType dstBt = ft.getBasicType();
				std::vector<uint32_t> comps;
				comps.reserve(static_cast<size_t>(rows));
				for (int r = 0; r < rows; ++r)
				{
					if (cursor < scalars.size())
					{
						auto sp = scalars[cursor++];
						comps.push_back(CastScalar(sp.first, sp.second, dstBt));
					}
					else
					{
						if (dstBt == EbtInt) comps.push_back(GetConstInt(0));
						else if (dstBt == EbtBool) comps.push_back(GetConstBool(false));
						else comps.push_back(GetConstFloat(0.0f));
					}
				}
				if (rows == 1)
					return comps[0];
				uint32_t vt = GetValueType(ft);
				uint32_t vid = AllocId();
				std::vector<uint32_t> vops = { vt, vid };
				vops.insert(vops.end(), comps.begin(), comps.end());
				EmitInst(fnBodyWords, OpCompositeConstruct, vops);
				return vid;
			}

			uint32_t EmitConstructor(TIntermAggregate* agg)
			{
				const TType& dstType = agg->getType();
				TNodeArray& args = agg->getNodes();

				if (agg->getOp() == EOpConstructStruct)
				{
					uint32_t stId = GetValueType(dstType);
					const TTypeList* sl = dstType.getStruct();
					if (!sl || sl->empty())
						return GetConstNull(stId);

					if (args.size() == 1)
					{
						TIntermTyped* a0 = args[0]->getAsTyped();
						if (TIntermConstant* c0 = a0 ? a0->getAsConstant() : nullptr)
						{
							if (c0->getCount() <= 1 && c0->toFloat(0) == 0.0f && c0->toInt(0) == 0 && !c0->toBool(0))
								return GetConstNull(stId);
						}
						if (a0 && a0->getType().isScalar() &&
							(sl->size() != 1 || (*sl)[0].type->isArray() || (*sl)[0].type->isMatrix() || (*sl)[0].type->getBasicType() == EbtStruct))
						{
							return ConstructTypeFromScalar(EmitRValue(a0), a0->getType(), dstType);
						}
					}

					std::vector<uint32_t> fieldVals;
					fieldVals.reserve(sl->size());
					if (args.size() == sl->size())
					{
						for (size_t i = 0; i < sl->size(); ++i)
						{
							TIntermTyped* a = args[i]->getAsTyped();
							const TType& ft = *(*sl)[i].type;
							uint32_t v = EmitRValue(a);
							if (a->getType().isScalar() && (ft.isArray() || ft.isMatrix() || ft.getBasicType() == EbtStruct))
								fieldVals.push_back(ConstructTypeFromScalar(v, a->getType(), ft));
							else
								fieldVals.push_back(CoerceToType(v, a->getType(), ft));
						}
					}
					else
					{
						std::vector<std::pair<uint32_t, TBasicType>> flatScalars;
						for (size_t i = 0; i < args.size(); ++i)
						{
							if (TIntermTyped* a = args[i]->getAsTyped())
								FlattenValueToScalars(EmitRValue(a), a->getType(), flatScalars);
						}
						size_t cursor = 0;
						for (size_t i = 0; i < sl->size(); ++i)
							fieldVals.push_back(ConsumeScalarsForType(flatScalars, cursor, *(*sl)[i].type));
					}

					uint32_t id = AllocId();
					std::vector<uint32_t> ops = { stId, id };
					ops.insert(ops.end(), fieldVals.begin(), fieldVals.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, ops);
					return id;
				}

				if (agg->getOp() == EOpConstructArray)
				{
					TType elemT = dstType;
					elemT.clearArrayness();
					std::vector<uint32_t> elems;
					for (size_t i = 0; i < args.size(); ++i)
					{
						TIntermTyped* a = args[i]->getAsTyped();
						uint32_t v = EmitRValue(a);
						elems.push_back(CoerceToType(v, a->getType(), elemT));
					}
					uint32_t arrT = GetValueType(dstType);
					uint32_t id = AllocId();
					std::vector<uint32_t> ops = { arrT, id };
					ops.insert(ops.end(), elems.begin(), elems.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, ops);
					return id;
				}

				if (agg->getOp() == EOpConstructMat2x2FromMat || agg->getOp() == EOpConstructMat3x3FromMat ||
					(dstType.isMatrix() && args.size() == 1 && args[0]->getAsTyped() && args[0]->getAsTyped()->getType().isMatrix()))
				{
					int dstCols = dstType.getColsCount();
					int dstRows = dstType.getRowsCount();
					TIntermTyped* srcNode = args[0]->getAsTyped();
					uint32_t srcMat = EmitRValue(srcNode);
					int srcCols = srcNode->getType().getColsCount();
					int srcRows = srcNode->getType().getRowsCount();
					uint32_t srcColT = GetVecType(GetFloatType(), static_cast<uint32_t>(srcRows));
					uint32_t dstColT = GetVecType(GetFloatType(), static_cast<uint32_t>(dstRows));

					std::vector<uint32_t> colsOut;
					for (int c = 0; c < dstCols; ++c)
					{
						if (c < srcCols && dstRows <= srcRows)
						{
							uint32_t colVal = AllocId();
							EmitInst(fnBodyWords, OpCompositeExtract, { srcColT, colVal, srcMat, static_cast<uint32_t>(c) });
							if (dstRows == srcRows)
							{
								colsOut.push_back(colVal);
							}
							else
							{
								uint32_t shufCol = AllocId();
								std::vector<uint32_t> sops = { dstColT, shufCol, colVal, colVal };
								for (int r = 0; r < dstRows; ++r)
									sops.push_back(static_cast<uint32_t>(r));
								EmitInst(fnBodyWords, OpVectorShuffle, sops);
								colsOut.push_back(shufCol);
							}
						}
						else
						{
							uint32_t colVal = 0;
							if (c < srcCols)
							{
								colVal = AllocId();
								EmitInst(fnBodyWords, OpCompositeExtract, { srcColT, colVal, srcMat, static_cast<uint32_t>(c) });
							}
							std::vector<uint32_t> compIds = { dstColT, 0 };
							uint32_t zeroId = GetConstFloat(0.0f);
							uint32_t oneId = GetConstFloat(1.0f);
							for (int r = 0; r < dstRows; ++r)
							{
								if (c < srcCols && r < srcRows)
								{
									uint32_t sc = AllocId();
									EmitInst(fnBodyWords, OpCompositeExtract, { GetFloatType(), sc, colVal, static_cast<uint32_t>(r) });
									compIds.push_back(sc);
								}
								else
								{
									compIds.push_back((c == r) ? oneId : zeroId);
								}
							}
							uint32_t newCol = AllocId();
							compIds[1] = newCol;
							EmitInst(fnBodyWords, OpCompositeConstruct, compIds);
							colsOut.push_back(newCol);
						}
					}
					uint32_t matT = GetMatType(static_cast<uint32_t>(dstCols), static_cast<uint32_t>(dstRows));
					uint32_t resId = AllocId();
					std::vector<uint32_t> mops = { matT, resId };
					mops.insert(mops.end(), colsOut.begin(), colsOut.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, mops);
					return resId;
				}

				// Flatten all arguments into target-typed scalars
				TBasicType dstBt = dstType.getBasicType();
				std::vector<uint32_t> scalars;
				for (TIntermNode* child : args)
				{
					TIntermTyped* a = child->getAsTyped();
					if (!a) continue;
					uint32_t av = EmitRValue(a);
					const TType& at = a->getType();
					TBasicType abt = at.getBasicType();
					uint32_t acompT = (abt == EbtInt) ? GetIntType() : (abt == EbtBool ? GetBoolType() : GetFloatType());

					if (at.isMatrix())
					{
						int acols = at.getColsCount();
						int arows = at.getRowsCount();
						uint32_t colVecT = GetVecType(acompT, static_cast<uint32_t>(arows));
						for (int c = 0; c < acols; ++c)
						{
							uint32_t cv = AllocId();
							EmitInst(fnBodyWords, OpCompositeExtract, { colVecT, cv, av, static_cast<uint32_t>(c) });
							for (int r = 0; r < arows; ++r)
							{
								uint32_t sv = AllocId();
								EmitInst(fnBodyWords, OpCompositeExtract, { acompT, sv, cv, static_cast<uint32_t>(r) });
								scalars.push_back(CastScalar(sv, abt, dstBt));
							}
						}
					}
					else if (at.isVector())
					{
						int arows = at.getRowsCount();
						for (int r = 0; r < arows; ++r)
						{
							uint32_t sv = AllocId();
							EmitInst(fnBodyWords, OpCompositeExtract, { acompT, sv, av, static_cast<uint32_t>(r) });
							scalars.push_back(CastScalar(sv, abt, dstBt));
						}
					}
					else
					{
						scalars.push_back(CastScalar(av, abt, dstBt));
					}
				}

				int needed = dstType.getInstanceSize();
				const uint32_t zeroConst = dstBt == EbtInt ? GetConstInt(0) : (dstBt == EbtBool ? GetConstBool(false) : GetConstFloat(0.0f));
				if (scalars.empty())
				{
					scalars.push_back(zeroConst);
				}
				const bool singleScalarSplat = (scalars.size() == 1);
				while (static_cast<int>(scalars.size()) < needed)
				{
					// Single-scalar splat or pad with zero
					scalars.push_back(singleScalarSplat ? scalars[0] : zeroConst);
				}

				if (dstType.isMatrix())
				{
					int cols = dstType.getColsCount();
					int rows = dstType.getRowsCount();
					uint32_t colVecT = GetVecType(GetFloatType(), static_cast<uint32_t>(rows));
					std::vector<uint32_t> colsOut;
					for (int c = 0; c < cols; ++c)
					{
						uint32_t cv = AllocId();
						std::vector<uint32_t> cops = { colVecT, cv };
						for (int r = 0; r < rows; ++r)
							cops.push_back(scalars[static_cast<size_t>(c * rows + r)]);
						EmitInst(fnBodyWords, OpCompositeConstruct, cops);
						colsOut.push_back(cv);
					}
					uint32_t matT = GetValueType(dstType);
					uint32_t mid = AllocId();
					std::vector<uint32_t> mops = { matT, mid };
					mops.insert(mops.end(), colsOut.begin(), colsOut.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, mops);
					return mid;
				}

				if (needed <= 1)
					return scalars[0];

				uint32_t vecT = GetValueType(dstType);
				uint32_t vid = AllocId();
				std::vector<uint32_t> vops = { vecT, vid };
				for (int i = 0; i < needed; ++i)
					vops.push_back(scalars[static_cast<size_t>(i)]);
				EmitInst(fnBodyWords, OpCompositeConstruct, vops);
				return vid;
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

			const SamplerResourceInfo& GetOrCreateFallbackSampler(SpvDim dim)
			{
				for (const auto& kv : samplerBySymId)
				{
					if (kv.second.dim == dim)
						return kv.second;
				}
				auto it = fallbackSamplerByDim.find(static_cast<uint32_t>(dim));
				if (it != fallbackSamplerByDim.end())
					return it->second;

				SamplerResourceInfo sri;
				sri.symId = -1;
				sri.name = "_fallback_sampler";
				sri.slot = (std::min)(nextAutoSamplerSlot++, 15u);
				sri.dim = dim;
				sri.imageTypeId = GetImageType(dim);
				sri.sampledImgTypeId = GetSampledImageType(sri.imageTypeId);

				uint32_t imgPtrT = GetPtrType(SC_UniformConstant, sri.imageTypeId);
				sri.imageVarId = AllocId();
				EmitInst(secTypesConstsGlobals, OpVariable, { imgPtrT, sri.imageVarId, SC_UniformConstant });
				EmitInst(secAnnotations, OpDecorate, { sri.imageVarId, Dec_DescriptorSet, 0u });
				EmitInst(secAnnotations, OpDecorate, { sri.imageVarId, Dec_Binding, 16u + sri.slot });

				uint32_t smpPtrT = GetPtrType(SC_UniformConstant, GetSamplerType());
				sri.samplerVarId = AllocId();
				EmitInst(secTypesConstsGlobals, OpVariable, { smpPtrT, sri.samplerVarId, SC_UniformConstant });
				EmitInst(secAnnotations, OpDecorate, { sri.samplerVarId, Dec_DescriptorSet, 0u });
				EmitInst(secAnnotations, OpDecorate, { sri.samplerVarId, Dec_Binding, 32u + sri.slot });

				auto res = fallbackSamplerByDim.emplace(static_cast<uint32_t>(dim), std::move(sri));
				return res.first->second;
			}

			uint32_t EmitTextureSample(TIntermAggregate* agg)
			{
				TNodeArray& args = agg->getNodes();
				uint32_t v4f = GetVecType(GetFloatType(), 4);
				if (args.size() < 2)
					return GetConstNull(v4f);

				TIntermSymbol* sampSym = args[0]->getAsSymbolNode();
				if (!sampSym)
					return GetConstNull(v4f);

				const SamplerResourceInfo* psri = LookupSamplerSym(sampSym->getId());
				if (!psri)
					return GetConstNull(v4f);

				const SamplerResourceInfo& sri = *psri;
				uint32_t imgVal = AllocId();
				EmitInst(fnBodyWords, OpLoad, { sri.imageTypeId, imgVal, sri.imageVarId });
				uint32_t smpVal = AllocId();
				EmitInst(fnBodyWords, OpLoad, { GetSamplerType(), smpVal, sri.samplerVarId });
				uint32_t sampledImg = AllocId();
				EmitInst(fnBodyWords, OpSampledImage, { sri.sampledImgTypeId, sampledImg, imgVal, smpVal });

				TIntermTyped* coordNode = args[1]->getAsTyped();
				uint32_t coordRaw = EmitRValue(coordNode);
				const TType& ct = coordNode->getType();

				TOperator op = agg->getOp();
				uint32_t coordDims = (sri.dim == Dim_3D || sri.dim == Dim_Cube) ? 3u : (sri.dim == Dim_1D ? 1u : 2u);

				// Projective texture lookup: divide coord.xy(z) by coord.w
				if (op == EOpTex2DProj || op == EOpTex3DProj || op == EOpTexCubeProj || op == EOpTex1DProj || op == EOpShadow2DProj)
				{
					uint32_t c4 = CoerceVectorOrScalar(coordRaw, ct, EbtFloat, 4);
					uint32_t wScalar = AllocId();
					EmitInst(fnBodyWords, OpCompositeExtract, { GetFloatType(), wScalar, c4, 3u });
					uint32_t num = CoerceVectorOrScalar(c4, TType(EbtFloat, EbpUndefined, EvqTemporary, 1, 4), EbtFloat, coordDims);
					uint32_t den = SplatScalar(wScalar, GetFloatType(), coordDims);
					uint32_t projCoord = AllocId();
					EmitInst(fnBodyWords, OpFDiv, { GetVecType(GetFloatType(), coordDims), projCoord, num, den });

					uint32_t resId = AllocId();
					if (isVertex)
						EmitInst(fnBodyWords, OpImageSampleExplicitLod, { v4f, resId, sampledImg, projCoord, ImgOp_Lod, GetConstFloat(0.0f) });
					else
						EmitInst(fnBodyWords, OpImageSampleImplicitLod, { v4f, resId, sampledImg, projCoord });
					return CoerceToType(resId, TType(EbtFloat, EbpUndefined, EvqTemporary, 1, 4), agg->getType());
				}

				// Explicit LOD: coord is vec4 where .w is mip lod
				if (op == EOpTex2DLod || op == EOpTex3DLod || op == EOpTexCubeLod || op == EOpTex1DLod)
				{
					uint32_t c4 = CoerceVectorOrScalar(coordRaw, ct, EbtFloat, 4);
					uint32_t lodScalar = AllocId();
					EmitInst(fnBodyWords, OpCompositeExtract, { GetFloatType(), lodScalar, c4, 3u });
					uint32_t uv = CoerceVectorOrScalar(c4, TType(EbtFloat, EbpUndefined, EvqTemporary, 1, 4), EbtFloat, coordDims);
					uint32_t resId = AllocId();
					EmitInst(fnBodyWords, OpImageSampleExplicitLod, { v4f, resId, sampledImg, uv, ImgOp_Lod, lodScalar });
					return CoerceToType(resId, TType(EbtFloat, EbpUndefined, EvqTemporary, 1, 4), agg->getType());
				}

				// Bias: coord is vec4 where .w is bias
				if (op == EOpTex2DBias || op == EOpTex3DBias || op == EOpTexCubeBias || op == EOpTex1DBias)
				{
					uint32_t c4 = CoerceVectorOrScalar(coordRaw, ct, EbtFloat, 4);
					uint32_t biasScalar = AllocId();
					EmitInst(fnBodyWords, OpCompositeExtract, { GetFloatType(), biasScalar, c4, 3u });
					uint32_t uv = CoerceVectorOrScalar(c4, TType(EbtFloat, EbpUndefined, EvqTemporary, 1, 4), EbtFloat, coordDims);
					uint32_t resId = AllocId();
					if (isVertex)
						EmitInst(fnBodyWords, OpImageSampleExplicitLod, { v4f, resId, sampledImg, uv, ImgOp_Lod, GetConstFloat(0.0f) });
					else
						EmitInst(fnBodyWords, OpImageSampleImplicitLod, { v4f, resId, sampledImg, uv, ImgOp_Bias, biasScalar });
					return CoerceToType(resId, TType(EbtFloat, EbpUndefined, EvqTemporary, 1, 4), agg->getType());
				}

				// Grad (or 4-arg tex2D(s, uv, ddx, ddy))
				if (args.size() >= 4 || op == EOpTex2DGrad || op == EOpTex3DGrad || op == EOpTexCubeGrad || op == EOpTex1DGrad)
				{
					uint32_t uv = CoerceVectorOrScalar(coordRaw, ct, EbtFloat, coordDims);
					TIntermTyped* ddxNode = args[2]->getAsTyped();
					TIntermTyped* ddyNode = args[3]->getAsTyped();
					uint32_t ddxVal = CoerceVectorOrScalar(EmitRValue(ddxNode), ddxNode->getType(), EbtFloat, coordDims);
					uint32_t ddyVal = CoerceVectorOrScalar(EmitRValue(ddyNode), ddyNode->getType(), EbtFloat, coordDims);
					uint32_t resId = AllocId();
					EmitInst(fnBodyWords, OpImageSampleExplicitLod, { v4f, resId, sampledImg, uv, ImgOp_Grad, ddxVal, ddyVal });
					return CoerceToType(resId, TType(EbtFloat, EbpUndefined, EvqTemporary, 1, 4), agg->getType());
				}

				// Standard 2-arg sample
				uint32_t uv = CoerceVectorOrScalar(coordRaw, ct, EbtFloat, coordDims);
				uint32_t resId = AllocId();
				if (isVertex)
					EmitInst(fnBodyWords, OpImageSampleExplicitLod, { v4f, resId, sampledImg, uv, ImgOp_Lod, GetConstFloat(0.0f) });
				else
					EmitInst(fnBodyWords, OpImageSampleImplicitLod, { v4f, resId, sampledImg, uv });
				return CoerceToType(resId, TType(EbtFloat, EbpUndefined, EvqTemporary, 1, 4), agg->getType());
			}

			uint32_t EmitUnaryExpr(TIntermUnary* un)
			{
				TOperator op = un->getOp();
				TIntermTyped* operand = un->getOperand();
				const TType& resType = un->getType();
				uint32_t resTypeId = GetValueType(resType);

				// Pre/post increment & decrement
				if (op == EOpPreIncrement || op == EOpPreDecrement || op == EOpPostIncrement || op == EOpPostDecrement)
				{
					LValueRef lv;
					if (EmitLValue(operand, lv))
					{
						uint32_t oldVal = LoadFromLValue(lv);
						uint32_t rows = static_cast<uint32_t>((std::max)(1, lv.resultType.getRowsCount()));
						bool isInt = (lv.resultType.getBasicType() == EbtInt);
						uint32_t step = isInt ? SplatScalar(GetConstInt(1), GetIntType(), rows)
							: SplatScalar(GetConstFloat(1.0f), GetFloatType(), rows);
						uint32_t newVal = AllocId();
						SpvOp addSub = (op == EOpPreIncrement || op == EOpPostIncrement) ? (isInt ? OpIAdd : OpFAdd) : (isInt ? OpISub : OpFSub);
						EmitInst(fnBodyWords, addSub, { GetValueType(lv.resultType), newVal, oldVal, step });
						StoreToLValue(lv, newVal);
						return (op == EOpPreIncrement || op == EOpPreDecrement) ? newVal : oldVal;
					}
				}

				// clip(x): discard fragment if any component < 0.0
				if (op == EOpFclip)
				{
					uint32_t val = EmitRValue(operand);
					uint32_t rows = static_cast<uint32_t>((std::max)(1, operand->getType().getRowsCount()));
					val = CoerceVectorOrScalar(val, operand->getType(), EbtFloat, rows);
					uint32_t zero = SplatScalar(GetConstFloat(0.0f), GetFloatType(), rows);
					uint32_t cmpT = GetVecType(GetBoolType(), rows);
					uint32_t ltId = AllocId();
					EmitInst(fnBodyWords, OpFOrdLessThan, { cmpT, ltId, val, zero });
					uint32_t killCond = ltId;
					if (rows > 1)
					{
						killCond = AllocId();
						EmitInst(fnBodyWords, OpAny, { GetBoolType(), killCond, ltId });
					}
					if (!isVertex)
					{
						uint32_t killLbl = AllocId();
						uint32_t contLbl = AllocId();
						EmitInst(fnBodyWords, OpSelectionMerge, { contLbl, 0u });
						EmitInst(fnBodyWords, OpBranchConditional, { killCond, killLbl, contLbl });
						StartBlock(killLbl);
						EmitInst(fnBodyWords, OpKill, {});
						blockTerminated = true;
						StartBlock(contLbl);
					}
					return 0;
				}

				uint32_t v = EmitRValue(operand);
				const TType& ot = operand->getType();

				switch (op)
				{
				case EOpNegative:
				{
					if (resType.isMatrix())
					{
						int cols = resType.getColsCount();
						int rows = resType.getRowsCount();
						uint32_t colT = GetVecType(GetFloatType(), static_cast<uint32_t>(rows));
						std::vector<uint32_t> negCols;
						for (int c = 0; c < cols; ++c)
						{
							uint32_t col = AllocId(), nc = AllocId();
							EmitInst(fnBodyWords, OpCompositeExtract, { colT, col, v, static_cast<uint32_t>(c) });
							EmitInst(fnBodyWords, OpFNegate, { colT, nc, col });
							negCols.push_back(nc);
						}
						uint32_t id = AllocId();
						std::vector<uint32_t> ops = { resTypeId, id };
						ops.insert(ops.end(), negCols.begin(), negCols.end());
						EmitInst(fnBodyWords, OpCompositeConstruct, ops);
						return id;
					}
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, resType.getBasicType() == EbtInt ? OpSNegate : OpFNegate, { resTypeId, id, v });
					return id;
				}
				case EOpLogicalNot:
				case EOpVectorLogicalNot:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					uint32_t bv = CoerceVectorOrScalar(v, ot, EbtBool, rows);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpLogicalNot, { GetVecType(GetBoolType(), rows), id, bv });
					return id;
				}
				case EOpBitwiseNot:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					uint32_t iv = CoerceVectorOrScalar(v, ot, EbtInt, rows);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpNot, { GetVecType(GetIntType(), rows), id, iv });
					return id;
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
					uint32_t bv = CoerceVectorOrScalar(v, ot, EbtBool, rows);
					if (rows <= 1) return bv;
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpAny, { GetBoolType(), id, bv });
					return id;
				}
				case EOpAll:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, ot.getRowsCount()));
					uint32_t bv = CoerceVectorOrScalar(v, ot, EbtBool, rows);
					if (rows <= 1) return bv;
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpAll, { GetBoolType(), id, bv });
					return id;
				}
				case EOpSaturate:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					uint32_t fv = CoerceVectorOrScalar(v, ot, EbtFloat, rows);
					uint32_t z = SplatScalar(GetConstFloat(0.0f), GetFloatType(), rows);
					uint32_t o = SplatScalar(GetConstFloat(1.0f), GetFloatType(), rows);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpExtInst, { resTypeId, id, glsl450Id, GLSL450_FClamp, fv, z, o });
					return id;
				}
				case EOpTranspose:
				{
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpTranspose, { resTypeId, id, v });
					return id;
				}
				case EOpDeterminant:
				{
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpExtInst, { GetFloatType(), id, glsl450Id, GLSL450_Determinant, v });
					return id;
				}
				case EOpLog10:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					uint32_t fv = CoerceVectorOrScalar(v, ot, EbtFloat, rows);
					uint32_t lg2 = AllocId();
					EmitInst(fnBodyWords, OpExtInst, { resTypeId, lg2, glsl450Id, GLSL450_Log2, fv });
					uint32_t invLog2_10 = SplatScalar(GetConstFloat(0.30102999566f), GetFloatType(), rows);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpFMul, { resTypeId, id, lg2, invLog2_10 });
					return id;
				}
				case EOpD3DCOLORtoUBYTE4:
				{
					uint32_t fv = CoerceVectorOrScalar(v, ot, EbtFloat, 4);
					uint32_t shuf = AllocId();
					EmitInst(fnBodyWords, OpVectorShuffle, { GetVecType(GetFloatType(), 4), shuf, fv, fv, 2u, 1u, 0u, 3u });
					uint32_t scale = SplatScalar(GetConstFloat(255.001953f), GetFloatType(), 4);
					uint32_t scaled = AllocId();
					EmitInst(fnBodyWords, OpFMul, { GetVecType(GetFloatType(), 4), scaled, shuf, scale });
					return CoerceVectorOrScalar(scaled, TType(EbtFloat, EbpUndefined, EvqTemporary, 1, 4), resType.getBasicType(), 4);
				}
				case EOpDPdx:
				case EOpDPdy:
				case EOpFwidth:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					uint32_t fv = CoerceVectorOrScalar(v, ot, EbtFloat, rows);
					if (isVertex)
						return SplatScalar(GetConstFloat(0.0f), GetFloatType(), rows);
					uint32_t id = AllocId();
					SpvOp dop = (op == EOpDPdx) ? OpDPdx : (op == EOpDPdy ? OpDPdy : OpFwidth);
					EmitInst(fnBodyWords, dop, { resTypeId, id, fv });
					return id;
				}
				default:
					break;
				}

				// GLSL.std.450 unary math functions
				uint32_t extOp = GLSL450_FAbs;
				bool isInt = (resType.getBasicType() == EbtInt);
				switch (op)
				{
				case EOpRadians:     extOp = GLSL450_Radians; break;
				case EOpDegrees:     extOp = GLSL450_Degrees; break;
				case EOpSin:         extOp = GLSL450_Sin; break;
				case EOpCos:         extOp = GLSL450_Cos; break;
				case EOpTan:         extOp = GLSL450_Tan; break;
				case EOpAsin:        extOp = GLSL450_Asin; break;
				case EOpAcos:        extOp = GLSL450_Acos; break;
				case EOpAtan:        extOp = GLSL450_Atan; break;
				case EOpExp:         extOp = GLSL450_Exp; break;
				case EOpLog:         extOp = GLSL450_Log; break;
				case EOpExp2:        extOp = GLSL450_Exp2; break;
				case EOpLog2:        extOp = GLSL450_Log2; break;
				case EOpSqrt:        extOp = GLSL450_Sqrt; break;
				case EOpInverseSqrt: extOp = GLSL450_InverseSqrt; break;
				case EOpAbs:         extOp = isInt ? GLSL450_SAbs : GLSL450_FAbs; break;
				case EOpSign:        extOp = isInt ? GLSL450_SSign : GLSL450_FSign; break;
				case EOpFloor:       extOp = GLSL450_Floor; break;
				case EOpCeil:        extOp = GLSL450_Ceil; break;
				case EOpFract:       extOp = GLSL450_Fract; break;
				case EOpRound:       extOp = GLSL450_Round; break;
				case EOpTrunc:       extOp = GLSL450_Trunc; break;
				case EOpLength:
				{
					uint32_t fv = CoerceVectorOrScalar(v, ot, EbtFloat, static_cast<uint32_t>((std::max)(1, ot.getRowsCount())));
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpExtInst, { GetFloatType(), id, glsl450Id, GLSL450_Length, fv });
					return id;
				}
				case EOpNormalize:
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					uint32_t fv = CoerceVectorOrScalar(v, ot, EbtFloat, rows);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpExtInst, { resTypeId, id, glsl450Id, GLSL450_Normalize, fv });
					return id;
				}
				default:
					extOp = GLSL450_FAbs;
					break;
				}

				uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
				uint32_t argVal = CoerceVectorOrScalar(v, ot, isInt ? EbtInt : EbtFloat, rows);
				uint32_t id = AllocId();
				EmitInst(fnBodyWords, OpExtInst, { resTypeId, id, glsl450Id, extOp, argVal });
				return id;
			}

			uint32_t EmitAggregateExpr(TIntermAggregate* agg)
			{
				TOperator op = agg->getOp();
				TNodeArray& args = agg->getNodes();
				const TType& resType = agg->getType();
				uint32_t resTypeId = GetValueType(resType);

				// Constructors
				if ((op >= EOpConstructInt && op <= EOpConstructMat3x3FromMat))
					return EmitConstructor(agg);

				// Texture lookups
				if (op >= EOpTex1D && op <= EOpTex2DArrayBias)
					return EmitTextureSample(agg);

				// Comma operator
				if (op == EOpComma)
				{
					uint32_t last = 0;
					for (TIntermNode* child : args)
					{
						if (child && child->getAsTyped())
							last = EmitRValue(child->getAsTyped());
					}
					return last;
				}

				// User helper function call (supports in, out, inout parameters via copy-in/copy-out!)
				if (op == EOpFunctionCall)
				{
					std::string mangled = agg->getName().c_str();
					auto fit = funcsByMangled.find(mangled);
					if (fit == funcsByMangled.end())
						return resType.getBasicType() == EbtVoid ? 0u : GetConstNull(resTypeId);

					const FuncRecord& callee = fit->second;
					struct CopyBackItem
					{
						LValueRef lv;
						uint32_t tmpVarId = 0;
						TType paramType;
					};
					std::vector<CopyBackItem> copyBacks;
					std::vector<uint32_t> callArgs;

					for (size_t i = 0; i < callee.params.size() && i < args.size(); ++i)
					{
						const ParamSpec& ps = callee.params[i];
						TIntermTyped* argNode = args[i]->getAsTyped();
						if (ps.isSampler)
						{
							TIntermSymbol* argSym = argNode ? argNode->getAsSymbolNode() : nullptr;
							const SamplerResourceInfo* argSri = argSym ? LookupSamplerSym(argSym->getId()) : nullptr;
							if (!argSri || argSri->dim != ps.samplerDim)
								argSri = &GetOrCreateFallbackSampler(ps.samplerDim);
							callArgs.push_back(argSri->imageVarId);
							callArgs.push_back(argSri->samplerVarId);
						}
						else if (ps.passByPtr)
						{
							uint32_t tmpVar = AllocLocalVar(ps.valTypeId);
							if (ps.qualifier == EvqInOut)
							{
								uint32_t inVal = EmitRValue(argNode);
								inVal = CoerceToType(inVal, argNode->getType(), ps.astType);
								EmitInst(fnBodyWords, OpStore, { tmpVar, inVal });
							}
							LValueRef argLv;
							if (EmitLValue(argNode, argLv))
								copyBacks.push_back({ argLv, tmpVar, ps.astType });
							callArgs.push_back(tmpVar);
						}
						else
						{
							uint32_t val = EmitRValue(argNode);
							val = CoerceToType(val, argNode->getType(), ps.astType);
							callArgs.push_back(val);
						}
					}

					uint32_t callResId = AllocId();
					std::vector<uint32_t> callOps = { callee.retTypeId, callResId, callee.funcId };
					callOps.insert(callOps.end(), callArgs.begin(), callArgs.end());
					EmitInst(fnBodyWords, OpFunctionCall, callOps);

					for (const auto& cb : copyBacks)
					{
						uint32_t outVal = AllocId();
						EmitInst(fnBodyWords, OpLoad, { GetValueType(cb.paramType), outVal, cb.tmpVarId });
						outVal = CoerceToType(outVal, cb.paramType, cb.lv.resultType);
						StoreToLValue(cb.lv, outVal);
					}
					return callResId;
				}

				// mul(a, b) aggregate form
				if (op == EOpMul && args.size() == 2)
				{
					TIntermTyped* a0 = args[0]->getAsTyped();
					TIntermTyped* a1 = args[1]->getAsTyped();
					uint32_t v0 = EmitRValue(a0);
					uint32_t v1 = EmitRValue(a1);
					const TType& t0 = a0->getType();
					const TType& t1 = a1->getType();
					// Remember transposed matrix convention: HLSL mul(a, b) == b_col * a_col
					if (t0.isMatrix() && t1.isMatrix())
					{
						uint32_t id = AllocId();
						EmitInst(fnBodyWords, OpMatrixTimesMatrix, { resTypeId, id, v1, v0 });
						return id;
					}
					if (t0.isVector() && t1.isMatrix())
					{
						uint32_t id = AllocId();
						EmitInst(fnBodyWords, OpMatrixTimesVector, { resTypeId, id, v1, v0 });
						return id;
					}
					if (t0.isMatrix() && t1.isVector())
					{
						uint32_t id = AllocId();
						EmitInst(fnBodyWords, OpVectorTimesMatrix, { resTypeId, id, v1, v0 });
						return id;
					}
					return ApplyNumericBinaryOp(EOpMul, v0, t0, v1, t1, resType);
				}

				// modf(x, out i) -> i = trunc(x); return x - trunc(x);
				if (op == EOpModf && args.size() == 2)
				{
					TIntermTyped* xNode = args[0]->getAsTyped();
					TIntermTyped* iNode = args[1]->getAsTyped();
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					uint32_t xVal = CoerceVectorOrScalar(EmitRValue(xNode), xNode->getType(), EbtFloat, rows);
					uint32_t truncId = AllocId();
					EmitInst(fnBodyWords, OpExtInst, { resTypeId, truncId, glsl450Id, GLSL450_Trunc, xVal });
					LValueRef outLv;
					if (EmitLValue(iNode, outLv))
					{
						uint32_t coercedInt = CoerceVectorOrScalar(truncId, resType, outLv.resultType.getBasicType(), static_cast<uint32_t>((std::max)(1, outLv.resultType.getRowsCount())));
						StoreToLValue(outLv, coercedInt);
					}
					uint32_t fracId = AllocId();
					EmitInst(fnBodyWords, OpFSub, { resTypeId, fracId, xVal, truncId });
					return fracId;
				}

				// sincos(x, out s, out c)
				if (op == EOpSinCos && args.size() == 3)
				{
					TIntermTyped* xNode = args[0]->getAsTyped();
					uint32_t rows = static_cast<uint32_t>((std::max)(1, xNode->getType().getRowsCount()));
					uint32_t vecF = GetVecType(GetFloatType(), rows);
					uint32_t xVal = CoerceVectorOrScalar(EmitRValue(xNode), xNode->getType(), EbtFloat, rows);
					uint32_t sId = AllocId(), cId = AllocId();
					EmitInst(fnBodyWords, OpExtInst, { vecF, sId, glsl450Id, GLSL450_Sin, xVal });
					EmitInst(fnBodyWords, OpExtInst, { vecF, cId, glsl450Id, GLSL450_Cos, xVal });
					LValueRef slv, clv;
					TType ft(EbtFloat, EbpUndefined, EvqTemporary, 1, static_cast<int>(rows));
					if (EmitLValue(args[1]->getAsTyped(), slv))
						StoreToLValue(slv, CoerceToType(sId, ft, slv.resultType));
					if (EmitLValue(args[2]->getAsTyped(), clv))
						StoreToLValue(clv, CoerceToType(cId, ft, clv.resultType));
					return 0;
				}

				// lit(n_dot_l, n_dot_h, m)
				if (op == EOpLit && args.size() >= 3)
				{
					TIntermTyped* a0 = args[0]->getAsTyped();
					TIntermTyped* a1 = args[1]->getAsTyped();
					TIntermTyped* a2 = args[2]->getAsTyped();
					uint32_t nl = CoerceVectorOrScalar(EmitRValue(a0), a0->getType(), EbtFloat, 1);
					uint32_t nh = CoerceVectorOrScalar(EmitRValue(a1), a1->getType(), EbtFloat, 1);
					uint32_t m  = CoerceVectorOrScalar(EmitRValue(a2), a2->getType(), EbtFloat, 1);
					uint32_t f0 = GetConstFloat(0.0f), f1 = GetConstFloat(1.0f);
					uint32_t diff = AllocId();
					EmitInst(fnBodyWords, OpExtInst, { GetFloatType(), diff, glsl450Id, GLSL450_FMax, nl, f0 });
					uint32_t nhClamped = AllocId();
					EmitInst(fnBodyWords, OpExtInst, { GetFloatType(), nhClamped, glsl450Id, GLSL450_FMax, nh, f0 });
					uint32_t p = AllocId();
					EmitInst(fnBodyWords, OpExtInst, { GetFloatType(), p, glsl450Id, GLSL450_Pow, nhClamped, m });
					uint32_t posCond = AllocId();
					EmitInst(fnBodyWords, OpFOrdGreaterThan, { GetBoolType(), posCond, nl, f0 });
					uint32_t spec = AllocId();
					EmitInst(fnBodyWords, OpSelect, { GetFloatType(), spec, posCond, p, f0 });
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpCompositeConstruct, { GetVecType(GetFloatType(), 4), id, f1, diff, spec, f1 });
					return id;
				}

				// Vector comparison aggregates
				if (op == EOpLessThan || op == EOpGreaterThan || op == EOpLessThanEqual ||
					op == EOpGreaterThanEqual || op == EOpVectorEqual || op == EOpVectorNotEqual)
				{
					return EmitComparison(op, args[0]->getAsTyped(), args[1]->getAsTyped(), resType);
				}

				// Dot product (note: OpDot in SPIR-V requires Vector operands >= 2 components; for 1-component float use OpFMul!)
				if (op == EOpDot && args.size() == 2)
				{
					TIntermTyped* a0 = args[0]->getAsTyped();
					TIntermTyped* a1 = args[1]->getAsTyped();
					uint32_t rows = static_cast<uint32_t>((std::max)({ 1, a0->getType().getRowsCount(), a1->getType().getRowsCount() }));
					uint32_t v0 = CoerceVectorOrScalar(EmitRValue(a0), a0->getType(), EbtFloat, rows);
					uint32_t v1 = CoerceVectorOrScalar(EmitRValue(a1), a1->getType(), EbtFloat, rows);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, rows == 1 ? OpFMul : OpDot, { GetFloatType(), id, v0, v1 });
					return id;
				}

				// fmod / EOpMod aggregate
				if (op == EOpMod && args.size() == 2)
				{
					TIntermTyped* a0 = args[0]->getAsTyped();
					TIntermTyped* a1 = args[1]->getAsTyped();
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					bool isInt = (resType.getBasicType() == EbtInt);
					uint32_t v0 = CoerceVectorOrScalar(EmitRValue(a0), a0->getType(), isInt ? EbtInt : EbtFloat, rows);
					uint32_t v1 = CoerceVectorOrScalar(EmitRValue(a1), a1->getType(), isInt ? EbtInt : EbtFloat, rows);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, isInt ? OpSMod : OpFRem, { resTypeId, id, v0, v1 });
					return id;
				}

				// Remaining GLSL.std.450 multi-arg builtins (min, max, clamp, mix/lerp, pow, atan2, step, smoothstep, distance, cross, reflect, refract, faceforward, ldexp)
				std::vector<uint32_t> evalArgs;
				uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
				bool isInt = (resType.getBasicType() == EbtInt);

				if (op == EOpDistance && args.size() == 2)
				{
					TIntermTyped* a0 = args[0]->getAsTyped();
					TIntermTyped* a1 = args[1]->getAsTyped();
					uint32_t r = static_cast<uint32_t>((std::max)({ 1, a0->getType().getRowsCount(), a1->getType().getRowsCount() }));
					uint32_t v0 = CoerceVectorOrScalar(EmitRValue(a0), a0->getType(), EbtFloat, r);
					uint32_t v1 = CoerceVectorOrScalar(EmitRValue(a1), a1->getType(), EbtFloat, r);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpExtInst, { GetFloatType(), id, glsl450Id, GLSL450_Distance, v0, v1 });
					return id;
				}
				if (op == EOpRefract && args.size() == 3)
				{
					TIntermTyped* a0 = args[0]->getAsTyped();
					TIntermTyped* a1 = args[1]->getAsTyped();
					TIntermTyped* a2 = args[2]->getAsTyped();
					uint32_t v0 = CoerceVectorOrScalar(EmitRValue(a0), a0->getType(), EbtFloat, rows);
					uint32_t v1 = CoerceVectorOrScalar(EmitRValue(a1), a1->getType(), EbtFloat, rows);
					uint32_t eta = CoerceVectorOrScalar(EmitRValue(a2), a2->getType(), EbtFloat, 1);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpExtInst, { resTypeId, id, glsl450Id, GLSL450_Refract, v0, v1, eta });
					return id;
				}
				if (op == EOpLdexp && args.size() == 2)
				{
					TIntermTyped* a0 = args[0]->getAsTyped();
					TIntermTyped* a1 = args[1]->getAsTyped();
					uint32_t v0 = CoerceVectorOrScalar(EmitRValue(a0), a0->getType(), EbtFloat, rows);
					uint32_t expI = CoerceVectorOrScalar(EmitRValue(a1), a1->getType(), EbtInt, rows);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpExtInst, { resTypeId, id, glsl450Id, GLSL450_Ldexp, v0, expI });
					return id;
				}

				for (TIntermNode* child : args)
				{
					TIntermTyped* a = child->getAsTyped();
					evalArgs.push_back(CoerceVectorOrScalar(EmitRValue(a), a->getType(), isInt ? EbtInt : EbtFloat, rows));
				}

				uint32_t extOp = GLSL450_FMax;
				switch (op)
				{
				case EOpMin:         extOp = isInt ? GLSL450_SMin : GLSL450_FMin; break;
				case EOpMax:         extOp = isInt ? GLSL450_SMax : GLSL450_FMax; break;
				case EOpClamp:       extOp = isInt ? GLSL450_SClamp : GLSL450_FClamp; break;
				case EOpMix:         extOp = GLSL450_FMix; break;
				case EOpPow:         extOp = GLSL450_Pow; break;
				case EOpAtan2:       extOp = GLSL450_Atan2; break;
				case EOpStep:        extOp = GLSL450_Step; break;
				case EOpSmoothStep:  extOp = GLSL450_SmoothStep; break;
				case EOpCross:       extOp = GLSL450_Cross; break;
				case EOpReflect:     extOp = GLSL450_Reflect; break;
				case EOpFaceForward: extOp = GLSL450_FaceForward; break;
				default:             extOp = GLSL450_FMax; break;
				}

				uint32_t id = AllocId();
				std::vector<uint32_t> ops = { resTypeId, id, glsl450Id, extOp };
				ops.insert(ops.end(), evalArgs.begin(), evalArgs.end());
				EmitInst(fnBodyWords, OpExtInst, ops);
				return id;
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

			uint32_t EmitBinaryExpr(TIntermBinary* bin)
			{
				TOperator op = bin->getOp();
				TIntermTyped* leftNode = bin->getLeft();
				TIntermTyped* rightNode = bin->getRight();
				const TType& resType = bin->getType();

				// Simple assignment
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
								uint32_t rhsVal = EmitRValue(rightNode);
								rhsVal = CoerceVectorOrScalar(rhsVal, rightNode->getType(), EbtFloat, static_cast<uint32_t>((std::max)(size_t(1), swiz.size())));
								uint32_t curMat = LoadFromLValue(matLv);
								uint32_t matTypeId = GetValueType(matLv.resultType);
								uint32_t cols = static_cast<uint32_t>((std::max)(1, matLv.resultType.getColsCount()));
								uint32_t rows = static_cast<uint32_t>((std::max)(1, matLv.resultType.getRowsCount()));
								for (size_t i = 0; i < swiz.size(); ++i)
								{
									uint32_t compVal = rhsVal;
									if (swiz.size() > 1)
									{
										compVal = AllocId();
										EmitInst(fnBodyWords, OpCompositeExtract, { GetFloatType(), compVal, rhsVal, static_cast<uint32_t>(i) });
									}
									uint32_t cIdx = (std::min)(swiz[i] % 4u, cols - 1u);
									uint32_t rIdx = (std::min)(swiz[i] / 4u, rows - 1u);
									uint32_t nextMat = AllocId();
									EmitInst(fnBodyWords, OpCompositeInsert, { matTypeId, nextMat, compVal, curMat, cIdx, rIdx });
									curMat = nextMat;
								}
								StoreToLValue(matLv, curMat);
								return rhsVal;
							}
						}
					}

					LValueRef lv;
					uint32_t rhsVal = EmitRValue(rightNode);
					if (EmitLValue(leftNode, lv))
					{
						rhsVal = CoerceToType(rhsVal, rightNode->getType(), lv.resultType);
						StoreToLValue(lv, rhsVal);
					}
					return rhsVal;
				}

				// Compound assignments (+=, -=, *=, /=, %=, &=, |=, ^=, <<=, >>=)
				if (op >= EOpAddAssign && op <= EOpRightShiftAssign)
				{
					LValueRef lv;
					if (EmitLValue(leftNode, lv))
					{
						uint32_t oldVal = LoadFromLValue(lv);
						uint32_t rhsVal = EmitRValue(rightNode);
						uint32_t newVal = 0;
						if (op == EOpVectorTimesMatrixAssign)
						{
							newVal = AllocId();
							EmitInst(fnBodyWords, OpMatrixTimesVector, { GetValueType(lv.resultType), newVal, rhsVal, oldVal });
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

				// Vector swizzle (supports scalar swizzle e.g. x.xxxx too)
				if (op == EOpVectorSwizzle)
				{
					uint32_t lhsVal = EmitRValue(leftNode);
					const TType& lt = leftNode->getType();
					std::vector<uint32_t> swiz;
					ExtractSwizzleIndices(rightNode, swiz);
					uint32_t compT = (lt.getBasicType() == EbtInt) ? GetIntType() : (lt.getBasicType() == EbtBool ? GetBoolType() : GetFloatType());
					if (lt.isScalar())
					{
						return (swiz.size() <= 1) ? lhsVal : SplatScalar(lhsVal, compT, static_cast<uint32_t>(swiz.size()));
					}
					uint32_t maxIdx = static_cast<uint32_t>((std::max)(1, lt.getRowsCount()) - 1);
					for (uint32_t& idx : swiz)
						idx = (std::min)(idx, maxIdx);
					if (swiz.size() <= 1)
					{
						uint32_t extId = AllocId();
						EmitInst(fnBodyWords, OpCompositeExtract, { compT, extId, lhsVal, swiz.empty() ? 0u : swiz[0] });
						return extId;
					}
					uint32_t vecT = GetVecType(compT, static_cast<uint32_t>(swiz.size()));
					uint32_t shufId = AllocId();
					std::vector<uint32_t> ops = { vecT, shufId, lhsVal, lhsVal };
					ops.insert(ops.end(), swiz.begin(), swiz.end());
					EmitInst(fnBodyWords, OpVectorShuffle, ops);
					return shufId;
				}

				// Matrix swizzle (e.g. m._m00_m01 / m._11_12)
				if (op == EOpMatrixSwizzle)
				{
					uint32_t matVal = EmitRValue(leftNode);
					const TType& lt = leftNode->getType();
					uint32_t cols = static_cast<uint32_t>((std::max)(1, lt.getColsCount()));
					uint32_t rows = static_cast<uint32_t>((std::max)(1, lt.getRowsCount()));
					std::vector<uint32_t> swiz;
					ExtractSwizzleIndices(rightNode, swiz);
					std::vector<uint32_t> comps;
					for (uint32_t s : swiz)
					{
						uint32_t cIdx = (std::min)(s % 4u, cols - 1u);
						uint32_t rIdx = (std::min)(s / 4u, rows - 1u);
						uint32_t cid = AllocId();
						EmitInst(fnBodyWords, OpCompositeExtract, { GetFloatType(), cid, matVal, cIdx, rIdx });
						comps.push_back(cid);
					}
					if (comps.size() <= 1)
						return comps.empty() ? GetConstFloat(0.0f) : comps[0];
					uint32_t vecT = GetVecType(GetFloatType(), static_cast<uint32_t>(comps.size()));
					uint32_t vid = AllocId();
					std::vector<uint32_t> ops = { vecT, vid };
					ops.insert(ops.end(), comps.begin(), comps.end());
					EmitInst(fnBodyWords, OpCompositeConstruct, ops);
					return vid;
				}

				// Struct field, array element, matrix row, or vector component indexing
				if (op == EOpIndexDirectStruct || op == EOpIndexDirect || op == EOpIndexIndirect)
				{
					uint32_t uniVal = 0;
					if (TryEmitUniformLoad(bin, uniVal))
						return uniVal;

					LValueRef lv;
					if (EmitLValue(bin, lv))
						return LoadFromLValue(lv);

					uint32_t lhsVal = EmitRValue(leftNode);
					if (op == EOpIndexDirectStruct || rightNode->getAsConstant() != nullptr)
					{
						uint32_t idx = static_cast<uint32_t>((std::max)(0, ExtractImmediateInt(rightNode)));
						uint32_t extId = AllocId();
						EmitInst(fnBodyWords, OpCompositeExtract, { GetValueType(resType), extId, lhsVal, idx });
						return extId;
					}

					// Dynamic index on pure R-value: spill to function local variable and OpAccessChain
					uint32_t tmpPtr = AllocLocalVar(GetValueType(leftNode->getType()));
					EmitInst(fnBodyWords, OpStore, { tmpPtr, lhsVal });
					uint32_t dynIdx = CoerceVectorOrScalar(EmitRValue(rightNode), rightNode->getType(), EbtInt, 1);
					uint32_t elemT = GetValueType(resType);
					uint32_t acId = AllocId();
					EmitInst(fnBodyWords, OpAccessChain, { GetPtrType(SC_Function, elemT), acId, tmpPtr, dynIdx });
					uint32_t loaded = AllocId();
					EmitInst(fnBodyWords, OpLoad, { elemT, loaded, acId });
					return loaded;
				}

				// Short-circuit / vector logical AND / OR / XOR
				if (op == EOpLogicalAnd || op == EOpLogicalOr || op == EOpLogicalXor)
				{
					if (resType.isVector() || op == EOpLogicalXor)
					{
						uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
						uint32_t l = CoerceVectorOrScalar(EmitRValue(leftNode), leftNode->getType(), EbtBool, rows);
						uint32_t r = CoerceVectorOrScalar(EmitRValue(rightNode), rightNode->getType(), EbtBool, rows);
						uint32_t id = AllocId();
						SpvOp lop = (op == EOpLogicalAnd) ? OpLogicalAnd : (op == EOpLogicalOr ? OpLogicalOr : OpLogicalNotEqual);
						EmitInst(fnBodyWords, lop, { GetVecType(GetBoolType(), rows), id, l, r });
						return id;
					}

					uint32_t resPtr = AllocLocalVar(GetBoolType());
					uint32_t lhsCond = CoerceVectorOrScalar(EmitRValue(leftNode), leftNode->getType(), EbtBool, 1);
					EmitInst(fnBodyWords, OpStore, { resPtr, lhsCond });
					uint32_t rhsLbl = AllocId();
					uint32_t mergeLbl = AllocId();
					EmitInst(fnBodyWords, OpSelectionMerge, { mergeLbl, 0u });
					if (op == EOpLogicalAnd)
						EmitInst(fnBodyWords, OpBranchConditional, { lhsCond, rhsLbl, mergeLbl });
					else
						EmitInst(fnBodyWords, OpBranchConditional, { lhsCond, mergeLbl, rhsLbl });
					StartBlock(rhsLbl);
					uint32_t rhsCond = CoerceVectorOrScalar(EmitRValue(rightNode), rightNode->getType(), EbtBool, 1);
					EmitInst(fnBodyWords, OpStore, { resPtr, rhsCond });
					EmitBranchIfLive(mergeLbl);
					StartBlock(mergeLbl);
					uint32_t outId = AllocId();
					EmitInst(fnBodyWords, OpLoad, { GetBoolType(), outId, resPtr });
					return outId;
				}

				// Comparisons
				if (op == EOpEqual || op == EOpNotEqual || op == EOpVectorEqual || op == EOpVectorNotEqual ||
					op == EOpLessThan || op == EOpGreaterThan || op == EOpLessThanEqual || op == EOpGreaterThanEqual)
				{
					return EmitComparison(op, leftNode, rightNode, resType);
				}

				uint32_t l = EmitRValue(leftNode);
				uint32_t r = EmitRValue(rightNode);
				return ApplyNumericBinaryOp(op, l, leftNode->getType(), r, rightNode->getType(), resType);
			}

			uint32_t EmitSelection(TIntermSelection* sel)
			{
				const TType& resType = sel->getType();
				TIntermTyped* condTyped = sel->getCondition() ? sel->getCondition()->getAsTyped() : nullptr;

				// Statement-form if / else
				if (resType.getBasicType() == EbtVoid)
				{
					uint32_t condVal = condTyped
						? CoerceVectorOrScalar(EmitRValue(condTyped), condTyped->getType(), EbtBool, 1)
						: GetConstBool(false);
					uint32_t thenLbl = AllocId();
					uint32_t elseLbl = sel->getFalseBlock() ? AllocId() : 0u;
					uint32_t mergeLbl = AllocId();
					EmitInst(fnBodyWords, OpSelectionMerge, { mergeLbl, 0u });
					EmitInst(fnBodyWords, OpBranchConditional, { condVal, thenLbl, elseLbl ? elseLbl : mergeLbl });

					StartBlock(thenLbl);
					if (sel->getTrueBlock())
						EmitStatement(sel->getTrueBlock());
					EmitBranchIfLive(mergeLbl);

					if (elseLbl)
					{
						StartBlock(elseLbl);
						EmitStatement(sel->getFalseBlock());
						EmitBranchIfLive(mergeLbl);
					}

					StartBlock(mergeLbl);
					return 0;
				}

				// Vector component-wise ternary selection (bvecN ? vecN : vecN)
				if (condTyped && condTyped->isVector())
				{
					uint32_t rows = static_cast<uint32_t>((std::max)(1, resType.getRowsCount()));
					uint32_t cVal = CoerceVectorOrScalar(EmitRValue(condTyped), condTyped->getType(), EbtBool, rows);
					TIntermTyped* tb = sel->getTrueBlock()->getAsTyped();
					TIntermTyped* fb = sel->getFalseBlock()->getAsTyped();
					uint32_t tVal = CoerceToType(EmitRValue(tb), tb->getType(), resType);
					uint32_t fVal = CoerceToType(EmitRValue(fb), fb->getType(), resType);
					uint32_t id = AllocId();
					EmitInst(fnBodyWords, OpSelect, { GetValueType(resType), id, cVal, tVal, fVal });
					return id;
				}

				// Scalar condition ternary (cond ? exprTrue : exprFalse)
				uint32_t dstTypeId = GetValueType(resType);
				uint32_t resPtr = AllocLocalVar(dstTypeId);
				uint32_t condVal = condTyped
					? CoerceVectorOrScalar(EmitRValue(condTyped), condTyped->getType(), EbtBool, 1)
					: GetConstBool(false);
				uint32_t thenLbl = AllocId();
				uint32_t elseLbl = AllocId();
				uint32_t mergeLbl = AllocId();
				EmitInst(fnBodyWords, OpSelectionMerge, { mergeLbl, 0u });
				EmitInst(fnBodyWords, OpBranchConditional, { condVal, thenLbl, elseLbl });

				StartBlock(thenLbl);
				TIntermTyped* tb = sel->getTrueBlock()->getAsTyped();
				uint32_t tVal = CoerceToType(EmitRValue(tb), tb->getType(), resType);
				EmitInst(fnBodyWords, OpStore, { resPtr, tVal });
				EmitBranchIfLive(mergeLbl);

				StartBlock(elseLbl);
				TIntermTyped* fb = sel->getFalseBlock()->getAsTyped();
				uint32_t fVal = CoerceToType(EmitRValue(fb), fb->getType(), resType);
				EmitInst(fnBodyWords, OpStore, { resPtr, fVal });
				EmitBranchIfLive(mergeLbl);

				StartBlock(mergeLbl);
				uint32_t outId = AllocId();
				EmitInst(fnBodyWords, OpLoad, { dstTypeId, outId, resPtr });
				return outId;
			}

			uint32_t EmitRValue(TIntermTyped* node)
			{
				if (!node)
					return 0;

				uint32_t uniVal = 0;
				if (TryEmitUniformLoad(node, uniVal))
					return uniVal;

				if (TIntermConstant* c = node->getAsConstant())
					return EmitConstantNode(c);

				if (TIntermSymbol* s = node->getAsSymbolNode())
				{
					LValueRef lv;
					if (EmitLValue(s, lv))
						return LoadFromLValue(lv);
					return GetConstNull(GetValueType(s->getType()));
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

				return GetConstNull(GetValueType(node->getType()));
			}

			TType currentFnRetAstType;

			void EmitDeclarationStmt(TIntermDeclaration* decl)
			{
				TIntermTyped* inner = decl ? decl->getDeclaration() : nullptr;
				if (!inner || decl->getType().getBasicType() == EbtTexture || IsSampler(decl->getType().getBasicType()))
					return;

				if (TIntermSymbol* sym = inner->getAsSymbolNode())
				{
					uint32_t valT = GetValueType(sym->getType());
					uint32_t ptrId = AllocLocalVar(valT);
					fnLocals[sym->getId()] = { ptrId, valT };
					return;
				}

				if (TIntermBinary* assign = inner->getAsBinaryNode())
				{
					TIntermSymbol* sym = assign->getLeft() ? assign->getLeft()->getAsSymbolNode() : nullptr;
					if (!sym)
						return;
					const TType& symType = sym->getType();
					uint32_t valT = GetValueType(symType);
					uint32_t ptrId = AllocLocalVar(valT);
					fnLocals[sym->getId()] = { ptrId, valT };

					TIntermTyped* rhs = assign->getRight();
					if (!rhs)
						return;

					if (decl->containsArrayInitialization() && rhs->getAsAggregate() && rhs->getAsAggregate()->getOp() != EOpConstructArray)
					{
						TType elemT = symType;
						elemT.clearArrayness();
						uint32_t elemTId = GetValueType(elemT);
						int arrLen = (std::max)(1, symType.getArraySize());
						TNodeArray& initNodes = rhs->getAsAggregate()->getNodes();
						std::vector<uint32_t> elems;
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
								elems.push_back(GetConstNull(elemTId));
							}
						}
						uint32_t arrVal = AllocId();
						std::vector<uint32_t> ops = { valT, arrVal };
						ops.insert(ops.end(), elems.begin(), elems.end());
						EmitInst(fnBodyWords, OpCompositeConstruct, ops);
						EmitInst(fnBodyWords, OpStore, { ptrId, arrVal });
					}
					else
					{
						uint32_t initVal = CoerceToType(EmitRValue(rhs), rhs->getType(), symType);
						EmitInst(fnBodyWords, OpStore, { ptrId, initVal });
					}
				}
			}

			void EmitLoopStmt(TIntermLoop* loop)
			{
				TLoopType lt = loop->getType();
				uint32_t headerLbl = AllocId();
				uint32_t bodyLbl = AllocId();
				uint32_t contLbl = AllocId();
				uint32_t mergeLbl = AllocId();

				EmitBranchIfLive(headerLbl);
				StartBlock(headerLbl);

				if (lt == ELoopDoWhile)
				{
					EmitInst(fnBodyWords, OpLoopMerge, { mergeLbl, contLbl, 0u });
					EmitInst(fnBodyWords, OpBranch, { bodyLbl });
					blockTerminated = true;

					loopStack.push_back({ mergeLbl, contLbl });
					StartBlock(bodyLbl);
					if (loop->getBody())
						EmitStatement(loop->getBody());
					EmitBranchIfLive(contLbl);
					loopStack.pop_back();

					StartBlock(contLbl);
					if (loop->getCondition())
					{
						uint32_t condVal = CoerceVectorOrScalar(EmitRValue(loop->getCondition()), loop->getCondition()->getType(), EbtBool, 1);
						EmitInst(fnBodyWords, OpBranchConditional, { condVal, headerLbl, mergeLbl });
						blockTerminated = true;
					}
					else
					{
						EmitBranchIfLive(headerLbl);
					}
					StartBlock(mergeLbl);
				}
				else
				{
					uint32_t condLbl = AllocId();
					EmitInst(fnBodyWords, OpLoopMerge, { mergeLbl, contLbl, 0u });
					EmitInst(fnBodyWords, OpBranch, { condLbl });
					blockTerminated = true;

					StartBlock(condLbl);
					if (loop->getCondition())
					{
						uint32_t condVal = CoerceVectorOrScalar(EmitRValue(loop->getCondition()), loop->getCondition()->getType(), EbtBool, 1);
						EmitInst(fnBodyWords, OpBranchConditional, { condVal, bodyLbl, mergeLbl });
						blockTerminated = true;
					}
					else
					{
						EmitBranchIfLive(bodyLbl);
					}

					loopStack.push_back({ mergeLbl, contLbl });
					StartBlock(bodyLbl);
					if (loop->getBody())
						EmitStatement(loop->getBody());
					EmitBranchIfLive(contLbl);
					loopStack.pop_back();

					StartBlock(contLbl);
					if (loop->getExpression())
						EmitRValue(loop->getExpression());
					EmitBranchIfLive(headerLbl);

					StartBlock(mergeLbl);
				}
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
						EmitInst(fnBodyWords, OpKill, {});
						blockTerminated = true;
					}
					break;
				case EOpBreak:
					if (!loopStack.empty())
					{
						EmitInst(fnBodyWords, OpBranch, { loopStack.back().first });
						blockTerminated = true;
					}
					break;
				case EOpContinue:
					if (!loopStack.empty())
					{
						EmitInst(fnBodyWords, OpBranch, { loopStack.back().second });
						blockTerminated = true;
					}
					break;
				case EOpReturn:
					if (br->getExpression() && currentFnRetTypeId != GetVoidType())
					{
						uint32_t rv = EmitRValue(br->getExpression());
						rv = CoerceToType(rv, br->getExpression()->getType(), currentFnRetAstType);
						EmitInst(fnBodyWords, OpReturnValue, { rv });
					}
					else
					{
						EmitInst(fnBodyWords, OpReturn, {});
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

			void EmitUserFunction(const FuncRecord& fn, const std::vector<TIntermNode*>& bodyNodes)
			{
				fnLocalVarWords.clear();
				fnBodyWords.clear();
				fnLocals.clear();
				fnSamplerParams.clear();
				loopStack.clear();
				blockTerminated = false;
				currentFnRetTypeId = fn.retTypeId;
				currentFnRetAstType = fn.returnAstType;

				std::vector<uint32_t> paramTypeIds;
				std::vector<uint32_t> primaryParamIds(fn.params.size(), 0);
				std::vector<uint32_t> secondaryParamIds(fn.params.size(), 0);
				for (size_t i = 0; i < fn.params.size(); ++i)
				{
					const ParamSpec& ps = fn.params[i];
					paramTypeIds.push_back(ps.paramTypeId);
					primaryParamIds[i] = AllocId();
					if (ps.isSampler)
					{
						paramTypeIds.push_back(ps.smpParamTypeId);
						secondaryParamIds[i] = AllocId();
					}
				}
				uint32_t fnTypeId = GetFunctionType(fn.retTypeId, paramTypeIds);

				EmitInst(secFunctions, OpFunction, { fn.retTypeId, fn.funcId, 0u, fnTypeId });
				for (size_t i = 0; i < fn.params.size(); ++i)
				{
					const ParamSpec& ps = fn.params[i];
					EmitInst(secFunctions, OpFunctionParameter, { ps.paramTypeId, primaryParamIds[i] });
					if (ps.isSampler)
						EmitInst(secFunctions, OpFunctionParameter, { ps.smpParamTypeId, secondaryParamIds[i] });
				}

				uint32_t entryLabel = AllocId();
				// Create addressable Function variables for value parameters so HLSL code can mutate `in` params freely
				std::vector<std::pair<uint32_t, uint32_t>> valueParamInitStores;
				for (size_t i = 0; i < fn.params.size(); ++i)
				{
					const ParamSpec& ps = fn.params[i];
					if (ps.isSampler)
					{
						SamplerResourceInfo sri;
						sri.symId = ps.symId;
						sri.name = ps.name;
						sri.slot = 0;
						sri.dim = ps.samplerDim;
						sri.imageVarId = primaryParamIds[i];
						sri.samplerVarId = secondaryParamIds[i];
						sri.imageTypeId = ps.valTypeId;
						sri.sampledImgTypeId = GetSampledImageType(ps.valTypeId);
						fnSamplerParams[ps.symId] = std::move(sri);
					}
					else if (ps.passByPtr)
					{
						fnLocals[ps.symId] = { primaryParamIds[i], ps.valTypeId };
					}
					else
					{
						uint32_t localPtr = AllocLocalVar(ps.valTypeId);
						fnLocals[ps.symId] = { localPtr, ps.valTypeId };
						valueParamInitStores.push_back({ localPtr, primaryParamIds[i] });
					}
				}

				for (const auto& st : valueParamInitStores)
					EmitInst(fnBodyWords, OpStore, { st.first, st.second });

				for (TIntermNode* bn : bodyNodes)
				{
					if (blockTerminated)
						break;
					EmitStatement(bn);
				}

				if (!blockTerminated)
				{
					if (fn.retTypeId == GetVoidType())
						EmitInst(fnBodyWords, OpReturn, {});
					else
						EmitInst(fnBodyWords, OpReturnValue, { GetConstNull(fn.retTypeId) });
					blockTerminated = true;
				}

				EmitInst(secFunctions, OpLabel, { entryLabel });
				secFunctions.insert(secFunctions.end(), fnLocalVarWords.begin(), fnLocalVarWords.end());
				secFunctions.insert(secFunctions.end(), fnBodyWords.begin(), fnBodyWords.end());
				EmitInst(secFunctions, OpFunctionEnd, {});
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
							for (TIntermNode* pn : cagg->getNodes())
							{
								TIntermSymbol* psym = pn ? pn->getAsSymbolNode() : nullptr;
								if (!psym) continue;
								ParamSpec ps;
								ps.symId = psym->getId();
								ps.name = psym->getSymbol().c_str();
								ps.semantic = psym->getInfo() ? psym->getInfo()->getSemantic().c_str() : "";
								ps.astType = psym->getType();
								ps.qualifier = psym->getQualifier();
								TBasicType pbt = ps.astType.getBasicType();
								if (IsSampler(pbt))
								{
									ps.isSampler = true;
									ps.passByPtr = false;
									if (pbt == EbtSamplerCube) ps.samplerDim = Dim_Cube;
									else if (pbt == EbtSampler3D) ps.samplerDim = Dim_3D;
									else if (pbt == EbtSampler1D || pbt == EbtSampler1DShadow) ps.samplerDim = Dim_1D;
									else ps.samplerDim = Dim_2D;
								}
								else
								{
									ps.passByPtr = (ps.qualifier == EvqOut || ps.qualifier == EvqInOut);
								}
								rec.params.push_back(std::move(ps));
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
					// Case-insensitive fallback
					std::string epUpper = ToUpperStr(entryPointName);
					for (const auto& kv : funcsByMangled)
					{
						if (ToUpperStr(kv.second.plainName) == epUpper)
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

				// 2. Reachability traversal from entry point (collects helper functions, uniforms, samplers, private globals)
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

					ReachScanner(std::unordered_set<int>& l) : locals(l)
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

					orderedCalledFuncs.push_back(mangled); // post-order: callees emitted before callers
				};

				visitFuncDfs(mainTargetMangled);

				// 3. Emit Capabilities, GLSL.std.450 import, MemoryModel
				EmitInst(secCapabilities, OpCapability, { 1u }); // Capability Shader = 1
				glsl450Id = AllocId();
				EmitInstWithStr(secExtInstImports, OpExtInstImport, { glsl450Id }, "GLSL.std.450");
				EmitInst(secMemoryModel, OpMemoryModel, { 0u, 1u }); // Logical (0), GLSL450 (1)

				// Pre-allocate core scalar types so IDs stay neat
				GetVoidType();
				GetBoolType();
				GetIntType();
				GetFloatType();

				// 4. Build $Globals Uniform Block (Set 0, Binding 0 for VS / Binding 1 for PS)
				bool needSampled1D = false;
				if (!foundUniformSyms.empty())
				{
					// Order uniforms matching declaredParams order first, then remaining AST order
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

					std::vector<uint32_t> memberTypeIds;
					uint32_t curOffset = 0;
					for (size_t i = 0; i < foundUniformSyms.size(); ++i)
					{
						TIntermSymbol* usym = foundUniformSyms[i];
						uint32_t mAlign = 4, mSize = 4;
						uint32_t mTypeId = GetUniformMemberType(usym->getType(), mAlign, mSize);
						curOffset = AlignUp(curOffset, mAlign);

						UniformFieldInfo ufi;
						ufi.symId = usym->getId();
						ufi.name = usym->getSymbol().c_str();
						ufi.semantic = usym->getInfo() ? usym->getInfo()->getSemantic().c_str() : "";
						ufi.astType = usym->getType();
						ufi.memberIndex = static_cast<uint32_t>(i);
						ufi.byteOffset = curOffset;
						ufi.byteSize = mSize;
						ufi.isMutableCopy = uniformSymMutable[ufi.symId];
						if (ufi.isMutableCopy)
						{
							uint32_t valT = GetValueType(ufi.astType);
							uint32_t privPtrT = GetPtrType(SC_Private, valT);
							ufi.mutablePrivateVarId = AllocId();
							EmitInst(secTypesConstsGlobals, OpVariable, { privPtrT, ufi.mutablePrivateVarId, SC_Private });
						}

						symIdToUniformIdx[ufi.symId] = uniformFields.size();
						uniformFields.push_back(ufi);
						memberTypeIds.push_back(mTypeId);
						curOffset += mSize;

						// Record discovered parameter metadata for CEffectFileRHI
						auto existing = std::find_if(ioDiscoveredParams.begin(), ioDiscoveredParams.end(),
							[&](const FxUniformParam& p) { return p.name == ufi.name; });
						if (existing == ioDiscoveredParams.end())
						{
							FxUniformParam p;
							p.name = ufi.name;
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
					}

					globalsByteSize = AlignUp((std::max)(curOffset, 16u), 16u);
					globalsStructTypeId = AllocId();
					std::vector<uint32_t> sops = { globalsStructTypeId };
					sops.insert(sops.end(), memberTypeIds.begin(), memberTypeIds.end());
					EmitInst(secTypesConstsGlobals, OpTypeStruct, sops);
					EmitInstWithStr(secDebugNames, OpName, { globalsStructTypeId }, "$Globals");
					EmitInst(secAnnotations, OpDecorate, { globalsStructTypeId, Dec_Block });

					for (const UniformFieldInfo& ufi : uniformFields)
					{
						EmitInstWithStr(secDebugNames, OpMemberName, { globalsStructTypeId, ufi.memberIndex }, ufi.name);
						EmitInst(secAnnotations, OpMemberDecorate, { globalsStructTypeId, ufi.memberIndex, Dec_Offset, ufi.byteOffset });
						if (ufi.astType.isMatrix())
						{
							EmitInst(secAnnotations, OpMemberDecorate, { globalsStructTypeId, ufi.memberIndex, Dec_ColMajor });
							EmitInst(secAnnotations, OpMemberDecorate, { globalsStructTypeId, ufi.memberIndex, Dec_MatrixStride, 16u });
						}

						FxConstantMember cm;
						cm.name = ufi.name;
						cm.byteOffset = ufi.byteOffset;
						cm.byteSize = ufi.byteSize;
						cm.rows = static_cast<uint16_t>((std::max)(1, ufi.astType.getRowsCount()));
						cm.cols = static_cast<uint16_t>((std::max)(1, ufi.astType.getColsCount()));
						cm.elements = static_cast<uint16_t>(ufi.astType.isArray() ? (std::max)(1, ufi.astType.getArraySize()) : 1);
						cm.baseType = (ufi.astType.getBasicType() == EbtInt) ? 1 : (ufi.astType.getBasicType() == EbtBool ? 2 : 0);
						outStage.cbufferMembers.push_back(std::move(cm));
					}

					uint32_t uboPtrT = GetPtrType(SC_Uniform, globalsStructTypeId);
					globalsBlockVarId = AllocId();
					EmitInst(secTypesConstsGlobals, OpVariable, { uboPtrT, globalsBlockVarId, SC_Uniform });
					EmitInst(secAnnotations, OpDecorate, { globalsBlockVarId, Dec_DescriptorSet, 0u });
					EmitInst(secAnnotations, OpDecorate, { globalsBlockVarId, Dec_Binding, isVertex ? 0u : 1u });
					outStage.cbufferSize = globalsByteSize;
				}

				// 5. Build separate Texture (Set 0, Binding 16 + slot) & Sampler (Set 0, Binding 32 + slot) resources
				std::unordered_set<uint32_t> usedSamplerSlots;
				for (TIntermSymbol* ssym : foundSamplerSyms)
				{
					SamplerResourceInfo sri;
					sri.symId = ssym->getId();
					sri.name = ssym->getSymbol().c_str();
					TBasicType sbt = ssym->getBasicType();
					if (sbt == EbtSamplerCube) sri.dim = Dim_Cube;
					else if (sbt == EbtSampler3D) sri.dim = Dim_3D;
					else if (sbt == EbtSampler1D || sbt == EbtSampler1DShadow) { sri.dim = Dim_1D; needSampled1D = true; }
					else sri.dim = Dim_2D;

					int assignedSlot = -1;
					for (size_t i = 0; i < declaredSamplers.size(); ++i)
					{
						if (declaredSamplers[i].samplerName == sri.name)
						{
							assignedSlot = (declaredSamplers[i].registerIndex >= 0) ? declaredSamplers[i].registerIndex : static_cast<int>(i);
							std::string stUpper = ToUpperStr(declaredSamplers[i].samplerType);
							if (stUpper.find("CUBE") != std::string::npos) sri.dim = Dim_Cube;
							else if (stUpper.find("3D") != std::string::npos) sri.dim = Dim_3D;
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

					sri.imageTypeId = GetImageType(sri.dim);
					sri.sampledImgTypeId = GetSampledImageType(sri.imageTypeId);

					uint32_t imgPtrT = GetPtrType(SC_UniformConstant, sri.imageTypeId);
					sri.imageVarId = AllocId();
					EmitInst(secTypesConstsGlobals, OpVariable, { imgPtrT, sri.imageVarId, SC_UniformConstant });
					EmitInst(secAnnotations, OpDecorate, { sri.imageVarId, Dec_DescriptorSet, 0u });
					EmitInst(secAnnotations, OpDecorate, { sri.imageVarId, Dec_Binding, 16u + sri.slot });

					uint32_t smpPtrT = GetPtrType(SC_UniformConstant, GetSamplerType());
					sri.samplerVarId = AllocId();
					EmitInst(secTypesConstsGlobals, OpVariable, { smpPtrT, sri.samplerVarId, SC_UniformConstant });
					EmitInst(secAnnotations, OpDecorate, { sri.samplerVarId, Dec_DescriptorSet, 0u });
					EmitInst(secAnnotations, OpDecorate, { sri.samplerVarId, Dec_Binding, 32u + sri.slot });

					samplerBySymId[sri.symId] = sri;
					outStage.textures.push_back({ sri.name, sri.slot, 1u });
					outStage.samplers.push_back({ sri.name, sri.slot, 1u });
				}
				if (needSampled1D)
					EmitInst(secCapabilities, OpCapability, { 3u }); // Sampled1D = 3

				// 6. Allocate Private global variables (static / non-folded const globals)
				std::sort(foundPrivateSyms.begin(), foundPrivateSyms.end(), [](TIntermSymbol* a, TIntermSymbol* b) {
					return a->getId() < b->getId();
				});
				for (TIntermSymbol* psym : foundPrivateSyms)
				{
					GlobalVarInfo gvi;
					gvi.symId = psym->getId();
					gvi.valTypeId = GetValueType(psym->getType());
					uint32_t ptrT = GetPtrType(SC_Private, gvi.valTypeId);
					gvi.varId = AllocId();
					EmitInst(secTypesConstsGlobals, OpVariable, { ptrT, gvi.varId, SC_Private });
					auto tdit = topDecls.find(gvi.symId);
					if (tdit != topDecls.end())
						gvi.initExpr = tdit->second.initRhs;
					privateGlobals[gvi.symId] = gvi;
					privateGlobalInitOrder.push_back(gvi.symId);
				}

				// 7. Assign SPIR-V IDs & parameter types for all reachable functions, then emit them
				for (const std::string& mname : orderedCalledFuncs)
				{
					FuncRecord& fn = funcsByMangled[mname];
					fn.funcId = AllocId();
					fn.retTypeId = GetValueType(fn.returnAstType);
					for (ParamSpec& ps : fn.params)
					{
						if (ps.isSampler)
						{
							if (ps.samplerDim == Dim_1D && !needSampled1D)
							{
								needSampled1D = true;
								EmitInst(secCapabilities, OpCapability, { 3u });
							}
							ps.valTypeId = GetImageType(ps.samplerDim);
							ps.paramTypeId = GetPtrType(SC_UniformConstant, ps.valTypeId);
							ps.smpParamTypeId = GetPtrType(SC_UniformConstant, GetSamplerType());
						}
						else
						{
							ps.valTypeId = GetValueType(ps.astType);
							ps.paramTypeId = ps.passByPtr ? GetPtrType(SC_Function, ps.valTypeId) : ps.valTypeId;
						}
					}
					EmitInstWithStr(secDebugNames, OpName, { fn.funcId }, fn.plainName);
				}

				for (const std::string& mname : orderedCalledFuncs)
				{
					const FuncRecord& fn = funcsByMangled[mname];
					EmitUserFunction(fn, funcBodies[mname]);
				}

				// 8. EmitSPIR-V Entry Point wrapper `%entry_main` ("main")
				const FuncRecord& entryTarget = funcsByMangled[mainTargetMangled];
				uint32_t entryMainId = AllocId();
				uint32_t voidFnType = GetFunctionType(GetVoidType(), {});
				EmitInstWithStr(secDebugNames, OpName, { entryMainId }, "main");

				fnLocalVarWords.clear();
				fnBodyWords.clear();
				fnLocals.clear();
				fnSamplerParams.clear();
				loopStack.clear();
				blockTerminated = false;
				currentFnRetTypeId = GetVoidType();
				currentFnRetAstType = TType(EbtVoid, EbpUndefined);

				std::vector<uint32_t> interfaceVars;
				std::map<uint32_t, uint32_t> stageInputLocVars;
				std::map<uint32_t, uint32_t> stageOutputLocVars;
				uint32_t builtinVertexIdxVar = 0;
				uint32_t builtinInstanceIdxVar = 0;
				uint32_t builtinFragCoordVar = 0;
				uint32_t builtinFrontFaceVar = 0;
				uint32_t builtinPosOutVar = 0;
				uint32_t builtinPointSizeOutVar = 0;
				uint32_t builtinFragDepthOutVar = 0;

				const uint32_t v4f = GetVecType(GetFloatType(), 4);

				auto getStageInputLocVar = [&](uint32_t loc) -> uint32_t {
					auto it = stageInputLocVars.find(loc);
					if (it != stageInputLocVars.end()) return it->second;
					uint32_t v = AllocId();
					EmitInst(secTypesConstsGlobals, OpVariable, { GetPtrType(SC_Input, v4f), v, SC_Input });
					EmitInst(secAnnotations, OpDecorate, { v, Dec_Location, loc });
					stageInputLocVars[loc] = v;
					interfaceVars.push_back(v);
					return v;
				};

				auto getStageOutputLocVar = [&](uint32_t loc) -> uint32_t {
					auto it = stageOutputLocVars.find(loc);
					if (it != stageOutputLocVars.end()) return it->second;
					uint32_t v = AllocId();
					EmitInst(secTypesConstsGlobals, OpVariable, { GetPtrType(SC_Output, v4f), v, SC_Output });
					EmitInst(secAnnotations, OpDecorate, { v, Dec_Location, loc });
					stageOutputLocVars[loc] = v;
					interfaceVars.push_back(v);
					return v;
				};

				// Initialize mutable uniform copies from $Globals UBO
				for (const UniformFieldInfo& ufi : uniformFields)
				{
					if (!ufi.isMutableCopy) continue;
					uint32_t uAlign = 4, uSize = 4;
					uint32_t uMemT = GetUniformMemberType(ufi.astType, uAlign, uSize);
					uint32_t acId = AllocId();
					EmitInst(fnBodyWords, OpAccessChain, { GetPtrType(SC_Uniform, uMemT), acId, globalsBlockVarId, GetConstInt(static_cast<int32_t>(ufi.memberIndex)) });
					uint32_t rawLoad = AllocId();
					EmitInst(fnBodyWords, OpLoad, { uMemT, rawLoad, acId });
					uint32_t convVal = ConvertUniformLoadedToValue(rawLoad, ufi.astType);
					EmitInst(fnBodyWords, OpStore, { ufi.mutablePrivateVarId, convVal });
				}

				// Initialize Private static/const globals
				for (int sid : privateGlobalInitOrder)
				{
					const GlobalVarInfo& gvi = privateGlobals[sid];
					auto tdit = topDecls.find(sid);
					if (tdit == topDecls.end() || !tdit->second.initRhs)
						continue;
					const TType& symType = tdit->second.sym->getType();
					TIntermTyped* rhs = tdit->second.initRhs;
					if (tdit->second.containsArrayInit && rhs->getAsAggregate() && rhs->getAsAggregate()->getOp() != EOpConstructArray)
					{
						TType elemT = symType;
						elemT.clearArrayness();
						uint32_t elemTId = GetValueType(elemT);
						int arrLen = (std::max)(1, symType.getArraySize());
						TNodeArray& initNodes = rhs->getAsAggregate()->getNodes();
						std::vector<uint32_t> elems;
						for (int i = 0; i < arrLen; ++i)
						{
							if (static_cast<size_t>(i) < initNodes.size() && initNodes[static_cast<size_t>(i)]->getAsTyped())
							{
								TIntermTyped* inNode = initNodes[static_cast<size_t>(i)]->getAsTyped();
								elems.push_back(CoerceToType(EmitRValue(inNode), inNode->getType(), elemT));
							}
							else
							{
								elems.push_back(GetConstNull(elemTId));
							}
						}
						uint32_t arrVal = AllocId();
						std::vector<uint32_t> ops = { gvi.valTypeId, arrVal };
						ops.insert(ops.end(), elems.begin(), elems.end());
						EmitInst(fnBodyWords, OpCompositeConstruct, ops);
						EmitInst(fnBodyWords, OpStore, { gvi.varId, arrVal });
					}
					else
					{
						uint32_t initVal = CoerceToType(EmitRValue(rhs), rhs->getType(), symType);
						EmitInst(fnBodyWords, OpStore, { gvi.varId, initVal });
					}
				}

				uint32_t autoFallbackSemIdx = 0;
				std::function<uint32_t(const TType&, const std::string&)> readStageInputValue = [&](const TType& t, const std::string& rawSem) -> uint32_t {
					if (t.getBasicType() == EbtStruct)
					{
						const TTypeList* sl = t.getStruct();
						std::vector<uint32_t> fvals;
						if (sl)
						{
							for (size_t i = 0; i < sl->size(); ++i)
							{
								const TType& ft = *(*sl)[i].type;
								std::string fsem = ft.hasSemantic() ? ft.getSemantic().c_str() : "";
								fvals.push_back(readStageInputValue(ft, fsem));
							}
						}
						uint32_t stId = GetValueType(t);
						uint32_t resId = AllocId();
						std::vector<uint32_t> ops = { stId, resId };
						ops.insert(ops.end(), fvals.begin(), fvals.end());
						EmitInst(fnBodyWords, OpCompositeConstruct, ops);
						return resId;
					}

					std::string baseUpper;
					uint32_t semIdx = 0;
					ParseSemanticAndIndex(rawSem, baseUpper, semIdx);
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
							if (!builtinVertexIdxVar)
							{
								builtinVertexIdxVar = AllocId();
								EmitInst(secTypesConstsGlobals, OpVariable, { GetPtrType(SC_Input, GetIntType()), builtinVertexIdxVar, SC_Input });
								EmitInst(secAnnotations, OpDecorate, { builtinVertexIdxVar, Dec_BuiltIn, BI_VertexIndex });
								interfaceVars.push_back(builtinVertexIdxVar);
							}
							uint32_t iv = AllocId();
							EmitInst(fnBodyWords, OpLoad, { GetIntType(), iv, builtinVertexIdxVar });
							return CoerceVectorOrScalar(iv, TType(EbtInt, EbpUndefined, EvqTemporary, 1, 1), dstBt, dstRows);
						}
						if (baseUpper == "SV_INSTANCEID" || baseUpper == "INSTANCEID")
						{
							if (!builtinInstanceIdxVar)
							{
								builtinInstanceIdxVar = AllocId();
								EmitInst(secTypesConstsGlobals, OpVariable, { GetPtrType(SC_Input, GetIntType()), builtinInstanceIdxVar, SC_Input });
								EmitInst(secAnnotations, OpDecorate, { builtinInstanceIdxVar, Dec_BuiltIn, BI_InstanceIndex });
								interfaceVars.push_back(builtinInstanceIdxVar);
							}
							uint32_t iv = AllocId();
							EmitInst(fnBodyWords, OpLoad, { GetIntType(), iv, builtinInstanceIdxVar });
							return CoerceVectorOrScalar(iv, TType(EbtInt, EbpUndefined, EvqTemporary, 1, 1), dstBt, dstRows);
						}

						int32_t loc = MapSemanticToVSLocation(baseUpper.c_str(), semIdx);
						uint32_t varId = getStageInputLocVar(loc >= 0 ? static_cast<uint32_t>(loc) : 0u);
						uint32_t rawV4 = AllocId();
						EmitInst(fnBodyWords, OpLoad, { v4f, rawV4, varId });
						return CoerceVectorOrScalar(rawV4, TType(EbtFloat, EbpUndefined, EvqTemporary, 1, 4), dstBt, dstRows);
					}
					else
					{
						if (baseUpper == "VPOS" || baseUpper == "SV_POSITION" || baseUpper == "POSITION")
						{
							if (!builtinFragCoordVar)
							{
								builtinFragCoordVar = AllocId();
								EmitInst(secTypesConstsGlobals, OpVariable, { GetPtrType(SC_Input, v4f), builtinFragCoordVar, SC_Input });
								EmitInst(secAnnotations, OpDecorate, { builtinFragCoordVar, Dec_BuiltIn, BI_FragCoord });
								interfaceVars.push_back(builtinFragCoordVar);
							}
							uint32_t fc = AllocId();
							EmitInst(fnBodyWords, OpLoad, { v4f, fc, builtinFragCoordVar });
							return CoerceVectorOrScalar(fc, TType(EbtFloat, EbpUndefined, EvqTemporary, 1, 4), dstBt, dstRows);
						}
						if (baseUpper == "VFACE" || baseUpper == "SV_ISFRONTFACE")
						{
							if (!builtinFrontFaceVar)
							{
								builtinFrontFaceVar = AllocId();
								EmitInst(secTypesConstsGlobals, OpVariable, { GetPtrType(SC_Input, GetBoolType()), builtinFrontFaceVar, SC_Input });
								EmitInst(secAnnotations, OpDecorate, { builtinFrontFaceVar, Dec_BuiltIn, BI_FrontFacing });
								interfaceVars.push_back(builtinFrontFaceVar);
							}
							uint32_t ff = AllocId();
							EmitInst(fnBodyWords, OpLoad, { GetBoolType(), ff, builtinFrontFaceVar });
							uint32_t signedFace = AllocId();
							EmitInst(fnBodyWords, OpSelect, { GetFloatType(), signedFace, ff, GetConstFloat(1.0f), GetConstFloat(-1.0f) });
							return CoerceVectorOrScalar(signedFace, TType(EbtFloat, EbpUndefined, EvqTemporary, 1, 1), dstBt, dstRows);
						}

						uint32_t loc = static_cast<uint32_t>(MapVaryingLocation(baseUpper, semIdx));
						uint32_t varId = getStageInputLocVar(loc);
						uint32_t rawV4 = AllocId();
						EmitInst(fnBodyWords, OpLoad, { v4f, rawV4, varId });
						return CoerceVectorOrScalar(rawV4, TType(EbtFloat, EbpUndefined, EvqTemporary, 1, 4), dstBt, dstRows);
					}
				};

				std::function<void(uint32_t, const TType&, const std::string&)> writeStageOutputValue = [&](uint32_t valId, const TType& t, const std::string& rawSem) {
					if (t.getBasicType() == EbtVoid)
						return;
					if (t.getBasicType() == EbtStruct)
					{
						const TTypeList* sl = t.getStruct();
						if (sl)
						{
							for (size_t i = 0; i < sl->size(); ++i)
							{
								const TType& ft = *(*sl)[i].type;
								uint32_t fId = AllocId();
								EmitInst(fnBodyWords, OpCompositeExtract, { GetValueType(ft), fId, valId, static_cast<uint32_t>(i) });
								std::string fsem = ft.hasSemantic() ? ft.getSemantic().c_str() : "";
								writeStageOutputValue(fId, ft, fsem);
							}
						}
						return;
					}

					std::string baseUpper;
					uint32_t semIdx = 0;
					ParseSemanticAndIndex(rawSem, baseUpper, semIdx);

					if (isVertex)
					{
						if (baseUpper == "POSITION" || baseUpper == "SV_POSITION" || baseUpper.empty())
						{
							if (!builtinPosOutVar)
							{
								builtinPosOutVar = AllocId();
								EmitInst(secTypesConstsGlobals, OpVariable, { GetPtrType(SC_Output, v4f), builtinPosOutVar, SC_Output });
								EmitInst(secAnnotations, OpDecorate, { builtinPosOutVar, Dec_BuiltIn, BI_Position });
								interfaceVars.push_back(builtinPosOutVar);
							}
							uint32_t p4 = CoerceVectorOrScalar(valId, t, EbtFloat, 4);
							// Flip clip-space Y (pos.y = -pos.y) so Vulkan matches Direct3D NDC & viewport orientation
							uint32_t py = AllocId();
							EmitInst(fnBodyWords, OpCompositeExtract, { GetFloatType(), py, p4, 1u });
							uint32_t negPy = AllocId();
							EmitInst(fnBodyWords, OpFNegate, { GetFloatType(), negPy, py });
							uint32_t flippedPos = AllocId();
							EmitInst(fnBodyWords, OpCompositeInsert, { v4f, flippedPos, negPy, p4, 1u });
							EmitInst(fnBodyWords, OpStore, { builtinPosOutVar, flippedPos });
							return;
						}
						if (baseUpper == "PSIZE")
						{
							if (!builtinPointSizeOutVar)
							{
								builtinPointSizeOutVar = AllocId();
								EmitInst(secTypesConstsGlobals, OpVariable, { GetPtrType(SC_Output, GetFloatType()), builtinPointSizeOutVar, SC_Output });
								EmitInst(secAnnotations, OpDecorate, { builtinPointSizeOutVar, Dec_BuiltIn, BI_PointSize });
								interfaceVars.push_back(builtinPointSizeOutVar);
							}
							uint32_t ps = CoerceVectorOrScalar(valId, t, EbtFloat, 1);
							EmitInst(fnBodyWords, OpStore, { builtinPointSizeOutVar, ps });
							return;
						}

						uint32_t loc = static_cast<uint32_t>(MapVaryingLocation(baseUpper, semIdx));
						uint32_t outVar = getStageOutputLocVar(loc);
						uint32_t v4 = CoerceVectorOrScalar(valId, t, EbtFloat, 4);
						EmitInst(fnBodyWords, OpStore, { outVar, v4 });
					}
					else
					{
						if (baseUpper == "DEPTH" || baseUpper == "SV_DEPTH")
						{
							if (!builtinFragDepthOutVar)
							{
								builtinFragDepthOutVar = AllocId();
								EmitInst(secTypesConstsGlobals, OpVariable, { GetPtrType(SC_Output, GetFloatType()), builtinFragDepthOutVar, SC_Output });
								EmitInst(secAnnotations, OpDecorate, { builtinFragDepthOutVar, Dec_BuiltIn, BI_FragDepth });
								interfaceVars.push_back(builtinFragDepthOutVar);
							}
							uint32_t d = CoerceVectorOrScalar(valId, t, EbtFloat, 1);
							EmitInst(fnBodyWords, OpStore, { builtinFragDepthOutVar, d });
							return;
						}

						uint32_t loc = (std::min)(semIdx, 3u);
						uint32_t outVar = getStageOutputLocVar(loc);
						uint32_t v4 = CoerceVectorOrScalar(valId, t, EbtFloat, 4);
						EmitInst(fnBodyWords, OpStore, { outVar, v4 });
					}
				};

				// Build arguments for target HLSL entry function call
				struct EntryOutParam
				{
					uint32_t tmpPtr = 0;
					TType astType;
					std::string semantic;
				};
				std::vector<EntryOutParam> entryOutParams;
				std::vector<uint32_t> entryCallArgs;

				for (const ParamSpec& ps : entryTarget.params)
				{
					if (ps.passByPtr)
					{
						uint32_t tmpPtr = AllocLocalVar(ps.valTypeId);
						if (ps.qualifier == EvqInOut)
						{
							uint32_t inVal = readStageInputValue(ps.astType, ps.semantic);
							EmitInst(fnBodyWords, OpStore, { tmpPtr, inVal });
						}
						entryCallArgs.push_back(tmpPtr);
						entryOutParams.push_back({ tmpPtr, ps.astType, ps.semantic });
					}
					else
					{
						entryCallArgs.push_back(readStageInputValue(ps.astType, ps.semantic));
					}
				}

				uint32_t entryRetId = AllocId();
				std::vector<uint32_t> callOps = { entryTarget.retTypeId, entryRetId, entryTarget.funcId };
				callOps.insert(callOps.end(), entryCallArgs.begin(), entryCallArgs.end());
				EmitInst(fnBodyWords, OpFunctionCall, callOps);

				if (entryTarget.returnAstType.getBasicType() != EbtVoid)
					writeStageOutputValue(entryRetId, entryTarget.returnAstType, entryTarget.returnSemantic);

				for (const EntryOutParam& eop : entryOutParams)
				{
					uint32_t outVal = AllocId();
					EmitInst(fnBodyWords, OpLoad, { GetValueType(eop.astType), outVal, eop.tmpPtr });
					writeStageOutputValue(outVal, eop.astType, eop.semantic);
				}

				EmitInst(fnBodyWords, OpReturn, {});

				uint32_t mainEntryLabel = AllocId();
				EmitInst(secFunctions, OpFunction, { GetVoidType(), entryMainId, 0u, voidFnType });
				EmitInst(secFunctions, OpLabel, { mainEntryLabel });
				secFunctions.insert(secFunctions.end(), fnLocalVarWords.begin(), fnLocalVarWords.end());
				secFunctions.insert(secFunctions.end(), fnBodyWords.begin(), fnBodyWords.end());
				EmitInst(secFunctions, OpFunctionEnd, {});

				// Emit OpEntryPoint and OpExecutionMode
				uint32_t execModel = isVertex ? 0u : 4u; // 0 = Vertex, 4 = Fragment
				EmitInstWithStr(secEntryPoints, OpEntryPoint, { execModel, entryMainId }, "main", interfaceVars);
				if (!isVertex)
				{
					EmitInst(secExecutionModes, OpExecutionMode, { entryMainId, 7u }); // OriginUpperLeft = 7
					if (builtinFragDepthOutVar)
						EmitInst(secExecutionModes, OpExecutionMode, { entryMainId, 12u }); // DepthReplacing = 12
				}

				// 9. Assemble final SPIR-V 1.0 binary word stream
				std::vector<uint32_t> spvWords;
				spvWords.reserve(5 + secCapabilities.size() + secExtInstImports.size() + secMemoryModel.size() +
					secEntryPoints.size() + secExecutionModes.size() + secDebugNames.size() + secAnnotations.size() +
					secTypesConstsGlobals.size() + secFunctions.size());

				AppendWord(spvWords, SPV_MAGIC);
				AppendWord(spvWords, SPV_VERSION_1_0);
				AppendWord(spvWords, 0u); // Generator magic
				AppendWord(spvWords, nextId);
				AppendWord(spvWords, 0u); // Instruction schema

				auto appendSec = [&](const std::vector<uint32_t>& s) {
					spvWords.insert(spvWords.end(), s.begin(), s.end());
				};
				appendSec(secCapabilities);
				appendSec(secExtInstImports);
				appendSec(secMemoryModel);
				appendSec(secEntryPoints);
				appendSec(secExecutionModes);
				appendSec(secDebugNames);
				appendSec(secAnnotations);
				appendSec(secTypesConstsGlobals);
				appendSec(secFunctions);

				outStage.valid = true;
				outStage.target = EShaderTarget::SPIRV_1_0;
				outStage.entryPoint = "main";
				outStage.profile = isVertex ? "spirv_vs" : "spirv_ps";
				outStage.bytecode.resize(spvWords.size() * sizeof(uint32_t));
				std::memcpy(outStage.bytecode.data(), spvWords.data(), outStage.bytecode.size());
				return true;
			}
		};

		struct SpirvAstVisitorPayload
		{
			SpirvModuleBuilder* builder = nullptr;
			FxCompiledStage* outStage = nullptr;
			std::vector<FxUniformParam>* ioParams = nullptr;
		};

		bool C_DECL SpirvAstVisitorCallback(TIntermNode* rootNode, EShLanguage /*language*/, void* userData)
		{
			auto* payload = static_cast<SpirvAstVisitorPayload*>(userData);
			return payload->builder->BuildModule(rootNode, *payload->outStage, *payload->ioParams);
		}
	} // anonymous namespace

	bool CSpirvCodeGen::CompileStageSPIRV(
		const std::string& hlslSource,
		const std::string& entryPoint,
		bool isVertex,
		const std::vector<FxSamplerDesc>& declaredSamplers,
		const std::vector<FxUniformParam>& declaredParams,
		FxCompiledStage& outStage,
		std::vector<FxUniformParam>& ioDiscoveredParams,
		std::string& outError)
	{
		std::lock_guard<std::mutex> lock(g_spirvAstMutex);

		Hlsl2Glsl_Initialize();
		ShHandle parser = Hlsl2Glsl_ConstructCompiler(isVertex ? EShLangVertex : EShLangFragment);
		if (!parser)
		{
			Hlsl2Glsl_Shutdown();
			outError = "Hlsl2Glsl_ConstructCompiler failed for SPIR-V";
			return false;
		}

		SpirvModuleBuilder builder(isVertex, entryPoint, declaredSamplers, declaredParams);
		SpirvAstVisitorPayload payload{ &builder, &outStage, &ioDiscoveredParams };

		// Note: opt = 0 (do NOT swizzle BGRA vertex color because Vulkan B8G8R8A8_UNORM handles BGRA in hardware)
		int ok = Hlsl2Glsl_ParseAST(parser, hlslSource.c_str(), ETargetGLSL_140, nullptr, 0, &SpirvAstVisitorCallback, &payload);
		if (!ok)
		{
			if (!builder.errorMsg.empty())
			{
				outError = builder.errorMsg;
			}
			else
			{
				const char* log = Hlsl2Glsl_GetInfoLog(parser);
				outError = log ? log : "Hlsl2Glsl_ParseAST failed for SPIR-V";
			}
			Hlsl2Glsl_DestructCompiler(parser);
			Hlsl2Glsl_Shutdown();
			return false;
		}

		Hlsl2Glsl_DestructCompiler(parser);
		Hlsl2Glsl_Shutdown();
		return true;
	}

	bool CSpirvCodeGen::ValidateSpirvBinary(const std::vector<uint8_t>& spirvBytes, std::string& outError)
	{
		if (spirvBytes.size() < 20 || (spirvBytes.size() % 4) != 0)
		{
			outError = "SPIR-V binary size invalid (<20 bytes or not 4-byte aligned)";
			return false;
		}

		size_t wordCount = spirvBytes.size() / 4;
		std::vector<uint32_t> words(wordCount);
		std::memcpy(words.data(), spirvBytes.data(), spirvBytes.size());

		if (words[0] != SPV_MAGIC)
		{
			outError = "SPIR-V header magic mismatch";
			return false;
		}
		if (words[1] != SPV_VERSION_1_0)
		{
			outError = "SPIR-V version is not 1.0";
			return false;
		}
		uint32_t idBound = words[3];
		if (idBound < 2 || idBound > 0x3FFFFFu)
		{
			outError = "SPIR-V ID bound invalid";
			return false;
		}

		bool hasMemoryModel = false;
		bool hasEntryPoint = false;
		bool insideFunction = false;
		bool insideBlock = false;
		bool blockEndedWithTerminator = false;

		auto isTerminator = [](uint16_t op) -> bool {
			return op == OpBranch || op == OpBranchConditional || op == OpKill || op == OpReturn || op == OpReturnValue;
		};

		auto isKnownOp = [](uint16_t op) -> bool {
			switch (static_cast<SpvOp>(op))
			{
			case OpNop: case OpName: case OpMemberName: case OpExtInstImport: case OpExtInst:
			case OpMemoryModel: case OpEntryPoint: case OpExecutionMode: case OpCapability:
			case OpTypeVoid: case OpTypeBool: case OpTypeInt: case OpTypeFloat: case OpTypeVector:
			case OpTypeMatrix: case OpTypeImage: case OpTypeSampler: case OpTypeSampledImage:
			case OpTypeArray: case OpTypeStruct: case OpTypePointer: case OpTypeFunction:
			case OpConstantTrue: case OpConstantFalse: case OpConstant: case OpConstantComposite: case OpConstantNull:
			case OpFunction: case OpFunctionParameter: case OpFunctionEnd: case OpFunctionCall:
			case OpVariable: case OpLoad: case OpStore: case OpAccessChain:
			case OpDecorate: case OpMemberDecorate:
			case OpVectorShuffle: case OpCompositeConstruct: case OpCompositeExtract: case OpCompositeInsert:
			case OpTranspose: case OpSampledImage: case OpImageSampleImplicitLod: case OpImageSampleExplicitLod:
			case OpImageSampleProjImplicitLod: case OpImageSampleProjExplicitLod:
			case OpConvertFToS: case OpConvertSToF:
			case OpSNegate: case OpFNegate: case OpIAdd: case OpFAdd: case OpISub: case OpFSub:
			case OpIMul: case OpFMul: case OpSDiv: case OpFDiv: case OpSMod: case OpFRem: case OpFMod:
			case OpVectorTimesScalar: case OpMatrixTimesScalar: case OpVectorTimesMatrix:
			case OpMatrixTimesVector: case OpMatrixTimesMatrix: case OpDot: case OpAny: case OpAll:
			case OpLogicalEqual: case OpLogicalNotEqual: case OpLogicalOr: case OpLogicalAnd: case OpLogicalNot:
			case OpSelect: case OpIEqual: case OpINotEqual:
			case OpSGreaterThan: case OpSGreaterThanEqual: case OpSLessThan: case OpSLessThanEqual:
			case OpFOrdEqual: case OpFUnordNotEqual: case OpFOrdLessThan: case OpFOrdGreaterThan:
			case OpFOrdLessThanEqual: case OpFOrdGreaterThanEqual:
			case OpShiftRightArithmetic: case OpShiftLeftLogical:
			case OpBitwiseOr: case OpBitwiseXor: case OpBitwiseAnd: case OpNot:
			case OpDPdx: case OpDPdy: case OpFwidth:
			case OpLoopMerge: case OpSelectionMerge: case OpLabel:
			case OpBranch: case OpBranchConditional: case OpKill: case OpReturn: case OpReturnValue:
				return true;
			default:
				return false;
			}
		};

		size_t pc = 5;
		while (pc < wordCount)
		{
			uint32_t instHeader = words[pc];
			uint16_t op = static_cast<uint16_t>(instHeader & 0xFFFFu);
			uint32_t wc = instHeader >> 16;
			if (wc == 0 || pc + wc > wordCount)
			{
				outError = "SPIR-V instruction word count out of bounds";
				return false;
			}
			if (!isKnownOp(op))
			{
				outError = "Unknown or unsupported SPIR-V opcode: " + std::to_string(op);
				return false;
			}

			if (op == OpMemoryModel) hasMemoryModel = true;
			else if (op == OpEntryPoint) hasEntryPoint = true;
			else if (op == OpFunction)
			{
				if (insideFunction) { outError = "Nested OpFunction in SPIR-V"; return false; }
				insideFunction = true;
				insideBlock = false;
			}
			else if (op == OpLabel)
			{
				if (!insideFunction) { outError = "OpLabel outside OpFunction"; return false; }
				if (insideBlock && !blockEndedWithTerminator)
				{
					outError = "Basic block missing terminator before next OpLabel";
					return false;
				}
				insideBlock = true;
				blockEndedWithTerminator = false;
			}
			else if (isTerminator(op))
			{
				if (!insideBlock || blockEndedWithTerminator)
				{
					outError = "Invalid or duplicate basic-block terminator in SPIR-V";
					return false;
				}
				blockEndedWithTerminator = true;
			}
			else if (op == OpFunctionEnd)
			{
				if (!insideFunction || (insideBlock && !blockEndedWithTerminator))
				{
					outError = "OpFunctionEnd without terminated last basic block";
					return false;
				}
				insideFunction = false;
				insideBlock = false;
			}

			pc += wc;
		}

		if (insideFunction)
		{
			outError = "Unterminated OpFunction at end of SPIR-V module";
			return false;
		}
		if (!hasMemoryModel || !hasEntryPoint)
		{
			outError = "SPIR-V module missing OpMemoryModel or OpEntryPoint";
			return false;
		}
		return true;
	}
} // namespace ParaEngine
