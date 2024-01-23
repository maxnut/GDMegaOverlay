#include "SafeMode.h"

#include <Geode/modify/EndLevelLayer.hpp>

#include <Geode/Utils.hpp>
#include <Geode/platform/windows.hpp>

#include <Geode/Geode.hpp>

#include "../util.hpp"

using namespace geode::prelude;
using namespace SafeMode;

void SafeMode::updateState()
{
	for (auto& patch : patches)
	{
		if (Mod::get()->getSavedValue<bool>("level/safe_mode/enabled"))
		{
			if (!patch->isEnabled())
				patch->enable();
		}
		else
		{
			if (patch->isEnabled())
				patch->disable();
		}
	}
}

void SafeMode::endLevelLayerCustomSetupHook(CCLayer* self)
{
	reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*)>(geode::base::get() + 0xE74F0)(self);

	if (
		!Mod::get()->getSavedValue<bool>("level/safe_mode/enabled") ||
		!Mod::get()->getSavedValue<bool>("level/safe_mode/endscreen_enabled")
		) return;

	auto layer = getChildOfType<CCLayer>(self, 0);
	CCLabelBMFont* endScreenMessageLabel = nullptr;
	TextArea* endScreenTextArea = nullptr;


	CCObject* object;
	CCARRAY_FOREACH(layer->getChildren(), object)
	{
		if (auto textArea = dynamic_cast<TextArea*>(object); textArea)
			endScreenTextArea = textArea;
		else if (
			auto messageLabel = dynamic_cast<CCLabelBMFont*>(object);
			messageLabel
		) {
			std::string labelText = messageLabel->getString();

			if (
				!labelText.starts_with("Attempts: ") && !labelText.starts_with("Jumps: ") &&
				!labelText.starts_with("Time: ") && !labelText.starts_with("Points: ")
				)
				endScreenMessageLabel = messageLabel;
		}
	}

	// TODO: Create a CCLabelBMFont if both of these are false

	if (endScreenTextArea)
		endScreenTextArea->setString("- Safe Mode -");

	if (endScreenMessageLabel)
		endScreenMessageLabel->setString("- Safe Mode -");
}

$execute
{
	for (std::size_t i = 0; i < PATCHES_SIZE; i++)
	{
		patches[i] = Mod::get()->patch(reinterpret_cast<void*>(base::get() + std::get<0>(opcodes[i])), std::get<1>(opcodes[i])).unwrap();
		patches[i]->disable();
	}

	Mod::get()->hook(reinterpret_cast<void*>(geode::base::get() + 0xE74F0), &endLevelLayerCustomSetupHook, "EndLevelLayer::customSetup", tulip::hook::TulipConvention::Thiscall);
}
