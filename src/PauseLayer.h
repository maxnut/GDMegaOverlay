#pragma once
#include "pch.h"



namespace PauseLayer
{
	inline bool(__thiscall* init)(CCLayer* self);
	bool __fastcall hook(CCLayer* self);
}