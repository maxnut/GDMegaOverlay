#pragma once
#include "pch.h"
#include <vector>
#include <filesystem>
#include <fstream>

struct Action
{
    bool press, player2, dummy;
    uint32_t frame;
    double yAccel;
    float px, py;
};

class Replay
{
protected:
    std::vector<Action> actions;

public:
    Replay(){};

    uint32_t GetActionsSize() { return actions.size(); }
    std::vector<Action> &getActions() { return actions; }
    void AddAction(Action a) { actions.push_back(a); }

    void ClearActions() { actions.clear(); }

    void RemoveActionsAfter(uint32_t frame)
    {
        const auto check = [&](const Action &action) -> bool
        {
            return action.frame >= frame;
        };
        actions.erase(std::remove_if(actions.begin(), actions.end(), check), actions.end());
    }

    void Load(std::string name)
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
        left -= sizeof(float);
        for (size_t _ = 0; _ <= left / 23U; ++_)
        {
            Action a;
            file.read((char *)&a.press, sizeof(bool));
            file.read((char *)&a.player2, sizeof(bool));
            file.read((char *)&a.dummy, sizeof(bool));
            file.read((char *)&a.frame, sizeof(uint32_t));
            file.read((char *)&a.yAccel, sizeof(double));
            file.read((char *)&a.px, sizeof(float));
            file.read((char *)&a.py, sizeof(float));
            AddAction(a);
        }
        file.close();
        gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, ("Replay loaded with " + std::to_string(GetActionsSize()) + " actions."))->show();
    }

    void Save(std::string name);
};