#pragma once

#include <vector>

namespace StartposSwitcher
{
    inline std::vector<int*> startposObjects;
    inline int index = -1;

    void initHooks();

    void change(bool right);

    inline int*(__thiscall* create)();
    int* __fastcall createHook();

    inline int(__thiscall* playLayerQuit)(int* self);
    int __fastcall playLayerQuitHook(int* self, void*);
}