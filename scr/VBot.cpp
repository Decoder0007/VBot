#include "VBot.h"
#include <thread>
#include <sstream>

using namespace cocos2d;

size_t base = reinterpret_cast<size_t>(GetModuleHandle(0));

std::list<float> pushCoords = {};
std::list<float> releaseCoords = {};

bool enabled = false;
bool mode = false; // False = Playback | True = Record

float getXPos() {
	gd::PlayLayer* playLayer = gd::GameManager::sharedState()->getPlayLayer();
	return playLayer->m_pPlayer1->position.x;
}

bool __fastcall PlayLayer::initHook(CCLayer* self, int edx, void* GJGameLevel) {
	auto result = init(self, GJGameLevel);
	if (!result) return result;

	float xpos = getXPos();

	auto winSize = CCDirector::sharedDirector()->getWinSize();

	auto menu = CCMenu::create();
	menu->setPositionX(0);
	menu->setPositionY(13);

	auto xposString = (std::string)"Xpos: " + std::to_string((float)xpos);
	auto xposCString = xposString.c_str();
	auto XposText = CCLabelBMFont::create(xposCString, "goldFont.fnt");
	XposText->setPositionX(5);
	XposText->setPositionY(10);
	XposText->setScale(0.6);
	XposText->setAnchorPoint({ 0, 0.5 });

	auto ModeText = CCLabelBMFont::create("Mode: Playback", "goldFont.fnt");
	ModeText->setPositionX(5);
	ModeText->setPositionY(23);
	ModeText->setScale(0.6);
	ModeText->setAnchorPoint({ 0, 0.5 });

	auto VBotText = CCLabelBMFont::create("VBot", "goldFont.fnt");
	VBotText->setPositionX(5);
	VBotText->setPositionY(36);
	VBotText->setScale(0.6);
	VBotText->setAnchorPoint({ -1, 0.5 });
	
	menu->setTag(10000);
	XposText->setTag(10001);
	ModeText->setTag(10002);
	VBotText->setTag(10003);

	menu->setZOrder(1000);
	XposText->setZOrder(1000);
	ModeText->setZOrder(1000);
	VBotText->setZOrder(1000);

	menu->addChild(XposText);
	menu->addChild(ModeText);
	menu->addChild(VBotText);
	self->addChild(menu);

	return result;
}

bool __fastcall PlayLayer::pushButtonHook(CCLayer* self, uintptr_t, int state, bool player) {
	if (mode) {
		PlayLayer::pushButton(self, 0, true);
		float xpos = getXPos();
		pushCoords.insert(pushCoords.end(), xpos);
	}

	return true;
}

bool __fastcall PlayLayer::releaseButtonHook(CCLayer* self, uintptr_t, int state, bool player) {
	if (mode) {
		PlayLayer::releaseButton(self, 0, true);
		float xpos = getXPos();
		releaseCoords.insert(releaseCoords.end(), xpos);
	}

	return true;
}

void Playback_Code(CCLayer* self, float xpos) {
	if (std::find(pushCoords.begin(), pushCoords.end(), xpos) != pushCoords.end()) PlayLayer::pushButton(self, 0, true);
	if (std::find(releaseCoords.begin(), releaseCoords.end(), xpos) != releaseCoords.end()) PlayLayer::releaseButton(self, 0, true);

}

void Record_Code(CCLayer* self, float xpos) {
	

}

void __fastcall PlayLayer::updateHook(CCLayer* self, int edx, float deltatime) {
	PlayLayer::update(self, deltatime);
	auto menu = reinterpret_cast<CCMenu*>(self->getChildByTag(10000));

	float xpos = getXPos();
	auto xposString = (std::string)"Xpos: " + std::to_string((float)xpos);
	auto xposCString = xposString.c_str();
	auto XposText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10001));
	XposText->setString(xposCString);
	
	auto ModeText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10002));

	if (!mode) {
		ModeText->setString("Mode: Playback");
		Playback_Code(self, xpos);
	}
	if (mode) {
		ModeText->setString("Mode: Record");
		Record_Code(self, xpos);
	}
}

void PauseLayer::callbacks::switchMode(CCObject*) {
	mode = !mode;
	if (mode) {
		pushCoords.clear();
		releaseCoords.clear();
	}
}

void PauseLayer::callbacks::switchEnabled(CCObject*) {
	enabled = !enabled;
}

bool __fastcall PauseLayer::initHook(CCLayer* self) {
	auto result = PauseLayer::init(self);
	if (!result) return result;

	auto menu = CCMenu::create();
	menu->setPositionX(0);
	menu->setPositionY(0);

	auto ModeText = CCLabelBMFont::create("Switch Mode: ", "goldFont.fnt");
	ModeText->setPositionX(5);
	ModeText->setPositionY(10);
	ModeText->setScale(0.6);
	ModeText->setAnchorPoint({ 0, 0.5 });

	auto playback = CCSprite::createWithSpriteFrameName("GJ_rotationControlBtn01_001.png");
	auto record = CCSprite::createWithSpriteFrameName("GJ_rotationControlBtn02_001.png");
	
	auto ModeButton = gd::CCMenuItemToggler::create((mode) ? playback : record, (mode) ? record : playback, menu, menu_selector(PauseLayer::callbacks::switchMode));
	ModeButton->setPositionX(ModeText->getScaledContentSize().width+5+ModeButton->getScaledContentSize().width/2);
	ModeButton->setPositionY(10);
	ModeButton->setScale(0.6);
	ModeButton->setAnchorPoint({ 0.5, 0.5 });
	
	menu->setTag(10000);
	ModeText->setTag(10004);
	ModeButton->setTag(10005);

	menu->setZOrder(1000);
	ModeText->setZOrder(1000);
	ModeButton->setZOrder(10005);

	menu->addChild(ModeText);
	menu->addChild(ModeButton);
	self->addChild(menu);

	return result;
}

void Vbot::mem_init() {
	MH_CreateHook(
		(PVOID)(base + 0x01FB780),
		PlayLayer::initHook,
		(LPVOID*)&PlayLayer::init
	);

	MH_CreateHook(
		(PVOID)(base + 0x2029C0),
		PlayLayer::updateHook,
		(LPVOID*)&PlayLayer::update
	);

	MH_CreateHook(
		(PVOID)(base + 0x1E4620),
		PauseLayer::initHook,
		(LPVOID*)&PauseLayer::init
	);

	MH_CreateHook(
		(PVOID)(base + 0x111500),
		PlayLayer::pushButtonHook,
		(LPVOID*)&PlayLayer::pushButton
	);

	MH_CreateHook(
		(PVOID)(base + 0x111660),
		PlayLayer::releaseButtonHook,
		(LPVOID*)&PlayLayer::releaseButton
	);
}