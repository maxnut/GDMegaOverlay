#include "../Common.h"
#include "../ConstData.h"
#include "../GUI/GUI.h"

#include "../util.hpp"
#include "AudioRecord.h"
#include "../Settings.hpp"

#include <fstream>
#include <imgui.h>
#include <iostream>

#include "Macrobot.h"

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/CheckpointObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/binding/GameManager.hpp>

using namespace geode::prelude;
using namespace Macrobot;

class $modify(CCKeyboardDispatcher)
{
	bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool arr)
	{
		if (key == enumKeyCodes::KEY_D || key == enumKeyCodes::KEY_ArrowRight)
			direction = down ? 1 : 0;

		if (key == enumKeyCodes::KEY_A || key == enumKeyCodes::KEY_ArrowLeft)
			direction = down ? -1 : 0;

		return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, arr);
	}
};

class $modify(PlayLayer)
{
	void onQuit()
	{
		PlayLayer::onQuit();
		this->m_player1 = nullptr;
		this->m_player2 = nullptr;
		gameTime = 0;
		checkpoints.clear();
	}

	void loadFromCheckpoint(CheckpointObject * checkpoint)
	{
		if (checkpoints.contains(checkpoint) && playerMode != -1 && GameManager::get()->getPlayLayer())
		{
			CheckpointData checkpointData = checkpoints[checkpoint];
			const auto check = [&](const Action &action) -> bool { return action.time >= checkpointData.time; };
			macro.inputs.erase(std::remove_if (macro.inputs.begin(), macro.inputs.end(), check), macro.inputs.end());
			PlayLayer::loadFromCheckpoint(checkpoint);

			*((double *)GameManager::get()->getPlayLayer() + 1412) = checkpointData.time;
			gameTime = checkpointData.time;
			checkpointData.p1.apply(this->m_player1, true);
			checkpointData.p2.apply(this->m_player2, true);
			if (playerMode == 1)
			{
				this->m_player1->releaseButton(PlayerButton::Jump);
				this->m_player2->releaseButton(PlayerButton::Jump);
			}
		}
		else
			PlayLayer::loadFromCheckpoint(checkpoint);
	}

	void resetLevel()
	{
		if (playerMode != -1)
		{
			actionIndex = 0;
			correctionIndex = 0;
			gameTime = 9999999999;

			PlayLayer::resetLevel();

			if (gameTime == 9999999999)
			{
				*((double *)GameManager::get()->getPlayLayer() + 1412) = 0.0;
				gameTime = 0;
				if (playerMode == 1)
				{
					checkpoints.clear();
					macro.inputs.clear();
				}
			}

			if (this->m_player1->m_isPlatformer)
			{
				if ((ImGui::IsKeyDown(ImGuiKey_RightArrow) || ImGui::IsKeyDown(ImGuiKey_D)) &&
					!(ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_A)))
					this->m_player1->pushButton(PlayerButton::Right);
				else if (!(ImGui::IsKeyDown(ImGuiKey_RightArrow) || ImGui::IsKeyDown(ImGuiKey_D)) &&
						(ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_A)))
					this->m_player1->pushButton(PlayerButton::Left);
			}
		}
		else
			PlayLayer::resetLevel();
	}
};

class $modify(PlayerObject)
{
	void pushButton(PlayerButton btn)
	{
		PlayerObject::pushButton(btn);

		if (GameManager::get()->getPlayLayer() && playerMode == 1 && gameTime != 9999999999)
		{
			Action *ac = recordAction(btn, gameTime, true, this == GameManager::get()->getPlayLayer()->m_player1);

			if (Settings::get<int>("macrobot/corrections") > 0)
			{
				Correction c;
				c.time = gameTime;
				c.player2 = this == GameManager::get()->getPlayLayer()->m_player2;
				c.checkpoint.fromPlayer(this, false);
				ac->correction = c;
			}
		}
	}

	void releaseButton(PlayerButton btn)
	{
		PlayerObject::releaseButton(btn);

		if (GameManager::get()->getPlayLayer() && playerMode == 1 && gameTime != 9999999999)
		{
			if (btn == PlayerButton::Right && direction == 1)
				return;

			if (btn == PlayerButton::Left && direction == -1)
				return;

			Action *ac = recordAction(btn, gameTime, false, this == GameManager::get()->getPlayLayer()->m_player1);

			if (Settings::get<int>("macrobot/corrections") > 0)
			{
				Correction c;
				c.time = gameTime;
				c.player2 = this == GameManager::get()->getPlayLayer()->m_player2;
				c.checkpoint.fromPlayer(this, false);
				ac->correction = c;
			}
		}
	}
};

