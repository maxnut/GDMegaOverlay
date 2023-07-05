#pragma once
#include "pch.h"
#include "Replay.h"
#include "Practice.h"
#include "record.hpp"

class ReplayPlayer
{
    protected:
    Replay replay;
    uint32_t actionIndex, actionIndex2;
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
    uint32_t GetFrameCapturesSize() {return replay.GetFrameCapturesSize();}
    uint32_t GetActionIndex() {return actionIndex;}
    uint32_t GetCapturesIndex() {return actionIndex2;}

    Practice& GetPractice() {return practice;}
    bool IsRecording() {return recording;}
    bool IsPlaying() {return playing;}

    void Save(std::string name) {replay.Save(name);}
    void Load(std::string name);
    void Delete(std::string name);
    void Merge(std::string name) {replay.Merge("GDMenu/macros/" + name + ".macro");}

    void RecordAction(bool press, gd::PlayerObject* pl, bool player1);
    void HandleActivatedObjects(bool a, bool b, gd::GameObject* object);

    Replay* GetReplay() {return &replay;}

    void ToggleRecording();
    void TogglePlaying();
    void ClearActions(){replay.ClearActions();};

    void ChangeClickpack();

    void UpdateFrameOffset();
    void StartPlaying(gd::PlayLayer* playLayer);
    void Reset(gd::PlayLayer* playLayer);
    float Update(gd::PlayLayer* playLayer);

    Recorder recorder;
};