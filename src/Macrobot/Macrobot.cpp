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

		if (!down || arr)
			return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, arr);
		
		int advanceKey = Settings::get<int>("macrobot/frame_step/key", ImGuiKey_G);

		int convertedKey = ConvertKeyEnum(key);

		if (convertedKey == advanceKey && PlayLayer::get())
			targetSteps = PlayLayer::get()->m_gameState.m_unk1f8 + Settings::get<int>("macrobot/frame_step/steps", 1);

		return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, arr);
	}
};

class $modify(PlayLayer)
{
	void onQuit()
	{
		PlayLayer::onQuit();
		checkpoints.clear();
	}

	void loadFromCheckpoint(CheckpointObject * checkpoint)
	{
		resetFromStart = false;
		if (checkpoints.contains(checkpoint) && playerMode != DISABLED && PlayLayer::get())
		{
			CheckpointData checkpointData = checkpoints[checkpoint];
			const auto check = [&](const Action &action) -> bool { return action.frame > checkpointData.frame; };
			macro.inputs.erase(std::remove_if (macro.inputs.begin(), macro.inputs.end(), check), macro.inputs.end());
			PlayLayer::loadFromCheckpoint(checkpoint);

			checkpointData.p1.apply(this->m_player1, true);
			if (MBO(bool, this, 878))
				checkpointData.p2.apply(this->m_player2, true);
		}
		else
			PlayLayer::loadFromCheckpoint(checkpoint);
	}

