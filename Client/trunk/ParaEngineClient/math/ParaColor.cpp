//-----------------------------------------------------------------------------
// Class:	Color
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.8.18
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "StringHelper.h"
#include "ParaColor.h"


using namespace ParaEngine;

// Common colors.
const LinearColor LinearColor::White(1.f, 1.f, 1.f);
const LinearColor LinearColor::Gray(0.5f, 0.5f, 0.5f);
const LinearColor LinearColor::Black(0, 0, 0);
const LinearColor LinearColor::Transparent(0, 0, 0, 0);
const LinearColor LinearColor::Red(1.f, 0, 0);
const LinearColor LinearColor::Green(0, 1.f, 0);
const LinearColor LinearColor::Blue(0, 0, 1.f);
const LinearColor LinearColor::Yellow(1.f, 1.f, 0);

const Color Color::White(255, 255, 255);
const Color Color::Black(0, 0, 0);
const Color Color::Red(255, 0, 0);
const Color Color::Green(0, 255, 0);
const Color Color::Blue(0, 0, 255);
const Color Color::Yellow(255, 255, 0);
const Color Color::Cyan(0, 255, 255);
const Color Color::Magenta(255, 0, 255);

/**
* Helper used by Color -> LinearColor conversion. We don't use a lookup table as unlike pow, multiplication is fast.
*/
static const float OneOver255 = 1.0f / 255.0f;

//	Color->LinearColor conversion.
LinearColor::LinearColor(const class Color& C)
{
	*this = C.ReinterpretAsLinear();
	/*r = PowOneOver255Table[C.r];
	g = PowOneOver255Table[C.g];
	b = PowOneOver255Table[C.b];
	a = float(C.a) * OneOver255;*/
}

LinearColor::LinearColor(const class Vector3& Vector) :
r(Vector.x),g(Vector.y),b(Vector.z),a(1.0f)
{}


/** Quantizes the linear color and returns the result as a Color with optional sRGB conversion and quality as goal. */
Color LinearColor::ToColor(bool bSRGB) const
{
	float FloatR = Math::Clamp(r, 0.0f, 1.0f);
	float FloatG = Math::Clamp(g, 0.0f, 1.0f);
	float FloatB = Math::Clamp(b, 0.0f, 1.0f);
	float FloatA = Math::Clamp(a, 0.0f, 1.0f);

	if (bSRGB)
	{
		FloatR = Math::Pow(FloatR, 1.0f / 2.2f);
		FloatG = Math::Pow(FloatG, 1.0f / 2.2f);
		FloatB = Math::Pow(FloatB, 1.0f / 2.2f);
	}

	Color ret;

	ret.a = (uint8)Math::Floor(FloatA * 255.999f);
	ret.r = (uint8)Math::Floor(FloatR * 255.999f);
	ret.g = (uint8)Math::Floor(FloatG * 255.999f);
	ret.b = (uint8)Math::Floor(FloatB * 255.999f);

	return ret;
}


Color LinearColor::Quantize() const
{
	return Color(
		(uint8)Math::Clamp<int32>(Math::Trunc(r*255.f), 0, 255),
		(uint8)Math::Clamp<int32>(Math::Trunc(g*255.f), 0, 255),
		(uint8)Math::Clamp<int32>(Math::Trunc(b*255.f), 0, 255),
		(uint8)Math::Clamp<int32>(Math::Trunc(a*255.f), 0, 255)
		);
}

/**
* Returns a desaturated color, with 0 meaning no desaturation and 1 == full desaturation
*
* @param	Desaturation	Desaturation factor in range [0..1]
* @return	Desaturated color
*/
LinearColor LinearColor::Desaturate(float Desaturation) const
{
	float Lum = ComputeLuminance();
	return Math::Lerp(*this, LinearColor(Lum, Lum, Lum, 0), Desaturation);
}

/** Computes the perceptually weighted luminance value of a color. */
float LinearColor::ComputeLuminance() const
{
	return r * 0.3f + g * 0.59f + b * 0.11f;
}

// Convert from RGBE to float as outlined in Gregory Ward's Real Pixels article, Graphics Gems II, page 80.
LinearColor Color::FromRGBE() const
{
	if (a == 0)
		return LinearColor::Black;
	else
	{
		const float Scale = (float)ldexp(1 / 255.0, a - 128);
		return LinearColor(r * Scale, g * Scale, b * Scale, 1.0f);
	}
}

