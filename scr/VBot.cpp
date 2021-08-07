#include "VBot.h"
#include <thread>
#include <sstream>

using namespace cocos2d;

size_t base = reinterpret_cast<size_t>(GetModuleHandle(0));

std::list<float> pushCoords = {};
std::list<float> releaseCoords = {};

bool enabled = false;
bool mode = false; // False = Playback | True = Record
int clicks = 0;
bool waitingForFirstClick = true;

float getXPos() {
	gd::PlayLayer* playLayer = gd::GameManager::sharedState()->getPlayLayer();
	return playLayer->m_pPlayer1->position.x;
}

bool __fastcall PlayLayer::pushButtonHook(CCLayer* self, uintptr_t, int state, bool player) {
	auto menu = reinterpret_cast<CCMenu*>(self->getChildByTag(10000));
	if (mode) {
		PlayLayer::pushButton(self, 0, true);
		if (waitingForFirstClick) {
			waitingForFirstClick = false;
			pushCoords.clear();
			releaseCoords.clear();
		}
		float xpos = getXPos();
		pushCoords.insert(pushCoords.end(), xpos);
		
		clicks += 1;
	}

	if (!mode) {
		auto InputDisabledText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10004));
		auto SwitchRecordText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10005));
		CCFadeOut* fadeOut = CCFadeOut::create(1.75f);
		CCFadeOut* fadeOut2 = CCFadeOut::create(1.75f);
		InputDisabledText->runAction(fadeOut);
		SwitchRecordText->runAction(fadeOut2);
	}

	return true;
}

bool __fastcall PlayLayer::releaseButtonHook(CCLayer* self, uintptr_t, int state, bool player) {
	if (mode) {
		PlayLayer::releaseButton(self, 0, true);
		float xpos = getXPos();
		releaseCoords.insert(releaseCoords.end(), xpos);
	}

	if (!mode) {
		auto menu = reinterpret_cast<CCMenu*>(self->getChildByTag(10000));
		auto InputDisabledText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10006));
		
	}

	return true;
}

bool __fastcall PlayLayer::initHook(CCLayer* self, int edx, void* GJGameLevel) {
	auto result = init(self, GJGameLevel);
	if (!result) return result;

	float xpos = getXPos();

	auto winSize = CCDirector::sharedDirector()->getWinSize();

	auto menu = CCMenu::create();
	menu->setPositionX(0);
	menu->setPositionY(13);

	auto VBotText = CCLabelBMFont::create("VBot v1.3", "goldFont.fnt");
	VBotText->setPositionX(5);
	VBotText->setPositionY(39);
	VBotText->setScale(0.6);
	VBotText->setAnchorPoint({ 0, 0.5 });

	auto ModeText = CCLabelBMFont::create("Mode: Playback", "goldFont.fnt");
	ModeText->setPositionX(5);
	ModeText->setPositionY(26);
	ModeText->setScale(0.6);
	ModeText->setAnchorPoint({ 0, 0.5 });

	auto xposString = (std::string)"Xpos: " + std::to_string((float)xpos);
	auto xposCString = xposString.c_str();
	auto XposText = CCLabelBMFont::create(xposCString, "goldFont.fnt");
	XposText->setPositionX(5);
	XposText->setPositionY(13);
	XposText->setScale(0.6);
	XposText->setAnchorPoint({ 0, 0.5 });

	auto ClicksText = CCLabelBMFont::create("Clicks: 0", "goldFont.fnt");
	ClicksText->setPositionX(5);
	ClicksText->setPositionY(0);
	ClicksText->setScale(0.6);
	ClicksText->setAnchorPoint({ 0, 0.5 });

	auto InputDisabledText = CCLabelBMFont::create("Input is disabled in Playback mode", "bigFont-uhd.fnt");
	InputDisabledText->setAnchorPoint({ 0.5, 0.5 });
	InputDisabledText->setPositionX(winSize.width / 2);
	InputDisabledText->setPositionY(65);
	InputDisabledText->setScale(0.75);
	InputDisabledText->setOpacity(0);

	auto SwitchRecordText = CCLabelBMFont::create("Switch to record mode in the pause menu", "bigFont-uhd.fnt");
	SwitchRecordText->setAnchorPoint({ 0.5, 0.5 });
	SwitchRecordText->setPositionX(winSize.width / 2);
	SwitchRecordText->setPositionY(47);
	SwitchRecordText->setScale(0.50);
	SwitchRecordText->setOpacity(0);

	auto MouseUpSprite = CCSprite::create("GJ_button_04-uhd.png");
	auto MouseDownSprite = CCSprite::create("GJ_button_01-uhd.png");
	MouseUpSprite->setPositionX(winSize.width - (MouseUpSprite->getScaledContentSize().width / 2) - 5);
	MouseDownSprite->setPositionX(winSize.width - (MouseDownSprite->getScaledContentSize().width / 2) - 5);
	MouseUpSprite->setPositionY(10);
	MouseDownSprite->setPositionY(10);
	MouseDownSprite->setVisible(false);
		
	menu->setTag(10000);
	XposText->setTag(10001);
	ModeText->setTag(10002);
	VBotText->setTag(10003);
	InputDisabledText->setTag(10004);
	SwitchRecordText->setTag(10005);
	ClicksText->setTag(10006);
	MouseUpSprite->setTag(10007);
	MouseDownSprite->setTag(10008);

	menu->setZOrder(1000);
	XposText->setZOrder(1000);
	ModeText->setZOrder(1000);
	VBotText->setZOrder(1000);
	InputDisabledText->setZOrder(1000);
	SwitchRecordText->setZOrder(1000);
	ClicksText->setZOrder(1000);
	MouseUpSprite->setZOrder(1000);
	MouseDownSprite->setZOrder(1000);

	menu->addChild(XposText);
	menu->addChild(ModeText);
	menu->addChild(VBotText);
	menu->addChild(InputDisabledText);
	menu->addChild(SwitchRecordText);
	menu->addChild(ClicksText);
	menu->addChild(MouseUpSprite);
	menu->addChild(MouseDownSprite);
	self->addChild(menu);

	return result;
}

