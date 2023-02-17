#include "ReplayPlayer.h"
#include "bools.h"
#include "PlayLayer.h"
#include "Hacks.h"
#include "ctime"
#include "fmod.hpp"

extern struct HacksStr hacks;
FMOD::System *sys;
std::vector<FMOD::Sound *> clicks, releases, mediumclicks;
FMOD::Channel *channel = 0, *channel2 = 0;

int Hacks::amountOfClicks = 0;
int Hacks::amountOfMediumClicks = 0;
int Hacks::amountOfReleases = 0;

bool oldClick = false;

double oldTime = 0, oldTimeP2 = 0;

uint32_t ReplayPlayer::GetFrame()
{
    gd::PlayLayer *pl = gd::GameManager::sharedState()->getPlayLayer();
    if (pl)
        return static_cast<uint32_t>((float)pl->m_time * hacks.fps) + frameOffset;
    else
        return -1;
}

void ReplayPlayer::StartPlaying(gd::PlayLayer *playLayer)
{
    UpdateFrameOffset();
    actionIndex = 0;
}

ReplayPlayer::ReplayPlayer()
{
    srand(time(NULL));
    FMOD::System_Create(&sys);
    sys->init(32, FMOD_INIT_NORMAL, nullptr);

    clicks.clear();
    releases.clear();
    mediumclicks.clear();

    bool exist = std::filesystem::exists("GDMenu/clicks/clicks/1.wav");
    uint32_t i = 1;
    while (exist)
    {
        auto filestr = "GDMenu/clicks/clicks/" + std::to_string(i) + ".wav";
        FMOD::Sound *s;
        sys->createSound(filestr.c_str(), FMOD_LOOP_OFF, 0, &s);
        if (s)
            clicks.push_back(s);
        i++;
        filestr = "GDMenu/clicks/clicks/" + std::to_string(i) + ".wav";
        exist = std::filesystem::exists(filestr);
    }
    Hacks::amountOfClicks = clicks.size();

    exist = std::filesystem::exists("GDMenu/clicks/releases/1.wav");
    i = 1;
    while (exist)
    {
        auto filestr = "GDMenu/clicks/releases/" + std::to_string(i) + ".wav";
        FMOD::Sound *s;
        sys->createSound(filestr.c_str(), FMOD_LOOP_OFF, 0, &s);
        if (s)
            releases.push_back(s);
        i++;
        filestr = "GDMenu/clicks/releases/" + std::to_string(i) + ".wav";
        exist = std::filesystem::exists(filestr);
    }
    Hacks::amountOfReleases = releases.size();

    exist = std::filesystem::exists("GDMenu/clicks/mediumclicks/1.wav");
    i = 1;
    while (exist)
    {
        auto filestr = "GDMenu/clicks/mediumclicks/" + std::to_string(i) + ".wav";
        FMOD::Sound *s;
        sys->createSound(filestr.c_str(), FMOD_LOOP_OFF, 0, &s);
        if (s)
            mediumclicks.push_back(s);
        i++;
        filestr = "GDMenu/clicks/mediumclicks/" + std::to_string(i) + ".wav";
        exist = std::filesystem::exists(filestr);
    }
    Hacks::amountOfMediumClicks = mediumclicks.size();
}

void ReplayPlayer::ToggleRecording()
{
    playing = false;
    recording = !recording;

    replay.fps = hacks.fps;
    Hacks::FPSBypass(replay.fps);
    hacks.tpsBypass = replay.fps;
    Hacks::tps = hacks.tpsBypass;

    if (replay.GetActionsSize() > 0 && IsRecording() && hacks.actionStart <= 0)
    {
        replay.ClearActions();
    }
}

void ReplayPlayer::TogglePlaying()
{
    recording = false;
    playing = !playing;
    Hacks::FPSBypass(hacks.fps);
    hacks.tpsBypass = hacks.fps;
    Hacks::tps = hacks.tpsBypass;
    Hacks::level["mods"][24]["toggle"] = true;
    Hacks::ToggleJSONHack(Hacks::level, 24, false);
}

