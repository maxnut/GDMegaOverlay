#pragma once

namespace Speedhack
{
inline void(__thiscall* CCSchedulerUpdate)(void*, float);
void __fastcall CCSchedulerUpdateHook(void* self, void*, float dt);

void initHooks();
}; // namespace Speedhack