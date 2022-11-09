#pragma once
#include "pch.h"



namespace LevelSearchLayer
{
	inline bool(__thiscall* init)(CCLayer* self);
	bool __fastcall hook(CCLayer* self);
}