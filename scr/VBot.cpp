#include "VBot.h"
#include <fstream>
#include <direct.h>

using namespace cocos2d;

size_t base = reinterpret_cast<size_t>(GetModuleHandle(0));

std::list<float> pushCoords = {};
std::list<float> releaseCoords = {};

bool enabled = true;
bool mode = false; // False = Playback | True = Record
int clicks = 0;
bool waitingForFirstClick = false;
bool autoSave = false;
bool autoLoad = true;
bool mouseDown = false;


void PlayLayer::Playback_Code(CCLayer* self, float xpos) {

	auto menu = reinterpret_cast<CCMenu*>(self->getChildByTag(10000));
	auto MouseUpSprite = reinterpret_cast<CCSprite*>(menu->getChildByTag(10007));
	auto MouseDownSprite = reinterpret_cast<CCSprite*>(menu->getChildByTag(10008));
	MouseUpSprite->setVisible(true);

	if (std::find(pushCoords.begin(), pushCoords.end(), xpos) != pushCoords.end()) {
		PlayLayer::pushButton(self, 0, true);

		clicks += 1;
		MouseDownSprite->setVisible(true);
	}

	if (std::find(releaseCoords.begin(), releaseCoords.end(), xpos) != releaseCoords.end()) {
		PlayLayer::releaseButton(self, 0, true);

		MouseDownSprite->setVisible(false);
	}
}

void PlayLayer::Record_Code(CCLayer* self, float xpos) {
	auto menu = reinterpret_cast<CCMenu*>(self->getChildByTag(10000));
	auto MouseUpSprite = reinterpret_cast<CCSprite*>(menu->getChildByTag(10007));
	auto MouseDownSprite = reinterpret_cast<CCSprite*>(menu->getChildByTag(10008));

	MouseUpSprite->setVisible(false);
	MouseDownSprite->setVisible(false);
}

bool __fastcall PlayLayer::pushButtonHook(CCLayer* self, uintptr_t, int state, bool player) {
	mouseDown = true;
	if (enabled) {
		auto menu = reinterpret_cast<CCMenu*>(self->getChildByTag(10000));
		if (gd::GameManager::sharedState()->getPlayLayer() != nullptr) {
			if (mode) {
				PlayLayer::pushButton(self, state, player);

				if (waitingForFirstClick) {
					pushCoords.clear();
					releaseCoords.clear();
					waitingForFirstClick = false;
				}

				float xpos = Vbot::getXPos();
				auto xposInList = std::find(pushCoords.begin(), pushCoords.end(), xpos) != pushCoords.end();

				if (!xposInList) {
					pushCoords.insert(pushCoords.end(), xpos);
					clicks += 1;
				}
			}
			if (!mode) {
				auto InputDisabledText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10004));
				auto SwitchRecordText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10005));
				CCFadeOut* fadeOut = CCFadeOut::create(1.75f);
				CCFadeOut* fadeOut2 = CCFadeOut::create(1.75f);
				InputDisabledText->runAction(fadeOut);
				SwitchRecordText->runAction(fadeOut2);
			}
		}
		else {
			PlayLayer::pushButton(self, state, player);
			clicks += 1;
		}
	}
	else {
		PlayLayer::pushButton(self, state, player);
	}
	return true;
}

bool __fastcall PlayLayer::releaseButtonHook(CCLayer* self, uintptr_t, int state, bool player) {
	mouseDown = false;
	if (enabled) {
		if (gd::GameManager::sharedState()->getPlayLayer() != nullptr) {
			if (mode) {
				PlayLayer::releaseButton(self, state, player);

				float xpos = Vbot::getXPos();
				auto xposInList = std::find(releaseCoords.begin(), releaseCoords.end(), xpos) != releaseCoords.end();

				if (!xposInList) {
					releaseCoords.insert(releaseCoords.end(), xpos);
				}
			}

			if (!mode) {
				auto menu = reinterpret_cast<CCMenu*>(self->getChildByTag(10000));
				auto InputDisabledText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10006));
			}
		}
		else {
			PlayLayer::releaseButton(self, state, player);
		}
	}
	else {
		PlayLayer::releaseButton(self, state, player);
	}
	return true;
}

