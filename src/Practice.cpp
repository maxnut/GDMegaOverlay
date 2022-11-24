#include "ReplayPlayer.h"
#include "Practice.h"
#include "Hacks.h"

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

void CheckpointData::Apply(gd::PlayerObject *p, bool addedAction) const
{
    p->m_yAccel = yAccel;
    p->setRotation(rotation);
    auto playLayer = gd::GameManager::sharedState()->getPlayLayer();

    if (ReplayPlayer::getInstance().IsRecording() && !addedAction)
    {
        if (p == playLayer->m_pPlayer1)
            ReplayPlayer::getInstance().RecordAction(true, p, true, true);
        else if (p == playLayer->m_pPlayer2 && playLayer->m_bIsDualMode)
            ReplayPlayer::getInstance().RecordAction(true, p, false, true);
    }
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