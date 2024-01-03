#pragma once

namespace ReplayLastCheckpoint
{

inline bool levelCompleted = false;

inline void(__thiscall* playLayerResetLevelFromStart)(void*);
void __fastcall playLayerResetLevelFromStartHook(void* self, void*);

inline void(__thiscall* playLayerLevelComplete)(void*);
void __fastcall playLayerLevelCompleteHook(void* self, void*);

inline int(__thiscall* playLayerResetLevel)(void*);
int __fastcall playLayerResetLevelHook(void* self, void*);

void initHooks();
} // namespace ReplayLastCheckpoint