#pragma once
#include "pch.h"

namespace PlayLayer
{

	extern bool isBot, hadAction, wasPaused;
	extern int respawnAction, respawnAction2;
	extern float player1RotRate, player2RotRate;

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

	inline void(__thiscall* playGravityEffect)(gd::PlayLayer* self, bool idk);
	void __fastcall playGravityEffectHook(gd::PlayLayer* self, void*, bool idk);

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

	inline void(__thiscall* uiOnPause)(gd::UILayer* self, CCObject *obj);
	void __fastcall uiOnPauseHook(gd::UILayer* self, void*, CCObject *obj);

	inline void(__thiscall* togglePlayerScale)(gd::PlayerObject* self, bool toggle);
	void __fastcall togglePlayerScaleHook(gd::PlayerObject* self, void*, bool toggle);

	inline void(__thiscall* toggleDartMode)(gd::PlayerObject* self, bool toggle);
	void __fastcall toggleDartModeHook(gd::PlayerObject* self, void*, bool toggle);

	inline void(__thiscall* uiTouchBegan)(gd::UILayer* self, cocos2d::CCTouch* touch, cocos2d::CCEvent* evnt);
	void __fastcall uiTouchBeganHook(gd::UILayer* self, void*, cocos2d::CCTouch* touch, cocos2d::CCEvent* evnt);

	inline void(__thiscall* ringJump)(gd::PlayerObject* self, gd::GameObject* ring);
	void __fastcall ringJumpHook(gd::PlayerObject* self, void*, gd::GameObject* ring);

	inline void(__thiscall* newBest)(gd::PlayLayer* self, bool b1, int i1, int i2, bool b2, bool b3, bool b4);
	void __fastcall newBestHook(gd::PlayLayer* self, void*, bool b1, int i1, int i2, bool b2, bool b3, bool b4);

	inline void(__thiscall* activateObject)(gd::GameObject* self, gd::PlayerObject* player);
	void __fastcall activateObjectHook(gd::GameObject* self, void*, gd::PlayerObject* player);

	inline void*(__thiscall* getObjectRect)(cocos2d::CCNode *obj, gd::GameObject* self, float w, float h);
	void* __fastcall getObjectRectHook(cocos2d::CCNode *obj, void*, gd::GameObject* self, float w, float h);

	inline void*(__thiscall* getObjectRect2)(cocos2d::CCNode *obj, float w, float h);
	void* __fastcall getObjectRectHook2(cocos2d::CCNode *obj, void*, float w, float h);

	inline void(__thiscall* bump)(gd::GJBaseGameLayer* self, gd::PlayerObject* player, gd::GameObject* object);
	void __fastcall bumpHook(gd::GJBaseGameLayer* self, void*, gd::PlayerObject* player, gd::GameObject* object);

	inline void(__thiscall* flipGravity)(gd::PlayLayer *self, gd::PlayerObject* player, bool idk, bool idk2);
	void __fastcall flipGravityHook(gd::PlayLayer *self, void*, gd::PlayerObject* player, bool idk, bool idk2);

	inline gd::GameObject*(__thiscall* hasBeenActivatedByPlayer)(gd::GameObject *self, gd::GameObject *other);
	gd::GameObject* __fastcall hasBeenActivatedByPlayerHook(gd::GameObject *self, void*, gd::GameObject *other);

	inline gd::GameObject*(__thiscall* powerOffObject)(gd::GameObject *self);
	gd::GameObject* __fastcall powerOffObjectHook(gd::GameObject *self);

	inline gd::GameObject*(__thiscall* playShineEffect)(gd::GameObject *self);
	gd::GameObject* __fastcall playShineEffectHook(gd::GameObject *self);

	inline void(__thiscall* addPoint)(gd::HardStreak *self, CCPoint point);
	void __fastcall addPointHook(gd::HardStreak *self, void*, CCPoint point);

	


	void SetHitboxSize(float size);

	void UpdatePositions(int index);

	void Quit();

	bool IsCheating();

	float GetStartPercent();

	void SyncMusic();

}