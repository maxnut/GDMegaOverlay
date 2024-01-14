#include "DiscordRPCManager.h"
#include "../Settings.h"
#include "../Common.h"
#include "../types/SeedValue.hpp"
#include "../types/GJGameLevel.hpp"
#include "../utils.hpp"

void DiscordRPCManager::init()
{
	if (!core) discord::Core::Create(clientID, DiscordCreateFlags_NoRequireDiscord, &core);
	if (!core) return;

	rpcTimeStart = std::chrono::duration_cast<std::chrono::seconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
	playerName = MBO(std::string, Common::gjAccountManager, 0x10C);

	if (!Settings::get<bool>("general/discordrpc/enabled")) return;

	updateRPC(State::DEFAULT);
}

void DiscordRPCManager::updateRPC(State state, gd::GJGameLevel* level)
{
	if (!Settings::get<bool>("general/discordrpc/enabled")) return;

	if (core)
	{
		discord::Activity activity{};
		playerName = MBO(std::string, Common::gjAccountManager, 0x10C);

		if (!level)
			state = State::DEFAULT;

		switch (state)
		{
		case State::PLAYING_LEVEL:
			activity.SetState("Playing a level");
			if (level->m_creatorName == "") // usually only happens on main levels
				activity.SetDetails(std::format("{}", level->m_levelName).c_str());
			else
				activity.SetDetails(std::format("{} by {}", level->m_levelName, level->m_creatorName).c_str());

			activity.GetAssets().SetSmallImage(getLevelDifficultyAssetName(level).c_str());

			break;
		case State::EDITING_LEVEL:
			activity.SetState("Editing a level");
			activity.SetDetails(level->m_levelName.c_str());

			activity.GetAssets().SetSmallImage("editor");

			break;

		case State::DEFAULT:
			activity.SetState("");
			activity.SetDetails("Browsing Menus");
			break;
		}

		activity.GetTimestamps().SetStart(rpcTimeStart);
		activity.GetAssets().SetLargeText(playerName.c_str());
		activity.GetAssets().SetLargeImage("cool");
		activity.SetType(discord::ActivityType::Playing);
		core->ActivityManager().UpdateActivity(activity, nullptr);
	}
}

std::string DiscordRPCManager::getLevelDifficultyAssetName(gd::GJGameLevel* level)
{
	if (level->m_ratingsSum != 0)
		level->m_difficulty = static_cast<gd::GJDifficulty>(level->m_ratingsSum / 10);

	if (level->m_demon.value())
	{
		switch (level->m_demonDifficulty)
		{
		case 3: return "easy_demon";
		case 4: return "medium_demon";
		case 5: return "insane_demon";
		case 6: return "extreme_demon";
		default:
		case 0: return "hard_demon";
		}
	}

	switch (level->m_difficulty)
	{
	case gd::GJDifficulty::kGJDifficultyAuto: return "auto";
	case gd::GJDifficulty::kGJDifficultyEasy: return "easy";
	case gd::GJDifficulty::kGJDifficultyNormal: return "normal";
	case gd::GJDifficulty::kGJDifficultyHard: return "hard";
	case gd::GJDifficulty::kGJDifficultyHarder: return "harder";
	case gd::GJDifficulty::kGJDifficultyInsane: return "insane";
	case gd::GJDifficulty::kGJDifficultyDemon: return "hard_demon";
	default: return "na";
	}

	return "na";
}


bool __fastcall DiscordRPCManager::playLayerInitHook(int* self, void*, gd::GJGameLevel* level, bool unk1, bool unk2)
{
	updateRPC(DiscordRPCManager::State::PLAYING_LEVEL, level);

	return playLayerInit(self, level, unk1, unk2);
}
int __fastcall DiscordRPCManager::playLayerOnExitHook(int* self, void*, int unk)
{
	updateRPC(DiscordRPCManager::State::DEFAULT);

	return playLayerOnExit(self, unk);
}

bool __fastcall DiscordRPCManager::levelEditorLayerInitHook(int* self, void*, gd::GJGameLevel* level, bool unk)
{
	updateRPC(DiscordRPCManager::State::EDITING_LEVEL, level);

	return levelEditorLayerInit(self, level, unk);
}
int __fastcall DiscordRPCManager::levelEditorLayerOnExitHook(int* self, void*, int unk)
{
	updateRPC(DiscordRPCManager::State::DEFAULT);

	return levelEditorLayerOnExit(self, unk);
}

bool __fastcall DiscordRPCManager::menuLayerInitHook(int* self, void*)
{
	updateRPC(DiscordRPCManager::State::DEFAULT);

	return menuLayerInit(self);
}

void DiscordRPCManager::initHooks()
{
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2DC4A0), playLayerInitHook,
				  reinterpret_cast<void**>(&playLayerInit));
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2BA240), playLayerOnExitHook,
				  reinterpret_cast<void**>(&playLayerOnExit));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x239A70), levelEditorLayerInitHook,
				  reinterpret_cast<void**>(&levelEditorLayerInit));
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0xA2EF0), levelEditorLayerOnExitHook,
				  reinterpret_cast<void**>(&levelEditorLayerOnExit));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x27B450), menuLayerInitHook,
				  reinterpret_cast<void**>(&menuLayerInit));
}