Macrobot::Action* Macrobot::recordAction(PlayerButton key, double frame, bool press, bool player1)
{
	Action ac(frame, (int)key, !player1, press);

	macro.inputs.push_back(ac);

	return &macro.inputs[macro.inputs.size() - 1];
}

class $modify(CheckpointObject)
{
	bool init()
	{
		bool res = CheckpointObject::init();

		if (playerMode != -1 && gameTime > 0 && GameManager::get()->getPlayLayer())
		{
			CheckpointData data;
			data.time = gameTime;
			data.p1.fromPlayer(GameManager::get()->getPlayLayer()->m_player1, true);
			data.p2.fromPlayer(GameManager::get()->getPlayLayer()->m_player2, true);

			checkpoints[this] = data;
		}

		return res;
	}
};

void Macrobot::GJBaseGameLayerProcessCommands(GJBaseGameLayer* self)
{
	if (playerMode != -1 && GameManager::get()->getPlayLayer())
	{
		double currentTime = *((double *)GameManager::get()->getPlayLayer() + 1412);
		gameTime = currentTime;

		if (playerMode == 0 && macro.inputs.size() > 0 && actionIndex < macro.inputs.size() &&
			gameTime >= macro.inputs[actionIndex].time)
		{
			do
			{
				Action &ac = macro.inputs[actionIndex];
				if (ac.down)
					GameManager::get()->getPlayLayer()->handleButton(true, ac.button, !ac.player2);
				else
					GameManager::get()->getPlayLayer()->handleButton(false, ac.button, !ac.player2);

				int correctionType = Settings::get<int>("macrobot/corrections");

				if (correctionType > 0 && ac.correction.has_value())
				{
					Correction &co = ac.correction.value();
					co.checkpoint.apply(co.player2 ? GameManager::get()->getPlayLayer()->m_player2 : GameManager::get()->getPlayLayer()->m_player1, false);
				}
				actionIndex++;
			} while (actionIndex < macro.inputs.size() && gameTime >= macro.inputs[actionIndex].time);
		}
	}

	reinterpret_cast<void(__thiscall *)(GJBaseGameLayer *)>(base::get() + 0x1BD240)(self);
}

$execute
{
	Mod::get()->hook(reinterpret_cast<void *>(base::get() + 0x1BD240), &GJBaseGameLayerProcessCommands, "GJBaseGameLayer::processCommands", tulip::hook::TulipConvention::Thiscall);
}

void Macrobot::PlayerCheckpoint::fromPlayer(PlayerObject *player, bool fullCapture)
{
	// playerObject + 2280 isplatformer
	// playerObject + 2160 xVelPlatformer
	cocos2d::CCPoint position = player->m_position;
	this->yVel = player->m_yVelocity;
	this->rotation = player->getRotation();
	this->xVel = player->m_platformerXVelocity;
	this->xPos = position.x;
	this->yPos = position.y;
	this->nodeXPos = player->getPositionX();
	this->nodeYPos = player->getPositionY();
	this->rotationRate = player->m_rotationSpeed;

	if (fullCapture)
	{
		this->randomProperties.reserve(2268);
		// dont ask
		for (int i = 0; i < 2265; i++)
			this->randomProperties.push_back(MBO(float, player, 160 + i));
	}
}

/* Macrobot::PlayerCheckpoint::~PlayerCheckpoint()
{
	if (randomProperties != nullptr)
	{
		delete[] randomProperties;
		randomProperties = nullptr;
	}
} */

void Macrobot::PlayerCheckpoint::apply(PlayerObject* player, bool fullRestore)
{
	if (gameTime <= 0)
		return;

	player->m_yVelocity =
		this->yVel; // get all these offsets from playerobject constructor
	player->setRotation(this->rotation);

	player->setPositionX(this->nodeXPos);
	player->setPositionY(this->nodeYPos);

	player->m_position =
		cocos2d::CCPoint(this->xPos, this->yPos);

	player->m_platformerXVelocity = this->xVel; // playerobject_updatemove

	player->m_rotationSpeed = this->rotationRate;

	if (fullRestore)
	{
		// 🗣️ 🔥 🗣️ 🔥 🗣️ 🔥 🗣️ 🔥 🗣️ 🔥 🗣️ 🔥
		// no but seriously this has no right of working so well
		for (int i = 1410; i < 1600; i++)
		{
			if (this->randomProperties[i] < 10000 && this->randomProperties[i] > -10000)
			{
				*reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(player) + 160 + i) = this->randomProperties[i];
			}
		}

		for (int i = 1800; i < 2265; i++)
		{
			if (this->randomProperties[i] < 10000 && this->randomProperties[i] > -10000)
			{
				*reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(player) + 160 + i) = this->randomProperties[i];
			}
		}

		for (int i = 0; i < 1200; i++)
		{
			if (this->randomProperties[i] < 10000 && this->randomProperties[i] > -10000)
			{
				*reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(player) + 160 + i) = this->randomProperties[i];
			}
		}
	}

	// 1350 - 1410
}

