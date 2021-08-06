#pragma once
#include <cocos2d.h>
#include "MinHook.h"
#include "gd.h"

using namespace cocos2d;

namespace Vbot {
    void mem_init();
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
}

namespace PauseLayer {
    inline bool(__thiscall* init)(CCLayer* self);
    bool __fastcall initHook(CCLayer* self);

    class callbacks {
    public:
        void switchMode(CCObject*);
        void switchEnabled(CCObject*);
    };
}