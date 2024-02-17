#pragma once

#include <vector>

#include <Geode/modify/StartPosObject.hpp>

namespace StartposSwitcher
{
	inline std::vector<StartPosObject*> startposObjects;
	inline int index = -1;
	inline cocos2d::CCLabelBMFont* startPosLabel = nullptr;

	void change(bool right);
	void showLabel();
}