bool __fastcall PlayLayer::initHook(CCLayer* self, int edx, void* GJGameLevel) {
	auto result = init(self, GJGameLevel);
	if (!result) return result;

	float xpos = Vbot::getXPos();

	clicks = 0;

	auto winSize = CCDirector::sharedDirector()->getWinSize();

	auto menu = CCMenu::create();
	menu->setPositionX(0);
	menu->setPositionY(13);

	auto VBotText = CCLabelBMFont::create("VBot v1.7", "goldFont-uhd.fnt");
	VBotText->setPositionX(5);
	VBotText->setPositionY(52);
	VBotText->setScale(0.6);
	VBotText->setAnchorPoint({ 0, 0.5 });

	auto EnabledText = CCLabelBMFont::create("Enabled", "goldFont-uhd.fnt");
	EnabledText->setPositionX(5);
	EnabledText->setPositionY(39);
	EnabledText->setScale(0.6);
	EnabledText->setAnchorPoint({ 0, 0.5 });

	auto ModeText = CCLabelBMFont::create("Mode: Playback", "goldFont-uhd.fnt");
	ModeText->setPositionX(5);
	ModeText->setPositionY(26);
	ModeText->setScale(0.6);
	ModeText->setAnchorPoint({ 0, 0.5 });

	auto xposString = (std::string)"Xpos: " + std::to_string((float)xpos);
	auto xposCString = xposString.c_str();
	auto XposText = CCLabelBMFont::create(xposCString, "goldFont-uhd.fnt");
	XposText->setPositionX(5);
	XposText->setPositionY(13);
	XposText->setScale(0.6);
	XposText->setAnchorPoint({ 0, 0.5 });

	auto ClicksText = CCLabelBMFont::create("Clicks: 0", "goldFont-uhd.fnt");
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
	EnabledText->setTag(10009);

	menu->setZOrder(1000);
	XposText->setZOrder(1000);
	ModeText->setZOrder(1000);
	VBotText->setZOrder(1000);
	InputDisabledText->setZOrder(1000);
	SwitchRecordText->setZOrder(1000);
	ClicksText->setZOrder(1000);
	MouseUpSprite->setZOrder(1000);
	MouseDownSprite->setZOrder(1001);
	EnabledText->setZOrder(1000);

	menu->addChild(XposText);
	menu->addChild(ModeText);
	menu->addChild(VBotText);
	menu->addChild(InputDisabledText);
	menu->addChild(SwitchRecordText);
	menu->addChild(ClicksText);
	menu->addChild(MouseUpSprite);
	menu->addChild(MouseDownSprite);
	menu->addChild(EnabledText);
	self->addChild(menu);

	Vbot::AutoLoadMacro();

	return result;
}

void __fastcall PlayLayer::updateHook(CCLayer* self, int edx, float deltatime) {
	PlayLayer::update(self, deltatime);
	auto menu = reinterpret_cast<CCMenu*>(self->getChildByTag(10000));
	auto ClicksText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10006));

	float xpos = Vbot::getXPos();
	auto xposString = (std::string)"Xpos: " + std::to_string((float)xpos);
	auto xposCString = xposString.c_str();
	auto XposText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10001));
	XposText->setString(xposCString);
	
	auto ModeText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10002));
	auto EnabledText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10009));
	if (enabled) EnabledText->setString("State: Enabled"); else EnabledText->setString("State: Disabled");

	if (enabled){
		if (!mode) {
			ModeText->setString("Mode: Playback");
			PlayLayer::Playback_Code(self, xpos);
		}
		if (mode) {
			ModeText->setString("Mode: Record");
			PlayLayer::Record_Code(self, xpos);
		}
	}
	
	auto ClicksString = (std::string)"Clicks: " + std::to_string(clicks);
	ClicksText->setString(ClicksString.c_str(), "goldFont-uhd.fnt");
}

