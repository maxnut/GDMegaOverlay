#pragma once
#include "pch.h"



namespace MenuLayer
{
	inline bool(__thiscall* init)(CCLayer* self);
	bool __fastcall hook(CCLayer* self);

	inline void(__thiscall* onBack)(CCLayer* self, cocos2d::CCObject* sender);
	void __fastcall onBackHook(CCLayer* self, void*, cocos2d::CCObject* sender);

	inline const char*(__thiscall* loadingString)(CCLayer* self);
	const char* __fastcall loadingStringHook(CCLayer* self, void*);
}