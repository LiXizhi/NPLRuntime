// Author: LiXizhi
// Desc: 2007/3
// we simulate vegatation animation, as follows
// Distort each mesh vertex according to wave function whose amplitude is a linear function of vertex height in model space.
// the wave function in X direction: WaveHeight = Amplitude*sin((2Pi*frequency)*time+2Pi/WaveLength*X), where Amplitude = MAGNITUDE*V(height).V(height) is vertex up(Y) component in model space.
// The new x component of a vertex is: Xnew = X+A*sin(2Pi*f*T + 2Pi/L*X)--> d(Xnew) = [1+A*2Pi/L*cos(2Pi*f*T + 2Pi/L*X)]*d(X)
// Hence we must ensure that |A*2Pi/L|<1 in order for d(X) and d(Xnew) having the same sigh at any position along the X. In other words, |L|>|2Pi*A|
// Note: (1) X should be in world space, because the world space origin may change at integer values, The WaveLenth must also be integers, such as 1,2,3 meters
//       (2) View clipping on CPU must be slightly bigger.
// TODO: use instancing if we assume that A, L, f are constant for each model. For working parameters I tested are A=0.1meter, L=6 meters, anglevelocity=2Pi*f=1
// Generally wave length L should be three times larger than tree trunck. 

// the highest vertex will sway this meters.	
#define	VEGETATION_WAVE_MAGNITUDE  0.04
// this is usually the maximum model height in model space. This may be set from model file.
#define VEGETATION_MAX_HEIGHT	4
// here is the wave function in Y direction: WaveHeight = Amplitude*sin((2Pi*frequency)*time+2Pi/WaveLength*X)
#define VEGETATION_WAVE_LENGTH	6
