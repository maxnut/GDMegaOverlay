#include "../Common.h"
#include "../ConstData.h"
#include "../GUI/GUI.h"
#include "../JsonHacks/JsonHacks.h"

#include "../util.hpp"
#include "AudioRecord.h"

#include <fstream>
#include <imgui.h>
#include <iostream>

#include "Macrobot.h"

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GameManager.hpp>

using namespace geode::prelude;
using namespace Macrobot;

class $modify(PlayLayer)
{
	void loadFromCheckpoint(CheckpointObject* checkpoint)
	{
		if (playerMode != -1 && playerObject1)
		{
			CheckpointData checkpointData = checkpoints[checkpoint];
			const auto check = [&](const Action& action) -> bool { return action.time >= checkpointData.time; };
			macro.inputs.erase(std::remove_if(macro.inputs.begin(), macro.inputs.end(), check), macro.inputs.end());
			PlayLayer::loadFromCheckpoint(checkpoint);
			*((double*)GameManager::get()->getPlayLayer() + 1412) = checkpointData.time;
			gameTime = checkpointData.time;
			checkpointData.p1.apply(playerObject1, true);
			checkpointData.p2.apply(playerObject2, true);
			if (playerMode == 1)
			{
				playerObject1->releaseButton(PlayerButton::Jump);
				playerObject2->releaseButton(PlayerButton::Jump);
			}
		}
		else
			PlayLayer::loadFromCheckpoint(checkpoint);
	}

	void resetLevel()
	{
		if (playerMode != -1)
		{
			framerate = GameManager::get()->m_customFPSTarget;
			playerObject1 = this->m_player1;
			playerObject2 = this->m_player2;
			actionIndex = 0;
			correctionIndex = 0;
			gameTime = 9999999999;

			PlayLayer::resetLevel();

			if (gameTime == 9999999999)
			{
				*((double*)GameManager::get()->getPlayLayer() + 1412) = 0.0;
				gameTime = 0;
				if (playerMode == 1)
				{
					checkpoints.clear();
					macro.inputs.clear();
				}
			}

			if (playerObject1->m_isPlatformer)
			{
				if ((ImGui::IsKeyDown(ImGuiKey_RightArrow) || ImGui::IsKeyDown(ImGuiKey_D)) &&
					!(ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_A)))
					playerObject1->pushButton(PlayerButton::Right);
				else if (!(ImGui::IsKeyDown(ImGuiKey_RightArrow) || ImGui::IsKeyDown(ImGuiKey_D)) &&
						(ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_A)))
					playerObject1->pushButton(PlayerButton::Left);
			}
		}
		else
			PlayLayer::resetLevel();
	}
};

class $modify(GJBaseGameLayer)
{
	void update(float dt)
	{
		GJBaseGameLayer::update(dt);
		if (playerMode != -1 && playerObject1)
		{
			double currentTime = *((double*)GameManager::get()->getPlayLayer() + 1412);
			gameTime = currentTime;

			if (AudioRecord::recording && !JsonHacks::player["mods"][0]["toggle"].get<bool>())
			{
				JsonHacks::player["mods"][0]["toggle"] = true;
				JsonHacks::toggleHack(JsonHacks::player, 0, false);
			}

			int correctionType = Mod::get()->getSavedValue<int>("macrobot/corrections", 0);
			if (AudioRecord::recording)
				correctionType = 1;

			if (playerMode == 0 && macro.inputs.size() > 0 && actionIndex < macro.inputs.size() &&
				gameTime >= macro.inputs[actionIndex].time)
			{
				do
				{
					Action& ac = macro.inputs[actionIndex];
					if (ac.down)
						reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*, bool, int, bool)>(util::gd_base + 0x1B69F0)(
							GameManager::get()->getPlayLayer(), true, ac.button, !ac.player2);
					else
						reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*, bool, int, bool)>(util::gd_base + 0x1B69F0)(
							GameManager::get()->getPlayLayer(), false, ac.button, !ac.player2);

					if (correctionType > 0 && ac.correction.has_value())
					{
						Correction& co = ac.correction.value();
						co.checkpoint.apply(co.player2 ? playerObject2 : playerObject1, false);
					}

					actionIndex++;
				} while (actionIndex < macro.inputs.size() && gameTime >= macro.inputs[actionIndex].time);
			}
		}
	}
};