/**
* Converts byte hue-saturation-brightness to floating point red-green-blue.
*/
LinearColor LinearColor::FGetHSV(uint8 H, uint8 S, uint8 V)
{
	float Brightness = V * 1.4f / 255.f;
	Brightness *= 0.7f / (0.01f + Math::Sqrt(Brightness));
	Brightness = Math::Clamp(Brightness, 0.f, 1.f);
	const Vector3 Hue = (H<86) ? Vector3((85 - H) / 85.f, (H - 0) / 85.f, 0) : (H<171) ? Vector3(0, (170 - H) / 85.f, (H - 85) / 85.f) : Vector3((H - 170) / 85.f, 0, (255 - H) / 84.f);
	const Vector3 ColorVector = (Hue + S / 255.f * (Vector3(1, 1, 1) - Hue)) * Brightness;
	return LinearColor(ColorVector.x, ColorVector.y, ColorVector.z, 1);
}


/** Converts a linear space RGB color to an HSV color */
LinearColor LinearColor::LinearRGBToHSV() const
{
	const float RGBMin = Math::Min3(r, g, b);
	const float RGBMax = Math::Max3(r, g, b);
	const float RGBRange = RGBMax - RGBMin;

	const float Hue = (RGBMax == RGBMin ? 0.0f :
		RGBMax == r ? fmod((((g - b) / RGBRange) * 60.0f) + 360.0f, 360.0f) :
		RGBMax == g ? (((b - r) / RGBRange) * 60.0f) + 120.0f :
		RGBMax == b ? (((r - g) / RGBRange) * 60.0f) + 240.0f :
		0.0f);

	const float Saturation = (RGBMax == 0.0f ? 0.0f : RGBRange / RGBMax);
	const float Value = RGBMax;

	// In the new color, r = H, g = S, b = V, a = a
	return LinearColor(Hue, Saturation, Value, a);
}



/** Converts an HSV color to a linear space RGB color */
LinearColor LinearColor::HSVToLinearRGB() const
{
	// In this color, r = H, g = S, b = V
	const float Hue = r;
	const float Saturation = g;
	const float Value = b;

	const float HDiv60 = Hue / 60.0f;
	const float HDiv60_Floor = floorf(HDiv60);
	const float HDiv60_Fraction = HDiv60 - HDiv60_Floor;

	const float RGBValues[4] = {
		Value,
		Value * (1.0f - Saturation),
		Value * (1.0f - (HDiv60_Fraction * Saturation)),
		Value * (1.0f - ((1.0f - HDiv60_Fraction) * Saturation)),
	};
	const uint32 RGBSwizzle[6][3] = {
		{ 0, 3, 1 },
		{ 2, 0, 1 },
		{ 1, 0, 3 },
		{ 1, 2, 0 },
		{ 3, 1, 0 },
		{ 0, 1, 2 },
	};
	const uint32 SwizzleIndex = ((uint32)HDiv60_Floor) % 6;

	return LinearColor(RGBValues[RGBSwizzle[SwizzleIndex][0]],
		RGBValues[RGBSwizzle[SwizzleIndex][1]],
		RGBValues[RGBSwizzle[SwizzleIndex][2]],
		a);
}


/**
* Makes a random but quite nice color.
*/
Color Color::MakeRandomColor()
{
	const uint8 Hue = (uint8)(Math::Rand()*255.f);
	return Color(LinearColor::FGetHSV(Hue, 0, 255));
}

Color Color::MakeRedToGreenColorFromScalar(float Scalar)
{
	uint8 r, g, b;
	r = g = b = 0;

	float RedSclr = Math::Clamp<float>((1.0f - Scalar) / 0.5f, 0.f, 1.f);
	float GreenSclr = Math::Clamp<float>((Scalar / 0.5f), 0.f, 1.f);
	r = (uint8)Math::Trunc(255 * RedSclr);
	g = (uint8)Math::Trunc(255 * GreenSclr);
	return Color(r, g, b);
}

void ParaEngine::Color::operator=(const uint32 color)
{
	DWColor() = color;
}

void ParaEngine::Color::operator=(const LinearColor& color)
{
	*this = color.Quantize();
}