void __fastcall PlayLayer::levelCompleteHook(void* self) {
	levelComplete(self);
	if (enabled) Vbot::AutoSaveMacro();
	if (mode) Vbot::switchModeFunc();
}

void __fastcall PlayLayer::resetLevelHook(void* self) {
	resetLevel(self);

	gd::PlayLayer* playLayer = gd::GameManager::sharedState()->getPlayLayer();
	auto isPracticeMode = playLayer->is_practice_mode;
	float xpos = Vbot::getXPos();

	if (enabled && mode) {

		if (isPracticeMode) {
			// Push Coords
			while (pushCoords.back() >= xpos && pushCoords.size() != 0) {
				pushCoords.pop_back();
			}

			// Release Coords
			while (releaseCoords.back() >= xpos && releaseCoords.size() != 0) {
				releaseCoords.pop_back();
			}
		}
	}

	if (enabled && !isPracticeMode) {
		waitingForFirstClick = true;
		clicks = 0;
	}

	if (enabled && mode && mouseDown) {
		if (waitingForFirstClick) {
			pushCoords.clear();
			releaseCoords.clear();
			waitingForFirstClick = false;
		}

		float xpos = Vbot::getXPos();
		pushCoords.insert(pushCoords.end(), xpos);
		clicks += 1;
	}
}


void PauseLayer::callbacks::modeInfoWindow(CCObject*) {
	gd::FLAlertLayer::create(nullptr, "Info", "OK", nullptr, "<cb>blue</c> - Playback Mode\n<cg>green</c> - Record Mode")->show();
}

void PauseLayer::callbacks::switchMode(CCObject*) {
	Vbot::switchModeFunc();
	if (!mode) Vbot::AutoLoadMacro();
}

void PauseLayer::callbacks::switchEnabled(CCObject*) {
	enabled = !enabled;
}

void PauseLayer::callbacks::switchAutoSave(CCObject*) {
	autoSave = !autoSave;
}

void PauseLayer::callbacks::switchAutoLoad(CCObject*) {
	autoLoad = !autoLoad;
}

void PauseLayer::callbacks::SaveMacroCallback(CCObject*) {
	Vbot::SaveMacro();
}

void PauseLayer::callbacks::LoadMacroCallback(CCObject*) {
	Vbot::LoadMacro();
}