void ReplayPlayer::Reset(gd::PlayLayer *playLayer)
{
    oldTime = 0;

    bool addedAction = false;
    oldClick = false;

    if (IsPlaying())
    {
        UpdateFrameOffset();
        actionIndex = hacks.actionStart;
        playLayer->releaseButton(0, false);
        playLayer->releaseButton(0, true);
        practice.activatedObjects.clear();
        practice.activatedObjectsP2.clear();
        if (playLayer->m_checkpoints->count() <= 0)
            playLayer->m_totalTime = 0;
    }
    else
    {
        bool hasCheckpoint = playLayer->m_checkpoints->count() > 0;
        const auto checkpoint = practice.GetLast();
        if (!hasCheckpoint)
        {
            practice.activatedObjects.clear();
            practice.activatedObjectsP2.clear();
            frameOffset = 0;
            playLayer->m_totalTime = 0;
        }
        else
        {
            frameOffset = checkpoint.frameOffset;
            constexpr auto delete_from = [&](auto &vec, size_t index)
            {
                vec.erase(vec.begin() + index, vec.end());
            };
            delete_from(practice.activatedObjects, checkpoint.activatedObjectsSize);
            delete_from(practice.activatedObjectsP2, checkpoint.activatedObjectsP2Size);
            if (IsRecording())
            {
                for (const auto &object : practice.activatedObjects)
                {
                    object->m_bHasBeenActivated = true;
                }
                for (const auto &object : practice.activatedObjectsP2)
                {
                    object->m_bHasBeenActivatedP2 = true;
                }
            }
        }

        if (IsRecording())
        {
            replay.RemoveActionsAfter(GetFrame());
            if (playLayer->m_pLevelSettings->m_twoPlayerMode)
                RecordAction(false, playLayer->m_pPlayer2, false);
        }
    }

    if ((IsRecording() || hacks.fixPractice) && playLayer->m_isPracticeMode && playLayer->m_checkpoints->count() > 0)
        practice.ApplyCheckpoint();

    if (IsPlaying() || IsRecording() && GetActionsSize() <= 0)
    {
        PlayLayer::releaseButton(playLayer->m_pPlayer1, 0);
        if (playLayer->m_bIsDualMode)
        {
            PlayLayer::releaseButton(playLayer->m_pPlayer2, 0);
        }
    }
}

void ReplayPlayer::Load(std::string name)
{
    replay.Load(name);
}

