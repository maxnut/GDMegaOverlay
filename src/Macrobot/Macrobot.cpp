#include "../Common.h"
#include "../ConstData.h"
#include "../GUI/GUI.h"

#include "../util.hpp"
#include "AudioRecord.h"
#include "../Settings.hpp"

#include <fstream>
#include <imgui-cocos.hpp>
#include <iostream>

#include "Macrobot.h"
#include "Clickpacks.h"
#include "../Hacks/PhysicsBypass.h"

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
		gameTime = 0;
		checkpoints.clear();
	}

	void loadFromCheckpoint(CheckpointObject * checkpoint)
	{
		if (checkpoints.contains(checkpoint) && playerMode != DISABLED && GameManager::get()->getPlayLayer())
		{
			CheckpointData checkpointData = checkpoints[checkpoint];
			const auto check = [&](const Action &action) -> bool { return action.frame >= checkpointData.frame; };
			macro.inputs.erase(std::remove_if (macro.inputs.begin(), macro.inputs.end(), check), macro.inputs.end());
			PlayLayer::loadFromCheckpoint(checkpoint);

			gameTime = checkpointData.time;
			checkpointData.p1.apply(this->m_player1, true);
			checkpointData.p2.apply(this->m_player2, true);
		}
		else
			PlayLayer::loadFromCheckpoint(checkpoint);
	}

	void resetLevel()
	{
		if (playerMode != DISABLED)
		{
			actionIndex = 0;
			correctionIndex = 0;
			gameTime = 9999999999;

			resetFrame = true;

			downForKey1.clear();
			downForKey2.clear();
			timeForKey1.clear();
			timeForKey2.clear();

			PlayLayer::resetLevel();

			GJGameLevel *level = GameManager::get()->getPlayLayer()->m_level;
			macro.levelInfo.id = level->m_levelID.value();
			macro.levelInfo.name = level->m_levelName;

			if (gameTime == 9999999999)
			{
				gameTime = 0;
				if (playerMode == RECORDING)
				{
					checkpoints.clear();
					macro.inputs.clear();
				}
			}

			resetFrame = false;

			if (this->m_player1->m_isPlatformer)
			{
				if ((ImGui::IsKeyDown(ImGuiKey_RightArrow) || ImGui::IsKeyDown(ImGuiKey_D)) &&
					!(ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_A)))
					this->m_player1->pushButton(PlayerButton::Right);
				else if (!(ImGui::IsKeyDown(ImGuiKey_RightArrow) || ImGui::IsKeyDown(ImGuiKey_D)) &&
						(ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_A)))
					this->m_player1->pushButton(PlayerButton::Left);
			}

			bool isDual = MBO(bool, this, 878);

			gameTime += (1.0 / (double)Common::getTPS()) + 0.00000001;
			this->handleButton(false, 1, true);
			if(isDual)
				this->handleButton(false, 1, false);
			gameTime -= (1.0 / (double)Common::getTPS()) + 0.00000001;
		}
		else
			PlayLayer::resetLevel();
	}
};

class $modify(PlayerObject)
{
	void pushButton(PlayerButton btn)
	{
		if((playerMode == PLAYBACK && !botInput) || resetFrame)
			return;
		PlayerObject::pushButton(btn);
		
		auto pl = GameManager::get()->getPlayLayer();

		if (pl && playerMode == RECORDING && gameTime != 9999999999)
		{
			if(!pl->m_levelSettings->m_twoPlayerMode && this == pl->m_player2)
				return;

			recordAction(btn, gameTime, true, this == pl->m_player1);
		}
	}

	void releaseButton(PlayerButton btn)
	{
		if((playerMode == PLAYBACK && !botInput) || resetFrame)
			return;
		PlayerObject::releaseButton(btn);

		auto pl = GameManager::get()->getPlayLayer();

		if (pl && playerMode == RECORDING && gameTime != 9999999999)
		{
			if(!pl->m_levelSettings->m_twoPlayerMode && this == pl->m_player2)
				return;

			if (btn == PlayerButton::Right && direction == 1)
				return;

			if (btn == PlayerButton::Left && direction == -1)
				return;

			recordAction(btn, gameTime, false, this == pl->m_player1);
		}
	}
};

