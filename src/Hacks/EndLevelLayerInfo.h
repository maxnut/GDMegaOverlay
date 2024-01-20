#pragma once

namespace EndLevelLayerInfo
{
	void initHooks();

	inline void(__thiscall* endLevelLayerCustomSetup)(cocos2d::CCLayer*);
	void __fastcall endLevelLayerCustomSetupHook(cocos2d::CCLayer*, void*);

	inline void(__thiscall* endLevelLayerPlayEndEffect)(cocos2d::CCLayer*, bool);
	void __fastcall endLevelLayerPlayEndEffectHook(cocos2d::CCLayer*, void*, bool);
}