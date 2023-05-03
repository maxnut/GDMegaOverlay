#pragma once
#include "pch.h"

extern int hackAmts[];
extern int shortcutIndexKey;

class Shortcuts
{

public:
    struct Shortcut
    {
        int key;
        char name[30];
    };
    static std::vector<Shortcut> shortcuts;

    static void Save();
    static void Load();
};