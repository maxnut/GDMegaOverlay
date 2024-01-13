#pragma once
#include <discord.h>
#include "../types/GJGameLevel.hpp"

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
	void initHooks();
	void updateRPC(State, gd::GJGameLevel* = nullptr);
	std::string getLevelDifficultyAssetName(gd::GJGameLevel*);


	inline static bool(__thiscall* playLayerInit)(int*, gd::GJGameLevel*, bool, bool);
	bool __fastcall playLayerInitHook(int*, void*, gd::GJGameLevel*, bool, bool);
	inline int(__thiscall* playLayerOnExit)(int*, int);
	int __fastcall playLayerOnExitHook(int*, void*, int);

	inline static bool(__thiscall* levelEditorLayerInit)(int*, gd::GJGameLevel*, bool);
	bool __fastcall levelEditorLayerInitHook(int*, void*, gd::GJGameLevel*, bool);
	inline static int(__thiscall* levelEditorLayerOnExit)(int*, int);
	int __fastcall levelEditorLayerOnExitHook(int*, void*, int);

	inline static bool(__thiscall* menuLayerInit)(int*);
	bool __fastcall menuLayerInitHook(int*, void*);
};
