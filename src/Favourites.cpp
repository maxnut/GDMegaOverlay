#include "Favourites.h"
#include "bools.h"
#include "Hacks.h"
#include "PlayLayer.h"

extern struct HacksStr hacks;
extern struct Labels labels;
extern struct Debug debug;

std::vector<Favourites::Favourite> Favourites::favourites;

void Favourites::Save()
{
    if (!std::filesystem::is_directory("GDMenu") || !std::filesystem::exists("GDMenu"))
        std::filesystem::create_directory("GDMenu");

    std::ofstream file("GDMenu/favourites.bin", std::ios::out | std::ios::binary);
    for (const auto &s : favourites)
    {
        file.write((char *)&s, sizeof(Favourite));
    }
    size_t integrityCheck = 19216810;
    file.write((char *)&integrityCheck, sizeof(size_t));
    file.close();
}

void Favourites::Load()
{
    if(favourites.size() > 0) return;
    std::ifstream file("GDMenu/favourites.bin", std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        return;
    }
    file.seekg(0, std::ios::end);
    size_t left = static_cast<size_t>(file.tellg());
    file.seekg(0);
    left -= sizeof(size_t);
    for (size_t _ = 0; _ < left / sizeof(Favourite); ++_)
    {
        Favourite s;
        file.read((char *)&s, sizeof(Favourite));
        favourites.push_back(s);
    }
    size_t integrityCheck;
    file.read((char *)&integrityCheck, sizeof(size_t));
    if(integrityCheck != 19216810)
    {
        favourites.clear();
    }
    file.close();
}