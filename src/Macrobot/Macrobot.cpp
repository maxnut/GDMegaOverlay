#include "Macrobot.h"

#include "../Common.h"
#include "../ConstData.h"
#include "../GUI/GUI.h"
#include "../Settings.h"
#include "../utils.hpp"

#include <MinHook.h>
#include <fstream>
#include <imgui.h>
#include <iostream>
#include <unordered_map>

std::unordered_map<void*, Macrobot::CheckpointData> checkpoints;

void Macrobot::recordAction(int key, double frame, bool press, bool player1)
{
	Action ac;
	ac.key = key;
	ac.frame = frame;
	ac.press = press;
	ac.player1 = player1;

	actions.push_back(ac);
}

bool __fastcall Macrobot::playerObjectPushButtonHook(void* self, void*, int btn)
{
	int res = playerObjectPushButton(self, btn);
	if (playerObject1 && playerMode == 1 && frame != 9999999999)
	{
		recordAction(btn, frame, true, self == playerObject1);
		int correctionType = Settings::get<int>("macrobot/corrections", 0);
		if (correctionType == 1)
		{
			Correction c;
			c.frame = frame;
			c.player1 = self == playerObject1;
			c.checkpoint.fromPlayer((cocos2d::CCNode*)self);
			corrections.push_back(c);
		}
	}
	return res;
}

bool __fastcall Macrobot::playerObjectReleaseButtonHook(void* self, void*, int btn)
{
	int res = playerObjectReleaseButton(self, btn);
	if (playerObject1 && playerMode == 1 && frame != 9999999999)
	{
		recordAction(btn, frame, false, self == playerObject1);
		int correctionType = Settings::get<int>("macrobot/corrections", 0);
		if (correctionType == 1)
		{
			Correction c;
			c.frame = frame;
			c.player1 = self == playerObject1;
			c.checkpoint.fromPlayer((cocos2d::CCNode*)self);
			corrections.push_back(c);
		}
	}
	return res;
}

void* __fastcall Macrobot::checkpointObjectInitHook(void* self, void*)
{
	if (frame > 0 && playerObject1 && *((double*)Common::getBGL() + 1412) > 0)
	{
		CheckpointData data;
		data.time = *((double*)Common::getBGL() + 1412);
		data.p1.fromPlayer(playerObject1);
		data.p2.fromPlayer(playerObject2);

		checkpoints[self] = data;
	}
	return checkpointObjectInit(self);
}

void Macrobot::PlayerCheckpoint::fromPlayer(cocos2d::CCNode* player)
{
	// playerObject + 2280 isplatformer
	// playerObject + 2160 xVelPlatformer
	cocos2d::CCPoint position = MBO(cocos2d::CCPoint, player, 1068);
	this->yVel = MBO(double, player, 1944);
	this->rotation = player->getRotation();
	this->xVel = MBO(double, player, 2160);
	this->xPos = position.x;
	this->yPos = position.y;
	this->nodeXPos = player->getPositionX();
	this->nodeYPos = player->getPositionY();
	this->rotationRate = MBO(float, player, 1496);

	//dont ask
	for (int i = 0; i < 2265; i++)
		this->randomProperties[i] = MBO(float, player, 160 + i);
}

void Macrobot::PlayerCheckpoint::apply(cocos2d::CCNode* player)
{
	if(frame <= 0)
		return;
	
	*reinterpret_cast<double*>(reinterpret_cast<uintptr_t>(player) + 1944) = this->yVel;
	player->setRotation(this->rotation);

	player->setPositionX(this->nodeXPos);
	player->setPositionY(this->nodeYPos);

	*reinterpret_cast<cocos2d::CCPoint*>(reinterpret_cast<uintptr_t>(player) + 1068) =
		cocos2d::CCPoint(this->xPos, this->yPos);

	if (MBO(bool, player, 2280))
		*reinterpret_cast<double*>(reinterpret_cast<uintptr_t>(player) + 2160) = this->xVel;

	*reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(player) + 1496) = this->rotationRate;

	//🗣️ 🔥 🗣️ 🔥 🗣️ 🔥 🗣️ 🔥 🗣️ 🔥 🗣️ 🔥
	//no but seriously this has no right of working so well
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

	//1350 - 1410
}

