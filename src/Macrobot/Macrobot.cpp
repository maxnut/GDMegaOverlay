#include "../Common.h"
#include "../ConstData.h"
#include "../GUI/GUI.h"
#include "../JsonHacks/JsonHacks.h"
#include "../Settings.h"
#include "../utils.hpp"
#include "AudioRecord.h"
#include "types/GJGameLevel.hpp"

#include <MinHook.h>
#include <fstream>
#include <imgui.h>
#include <iostream>

#include "Macrobot.h"

Macrobot::Action* Macrobot::recordAction(int key, double frame, bool press, bool player1)
{
	Action ac(frame, key, !player1, press);

	macro.inputs.push_back(ac);

	return &macro.inputs[macro.inputs.size() - 1];
}

bool __fastcall Macrobot::playerObjectPushButtonHook(void* self, void*, int btn)
{
	int res = playerObjectPushButton(self, btn);
	if (playerObject1 && playerMode == 1 && time != 9999999999)
	{
		Action* ac = recordAction(btn, time, true, self == playerObject1);

		Correction c;
		c.time = time;
		c.player2 = self == playerObject2;
		c.checkpoint.fromPlayer((cocos2d::CCNode*)self, false);
		ac->correction = c;
	}
	return res;
}

bool __fastcall Macrobot::playerObjectReleaseButtonHook(void* self, void*, int btn)
{
	int res = playerObjectReleaseButton(self, btn);
	if (playerObject1 && playerMode == 1 && time != 9999999999)
	{
		if (btn == 3 && (ImGui::IsKeyDown(ImGuiKey_RightArrow) || ImGui::IsKeyDown(ImGuiKey_D)))
			return res;

		if (btn == 2 && (ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_A)))
			return res;

		Action* ac = recordAction(btn, time, false, self == playerObject1);

		Correction c;
		c.time = time;
		c.player2 = self == playerObject2;
		c.checkpoint.fromPlayer((cocos2d::CCNode*)self, false);
		ac->correction = c;
	}
	return res;
}

void* __fastcall Macrobot::checkpointObjectInitHook(void* self, void*)
{
	if (time > 0 && playerObject1 && *((double*)Common::getBGL() + 1412) > 0)
	{
		CheckpointData data;
		data.time = *((double*)Common::getBGL() + 1412);
		data.p1.fromPlayer(playerObject1, true);
		data.p2.fromPlayer(playerObject2, true);

		checkpoints[self] = data;
	}
	return checkpointObjectInit(self);
}

void Macrobot::PlayerCheckpoint::fromPlayer(cocos2d::CCNode* player, bool fullCapture)
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

void Macrobot::PlayerCheckpoint::apply(cocos2d::CCNode* player, bool fullRestore)
{
	if (time <= 0)
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

void __fastcall Macrobot::playLayerLoadFromCheckpointHook(void* self, void*, void* checkpoint)
{
	if (playerMode != -1 && playerObject1)
	{
		CheckpointData checkpointData = checkpoints[checkpoint];

		const auto check = [&](const Action& action) -> bool { return action.time >= checkpointData.time; };
		macro.inputs.erase(std::remove_if(macro.inputs.begin(), macro.inputs.end(), check), macro.inputs.end());

		playLayerLoadFromCheckpoint(self, checkpoint);

		*((double*)Common::getBGL() + 1412) = checkpointData.time;
		time = checkpointData.time;

		checkpointData.p1.apply(playerObject1, true);
		checkpointData.p2.apply(playerObject2, true);

		if (playerMode == 1)
		{
			playerObjectReleaseButtonHook(playerObject1, 0, 1);
			playerObjectReleaseButtonHook(playerObject2, 0, 1);
		}
	}
	else
		playLayerLoadFromCheckpoint(self, checkpoint);
}

void __fastcall Macrobot::GJBaseGameLayerUpdateHook(void* self, void*, float dt)
{
	GJBaseGameLayerUpdate(self, dt);

	if (playerMode != -1 && playerObject1)
	{
		double currentTime = *((double*)Common::getBGL() + 1412);
		time = currentTime;

		if (AudioRecord::recording && !JsonHacks::player["mods"][0]["toggle"].get<bool>())
		{
			JsonHacks::player["mods"][0]["toggle"] = true;
			JsonHacks::toggleHack(JsonHacks::player, 0, false);
		}

		int correctionType = Settings::get<int>("macrobot/corrections", 0);
		if (AudioRecord::recording)
			correctionType = 1;

		if (playerMode == 0 && macro.inputs.size() > 0 && actionIndex < macro.inputs.size() &&
			time >= macro.inputs[actionIndex].time)
		{
			do
			{
				Action& ac = macro.inputs[actionIndex];
				if (ac.down)
					reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*, bool, int, bool)>(utils::gd_base + 0x1B69F0)(
						Common::getBGL(), true, ac.button, !ac.player2);
				else
					reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*, bool, int, bool)>(utils::gd_base + 0x1B69F0)(
						Common::getBGL(), false, ac.button, !ac.player2);

				if (correctionType > 0 && ac.correction.has_value())
				{
					Correction& co = ac.correction.value();
					co.checkpoint.apply(co.player2 ? playerObject2 : playerObject1, false);
				}

				actionIndex++;
			} while (actionIndex < macro.inputs.size() && time >= macro.inputs[actionIndex].time);
		}
	}
}

