#pragma once

class FastRandom
{
public:
	FastRandom(int32 seed);
	FastRandom();
	void Init(int32 seed);

	int32 randomLong();
	double randomDouble();

	float random();
	int32 random(int32 from, int32 to);

	// Returns a random value as boolean.
	bool randomBoolean();

	/** Returns a random character string with a specified length.
	@param length The length of the generated string
	@return Random character string
	*/
	std::string randomCharacterString(int length);

	// Calculates a standardized normal distributed value (using the polar method).
	double standNormalDistrDouble();

	static double randomNoise(int32 x, int32 y, int32 z, int32 seed);

	
public:
	uint32 m_seed;
};

