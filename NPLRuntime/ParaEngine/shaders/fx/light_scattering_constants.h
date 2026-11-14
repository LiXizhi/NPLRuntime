#ifdef WIN32
#define float4 Vector4
#endif

//
// Define a structure used by both C++ and HLSL
//
struct sLightScatteringShaderParams
{
	float4 vBeta1;
	float4 vBeta2;
	float4 vBetaD1;
	float4 vBetaD2;
	float4 vSumBeta1Beta2;
	float4 vLog2eBetaSum;
	float4 vRcpSumBeta1Beta2;
	float4 vHG;
	float4 vConstants;
	float4 vTermMultipliers;
	float4 vSoilReflectivity;
};

#ifndef WIN32
const sLightScatteringShaderParams atm : AtmosphericLightingParams;
const SCALAR4 object_vi: sunlight_inscatter;
const SCALAR4 object_ve: sunlight_extinction;

void atmosphericExtinction(
	uniform SCALAR3 eyeVector,
	uniform SCALAR3 sunVector,
	uniform SCALAR s,
	out SCALAR3 vExt)
{
	// compute cosine of theta angle
	SCALAR cosTheta = dot(eyeVector, sunVector);

	// compute extinction term vExt
	// -(beta_1+beta_2) * s * log_2 e
	vExt = -atm.vSumBeta1Beta2 * s * atm.vConstants.y;
	vExt.x = exp(vExt.x);
	vExt.y = exp(vExt.y);
	vExt.z = exp(vExt.z);

	// scale extinction prior to its use
	// (this is optional)
	//vExt = vExt*atm.vTermMultipliers.y*atm.vSoilReflectivity;

}

void atmosphericLighting(
	uniform SCALAR3 eyeVector,
	uniform SCALAR3 sunVector,
	uniform SCALAR3 norm,
	uniform SCALAR4 sunColor,
	uniform SCALAR s,
	out SCALAR4 vExt,
	out SCALAR4 vIns)
{
	//
	// This shader assumes a world-space vertex is 
	// provided, and distance values represent 
	// real-world distances. A set of pre-
	// calculated atmospheric data is provided
	// in the atm structure.
	//
	
	// compute cosine of theta angle
	SCALAR cosTheta = dot(eyeVector, sunVector);

	// compute extinction term vExt
	// -(beta_1+beta_2) * s * log_2 e
	vExt = -atm.vSumBeta1Beta2 * s * atm.vConstants.y;
	vExt.x = exp(vExt.x);
	vExt.y = exp(vExt.y);
	vExt.z = exp(vExt.z);
	vExt.w = 0.0f;

	// Compute theta terms used by inscattering.
	// compute phase2 theta as
    // (1-g^2)/(1+g-2g*cos(theta))^(3/2)
	// atm.vHG = [1-g^2, 1+g, 2g]
	SCALAR p1Theta = (cosTheta*cosTheta)+atm.vConstants.x;
	SCALAR p2Theta = (atm.vHG.z*cosTheta)+atm.vHG.y;
	p2Theta = 1.0f/(sqrt(p2Theta));
	p2Theta = (p2Theta*p2Theta*p2Theta) * atm.vHG.x;


	// compute inscattering (vIns) as
	// (vBetaD1*p1Theta + vBetaD1*p2Theta) *
	// (1-vExt) * atm.vRcpSumBeta1Beta2
	// 
	// atm.vRcpSumBeta1Beta2 = 
	// 1.0f/ (Rayleigh+Mie)
	vIns = ((atm.vBetaD1*p1Theta)+
			(atm.vBetaD2*p2Theta))
			*(atm.vConstants.x-vExt)
			*atm.vRcpSumBeta1Beta2;

	// scale inscatter and extinction 
	// for effect (optional)
	vIns = vIns*atm.vTermMultipliers.x;
	// scale extinction prior to its use
	// (this is optional)
	vExt = vExt*atm.vTermMultipliers.y*atm.vSoilReflectivity;

	// reduce inscattering on unlit surfaces
	// by modulating with a monochrome
	// Lambertian scalar. This is slightly
	// offset to allow some inscattering to
	// bleed into unlit areas
	SCALAR NdL = saturate(dot(norm, sunVector));
	vIns= vIns*NdL;

	// apply sunlight color
	// and strength to each term
	// and output
	vIns.xyz = vIns*sunColor*sunColor.w;
	vIns.w = 0.0f;

	vExt.xyz = vExt*sunColor*sunColor.w;
	vExt.w = 1.0f;
}

#endif
