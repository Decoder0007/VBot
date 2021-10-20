#include "VBot.h"
#include <fstream>
#include <direct.h>
#include "SpeedHack.h"
#include "FPSBypass.h"
#include <filesystem>

using namespace cocos2d;

size_t base = reinterpret_cast<size_t>(GetModuleHandle(0));

std::list<float> pushCoords = {};
std::list<float> releaseCoords = {};
std::list<int> cpframes = {};

bool enabled = true;
bool mode = false; // False = Playback | True = Record
int clicks = 0;
bool waitingForFirstClick = false;
bool autoSave = true;
bool autoLoad = true;
bool mouseDown = false;
int MacroPage = 1;
int frame;
float xpos;
bool frameMode = true;
bool startupMacro = false;
float lastXPos;

CCMenu* BtnMenu = nullptr;

bool __fastcall MenuLayer::initHook(CCLayer* self) {
	auto res = MenuLayer::init(self);
	if (!res) {
		return res;
	}

	/*
	std::string line;
	std::fstream file;
	file.open("VBot/loadinfo.cfg", std::ios::in);

	if (file.is_open()) {
		getline(file, line); // Get if macro is to be loaded
		if (std::stoi(line) == 1) {
			startupMacro = true;
			getline(file, line); // Get path to the loaded macro
			VBotLayer::LoadMacroStartup(line);

			auto test = CCSprite::createWithSpriteFrameName("GJ_duplicateObjectBtn_001.png");
			auto menu = CCMenu::create();
			menu->addChild(test);
			self->addChild(menu);
		}
		
		file.close();
	}
	*/

	return res;
}