Macrobot::Action* Macrobot::recordAction(PlayerButton key, double frame, bool press, bool player1)
{
	Action ac(frame, (int)key, !player1, press);

	ac.frame = macro.frameForTime(frame);

	PlayLayer* pl = GameManager::get()->getPlayLayer();

	if (Settings::get<int>("macrobot/corrections") > 0)
	{
		Correction c;
		c.time = gameTime;
		c.player2 = !player1;
		c.checkpoint.fromPlayer(player1 ? pl->m_player1 : pl->m_player2, false);
		ac.correction = c;
	}

	macro.inputs.push_back(ac);

	return &macro.inputs[macro.inputs.size() - 1];
}

class $modify(CheckpointObject)
{
	bool init()
	{
		bool res = CheckpointObject::init();

		if (playerMode != DISABLED && gameTime > 0 && GameManager::get()->getPlayLayer())
		{
			CheckpointData data;
			data.time = gameTime;
			data.frame = macro.frameForTime(gameTime);
			data.p1.fromPlayer(GameManager::get()->getPlayLayer()->m_player1, true);
			data.p2.fromPlayer(GameManager::get()->getPlayLayer()->m_player2, true);

			//log::debug("FROMPLAYER {} {} {}", data.p1.yVel, data.p1.xPos, gdr::frameForTime(data.time));

			checkpoints[this] = data;
		}

		return res;
	}
};

void Macrobot::handleAction(bool down, int button, bool player1, float timestamp)
{
	botInput = true;
	GameManager::get()->getPlayLayer()->handleButton(down, button, player1);
	botInput = false;

	bool playClicks = Settings::get<bool>("macrobot/clicks/enabled", false);

	if(!playClicks)
		return;

	std::unordered_map<int, bool> &downForKey = player1 ? downForKey1 : downForKey2;
	std::unordered_map<int, float> &timeForKey = player1 ? timeForKey1 : timeForKey2;

	if(downForKey.contains(button) && downForKey[button] == down)
		return;

	if(!timeForKey.contains(button))
		timeForKey[button] = 0;

	downForKey[button] = down;

	float timeDifference = timestamp - timeForKey[button];
	float softclickTime = Settings::get<float>("clickpacks/softclicks_at", 0.1f);

	float minPitch = Settings::get<float>("clickpacks/click/min_pitch", 0.98f);
	float maxPitch = Settings::get<float>("clickpacks/click/max_pitch", 1.02f);

	float volume = Settings::get<float>("clickpacks/click/volume", 1.f);

	float pitchVar = minPitch +
					static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxPitch - minPitch)));

	FMOD::Sound* soundToPlay = nullptr;
	FMOD::System* system = FMODAudioEngine::sharedEngine()->m_system;

	switch(button)
	{
		case 1:
			soundToPlay = down ? (timeDifference <= softclickTime ? Clickpacks::currentClickpack.randomSoftClick() : Clickpacks::currentClickpack.randomClick()) : Clickpacks::currentClickpack.randomRelease();
			break;
		case 2:
		case 3:
			soundToPlay = down ? Clickpacks::currentClickpack.randomPlatClick() : Clickpacks::currentClickpack.randomPlatRelease();
	}

	timeForKey[button] = timestamp;

	system->playSound(soundToPlay, nullptr, false, &clickChannel);

	clickChannel->setPitch(pitchVar);
	clickChannel->setVolume(volume);
}

