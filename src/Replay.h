#pragma once
#include "pch.h"
#include <vector>
#include <filesystem>
#include <fstream>

struct Action
{
    bool press, player2;
    uint32_t frame;
    double yAccel;
    float px, py;
};

class Replay
{
protected:
    std::vector<Action> actions;

public:
    float fps = 360.0f;
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

    void Load(std::string name);

    void Save(std::string name);
};