#include "DiscordRPCManager.h"

#include "../Common.h"
#include <Geode/binding/GJGameLevel.hpp>
#include "../util.hpp"

#include <discord_register.h>
#include <discord_rpc.h>

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
		bool res = PlayLayer::init(level, unk1, unk2);
		updateRPC(DiscordRPCManager::State::PLAYING_LEVEL, level);
		return res;
	}

	void onQuit()
	{
		updateRPC(DiscordRPCManager::State::DEFAULT);
		PlayLayer::onQuit();
	}
};

class $modify(LevelEditorLayer)
{
	bool init(GJGameLevel* level, bool unk)
	{
		updateRPC(DiscordRPCManager::State::EDITING_LEVEL, level);
		return LevelEditorLayer::init(level, unk);
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
	DiscordEventHandlers handler;

	handler.ready = handleDiscordReady;
	handler.errored = handleDiscordError;
	handler.disconnected = handleDiscordDisconnected;
	Discord_Initialize(clientID, &handler, 1, "322170");
	Discord_RunCallbacks();

	rpcStartTime = std::chrono::duration_cast<std::chrono::seconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
	playerName = GJAccountManager::sharedState()->m_username; //MBO(std::string, Common::gjAccountManager, 0x10C);

	updateRPC(State::DEFAULT);
	hasInit = true;
}

void DiscordRPCManager::updateState()
{
	if (!Mod::get()->getSavedValue<bool>("general/discordrpc/enabled") && hasInit)
	{
		Discord_ClearPresence();
		Discord_Shutdown();
		hasInit = false;
	}
	else if (Mod::get()->getSavedValue<bool>("general/discordrpc/enabled") && !hasInit)
	{
		init();
		hasInit = true;
	}
}

void DiscordRPCManager::updateRPC(State state, GJGameLevel* level)
{
	if (!Mod::get()->getSavedValue<bool>("general/discordrpc/enabled")) return;

	DiscordRichPresence presence{};

	if (Mod::get()->getSavedValue<bool>("general/discordrpc/incognito"))
	{
		presence.state = "Playing the game";
		presence.startTimestamp = rpcStartTime;
		presence.largeImageKey = "cool";
		presence.instance = 0;

		Discord_UpdatePresence(&presence);
		return;
	}

	playerName = GJAccountManager::sharedState()->m_username;//MBO(std::string, Common::gjAccountManager, 0x10C);

	if (!level)
		state = State::DEFAULT;

	switch (state)
	{
	case State::PLAYING_LEVEL:
		presence.state = "Playing a level";
		levelStartTime = std::chrono::duration_cast<std::chrono::seconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count();

		if (level->m_creatorName.empty()) // usually only happens on main levels
			presence.details = std::format("{}", level->m_levelName.c_str()).c_str();
		else
			presence.details = std::format("{} by {}", level->m_levelName.c_str(), level->m_creatorName.c_str()).c_str();

		presence.smallImageKey = getLevelDifficultyAssetName(level);

		break;
	case State::EDITING_LEVEL:
		presence.state = "Editing a level";
		presence.details = level->m_levelName.c_str();

		presence.smallImageKey = "editor";

		break;

	case State::DEFAULT:
		presence.state = "";
		presence.details = "Browsing Menus";
		break;
	}

	presence.startTimestamp = state == State::PLAYING_LEVEL ? levelStartTime : rpcStartTime;
	presence.largeImageText = playerName.c_str();
	presence.largeImageKey = "cool";
	presence.instance = 0;
	Discord_UpdatePresence(&presence);
}

const char* DiscordRPCManager::getLevelDifficultyAssetName(GJGameLevel* level)
{
	if (level->m_autoLevel)
		return "auto";

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
	case GJDifficulty::Easy: return "easy";
	case GJDifficulty::Normal: return "normal";
	case GJDifficulty::Hard: return "hard";
	case GJDifficulty::Harder: return "harder";
	case GJDifficulty::Insane: return "insane";
	case GJDifficulty::Demon: return "hard_demon";
	}

	return "na";
}

void DiscordRPCManager::handleDiscordReady(const DiscordUser* user)
{
	log::info("Connected to Discord RPC");
	log::info("Username: {}", user->username);
	log::info("UserID: {}", user->userId);
}

void DiscordRPCManager::handleDiscordError(int errorCode, const char* message)
{
	log::info("Discord RPC error");
	log::info("Error Code: {}", errorCode);
	log::info("Message: {}", message);
}

void DiscordRPCManager::handleDiscordDisconnected(int errorCode, const char* message)
{
	log::info("Discord RPC disconnected");
	log::info("Error Code: {}", errorCode);
	log::info("Message: {}", message);
}

// geode bindings pls accept my pr
void DiscordRPCManager::editorPauseLayerOnExitEditorHook(void* self, void* sender)
{
	updateRPC(DiscordRPCManager::State::DEFAULT);

	reinterpret_cast<void(__thiscall*)(void*, void*)>(base::get() + 0xA2EF0)(self, sender);
}

$execute
{
	Mod::get()->hook(reinterpret_cast<void*>(base::get() + 0xA2EF0), &editorPauseLayerOnExitEditorHook, "EditorPauseLayer::onExitEditor", tulip::hook::TulipConvention::Thiscall);
}
