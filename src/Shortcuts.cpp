#include "Shortcuts.h"
#include "bools.h"
#include "Hacks.h"
#include "PlayLayer.h"

extern struct HacksStr hacks;
extern struct Labels labels;
extern struct Debug debug;

std::vector<Shortcuts::Shortcut> Shortcuts::shortcuts;

void Shortcuts::Save()
{
    if (!std::filesystem::is_directory("GDMenu") || !std::filesystem::exists("GDMenu"))
        std::filesystem::create_directory("GDMenu");

    Hacks::writeOutput((int)shortcuts.size());

    std::ofstream file("GDMenu/shortcuts.bin", std::ios::out | std::ios::binary);
    for (const auto &s : shortcuts)
    {
        file.write((char *)&s, sizeof(Shortcut));
    }
    size_t integrityCheck = 19216810;
    file.write((char *)&integrityCheck, sizeof(size_t));
    file.close();
}

void Shortcuts::Load()
{
    if(shortcuts.size() > 0) return;
    std::ifstream file("GDMenu/shortcuts.bin", std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        return;
    }
    file.seekg(0, std::ios::end);
    size_t left = static_cast<size_t>(file.tellg());
    file.seekg(0);
    left -= sizeof(size_t);
    for (size_t _ = 0; _ < left / sizeof(Shortcut); ++_)
    {
        Shortcut s;
        file.read((char *)&s, sizeof(Shortcut));
        shortcuts.push_back(s);
    }
    size_t integrityCheck;
    file.read((char *)&integrityCheck, sizeof(size_t));
    if(integrityCheck != 19216810)
    {
        shortcuts.clear();
    }
    file.close();
}