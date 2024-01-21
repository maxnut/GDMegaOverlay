#pragma once

#include "../Common.h"
#include "../Settings.h"

#include <functional>

namespace Labels
{

class Label
{
public:
	cocos2d::CCLabelBMFont* pointer = nullptr;
	std::string settingName = "";
	std::function<void(cocos2d::CCLabelBMFont*)> function;

	void process()
	{
		if (Settings::get<bool>("labels/" + settingName + "/enabled", false))
			function(pointer);
	}
};

inline std::vector<Label> labels, tl, tr, bl, br;

inline std::vector<float> clicks;

inline size_t totalClicks = 0;
inline size_t frames = 0;
inline size_t deaths = 0;

inline float totalDelta = 0;

inline bool clickRegistered = false;
inline bool labelsCreated = false;
inline bool click = false;

inline bool dead = false;

Label setupLabel(std::string labelSettingName, const std::function<void(cocos2d::CCLabelBMFont*)> &function, cocos2d::CCLayer* playLayer);

void calculatePositions();

void settingsForLabel(std::string labelSettingName, std::function<void()>extraSettings);
void renderWindow();
} // namespace Labels