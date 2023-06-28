#include "utils.h"

int utils::randomInt(int min, int max)
{
	static std::random_device device;
	static std::mt19937 generator(device());

	std::uniform_int_distribution<int> distribution(min, max);

	return distribution(generator);
}
