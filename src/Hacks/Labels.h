#pragma once

#include "../Common.h"
#include "../Settings.h"

#include <functional>

namespace Labels
{

class Label
{
  public:
	cocos2d::CCLabelBMFont* pointer = nullptr;
	std::string settingName = "";
    std::function<void(cocos2d::CCLabelBMFont*)> function;

    void process()
    {
        if (Settings::get<bool>("labels/" + settingName + "/enabled", false))
            function(pointer);
    }
};

inline std::vector<Label> labels, tl, tr, bl, br;

inline std::vector<float> clicks;

inline size_t totalClicks = 0;
inline size_t frames = 0;
inline size_t deaths = 0;

inline float totalDelta = 0;

inline bool clickRegistered = false;
inline bool labelsCreated = false;
inline bool click = false;

inline bool dead = false;

inline bool(__thiscall* playLayerInit)(cocos2d::CCLayer* self, void* level, bool idk1, bool idk2);
bool __fastcall playLayerInitHook(cocos2d::CCLayer* self, void*, void* level, bool idk1, bool idk2);

inline void(__thiscall* playLayerPostUpdate)(cocos2d::CCLayer* self, float dt);
void __fastcall playLayerPostUpdateHook(cocos2d::CCLayer* self, void*, float dt);

inline void(__thiscall* playLayerDestroyPlayer)(cocos2d::CCLayer* self, void*, void*);
void __fastcall playLayerDestroyPlayerHook(cocos2d::CCLayer* self, void*, void* player, void* object);

inline bool(__thiscall* playerObjectPushButton)(void*, int);
bool __fastcall playerObjectPushButtonHook(void* self, void*, int btn);

inline bool(__thiscall* playerObjectReleaseButton)(void*, int);
bool __fastcall playerObjectReleaseButtonHook(void* self, void*, int btn);

inline int(__thiscall* playLayerResetLevel)(void*);
int __fastcall playLayerResetLevelHook(void* self, void*);

Label setupLabel(std::string labelSettingName, const std::function<void(cocos2d::CCLabelBMFont*)> &function, cocos2d::CCLayer* playLayer);

void calculatePositions();
void initHooks();

void settingsForLabel(std::string labelSettingName, std::function<void()>extraSettings);
void renderWindow();
} // namespace Labels