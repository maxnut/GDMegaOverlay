#pragma once

namespace ReplayLastCheckpoint
{
inline void(__thiscall* playLayerResetLevelFromStart)(void*);
void __fastcall playLayerResetLevelFromStartHook(void* self, void*);

void initHooks();
} // namespace ReplayLastCheckpoint