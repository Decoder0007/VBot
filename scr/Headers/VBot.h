#pragma once
#include <cocos2d.h>
#include <GUI/CCControlExtension/CCScale9Sprite.h>
#include "MinHook.h"
#include "gd.h"

using namespace cocos2d;

namespace MenuLayer {
    inline bool(__thiscall* init)(CCLayer* self);
    bool __fastcall initHook(CCLayer* self);
}

namespace PlayLayer {

    inline bool(__thiscall* init)(CCLayer* self, void* GJGameLevel);
    bool __fastcall initHook(CCLayer* self, int edx, gd::GJGameLevel* GJGameLevel);

    inline void(__thiscall* update)(CCLayer* self, float deltatime);
    void __fastcall updateHook(CCLayer* self, int edx, float deltatime);

    inline bool(__thiscall* pushButton)(CCLayer* self, int state, bool player);
    bool __fastcall pushButtonHook(CCLayer* self, uintptr_t, int state, bool player);

    inline bool(__thiscall* releaseButton)(CCLayer* self, int state, bool player);
    bool __fastcall releaseButtonHook(CCLayer* self, uintptr_t, int state, bool player);

    inline void(__thiscall* levelComplete)(void* self);
    void __fastcall levelCompleteHook(void* self);

    inline void(__thiscall* resetLevel)(void* self);
    void __fastcall resetLevelHook(void* self);

    inline int(__thiscall* createCheckpoint)(void* self);
    int __fastcall createCheckpointHook(void* self);

    inline int(__thiscall* removeCheckpoint)(void* self);
    int __fastcall removeCheckpointHook(void* self);

    void Playback_Code(CCLayer* self);
    void Record_Code(CCLayer* self);
    void StraightFly(CCLayer* self);
    void AutoWave(CCLayer* self);
    void AI(CCLayer* self);
}

namespace PauseLayer {
    inline bool(__thiscall* init)(CCLayer* self);
    bool __fastcall initHook(CCLayer* self);

    class callbacks {
    public:
        void OpenLayer(CCObject*);
    };
}

namespace VBotLayer {
    void init(CCLayer*);
    
    void SaveMacro(std::string);
    void LoadMacro(std::string);
    float getXPos();
    void switchModeFunc();
    void CreateMacroList(CCMenu*);

    class callbacks {
    public:
        void switchMode(CCObject*);
        void switchEnabled(CCObject*);
        void switchAutoSave(CCObject*);
        void switchAutoLoad(CCObject*);
        void switchFrameMode(CCObject*);
        void switchFrameAdvance(CCObject*);
        void switchHideUI(CCObject*);
        void switchMiniUI(CCObject*);
        void modeInfoWindow(CCObject*);
        void SaveMacroCallback(CCObject*);
        void LoadMacroCallback(CCObject*);
        void SpeedHackSetSpeed(CCObject*);
        void SetFPSCap(CCObject*);
        void Close(CCObject*);
        void Del1(CCObject*);
        void Del2(CCObject*);
        void Del3(CCObject*);
        void Del4(CCObject*);
        void Del5(CCObject*);
        void Load1(CCObject*);
        void Load2(CCObject*);
        void Load3(CCObject*);
        void Load4(CCObject*);
        void Load5(CCObject*);
    };
}

namespace RunMod {
    void mem_init();
}