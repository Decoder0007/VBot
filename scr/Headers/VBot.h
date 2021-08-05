#pragma once
#include <cocos2d.h>
#include "MinHook.h"
#include "gd.h"

using namespace cocos2d;

namespace PlayLayer {

    inline bool(__thiscall* init)(CCLayer* self, void* GJGameLevel);
    bool __fastcall initHook(CCLayer* self, void*, void* GJGameLevel);

    void pushButton();
    void releaseButton();
    void mem_init();
}