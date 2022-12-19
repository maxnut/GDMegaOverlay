#pragma once
#include "pch.h"
#include "Replay.h"
#include "Practice.h"
#include "record.hpp"

class ReplayPlayer
{
    protected:
    Replay replay;
    uint32_t actionIndex;
    Practice practice;
    uint32_t frameOffset;
    bool playing;
    bool recording;

    public:

    static ReplayPlayer& getInstance()
    {
        static ReplayPlayer instance;
        return instance;
    }

    ReplayPlayer();

    uint32_t GetFrame();
    uint32_t GetActionsSize() {return replay.GetActionsSize();}
    uint32_t GetActionIndex() {return actionIndex;}

    Practice& GetPractice() {return practice;}
    bool IsRecording() {return recording;}
    bool IsPlaying() {return playing;}

    void Save(std::string name) {replay.Save(name);}
    void Load(std::string name);

    void RecordAction(bool press, gd::PlayerObject* pl, bool player1);
    void HandleActivatedObjects(bool a, bool b, gd::GameObject* object);

    Replay* GetReplay() {return &replay;}

    void ToggleRecording();
    void TogglePlaying();
    void ClearActions(){replay.ClearActions();};

    void UpdateFrameOffset();
    void StartPlaying(gd::PlayLayer* playLayer);
    void Reset(gd::PlayLayer* playLayer);
    void Update(gd::PlayLayer* playLayer);

    Recorder recorder;
};