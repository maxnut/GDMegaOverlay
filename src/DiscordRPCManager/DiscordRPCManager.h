#pragma once
#include <Geode/binding/GJGameLevel.hpp>
#include <discord_register.h>
#include <discord_rpc.h>

namespace DiscordRPCManager
{
	enum class State
	{
		PLAYING_LEVEL,
		EDITING_LEVEL,
		DEFAULT
	};

	inline const char* clientID = "1055528380956672081";

	inline std::string playerName = "";
	inline long long rpcStartTime = 0;
	inline long long levelStartTime = 0;
	inline bool hasInit = false;

	void init();
	void updateState();
	void updateRPC(State, GJGameLevel* = nullptr);
	const char* getLevelDifficultyAssetName(GJGameLevel*);

	void handleDiscordReady(const DiscordUser*);
	void handleDiscordError(int, const char*);
	void handleDiscordDisconnected(int, const char*);

	void editorPauseLayerOnExitEditorHook(void*, void*);
};