	void resetLevel()
	{
		targetSteps = 0;
		
		if (playerMode != DISABLED)
		{
			actionIndex = 0;

			resetFromStart = true;
			resetFrame = true;

			downForKey1.clear();
			downForKey2.clear();
			timeForKey1.clear();
			timeForKey2.clear();

			PlayLayer::resetLevel();

			GJGameLevel *level = PlayLayer::get()->m_level;
			macro.levelInfo.id = level->m_levelID.value();
			macro.levelInfo.name = level->m_levelName;

			if (resetFromStart && playerMode == RECORDING)
			{
				checkpoints.clear();
				macro.inputs.clear();

				macro.framerate = Common::getTPS();
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

			if (playerMode == RECORDING)
			{
				//TODO only do this when necessary
				bool isDual = MBO(bool, this, 878);

				Macrobot::recordAction(PlayerButton::Jump, this->m_gameState.m_unk1f8 + 1, false, true);
				if (isDual)
					Macrobot::recordAction(PlayerButton::Jump, this->m_gameState.m_unk1f8 + 1, false, false);
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
		if ((playerMode == PLAYBACK && !botInput) || resetFrame)
			return;
		PlayerObject::pushButton(btn);
		
		auto pl = PlayLayer::get();

		if (pl && playerMode == RECORDING && !resetFrame)
		{
			if (!pl->m_levelSettings->m_twoPlayerMode && this == pl->m_player2)
				return;

			recordAction(btn, pl->m_gameState.m_unk1f8, true, this == pl->m_player1);
		}
	}

	void releaseButton(PlayerButton btn)
	{
		if ((playerMode == PLAYBACK && !botInput) || resetFrame)
			return;
		PlayerObject::releaseButton(btn);

		auto pl = PlayLayer::get();

		if (pl && playerMode == RECORDING && !resetFrame)
		{
			if (!pl->m_levelSettings->m_twoPlayerMode && this == pl->m_player2)
				return;

			if (btn == PlayerButton::Right && direction == 1)
				return;

			if (btn == PlayerButton::Left && direction == -1)
				return;

			recordAction(btn, pl->m_gameState.m_unk1f8, false, this == pl->m_player1);
		}
	}
};

Macrobot::Action* Macrobot::recordAction(PlayerButton key, uint32_t frame, bool press, bool player1)
{
	Action ac(frame, (int)key, !player1, press);

	PlayLayer* pl = PlayLayer::get();
	PlayerObject* player = player1 ? pl->m_player1 : pl->m_player2;

	if ((CorrectionType)Settings::get<int>("macrobot/corrections") == ACTION && (player->m_position.x != 0 && player->m_position.y != 0))
	{
		ac.correction = Correction(frame, !player1);
		ac.correction->checkpoint.fromPlayer(player, false);
	}

	macro.inputs.push_back(ac);

	return &macro.inputs[macro.inputs.size() - 1];
}

class $modify(GJBaseGameLayer)
{
	void update(float dt)
	{
		if (Settings::get<bool>("macrobot/frame_step/enabled", false))
		{
			if (PlayLayer::get()->m_gameState.m_unk1f8 < targetSteps)
				GJBaseGameLayer::update(dt);
			return;
		}

		GJBaseGameLayer::update(dt);
	}
};

class $modify(CheckpointObject)
{
	bool init()
	{
		bool res = CheckpointObject::init();

		if (playerMode != DISABLED && PlayLayer::get() && PlayLayer::get()->m_gameState.m_unk1f8 > 0)
		{
			CheckpointData data;
			data.frame = PlayLayer::get()->m_gameState.m_unk1f8;
			data.p1.fromPlayer(PlayLayer::get()->m_player1, true);
			data.p2.fromPlayer(PlayLayer::get()->m_player2, true);

			checkpoints[this] = data;
		}

		return res;
	}
};

void Macrobot::handleAction(Action& action)
{
	auto pl = PlayLayer::get();

	int button = action.button;
	bool down = action.down;
	bool player1 = !action.player2;

	botInput = true;
	pl->handleButton(down, button, player1);
	botInput = false;

	if ((CorrectionType)Settings::get<int>("macrobot/corrections") == ACTION && action.correction.has_value())
		action.correction->checkpoint.apply(action.correction->player2 ? pl->m_player2 : pl->m_player1, false);

	if (!Settings::get<bool>("macrobot/clicks/enabled", false))
		return;

	float timestamp = action.frame / macro.framerate;

	std::unordered_map<int, bool> &downForKey = player1 ? downForKey1 : downForKey2;
	std::unordered_map<int, float> &timeForKey = player1 ? timeForKey1 : timeForKey2;

	if (downForKey.contains(button) && downForKey[button] == down)
		return;

	if (!timeForKey.contains(button))
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
	reinterpret_cast<void(__thiscall *)(GJBaseGameLayer *)>(base::get() + 0x1BD240)(self);

	if (playerMode == PLAYBACK && PlayLayer::get())
	{
		uint32_t gameFrame = self->m_gameState.m_unk1f8;

		while (actionIndex < macro.inputs.size() && (gameFrame >= macro.inputs[actionIndex].frame))
		{
			handleAction(macro.inputs[actionIndex]);
			actionIndex++;
		}
	}
}

$execute
{
	Mod::get()->hook(reinterpret_cast<void *>(base::get() + 0x1BD240), &GJBaseGameLayerProcessCommands, "GJBaseGameLayer::processCommands", tulip::hook::TulipConvention::Thiscall);
}

void Macrobot::MPlayerCheckpoint::fromPlayer(PlayerObject *player, bool fullCapture)
{
	// playerObject + 2280 isplatformer
	// playerObject + 2160 xVelPlatformer
	if (!player)
		return;

	cocos2d::CCPoint position = player->m_position;
	this->yVel = player->m_yVelocity;
	this->rotation = player->getRotation();
	this->xVel = player->m_platformerXVelocity;
	this->xPos = position.x;
	this->yPos = position.y;
	this->nodeXPos = player->getPositionX();
	this->nodeYPos = player->getPositionY();
	this->rotationRate = player->m_rotationSpeed;
	this->lastSnappedTo = player->m_objectSnappedTo;
	this->lastSnappedTo2 = MBO(GameObject*, player, 1724);
	this->isOnSlope = player->m_isOnSlope;
	this->wasOnSlope = player->m_wasOnSlope;

	if (fullCapture)
	{
		this->randomProperties.reserve(2268);
		for (int i = 0; i < 2265; i++)
			this->randomProperties.push_back(MBO(float, player, 160 + i));
	}
}

void Macrobot::MPlayerCheckpoint::apply(PlayerObject* player, bool fullRestore)
{
	if (PlayLayer::get()->m_gameState.m_unk1f8 <= 0)
		return;

	if (fullRestore)
	{
		//this range contains a value that makes orb y vel work, if it doesnt work on future gd updates change this
		for (int i = 1410; i < 1700; i++)
		{
			if (this->randomProperties[i] < 10000 && this->randomProperties[i] > -10000)
			{
				*reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(player) + 160 + i) = this->randomProperties[i];
			}
		}

		for (int i = 1794; i < 2265; i++)
		{
			if (this->randomProperties[i] < 10000 && this->randomProperties[i] > -10000)
			{
				*reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(player) + 160 + i) = this->randomProperties[i];
			}
		}

		//this first range contains a value that makes the player turn big if its < 100? tf
		for (int i = 100; i < 1300; i++)
		{
			if (this->randomProperties[i] < 10000 && this->randomProperties[i] > -10000)
			{
				*reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(player) + 160 + i) = this->randomProperties[i];
			}
		}

		player->m_objectSnappedTo = this->lastSnappedTo;
		MBO(GameObject*, player, 1724) = this->lastSnappedTo2; // get this in checkjumpsnaptoobject
	}