void ReplayPlayer::Update(gd::PlayLayer *playLayer)
{
    sys->update();

    if ((replay.fps != hacks.fps || hacks.tpsBypass != replay.fps) && (IsPlaying() || IsRecording()))
    {
        hacks.fps = replay.fps;
        Hacks::FPSBypass(hacks.fps);
        hacks.tpsBypass = hacks.fps;
        Hacks::tps = hacks.tpsBypass;
    }

    if (!IsPlaying() || actionIndex >= replay.getActions().size() || replay.getActions().size() <= 0 || playLayer->m_hasCompletedLevel || playLayer->m_isDead)
        return;

    size_t limit = 1;
    while (actionIndex + limit < replay.getActions().size() && (replay.getActions()[actionIndex + limit].player2 || (replay.getActions()[actionIndex].frame == replay.getActions()[actionIndex + limit].frame || replay.getActions()[actionIndex].px >= 0 && replay.getActions()[actionIndex].px == replay.getActions()[actionIndex + limit].px)))
        limit++;

    for (size_t i = 0; i < limit; i++)
    {
        auto ac = replay.getActions()[actionIndex];
        if (ac.frame >= 0 && GetFrame() >= ac.frame || playLayer->m_pPlayer1->m_position.x >= ac.px && ac.px >= 0)
        {
            if (!ac.player2 && !hacks.disableBotCorrection && ac.yAccel >= 0)
            {
                playLayer->m_pPlayer1->m_position.x = ac.px;
                playLayer->m_pPlayer1->m_yAccel = ac.yAccel;
                playLayer->m_pPlayer1->m_position.y = ac.py;
            }
            else if (!hacks.disableBotCorrection && ac.yAccel >= 0)
            {
                playLayer->m_pPlayer2->m_position.x = ac.px;
                playLayer->m_pPlayer2->m_yAccel = ac.yAccel;
                playLayer->m_pPlayer2->m_position.y = ac.py;
            }

            float v;
            float p;
            double t;
            uint16_t rc, rr, rmc;

            if (playLayer->m_pLevelSettings->m_twoPlayerMode && ac.player2)
                t = playLayer->m_time - oldTimeP2;
            else
                t = playLayer->m_time - oldTime;

            if (hacks.clickbot && clicks.size() > 0 && releases.size() > 0)
            {
                rc = rand() % clicks.size();
                rr = rand() % releases.size();
                if (Hacks::amountOfMediumClicks > 0)
                    rmc = rand() % mediumclicks.size();
                p = hacks.minPitch + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (hacks.maxPitch - hacks.minPitch)));
                v = t >= hacks.playMediumClicksAt ? 0.5f * ((float)t * 6) * hacks.baseVolume : 0.6f * hacks.baseVolume;
                if (v > 0.5f * hacks.baseVolume)
                    v = 0.5f * hacks.baseVolume;
            }
            if (ac.press)
            {
                if (hacks.clickbot && t > hacks.minTimeDifference && Hacks::amountOfClicks > 0 && ac.press != oldClick)
                {
                    sys->playSound(t > 0.0 && t < hacks.playMediumClicksAt && Hacks::amountOfMediumClicks > 0 ? mediumclicks[rmc] : clicks[rc], nullptr, false, &channel);
                    channel->setPitch(p);
                    channel->setVolume(v);
                }
                if (playLayer->m_pLevelSettings->m_twoPlayerMode)
                    oldTimeP2 = playLayer->m_time;
                else
                    oldTime = playLayer->m_time;
                PlayLayer::isBot = true;
                if (ac.player2)
                {
                    PlayLayer::pushButtonHook(playLayer->m_pPlayer2, 0, 0);
                    if (!playLayer->m_pLevelSettings->m_twoPlayerMode)
                        PlayLayer::pushButtonHook(playLayer->m_pPlayer1, 0, 0);
                }
                else
                {
                    PlayLayer::pushButtonHook(playLayer->m_pPlayer1, 0, 0);
                    if (!playLayer->m_pLevelSettings->m_twoPlayerMode)
                        PlayLayer::pushButtonHook(playLayer->m_pPlayer2, 0, 0);
                }
                PlayLayer::isBot = false;
            }
            else
            {
                if (hacks.clickbot && Hacks::amountOfReleases > 0 && ac.press != oldClick)
                {
                    sys->playSound(releases[rr], nullptr, false, &channel);
                    channel2->setPitch(p);
                    channel2->setVolume(v + 0.5f);
                }
                if (playLayer->m_pLevelSettings->m_twoPlayerMode)
                    oldTimeP2 = playLayer->m_time;
                else
                    oldTime = playLayer->m_time;
                PlayLayer::isBot = true;
                if (ac.player2)
                {
                    PlayLayer::releaseButtonHook(playLayer->m_pPlayer2, 0, 0);
                    if (!playLayer->m_pLevelSettings->m_twoPlayerMode)
                        PlayLayer::releaseButtonHook(playLayer->m_pPlayer1, 0, 0);
                }
                else
                {
                    PlayLayer::releaseButtonHook(playLayer->m_pPlayer1, 0, 0);
                    if (!playLayer->m_pLevelSettings->m_twoPlayerMode)
                        PlayLayer::releaseButtonHook(playLayer->m_pPlayer2, 0, 0);
                }
                PlayLayer::isBot = false;
            }

            oldClick = ac.press;

            actionIndex++;
        }
    }
}

void ReplayPlayer::UpdateFrameOffset()
{
    frameOffset = GetPractice().GetLast().frameOffset;
}

void ReplayPlayer::HandleActivatedObjects(bool a, bool b, gd::GameObject *object)
{
    auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
    if (play_layer && play_layer->m_isPracticeMode && IsRecording())
    {
        if (object->m_bHasBeenActivated && !a)
            GetPractice().activatedObjects.push_back(object);
        if (object->m_bHasBeenActivatedP2 && !b)
            GetPractice().activatedObjectsP2.push_back(object);
    }
}

void ReplayPlayer::RecordAction(bool press, gd::PlayerObject *pl, bool player1)
{
    if (!IsRecording())
        return;
    Action a;
    a.player2 = !player1;
    a.frame = GetFrame();
    a.press = press;
    a.yAccel = pl->m_yAccel;
    a.px = pl->m_position.x;
    a.py = pl->m_position.y;
    replay.AddAction(a);
}