// fast, for more accuracy use LinearColor::ToColor()
// TODO: doesn't handle negative colors well, implicit constructor can cause
// accidental conversion (better use .ToColor(true))
ParaEngine::Color::Color(const LinearColor& C) 
{
	*this = C.Quantize();

	// put these into the body for proper ordering with INTEL vs non-INTEL_BYTE_ORDER
	/*r = (uint8)Math::Clamp(Math::Trunc(Math::Pow(C.r, 1.0f / 2.2f) * 255.0f), (int32)0, (int32)255);
	g = (uint8)Math::Clamp(Math::Trunc(Math::Pow(C.g, 1.0f / 2.2f) * 255.0f), (int32)0, (int32)255);
	b = (uint8)Math::Clamp(Math::Trunc(Math::Pow(C.b, 1.0f / 2.2f) * 255.0f), (int32)0, (int32)255);
	a = (uint8)Math::Clamp(Math::Trunc(C.a              * 255.0f), (int32)0, (int32)255);*/
}

ParaEngine::Color::Color(uint32 InColor)
{
	DWColor() = InColor;
}

uint8 valueFromHexChar(char high)
{
	uint8 nHigh = 0;
	if (high >= '0' && high <= '9')
		nHigh = high - '0';
	else if (high >= 'a' && high <= 'f')
		nHigh = high - 'a' + 10;
	else if (high >= 'A' && high <= 'F')
		nHigh = high - 'A' + 10;
	return nHigh;
}

uint8 fromHexStr(const char* s)
{
	char high = s[0];
	if (high != '\0')
	{
		char low = s[1];
		if (low != '\0')
		{
			return ((valueFromHexChar(high) << 4) + valueFromHexChar(low));
		}
	}
	return 0xff;
}

Color ParaEngine::Color::FromString(const char* sColor)
{
	if (sColor && sColor[0]!='\0')
	{
		if (sColor[0] != '#')
		{
			int r = 255, g = 255, b = 255, a = 255;
			int nCount = sscanf(sColor, "%d %d %d %d", &r, &g, &b, &a);
			if (nCount == 3)
				a = 255;
			return COLOR_ARGB(a, r, g, b);
		}
		else
		{
			// #RGBA  or #RGB
			++sColor;
			int nLength = 0;
			while (sColor[nLength++]!='\0' && nLength<8)
			{
			}
			if (nLength >= 8)
				return Color(fromHexStr(sColor), fromHexStr(sColor + 2), fromHexStr(sColor + 4), fromHexStr(sColor + 6));
			else if (nLength >= 6)
				return Color(fromHexStr(sColor), fromHexStr(sColor + 2), fromHexStr(sColor + 4), 0xff);

		}
	}
	return 0;
}

std::string ParaEngine::Color::ToRGBString()
{
	char temp[30];
	string color = "";
	ParaEngine::StringHelper::fast_itoa((r), temp, 30);
	color += temp; color += " ";
	ParaEngine::StringHelper::fast_itoa((g), temp, 30);
	color += temp; color += " ";
	ParaEngine::StringHelper::fast_itoa((b), temp, 30);
	color += temp;
	return color;
}

std::string ParaEngine::Color::ToRGBAString()
{
	char temp[30];
	string color = "";
	ParaEngine::StringHelper::fast_itoa((r), temp, 30);
	color += temp; color += " ";
	ParaEngine::StringHelper::fast_itoa((g), temp, 30);
	color += temp; color += " ";
	ParaEngine::StringHelper::fast_itoa((b), temp, 30);
	color += temp; color += " ";
	ParaEngine::StringHelper::fast_itoa((a), temp, 30);
	color += temp;
	return color;
}

uint16 ParaEngine::Color::convert32_16(uint32 rgb)
{
	uint32 a = ((rgb & 0xF0000000) >> 16);
	uint32 r = ((rgb & 0x00F00000) >> 12);
	uint32 g = ((rgb & 0x0000F000) >> 8);
	uint32 b = ((rgb & 0x000000F0) >> 4);
	return (uint16)(a | r | g | b);
}