void Macrobot::GJBaseGameLayerProcessCommands(GJBaseGameLayer* self)
{
	if (playerMode != DISABLED && GameManager::get()->getPlayLayer())
	{
		gameTime += (1.0 / (double)Common::getTPS()) + 0.00000001;

		uint32_t gameFrame = macro.frameForTime(gameTime);

		if(macro.inputs[actionIndex].time >= 0)
			macro.inputs[actionIndex].frame = macro.frameForTime(macro.inputs[actionIndex].time);

		//log::debug("PROCESSCOMMANDS {} {} {}", MBO(double, GameManager::get()->getPlayLayer()->m_player1, 1936), GameManager::get()->getPlayLayer()->m_player1->m_position.x, gameFrame);

		if (playerMode == PLAYBACK && macro.inputs.size() > 0 && actionIndex < macro.inputs.size() &&
			gameFrame >= macro.inputs[actionIndex].frame)
		{
			do
			{
				Action &ac = macro.inputs[actionIndex];

				if(ac.time >= 0)
					ac.frame = macro.frameForTime(ac.time);

				handleAction(ac.down, ac.button, !ac.player2, ac.frame / macro.framerate);

				int correctionType = Settings::get<int>("macrobot/corrections");

				if (correctionType > 0 && ac.correction.has_value())
				{
					Correction &co = ac.correction.value();
					co.checkpoint.apply(co.player2 ? GameManager::get()->getPlayLayer()->m_player2 : GameManager::get()->getPlayLayer()->m_player1, false);
				}
				actionIndex++;
			} while (actionIndex < macro.inputs.size() && (gameFrame >= macro.inputs[actionIndex].frame));
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
	if(!player)
		return;

	cocos2d::CCPoint position = player->m_position;
	this->yVel = MBO(double, player, 1936);
	this->rotation = player->getRotation();
	this->xVel = player->m_platformerXVelocity;
	this->xPos = position.x;
	this->yPos = position.y;
	this->nodeXPos = player->getPositionX();
	this->nodeYPos = player->getPositionY();
	this->rotationRate = player->m_rotationSpeed;
	this->lastSnappedTo = player->m_objectSnappedTo;

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

		for (int i = 0; i < 1300; i++)
		{
			if (this->randomProperties[i] < 10000 && this->randomProperties[i] > -10000)
			{
				*reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(player) + 160 + i) = this->randomProperties[i];
			}
		}
	}

	// 1350 - 1410
	
	player->m_objectSnappedTo = this->lastSnappedTo;

	MBO(double, player, 1936) = this->yVel; //remove this when geode fixes the offset
	player->setRotation(this->rotation);

	player->setPositionX(this->nodeXPos);
	player->setPositionY(this->nodeYPos);

	player->m_position =
		cocos2d::CCPoint(this->xPos, this->yPos);

	player->m_platformerXVelocity = this->xVel;

	player->m_rotationSpeed = this->rotationRate;
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

	macro.author = playerName;
	macro.description = macroDescription;
	macro.duration = macro.inputs[macro.inputs.size() - 1].frame / macro.framerate;
	macro.gameVersion = 2.204f;
	macro.version = 1.0f;

	macro.framerate = 240.f;

	if (Settings::get<bool>("general/tps/enabled"))
		macro.framerate = Settings::get<float>("general/tps/value", 240.f);

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

	PhysicsBypass::calculateTickrate();
}

void Macrobot::drawWindow()
{
	if (GUI::shouldRender())
	{
		if (ImGui::RadioButton("Disable", (int*)&Macrobot::playerMode, (int)DISABLED))
		{
			Common::calculateFramerate();
			PhysicsBypass::calculateTickrate();
		}
		if (ImGui::RadioButton("Record", (int*)&Macrobot::playerMode, (int)RECORDING))
		{
			Common::calculateFramerate();
			PhysicsBypass::calculateTickrate();
			if (GameManager::get()->getPlayLayer())
				GameManager::get()->getPlayLayer()->resetLevelFromStart();
		}
		if (ImGui::RadioButton("Play", (int*)&Macrobot::playerMode, (int)PLAYBACK))
		{
			Common::calculateFramerate();
			PhysicsBypass::calculateTickrate();
		}

		ImGui::PushItemWidth(80);
		GUI::inputText("Macro Name", &macroName);
		GUI::inputText("Macro Description", &macroDescription);
		ImGui::PopItemWidth();

		if (GUI::button("Save##macro"))
			save(macroName);
		GUI::sameLine();
		if (GUI::button("Load##macropopup"))
		{
			getMacros();
			ImGui::OpenPopup("Load Macro");
		}

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

				if (GUI::button("Open Macros Folder"))
					ShellExecute(0, NULL, string::wideToUtf8((Mod::get()->getSaveDir() / "macros").wstring()).c_str(), NULL, NULL, SW_SHOW);
			}
			else
			{
				ImGui::Text("There are no macros to load! Create some macros first.");
			}
		});

		int corrections = Settings::get<int>("macrobot/corrections");

		if (GUI::combo("Corrections", &corrections, correctionType, 2))
			Mod::get()->setSavedValue<int>("macrobot/corrections", corrections);

		GUI::checkbox("Click Sounds", "macrobot/clicks/enabled");
		GUI::arrowButton("Clickpacks");
		Clickpacks::drawGUI();

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