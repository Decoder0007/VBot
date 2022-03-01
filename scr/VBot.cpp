#include "VBot.h"
#include <fstream>
#include <direct.h>
#include "SpeedHack.h"
#include "FPSBypass.h"
#include <filesystem>
#include "FPSMultiplier.h"
#include <conio.h>
#pragma comment(lib, "Winmm.lib")

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

int holdYpos = 0;

bool hasSetFPS = false;

bool FrameAdvance = false;
int advanceFrames = 5;
bool keyf = false;

bool AIEnabled = false;

bool HideUI = false;

bool MiniUI = false;

CCMenu* BtnMenu = nullptr;

bool __fastcall MenuLayer::initHook(CCLayer* self) {
	auto res = MenuLayer::init(self);
	if (!res) {
		return res;
	}

	AllocConsole();
	SetConsoleTitleA("VBot Debug Mode");
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	std::cout << "VBot Successfully Loaded!" << std::endl;

	_mkdir("VBot");
	_mkdir("VBot/Macros");
	_mkdir("VBot/Levels");

	std::string line;
	std::fstream file;
	std::string path;
	std::fstream consolelog;
	consolelog.open("VBotLog.txt", std::ios::out);
	file.open("VBot/loadinfo.cfg", std::ios::in);

	if (file.is_open()) {
		getline(file, line); // Get if macro is to be loaded
		consolelog << "Got first line as:  " + line << std::endl;
		std::cout << "Got first line as:  " + line << std::endl;
		if (std::stoi(line) == 1) {
			startupMacro = true;
			getline(file, line); // Get path to the loaded macro
			consolelog << "Got path to macro as:  " + line << std::endl;
			std::cout << "Got path to macro as:  " + line << std::endl;
			path = line;
		}
		else {
			startupMacro = false;
		}
		file.close();
	}
	else {
		startupMacro = false;
	}

	if (startupMacro) {
		remove("VBot/loadinfo.cfg");
		pushCoords.clear();
		releaseCoords.clear();
		std::string line;
		std::fstream file;
		std::string base_filename = path.substr(path.find_last_of("\\") + 1);
		base_filename.pop_back(); // Remove the space
		base_filename.pop_back(); // Remove the "

		consolelog << "Opening macro using VBot/Macros/" + base_filename << std::endl;
		std::cout << "Opening macro using VBot/Macros/" + base_filename << std::endl;
		file.open(("VBot/Macros/" + base_filename), std::ios::in);

		if (file.is_open()) {
			consolelog << "File opened" << std::endl;
			std::cout << "File opened" << std::endl;
			consolelog << "Reading Data" << std::endl;
			std::cout << "Reading Data" << std::endl;
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

		consolelog << "Data read" << std::endl;
		std::cout << "Data read" << std::endl;

		std::string line2;
		std::fstream levelfile;


		levelfile.open(("VBot/Levels/" + base_filename + "level"), std::ios::in);
		consolelog << "Opened level file with name:  VBot/Levels/" + base_filename + "level" << std::endl;
		std::cout << "Opened level file with name:  VBot/Levels/" + base_filename + "level" << std::endl; //Need the extra level because im using the Macro file name

		auto level = gd::GJGameLevel::create();

		if (levelfile.is_open()) {

			getline(levelfile, line2);
			level->setLevelData(line2);
			getline(levelfile, line2);
			level->m_nSongID = std::stoi(line2);
			getline(levelfile, line2);
			level->m_nCoins = std::stoi(line2);
			getline(levelfile, line2);
			level->m_sLevelName = line2;
			getline(levelfile, line2);
			level->m_nObjectCount = std::stoi(line2);
			getline(levelfile, line2);
			level->m_bTwoPlayerMode = std::stoi(line2);
			level->m_nLikes = pushCoords.size();
			level->m_sCreatorName = "VBot";
			level->m_bIsChkValid = true;
			level->m_bAutoLevel = true;
			level->m_bIsEpic = true;
			level->m_nDownloads = -1;
			level->m_bIsEditable = false;
			level->m_eLevelType = gd::GJLevelType::kGJLevelTypeLocal;

			levelfile.close();
		}

		for (int i = 0; i < self->getChildrenCount(); i++) {
			CCNode* obj = reinterpret_cast<CCNode*>(self->getChildren()->objectAtIndex(i));
			obj->setPositionY(20000);
		}
		auto layer = gd::LevelInfoLayer::create(level);
		self->addChild(layer, 100); // Add Level Info Layer

		consolelog << "Added layer (hopefully)" << std::endl;
		std::cout << "Added layer (hopefully)" << std::endl;
	}

	consolelog.close();

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
/*void PlayLayer::StraightFly(CCLayer* self) {
	flyFrame++;
	if (flyFrame == 0) PlayLayer::pushButton(self, 0, true);
	if (flyFrame == flyInterval) PlayLayer::releaseButton(self, 0, true);
	if (flyFrame == (flyInterval*2) - 1) flyFrame = -1;
}
void PlayLayer::AutoWave(CCLayer* self) {
	waveFrame++;
	if (waveFrame == 0) PlayLayer::pushButtonHook(self, 0, 0, true);
	if (waveFrame == waveInterval) PlayLayer::releaseButtonHook(self, 0, 0, true);
	if (waveFrame == (waveInterval * 2) - 1) waveFrame = -1;
}*/
void PlayLayer::AI(CCLayer* self) {
	auto playLayer = (gd::PlayLayer*)(self);
	if (AIEnabled) {
		int gamemode;
		// Cube / Square whatever you want to call it
		if (!playLayer->m_pPlayer1->m_isShip && !playLayer->m_pPlayer1->m_isBall && !playLayer->m_pPlayer1->m_isBird && !playLayer->m_pPlayer1->m_isDart && !playLayer->m_pPlayer1->m_isRobot && !playLayer->m_pPlayer1->m_isSpider) gamemode = 0;
		// Ship
		if (playLayer->m_pPlayer1->m_isShip) gamemode = 1;
		// Ball
		if (playLayer->m_pPlayer1->m_isBall) gamemode = 2;
		// UFO
		if (playLayer->m_pPlayer1->m_isBird) gamemode = 3;
		// Wave
		if (playLayer->m_pPlayer1->m_isDart) gamemode = 4;
		// Robot
		if (playLayer->m_pPlayer1->m_isRobot) gamemode = 5;
		// Spider
		if (playLayer->m_pPlayer1->m_isSpider) gamemode = 6;


	}
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

					auto clickInPush = std::find(pushCoords.begin(), pushCoords.end(), xpos) != pushCoords.end();
					auto clickInRelease = std::find(releaseCoords.begin(), releaseCoords.end(), xpos) != releaseCoords.end();

					if (!clickInPush) {
						pushCoords.insert(pushCoords.end(), xpos);
						clicks += 1;
					}
					if (clickInRelease) releaseCoords.remove(xpos);
				}
				if (frameMode) {
					PlayLayer::pushButton(self, state, player);

					if (waitingForFirstClick) {
						pushCoords.clear();
						releaseCoords.clear();
						waitingForFirstClick = false;
					}

					auto clickInPush = std::find(pushCoords.begin(), pushCoords.end(), frame) != pushCoords.end();
					auto clickInRelease = std::find(releaseCoords.begin(), releaseCoords.end(), frame) != releaseCoords.end();

					if (!clickInPush) {
						pushCoords.insert(pushCoords.end(), frame);
						clicks += 1;
					}
					if (clickInRelease) releaseCoords.remove(frame);
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

					auto clickInRelease = std::find(releaseCoords.begin(), releaseCoords.end(), xpos) != releaseCoords.end();
					auto clickInPush = std::find(pushCoords.begin(), pushCoords.end(), xpos) != pushCoords.end();

					if (!clickInRelease) {
						releaseCoords.insert(releaseCoords.end(), xpos);
					}
					if(clickInPush) pushCoords.remove(xpos);
				}
				if (frameMode) {
					PlayLayer::releaseButton(self, state, player);

					auto clickInRelease = std::find(releaseCoords.begin(), releaseCoords.end(), frame) != releaseCoords.end();
					auto clickInPush = std::find(pushCoords.begin(), pushCoords.end(), frame) != pushCoords.end();

					if (!clickInRelease) {
						releaseCoords.insert(releaseCoords.end(), frame);
					}
					if (clickInPush) pushCoords.remove(frame);
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
	clicks = 0;

	FPSMultiplierSetup();

	auto winSize = CCDirector::sharedDirector()->getWinSize();

	auto menu = CCMenu::create();
	menu->setPositionX(8);
	menu->setPositionY(35);

	auto VBotText = CCLabelBMFont::create("VBot v1.10", "goldFont.fnt");
	VBotText->setScale(0.6);
	VBotText->setAnchorPoint({ 0, 0.5 });

	auto EnabledText = CCLabelBMFont::create("Enabled", "goldFont.fnt");
	EnabledText->setScale(0.6);
	EnabledText->setAnchorPoint({ 0, 0.5 });

	auto ModeText = CCLabelBMFont::create("Mode: Playback", "goldFont.fnt");
	ModeText->setScale(0.6);
	ModeText->setAnchorPoint({ 0, 0.5 });

	auto xposString = (std::string)"???";
	auto xposCString = xposString.c_str();
	auto PositionText = CCLabelBMFont::create(xposCString, "goldFont.fnt");
	PositionText->setScale(0.6);
	PositionText->setAnchorPoint({ 0, 0.5 });

	auto ClicksText = CCLabelBMFont::create("Clicks: 0", "goldFont.fnt");
	ClicksText->setScale(0.6);
	ClicksText->setAnchorPoint({ 0, 0.5 });

	auto InputDisabledText = CCLabelBMFont::create("Input is disabled in Playback mode", "bigFont.fnt");
	InputDisabledText->setAnchorPoint({ 0.5, 0.5 });
	InputDisabledText->setPositionX(winSize.width / 2);
	InputDisabledText->setPositionY(65);
	InputDisabledText->setScale(0.75);
	InputDisabledText->setOpacity(0);

	auto SwitchRecordText = CCLabelBMFont::create("Switch to record mode in the pause menu", "bigFont.fnt");
	SwitchRecordText->setAnchorPoint({ 0.5, 0.5 });
	SwitchRecordText->setPositionX(winSize.width / 2);
	SwitchRecordText->setPositionY(47);
	SwitchRecordText->setScale(0.50);
	SwitchRecordText->setOpacity(0);

	auto MouseUpSprite = CCSprite::create("GJ_button_04.png");
	auto MouseDownSprite = CCSprite::create("GJ_button_01.png");
	MouseUpSprite->setPositionX(winSize.width - (MouseUpSprite->getScaledContentSize().width / 2) - 5);
	MouseDownSprite->setPositionX(winSize.width - (MouseDownSprite->getScaledContentSize().width / 2) - 5);
	MouseUpSprite->setPositionY(-14);
	MouseDownSprite->setPositionY(-14);
	MouseDownSprite->setVisible(false);
		
	menu->setTag(10000);
	PositionText->setTag(10001);
	ModeText->setTag(10002);
	VBotText->setTag(10003);
	InputDisabledText->setTag(10004);
	SwitchRecordText->setTag(10005);
	ClicksText->setTag(10006);
	MouseUpSprite->setTag(10007);
	MouseDownSprite->setTag(10008);
	EnabledText->setTag(10009);

	menu->setZOrder(1000);
	PositionText->setZOrder(1000);
	ModeText->setZOrder(1000);
	VBotText->setZOrder(1000);
	InputDisabledText->setZOrder(1000);
	SwitchRecordText->setZOrder(1000);
	ClicksText->setZOrder(1000);
	MouseUpSprite->setZOrder(1000);
	MouseDownSprite->setZOrder(1001);
	EnabledText->setZOrder(1000);

	menu->addChild(VBotText);
	menu->addChild(PositionText);
	menu->addChild(ModeText);
	menu->addChild(ClicksText);
	menu->addChild(EnabledText);
	menu->alignItemsVerticallyWithPadding(-3);
	menu->addChild(InputDisabledText);
	menu->addChild(SwitchRecordText);
	menu->addChild(MouseUpSprite);
	menu->addChild(MouseDownSprite);
	
	self->addChild(menu);

	if (!mode && autoLoad) VBotLayer::LoadMacro(gd::GameManager::sharedState()->m_pPlayLayer->m_level->m_sLevelName);

	return result;
}
void __fastcall PlayLayer::updateHook(CCLayer* self, int edx, float deltatime) {
	
	const unsigned short MSB = 0x8000;

	if (FrameAdvance) {
		if (GetAsyncKeyState(KEY_F) & MSB)
		{
			for (int i = 0; i != advanceFrames; i++)
			{
				PlayLayer::update(self, deltatime);
			}
			while (GetAsyncKeyState(KEY_F) & MSB) {
				Sleep(10);
			}
		}
	}
	else {
		PlayLayer::update(self, deltatime);
	}
	
	if (!hasSetFPS) {
		g_target_fps = 1 / deltatime;
		hasSetFPS = true;
	}

	auto menu = reinterpret_cast<CCMenu*>(self->getChildByTag(10000));
	auto ClicksText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10006));
	auto PositionText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10001));
	auto PlayLayer = gd::GameManager::sharedState()->getPlayLayer();

	if (enabled == false && HideUI) menu->setVisible(false);
	else menu->setVisible(true);

	if (MiniUI) menu->setPositionY(-17);
	else menu->setPositionY(35);

	if (lastXPos != VBotLayer::getXPos()) frame++;

	if (VBotLayer::getXPos() == 0) {
		cpframes.clear();
		cpframes.insert(cpframes.begin(), 0);
	}

	lastXPos = VBotLayer::getXPos();

	if (!frameMode) {
		xpos = VBotLayer::getXPos();
		auto xposString = (std::string)"XPos: " + std::to_string((float)xpos);
		PositionText->setString(xposString.c_str());
	} else {
		auto frameString = (std::string)"Frame: " + std::to_string((int)frame);
		PositionText->setString(frameString.c_str());
	}
	
	auto ModeText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10002));
	auto EnabledText = reinterpret_cast<CCLabelBMFont*>(menu->getChildByTag(10009));
	if (enabled){

		EnabledText->setString("State: Enabled"); 
		if (!mode) {
			ModeText->setString("Mode: Playback");
			PlayLayer::Playback_Code(self);
		}
		if (mode) {
			ModeText->setString("Mode: Record");
			PlayLayer::Record_Code(self);
			PlayLayer::AI(self);
			//PlayLayer::StraightFly(self);
			//PlayLayer::AutoWave(self);
		}
	}
	else EnabledText->setString("State: Disabled");
	
	auto ClicksString = (std::string)"Clicks: " + std::to_string(clicks);
	ClicksText->setString(ClicksString.c_str(), "goldFont.fnt");
}
void __fastcall PlayLayer::levelCompleteHook(void* self) {
	levelComplete(self);
	if (mode && autoSave && enabled) VBotLayer::SaveMacro(gd::GameManager::sharedState()->m_pPlayLayer->m_level->m_sLevelName);
	if (mode) VBotLayer::switchModeFunc();
}
void __fastcall PlayLayer::resetLevelHook(void* self) {
	resetLevel(self);

	gd::PlayLayer* playLayer = gd::GameManager::sharedState()->getPlayLayer();
	auto isPracticeMode = playLayer->m_isPracticeMode;

	frame = 0;

	if (cpframes.front() != 0) {
		cpframes.insert(cpframes.begin(), 0);
	}

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
				frame = cpframes.back();

				// Push Coords
				while (pushCoords.back() >= cpframes.back() && pushCoords.size() != 0) {
					pushCoords.pop_back();
				}

				// Release Coords
				while (releaseCoords.back() >= cpframes.back() && releaseCoords.size() != 0) {
					releaseCoords.pop_back();
				}
			}
			waitingForFirstClick = false;
		}
		else {
			cpframes.clear();
			cpframes.insert(cpframes.begin(), 0);
			waitingForFirstClick = true;
			clicks = 0;
		}

		if (mouseDown) {
			if (waitingForFirstClick) {
				pushCoords.clear();
				releaseCoords.clear();
				waitingForFirstClick = false;
			}

			if (!frameMode) {
				auto clickInPush = std::find(pushCoords.begin(), pushCoords.end(), xpos) != pushCoords.end();
				auto clickInRelease = std::find(releaseCoords.begin(), releaseCoords.end(), xpos) != releaseCoords.end();
				if (!clickInPush) pushCoords.insert(pushCoords.end(), xpos);
				if (clickInRelease) releaseCoords.remove(xpos);
			}
			else {
				auto clickInPush = std::find(pushCoords.begin(), pushCoords.end(), frame) != pushCoords.end();
				auto clickInRelease = std::find(releaseCoords.begin(), releaseCoords.end(), frame) != releaseCoords.end();
				if (!clickInPush) pushCoords.insert(pushCoords.end(), frame);
				if (clickInRelease) releaseCoords.remove(frame);
			}
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

	auto openLayerButtonSprite = gd::ButtonSprite::create("V", 0, false, "goldFont.fnt", "GJ_button_01.png", 0, 1);
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

	auto options = extension::CCScale9Sprite::create("GJ_square01.png");
	options->setContentSize({ winSize.width - 20, winSize.height - 10 });
	options->setPosition({ winSize.width / 2, winSize.height / 2 });

	auto VBotText = CCLabelBMFont::create("VBot Options", "goldFont.fnt");
	VBotText->setPositionX(winSize.width / 2);
	VBotText->setPositionY(winSize.height - 25);
	VBotText->setScale(0.8);

	auto ModeText = CCLabelBMFont::create("Switch Mode: ", "goldFont.fnt");
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

	auto saveButtonSprite = gd::ButtonSprite::create("Save Macro", 0, false, "bigFont.fnt", "GJ_button_03.png", 0, 1);
	saveButtonSprite->setScale(0.5);
	auto saveButton = gd::CCMenuItemSpriteExtra::create(saveButtonSprite, menu, menu_selector(VBotLayer::callbacks::SaveMacroCallback));
	saveButton->setPositionX(95);
	saveButton->setPositionY(245);

	auto loadButtonSprite = gd::ButtonSprite::create("Load Macro", 0, false, "bigFont.fnt", "GJ_button_02.png", 0, 1);
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
	auto frameModeText = CCLabelBMFont::create("Frame Mode", "bigFont.fnt");
	auto frameModeButton = gd::CCMenuItemToggler::create((frameMode) ? checkOn : checkOff, (frameMode) ? checkOff : checkOn, frameModeMenu, menu_selector(VBotLayer::callbacks::switchFrameMode));
	frameModeMenu->addChild(frameModeText);
	frameModeMenu->addChild(frameModeButton);
	frameModeMenu->alignItemsHorizontallyWithPadding(10);
	frameModeMenu->setAnchorPoint({ 0, 0.5 });
	frameModeMenu->setScale(0.5);

	auto frameAdvanceMenu = CCMenu::create();
	auto frameAdvanceText = CCLabelBMFont::create("Frame Advance", "bigFont.fnt");
	auto frameAdvanceButton = gd::CCMenuItemToggler::create((FrameAdvance) ? checkOn : checkOff, (FrameAdvance) ? checkOff : checkOn, frameAdvanceMenu, menu_selector(VBotLayer::callbacks::switchFrameAdvance));
	frameAdvanceMenu->addChild(frameAdvanceText);
	frameAdvanceMenu->addChild(frameAdvanceButton);
	frameAdvanceMenu->alignItemsHorizontallyWithPadding(10);
	frameAdvanceMenu->setAnchorPoint({ 0, 0.5 });
	frameAdvanceMenu->setScale(0.5);

	auto hideUIMenu = CCMenu::create();
	auto hideUIText = CCLabelBMFont::create("Hide UI On Disabled", "bigFont.fnt");
	auto hideUIButton = gd::CCMenuItemToggler::create((HideUI) ? checkOn : checkOff, (HideUI) ? checkOff : checkOn, hideUIMenu, menu_selector(VBotLayer::callbacks::switchHideUI));
	hideUIMenu->addChild(hideUIText);
	hideUIMenu->addChild(hideUIButton);
	hideUIMenu->alignItemsHorizontallyWithPadding(10);
	hideUIMenu->setAnchorPoint({ 0, 0.5 });
	hideUIMenu->setScale(0.5);

	auto miniUIMenu = CCMenu::create();
	auto miniUIText = CCLabelBMFont::create("Minimal UI", "bigFont.fnt");
	auto miniUIButton = gd::CCMenuItemToggler::create((MiniUI) ? checkOn : checkOff, (MiniUI) ? checkOff : checkOn, miniUIMenu, menu_selector(VBotLayer::callbacks::switchMiniUI));
	miniUIMenu->addChild(miniUIText);
	miniUIMenu->addChild(miniUIButton);
	miniUIMenu->alignItemsHorizontallyWithPadding(10);
	miniUIMenu->setAnchorPoint({ 0, 0.5 });
	miniUIMenu->setScale(0.5);

	auto centerMenu = CCMenu::create();
	centerMenu->addChild(frameModeMenu);
	centerMenu->addChild(frameAdvanceMenu);
	centerMenu->addChild(hideUIMenu);
	centerMenu->addChild(miniUIMenu);
	centerMenu->setPosition({ winSize.width / 2, 100 });
	centerMenu->alignItemsVerticallyWithPadding(-145);


	MacroPage = 1;
	VBotLayer::CreateMacroList(menu);


	auto speedMenu = CCMenu::create();
	auto speedhackText = CCLabelBMFont::create("Speedhack", "bigFont.fnt");
	auto speedhackInput = gd::CCTextInputNode::create("Speed", speedMenu, "bigFont.fnt", 50, 30);
	auto speedhackInputBG = extension::CCScale9Sprite::create("square02_small.png");
	auto speedhackSetSpeedSprite = gd::ButtonSprite::create("Set Speed", 0, false, "bigFont.fnt", "GJ_button_01.png", 28, 0.25);
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
	auto setFPSText = CCLabelBMFont::create("FPS Bypass", "bigFont.fnt");
	auto setFPSInput = gd::CCTextInputNode::create("FPS", fpsMenu, "bigFont.fnt", 50, 30);
	auto setFPSInputBG = extension::CCScale9Sprite::create("square02_small.png");
	auto setFPSCapSprite = gd::ButtonSprite::create("Set FPS", 0, false, "bigFont.fnt", "GJ_button_01.png", 28, 0.25);
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
	auto closeLayerBtn = gd::CCMenuItemSpriteExtra::create(closeLayerBtnSprite, menu, menu_selector(VBotLayer::callbacks::Close));
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
	centerMenu->setZOrder(1000);

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
	menu->addChild(centerMenu);
	self->addChild(menu);
}
void VBotLayer::SaveMacro(std::string levelName) {

	std::fstream levelfile;
	gd::GJGameLevel* GJGameLevel = gd::GameManager::sharedState()->getPlayLayer()->m_level;
	levelfile.open("VBot/Levels/" + GJGameLevel->m_sLevelName + ".vbotlevel", std::ios::out);

	// Save Level Data
	if (levelfile.is_open()) {
		levelfile << GJGameLevel->m_sLevelString; // 1
		levelfile << "\n";
		levelfile << std::to_string(GJGameLevel->m_nSongID); // 2
		levelfile << "\n";
		levelfile << std::to_string(GJGameLevel->m_nCoins); // 3
		levelfile << "\n";
		levelfile << GJGameLevel->m_sLevelName; // 4
		levelfile << "\n";
		levelfile << std::to_string(GJGameLevel->m_nObjectCount); // 5
		levelfile << "\n";
		levelfile << GJGameLevel->m_bTwoPlayerMode; // 6
	}

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

		gd::AchievementNotifier::sharedState()->notifyAchievement("Success!", "Saved Macro", "GJ_completesIcon_001.png", 1);

		if(gd::GameManager::sharedState()->getPlayLayer()->m_bIsPaused){
			auto PauseLayer = (CCLayer*)(CCDirector::sharedDirector()->getRunningScene()->getChildren()->objectAtIndex(1));
			VBotLayer::CreateMacroList((CCMenu*)PauseLayer->getChildren()->lastObject());
		}

	}
	else {
		gd::AchievementNotifier::sharedState()->notifyAchievement("Error", "Macro could not be saved", "GJ_deleteIcon_001.png", 1);
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

		auto attempts = gd::GameManager::sharedState()->m_pPlayLayer->m_currentAttempt;
		auto isPaused = gd::GameManager::sharedState()->m_pPlayLayer->m_bIsPaused;

		if (attempts > 1 || isPaused) {
			gd::AchievementNotifier::sharedState()->notifyAchievement("Success!", "Loaded Macro", "GJ_completesIcon_001.png", 1);
		}
	}
	else {
		if (frame > 0) {
			gd::AchievementNotifier::sharedState()->notifyAchievement("Error", "Macro could not be loaded", "GJ_deleteIcon_001.png", 1);
		}
	}
}
float VBotLayer::getXPos() {
	return gd::GameManager::sharedState()->m_pPlayLayer->m_pPlayer1->getPositionX();
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

	auto MacroListBG = extension::CCScale9Sprite::create("GJ_square01.png");
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
		auto MacroCard = extension::CCScale9Sprite::create("GJ_square01.png");
		MacroCard->setContentSize({ 145, 50 });
		MacroCard->setAnchorPoint({ 0.5, 0.5 });
		MacroCard->setPositionX(((winSize.width / 5) * 4) - (winSize.width / 2));
		MacroCard->setPositionY((winSize.height / 2) - (50 * i) - 60);

		auto MacroLabel = CCLabelBMFont::create(MacroName.c_str(), "bigFont.fnt");
		MacroLabel->setScale(0.2);
		MacroLabel->setPosition({ MacroCard->getPositionX() - 62, MacroCard->getPositionY() });
		MacroLabel->setAnchorPoint({ 0, 0.5 });

		MacroCardMenu->setZOrder(1);
		MacroCardMenu->addChild(MacroCard);
		MacroCardMenu->addChild(MacroLabel);

		auto MacroDelSprite = CCSprite::createWithSpriteFrameName("GJ_deleteSongBtn_001.png");

		cocos2d::CCMenuItemSprite* MacroDelBtn = CCMenuItemSprite::create(MacroDelSprite, MacroDelSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Del1));

		if (i == 0) { MacroDelBtn = gd::CCMenuItemSpriteExtra::create(MacroDelSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Del1)); }
		if (i == 1) { MacroDelBtn = gd::CCMenuItemSpriteExtra::create(MacroDelSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Del2)); }
		if (i == 2) { MacroDelBtn = gd::CCMenuItemSpriteExtra::create(MacroDelSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Del3)); }
		if (i == 3) { MacroDelBtn = gd::CCMenuItemSpriteExtra::create(MacroDelSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Del4)); }
		if (i == 4) { MacroDelBtn = gd::CCMenuItemSpriteExtra::create(MacroDelSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Del5)); }

		MacroDelBtn->setScale(0.5);
		MacroDelBtn->setPosition({ MacroCard->getPositionX() + 62 - (MacroDelBtn->getScaledContentSize().width / 2), MacroCard->getPositionY() });

		MacroCardMenu->addChild(MacroDelBtn);


		auto MacroLoadSprite = gd::ButtonSprite::create("Load", 0, FALSE, "bigFont.fnt", "GJ_button_02.png", 0, 0.5);

		gd::CCMenuItemSpriteExtra* MacroLoadBtn = gd::CCMenuItemSpriteExtra::create(MacroLoadSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Load1));

		if (i == 0) { MacroLoadBtn = gd::CCMenuItemSpriteExtra::create(MacroLoadSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Load1)); }
		if (i == 1) { MacroLoadBtn = gd::CCMenuItemSpriteExtra::create(MacroLoadSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Load2)); }
		if (i == 2) { MacroLoadBtn = gd::CCMenuItemSpriteExtra::create(MacroLoadSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Load3)); }
		if (i == 3) { MacroLoadBtn = gd::CCMenuItemSpriteExtra::create(MacroLoadSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Load4)); }
		if (i == 4) { MacroLoadBtn = gd::CCMenuItemSpriteExtra::create(MacroLoadSprite, MacroMenu, menu_selector(VBotLayer::callbacks::Load5)); }

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
	if (!mode && autoLoad) LoadMacro(gd::GameManager::sharedState()->m_pPlayLayer->m_level->m_sLevelName);
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
void VBotLayer::callbacks::switchFrameAdvance(CCObject*) {
	FrameAdvance = !FrameAdvance;
}
void VBotLayer::callbacks::switchHideUI(CCObject*) {
	HideUI = !HideUI;
}
void VBotLayer::callbacks::switchMiniUI(CCObject*) {
	MiniUI = !MiniUI;
}
void VBotLayer::callbacks::SaveMacroCallback(CCObject*) {
	VBotLayer::SaveMacro(gd::GameManager::sharedState()->m_pPlayLayer->m_level->m_sLevelName);
}
void VBotLayer::callbacks::LoadMacroCallback(CCObject*) {
	VBotLayer::LoadMacro(gd::GameManager::sharedState()->m_pPlayLayer->m_level->m_sLevelName);
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
	g_target_fps = fpscap; // FPS Multiplier
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