#pragma once
#include "pch.h"
#include "HitboxNode.hpp"

namespace PlayLayer
{

	inline bool(__thiscall* init)(gd::PlayLayer* self, gd::GJGameLevel* level);
	bool __fastcall initHook(gd::PlayLayer* self, void*, gd::GJGameLevel* level);

	inline void(__thiscall* update)(gd::PlayLayer* self, float dt);
	void __fastcall updateHook(gd::PlayLayer* self, void*, float dt);

	inline void(__thiscall* destroyPlayer)(gd::PlayLayer* self, gd::PlayerObject*, gd::GameObject*);
	void __fastcall destroyPlayer_H(gd::PlayLayer* self, void*, gd::PlayerObject* player, gd::GameObject* obj);

	inline bool(__thiscall* pushButton)(gd::PlayerObject* self, int PlayerButton);
	bool __fastcall pushButtonHook(gd::PlayerObject* self, void*, int PlayerButton);

	inline void(__thiscall* pauseGame)(gd::PlayLayer* self, bool idk);
	void __fastcall pauseGameHook(gd::PlayLayer* self, void*, bool idk);

	inline bool(__thiscall* releaseButton)(gd::PlayerObject* self, int PlayerButton);
	bool __fastcall releaseButtonHook(gd::PlayerObject* self, void*, int PlayerButton);

	inline void(__thiscall* dispatchKeyboardMSG)(void* self, int key, bool down);
	void __fastcall dispatchKeyboardMSGHook(void* self, void*, int key, bool down);

	inline bool(__thiscall* editorInit)(gd::LevelEditorLayer *self, gd::GJGameLevel *lvl);
	bool __fastcall editorInitHook(gd::LevelEditorLayer *self, void *, gd::GJGameLevel *lvl);

	inline void(__thiscall* resetLevel)(gd::PlayLayer* self);
	void __fastcall resetLevelHook(gd::PlayLayer* self, void*);

	inline void(__thiscall* onQuit)(gd::PlayLayer* self);
	void __fastcall onQuitHook(gd::PlayLayer* self, void*);

	inline void(__thiscall* triggerObject)(gd::EffectGameObject* self, gd::GJBaseGameLayer* idk);
	void __fastcall triggerObjectHook(gd::EffectGameObject* self, void*, gd::GJBaseGameLayer* idk);

	inline void(__thiscall* lightningFlash)(gd::PlayLayer* self, CCPoint p, _ccColor3B c);
	void __fastcall lightningFlashHook(gd::PlayLayer* self, void* edx, CCPoint p, _ccColor3B c);

	inline void(__thiscall* togglePracticeMode)(gd::PlayLayer* self, bool on);
	void __fastcall togglePracticeModeHook(gd::PlayLayer* self, void* edx, bool on);

	inline gd::GameSoundManager*(__thiscall* levelComplete)(gd::PlayLayer* self);
	gd::GameSoundManager* __fastcall levelCompleteHook(gd::PlayLayer* self);

	inline void(__thiscall* death)(void* self, void* go, void* powerrangers);
	void __fastcall hkDeath(void* self, void*, void* go, void* powerrangers);

	void SetHitboxSize(float size);

	void UpdatePositions(int index);

	void Quit();

}