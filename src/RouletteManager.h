#pragma once
#include "pch.h"
#include <thread>
#include <nlohmann/json.hpp>


struct RouletteManagerStruct
{
private:
	inline static bool hasInitManager = false;

public:
	inline static std::atomic_bool isPlayingRoulette = false;
	inline static std::atomic_bool hasFinishedPreviousLevel = false;

	inline static std::array<bool, 6> difficultyArr;
	inline static std::array<bool, 5> demonDifficultyArr;
	inline static std::array<bool, 3> togglesStatesArr;

	inline static int lastLevelID = 0;
	inline static int lastLevelPercentage = 0;
	inline static int levelPercentageGoal = 1;
	
	inline static int skipsCount = 0;
	inline static int skipsMax = 3;

	RouletteManagerStruct()
	{
		if (!hasInitManager)
		{
			difficultyArr.fill(false);
			demonDifficultyArr.fill(false);
			togglesStatesArr.fill(false);

			difficultyArr[0] = true;
			demonDifficultyArr[0] = true;
			togglesStatesArr[0] = true;

			hasInitManager = true;
		}
	}
};

#ifdef INITIALIZEROULETTEMANAGER
	extern RouletteManagerStruct RouletteManager{};
#elif defined(DECLAREROULETTEMANAGER)
	extern RouletteManagerStruct RouletteManager;
#endif // !DECLAREROULETTEMANAGER
