#include "DiscordRPCManager.h"
#include "../Settings.h"
#include "../Common.h"
#include <Geode/utils/SeedValue.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include "../util.hpp"

#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;
using namespace DiscordRPCManager;

class $modify(PlayLayer)
{
	bool init(GJGameLevel* level, bool unk1, bool unk2)
	{
		updateRPC(DiscordRPCManager::State::PLAYING_LEVEL, level);
		return PlayLayer::init(level, unk1, unk2);
	}

	void onExit()
	{
		updateRPC(DiscordRPCManager::State::DEFAULT);
		PlayLayer::onExit();
	}
};

class $modify(LevelEditorLayer)
{
	bool init(GJGameLevel* level, bool unk)
	{
		updateRPC(DiscordRPCManager::State::EDITING_LEVEL, level);
		return LevelEditorLayer::init(level, unk);
	}

	void onExit()
	{
		updateRPC(DiscordRPCManager::State::DEFAULT);
		LevelEditorLayer::onExit();
	}
};

class $modify(MenuLayer)
{
	bool init()
	{
		updateRPC(DiscordRPCManager::State::DEFAULT);
		return MenuLayer::init();
	}
};

void DiscordRPCManager::init()
{
	if (!core) discord::Core::Create(clientID, DiscordCreateFlags_NoRequireDiscord, &core);
	if (!core) return;

	rpcTimeStart = std::chrono::duration_cast<std::chrono::seconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
	playerName = GJAccountManager::sharedState()->m_username; //MBO(std::string, Common::gjAccountManager, 0x10C);

	if (!Settings::get<bool>("general/discordrpc/enabled")) return;

	updateRPC(State::DEFAULT);
}

void DiscordRPCManager::updateRPC(State state, GJGameLevel* level)
{
	if (!Settings::get<bool>("general/discordrpc/enabled")) return;

	if (core)
	{
		discord::Activity activity{};
		playerName = GJAccountManager::sharedState()->m_username;//MBO(std::string, Common::gjAccountManager, 0x10C);

		if (!level)
			state = State::DEFAULT;

		switch (state)
		{
		case State::PLAYING_LEVEL:
			activity.SetState("Playing a level");
			if (level->m_creatorName.empty()) // usually only happens on main levels
				activity.SetDetails(std::format("{}", level->m_levelName.c_str()).c_str());
			else
				activity.SetDetails(std::format("{} by {}", level->m_levelName.c_str(), level->m_creatorName.c_str()).c_str());

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

std::string DiscordRPCManager::getLevelDifficultyAssetName(GJGameLevel* level)
{
	if (level->m_ratingsSum != 0)
		level->m_difficulty = static_cast<GJDifficulty>(level->m_ratingsSum / 10);

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
	case GJDifficulty::Auto: return "auto";
	case GJDifficulty::Easy: return "easy";
	case GJDifficulty::Normal: return "normal";
	case GJDifficulty::Hard: return "hard";
	case GJDifficulty::Harder: return "harder";
	case GJDifficulty::Insane: return "insane";
	case GJDifficulty::Demon: return "hard_demon";
	default: return "na";
	}

	return "na";
}