bool __fastcall PauseLayer::initHook(CCLayer* self) {
	auto result = PauseLayer::init(self);
	if (!result) return result;

	auto winSize = CCDirector::sharedDirector()->getWinSize();

	auto menu = CCMenu::create();
	menu->setPositionX(0);
	menu->setPositionY(0);

	auto playback = CCSprite::createWithSpriteFrameName("GJ_rotationControlBtn01_001.png");
	auto record = CCSprite::createWithSpriteFrameName("GJ_rotationControlBtn02_001.png");
	auto info = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
	auto checkOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
	auto checkOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
	info->setScale(0.6);

	auto options = extension::CCScale9Sprite::create("GJ_square01-uhd.png");
	options->setContentSize({ 110, 220 });
	options->setPositionX(64);
	options->setPositionY(185);

	auto VBotText = CCLabelBMFont::create("VBot Options", "goldFont-uhd.fnt");
	VBotText->setPositionX(65);
	VBotText->setPositionY(280);
	VBotText->setScale(0.525);

	auto ModeText = CCLabelBMFont::create("Switch Mode: ", "goldFont-uhd.fnt");
	ModeText->setPositionX(50);
	ModeText->setPositionY(250);
	ModeText->setScale(0.4);

	auto ModeButton = gd::CCMenuItemToggler::create((mode) ? playback : record, (mode) ? record : playback, menu, menu_selector(PauseLayer::callbacks::switchMode));
	ModeButton->setPositionX(90);
	ModeButton->setPositionY(250);
	ModeButton->setScale(0.6);

	auto ModeInfoButton = gd::CCMenuItemSpriteExtra::create(info, menu, menu_selector(PauseLayer::callbacks::modeInfoWindow));
	ModeInfoButton->setPositionX(105);
	ModeInfoButton->setPositionY(250);

	auto saveButtonSprite = gd::ButtonSprite::create("Save Macro", 0, false, "bigFont-uhd.fnt", "GJ_button_03-uhd.png", 0, 1);
	saveButtonSprite->setScale(0.4);
	auto saveButton = gd::CCMenuItemSpriteExtra::create(saveButtonSprite, menu, menu_selector(PauseLayer::callbacks::SaveMacroCallback));
	saveButton->setPositionX(options->getPositionX());
	saveButton->setPositionY(230);

	auto loadButtonSprite = gd::ButtonSprite::create("Load Macro", 0, false, "bigFont-uhd.fnt", "GJ_button_02-uhd.png", 0, 1);
	loadButtonSprite->setScale(0.4);
	auto loadButton = gd::CCMenuItemSpriteExtra::create(loadButtonSprite, menu, menu_selector(PauseLayer::callbacks::LoadMacroCallback));
	loadButton->setPositionX(options->getPositionX());
	loadButton->setPositionY(210);

	auto enabledButton = gd::CCMenuItemToggler::create((enabled) ? checkOn : checkOff, (enabled) ? checkOff : checkOn, menu, menu_selector(PauseLayer::callbacks::switchEnabled));
	enabledButton->setPositionX(25);
	enabledButton->setPositionY(95);
	enabledButton->setScale(0.6);

	auto enabledText = CCLabelBMFont::create("Enabled", "bigFont.fnt");
	enabledText->setPositionX(75);
	enabledText->setPositionY(96.5);
	enabledText->setScale(0.5);

	auto autoSaveButton = gd::CCMenuItemToggler::create((autoSave) ? checkOn : checkOff, (autoSave) ? checkOff : checkOn, menu, menu_selector(PauseLayer::callbacks::switchAutoSave));
	autoSaveButton->setPositionX(25);
	autoSaveButton->setPositionY(165);
	autoSaveButton->setScale(0.6);

	auto autoSaveText = CCLabelBMFont::create("Auto Save", "bigFont.fnt");
	autoSaveText->setPositionX(75);
	autoSaveText->setPositionY(166.5);
	autoSaveText->setScale(0.45);

	auto autoLoadButton = gd::CCMenuItemToggler::create((autoLoad) ? checkOn : checkOff, (autoLoad) ? checkOff : checkOn, menu, menu_selector(PauseLayer::callbacks::switchAutoLoad));
	autoLoadButton->setPositionX(25);
	autoLoadButton->setPositionY(185);
	autoLoadButton->setScale(0.6);

	auto autoLoadText = CCLabelBMFont::create("Auto Load", "bigFont.fnt");
	autoLoadText->setPositionX(75);
	autoLoadText->setPositionY(186.5);
	autoLoadText->setScale(0.45);

	menu->setTag(10000);
	ModeText->setTag(10001);
	ModeButton->setTag(10002);
    options->setTag(10003);
	VBotText->setTag(10004);
	ModeInfoButton->setTag(10005);
	saveButton->setTag(10006);
	loadButton->setTag(10007);
	enabledButton->setTag(10008);
	enabledText->setTag(10009);
	autoLoadButton->setTag(10010);
	autoLoadText->setTag(10011);
	autoSaveButton->setTag(10012);
	autoSaveText->setTag(10013);

	menu->setZOrder(1000);
	ModeText->setZOrder(1000);
	ModeButton->setZOrder(1000);
	options->setZOrder(999);
	VBotText->setZOrder(1000);
	ModeInfoButton->setZOrder(1000);
	saveButton->setZOrder(1000);
	loadButton->setZOrder(1000);
	enabledButton->setZOrder(1000);
	enabledText->setZOrder(1000);
	autoLoadButton->setZOrder(1000);
	autoLoadText->setZOrder(1000);
	autoSaveButton->setZOrder(1000);
	autoSaveText->setZOrder(1000);

	menu->addChild(ModeText);
	menu->addChild(ModeButton);
	menu->addChild(options);
	menu->addChild(VBotText);
	menu->addChild(ModeInfoButton);
	menu->addChild(saveButton);
	menu->addChild(loadButton);
	menu->addChild(enabledButton);
	menu->addChild(enabledText);
	menu->addChild(autoLoadButton);
	menu->addChild(autoLoadText);
	menu->addChild(autoSaveButton);
	menu->addChild(autoSaveText);
	self->addChild(menu);

	return result;
}


