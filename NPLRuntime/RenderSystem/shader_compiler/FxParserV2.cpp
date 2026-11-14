#include "FxParserV2.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <unordered_map>

namespace ParaEngine
{
	namespace
	{
		std::string ToLowerStr(const std::string& s)
		{
			std::string r = s;
			std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return r;
		}

		std::string TrimStr(const std::string& s)
		{
			size_t b = 0;
			while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b])))
				++b;
			size_t e = s.size();
			while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1])))
				--e;
			return s.substr(b, e - b);
		}

		bool IsIdentStart(char c)
		{
			return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
		}

		bool IsIdentChar(char c)
		{
			return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
		}

		// Strip // and /* */ comments while preserving newlines so line numbers stay accurate.
		std::string StripCommentsPreserveLines(const std::string& src)
		{
			std::string out;
			out.reserve(src.size());
			size_t i = 0;
			const size_t n = src.size();
			while (i < n)
			{
				if (src[i] == '"' || src[i] == '\'')
				{
					char q = src[i];
					out.push_back(src[i++]);
					while (i < n)
					{
						char c = src[i++];
						out.push_back(c);
						if (c == '\\' && i < n)
							out.push_back(src[i++]);
						else if (c == q)
							break;
					}
				}
				else if (i + 1 < n && src[i] == '/' && src[i + 1] == '/')
				{
					i += 2;
					while (i < n && src[i] != '\n')
						++i;
				}
				else if (i + 1 < n && src[i] == '/' && src[i + 1] == '*')
				{
					i += 2;
					while (i + 1 < n && !(src[i] == '*' && src[i + 1] == '/'))
					{
						if (src[i] == '\n')
							out.push_back('\n');
						++i;
					}
					if (i + 1 < n)
						i += 2;
				}
				else
				{
					out.push_back(src[i++]);
				}
			}
			return out;
		}

		uint32_t ParseFilterMode(const std::string& valLower)
		{
			if (valLower == "none") return 0;
			if (valLower == "point" || valLower == "nearest") return 1;
			if (valLower == "linear") return 2;
			if (valLower == "anisotropic") return 3;
			return 2;
		}

		uint32_t ParseAddressMode(const std::string& valLower)
		{
			if (valLower == "wrap" || valLower == "repeat") return 1;
			if (valLower == "mirror") return 2;
			if (valLower == "clamp") return 3;
			if (valLower == "border") return 4;
			if (valLower == "mirroronce") return 5;
			return 1;
		}

		uint32_t ParseBoolOrUint(const std::string& valLower)
		{
			if (valLower == "true") return 1u;
			if (valLower == "false") return 0u;
			if (valLower.size() > 2 && valLower[0] == '0' && valLower[1] == 'x')
				return static_cast<uint32_t>(std::strtoul(valLower.c_str(), nullptr, 16));
			return static_cast<uint32_t>(std::strtoul(valLower.c_str(), nullptr, 10));
		}

		uint32_t ParseCullMode(const std::string& valLower)
		{
			if (valLower == "none" || valLower == "1") return RSV_CULL_NONE;
			if (valLower == "cw" || valLower == "2") return RSV_CULL_CW;
			if (valLower == "ccw" || valLower == "3") return RSV_CULL_CCW;
			return RSV_CULL_CCW;
		}

		uint32_t ParseCmpFunc(const std::string& valLower)
		{
			if (valLower == "never") return RSV_CMP_NEVER;
			if (valLower == "less") return RSV_CMP_LESS;
			if (valLower == "equal") return RSV_CMP_EQUAL;
			if (valLower == "lessequal") return RSV_CMP_LESSEQUAL;
			if (valLower == "greater") return RSV_CMP_GREATER;
			if (valLower == "notequal") return RSV_CMP_NOTEQUAL;
			if (valLower == "greaterequal") return RSV_CMP_GREATEREQUAL;
			if (valLower == "always") return RSV_CMP_ALWAYS;
			return ParseBoolOrUint(valLower);
		}

		uint32_t ParseBlendFactor(const std::string& valLower)
		{
			if (valLower == "zero") return RSV_BLEND_ZERO;
			if (valLower == "one") return RSV_BLEND_ONE;
			if (valLower == "srccolor") return RSV_BLEND_SRCCOLOR;
			if (valLower == "invsrccolor") return RSV_BLEND_INVSRCCOLOR;
			if (valLower == "srcalpha") return RSV_BLEND_SRCALPHA;
			if (valLower == "invsrcalpha") return RSV_BLEND_INVSRCALPHA;
			if (valLower == "destalpha" || valLower == "dstalpha") return RSV_BLEND_DESTALPHA;
			if (valLower == "invdestalpha" || valLower == "invdstalpha") return RSV_BLEND_INVDESTALPHA;
			if (valLower == "destcolor" || valLower == "dstcolor") return RSV_BLEND_DESTCOLOR;
			if (valLower == "invdestcolor" || valLower == "invdstcolor") return RSV_BLEND_INVDESTCOLOR;
			if (valLower == "srcalphasat") return RSV_BLEND_SRCALPHASAT;
			return ParseBoolOrUint(valLower);
		}

		uint32_t ParseBlendOp(const std::string& valLower)
		{
			if (valLower == "add") return 1;
			if (valLower == "subtract") return 2;
			if (valLower == "revsubtract") return 3;
			if (valLower == "min") return 4;
			if (valLower == "max") return 5;
			return ParseBoolOrUint(valLower);
		}

		uint32_t ParseStencilOp(const std::string& valLower)
		{
			if (valLower == "keep") return RSV_STENCILOP_KEEP;
			if (valLower == "zero") return RSV_STENCILOP_ZERO;
			if (valLower == "replace") return RSV_STENCILOP_REPLACE;
			if (valLower == "incrsat") return RSV_STENCILOP_INCRSAT;
			if (valLower == "decrsat") return RSV_STENCILOP_DECRSAT;
			if (valLower == "invert") return RSV_STENCILOP_INVERT;
			if (valLower == "incr") return RSV_STENCILOP_INCR;
			if (valLower == "decr") return RSV_STENCILOP_DECR;
			return ParseBoolOrUint(valLower);
		}

		uint32_t ParseFillMode(const std::string& valLower)
		{
			if (valLower == "point") return RSV_FILL_POINT;
			if (valLower == "wireframe") return RSV_FILL_WIREFRAME;
			if (valLower == "solid") return RSV_FILL_SOLID;
			return RSV_FILL_SOLID;
		}

		bool ParseRenderStateAssignment(const std::string& rawKey, const std::string& rawVal, FxRenderStateAssignment& outState)
		{
			std::string k = ToLowerStr(TrimStr(rawKey));
			std::string v = ToLowerStr(TrimStr(rawVal));
			if (k.empty() || v.empty())
				return false;

			if (k == "zenable") { outState = { ERenderState::ZENABLE, ParseBoolOrUint(v) }; return true; }
			if (k == "zwriteenable") { outState = { ERenderState::ZWRITEENABLE, ParseBoolOrUint(v) }; return true; }
			if (k == "zfunc") { outState = { ERenderState::ZFUNC, ParseCmpFunc(v) }; return true; }
			if (k == "cullmode") { outState = { ERenderState::CULLMODE, ParseCullMode(v) }; return true; }
			if (k == "fillmode") { outState = { ERenderState::FILLMODE, ParseFillMode(v) }; return true; }
			if (k == "alphablendenable") { outState = { ERenderState::ALPHABLENDENABLE, ParseBoolOrUint(v) }; return true; }
			if (k == "srcblend") { outState = { ERenderState::SRCBLEND, ParseBlendFactor(v) }; return true; }
			if (k == "destblend") { outState = { ERenderState::DESTBLEND, ParseBlendFactor(v) }; return true; }
			if (k == "blendop") { outState = { ERenderState::BLENDOP, ParseBlendOp(v) }; return true; }
			if (k == "separatealphablendenable") { outState = { ERenderState::SEPARATEALPHABLENDENABLE, ParseBoolOrUint(v) }; return true; }
			if (k == "srcblendalpha") { outState = { ERenderState::SRCBLENDALPHA, ParseBlendFactor(v) }; return true; }
			if (k == "destblendalpha") { outState = { ERenderState::DESTBLENDALPHA, ParseBlendFactor(v) }; return true; }
			if (k == "blendopalpha") { outState = { ERenderState::BLENDOPALPHA, ParseBlendOp(v) }; return true; }
			if (k == "alphatestenable") { outState = { ERenderState::ALPHATESTENABLE, ParseBoolOrUint(v) }; return true; }
			if (k == "alpharef") { outState = { ERenderState::ALPHAREF, ParseBoolOrUint(v) }; return true; }
			if (k == "alphafunc") { outState = { ERenderState::ALPHAFUNC, ParseCmpFunc(v) }; return true; }
			if (k == "colorwriteenable" || k == "colorwriteenable[0]") { outState = { ERenderState::COLORWRITEENABLE, ParseBoolOrUint(v) }; return true; }
			if (k == "colorwriteenable1" || k == "colorwriteenable[1]") { outState = { ERenderState::COLORWRITEENABLE1, ParseBoolOrUint(v) }; return true; }
			if (k == "colorwriteenable2" || k == "colorwriteenable[2]") { outState = { ERenderState::COLORWRITEENABLE2, ParseBoolOrUint(v) }; return true; }
			if (k == "colorwriteenable3" || k == "colorwriteenable[3]") { outState = { ERenderState::COLORWRITEENABLE3, ParseBoolOrUint(v) }; return true; }
			if (k == "stencilenable") { outState = { ERenderState::STENCILENABLE, ParseBoolOrUint(v) }; return true; }
			if (k == "stencilfunc") { outState = { ERenderState::STENCILFUNC, ParseCmpFunc(v) }; return true; }
			if (k == "stencilref") { outState = { ERenderState::STENCILREF, ParseBoolOrUint(v) }; return true; }
			if (k == "stencilmask") { outState = { ERenderState::STENCILMASK, ParseBoolOrUint(v) }; return true; }
			if (k == "stencilwritemask") { outState = { ERenderState::STENCILWRITEMASK, ParseBoolOrUint(v) }; return true; }
			if (k == "stencilfail") { outState = { ERenderState::STENCILFAIL, ParseStencilOp(v) }; return true; }
			if (k == "stencilzfail") { outState = { ERenderState::STENCILZFAIL, ParseStencilOp(v) }; return true; }
			if (k == "stencilpass") { outState = { ERenderState::STENCILPASS, ParseStencilOp(v) }; return true; }
			if (k == "twosidedstencilmode") { outState = { ERenderState::TWOSIDEDSTENCILMODE, ParseBoolOrUint(v) }; return true; }
			if (k == "ccw_stencilfunc") { outState = { ERenderState::CCW_STENCILFUNC, ParseCmpFunc(v) }; return true; }
			if (k == "ccw_stencilfail") { outState = { ERenderState::CCW_STENCILFAIL, ParseStencilOp(v) }; return true; }
			if (k == "ccw_stencilzfail") { outState = { ERenderState::CCW_STENCILZFAIL, ParseStencilOp(v) }; return true; }
			if (k == "ccw_stencilpass") { outState = { ERenderState::CCW_STENCILPASS, ParseStencilOp(v) }; return true; }
			if (k == "scissortestenable") { outState = { ERenderState::SCISSORTESTENABLE, ParseBoolOrUint(v) }; return true; }
			if (k == "fogenable") { outState = { ERenderState::FOGENABLE, ParseBoolOrUint(v) }; return true; }
			if (k == "srgbwriteenable") { outState = { ERenderState::SRGBWRITEENABLE, ParseBoolOrUint(v) }; return true; }
			return false;
		}

		int ExtractFirstDigitNumber(const std::string& s)
		{
			for (size_t i = 0; i < s.size(); ++i)
			{
				if (std::isdigit(static_cast<unsigned char>(s[i])))
					return std::atoi(s.c_str() + i);
			}
			return -1;
		}

		void ParseDefaultValues(const std::string& initExpr, bool isBool, bool isInt, std::vector<float>& outFloats)
		{
			outFloats.clear();
			std::string s = TrimStr(initExpr);
			if (s.empty())
				return;

			std::string lower = ToLowerStr(s);
			if (lower == "true") { outFloats.push_back(1.0f); return; }
			if (lower == "false") { outFloats.push_back(0.0f); return; }

			// Strip constructor prefix like float3(...) or half4(...) or {...}
			size_t openPos = s.find_first_of("({");
			size_t closePos = s.find_last_of(")}");
			std::string inner = (openPos != std::string::npos && closePos != std::string::npos && closePos > openPos)
				? s.substr(openPos + 1, closePos - openPos - 1)
				: s;

			std::stringstream ss(inner);
			std::string item;
			while (std::getline(ss, item, ','))
			{
				std::string t = TrimStr(item);
				if (t.empty())
					continue;
				std::string tl = ToLowerStr(t);
				if (tl == "true")
					outFloats.push_back(1.0f);
				else if (tl == "false")
					outFloats.push_back(0.0f);
				else
				{
					// Remove trailing 'f'
					if (!t.empty() && (t.back() == 'f' || t.back() == 'F'))
						t.pop_back();
					char* endp = nullptr;
					float v = std::strtof(t.c_str(), &endp);
					if (endp != t.c_str())
					{
						if (isBool)
							outFloats.push_back(v != 0.0f ? 1.0f : 0.0f);
						else if (isInt)
							outFloats.push_back(std::round(v));
						else
							outFloats.push_back(v);
					}
				}
			}
		}

		bool IsKnownHLSLUniformType(const std::string& typeLower)
		{
			if (typeLower == "bool" || typeLower == "int" || typeLower == "uint" || typeLower == "dword" ||
				typeLower == "float" || typeLower == "half" || typeLower == "scalar")
				return true;
			const char* prefixes[] = { "float", "half", "int", "uint", "bool", "scalar" };
			for (const char* p : prefixes)
			{
				size_t plen = std::strlen(p);
				if (typeLower.compare(0, plen, p) == 0 && typeLower.size() > plen)
				{
					char c = typeLower[plen];
					if (c >= '1' && c <= '4')
						return true;
				}
			}
			if (typeLower == "matrix")
				return true;
			return false;
		}
	}

	std::string CFxParserV2::ResolveIncludes(
		const std::string& rawSource,
		const std::string& sourceName,
		FxIncludeCallback includeResolver,
		int depth)
	{
		if (depth > 16)
			return rawSource;

		std::istringstream iss(rawSource);
		std::ostringstream oss;
		std::string line;
		while (std::getline(iss, line))
		{
			std::string trimmed = TrimStr(line);
			if (trimmed.compare(0, 8, "#include") == 0 &&
				(trimmed.size() == 8 || std::isspace(static_cast<unsigned char>(trimmed[8])) || trimmed[8] == '"' || trimmed[8] == '<'))
			{
				size_t q1 = trimmed.find_first_of("\"<", 8);
				if (q1 != std::string::npos)
				{
					char closeChar = (trimmed[q1] == '"') ? '"' : '>';
					size_t q2 = trimmed.find(closeChar, q1 + 1);
					if (q2 != std::string::npos && q2 > q1 + 1)
					{
						std::string incPath = trimmed.substr(q1 + 1, q2 - q1 - 1);
						std::string incContent;
						if (includeResolver && includeResolver(incPath, sourceName, incContent))
						{
							oss << ResolveIncludes(incContent, incPath, includeResolver, depth + 1) << "\n";
							continue;
						}
					}
				}
			}
			oss << line << "\n";
		}
		return oss.str();
	}

	bool CFxParserV2::Parse(
		const std::string& rawSource,
		const std::string& sourceName,
		FxParsedDocument& outDoc,
		std::string& outError,
		FxIncludeCallback includeResolver)
	{
		outDoc = FxParsedDocument();
		outDoc.sourceName = sourceName;
		outError.clear();

		std::string included = ResolveIncludes(rawSource, sourceName, includeResolver, 0);
		std::string clean = StripCommentsPreserveLines(included);

		// Prepend standard SCALAR / dword compatibility macros if not already defined
		std::ostringstream d3dOut;
		std::ostringstream dxcOut;
		std::ostringstream glslOut;

		const char* commonCompatHeader =
			"#ifndef SCALAR\n"
			"#define SCALAR float\n"
			"#define SCALAR2 float2\n"
			"#define SCALAR3 float3\n"
			"#define SCALAR4 float4\n"
			"#define SCALAR2x2 float2x2\n"
			"#define SCALAR3x3 float3x3\n"
			"#define SCALAR4x4 float4x4\n"
			"#endif\n";

		const char* dxcSamplerCompatHeader =
			"struct __ParaSampler2D { Texture2D t; SamplerState s; };\n"
			"struct __ParaSampler3D { Texture3D t; SamplerState s; };\n"
			"struct __ParaSamplerCUBE { TextureCube t; SamplerState s; };\n"
			"__ParaSampler2D __MakeParaSampler2D(Texture2D t, SamplerState s) { __ParaSampler2D r; r.t = t; r.s = s; return r; }\n"
			"__ParaSampler3D __MakeParaSampler3D(Texture3D t, SamplerState s) { __ParaSampler3D r; r.t = t; r.s = s; return r; }\n"
			"__ParaSamplerCUBE __MakeParaSamplerCUBE(TextureCube t, SamplerState s) { __ParaSamplerCUBE r; r.t = t; r.s = s; return r; }\n"
			"float4 tex1D(__ParaSampler2D p, float u) { return p.t.Sample(p.s, float2(u, 0.5)); }\n"
			"float4 tex2D(__ParaSampler2D p, float2 uv) { return p.t.Sample(p.s, uv); }\n"
			"float4 tex2D(__ParaSampler2D p, float2 uv, float2 ddxVal, float2 ddyVal) { return p.t.SampleGrad(p.s, uv, ddxVal, ddyVal); }\n"
			"float4 tex2Dlod(__ParaSampler2D p, float4 uv) { return p.t.SampleLevel(p.s, uv.xy, uv.w); }\n"
			"float4 tex2Dbias(__ParaSampler2D p, float4 uv) { return p.t.SampleBias(p.s, uv.xy, uv.w); }\n"
			"float4 tex2Dgrad(__ParaSampler2D p, float2 uv, float2 ddxVal, float2 ddyVal) { return p.t.SampleGrad(p.s, uv, ddxVal, ddyVal); }\n"
			"float4 tex2Dproj(__ParaSampler2D p, float4 uv) { return p.t.Sample(p.s, uv.xy / uv.w); }\n"
			"float4 tex3D(__ParaSampler3D p, float3 uvw) { return p.t.Sample(p.s, uvw); }\n"
			"float4 tex3Dlod(__ParaSampler3D p, float4 uvw) { return p.t.SampleLevel(p.s, uvw.xyz, uvw.w); }\n"
			"float4 tex3Dbias(__ParaSampler3D p, float4 uvw) { return p.t.SampleBias(p.s, uvw.xyz, uvw.w); }\n"
			"float4 tex3Dproj(__ParaSampler3D p, float4 uvw) { return p.t.Sample(p.s, uvw.xyz / uvw.w); }\n"
			"float4 texCUBE(__ParaSamplerCUBE p, float3 dir) { return p.t.Sample(p.s, dir); }\n"
			"float4 texCUBElod(__ParaSamplerCUBE p, float4 dir) { return p.t.SampleLevel(p.s, dir.xyz, dir.w); }\n"
			"float4 texCUBEbias(__ParaSamplerCUBE p, float4 dir) { return p.t.SampleBias(p.s, dir.xyz, dir.w); }\n"
			"float4 texCUBEproj(__ParaSamplerCUBE p, float4 dir) { return p.t.Sample(p.s, dir.xyz / dir.w); }\n"
			"#define sampler2D __ParaSampler2D\n"
			"#define sampler1D __ParaSampler2D\n"
			"#define sampler3D __ParaSampler3D\n"
			"#define samplerCUBE __ParaSamplerCUBE\n"
			"#define sampler __ParaSampler2D\n";

		d3dOut << commonCompatHeader << "#ifndef dword\n#define dword uint\n#endif\n";
		dxcOut << commonCompatHeader << "#ifndef dword\n#define dword uint\n#endif\n" << dxcSamplerCompatHeader;
		glslOut << commonCompatHeader << "#ifndef dword\n#define dword int\n#endif\n";

		// Map texture variable names -> declared order index (0, 1, 2...)
		std::unordered_map<std::string, int> textureOrderMap;
		int nextTextureOrder = 0;
		int nextSamplerRegister = 0;

		const size_t n = clean.size();
		size_t pos = 0;

		auto skipWS = [&](size_t& p) {
			while (p < n && std::isspace(static_cast<unsigned char>(clean[p])))
				++p;
		};

		auto readIdent = [&](size_t& p) -> std::string {
			skipWS(p);
			if (p >= n || !IsIdentStart(clean[p]))
				return "";
			size_t start = p++;
			while (p < n && IsIdentChar(clean[p]))
				++p;
			return clean.substr(start, p - start);
		};

		auto skipBalanced = [&](size_t& p, char openCh, char closeCh) {
			if (p >= n || clean[p] != openCh)
				return;
			int depth = 0;
			while (p < n)
			{
				if (clean[p] == openCh) ++depth;
				else if (clean[p] == closeCh)
				{
					--depth;
					if (depth == 0) { ++p; return; }
				}
				++p;
			}
		};

		while (pos < n)
		{
			// Preserve whitespace & preprocessor directives verbatim at top level
			size_t wsStart = pos;
			while (pos < n && std::isspace(static_cast<unsigned char>(clean[pos])))
				++pos;
			if (pos > wsStart)
			{
				std::string ws = clean.substr(wsStart, pos - wsStart);
				d3dOut << ws;
				dxcOut << ws;
				glslOut << ws;
			}
			if (pos >= n)
				break;

			// Preprocessor line (#define, #ifdef, #endif, etc.)
			if (clean[pos] == '#')
			{
				size_t lineStart = pos;
				while (pos < n)
				{
					if (clean[pos] == '\\' && pos + 1 < n && (clean[pos + 1] == '\r' || clean[pos + 1] == '\n'))
					{
						pos += 2;
						if (pos < n && clean[pos - 1] == '\r' && clean[pos] == '\n')
							++pos;
						continue;
					}
					if (clean[pos] == '\n')
					{
						++pos;
						break;
					}
					++pos;
				}
				std::string ppLine = clean.substr(lineStart, pos - lineStart);
				d3dOut << ppLine;
				dxcOut << ppLine;
				glslOut << ppLine;
				continue;
			}

			// Peek first identifier at top level
			size_t stmtStart = pos;
			std::string firstWord = readIdent(pos);
			std::string firstLower = ToLowerStr(firstWord);

			// 1. technique / technique10 / technique11 block
			if (firstLower == "technique" || firstLower == "technique10" || firstLower == "technique11")
			{
				FxParsedTechnique tech;
				skipWS(pos);
				if (pos < n && IsIdentStart(clean[pos]))
					tech.name = readIdent(pos);
				else
					tech.name = "default_" + std::to_string(outDoc.techniques.size());

				skipWS(pos);
				if (pos < n && clean[pos] == '<')
					skipBalanced(pos, '<', '>');
				skipWS(pos);

				if (pos >= n || clean[pos] != '{')
				{
					outError = "Expected '{' after technique " + tech.name;
					return false;
				}
				size_t techOpen = pos;
				skipBalanced(pos, '{', '}');
				std::string techBody = clean.substr(techOpen + 1, (pos - techOpen) - 2);

				// Parse passes inside techBody
				size_t tp = 0;
				const size_t tn = techBody.size();
				auto skipTechWS = [&]() {
					while (tp < tn && std::isspace(static_cast<unsigned char>(techBody[tp])))
						++tp;
				};
				auto readTechIdent = [&]() -> std::string {
					skipTechWS();
					if (tp >= tn || !IsIdentStart(techBody[tp]))
						return "";
					size_t s = tp++;
					while (tp < tn && IsIdentChar(techBody[tp]))
						++tp;
					return techBody.substr(s, tp - s);
				};

				while (tp < tn)
				{
					skipTechWS();
					if (tp >= tn)
						break;
					std::string pw = readTechIdent();
					if (ToLowerStr(pw) != "pass")
					{
						++tp;
						continue;
					}
					FxParsedPass pass;
					skipTechWS();
					if (tp < tn && IsIdentStart(techBody[tp]))
						pass.name = readTechIdent();
					else
						pass.name = "P" + std::to_string(tech.passes.size());

					skipTechWS();
					if (tp < tn && techBody[tp] == '<')
					{
						int d = 0;
						while (tp < tn)
						{
							if (techBody[tp] == '<') ++d;
							else if (techBody[tp] == '>' && --d == 0) { ++tp; break; }
							++tp;
						}
					}
					skipTechWS();
					if (tp >= tn || techBody[tp] != '{')
						break;
					size_t passOpen = tp;
					int pdepth = 0;
					while (tp < tn)
					{
						if (techBody[tp] == '{') ++pdepth;
						else if (techBody[tp] == '}' && --pdepth == 0) { ++tp; break; }
						++tp;
					}
					std::string passBody = techBody.substr(passOpen + 1, (tp - passOpen) - 2);

					// Split passBody by ';'
					std::stringstream pss(passBody);
					std::string assignStmt;
					while (std::getline(pss, assignStmt, ';'))
					{
						std::string a = TrimStr(assignStmt);
						if (a.empty())
							continue;
						size_t eq = a.find('=');
						if (eq == std::string::npos)
							continue;
						std::string lhs = TrimStr(a.substr(0, eq));
						std::string rhs = TrimStr(a.substr(eq + 1));
						std::string lhsLower = ToLowerStr(lhs);
						std::string rhsLower = ToLowerStr(rhs);

						if (lhsLower == "vertexshader" || lhsLower == "pixelshader")
						{
							if (rhsLower == "null" || rhsLower == "0")
							{
								if (lhsLower == "vertexshader") pass.vsExplicitNull = true;
								else pass.psExplicitNull = true;
							}
							else if (rhsLower.compare(0, 7, "compile") == 0)
							{
								std::string rest = TrimStr(rhs.substr(7));
								std::istringstream rss(rest);
								std::string profile;
								rss >> profile;
								std::string afterProfile;
								std::getline(rss, afterProfile);
								afterProfile = TrimStr(afterProfile);
								size_t paren = afterProfile.find('(');
								std::string entry = (paren != std::string::npos) ? TrimStr(afterProfile.substr(0, paren)) : afterProfile;
								if (lhsLower == "vertexshader")
								{
									pass.vsProfile = profile;
									pass.vsEntry = entry;
								}
								else
								{
									pass.psProfile = profile;
									pass.psEntry = entry;
								}
							}
						}
						else
						{
							FxRenderStateAssignment rsa;
							if (ParseRenderStateAssignment(lhs, rhs, rsa))
								pass.renderStates.push_back(rsa);
						}
					}
					tech.passes.push_back(std::move(pass));
				}

				// Optional trailing ';' after technique { ... }
				size_t peekSemi = pos;
				skipWS(peekSemi);
				if (peekSemi < n && clean[peekSemi] == ';')
					pos = peekSemi + 1;

				outDoc.techniques.push_back(std::move(tech));
				continue;
			}

			// Read full top-level statement or function/struct block up to ';' or '}'
			pos = stmtStart;
			int braceDepth = 0;
			int parenDepth = 0;
			bool endedWithBrace = false;
			while (pos < n)
			{
				char c = clean[pos++];
				if (c == '(') ++parenDepth;
				else if (c == ')') --parenDepth;
				else if (c == '{') ++braceDepth;
				else if (c == '}')
				{
					--braceDepth;
					if (braceDepth == 0 && parenDepth <= 0)
					{
						// Structs or sampler_state blocks have a trailing ';', functions do not
						size_t saveP = pos;
						skipWS(saveP);
						if (saveP < n && clean[saveP] == ';')
						{
							pos = saveP + 1;
							endedWithBrace = false;
						}
						else
						{
							endedWithBrace = true;
						}
						break;
					}
				}
				else if (c == ';' && braceDepth == 0 && parenDepth <= 0)
				{
					break;
				}
			}

			std::string stmt = clean.substr(stmtStart, pos - stmtStart);
			if (endedWithBrace)
			{
				// Function definition — emit verbatim to D3D, DXC, and GLSL HLSL bodies
				d3dOut << stmt;
				dxcOut << stmt;
				glslOut << stmt;
				continue;
			}

			// Inspect non-function top-level declaration ending with ';'
			size_t scan = 0;
			auto skipStmtWS = [&]() {
				while (scan < stmt.size() && std::isspace(static_cast<unsigned char>(stmt[scan])))
					++scan;
			};
			auto readStmtWord = [&]() -> std::string {
				skipStmtWS();
				if (scan >= stmt.size() || !IsIdentStart(stmt[scan]))
					return "";
				size_t s = scan++;
				while (scan < stmt.size() && IsIdentChar(stmt[scan]))
					++scan;
				return stmt.substr(s, scan - s);
			};

			std::string w1 = readStmtWord();
			std::string w1Lower = ToLowerStr(w1);

			// struct / typedef / cbuffer -> emit verbatim
			if (w1Lower == "struct" || w1Lower == "typedef" || w1Lower == "cbuffer")
			{
				d3dOut << stmt;
				dxcOut << stmt;
				glslOut << stmt;
				continue;
			}

			// Forward function prototype e.g. float2 computeTextureCoord(...);
			if (stmt.find('(') != std::string::npos && stmt.find('=') == std::string::npos && stmt.find("register") == std::string::npos)
			{
				d3dOut << stmt;
				dxcOut << stmt;
				glslOut << stmt;
				continue;
			}

			// 2. D3DX9 texture declaration: texture tex0 : TEXTURE;
			if (w1Lower == "texture" || w1Lower == "texture2d" || w1Lower == "texturecube" || w1Lower == "texture3d")
			{
				std::string texName = readStmtWord();
				if (!texName.empty())
				{
					int num = ExtractFirstDigitNumber(texName);
					int idx = (num >= 0) ? num : nextTextureOrder;
					textureOrderMap[ToLowerStr(texName)] = idx;
					if (idx >= nextTextureOrder)
						nextTextureOrder = idx + 1;
				}
				// Stripped from both D3D SM4 backwards-compat sampler syntax and hlsl2glslfork
				continue;
			}

			// 3. Sampler declaration: sampler / sampler2D / sampler3D / samplerCUBE
			if (w1Lower == "sampler" || w1Lower == "sampler2d" || w1Lower == "sampler3d" || w1Lower == "samplercube" || w1Lower == "sampler1d")
			{
				FxSamplerDesc sdesc;
				std::string sampName = readStmtWord();
				sdesc.samplerName = sampName;

				if (w1Lower == "samplercube")
					sdesc.samplerType = "samplerCUBE";
				else if (w1Lower == "sampler3d")
					sdesc.samplerType = "sampler3D";
				else
				{
					// Auto-detect if used with texCUBE(...) or tex3D(...) in the rest of the shader
					if (clean.find("texCUBE(" + sampName) != std::string::npos || clean.find("texCUBE( " + sampName) != std::string::npos)
						sdesc.samplerType = "samplerCUBE";
					else if (clean.find("tex3D(" + sampName) != std::string::npos || clean.find("tex3D( " + sampName) != std::string::npos)
						sdesc.samplerType = "sampler3D";
					else
						sdesc.samplerType = "sampler2D";
				}

				// Check for explicit : register(sN)
				std::string stmtLower = ToLowerStr(stmt);
				size_t regPos = stmtLower.find("register");
				if (regPos != std::string::npos)
				{
					size_t rp1 = stmtLower.find('(', regPos);
					size_t rp2 = stmtLower.find(')', regPos);
					if (rp1 != std::string::npos && rp2 != std::string::npos && rp2 > rp1)
					{
						int rnum = ExtractFirstDigitNumber(stmtLower.substr(rp1 + 1, rp2 - rp1 - 1));
						if (rnum >= 0)
							sdesc.registerIndex = rnum;
					}
				}

				// Parse sampler_state { ... } if present
				size_t braceOpen = stmt.find('{');
				size_t braceClose = stmt.rfind('}');
				if (braceOpen != std::string::npos && braceClose != std::string::npos && braceClose > braceOpen)
				{
					std::string body = stmt.substr(braceOpen + 1, braceClose - braceOpen - 1);
					std::stringstream bss(body);
					std::string item;
					while (std::getline(bss, item, ';'))
					{
						std::string kv = TrimStr(item);
						size_t eq = kv.find('=');
						if (eq == std::string::npos)
							continue;
						std::string sk = ToLowerStr(TrimStr(kv.substr(0, eq)));
						std::string sv = TrimStr(kv.substr(eq + 1));
						std::string svLower = ToLowerStr(sv);

						if (sk == "texture")
						{
							// Strip <...> or (...)
							if (!sv.empty() && (sv.front() == '<' || sv.front() == '('))
								sv.erase(0, 1);
							if (!sv.empty() && (sv.back() == '>' || sv.back() == ')'))
								sv.pop_back();
							sdesc.textureName = TrimStr(sv);
							if (sdesc.registerIndex < 0)
							{
								auto it = textureOrderMap.find(ToLowerStr(sdesc.textureName));
								if (it != textureOrderMap.end())
									sdesc.registerIndex = it->second;
								else
								{
									int tnum = ExtractFirstDigitNumber(sdesc.textureName);
									if (tnum >= 0)
										sdesc.registerIndex = tnum;
								}
							}
						}
						else if (sk == "minfilter") { sdesc.hasMinFilter = true; sdesc.minFilter = ParseFilterMode(svLower); }
						else if (sk == "magfilter") { sdesc.hasMagFilter = true; sdesc.magFilter = ParseFilterMode(svLower); }
						else if (sk == "mipfilter") { sdesc.hasMipFilter = true; sdesc.mipFilter = ParseFilterMode(svLower); }
						else if (sk == "addressu") { sdesc.hasAddressU = true; sdesc.addressU = ParseAddressMode(svLower); }
						else if (sk == "addressv") { sdesc.hasAddressV = true; sdesc.addressV = ParseAddressMode(svLower); }
						else if (sk == "addressw") { sdesc.hasAddressW = true; sdesc.addressW = ParseAddressMode(svLower); }
						else if (sk == "bordercolor") { sdesc.hasBorderColor = true; sdesc.borderColor = ParseBoolOrUint(svLower); }
					}
				}

				if (sdesc.registerIndex < 0)
				{
					int snum = ExtractFirstDigitNumber(sdesc.samplerName);
					sdesc.registerIndex = (snum >= 0) ? snum : nextSamplerRegister;
				}
				if (sdesc.registerIndex >= nextSamplerRegister)
					nextSamplerRegister = sdesc.registerIndex + 1;

				d3dOut << sdesc.samplerType << " " << sdesc.samplerName << " : register(s" << sdesc.registerIndex << ");";
				{
					const char* texClass = "Texture2D";
					const char* makeFn = "__MakeParaSampler2D";
					if (sdesc.samplerType == "samplerCUBE")
					{
						texClass = "TextureCube";
						makeFn = "__MakeParaSamplerCUBE";
					}
					else if (sdesc.samplerType == "sampler3D")
					{
						texClass = "Texture3D";
						makeFn = "__MakeParaSampler3D";
					}
					dxcOut << texClass << " __para_tex_" << sdesc.samplerName << " : register(t" << sdesc.registerIndex << "); "
						<< "SamplerState __para_smp_" << sdesc.samplerName << " : register(s" << sdesc.registerIndex << ");\n"
						<< "#define " << sdesc.samplerName << " " << makeFn << "(__para_tex_" << sdesc.samplerName
						<< ", __para_smp_" << sdesc.samplerName << ")\n";
				}
				glslOut << sdesc.samplerType << " " << sdesc.samplerName << ";";
				outDoc.samplers.push_back(std::move(sdesc));
				continue;
			}

			// 4. Global variable / constant / uniform parameter
			bool hasStatic = false;
			bool hasConst = false;
			bool hasShared = false;
			bool hasUniformKw = false;

			scan = 0;
			std::string typeToken;
			while (true)
			{
				size_t beforeWord = scan;
				std::string w = readStmtWord();
				std::string wl = ToLowerStr(w);
				if (wl == "static") { hasStatic = true; continue; }
				if (wl == "const") { hasConst = true; continue; }
				if (wl == "shared" || wl == "extern") { hasShared = true; continue; }
				if (wl == "uniform") { hasUniformKw = true; continue; }
				if (wl == "row_major" || wl == "column_major") { continue; }
				typeToken = w;
				break;
			}

			std::string varName = readStmtWord();
			if (typeToken.empty() || varName.empty())
			{
				d3dOut << stmt;
				dxcOut << stmt;
				glslOut << stmt;
				continue;
			}

			// Check array brackets e.g. g_lightpos[MAX_LIGHTS_NUM]
			std::string arraySuffix;
			int elemCount = 1;
			skipStmtWS();
			if (scan < stmt.size() && stmt[scan] == '[')
			{
				size_t arrStart = scan;
				while (scan < stmt.size() && stmt[scan] != ']')
					++scan;
				if (scan < stmt.size() && stmt[scan] == ']')
					++scan;
				arraySuffix = stmt.substr(arrStart, scan - arrStart);
				int parsedElem = ExtractFirstDigitNumber(arraySuffix);
				if (parsedElem > 0)
					elemCount = parsedElem;
			}

			// Parse optional ': semantic' and ': register(...)' and '<annotations>' and '= initializer'
			std::string semantic;
			int regIndex = -1;
			std::string initExpr;

			while (scan < stmt.size())
			{
				skipStmtWS();
				if (scan >= stmt.size() || stmt[scan] == ';')
					break;
				if (stmt[scan] == '<')
				{
					int d = 0;
					while (scan < stmt.size())
					{
						if (stmt[scan] == '<') ++d;
						else if (stmt[scan] == '>' && --d == 0) { ++scan; break; }
						++scan;
					}
					continue;
				}
				if (stmt[scan] == ':')
				{
					++scan;
					std::string semOrReg = readStmtWord();
					if (ToLowerStr(semOrReg) == "register")
					{
						skipStmtWS();
						if (scan < stmt.size() && stmt[scan] == '(')
						{
							size_t r1 = ++scan;
							while (scan < stmt.size() && stmt[scan] != ')')
								++scan;
							regIndex = ExtractFirstDigitNumber(stmt.substr(r1, scan - r1));
							if (scan < stmt.size() && stmt[scan] == ')')
								++scan;
						}
					}
					else if (semantic.empty())
					{
						semantic = semOrReg;
					}
					continue;
				}
				if (stmt[scan] == '=')
				{
					++scan;
					size_t initStart = scan;
					while (scan < stmt.size() && stmt[scan] != ';')
						++scan;
					initExpr = TrimStr(stmt.substr(initStart, scan - initStart));
					break;
				}
				++scan;
			}

			// Case A: True compile-time constant (static const ... = ..., OR const ... = ... with NO semantic!)
			if (hasStatic || (hasConst && !initExpr.empty() && semantic.empty() && !hasUniformKw))
			{
				// Rewrite brace init e.g. `const float2 g_textureScale = {0.08, 0.08};` -> `static const float2 g_textureScale = float2(0.08, 0.08);` for GLSL compatibility
				std::string normInit = initExpr;
				if (!normInit.empty() && normInit.front() == '{' && normInit.back() == '}' && arraySuffix.empty())
				{
					std::string inner = normInit.substr(1, normInit.size() - 2);
					normInit = typeToken + "(" + inner + ")";
				}
				if (hasStatic && !hasConst)
				{
					d3dOut << "static " << typeToken << " " << varName << arraySuffix;
					dxcOut << "static " << typeToken << " " << varName << arraySuffix;
					glslOut << "static " << typeToken << " " << varName << arraySuffix;
				}
				else
				{
					d3dOut << "static const " << typeToken << " " << varName << arraySuffix;
					dxcOut << "static const " << typeToken << " " << varName << arraySuffix;
					glslOut << "static const " << typeToken << " " << varName << arraySuffix;
				}
				if (!initExpr.empty())
				{
					d3dOut << " = " << initExpr;
					dxcOut << " = " << initExpr;
					glslOut << " = " << normInit;
				}
				d3dOut << ";";
				dxcOut << ";";
				glslOut << ";";
				continue;
			}

			// Case B: Shader uniform parameter!
			std::string typeLower = ToLowerStr(typeToken);
			FxUniformParam param;
			param.name = varName;
			param.semantic = semantic;
			param.typeName = typeToken;
			param.registerIndex = regIndex;
			param.elementCount = elemCount;
			param.isBool = (typeLower.compare(0, 4, "bool") == 0);
			param.isInt = (typeLower.compare(0, 3, "int") == 0 || typeLower.compare(0, 4, "uint") == 0 || typeLower == "dword");
			if (!initExpr.empty() && IsKnownHLSLUniformType(typeLower))
			{
				ParseDefaultValues(initExpr, param.isBool, param.isInt, param.defaultFloats);
				param.hasDefault = !param.defaultFloats.empty();
			}
			outDoc.params.push_back(std::move(param));

			// Emit clean uniform declaration without `const` or `= {...}` initializer (defaults are applied via FxUniformParam!)
			d3dOut << typeToken << " " << varName << arraySuffix;
			if (!semantic.empty())
				d3dOut << " : " << semantic;
			d3dOut << ";";

			dxcOut << typeToken << " " << varName << arraySuffix;
			if (!semantic.empty())
				dxcOut << " : " << semantic;
			dxcOut << ";";

			glslOut << typeToken << " " << varName << arraySuffix;
			if (!semantic.empty())
				glslOut << " : " << semantic;
			glslOut << ";";
		}

		outDoc.hlslCodeForD3D = d3dOut.str();
		outDoc.hlslCodeForDXC = dxcOut.str();
		outDoc.hlslCodeForGLSL = glslOut.str();

		if (outDoc.techniques.empty())
		{
			outError = "No technique found in shader: " + sourceName;
			return false;
		}
		return true;
	}
}