uint32 ParaEngine::Color::convert16_32(uint16 rgb)
{
	uint32 a = ((rgb & 0xF000) << 16);
	uint32 r = ((rgb & 0x0F00) << 12);
	uint32 g = ((rgb & 0x00F0) << 8);
	uint32 b = ((rgb & 0x000F) << 4);
	return (a | r | g | b);
}

void ComputeAndFixedColorAndIntensity(const LinearColor& InLinearColor, Color& OutColor, float& OutIntensity)
{
	float MaxComponent = Math::Max(DELTA, Math::Max(InLinearColor.r, Math::Max(InLinearColor.g, InLinearColor.b)));
	OutColor = (InLinearColor / MaxComponent);
	OutIntensity = MaxComponent;
}

#ifdef WIN32
#pragma warning( push )
// only used in /fp:precise 
#pragma warning( disable : 4305 ) 
#endif

/**
* Pow table for fast Color -> LinearColor conversion.
*
* Math::Pow( i / 255.f, 2.2f )
*/
float LinearColor::PowOneOver255Table[256] =
{
	0, 5.07705190066176E-06, 2.33280046660989E-05, 5.69217657121931E-05, 0.000107187362341244, 0.000175123977503027, 0.000261543754548491, 0.000367136269815943, 0.000492503787191433,
	0.000638182842167022, 0.000804658499513058, 0.000992374304074325, 0.0012017395224384, 0.00143313458967186, 0.00168691531678928, 0.00196341621339647, 0.00226295316070643,
	0.00258582559623417, 0.00293231832393836, 0.00330270303200364, 0.00369723957890013, 0.00411617709328275, 0.00455975492252602, 0.00502820345685554, 0.00552174485023966,
	0.00604059365484981, 0.00658495738258168, 0.00715503700457303, 0.00775102739766061, 0.00837311774514858, 0.00902149189801213, 0.00969632870165823, 0.0103978022925553,
	0.0111260823683832, 0.0118813344348137, 0.0126637200315821, 0.0134733969401426, 0.0143105193748841, 0.0151752381596252, 0.0160677008908869, 0.01698805208925, 0.0179364333399502,
	0.0189129834237215, 0.0199178384387857, 0.0209511319147811, 0.0220129949193365, 0.0231035561579214, 0.0242229420675342, 0.0253712769047346, 0.0265486828284729, 0.027755279978126,
	0.0289911865471078, 0.0302565188523887, 0.0315513914002264, 0.0328759169483838, 0.034230206565082, 0.0356143696849188, 0.0370285141619602, 0.0384727463201946, 0.0399471710015256,
	0.0414518916114625, 0.0429870101626571, 0.0445526273164214, 0.0461488424223509, 0.0477757535561706, 0.049433457555908, 0.0511220500564934, 0.052841625522879, 0.0545922772817603,
	0.0563740975519798, 0.0581871774736854, 0.0600316071363132, 0.0619074756054558, 0.0638148709486772, 0.0657538802603301, 0.0677245896854243, 0.0697270844425988, 0.0717614488462391,
	0.0738277663277846, 0.0759261194562648, 0.0780565899581019, 0.080219258736215, 0.0824142058884592, 0.0846415107254295, 0.0869012517876603, 0.0891935068622478, 0.0915183529989195,
	0.0938758665255778, 0.0962661230633397, 0.0986891975410945, 0.1011451642096, 0.103634096655137, 0.106156067812744, 0.108711149979039, 0.11129941482466, 0.113920933406333,
	0.116575776178572, 0.119264013005047, 0.121985713169619, 0.124740945387051, 0.127529777813422, 0.130352278056244, 0.1332085131843, 0.136098549737202, 0.139022453734703,
	0.141980290685736, 0.144972125597231, 0.147998022982685, 0.151058046870511, 0.154152260812165, 0.157280727890073, 0.160443510725344, 0.16364067148529, 0.166872271890766,
	0.170138373223312, 0.173439036332135, 0.176774321640903, 0.18014428915439, 0.183548998464951, 0.186988508758844, 0.190462878822409, 0.193972167048093, 0.19751643144034,
	0.201095729621346, 0.204710118836677, 0.208359655960767, 0.212044397502288, 0.215764399609395, 0.219519718074868, 0.223310408341127, 0.227136525505149, 0.230998124323267,
	0.23489525921588, 0.238827984272048, 0.242796353254002, 0.24680041960155, 0.2508402364364, 0.254915856566385, 0.259027332489606, 0.263174716398492, 0.267358060183772,
	0.271577415438375, 0.275832833461245, 0.280124365261085, 0.284452061560024, 0.288815972797219, 0.293216149132375, 0.297652640449211, 0.302125496358853, 0.306634766203158,
	0.311180499057984, 0.315762743736397, 0.32038154879181, 0.325036962521076, 0.329729032967515, 0.334457807923889, 0.339223334935327, 0.344025661302187, 0.348864834082879,
	0.353740900096629, 0.358653905926199, 0.363603897920553, 0.368590922197487, 0.373615024646202, 0.37867625092984, 0.383774646487975, 0.388910256539059, 0.394083126082829,
	0.399293299902674, 0.404540822567962, 0.409825738436323, 0.415148091655907, 0.420507926167587, 0.425905285707146, 0.43134021380741, 0.436812753800359, 0.442322948819202,
	0.44787084180041, 0.453456475485731, 0.45907989242416, 0.46474113497389, 0.470440245304218, 0.47617726539744, 0.481952237050698, 0.487765201877811, 0.493616201311074,
	0.49950527660303, 0.505432468828216, 0.511397818884879, 0.517401367496673, 0.523443155214325, 0.529523222417277, 0.535641609315311, 0.541798355950137, 0.547993502196972,
	0.554227087766085, 0.560499152204328, 0.566809734896638, 0.573158875067523, 0.579546611782525, 0.585972983949661, 0.592438030320847, 0.598941789493296, 0.605484299910907,
	0.612065599865624, 0.61868572749878, 0.625344720802427, 0.632042617620641, 0.638779455650817, 0.645555272444934, 0.652370105410821, 0.659223991813387, 0.666116968775851,
	0.673049073280942, 0.680020342172095, 0.687030812154625, 0.694080519796882, 0.701169501531402, 0.708297793656032, 0.715465432335048, 0.722672453600255, 0.729918893352071,
	0.737204787360605, 0.744530171266715, 0.751895080583051, 0.759299550695091, 0.766743616862161, 0.774227314218442, 0.781750677773962, 0.789313742415586, 0.796916542907978,
	0.804559113894567, 0.81224148989849, 0.819963705323528, 0.827725794455034, 0.835527791460841, 0.843369730392169, 0.851251645184515, 0.859173569658532, 0.867135537520905,
	0.875137582365205, 0.883179737672745, 0.891262036813419, 0.899384513046529, 0.907547199521614, 0.915750129279253, 0.923993335251873, 0.932276850264543, 0.940600707035753,
	0.948964938178195, 0.957369576199527, 0.96581465350313, 0.974300202388861, 0.982826255053791, 0.99139284359294, 1
};
#ifdef WIN32
#pragma warning( pop ) 
#endif

