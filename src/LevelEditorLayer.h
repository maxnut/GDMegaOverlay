#pragma once
#include "pch.h"



namespace LevelEditorLayer
{

	inline void(__thiscall* draw)(gd::LevelEditorLayer* self);
	void __fastcall drawHook(gd::LevelEditorLayer* self, void*);

	inline void(__thiscall* onPlaytest)(gd::LevelEditorLayer* self);
	void __fastcall onPlaytestHook(gd::LevelEditorLayer* self, void*);

	inline void(__thiscall* exit)(CCLayer* self, CCObject* sender);
	void __fastcall exitHook(CCLayer* self, void*, CCObject* sender);

	inline void(__thiscall* fadeMusic)(gd::GameManager* self, char* idk);
	void __fastcall fadeMusicHook(gd::GameManager* self, void*, char* idk);
}