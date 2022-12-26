#include "ReplayPlayer.h"
#include "Practice.h"
#include "Hacks.h"
#include "PlayLayer.h"

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
    return cc;
}

int CheckpointData::Apply(gd::PlayerObject *p, bool tp)
{
    int out = 0;
    p->m_xAccel = xAccel;
    p->m_yAccel = yAccel;
    p->m_jumpAccel = jumpAccel;
    if (isHolding != p->m_isHolding)
    {
        out = p->m_isHolding ? 2 : 1; // 2 == press, 1 == release
    }

    if(tp && isHolding2 == p->m_isHolding2) out = p->m_isHolding2 ? 2 : 1;

    p->m_position.x = xPos;
    p->m_position.y = yPos;
    p->setRotationX(rotationX);
    p->setRotationY(rotationY);
    p->m_playerSpeed = playerSpeed;
    p->m_vehicleSize = vehichleSize;
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
    p->m_unk662 = unk662;
    p->m_unk630 = unk630;
    p->m_unk631 = unk631;
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