void PlayLayer::Playback_Code(CCLayer* self) {

	auto menu = reinterpret_cast<CCMenu*>(self->getChildByTag(10000));
	auto MouseUpSprite = reinterpret_cast<CCSprite*>(menu->getChildByTag(10007));
	auto MouseDownSprite = reinterpret_cast<CCSprite*>(menu->getChildByTag(10008));
	MouseUpSprite->setVisible(true);

	if (!frameMode) {
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

	if (frameMode) {
		if (std::find(pushCoords.begin(), pushCoords.end(), frame) != pushCoords.end()) {
			PlayLayer::pushButton(self, 0, true);

			clicks += 1;
			MouseDownSprite->setVisible(true);
		}

		if (std::find(releaseCoords.begin(), releaseCoords.end(), frame) != releaseCoords.end()) {
			PlayLayer::releaseButton(self, 0, true);

			MouseDownSprite->setVisible(false);
		}
	}
	
}
void PlayLayer::Record_Code(CCLayer* self) {
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
				if (!frameMode) {
					PlayLayer::pushButton(self, state, player);

					if (waitingForFirstClick) {
						pushCoords.clear();
						releaseCoords.clear();
						waitingForFirstClick = false;
					}

					auto clickInList = std::find(pushCoords.begin(), pushCoords.end(), xpos) != pushCoords.end();

					if (!clickInList) {
						pushCoords.insert(pushCoords.end(), xpos);
						clicks += 1;
					}
				}
				if (frameMode) {
					PlayLayer::pushButton(self, state, player);

					if (waitingForFirstClick) {
						pushCoords.clear();
						releaseCoords.clear();
						waitingForFirstClick = false;
					}

					auto clickInList = std::find(pushCoords.begin(), pushCoords.end(), frame) != pushCoords.end();

					if (!clickInList) {
						pushCoords.insert(pushCoords.end(), frame);
						clicks += 1;
					}
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
				if (!frameMode) {
					PlayLayer::releaseButton(self, state, player);

					auto clickInList = std::find(releaseCoords.begin(), releaseCoords.end(), xpos) != releaseCoords.end();

					if (!clickInList) {
						releaseCoords.insert(releaseCoords.end(), xpos);
					}

					// Basically it screws up if you click and release after you die so this detects if you release and removes the click
					if (std::find(pushCoords.begin(), pushCoords.end(), xpos) != pushCoords.end()) {
						pushCoords.remove(xpos);
					}
				}
				if (frameMode) {
					PlayLayer::releaseButton(self, state, player);

					auto clickInList = std::find(releaseCoords.begin(), releaseCoords.end(), frame) != releaseCoords.end();

					if (!clickInList) {
						releaseCoords.insert(releaseCoords.end(), frame);
					}

					// Basically it screws up if you click and release after you die so this detects if you release and removes the click
					if (std::find(pushCoords.begin(), pushCoords.end(), frame) != pushCoords.end()) {
						pushCoords.remove(frame);
					}
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
bool __fastcall PlayLayer::initHook(CCLayer* self, int edx, gd::GJGameLevel* GJGameLevel) {
	auto result = init(self, GJGameLevel);
	if (!result) return result;

	xpos = 0;
	frame = 0;
	lastXPos = 0;

	/*
	/////////////////////
	std::fstream myfile;
	myfile.open("VBot/Levels/"+GJGameLevel->levelName+".vbotlevel", std::ios::out);
	
	// Save Level Data
	if (myfile.is_open()) {
		myfile << GJGameLevel->levelString; // 1
		myfile << "\n";
		myfile << std::to_string(GJGameLevel->songID); // 2
		myfile << "\n";
		myfile << std::to_string(GJGameLevel->coins); // 3
		myfile << "\n";
		myfile << GJGameLevel->levelName; // 4
		myfile << "\n";
		myfile << std::to_string(GJGameLevel->objectCount); // 5
		myfile << "\n";
		myfile << GJGameLevel->twoPlayerMode; // 6
	}
	/////////////////////////////////////////////
	*/
	clicks = 0;

	auto winSize = CCDirector::sharedDirector()->getWinSize();

	auto menu = CCMenu::create();
	menu->setPositionX(0);
	menu->setPositionY(13);

	auto VBotText = CCLabelBMFont::create("VBot v1.9", "goldFont-uhd.fnt");
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

	auto xposString = (std::string)"???";
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

	if (!mode && autoLoad) VBotLayer::LoadMacro(gd::GameManager::sharedState()->m_pPlayLayer->level->levelName);

	return result;
}
void __fastcall PlayLayer::updateHook(CCLayer* self, int edx, float deltatime) {
	PlayLayer::update(self, deltatime);
	auto menu = reinterpret_cast<CCMenu*>(self->getChildByTag(10000));
	auto ClicksText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10006));
	auto XposText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10001));
	auto PlayLayer = gd::GameManager::sharedState()->getPlayLayer();

	if (lastXPos != VBotLayer::getXPos()) {
		frame++;
	}

	lastXPos = VBotLayer::getXPos();

	if (!frameMode) {
		xpos = VBotLayer::getXPos();
		auto xposString = (std::string)"XPos: " + std::to_string((float)xpos);
		XposText->setString(xposString.c_str());
	}
	// i know it says xpos but im too lazy to make another one
	else {
		auto xposString = (std::string)"Frame: " + std::to_string((int)frame);
		XposText->setString(xposString.c_str());
	}
	
	auto ModeText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10002));
	auto EnabledText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10009));
	if (enabled) EnabledText->setString("State: Enabled"); else EnabledText->setString("State: Disabled");

	if (enabled){
		if (!mode) {
			ModeText->setString("Mode: Playback");
			PlayLayer::Playback_Code(self);
		}
		if (mode) {
			ModeText->setString("Mode: Record");
			PlayLayer::Record_Code(self);
		}
	}
	
	auto ClicksString = (std::string)"Clicks: " + std::to_string(clicks);
	ClicksText->setString(ClicksString.c_str(), "goldFont-uhd.fnt");
}
void __fastcall PlayLayer::levelCompleteHook(void* self) {
	levelComplete(self);
	if (mode && autoSave && enabled) VBotLayer::SaveMacro(gd::GameManager::sharedState()->m_pPlayLayer->level->levelName);
	if (mode) VBotLayer::switchModeFunc();
}
void __fastcall PlayLayer::resetLevelHook(void* self) {
	resetLevel(self);

	gd::PlayLayer* playLayer = gd::GameManager::sharedState()->getPlayLayer();
	auto isPracticeMode = playLayer->is_practice_mode;

	frame = 0;

	if (enabled && mode) {

		if (isPracticeMode) {
			if (!frameMode) { // If in XPos mode
				// Push Coords
				while (pushCoords.back() >= xpos && pushCoords.size() != 0) {
					pushCoords.pop_back();
				}

				// Release Coords
				while (releaseCoords.back() >= xpos && releaseCoords.size() != 0) {
					releaseCoords.pop_back();
				}
			}
			else { // If in Frame mode
				if (cpframes.size() > 0) {
					frame = cpframes.back();
				}
				else {
					frame = 0;
				}

				// Push Coords
				while (pushCoords.back() >= frame && pushCoords.size() != 0) {
					pushCoords.pop_back();
				}

				// Release Coords
				while (releaseCoords.back() >= frame && releaseCoords.size() != 0) {
					releaseCoords.pop_back();
				}
			}
			waitingForFirstClick = false;
		}
		else {
			waitingForFirstClick = true;
			clicks = 0;
		}

		if (mouseDown) {
			if (waitingForFirstClick) {
				pushCoords.clear();
				releaseCoords.clear();
				waitingForFirstClick = false;
			}

			if (!frameMode) pushCoords.insert(pushCoords.end(), xpos);
			else pushCoords.insert(pushCoords.end(), frame);
			
			clicks++;
		}
	}
}
int __fastcall PlayLayer::createCheckpointHook(void* self) {
	int res = createCheckpoint(self);

	cpframes.insert(cpframes.end(), frame);

	return res;
}
int __fastcall PlayLayer::removeCheckpointHook(void* self) {
	int res = removeCheckpoint(self);

	cpframes.pop_back();

	return res;
}


