#ifndef __asm_math_H__
#define __asm_math_H__

namespace ParaEngine
{

/*=============================================================================
 ASM math routines posted by davepermen et al on flipcode forums
=============================================================================*/
const float pi = 4.0f * atan( 1.0f );
const float half_pi = 0.5f * pi;


float asm_arccos( float r ) {
    // return half_pi + arctan( r / -sqr( 1.f - r * r ) );
	
	return float( acos( r ) );

}

float asm_arcsin( float r ) {
    // return arctan( r / sqr( 1.f - r * r ) );

	return float( asin( r ) );

}

float asm_arctan( float r ) {
	return float( atan( r ) );

}

float asm_sin( float r ) {

	return sin( r );

}

float asm_cos( float r ) {

	return cos( r );
}

float asm_tan( float r ) {

	return tan( r );
}

// returns a for a * a = r
float asm_sqrt( float r )
{
	return sqrt( r );
}

// returns 1 / a for a * a = r
// -- Use this for Vector normalisation!!!
float asm_rsq( float r )
{
	return 1.f / sqrt( r );
}

// returns 1 / a for a * a = r
// Another version
float apx_rsq( float r ) {

	return 1.f / sqrt( r );
}

// returns a random number
__inline float asm_rand()
{
	return float( rand() );

}

// returns the maximum random number
__inline float asm_rand_max()
{
    // GCC etc
	return float( RAND_MAX );
}

// returns log2( r ) / log2( e )
float asm_ln( float r ) {    

	return log( r );

}


} // namespace
#endif
