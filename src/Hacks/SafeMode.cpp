#include "SafeMode.h"

#include <Geode/modify/EndLevelLayer.hpp>

#include <Geode/Utils.hpp>
#include <Geode/platform/windows.hpp>

#include <Geode/Geode.hpp>

#include "../util.hpp"
#include "../Settings.hpp"

#include "Common.h"

using namespace geode::prelude;
using namespace SafeMode;

class $modify(EndLevelLayer)
{
	void customSetup()
	{
		EndLevelLayer::customSetup();

		if (
			!Settings::get<bool>("level/safe_mode/enabled") ||
			!Settings::get<bool>("level/safe_mode/endscreen_enabled")
			) return;

		auto layer = reinterpret_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));
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
};

void SafeMode::updateAuto()
{
	if (Settings::get<bool>("level/safe_mode/auto", false))
		Mod::get()->setSavedValue<bool>("level/safe_mode/enabled", Common::isCheating);

	updateState();
}

void SafeMode::updateState()
{
	for (auto& patch : patches)
	{
		if (Settings::get<bool>("level/safe_mode/enabled"))
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

$execute
{
	for (std::size_t i = 0; i < PATCHES_SIZE; i++)
	{
		patches[i] = Mod::get()->patch(reinterpret_cast<void*>(base::get() + std::get<0>(opcodes[i])), std::get<1>(opcodes[i])).unwrap();
		patches[i]->disable();
	}
}
