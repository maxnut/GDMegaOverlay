#pragma once
#include "pch.h"

namespace LevelInfoLayer
{
	inline void(__thiscall* onBack)(gd::LevelInfoLayer* self, CCObject* sender);
	void __fastcall onBackHook(gd::LevelInfoLayer* self, void*, CCObject* sender);
}
