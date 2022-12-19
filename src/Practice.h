#pragma once
#include "pch.h"

struct CheckpointData
{
    double xAccel, yAccel, jumpAccel;
    float xPos, yPos, rotationX, rotationY, playerSpeed, vehichleSize;
    bool hasJustHeld, hasJustHeld2, isHolding, isHolding2, canRobotJump, isUpsideDown, isOnGround, isDashing, isRising, isSliding, unk662, unk630, unk631;
    size_t touchRing;
    gd::Gamemode gamemode;
    static CheckpointData fromPlayer(gd::PlayerObject *p)
    {
        CheckpointData cd;
        cd.xAccel = p->m_xAccel;
        cd.yAccel = p->m_yAccel;
        cd.jumpAccel = p->m_jumpAccel;
        cd.xPos = p->m_position.x;
        cd.yPos = p->m_position.y;
        cd.rotationX = p->getRotationX();
        cd.rotationY = p->getRotationY();
        cd.playerSpeed = p->m_playerSpeed;
        cd.vehichleSize = p->m_vehicleSize;
        cd.hasJustHeld = p->m_hasJustHeld;
        cd.hasJustHeld2 = p->m_hasJustHeld2;
        cd.isHolding = p->m_isHolding;
        cd.isHolding2 = p->m_isHolding2;
        cd.canRobotJump = p->m_canRobotJump;
        cd.isUpsideDown = p->m_isUpsideDown;
        cd.isOnGround = p->m_isOnGround;
        cd.isDashing = p->m_isDashing;
        cd.isSliding = p->m_isSliding;
        cd.isRising = p->m_isRising;
        cd.unk662 = p->m_unk662;
        cd.unk630 = p->m_unk630;
        cd.unk631 = p->m_unk631;
        cd.touchRing = p->m_touchingRings->count();
        cd.gamemode = GetGamemode(p);
        return cd;
    }

    int Apply(gd::PlayerObject *p);

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

    friend class ReplaySystem;

public:
    std::vector<gd::GameObject *> activatedObjects;
    std::vector<gd::GameObject *> activatedObjectsP2;

    Checkpoint CreateCheckpoint();

    Checkpoint GetLast();

    void ApplyCheckpoint();
};