#pragma once
#include "pch.h"



namespace EndLevelLayer
{
	extern int deaths;
	extern std::string accuracy;

	inline void(__thiscall* customSetup)(CCLayer* self);
	void __fastcall customSetupHook(CCLayer* self, void*);
}