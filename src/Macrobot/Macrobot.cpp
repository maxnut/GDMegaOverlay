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
	if (playerObject1)
	{
		if (playerMode == 1 && frame != 9999999999)
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
	}
	return res;
}

bool __fastcall Macrobot::playerObjectReleaseButtonHook(void* self, void*, int btn)
{
	int res = playerObjectReleaseButton(self, btn);
	if (playerObject1)
	{
		if (playerMode == 1 && frame != 9999999999)
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
	}
	return res;
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
}

void* __fastcall Macrobot::checkpointObjectInitHook(void* self, void*)
{
	if (frame > 0 && playerObject1)
	{
		CheckpointData data;
		data.time = *((double*)Common::getBGL() + 1411);
		data.p1.fromPlayer(playerObject1);
		data.p2.fromPlayer(playerObject2);

		checkpoints[self] = data;
	}
	return checkpointObjectInit(self);
}

void Macrobot::PlayerCheckpoint::apply(cocos2d::CCNode* player)
{
	*reinterpret_cast<double*>(reinterpret_cast<uintptr_t>(player) + 1944) = this->yVel;
	player->setRotation(this->rotation);

	*reinterpret_cast<cocos2d::CCPoint*>(reinterpret_cast<uintptr_t>(player) + 1068) =
		cocos2d::CCPoint(this->xPos, this->yPos);

	if (MBO(bool, player, 2280))
		*reinterpret_cast<double*>(reinterpret_cast<uintptr_t>(player) + 2160) = this->xVel;
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

		*((double*)Common::getBGL() + 1411) = checkpointData.time;
		frame = checkpointData.time;

		checkpointData.p1.apply(playerObject1);
		checkpointData.p2.apply(playerObject2);

		if (playerMode == 1)
		{
			recordAction(1, frame,
						 ImGui::IsMouseDown(0) || ImGui::IsKeyDown(ImGuiKey_UpArrow) || ImGui::IsKeyDown(ImGuiKey_W),
						 true);
			recordAction(2, frame, false, true);
			recordAction(3, frame, false, true);
			recordAction(1, frame,
						 ImGui::IsMouseDown(0) || ImGui::IsKeyDown(ImGuiKey_UpArrow) || ImGui::IsKeyDown(ImGuiKey_W),
						 false);
			recordAction(2, frame, false, false);
			recordAction(3, frame, false, false);
		}
	}
	else
		playLayerLoadFromCheckpoint(self, checkpoint);
}

void __fastcall Macrobot::GJBaseGameLayerUpdateHook(void* self, void*, float dt)
{
	if (playerMode != -1 && playerObject1)
	{
		double currentTime = *((double*)Common::getBGL() + 1411);
		frame = currentTime;

		if (playerMode == 0 && actions.size() > 0 && actionIndex < actions.size() &&
			frame >= actions[actionIndex].frame)
		{
			Action& ac = actions[actionIndex];
			if (ac.press)
				playerObjectPushButtonHook(ac.player1 ? playerObject1 : playerObject2, 0, ac.key);
			else
				playerObjectReleaseButtonHook(ac.player1 ? playerObject1 : playerObject2, 0, ac.key);

			actionIndex++;
		}

		int correctionType = Settings::get<int>("macrobot/corrections", 0);

		if (correctionType > 0 && playerMode == 0 && corrections.size() > 0 && correctionIndex < corrections.size() &&
			frame >= corrections[correctionIndex].frame)
		{
			Correction& co = corrections[correctionIndex];
			co.checkpoint.apply(co.player1 ? playerObject1 : playerObject2);

			correctionIndex++;
		}
	}

	return GJBaseGameLayerUpdate(self, dt);
}

int __fastcall Macrobot::playLayerResetLevelHook(void* self, void*)
{
	if (playerMode != -1)
	{
		framerate = MBO(float, Common::gameManager, 900);
		playerObject1 = MBO(cocos2d::CCNode*, self, 2160);
		playerObject2 = MBO(cocos2d::CCNode*, self, 2164);
		actionIndex = 0;
		correctionIndex = 0;
		frame = 9999999999;

		int res = playLayerResetLevel(self);

		if (frame == 9999999999)
		{
			*((double*)Common::getBGL() + 1411) = 0.0;
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
	// return false;
}

void Macrobot::save(std::string file)
{
	if (!std::filesystem::exists("GDMO\\macros"))
		std::filesystem::create_directory("GDMO\\macros");

	std::ofstream f("GDMO\\macros\\" + file + ".macro");
	f << actions.size();
	for (Action& ac : actions)
		f.write((const char*)&ac, sizeof(Action));
	f << corrections.size();
	for (Correction& co : corrections)
		f.write((const char*)&co, sizeof(Correction));
}

void Macrobot::load(std::string file)
{
	if (!std::filesystem::exists("GDMO\\macros"))
		std::filesystem::create_directory("GDMO\\macros");

	actions.clear();
	corrections.clear();

	std::ifstream f("GDMO\\macros\\" + file + ".macro");
	size_t size = 0;
	f >> size;
	for (size_t i = 0; i < size; i++)
	{
		Action ac;
		f.read((char*)&ac, sizeof(Action));
		actions.push_back(ac);
	}
	f >> size;
	for (size_t i = 0; i < size; i++)
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
				reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*)>(utils::gd_base + 0x2E42B0)(Common::getBGL());
		}
		if (ImGui::RadioButton("Play", &Macrobot::playerMode, 0))
		{
			Common::calculateFramerate();
			if (Common::getBGL())
				reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*)>(utils::gd_base + 0x2E42B0)(Common::getBGL());
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

		GUI::marker("[INFO]", "The bot is very likely to break without corrections. They're recommended for the time being, until it becomes better");
	}
}

void Macrobot::initHooks()
{
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2CC4D0), playerObjectPushButtonHook,
				  reinterpret_cast<void**>(&playerObjectPushButton));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2CC710), playerObjectReleaseButtonHook,
				  reinterpret_cast<void**>(&playerObjectReleaseButton));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2D4370), playerObjectLoadFromCheckpointHook,
				  reinterpret_cast<void**>(&playerObjectLoadFromCheckpoint));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x1B75E0), GJBaseGameLayerUpdateHook,
				  reinterpret_cast<void**>(&GJBaseGameLayerUpdate));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2E42B0), playLayerResetLevelHook,
				  reinterpret_cast<void**>(&playLayerResetLevel));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2E28D0), playLayerLoadFromCheckpointHook,
				  reinterpret_cast<void**>(&playLayerLoadFromCheckpoint));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2D5F00), checkpointObjectInitHook,
				  reinterpret_cast<void**>(&checkpointObjectInit));
}