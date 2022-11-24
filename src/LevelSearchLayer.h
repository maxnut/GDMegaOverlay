#pragma once
#include "pch.h"



namespace LevelSearchLayer
{
	inline bool(__thiscall* init)(CCLayer* self);
	bool __fastcall hook(CCLayer* self);

	inline bool(__thiscall* test)(gd::LevelInfoLayer* self);
	bool __fastcall testhook(gd::LevelInfoLayer* self, void*);
}