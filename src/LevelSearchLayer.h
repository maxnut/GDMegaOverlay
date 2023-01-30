#pragma once
#include "pch.h"
#include "gdstring.h"

namespace LevelSearchLayer
{
	inline bool(__thiscall *init)(gd::LevelSearchLayer *self);
	bool __fastcall hook(gd::LevelSearchLayer *self);

	inline void(__thiscall *http)(gd::GameLevelManager *self, gd::string gdurl, gd::string gdquery, gd::string idk, int type);
	void __fastcall httpHook(gd::GameLevelManager *self, void*, gd::string gdurl, gd::string gdquery, gd::string idk, int type);
}