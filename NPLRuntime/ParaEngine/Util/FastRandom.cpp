//-----------------------------------------------------------------------------
// Class: Fast random
// Authors:	LiXizhi, ported from NPL script
// Emails:	LiXizhi@yeah.net
// Date:	2014.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FastRandom.h"
#include <math.h>

using namespace ParaEngine;

#define FAST_RAND_MAX 0x7fff

FastRandom::FastRandom(int32 seed)
{
	Init(seed);
}

FastRandom::FastRandom()
{
	Init(randint(0,1000));
}

void FastRandom::Init(int32 seed)
{
	m_seed = seed;
}

int32 FastRandom::randomLong()
{
	m_seed = (214013L * m_seed + 2531011L);
	return (m_seed >> 16) & 0x7FFF;
}

double FastRandom::randomDouble()
{
	return (double)(randomLong() % FAST_RAND_MAX) / (FAST_RAND_MAX - 1);
}

float FastRandom::random()
{
	return (float)(randomLong() % FAST_RAND_MAX) / (FAST_RAND_MAX - 1);
}

int32 FastRandom::random(int32 a, int32 b)
{
	return a + (randomLong() % (b - a + 1));
}

bool FastRandom::randomBoolean()
{
	return randomLong() > 0;
}

std::string FastRandom::randomCharacterString(int length)
{
	std::string s;
	s.resize(length);
	for (int i = 0; i < length; ++i)
	{
		s[i] = 'a' + (char)random(0, 25);
	}
	return s;
}

double FastRandom::standNormalDistrDouble()
{
	double q = 1;
	double u1 = 0;
	double u2;

	while (q >= 1 || q == 0)
	{
		u1 = randomDouble();
		u2 = randomDouble();
		q = (u1 * u1) + (u2 * u2);
	}

	// for unknown reason, std::log2() is missing on android (compile fine, but crash at runtime), so we will do log(q)/log(2) here. 
	float p = Math::Sqrt((float)((-2.0 * (std::log(q) / 0.30102999566)) / q));
	return u1 * p;
}

double FastRandom::randomNoise(int32 x, int32 y, int32 z, int32 seed)
{
	uint32 u = x * 702395077 + y * 915488749 + z * 1299721 + seed * 1402024253;
	u ^= (u<<13);
	return (1.0 - ((u * (u * u * 15731 + 789221) + 1376312589)& 0x7fffffff) / 1073741824.0);
}
