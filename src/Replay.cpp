#include "Replay.h"
#include "Hacks.h"

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
        file.write((char *)&a.dummy, sizeof(bool));
        file.write((char *)&a.frame, sizeof(uint32_t));
        file.write((char *)&a.yAccel, sizeof(double));
        file.write((char *)&a.px, sizeof(float));
        file.write((char *)&a.py, sizeof(float));
    }
    file.close();
    gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, ("Replay saved with " + std::to_string(GetActionsSize()) + " actions."))->show();
}