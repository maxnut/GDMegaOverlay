#include "CCSchedulerHook.h"
#include "Hacks.h"
#include "ReplayPlayer.h"

extern struct HacksStr hacks;

extern struct HacksStr hacks;
bool g_disable_render = false;
float g_left_over = 0.f;

void(__thiscall* CCScheduler_update)(CCScheduler*, float);
void __fastcall CCScheduler_update_H(CCScheduler* self, int, float dt)
{

	auto& rs = ReplayPlayer::getInstance();
	const auto play_layer = gd::GameManager::sharedState()->getPlayLayer();

	auto d = CCDirector::sharedDirector();

	if (play_layer && play_layer->m_bIsPaused && d->getRunningScene()->getChildrenCount() > 1)
	{
		auto pauseLayer = static_cast<gd::PauseLayer*>(d->getRunningScene()->getChildren()->objectAtIndex(1));
		if (pauseLayer)
			pauseLayer->setVisible(!hacks.hidePause);
	}

	if (play_layer && (rs.IsRecording() || rs.IsPlaying() || hacks.tpsBypassBool || hacks.frameStep) &&
		!play_layer->m_bIsPaused)
	{
		const auto fps = ExternData::tps;
		auto speedhack = self->getTimeScale();

		const float target_dt = 1.f / fps / speedhack;

		// todo: find ways to disable more render stuff
		g_disable_render = false;

		unsigned times = static_cast<int>((dt + g_left_over) / target_dt);
		if (dt == 0.f && !hacks.frameStep)
			return CCScheduler_update(self, target_dt);

		auto start = std::chrono::high_resolution_clock::now();

		if (hacks.frameStep)
		{
			if (!hacks.holdAdvance && ExternData::steps <= 0 || hacks.holdAdvance && !ExternData::holdingAdvance)
			{
				if (ExternData::animationAction)
					ExternData::animationAction->step(dt);
				return;
			}

			if (hacks.tpsBypassBool || rs.IsRecording() || rs.IsPlaying())
			{
				for (unsigned i = 0; i < times; ++i)
				{
					CCScheduler_update(self, target_dt);
					using namespace std::literals;
					if (std::chrono::high_resolution_clock::now() - start > 33.333ms)
					{
						times = i + 1;
						break;
					}
				}
				g_left_over += dt - target_dt * times;
			}
			else
				CCScheduler_update(self, target_dt * speedhack);
			ExternData::steps--;
			return;
		}

		for (unsigned i = 0; i < times; ++i)
		{
			CCScheduler_update(self, target_dt);
			using namespace std::literals;
			if (std::chrono::high_resolution_clock::now() - start > 33.333ms)
			{
				times = i + 1;
				break;
			}
		}
		g_left_over += dt - target_dt * times;
	}
	else
	{
		CCScheduler_update(self, dt);
	}
}

// draw divider by mat

class CCDirectorVisible : public cocos2d::CCDirector
{
  public:
	void calculateDeltaTime()
	{
		CCDirector::calculateDeltaTime();
	};

	void setNextScene()
	{
		CCDirector::setNextScene();
	}
};

int frameCounter = 0;
double frame_remainder = 0;

float GetActiveFpsLimit()
{
	auto* app = cocos2d::CCApplication::sharedApplication();
	if (app->getVerticalSyncEnabled())
	{
		static const float refresh_rate = [] {
			DEVMODEA device_mode;
			memset(&device_mode, 0, sizeof(device_mode));
			device_mode.dmSize = sizeof(device_mode);
			device_mode.dmDriverExtra = 0;

			if (EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &device_mode) == 0)
			{
				return 60.f;
			}
			else
			{
				return static_cast<float>(device_mode.dmDisplayFrequency);
			}
		}();
		return refresh_rate;
	}
	else
	{
		return static_cast<float>(1.0 / cocos2d::CCDirector::sharedDirector()->getAnimationInterval());
	}
}

void(__thiscall* CCAction_step)(CCAction*, float dt);
void __fastcall CCAction_stepHook(CCAction* self, void*, float dt)
{
	CCAction_step(self, dt);
}

void(__thiscall* CCDirector_drawScene)(CCDirector*);
void __fastcall CCDirector_drawSceneHook(CCDirector* self, void*)
{
	// disable for first 300 frames of game being open
	if (!hacks.drawDivideBool || self->getTotalFrames() < 300)
	{
		return CCDirector_drawScene(self);
	}

	// scary floats
	// getAnimationInterval is 1/fps bypass
	// 1/((1/fps bypass) * target) = fps bypass/target
	const float thing = GetActiveFpsLimit() / static_cast<float>(ExternData::screenFps);

	frameCounter++;

	// run full scene draw (glClear, visit) each time the counter is full
	if (static_cast<double>(frameCounter) + frame_remainder >= thing)
	{
		frame_remainder += static_cast<double>(frameCounter) - thing;
		frameCounter = 0;
		return CCDirector_drawScene(self);
	}

	// otherwise, we only run updates

	// upcast to remove protection
	auto visible_director = static_cast<CCDirectorVisible*>(self);

	// this line seems to create a speedhack
	// visible_director->calculateDeltaTime();

	if (!self->isPaused())
	{
		self->getScheduler()->update(self->getDeltaTime());
	}

	if (self->getNextScene())
	{
		visible_director->setNextScene();
	}
}

void(__thiscall* PlayLayer_updateVisibility)(void*);
void __fastcall PlayLayer_updateVisibility_Hook(void* self)
{
	if (!g_disable_render)
		PlayLayer_updateVisibility(self);
}

void(__thiscall* AppDelegate_willSwitchToScene)(void*, CCScene* scene);
void __fastcall AppDelegate_willSwitchToScene_Hook(void* self, void*, CCScene* scene)
{
	if(ExternData::animationAction)
	{
		float elapsed = ExternData::animationAction->getElapsed();
		scene->runAction(ExternData::animationAction);
		ExternData::animationAction->step(0);
		ExternData::animationAction->step(elapsed);
	}
	AppDelegate_willSwitchToScene(self, scene);
}

void Setup()
{
	MH_CreateHook((void*)(gd::base + 0x205460), PlayLayer_updateVisibility_Hook, (void**)&PlayLayer_updateVisibility);
	MH_CreateHook(GetProcAddress((HMODULE)libcocosbase, "?update@CCScheduler@cocos2d@@UAEXM@Z"), CCScheduler_update_H,
				  (void**)&CCScheduler_update);
	MH_CreateHook(GetProcAddress((HMODULE)libcocosbase, "?drawScene@CCDirector@cocos2d@@QAEXXZ"),
				  CCDirector_drawSceneHook, (void**)&CCDirector_drawScene);
	MH_CreateHook(GetProcAddress((HMODULE)libcocosbase, "?step@CCActionInterval@cocos2d@@UAEXM@Z"), CCAction_stepHook,
				  (void**)&CCAction_step);
	MH_CreateHook((void*)(gd::base + 0x3D690), AppDelegate_willSwitchToScene_Hook,
				  (void**)&AppDelegate_willSwitchToScene);
}