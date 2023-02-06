#include "ReplayPlayer.h"
#include "Practice.h"
#include "Hacks.h"
#include "PlayLayer.h"
#include "imgui.h"

void CustomCheckpoint::GetCheckpoint()
{
    c = ReplayPlayer::getInstance().GetPractice().CreateCheckpoint();
}

Checkpoint Practice::CreateCheckpoint()
{
    gd::PlayLayer *pl = gd::GameManager::sharedState()->getPlayLayer();
    Checkpoint c;
    if (pl)
    {
        ReplayPlayer &rp = ReplayPlayer::getInstance();
        c.p1 = CheckpointData::fromPlayer(pl->m_pPlayer1);
        c.p2 = CheckpointData::fromPlayer(pl->m_pPlayer2);
        c.frameOffset = rp.GetFrame();
        c.activatedObjectsSize = activatedObjects.size();
        c.activatedObjectsP2Size = activatedObjectsP2.size();
    }
    return c;
}

CustomCheckpoint *CustomCheckpoint::createHook()
{
    CustomCheckpoint *cc = new CustomCheckpoint;
    if (cc && cc->init())
    {
        cc->autorelease();
        cc->GetCheckpoint();
    }
    else
    {
        CC_SAFE_DELETE(cc);
    }
    if (PlayLayer::hadAction)
    {
        PlayLayer::respawnAction = cc->c.p1.isHolding + 1;
        if(gd::GameManager::sharedState()->getPlayLayer()->m_pLevelSettings->m_twoPlayerMode) PlayLayer::respawnAction2 = cc->c.p2.isHolding + 1;
    }
    else PlayLayer::respawnAction = 0;
    return cc;
}

CheckpointData CheckpointData::fromPlayer(gd::PlayerObject *p)
{
    CheckpointData cd;
    cd.xAccel = p->m_xAccel;
    cd.yAccel = p->m_yAccel;
    cd.jumpAccel = p->m_jumpAccel;
    cd.xPos = p->m_position.x;
    cd.yPos = p->m_position.y;
    cd.rotation = p->getRotation();
    cd.unkRot = p->m_fUnkRotationField;
    cd.playerSpeed = p->m_playerSpeed;
    cd.vehichleSize = p->m_vehicleSize;
    cd.decelerationRate = p->m_decelerationRate;
    cd.isHolding = p->m_isHolding;
    cd.mouseDown = ImGui::GetIO().MouseDown[0];
    cd.isHolding2 = p->m_isHolding2;
    cd.canRobotJump = p->m_canRobotJump;
    cd.isUpsideDown = p->m_isUpsideDown;
    cd.isOnGround = p->m_isOnGround;
    cd.isDashing = p->m_isDashing;
    cd.isSliding = p->m_isSliding;
    cd.isRising = p->m_isRising;
    cd.isLocked = p->m_isLocked;
    cd.unk630 = p->m_unk630;
    cd.unk631 = p->m_unk631;
    cd.isDropping = p->m_isDropping;
    cd.touchRing = p->m_touchingRings->count();
    cd.gamemode = GetGamemode(p);
    return cd;
}

int CheckpointData::Apply(gd::PlayerObject *p, bool tp)
{
    int out = 0;
    p->m_xAccel = xAccel;
    p->m_yAccel = yAccel;
    p->m_jumpAccel = jumpAccel;
    if (mouseDown != ImGui::GetIO().MouseDown[0])
    {
        out = ImGui::GetIO().MouseDown[0] ? 2 : 1; // 2 == press, 1 == release
    }

    if (tp && isHolding2 == p->m_isHolding2)
        out = p->m_isHolding2 ? 2 : 1;

    auto pl = gd::GameManager::sharedState()->getPlayLayer();

    if(out == 0 && p == pl->m_pPlayer1 && PlayLayer::respawnAction > 0) out = PlayLayer::respawnAction;
    if(out == 0 && p == pl->m_pPlayer2 && PlayLayer::respawnAction2 > 0) out = PlayLayer::respawnAction2;

    p->m_position.x = xPos;
    p->m_position.y = yPos;
    p->setPositionX(xPos);
    p->setPositionY(yPos);
    p->setRotation(rotation);
    p->m_fUnkRotationField = unkRot;
    p->m_playerSpeed = playerSpeed;
    p->m_vehicleSize = vehichleSize;
    p->m_decelerationRate = decelerationRate;
    p->m_hasJustHeld = hasJustHeld;
    p->m_hasJustHeld2 = hasJustHeld2;
    p->m_isHolding = isHolding;
    p->m_isHolding2 = isHolding2;
    p->m_canRobotJump = canRobotJump;
    p->m_isUpsideDown = isUpsideDown;
    p->m_isOnGround = isOnGround;
    p->m_isDashing = isDashing;
    p->m_isSliding = isSliding;
    p->m_isRising = isRising;
    p->m_isLocked = isLocked;
    p->m_unk630 = unk630;
    p->m_unk631 = unk631;
    p->m_isDropping = isDropping;
    SetGamemode(p, gamemode);
    return out;
}

Checkpoint Practice::GetLast()
{
    auto playLayer = gd::GameManager::sharedState()->getPlayLayer();
    if (playLayer && playLayer->m_checkpoints->count())
    {
        auto checkpoint_obj = dynamic_cast<CustomCheckpoint *>(playLayer->m_checkpoints->lastObject());
        if (checkpoint_obj)
        {
            return checkpoint_obj->c;
        }
    }
    return {};
}

void Practice::ApplyCheckpoint()
{
    auto playLayer = gd::GameManager::sharedState()->getPlayLayer();
    if (playLayer)
    {
        Checkpoint c = GetLast();
        auto click1 = c.p1.Apply(playLayer->m_pPlayer1, playLayer->m_pLevelSettings->m_twoPlayerMode);
        if (click1 != 0)
        {
            if (click1 == 2 && c.p1.touchRing <= 0)
            {
                PlayLayer::pushButton(playLayer->m_pPlayer1, 0);
                ReplayPlayer::getInstance().RecordAction(true, playLayer->m_pPlayer1, true);
            }
            else if (click1 == 1)
            {
                PlayLayer::releaseButton(playLayer->m_pPlayer1, 0);
                ReplayPlayer::getInstance().RecordAction(false, playLayer->m_pPlayer1, true);
            }
            else
                PlayLayer::releaseButton(playLayer->m_pPlayer1, 0);
        }

        if(!playLayer->m_bIsDualMode) return;

        auto click2 = c.p2.Apply(playLayer->m_pPlayer2, playLayer->m_pLevelSettings->m_twoPlayerMode);
        if (click2 != 0)
        {
            if (click2 == 2 && c.p2.touchRing <= 0)
            {
                PlayLayer::pushButton(playLayer->m_pPlayer2, 0);
                ReplayPlayer::getInstance().RecordAction(true, playLayer->m_pPlayer2, false);
            }
            else if (click2 == 1)
            {
                PlayLayer::releaseButton(playLayer->m_pPlayer2, 0);
                ReplayPlayer::getInstance().RecordAction(false, playLayer->m_pPlayer2, false);
            }
            else
                PlayLayer::releaseButton(playLayer->m_pPlayer2, 0);
        }
    }
}