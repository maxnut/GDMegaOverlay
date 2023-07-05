#pragma once
#include "pch.h"

CCRotateBy* runNormalRotation(gd::PlayerObject* player, float rate);

struct CheckpointData
{
    double xAccel, yAccel, jumpAccel;
    float xPos, yPos, rotation, rotRate = 0, unkRot, playerSpeed, vehichleSize, decelerationRate, lastJumpTime, ballRotationElapsed = 0;
    bool hasJustHeld, hasJustHeld2, isHolding, isHolding2, canRobotJump, isUpsideDown, isOnGround, isDashing, isRising, isSliding, isLocked, isDropping, mouseDown, isOnSlope, wasOnSlope;
    size_t touchRing;
    gd::Gamemode gamemode;
    float rotationElapsed;
    gd::GameObject* objSnap;
    static CheckpointData fromPlayer(gd::PlayerObject *p);

    int Apply(gd::PlayerObject *p, bool tp);

    static gd::Gamemode GetGamemode(gd::PlayerObject *p)
    {
        if (p->m_isShip)
        {
            return gd::Gamemode::kGamemodeShip;
        }
        else if (p->m_isBird)
        {
            return gd::Gamemode::kGamemodeUfo;
        }
        else if (p->m_isBall)
        {
            return gd::Gamemode::kGamemodeBall;
        }
        else if (p->m_isDart)
        {
            return gd::Gamemode::kGamemodeWave;
        }
        else if (p->m_isRobot)
        {
            return gd::Gamemode::kGamemodeRobot;
        }
        else if (p->m_isSpider)
        {
            return gd::Gamemode::kGamemodeSpider;
        }
        else
        {
            return gd::Gamemode::kGamemodeCube;
        }
    }

    static void SetGamemode(gd::PlayerObject *p, gd::Gamemode gm)
    {
        p->m_isShip = gm == gd::Gamemode::kGamemodeShip;
        p->m_isBird = gm == gd::Gamemode::kGamemodeUfo;
        p->m_isBall = gm == gd::Gamemode::kGamemodeBall;
        p->m_isDart = gm == gd::Gamemode::kGamemodeWave;
        p->m_isRobot = gm == gd::Gamemode::kGamemodeRobot;
        p->m_isSpider = gm == gd::Gamemode::kGamemodeSpider;
    }
};

struct Checkpoint
{
    size_t activatedObjectsSize;
    size_t activatedObjectsP2Size;
    CheckpointData p1, p2;
    std::vector<float> ringRotations;
    uint32_t frameOffset = 0;
    float dt;
};

class CustomCheckpoint : public gd::CheckpointObject
{
public:
    Checkpoint c;

    void GetCheckpoint();

    static inline gd::CheckpointObject*(__thiscall *create)();
    static CustomCheckpoint *createHook();
};

class Practice
{

    friend class ReplaySystem;

public:
    std::vector<gd::GameObject *> activatedObjects;
    std::vector<gd::GameObject *> activatedObjectsP2;

    Checkpoint CreateCheckpoint();

    Checkpoint GetLast();

    void ApplyCheckpoint();

    static void SaveCheckpoints();
    static void LoadCheckpoints();
};