void Vbot::SaveMacro() {
	auto levelName = gd::GameManager::sharedState()->m_pPlayLayer->level->levelName;
	std::fstream myfile;
	_mkdir("VBot Macros");
	myfile.open("VBot Macros/" + levelName + ".vbot", std::ios::out);

	if (myfile.is_open()) {
		myfile << pushCoords.size();
		myfile << "\n";
		for (float xpos : pushCoords)
		{
			myfile << std::setprecision(6) << std::fixed << xpos;
			myfile << "\n";
		}
		myfile << releaseCoords.size();
		myfile << "\n";
		for (float xpos : releaseCoords)
		{
			myfile << std::setprecision(6) << std::fixed << xpos;
			myfile << "\n";
		}

		myfile << "End of macro\nYou shouldn't be here >:(";

		gd::FLAlertLayer::create(nullptr, "Success!", "Ok", nullptr, "<cl>Successfully</c> saved macro: <cg>" + levelName + "</c>")->show();
	}
	else {
		gd::FLAlertLayer::create(nullptr, "Error", "Ok", nullptr, "<cr>Failed</c> to create file: <cg>" + levelName + "</c>" + "\n<cr>Reason:</c> Unknown\n<cg>Solution:</c> Unknown")->show();
	}
	myfile.close();
}

void Vbot::LoadMacro() {
	_mkdir("VBot Macros");
	pushCoords.clear();
	releaseCoords.clear();
	std::string line;
	auto levelName = gd::GameManager::sharedState()->m_pPlayLayer->level->levelName;
	std::fstream file;

	file.open(("VBot Macros/" + levelName + ".vbot"), std::ios::in);

	if (file.is_open()) {
		getline(file, line); // Get the length of the pushCoords list
		int len;
		len = stoi(line);
		for (int lineno = 1; lineno <= len; lineno++) {
			getline(file, line);
			pushCoords.insert(pushCoords.end(), stof(line));
		}
		getline(file, line); // Get the length of the releaseCoords list
		len = stoi(line);
		for (int lineno = 1; lineno <= len; lineno++) {
			getline(file, line);
			releaseCoords.insert(releaseCoords.end(), stof(line));
		}
		file.close();

		auto attempts = gd::GameManager::sharedState()->m_pPlayLayer->current_attempt;
		auto isPaused = gd::GameManager::sharedState()->m_pPlayLayer->is_paused_again;

		if (attempts > 1 || isPaused) {
			gd::FLAlertLayer::create(nullptr, "Success!", "Ok", nullptr, "Successfully loaded macro: <cg>" + levelName + "</c>")->show();
		}
	}
	else {
		gd::FLAlertLayer::create(nullptr, "Error", "ok", nullptr, 475, "An <cr>error</c> occured while loading the macro: <cg>" + levelName + "</c>" + "\nReason: The macro probably doesn't exist.\nSolution: Record a macro")->show();
	}

}

void Vbot::AutoSaveMacro() {
	if (mode && autoSave) SaveMacro();
}

void Vbot::AutoLoadMacro() {
	if (!mode && autoLoad) LoadMacro();
}

float Vbot::getXPos() {
	gd::PlayLayer* playLayer = gd::GameManager::sharedState()->getPlayLayer();
	return playLayer->m_pPlayer1->position.x;
}

void Vbot::switchModeFunc() {
	mode = !mode;
	clicks = 0;
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
}