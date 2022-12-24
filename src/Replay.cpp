#include "Replay.h"
#include "Hacks.h"

void Replay::Load(std::string name)
{
        ClearActions();
        std::ifstream file("GDMenu/replays/" + name + ".replay", std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, ("Could not find replay"))->show();
            return;
        }
        file.seekg(0, std::ios::end);
        size_t left = static_cast<size_t>(file.tellg());
        file.seekg(0);
        left -= sizeof(float) * 2;
        for (size_t _ = 0; _ <= left / 22U; ++_)
        {
            Action a;
            file.read((char *)&a.press, sizeof(bool));
            file.read((char *)&a.player2, sizeof(bool));
            file.read((char *)&a.frame, sizeof(uint32_t));
            file.read((char *)&a.yAccel, sizeof(double));
            file.read((char *)&a.px, sizeof(float));
            file.read((char *)&a.py, sizeof(float));
            AddAction(a);
        }
        file.read((char *)&fps, sizeof(float));
        Hacks::writeOutput(fps);
        file.close();
        gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, ("Replay loaded with " + std::to_string(GetActionsSize()) + " actions."))->show();
    }

void Replay::Save(std::string name)
{
    if (GetActionsSize() <= 0)
    {
        gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, ("Replay must contain at least one action"))->show();
        return;
    }
    if (!std::filesystem::is_directory("GDMenu/replays") || !std::filesystem::exists("GDMenu/replays"))
        std::filesystem::create_directory("GDMenu/replays");

    std::ofstream file("GDMenu/replays/" + name + ".replay", std::ios::out | std::ios::binary);
    for (const auto &a : actions)
    {
        file.write((char *)&a.press, sizeof(bool));
        file.write((char *)&a.player2, sizeof(bool));
        file.write((char *)&a.frame, sizeof(uint32_t));
        file.write((char *)&a.yAccel, sizeof(double));
        file.write((char *)&a.px, sizeof(float));
        file.write((char *)&a.py, sizeof(float));
    }
    file.write((char *)&fps, sizeof(float));
    file.close();
    gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, ("Replay saved with " + std::to_string(GetActionsSize()) + " actions."))->show();
}