LinearColor& ParaEngine::LinearColor::operator=(uint32 color)
{
	*this = Color(color).ReinterpretAsLinear();
	return *this;
}

LinearColor& ParaEngine::LinearColor::operator=(const Color& color)
{
	*this = color.ReinterpretAsLinear();
	return *this;
}

ParaEngine::LinearColor::LinearColor(uint32 color)
{
	*this = Color(color).ReinterpretAsLinear();
}

ParaEngine::LinearColor::LinearColor(const float * colors)
{
	r = colors[0];
	g = colors[1];
	b = colors[2];
	a = colors[3];
}

ParaEngine::LinearColor::operator uint32() const 
{
	return (uint32)ToColor().DWColor();
}

ParaEngine::Vector3 ParaEngine::LinearColor::ToVector3() const
{
	return Vector3(r, g, b);
}


#ifdef __D3DX9_H__
ParaEngine::LinearColor::LinearColor(const D3DCOLORVALUE& color)
:r(color.r), g(color.g), b(color.b), a(color.a)
{
}
ParaEngine::LinearColor::operator const D3DCOLORVALUE&() const 
{
	return reinterpret_cast<const D3DCOLORVALUE&>(*this);
}
ParaEngine::LinearColor::operator D3DCOLORVALUE& ()
{
	return reinterpret_cast<D3DCOLORVALUE&>(*this);
}

ParaEngine::LinearColor::operator const float*() const
{
	return reinterpret_cast<const float*>(this);
}

#endif		
