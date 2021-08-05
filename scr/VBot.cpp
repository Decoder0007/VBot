#include "VBot.h"
#include <thread>
#include <sstream>

using namespace cocos2d;

size_t base = reinterpret_cast<size_t>(GetModuleHandle(0));
auto play_layer = *reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(base + 0x3222D0) + 0x164);


void PlayLayer::pushButton() {
	reinterpret_cast<int(__thiscall*)(uintptr_t, int, bool)>(base + 0x111500)(play_layer, 0, true);
}

void PlayLayer::releaseButton() {
	reinterpret_cast<int(__thiscall*)(uintptr_t, int, bool)>(base + 0x111660)(play_layer, 0, true);
}

float getXPos() {
	gd::PlayLayer* playLayer = gd::GameManager::sharedState()->getPlayLayer();
	return playLayer->m_pPlayer1->position.x;
}

void checkJump() {

	gd::PlayLayer* playLayer = gd::GameManager::sharedState()->getPlayLayer();

	float xpos = 0;

	while (1) {
		if (playLayer) {
			
			xpos = getXPos();
			
			/*if (xpos == 1133078005) {
				PlayLayer::pushButton();
			}
			if (xpos == 1133574305) {
				PlayLayer::releaseButton();
			}*/
		}
		
		Sleep(100);
	}
}


	

bool __fastcall PlayLayer::initHook(CCLayer* self, void*, void* GJGameLevel) {
	auto result = init(self, GJGameLevel);
	if (!result) return result;
	
	//std::thread botThread(checkJump);
	//botThread.detach();

	auto menu = CCMenu::create();
	float xpos = getXPos();
	auto xposText = std::to_string(xpos);
	const char* xposText2 = xposText.c_str();
	
	auto text = CCLabelBMFont::create(xposText2, "goldFont.fnt");
	text->setPositionX(20);
	text->setPositionY(20);

	menu->addChild(text);
	self->addChild(menu);

	while (1) {
		float xpos = getXPos();
		auto xposText = std::to_string(xpos);
		const char* xposText2 = xposText.c_str();

		text->setString(xposText2);
		Sleep(1);
	}

	return result;
}

void PlayLayer::mem_init() {
	MH_CreateHook(
		(PVOID)(base + 0x01FB780),
		PlayLayer::initHook,
		(LPVOID*)&PlayLayer::init
	);
}

/*
*/