#pragma once
#include "pch.h"

struct CheckpointData
{
    double yAccel;
    float rotation;
    bool buffer;
    static CheckpointData fromPlayer(gd::PlayerObject *p)
    {
        CheckpointData cd;
        cd.yAccel = p->m_yAccel;
        cd.rotation = p->getRotation();
        cd.buffer = p->m_hasJustHeld;
        return cd;
    }

    void Apply(gd::PlayerObject *p, bool addedAction) const;
};

struct Checkpoint
{
    CheckpointData p1, p2;
    uint32_t frameOffset;
};

class CustomCheckpoint : public gd::CheckpointObject
{
public:
    Checkpoint c;

    void GetCheckpoint();

    static inline CustomCheckpoint(__thiscall *create)();
    static CustomCheckpoint *createHook();
};

class Practice
{

public:
    Checkpoint CreateCheckpoint();

    Checkpoint GetLast();

    void ApplyCheckpoint(bool addedAction)
    {
        auto playLayer = gd::GameManager::sharedState()->getPlayLayer();
        if (playLayer)
        {
            const Checkpoint c = GetLast();
            c.p1.Apply(playLayer->m_pPlayer1, addedAction);
            c.p2.Apply(playLayer->m_pPlayer2, addedAction);
        }
    }
};