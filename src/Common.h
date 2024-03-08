#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <functional>
#include "util.hpp"

class GameObject;

namespace cocos2d
{
	class CCLayer;
}

namespace Common
{
	inline std::vector<std::pair<std::function<void(GameObject*)>, std::string>> sectionLoopFunctions;

	inline bool iconsLoaded = false;
	inline bool isCheating = false;

	void calculateFramerate();
	void setPriority();
	void onAudioSpeedChange();
	void onAudioPitchChange();
	void saveIcons();
	void loadIcons();

	void uncompleteLevel();

	float getTPS();

	void updateCheating();
};
