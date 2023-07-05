#pragma once
#include "pch.h"

class Favourites
{

public:
    struct Favourite
    {
        enum ftype { checkbox, button };
        ftype type;
        char name[30];
        bool* checkboxPointer;
    };
    static std::vector<Favourite> favourites;

    static void Save();
    static void Load();
};