#include <cocos2d.h>
#include "SafeMode.h"
#include "../Settings.h"
#include "../utils.hpp"

void SafeMode::initHooks()
{
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0xE74F0), endLevelLayerCustomSetupHook,
				  reinterpret_cast<void**>(&endLevelLayerCustomSetup));
}

void SafeMode::updateState()
{
	for (auto& elem : opcodes)
	{
		if (Settings::get<bool>("level/safe_mode/enabled"))
			utils::writeBytes(utils::gd_base + elem.address, utils::hexToBytes(elem.on), false);
		else
			utils::writeBytes(utils::gd_base + elem.address, utils::hexToBytes(elem.off), false);
	}
}

void __fastcall SafeMode::endLevelLayerCustomSetupHook(cocos2d::CCLayer* self, void*)
{
	endLevelLayerCustomSetup(self);

	if (
		!Settings::get<bool>("level/safe_mode/enabled") ||
		!Settings::get<bool>("level/safe_mode/endscreen_enabled")
		) return;

	auto layer = reinterpret_cast<cocos2d::CCLayer*>(self->getChildren()->objectAtIndex(0));
	cocos2d::CCLabelBMFont* messageLabel = nullptr;
	void* textAreaEndScreen = nullptr; 

	for (unsigned int i = 0; i < layer->getChildrenCount(); i++)
	{
		auto object = layer->getChildren()->objectAtIndex(i);

		if (utils::getClassName(object) == "TextArea")
			textAreaEndScreen = object;
		else if (utils::getClassName(object) == "cocos2d::CCLabelBMFont")
		{
			auto label = reinterpret_cast<cocos2d::CCLabelBMFont*>(object);
			std::string labelText = label->getString();

			if (!labelText.starts_with("Attempts: ") && !labelText.starts_with("Jumps: ") && !labelText.starts_with("Time: "))
				messageLabel = label;
		}
	}

	if (textAreaEndScreen)
		// TextArea::setString
		reinterpret_cast<void(__thiscall*)(void*, std::string)>(utils::gd_base + 0x52460)(textAreaEndScreen, "- Safe Mode -");

	if (messageLabel)
		messageLabel->setString("- Safe Mode -");
}