int __fastcall Macrobot::playLayerResetLevelHook(void* self, void*)
{
	if (playerMode != -1)
	{
		framerate = MBO(float, Common::gameManager, 900);
		playerObject1 = MBO(cocos2d::CCNode*, self, 2168);
		playerObject2 = MBO(cocos2d::CCNode*, self, 2172);
		actionIndex = 0;
		correctionIndex = 0;
		time = 9999999999;

		int res = playLayerResetLevel(self);

		if (time == 9999999999)
		{
			*((double*)Common::getBGL() + 1412) = 0.0;
			time = 0;
			if (playerMode == 1)
			{
				checkpoints.clear();
				macro.inputs.clear();
			}
		}

		if (MBO(bool, playerObject1, 2280))
		{
			if ((ImGui::IsKeyDown(ImGuiKey_RightArrow) || ImGui::IsKeyDown(ImGuiKey_D)) &&
				!(ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_A)))
				playerObjectPushButtonHook(playerObject1, 0, 3);
			else if (!(ImGui::IsKeyDown(ImGuiKey_RightArrow) || ImGui::IsKeyDown(ImGuiKey_D)) &&
					 (ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_A)))
				playerObjectPushButtonHook(playerObject1, 0, 2);
		}

		return res;
	}
	else
		return playLayerResetLevel(self);
}

bool __fastcall Macrobot::playerObjectLoadFromCheckpointHook(void* self, void*, void* checkpoint)
{
	return playerObjectLoadFromCheckpoint(self, checkpoint);
	// return false;
}

void Macrobot::save(std::string file)
{
	if (!std::filesystem::exists("GDMO\\macros"))
		std::filesystem::create_directory("GDMO\\macros");

	std::ofstream f("GDMO\\macros\\" + file + ".gdr", std::ios::binary);

	std::string playerName = MBO(std::string, Common::gjAccountManager, 0x10C);
	gd::GJGameLevel* level = MBO(gd::GJGameLevel*, Common::getBGL(), 1504);

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
	if (!std::filesystem::exists("GDMO\\macros"))
		std::filesystem::create_directory("GDMO\\macros");

	std::ifstream f("GDMO\\macros\\" + file + ".gdr", std::ios::binary);

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
			if (Common::getBGL())
				reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*)>(utils::gd_base + 0x2EA130)(Common::getBGL());
		}
		if (ImGui::RadioButton("Play", &Macrobot::playerMode, 0))
			Common::calculateFramerate();

		ImGui::PushItemWidth(80);
		GUI::inputText("Macro Name", &macroName);
		GUI::inputText("Macro Description", &macroDescription);
		ImGui::PopItemWidth();

		if (GUI::button("Save##macro"))
			save(macroName);
		ImGui::SameLine();
		if (GUI::button("Load##macro"))
			load(macroName);

		int corrections = Settings::get<int>("macrobot/corrections", 0);

		if (GUI::combo("Corrections", &corrections, correctionType, 2))
			Settings::set<int>("macrobot/corrections", corrections);

		GUI::marker("[INFO]", "Corrections are recommended to be safe, but the bot also works decently without.");
	}
}

void Macrobot::initHooks()
{
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2D1D30), playerObjectPushButtonHook,
				  reinterpret_cast<void**>(&playerObjectPushButton));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2D1F70), playerObjectReleaseButtonHook,
				  reinterpret_cast<void**>(&playerObjectReleaseButton));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2D9E60), playerObjectLoadFromCheckpointHook,
				  reinterpret_cast<void**>(&playerObjectLoadFromCheckpoint));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x1BB780), GJBaseGameLayerUpdateHook,
				  reinterpret_cast<void**>(&GJBaseGameLayerUpdate));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2EA130), playLayerResetLevelHook,
				  reinterpret_cast<void**>(&playLayerResetLevel));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2E85E0), playLayerLoadFromCheckpointHook,
				  reinterpret_cast<void**>(&playLayerLoadFromCheckpoint));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2DB9F0), checkpointObjectInitHook,
				  reinterpret_cast<void**>(&checkpointObjectInit));
}