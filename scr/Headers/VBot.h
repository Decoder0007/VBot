#pragma once
#include <cocos2d.h>
#include <GUI/CCControlExtension/CCScale9Sprite.h>
#include "MinHook.h"
#include "gd.h"

using namespace cocos2d;

namespace Vbot {
    void mem_init();
    void SaveMacro();
    void LoadMacro();
    void AutoSaveMacro();
    void AutoLoadMacro();
    float getXPos();
    void switchModeFunc();
}

namespace PlayLayer {

    inline bool(__thiscall* init)(CCLayer* self, void* GJGameLevel);
    bool __fastcall initHook(CCLayer* self, int edx, void* GJGameLevel);

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

    void Playback_Code(CCLayer* self, float xpos);
    void Record_Code(CCLayer* self, float xpos);
}

namespace PauseLayer {
    inline bool(__thiscall* init)(CCLayer* self);
    bool __fastcall initHook(CCLayer* self);


    

    class callbacks {
    public:
        void switchMode(CCObject*);
        void switchEnabled(CCObject*);
        void switchAutoSave(CCObject*);
        void switchAutoLoad(CCObject*);
        void modeInfoWindow(CCObject*);
        void SaveMacroCallback(CCObject*);
        void LoadMacroCallback(CCObject*);
    };
}