	player->m_yVelocity = this->yVel;
	player->setRotation(this->rotation);

	player->setPositionX(this->nodeXPos);
	player->setPositionY(this->nodeYPos);

	player->m_position = cocos2d::CCPoint(this->xPos, this->yPos);

	player->m_platformerXVelocity = this->xVel;

	player->m_rotationSpeed = this->rotationRate;

	player->m_isOnSlope = this->isOnSlope;
	player->m_wasOnSlope = this->wasOnSlope;

	//TODO: spamming checkpoints on slopes changes slope start time

	/* double slopeTime = MBO(double, self->m_player1, 2144);
	double slopeStartTime = MBO(double, self->m_player1, 1240);*/
}

void Macrobot::save(const std::string& file)
{
	if (macro.inputs.size() <= 0)
	{
		Common::showWithPriority(FLAlertLayer::create("Error", "No inputs in macro!", "Ok"));
		return;
	}

	if (file == "")
	{
		Common::showWithPriority(FLAlertLayer::create("Error", "Macro name is empty!", "Ok"));
		return;
	}

	std::ofstream f(Mod::get()->getSaveDir() / "macros" / (file + ".gdr"), std::ios::binary);

	if (!f)
	{
		Common::showWithPriority(FLAlertLayer::create("Error", "Could not save macro!\n" + (Mod::get()->getSaveDir() / "macros" / (file + ".gdr")).string() + "!", "Ok"));
		f.close();
		return;
	}

	macro.author = GJAccountManager::sharedState()->m_username;
	macro.description = macroDescription;
	macro.duration = macro.inputs[macro.inputs.size() - 1].frame / macro.framerate;
	macro.gameVersion = 2.204f;
	macro.version = 1.0f;

	auto data = macro.exportData(false);

	f.write(reinterpret_cast<const char *>(data.data()), data.size());
	f.close();

	Common::showWithPriority(FLAlertLayer::create("Info", fmt::format("{} saved with {} inputs.", file, macro.inputs.size()), "Ok"));
}


std::optional<Macro> Macrobot::loadMacro(const std::string& file, bool inputs)
{
	std::ifstream f(Mod::get()->getSaveDir() / "macros" / (file + ".gdr"), std::ios::binary);

	if (!f)
	{
		f.close();
		return std::nullopt;
	}

	f.seekg(0, std::ios::end);
	size_t fileSize = f.tellg();
	f.seekg(0, std::ios::beg);

	std::vector<std::uint8_t> macroData(fileSize);

	f.read(reinterpret_cast<char *>(macroData.data()), fileSize);
	f.close();

	return Macro::importData(macroData, inputs);
}


bool Macrobot::load(const std::string& file)
{
	auto opt = loadMacro(file);

	if (!opt)
	{
		Common::showWithPriority(FLAlertLayer::create("Error", "Could not load macro!\n" + (Mod::get()->getSaveDir() / "macros" / (file + ".gdr")).string() + "!", "Ok"));
		return false;
	}

	macro = opt.value();

	if (macro.botInfo.version != "1.1")
	{
		macro = Macro();
		Common::showWithPriority(FLAlertLayer::create("Error", "This macro was recorded on an older macrobot version!", "Ok"));
		return false;
	}

	Common::showWithPriority(FLAlertLayer::create("Info", fmt::format("{} loaded with {} inputs.", file, macro.inputs.size()), "Ok"));
	PhysicsBypass::calculateTickrate();
	
	return true;
}

void Macrobot::remove(const std::string& file)
{
	ghc::filesystem::remove(Mod::get()->getSaveDir() / "macros" / (file + ".gdr"));
}

