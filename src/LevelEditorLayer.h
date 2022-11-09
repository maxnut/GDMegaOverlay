#pragma once
#include "pch.h"



namespace LevelEditorLayer
{

	inline void(__thiscall* draw)(gd::LevelEditorLayer* self);
	void __fastcall drawHook(gd::LevelEditorLayer* self, void*);
}