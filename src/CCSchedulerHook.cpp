#include "CCSchedulerHook.h"
#include "ReplayPlayer.h"
#include "Hacks.h"

extern struct HacksStr hacks;

extern struct HacksStr hacks;
bool g_disable_render = false;
float g_left_over = 0.f;
gd::PauseLayer* pauseLayer;

void(__thiscall* CCScheduler_update)(CCScheduler*, float);
void __fastcall CCScheduler_update_H(CCScheduler* self, int, float dt) {

    auto& rs = ReplayPlayer::getInstance();
    const auto play_layer = gd::GameManager::sharedState()->getPlayLayer();

    auto d = CCDirector::sharedDirector();

    if(play_layer && d->getRunningScene()->getChildrenCount() > 1)
    {
        pauseLayer = static_cast<gd::PauseLayer*>(d->getRunningScene()->getChildren()->objectAtIndex(1));
        pauseLayer->setVisible(!hacks.hidePause);
    }

    if (play_layer && (rs.IsRecording() || rs.IsPlaying()) && !play_layer->m_bIsPaused) {
        const auto fps = hacks.fps;
        auto speedhack = CCDirector::sharedDirector()->getScheduler()->getTimeScale();

        const float target_dt = 1.f / fps / speedhack;

        // todo: find ways to disable more render stuff
        g_disable_render = false;

        unsigned times = static_cast<int>((dt + g_left_over) / target_dt);
        if (dt == 0.f)
            return CCScheduler_update(self, target_dt);
        auto start = std::chrono::high_resolution_clock::now();
        for (unsigned i = 0; i < times; ++i) {
            // if (i == times - 1)
            //     g_disable_render = false;
            CCScheduler_update(self, target_dt);
            using namespace std::literals;
            if (std::chrono::high_resolution_clock::now() - start > 33.333ms) {
                times = i + 1;
                break;
            }
        }
        g_left_over += dt - target_dt * times;
    } else {
        CCScheduler_update(self, dt);
    }
}

void(__thiscall* PlayLayer_updateVisibility)(void*);
void __fastcall PlayLayer_updateVisibility_Hook(void* self) {
    if (!g_disable_render)
        PlayLayer_updateVisibility(self);
}

void Setup()
{
    MH_CreateHook((void*)(gd::base + 0x205460), PlayLayer_updateVisibility_Hook, (void**)&PlayLayer_updateVisibility);
    MH_CreateHook(GetProcAddress((HMODULE)libcocosbase, "?update@CCScheduler@cocos2d@@UAEXM@Z"), CCScheduler_update_H, (void**)&CCScheduler_update);
}