class $modify(PlayerObject)
{
	void pushButton(PlayerButton btn)
	{
		PlayerObject::pushButton(btn);

		if(!GameManager::get()->getPlayLayer())
			return;

		if (playerObject1 && playerMode == 1 && gameTime != 9999999999)
		{
			Action* ac = recordAction(btn, gameTime, true, this == playerObject1);

			Correction c;
			c.time = gameTime;
			c.player2 = this == playerObject2;
			c.checkpoint.fromPlayer(this, false);
			ac->correction = c;
		}
	}

	void releaseButton(PlayerButton btn)
	{
		PlayerObject::releaseButton(btn);

		if(!GameManager::get()->getPlayLayer())
			return;

		if (playerObject1 && playerMode == 1 && gameTime != 9999999999)
		{
			if (btn == PlayerButton::Right && (ImGui::IsKeyDown(ImGuiKey_RightArrow) || ImGui::IsKeyDown(ImGuiKey_D)))
				return;

			if (btn == PlayerButton::Left && (ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_A)))
				return;

			Action* ac = recordAction(btn, gameTime, false, this == playerObject1);

			Correction c;
			c.time = gameTime;
			c.player2 = this == playerObject2;
			c.checkpoint.fromPlayer(this, false);
			ac->correction = c;
		}
	}
};

Macrobot::Action* Macrobot::recordAction(PlayerButton key, double frame, bool press, bool player1)
{
	Action ac(frame, (int)key, !player1, press);

	macro.inputs.push_back(ac);

	return &macro.inputs[macro.inputs.size() - 1];
}

void* Macrobot::checkpointObjectCreateHook(void* self)
{
	if (gameTime > 0 && playerObject1 && MBO(double, GameManager::get()->getPlayLayer(), 0x584) > 0)
	{
		CheckpointData data;
		data.time = MBO(double, GameManager::get()->getPlayLayer(), 0x584);
		data.p1.fromPlayer(playerObject1, true);
		data.p2.fromPlayer(playerObject2, true);

		checkpoints[self] = data;
	}
	return reinterpret_cast<void*(__thiscall*)()>(util::gd_base + 0x2EB9A0)();
}

$execute
{
	Mod::get()->hook(reinterpret_cast<void*>(util::gd_base + 0x2EB9A0), &checkpointObjectCreateHook, "CheckpointObject::create", tulip::hook::TulipConvention::Optcall);
}

void Macrobot::PlayerCheckpoint::fromPlayer(PlayerObject* player, bool fullCapture)
{
	// playerObject + 2280 isplatformer
	// playerObject + 2160 xVelPlatformer
	cocos2d::CCPoint position = MBO(cocos2d::CCPoint, player, 2132);
	this->yVel = MBO(double, player, 1936);
	this->rotation = player->getRotation();
	this->xVel = MBO(double, player, 2192);
	this->xPos = position.x;
	this->yPos = position.y;
	this->nodeXPos = player->getPositionX();
	this->nodeYPos = player->getPositionY();
	this->rotationRate = MBO(float, player, 1496);

	if (fullCapture)
	{
		// dont ask
		for (int i = 0; i < 2265; i++)
			this->randomProperties[i] = MBO(float, player, 160 + i);
	}
}