bool __fastcall PauseLayer::initHook(CCLayer* self) {
	auto result = PauseLayer::init(self);
	if (!result) return result;

	BtnMenu = (CCMenu*)self->getChildren()->objectAtIndex(8); //8 is the menu that holds the play, practice, edit, exit, restart buttons

	auto openLayerButtonSprite = gd::ButtonSprite::create("V", 0, false, "goldFont-uhd.fnt", "GJ_button_01-uhd.png", 0, 1);
	openLayerButtonSprite->setScale(1.5);
	auto openLayerButton = gd::CCMenuItemSpriteExtra::create(openLayerButtonSprite, BtnMenu, menu_selector(PauseLayer::callbacks::OpenLayer));
	openLayerButton->setPositionX(0);
	openLayerButton->setPositionY(0);

	openLayerButton->setTag(1000);
	openLayerButton->setZOrder(10000);

	BtnMenu->addChild(openLayerButton);

	BtnMenu->alignItemsHorizontallyWithPadding(5);

	return result;
}
void PauseLayer::callbacks::OpenLayer(CCObject*) {

	auto PauseLayer = (CCLayer*)(CCDirector::sharedDirector()->getRunningScene()->getChildren()->lastObject());
	auto BtnMenu = (CCMenu*)(PauseLayer->getChildren()->objectAtIndex(8));

	VBotLayer::init(PauseLayer);
}


