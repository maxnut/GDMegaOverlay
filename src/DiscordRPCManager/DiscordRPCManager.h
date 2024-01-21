#pragma once
#include <discord.h>
#include <Geode/binding/GJGameLevel.hpp>

namespace DiscordRPCManager
{
	enum class State
	{
		PLAYING_LEVEL,
		EDITING_LEVEL,
		DEFAULT
	};

	inline discord::Core* core{};
	const discord::ClientId clientID = 1055528380956672081;

	inline std::string playerName = "";
	inline long long rpcTimeStart = 0;

	void init();
	void updateRPC(State, GJGameLevel* = nullptr);
	std::string getLevelDifficultyAssetName(GJGameLevel*);
};