void Macrobot::drawMacroTable()
{
	static int macroIndex = 0;
	static std::string searchText = "";

	bool needReload = false;

	GUI::inputText("Search", &searchText);

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {10, 10});
	if (!ImGui::BeginTable("MacroTable", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_PadOuterX | ImGuiTableFlags_Resizable |
									ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody))
	{
		ImGui::PopStyleVar();
		return;
	}

	ImGui::TableSetupColumn("Name");
	ImGui::TableSetupColumn("Author");
	ImGui::TableSetupColumn("FPS");
	ImGui::TableSetupColumn("Duration");
	ImGui::TableSetupColumn("Level");
	ImGui::TableSetupColumn("Actions");
	ImGui::TableHeadersRow();

	for(auto& pair : macroList)
	{
		Macro& macro = pair.second;

		if (!searchText.empty() && pair.first.find(searchText) == std::string::npos)
			continue;

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(pair.first.c_str());
		ImGui::TableNextColumn();
		ImGui::Text(macro.author.c_str());
		ImGui::TableNextColumn();
		ImGui::Text("%.2f", macro.framerate);
		ImGui::TableNextColumn();
		ImGui::Text("%.2f", macro.duration);
		ImGui::TableNextColumn();
		ImGui::Text("%s (%i)", macro.levelInfo.name.c_str(), macro.levelInfo.id);
		ImGui::TableNextColumn();
		if (GUI::button(("Load##macro" + pair.first).c_str()))
		{
			load(pair.first);
			ImGui::CloseCurrentPopup();
			GUI::toggle();
		}
		ImGui::SameLine();
		if (GUI::button(("Remove##macro" + pair.first).c_str()))
		{
			remove(pair.first);
			needReload = true;
		}
		ImGui::SameLine();
		if (GUI::button(("Play##macro" + pair.first).c_str()))
		{

			GameLevelManager::get()->downloadLevel(macro.levelInfo.id, true);

			auto level = GameLevelManager::get()->getSavedLevel(macro.levelInfo.id);

			if (level && load(pair.first))
			{
				auto layer = PlayLayer::create(level, false, false);
				auto scene = CCScene::create();
				scene->addChild(layer);

				cocos2d::CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, scene));

				Macrobot::playerMode = PLAYBACK;

				Common::calculateFramerate();
				PhysicsBypass::calculateTickrate();

				layer->resetLevelFromStart();

				ImGui::CloseCurrentPopup();
				GUI::toggle();
			}
		}
	}
	
	ImGui::EndTable();
	ImGui::PopStyleVar();

	if (needReload)
		getMacros();
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
			if (PlayLayer::get())
				PlayLayer::get()->resetLevelFromStart();
		}
		if (ImGui::RadioButton("Play", (int*)&Macrobot::playerMode, (int)PLAYBACK))
		{
			Common::calculateFramerate();
			PhysicsBypass::calculateTickrate();
		}

		ImGui::PushItemWidth(80);
		GUI::inputText("Name##macro", &macroName);
		GUI::inputText("Description##macro", &macroDescription);
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
				drawMacroTable();
			else
				ImGui::Text("There are no macros to load! Create some macros first.");
		});

		int corrections = Settings::get<int>("macrobot/corrections");

		if (GUI::combo("Corrections", &corrections, correctionType, 2))
			Mod::get()->setSavedValue<int>("macrobot/corrections", corrections);

		GUI::checkbox("Click Sounds", "macrobot/clicks/enabled");
		GUI::arrowButton("Clickpacks");
		Clickpacks::drawGUI();
	}

	GUI::checkbox("Frame Step", "macrobot/frame_step/enabled");

	GUI::arrowButton("Frame Step Settings");
	GUI::modalPopup(
		"Frame Step Settings",
		[] {
			int key = Settings::get<int>("macrobot/frame_step/key", ImGuiKey_G);
			if (GUI::hotkey("Advance", &key))
				Mod::get()->setSavedValue<int>("macrobot/frame_step/key", key);

			GUI::inputInt("Advance Steps", "macrobot/frame_step/steps", 1);
		},
		ImGuiWindowFlags_AlwaysAutoResize);

	GUI::marker("[INFO]", "Corrections are recommended to be safe, but the bot also works decently without.");
}

void Macrobot::getMacros()
{
	macroList.clear();

	ghc::filesystem::path macroPath = Mod::get()->getSaveDir() / "macros";

	for (const auto& entry : ghc::filesystem::directory_iterator(macroPath))
	{
		if (entry.path().extension() == ".gdr")
		{
			std::string name = string::wideToUtf8(entry.path().stem().wstring());
			auto macroOpt = loadMacro(name, false);
			if (macroOpt)
				macroList.insert({name, macroOpt.value()});
		}
	}
}