void Macrobot::PlayerCheckpoint::apply(PlayerObject* player, bool fullRestore)
{
	if (gameTime <= 0)
		return;

	*reinterpret_cast<double*>(reinterpret_cast<uintptr_t>(player) + 1936) =
		this->yVel; // get all these offsets from playerobject constructor
	player->setRotation(this->rotation);

	player->setPositionX(this->nodeXPos);
	player->setPositionY(this->nodeYPos);

	*reinterpret_cast<cocos2d::CCPoint*>(reinterpret_cast<uintptr_t>(player) + 2132) =
		cocos2d::CCPoint(this->xPos, this->yPos);

	*reinterpret_cast<double*>(reinterpret_cast<uintptr_t>(player) + 2192) = this->xVel; // playerobject_updatemove

	*reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(player) + 1496) = this->rotationRate;

	if (fullRestore)
	{
		// 🗣️ 🔥 🗣️ 🔥 🗣️ 🔥 🗣️ 🔥 🗣️ 🔥 🗣️ 🔥
		// no but seriously this has no right of working so well
		for (int i = 1410; i < 1600; i++)
		{
			if (this->randomProperties[i] < 10000 && this->randomProperties[i] > -10000)
			{
				*reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(player) + 160 + i) = this->randomProperties[i];
			}
		}

		for (int i = 1800; i < 2265; i++)
		{
			if (this->randomProperties[i] < 10000 && this->randomProperties[i] > -10000)
			{
				*reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(player) + 160 + i) = this->randomProperties[i];
			}
		}

		for (int i = 0; i < 1200; i++)
		{
			if (this->randomProperties[i] < 10000 && this->randomProperties[i] > -10000)
			{
				*reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(player) + 160 + i) = this->randomProperties[i];
			}
		}
	}

	// 1350 - 1410
}

void Macrobot::save(std::string file)
{
	std::ofstream f(Mod::get()->getSaveDir().string() + "\\macros\\" + file + ".gdr", std::ios::binary);

	std::string playerName = GJAccountManager::sharedState()->m_username;
	GJGameLevel* level = GameManager::get()->getPlayLayer()->m_level;

	macro.author = playerName;
	macro.description = macroDescription;
	macro.duration = macro.inputs[macro.inputs.size() - 1].time;
	macro.gameVersion = 2.204f;
	macro.version = 1.0f;

	macro.levelInfo.id = level->m_levelID.value();
	macro.levelInfo.name = level->m_levelName;

	auto data = macro.exportData(false);
	f.write(reinterpret_cast<const char*>(data.data()), data.size());

	f.close();
}

void Macrobot::load(std::string file)
{
	std::ifstream f(Mod::get()->getSaveDir().string() + "\\macros\\" + file + ".gdr", std::ios::binary);

	f.seekg(0, std::ios::end);
	size_t fileSize = f.tellg();
	f.seekg(0, std::ios::beg);

	std::vector<std::uint8_t> macroData(fileSize);

	f.read(reinterpret_cast<char*>(macroData.data()), fileSize);

	f.close();

	std::cout << macroData.size();

	macro = Macro::importData(macroData);
}

void Macrobot::drawWindow()
{
	if (GUI::shouldRender())
	{
		if (ImGui::RadioButton("Disable", &Macrobot::playerMode, -1))
			Common::calculateFramerate();
		if (ImGui::RadioButton("Record", &Macrobot::playerMode, 1))
		{
			Common::calculateFramerate();
			if (GameManager::get()->getPlayLayer())
				reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*)>(util::gd_base + 0x2EA130)(GameManager::get()->getPlayLayer());
		}
		if (ImGui::RadioButton("Play", &Macrobot::playerMode, 0))
			Common::calculateFramerate();

		ImGui::PushItemWidth(80);
		GUI::inputText("Macro Name", &macroName);
		GUI::inputText("Macro Description", &macroDescription);
		ImGui::PopItemWidth();

		if (GUI::button("Save##macro"))
			save(macroName);
		GUI::sameLine();
		if (GUI::button("Load##macro"))
			load(macroName);

		int corrections = Mod::get()->getSavedValue<int>("macrobot/corrections", 0);

		if (GUI::combo("Corrections", &corrections, correctionType, 2))
			Mod::get()->setSavedValue<int>("macrobot/corrections", corrections);

		GUI::marker("[INFO]", "Corrections are recommended to be safe, but the bot also works decently without.");
	}
}