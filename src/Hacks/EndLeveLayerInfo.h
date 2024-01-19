#pragma once

namespace EndLevelLayerInfo
{
	void initHooks();

	inline void(__thiscall* endLevelLayerCustomSetup)(cocos2d::CCLayer*);
	void __fastcall endLevelLayerCustomSetupHook(cocos2d::CCLayer*, void*);
}