void Macrobot::save(const std::string& file)
{
	if (macro.inputs.size() <= 0)
	{
		FLAlertLayer::create("Error", "No inputs in macro!", "Ok")->show();
		return;
	}

	if (file == "")
	{
		FLAlertLayer::create("Error", "Macro name is empty!", "Ok")->show();
		return;
	}

	std::ofstream f(Mod::get()->getSaveDir() / "macros" / (file + ".gdr"), std::ios::binary);

	if (!f)
	{
		FLAlertLayer::create("Error", "Could not save macro!\n" + (Mod::get()->getSaveDir() / "macros" / (file + ".gdr")).string() + "!", "Ok")->show();
		f.close();
		return;
	}

	std::string playerName = GJAccountManager::sharedState()->m_username;
	GJGameLevel *level = GameManager::get()->getPlayLayer()->m_level;

	macro.author = playerName;
	macro.description = macroDescription;
	macro.duration = macro.inputs[macro.inputs.size() - 1].time;
	macro.gameVersion = 2.204f;
	macro.version = 1.0f;

	macro.levelInfo.id = level->m_levelID.value();
	macro.levelInfo.name = level->m_levelName;

	auto data = macro.exportData(false);
	f.write(reinterpret_cast<const char *>(data.data()), data.size());

	f.close();

	FLAlertLayer::create("Info", fmt::format("{} saved with {} inputs.", file, macro.inputs.size()), "Ok")->show();

	macroList.push_back(file);
}

void Macrobot::load(const std::string& file)
{
	std::ifstream f(Mod::get()->getSaveDir() / "macros" / (file + ".gdr"), std::ios::binary);

	if (!f)
	{
		FLAlertLayer::create("Error", "Could not load macro!\n" + (Mod::get()->getSaveDir() / "macros" / (file + ".gdr")).string() + "!", "Ok")->show();
		f.close();
		return;
	}

	f.seekg(0, std::ios::end);
	size_t fileSize = f.tellg();
	f.seekg(0, std::ios::beg);

	std::vector<std::uint8_t> macroData(fileSize);

	f.read(reinterpret_cast<char *>(macroData.data()), fileSize);

	f.close();

	macro = Macro::importData(macroData);

	FLAlertLayer::create("Info", fmt::format("{} loaded with {} inputs.", file, macro.inputs.size()), "Ok")->show();
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
				GameManager::get()->getPlayLayer()->resetLevelFromStart();
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
		if (GUI::button("Load##macropopup"))
			ImGui::OpenPopup("Load Macro");

		GUI::modalPopup("Load Macro", []{

			if (macroList.size() > 0)
			{
				static int macroIndex = 0;

				ImGui::Combo(
				"Macro", &macroIndex,
				[](void* vec, int idx, const char** out_text) {
					std::vector<std::string>* vector = reinterpret_cast<std::vector<std::string>*>(vec);
					if (idx < 0 || idx >= vector->size())
						return false;
					*out_text = vector->at(idx).c_str();
					return true;
				},
				reinterpret_cast<void*>(&macroList), macroList.size());

				if (GUI::button("Load##macro"))
				{
					load(macroList[macroIndex]);
					ImGui::CloseCurrentPopup();
				}
			}
			else
			{
				ImGui::Text("There are no macros to load! Create some macros first.");
			}
		});

		int corrections = Settings::get<int>("macrobot/corrections");

		if (GUI::combo("Corrections", &corrections, correctionType, 2))
			Mod::get()->setSavedValue<int>("macrobot/corrections", corrections);

		GUI::marker("[INFO]", "Corrections are recommended to be safe, but the bot also works decently without.");
	}
}

void Macrobot::getMacros()
{
	macroList.clear();

	ghc::filesystem::path macroPath = Mod::get()->getSaveDir() / "macros";

	for (const auto& entry : ghc::filesystem::directory_iterator(macroPath))
	{
		if (entry.path().extension() == ".gdr")
		{
			macroList.push_back(string::wideToUtf8(entry.path().stem().wstring()));
		}
	}
}