void __fastcall Macrobot::playLayerLoadFromCheckpointHook(void* self, void*, void* checkpoint)
{
	if (playerMode != -1 && playerObject1)
	{
		CheckpointData checkpointData = checkpoints[checkpoint];

		const auto check = [&](const Action& action) -> bool { return action.frame >= checkpointData.time; };
		actions.erase(std::remove_if(actions.begin(), actions.end(), check), actions.end());
		const auto check2 = [&](const Correction& correction) -> bool {
			return correction.frame >= checkpointData.time;
		};
		corrections.erase(std::remove_if(corrections.begin(), corrections.end(), check2), corrections.end());

		playLayerLoadFromCheckpoint(self, checkpoint);

		*((double*)Common::getBGL() + 1412) = checkpointData.time;
		frame = checkpointData.time;

		checkpointData.p1.apply(playerObject1);
		checkpointData.p2.apply(playerObject2);

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
	if (playerMode != -1 && playerObject1)
	{
		double currentTime = *((double*)Common::getBGL() + 1412);
		frame = currentTime;

		if (playerMode == 0 && actions.size() > 0 && actionIndex < actions.size() &&
			frame >= actions[actionIndex].frame)
		{
			do
			{
				Action& ac = actions[actionIndex];
				if (ac.press)
					reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*, bool, int, bool)>(utils::gd_base + 0x1B59A0)(Common::getBGL(), true, ac.key, ac.player1);
				else
					reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*, bool, int, bool)>(utils::gd_base + 0x1B59A0)(Common::getBGL(), false, ac.key, ac.player1);

				actionIndex++;
			} while (actionIndex < actions.size() && frame >= actions[actionIndex].frame);
		}

		int correctionType = Settings::get<int>("macrobot/corrections", 0);

		if (correctionType > 0 && playerMode == 0 && corrections.size() > 0 && correctionIndex < corrections.size() &&
			frame >= corrections[correctionIndex].frame)
		{
			do
			{
				Correction& co = corrections[correctionIndex];
				co.checkpoint.apply(co.player1 ? playerObject1 : playerObject2);

				correctionIndex++;
			} while (correctionIndex < corrections.size() && frame >= corrections[correctionIndex].frame);
		}
	}

	return GJBaseGameLayerUpdate(self, dt);
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
		frame = 9999999999;

		int res = playLayerResetLevel(self);

		if (frame == 9999999999)
		{
			*((double*)Common::getBGL() + 1412) = 0.0;
			frame = 0;
			if (playerMode == 1)
			{
				checkpoints.clear();
				actions.clear();
				corrections.clear();
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
	//return false;
}

void Macrobot::save(std::string file)
{
	if (!std::filesystem::exists("GDMO\\macros"))
		std::filesystem::create_directory("GDMO\\macros");

	std::ofstream f("GDMO\\macros\\" + file + ".macro", std::ios::binary);

	size_t size = actions.size();
	f.write((const char*)&size, sizeof(size_t));
	for (Action& ac : actions)
		f.write((const char*)&ac, sizeof(Action));
	size = corrections.size();
	f.write((const char*)&size, sizeof(size_t));
	for (Correction& co : corrections)
		f.write((const char*)&co, sizeof(Correction));
}

void Macrobot::load(std::string file)
{
	if (!std::filesystem::exists("GDMO\\macros"))
		std::filesystem::create_directory("GDMO\\macros");

	actions.clear();
	corrections.clear();

	std::ifstream f("GDMO\\macros\\" + file + ".macro", std::ios::binary);
	size_t size = 0;
	f.read((char*)&size, sizeof(size_t));
	for (size_t i = 0; i < size; i++)
	{
		Action ac;
		f.read((char*)&ac, sizeof(Action));
		actions.push_back(ac);
	}
	size_t size_c = 0;
	f.read((char*)&size_c, sizeof(size_t));
	for (size_t i = 0; i < size_c; i++)
	{
		Correction co;
		f.read((char*)&co, sizeof(Correction));
		corrections.push_back(co);
	}
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
				reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*)>(utils::gd_base + 0x2E8200)(Common::getBGL());
		}
		if (ImGui::RadioButton("Play", &Macrobot::playerMode, 0))
		{
			Common::calculateFramerate();
		}

		ImGui::PushItemWidth(80);
		ImGui::InputText("Macro Name", macroName, 50);
		ImGui::PopItemWidth();

		if (GUI::button("Save##macro"))
			save(macroName);
		ImGui::SameLine();
		if (GUI::button("Load##macro"))
			load(macroName);

		int corrections = Settings::get<int>("macrobot/corrections", 0);

		if (GUI::combo("Corrections", &corrections, correctionType, 2))
			Settings::set<int>("macrobot/corrections", corrections);

		GUI::marker("[INFO]", "The bot is very likely to break without corrections. They're recommended for the time "
							  "being, until it becomes better");
	}
}

void Macrobot::initHooks()
{
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2D0060), playerObjectPushButtonHook,
				  reinterpret_cast<void**>(&playerObjectPushButton));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2D02A0), playerObjectReleaseButtonHook,
				  reinterpret_cast<void**>(&playerObjectReleaseButton));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2D8060), playerObjectLoadFromCheckpointHook,
				  reinterpret_cast<void**>(&playerObjectLoadFromCheckpoint));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x1BA700), GJBaseGameLayerUpdateHook,
				  reinterpret_cast<void**>(&GJBaseGameLayerUpdate));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2E8200), playLayerResetLevelHook,
				  reinterpret_cast<void**>(&playLayerResetLevel));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2E66C0), playLayerLoadFromCheckpointHook,
				  reinterpret_cast<void**>(&playLayerLoadFromCheckpoint));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2D9BC0), checkpointObjectInitHook,
				  reinterpret_cast<void**>(&checkpointObjectInit));
}