void Playback_Code(CCLayer* self, float xpos) {
	auto menu = reinterpret_cast<CCMenu*>(self->getChildByTag(10000));
	auto MouseUpSprite = reinterpret_cast<CCSprite*>(menu->getChildByTag(10007));
	auto MouseDownSprite = reinterpret_cast<CCSprite*>(menu->getChildByTag(10008));
	
	if (std::find(pushCoords.begin(), pushCoords.end(), xpos) != pushCoords.end()) {
		PlayLayer::pushButton(self, 0, true);

		clicks += 1;
		
		MouseUpSprite->setVisible(false);
		MouseDownSprite->setVisible(true);
	}

	if (std::find(releaseCoords.begin(), releaseCoords.end(), xpos) != releaseCoords.end()) {
		PlayLayer::releaseButton(self, 0, true);

		MouseUpSprite->setVisible(true);
		MouseDownSprite->setVisible(false);
	}
}

void Record_Code(CCLayer* self, float xpos) {
	auto menu = reinterpret_cast<CCMenu*>(self->getChildByTag(10000));
	auto MouseUpSprite = reinterpret_cast<CCSprite*>(menu->getChildByTag(10007));
	auto MouseDownSprite = reinterpret_cast<CCSprite*>(menu->getChildByTag(10008));

	MouseUpSprite->setVisible(false);
	MouseDownSprite->setVisible(false);
}

void __fastcall PlayLayer::updateHook(CCLayer* self, int edx, float deltatime) {
	PlayLayer::update(self, deltatime);
	auto menu = reinterpret_cast<CCMenu*>(self->getChildByTag(10000));
	auto ClicksText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10006));

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

	auto ClicksString = (std::string)"Clicks: " + std::to_string(clicks);
	ClicksText->setString(ClicksString.c_str(), "goldFont.fnt");
}

void __fastcall PlayLayer::levelCompleteHook(void* self) {
	levelComplete(self);
	clicks = 0;
	mode = false;
}

void __fastcall PlayLayer::resetLevelHook(void* self) {
	resetLevel(self);
	clicks = 0;
	// Set waiting for first click to true so that they can switch to playback
	// mode and watch it up until they die. If they click when this is true
	// then the lists are cleared.
	waitingForFirstClick = true;
}

void __fastcall PlayLayer::onExitHook(void* self) {
	onExit(self);
	clicks = 0;
	// Set waiting for first click to true so that they can switch to playback
	// mode and watch it up until they die. If they click when this is true
	// then the lists are cleared.
	waitingForFirstClick = true;
}

bool __fastcall PauseLayer::initHook(CCLayer* self) {
	auto result = PauseLayer::init(self);
	if (!result) return result;

	auto winSize = CCDirector::sharedDirector()->getWinSize();

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
	ModeText->setTag(10001);
	ModeButton->setTag(10002);
	

	menu->setZOrder(1000);
	ModeText->setZOrder(1000);
	ModeButton->setZOrder(10005);

	menu->addChild(ModeText);
	menu->addChild(ModeButton);
	self->addChild(menu);

	return result;
}

void PauseLayer::callbacks::switchMode(CCObject*) {
	mode = !mode;
	clicks = 0;
	if (mode) {
		pushCoords.clear();
		releaseCoords.clear();
	}
}

void PauseLayer::callbacks::switchEnabled(CCObject*) {
	enabled = !enabled;
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

	MH_CreateHook(
		(PVOID)(base + 0x1FD3D0),
		PlayLayer::levelCompleteHook,
		(LPVOID*)&PlayLayer::levelComplete
	);

	MH_CreateHook(
		(PVOID)(base + 0x20BF00),
		PlayLayer::resetLevelHook,
		(LPVOID*)&PlayLayer::resetLevel
	);

	MH_CreateHook(
		(PVOID)(base + 0x20dc00),
		PlayLayer::onExitHook,
		(LPVOID*)&PlayLayer::onExit
	);
}