#pragma once

#include "HitboxNode.hpp"

namespace Hitbox
{

inline float lastPos = 0;

inline void(__thiscall* playLayerPostUpdate)(cocos2d::CCLayer* self, float dt);
void __fastcall playLayerPostUpdateHook(cocos2d::CCLayer* self, void*, float dt);

inline bool(__thiscall* playLayerInit)(cocos2d::CCLayer* self, void* level, bool idk1, bool idk2);
bool __fastcall playLayerInitHook(cocos2d::CCLayer* self, void*, void* level, bool idk1, bool idk2);

void initHooks();
} // namespace Hitbox