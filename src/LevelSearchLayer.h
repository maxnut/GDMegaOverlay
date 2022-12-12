#pragma once
#include "pch.h"

namespace LevelSearchLayer
{
	inline bool(__thiscall *init)(gd::LevelSearchLayer *self);
	bool __fastcall hook(gd::LevelSearchLayer *self);

	inline void(__thiscall *http)(gd::GameLevelManager *self, std::string gdurl, std::string gdquery, std::string idk, int type);
	void __fastcall httpHook(gd::GameLevelManager *self, void*, std::string gdurl, std::string gdquery, std::string idk, int type);
}