void VBotLayer::init(CCLayer* self) {
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	BtnMenu->setPositionY(20000); //Remember to put this back when i add the close button

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
	options->setContentSize({ winSize.width - 20, winSize.height - 10 });
	options->setPosition({ winSize.width / 2, winSize.height / 2 });

	auto VBotText = CCLabelBMFont::create("VBot Options", "goldFont-uhd.fnt");
	VBotText->setPositionX(winSize.width / 2);
	VBotText->setPositionY(winSize.height - 25);
	VBotText->setScale(0.8);

	auto ModeText = CCLabelBMFont::create("Switch Mode: ", "goldFont-uhd.fnt");
	ModeText->setScale(0.6);
	ModeText->setPositionX(25 + (ModeText->getScaledContentSize().width / 2));
	ModeText->setPositionY(winSize.height - 50);

	auto ModeButton = gd::CCMenuItemToggler::create((mode) ? playback : record, (mode) ? record : playback, menu, menu_selector(VBotLayer::callbacks::switchMode));
	ModeButton->setScale(0.6);
	ModeButton->setPositionX(25 + (ModeText->getScaledContentSize().width) + 10);
	ModeButton->setPositionY(winSize.height - 50);

	auto ModeInfoButton = gd::CCMenuItemSpriteExtra::create(info, menu, menu_selector(VBotLayer::callbacks::modeInfoWindow));
	ModeInfoButton->setPositionX(25 + ModeText->getScaledContentSize().width + 10 + ModeButton->getScaledContentSize().width);
	ModeInfoButton->setPositionY(winSize.height - 50);

	auto saveButtonSprite = gd::ButtonSprite::create("Save Macro", 0, false, "bigFont-uhd.fnt", "GJ_button_03-uhd.png", 0, 1);
	saveButtonSprite->setScale(0.5);
	auto saveButton = gd::CCMenuItemSpriteExtra::create(saveButtonSprite, menu, menu_selector(VBotLayer::callbacks::SaveMacroCallback));
	saveButton->setPositionX(95);
	saveButton->setPositionY(245);

	auto loadButtonSprite = gd::ButtonSprite::create("Load Macro", 0, false, "bigFont-uhd.fnt", "GJ_button_02-uhd.png", 0, 1);
	loadButtonSprite->setScale(0.5);
	auto loadButton = gd::CCMenuItemSpriteExtra::create(loadButtonSprite, menu, menu_selector(VBotLayer::callbacks::LoadMacroCallback));
	loadButton->setPositionX(95);
	loadButton->setPositionY(220);

	auto enabledButton = gd::CCMenuItemToggler::create((enabled) ? checkOn : checkOff, (enabled) ? checkOff : checkOn, menu, menu_selector(VBotLayer::callbacks::switchEnabled));
	enabledButton->setPositionX(45);
	enabledButton->setPositionY(145);
	enabledButton->setScale(0.6);

	auto enabledText = CCLabelBMFont::create("Enabled", "bigFont.fnt");
	enabledText->setPositionX(95);
	enabledText->setPositionY(146.5);
	enabledText->setScale(0.5);

	auto autoSaveButton = gd::CCMenuItemToggler::create((autoSave) ? checkOn : checkOff, (autoSave) ? checkOff : checkOn, menu, menu_selector(VBotLayer::callbacks::switchAutoSave));
	autoSaveButton->setPositionX(45);
	autoSaveButton->setPositionY(165);
	autoSaveButton->setScale(0.6);

	auto autoSaveText = CCLabelBMFont::create("Auto Save", "bigFont.fnt");
	autoSaveText->setPositionX(100);
	autoSaveText->setPositionY(166.5);
	autoSaveText->setScale(0.45);

	auto autoLoadButton = gd::CCMenuItemToggler::create((autoLoad) ? checkOn : checkOff, (autoLoad) ? checkOff : checkOn, menu, menu_selector(VBotLayer::callbacks::switchAutoLoad));
	autoLoadButton->setPositionX(45);
	autoLoadButton->setPositionY(185);
	autoLoadButton->setScale(0.6);

	auto autoLoadText = CCLabelBMFont::create("Auto Load", "bigFont.fnt");
	autoLoadText->setPositionX(100);
	autoLoadText->setPositionY(186.5);
	autoLoadText->setScale(0.45);

	auto frameModeMenu = CCMenu::create();
	frameModeMenu->setPosition({ winSize.width / 2, winSize.height - 50 });
	auto frameModeText = CCLabelBMFont::create("Frame Mode", "bigFont.fnt");
	auto frameModeButton = gd::CCMenuItemToggler::create((frameMode) ? checkOn : checkOff, (frameMode) ? checkOff : checkOn, frameModeMenu, menu_selector(VBotLayer::callbacks::switchFrameMode));
	frameModeMenu->addChild(frameModeText);
	frameModeMenu->addChild(frameModeButton);
	frameModeMenu->alignItemsHorizontallyWithPadding(10);
	frameModeMenu->setAnchorPoint({ 0, 0 });
	frameModeMenu->setScale(0.65);


	MacroPage = 1;
	VBotLayer::CreateMacroList(menu);


	auto speedMenu = CCMenu::create();
	auto speedhackText = CCLabelBMFont::create("Speedhack", "bigFont-uhd.fnt");
	auto speedhackInput = gd::CCTextInputNode::create("Speed", speedMenu, "bigFont-uhd.fnt", 50, 30);
	auto speedhackInputBG = extension::CCScale9Sprite::create("square02_small.png");
	auto speedhackSetSpeedSprite = gd::ButtonSprite::create("Set Speed", 0, false, "bigFont-uhd.fnt", "GJ_button_01-uhd.png", 28, 0.25);
	auto speedhackSetSpeedButton = gd::CCMenuItemSpriteExtra::create(speedhackSetSpeedSprite, speedMenu, menu_selector(VBotLayer::callbacks::SpeedHackSetSpeed));
	speedhackText->setScale(0.5);
	speedhackText->setZOrder(1);
	speedhackInput->setScale(0.5);
	speedhackInput->setAnchorPoint({ 0, 0 });
	speedhackInput->setZOrder(2);
	speedhackInput->setAllowedChars("0123456789.");
	speedhackSetSpeedButton->setZOrder(1);
	speedhackInputBG->setContentSize({ 55, 27 });
	speedhackInputBG->setZOrder(1);
	speedhackInputBG->setColor({ 0, 0, 0 });
	speedhackInputBG->setOpacity(100);
	speedhackText->setTag(1);
	speedMenu->addChild(speedhackText);
	speedhackInputBG->setTag(4);
	speedMenu->addChild(speedhackInputBG);
	speedhackSetSpeedButton->setTag(3);
	speedMenu->addChild(speedhackSetSpeedButton);
	speedMenu->alignItemsHorizontallyWithPadding(10);
	speedhackInput->setTag(2);
	speedhackInput->setPosition(speedhackInputBG->getPosition());
	speedMenu->addChild(speedhackInput);

	auto fpsMenu = CCMenu::create();
	auto setFPSText = CCLabelBMFont::create("FPS Bypass", "bigFont-uhd.fnt");
	auto setFPSInput = gd::CCTextInputNode::create("FPS", fpsMenu, "bigFont-uhd.fnt", 50, 30);
	auto setFPSInputBG = extension::CCScale9Sprite::create("square02_small.png");
	auto setFPSCapSprite = gd::ButtonSprite::create("Set FPS", 0, false, "bigFont-uhd.fnt", "GJ_button_01-uhd.png", 28, 0.25);
	auto setFPSCapButton = gd::CCMenuItemSpriteExtra::create(setFPSCapSprite, fpsMenu, menu_selector(VBotLayer::callbacks::SetFPSCap));
	setFPSText->setScale(0.5);
	setFPSText->setZOrder(1);
	setFPSInput->setScale(0.5);
	setFPSInput->setAnchorPoint({ 0, 0 });
	setFPSInput->setZOrder(2);
	setFPSInput->setAllowedChars("0123456789");
	setFPSCapButton->setZOrder(1);
	setFPSInputBG->setContentSize({ 40, 27 });
	setFPSInputBG->setZOrder(1);
	setFPSInputBG->setColor({ 0, 0, 0 });
	setFPSInputBG->setOpacity(100);
	setFPSText->setTag(1);
	fpsMenu->addChild(setFPSText);
	setFPSInput->setTag(2);
	fpsMenu->addChild(setFPSInput);
	setFPSCapButton->setTag(3);
	fpsMenu->addChild(setFPSCapButton);
	fpsMenu->alignItemsHorizontallyWithPadding(10);
	setFPSInputBG->setTag(4);
	setFPSInputBG->setPosition(setFPSInput->getPosition());
	fpsMenu->addChild(setFPSInputBG);
	auto setxto = (setFPSCapButton->getPositionX() + (setFPSCapButton->getScaledContentSize().width / 2)) - (setFPSText->getPositionX() - (setFPSText->getScaledContentSize().width / 2)) / 2;
	fpsMenu->setPosition({ setxto, 80 });
	speedMenu->setPosition({ fpsMenu->getPositionX(), 110 });


	auto closeLayerBtnSprite = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
	auto closeLayerBtn = CCMenuItemSpriteExtra::create(closeLayerBtnSprite, closeLayerBtnSprite, menu, menu_selector(VBotLayer::callbacks::Close));
	closeLayerBtn->setScale(0.65);
	closeLayerBtn->setPosition({ 20, winSize.height - 10 });


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
	speedMenu->setTag(10015);
	fpsMenu->setTag(10016);
	closeLayerBtn->setTag(10017);
	frameModeMenu->setTag(10018);

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
	speedMenu->setZOrder(1000);
	fpsMenu->setZOrder(1000);
	closeLayerBtn->setZOrder(1001);
	frameModeMenu->setZOrder(1000);

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
	menu->addChild(speedMenu);
	menu->addChild(fpsMenu);
	menu->addChild(closeLayerBtn);
	menu->addChild(frameModeMenu);
	self->addChild(menu);
}
void VBotLayer::SaveMacro(std::string levelName) {
	
	std::fstream myfile;
	_mkdir("VBot/Macros");
	myfile.open("VBot/Macros/" + levelName + ".vbot", std::ios::out);

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

		if(gd::GameManager::sharedState()->getPlayLayer()->is_paused_again){
			auto PauseLayer = (CCLayer*)(CCDirector::sharedDirector()->getRunningScene()->getChildren()->objectAtIndex(1));
			VBotLayer::CreateMacroList((CCMenu*)PauseLayer->getChildren()->lastObject());
		}

	}
	else {
		gd::FLAlertLayer::create(nullptr, "Error", "Ok", nullptr, "<cr>Failed</c> to create file: <cg>" + levelName + "</c>" + "\n<cr>Reason:</c> Unknown\n<cg>Solution:</c> Unknown")->show();
	}
	myfile.close();
}
void VBotLayer::LoadMacro(std::string levelName) {
	_mkdir("VBot/Macros");
	pushCoords.clear();
	releaseCoords.clear();
	std::string line;
	std::fstream file;

	file.open(("VBot/Macros/" + levelName + ".vbot"), std::ios::in);

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
void VBotLayer::LoadMacroStartup(std::string path) {
	pushCoords.clear();
	releaseCoords.clear();
	std::string line;
	std::fstream file;

	file.open((path), std::ios::in);

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
	}
}
float VBotLayer::getXPos() {
	gd::PlayLayer* playLayer = gd::GameManager::sharedState()->getPlayLayer();
	return playLayer->m_pPlayer1->position.x;
}
void VBotLayer::switchModeFunc() {
	mode = !mode;
	clicks = 0;
}
void VBotLayer::CreateMacroList(CCMenu* menu) {
	int indexMultiplier = (MacroPage-1) * 5;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	auto MacroMenu = CCMenu::create();
	MacroMenu->setPosition({ 0, 0 });
	MacroMenu->setZOrder(1000);
	MacroMenu->setTag(10014);

	auto MacroListBG = extension::CCScale9Sprite::create("GJ_square01-uhd.png");
	MacroListBG->setContentSize({ 160, 250 });
	MacroListBG->setAnchorPoint({ 0.5, 0.5 });
	MacroListBG->setPosition({ (winSize.width / 5) * 4, winSize.height / 2 });
	MacroListBG->setZOrder(0);

	auto MacroListC1 = CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
	MacroListC1->setRotation(0);
	MacroListC1->setPosition({ MacroListBG->getPositionX() - (MacroListBG->getScaledContentSize().width / 2), MacroListBG->getPositionY() - (MacroListBG->getScaledContentSize().height / 2) });
	MacroListC1->setAnchorPoint({ 0, 0 });
	MacroListC1->setScale(0.7);
	MacroListC1->setZOrder(2);

	auto MacroListC2 = CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
	MacroListC2->setRotationX(180);
	MacroListC2->setPosition({ MacroListBG->getPositionX() - (MacroListBG->getScaledContentSize().width / 2), MacroListBG->getPositionY() + (MacroListBG->getScaledContentSize().height / 2) });
	MacroListC2->setAnchorPoint({ 0, 0 });
	MacroListC2->setScale(0.7);
	MacroListC2->setZOrder(2);

	auto MacroListC3 = CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
	MacroListC3->setRotation(180);
	MacroListC3->setPosition({ MacroListBG->getPositionX() + (MacroListBG->getScaledContentSize().width / 2), MacroListBG->getPositionY() + (MacroListBG->getScaledContentSize().height / 2) });
	MacroListC3->setAnchorPoint({ 0, 0 });
	MacroListC3->setScale(0.7);
	MacroListC3->setZOrder(2);

	auto MacroListC4 = CCSprite::createWithSpriteFrameName("dailyLevelCorner_001.png");
	MacroListC4->setRotationY(180);
	MacroListC4->setPosition({ MacroListBG->getPositionX() + (MacroListBG->getScaledContentSize().width / 2), MacroListBG->getPositionY() - (MacroListBG->getScaledContentSize().height / 2) });
	MacroListC4->setAnchorPoint({ 0, 0 });
	MacroListC4->setScale(0.7);
	MacroListC4->setZOrder(2);

	MacroMenu->addChild(MacroListBG);
	MacroMenu->addChild(MacroListC1);
	MacroMenu->addChild(MacroListC2);
	MacroMenu->addChild(MacroListC3);
	MacroMenu->addChild(MacroListC4);

	auto path = "VBot/Macros";
	std::vector<std::string>Macros;
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		Macros.insert(Macros.end(), file.path().filename().string());
	}

	for (int i = 0; i < (Macros.size() < 5 ? Macros.size() : 5); i++)
	{
		auto MacroName = Macros[i + indexMultiplier];
		MacroName.pop_back();
		MacroName.pop_back();
		MacroName.pop_back();
		MacroName.pop_back();
		MacroName.pop_back();

		auto MacroCardMenu = CCMenu::create();
		auto MacroCard = extension::CCScale9Sprite::create("GJ_square01-uhd.png");
		MacroCard->setContentSize({ 145, 50 });
		MacroCard->setAnchorPoint({ 0.5, 0.5 });
		MacroCard->setPositionX(((winSize.width / 5) * 4) - (winSize.width / 2));
		MacroCard->setPositionY((winSize.height / 2) - (50 * i) - 60);

		auto MacroLabel = CCLabelBMFont::create(MacroName.c_str(), "bigFont-uhd.fnt");
		MacroLabel->setScale(0.2);
		MacroLabel->setPosition({ MacroCard->getPositionX() - 62, MacroCard->getPositionY() });
		MacroLabel->setAnchorPoint({ 0, 0.5 });

		MacroCardMenu->setZOrder(1);
		MacroCardMenu->addChild(MacroCard);
		MacroCardMenu->addChild(MacroLabel);

		auto MacroDelSprite = CCSprite::createWithSpriteFrameName("GJ_deleteSongBtn_001.png");

		CCMenuItemSpriteExtra* MacroDelBtn = CCMenuItemSpriteExtra::create(MacroDelSprite, MacroDelSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Del1));

		if (i == 0) { MacroDelBtn = CCMenuItemSpriteExtra::create(MacroDelSprite, MacroDelSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Del1)); }
		if (i == 1) { MacroDelBtn = CCMenuItemSpriteExtra::create(MacroDelSprite, MacroDelSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Del2)); }
		if (i == 2) { MacroDelBtn = CCMenuItemSpriteExtra::create(MacroDelSprite, MacroDelSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Del3)); }
		if (i == 3) { MacroDelBtn = CCMenuItemSpriteExtra::create(MacroDelSprite, MacroDelSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Del4)); }
		if (i == 4) { MacroDelBtn = CCMenuItemSpriteExtra::create(MacroDelSprite, MacroDelSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Del5)); }

		MacroDelBtn->setScale(0.5);
		MacroDelBtn->setPosition({ MacroCard->getPositionX() + 62 - (MacroDelBtn->getScaledContentSize().width / 2), MacroCard->getPositionY() });

		MacroCardMenu->addChild(MacroDelBtn);


		auto MacroLoadSprite = gd::ButtonSprite::create("Load", 0, FALSE, "bigFont-uhd.fnt", "GJ_button_02-uhd.png", 0, 0.5);

		CCMenuItemSpriteExtra* MacroLoadBtn = CCMenuItemSpriteExtra::create(MacroLoadSprite, MacroLoadSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Load1));

		if (i == 0) { MacroLoadBtn = CCMenuItemSpriteExtra::create(MacroLoadSprite, MacroLoadSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Load1)); }
		if (i == 1) { MacroLoadBtn = CCMenuItemSpriteExtra::create(MacroLoadSprite, MacroLoadSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Load2)); }
		if (i == 2) { MacroLoadBtn = CCMenuItemSpriteExtra::create(MacroLoadSprite, MacroLoadSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Load3)); }
		if (i == 3) { MacroLoadBtn = CCMenuItemSpriteExtra::create(MacroLoadSprite, MacroLoadSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Load4)); }
		if (i == 4) { MacroLoadBtn = CCMenuItemSpriteExtra::create(MacroLoadSprite, MacroLoadSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Load5)); }

		MacroLoadBtn->setScale(0.5);
		MacroLoadBtn->setPosition({ MacroCard->getPositionX() + 42 - (MacroLoadBtn->getScaledContentSize().width / 2), MacroCard->getPositionY() });

		MacroCardMenu->addChild(MacroLoadBtn);

		MacroMenu->addChild(MacroCardMenu);
	}

	menu->addChild(MacroMenu);
}
void VBotLayer::callbacks::modeInfoWindow(CCObject*) {
	gd::FLAlertLayer::create(nullptr, "Info", "OK", nullptr, "<cb>blue</c> - Playback Mode\n<cg>green</c> - Record Mode")->show();
}
void VBotLayer::callbacks::switchMode(CCObject*) {
	VBotLayer::switchModeFunc();
	if (!mode && autoLoad) LoadMacro(gd::GameManager::sharedState()->m_pPlayLayer->level->levelName);
}
void VBotLayer::callbacks::switchEnabled(CCObject*) {
	enabled = !enabled;
}
void VBotLayer::callbacks::switchAutoSave(CCObject*) {
	autoSave = !autoSave;
}
void VBotLayer::callbacks::switchAutoLoad(CCObject*) {
	autoLoad = !autoLoad;
}
void VBotLayer::callbacks::switchFrameMode(CCObject*) {
	frameMode = !frameMode;
}
void VBotLayer::callbacks::SaveMacroCallback(CCObject*) {
	VBotLayer::SaveMacro(gd::GameManager::sharedState()->m_pPlayLayer->level->levelName);
}
void VBotLayer::callbacks::LoadMacroCallback(CCObject*) {
	VBotLayer::LoadMacro(gd::GameManager::sharedState()->m_pPlayLayer->level->levelName);
}
void VBotLayer::callbacks::SpeedHackSetSpeed(CCObject*) {
	CCLayer* pauseLayer = (CCLayer*)CCDirector::sharedDirector()->getRunningScene()->getChildren()->lastObject();
	CCMenu* menu = (CCMenu*)pauseLayer->getChildByTag(10000);
	CCMenu* speedMenu = (CCMenu*)menu->getChildByTag(10015);
	gd::CCTextInputNode* setSpeedInput = (gd::CCTextInputNode*)speedMenu->getChildByTag(2);
	float speed = (float)std::stof(setSpeedInput->getString());
	Speedhack::InitializeSpeedHack(speed);
}
void VBotLayer::callbacks::SetFPSCap(CCObject*) {
	CCLayer* pauseLayer = (CCLayer*)CCDirector::sharedDirector()->getRunningScene()->getChildren()->lastObject();
	CCMenu* menu = (CCMenu*)pauseLayer->getChildByTag(10000);
	CCMenu* fpsMenu = (CCMenu*)menu->getChildByTag(10016);
	gd::CCTextInputNode* setFPSInput = (gd::CCTextInputNode*)fpsMenu->getChildByTag(2);

	int fpscap = (int)std::stoi(setFPSInput->getString());
	FPSBypass::SetFPS(fpscap);
}
void VBotLayer::callbacks::Close(CCObject*) {
	CCLayer* pauseLayer = (CCLayer*)CCDirector::sharedDirector()->getRunningScene()->getChildren()->lastObject();
	pauseLayer->removeChildByTag(10000);
	BtnMenu->setPositionY(130); //Move back buttons
}
void VBotLayer::callbacks::Del1(CCObject*) {
	int indexMultiplier = (MacroPage - 1) * 5;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	auto path = "VBot/Macros";
	std::vector<std::filesystem::path>Macros;
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		Macros.insert(Macros.end(), file.path());
	}

	std::filesystem::remove(Macros[indexMultiplier]);

	auto PauseLayer = (CCLayer*)(CCDirector::sharedDirector()->getRunningScene()->getChildren()->lastObject());
	MacroPage = 1;
	VBotLayer::CreateMacroList((CCMenu*)PauseLayer->getChildren()->lastObject());
}
void VBotLayer::callbacks::Del2(CCObject*) {
	int indexMultiplier = (MacroPage - 1) * 5;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	auto path = "VBot/Macros";
	std::vector<std::filesystem::path>Macros;
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		Macros.insert(Macros.end(), file.path());
	}

	std::filesystem::remove(Macros[indexMultiplier+1]);

	auto PauseLayer = (CCLayer*)(CCDirector::sharedDirector()->getRunningScene()->getChildren()->lastObject());
	MacroPage = 1;
	VBotLayer::CreateMacroList((CCMenu*)PauseLayer->getChildren()->lastObject());
}
void VBotLayer::callbacks::Del3(CCObject*) {
	int indexMultiplier = (MacroPage - 1) * 5;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	auto path = "VBot/Macros";
	std::vector<std::filesystem::path>Macros;
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		Macros.insert(Macros.end(), file.path());
	}

	std::filesystem::remove(Macros[indexMultiplier + 2]);

	auto PauseLayer = (CCLayer*)(CCDirector::sharedDirector()->getRunningScene()->getChildren()->lastObject());
	MacroPage = 1;
	VBotLayer::CreateMacroList((CCMenu*)PauseLayer->getChildren()->lastObject());
}
void VBotLayer::callbacks::Del4(CCObject*) {
	int indexMultiplier = (MacroPage - 1) * 5;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	auto path = "VBot/Macros";
	std::vector<std::filesystem::path>Macros;
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		Macros.insert(Macros.end(), file.path());
	}

	std::filesystem::remove(Macros[indexMultiplier + 3]);

	auto PauseLayer = (CCLayer*)(CCDirector::sharedDirector()->getRunningScene()->getChildren()->lastObject());
	MacroPage = 1;
	VBotLayer::CreateMacroList((CCMenu*)PauseLayer->getChildren()->lastObject());
}
void VBotLayer::callbacks::Del5(CCObject*) {
	int indexMultiplier = (MacroPage - 1) * 5;
	auto winSize = CCDirector::sharedDirector()->getWinSize();

	auto path = "VBot/Macros";
	std::vector<std::filesystem::path>Macros;
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		Macros.insert(Macros.end(), file.path());
	}

	std::filesystem::remove(Macros[indexMultiplier + 4]);

	auto PauseLayer = (CCLayer*)(CCDirector::sharedDirector()->getRunningScene()->getChildren()->lastObject());
	MacroPage = 1;
	VBotLayer::CreateMacroList((CCMenu*)PauseLayer->getChildren()->lastObject());
}
void VBotLayer::callbacks::Load1(CCObject*) {
	int indexMultiplier = (MacroPage - 1) * 5;

	auto path = "VBot/Macros";
	std::vector<std::filesystem::path>Macros;
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		Macros.insert(Macros.end(), file.path());
	}

	auto MacroName = Macros[indexMultiplier].filename().string();
	MacroName.pop_back();
	MacroName.pop_back();
	MacroName.pop_back();
	MacroName.pop_back();
	MacroName.pop_back();
	VBotLayer::LoadMacro(MacroName);
}
void VBotLayer::callbacks::Load2(CCObject*) {
	int indexMultiplier = (MacroPage - 1) * 5;

	auto path = "VBot/Macros";
	std::vector<std::filesystem::path>Macros;
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		Macros.insert(Macros.end(), file.path());
	}

	auto MacroName = Macros[indexMultiplier+1].filename().string();
	MacroName.pop_back();
	MacroName.pop_back();
	MacroName.pop_back();
	MacroName.pop_back();
	MacroName.pop_back();
	VBotLayer::LoadMacro(MacroName);
}
void VBotLayer::callbacks::Load3(CCObject*) {
	int indexMultiplier = (MacroPage - 1) * 5;

	auto path = "VBot/Macros";
	std::vector<std::filesystem::path>Macros;
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		Macros.insert(Macros.end(), file.path());
	}

	auto MacroName = Macros[indexMultiplier + 2].filename().string();
	MacroName.pop_back();
	MacroName.pop_back();
	MacroName.pop_back();
	MacroName.pop_back();
	MacroName.pop_back();
	VBotLayer::LoadMacro(MacroName);
}
void VBotLayer::callbacks::Load4(CCObject*) {
	int indexMultiplier = (MacroPage - 1) * 5;

	auto path = "VBot/Macros";
	std::vector<std::filesystem::path>Macros;
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		Macros.insert(Macros.end(), file.path());
	}

	auto MacroName = Macros[indexMultiplier + 3].filename().string();
	MacroName.pop_back();
	MacroName.pop_back();
	MacroName.pop_back();
	MacroName.pop_back();
	MacroName.pop_back();
	VBotLayer::LoadMacro(MacroName);
}
void VBotLayer::callbacks::Load5(CCObject*) {
	int indexMultiplier = (MacroPage - 1) * 5;

	auto path = "VBot/Macros";
	std::vector<std::filesystem::path>Macros;
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		Macros.insert(Macros.end(), file.path());
	}

	auto MacroName = Macros[indexMultiplier + 1].filename().string();
	MacroName.pop_back();
	MacroName.pop_back();
	MacroName.pop_back();
	MacroName.pop_back();
	MacroName.pop_back();
	VBotLayer::LoadMacro(MacroName);
}

void RunMod::mem_init() {
	MH_CreateHook(
		(PVOID)(base + 0x1907B0),
		MenuLayer::initHook,
		(LPVOID*)&MenuLayer::init
	);

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
		(PVOID)(base + 0x20B050),
		PlayLayer::createCheckpointHook,
		(LPVOID*)&PlayLayer::createCheckpoint);

	MH_CreateHook(
		(PVOID)(base + 0x20B830),
		PlayLayer::removeCheckpointHook,
		(LPVOID*)&PlayLayer::removeCheckpoint);
}