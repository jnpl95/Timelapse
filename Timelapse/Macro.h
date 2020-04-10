#pragma once
#include <cliext/queue>
#include "MainForm.h"
#include <Windows.h>
#include <winuser.h>
#include "Log.h"
#include "MapleFunctions.h"
#include "Addresses.h"

bool DBG_Macro = true;
enum class MacroType { LOOTMACRO = 1, ATTACKMACRO = 2, BUFFMACRO = 3, MPPOTMACRO = 4, HPPOTMACRO = 5, AUTOLOGIN = 6};
ref struct MacrosEnabled { static bool bMacroHP = false, bMacroMP = false, bMacroAttack = false, bMacroLoot = false; };

// Globally defined key data structure
typedef struct tag_KEYDATA {
	WORD keyRepeatCnt; // 16 bits
	BYTE keyScanCode; // 8 bits
	// additional keys on the enhanced keyboard he extended keys consist of the ALT and CTRL keys on the right-hand side of the keyboard;
	// the INS, DEL, HOME, END, PAGE UP, PAGE DOWN, and arrow keys in the clusters to the left of the numeric keypad;
	// the NUM LOCK key; the BREAK (CTRL+PAUSE) key; the PRINT SCRN key; and the divide (/) and ENTER keys in the numeric keypad.
	BYTE flagExtendedKey : 1; 
	BYTE Reserved : 3;
	BYTE flagAltDown : 1;
	BYTE flagRepeat : 1;
	BYTE flagUp : 1;
} KEYDATA;

static DWORD createKeyData(int Key) {
	KEYDATA kd;
	memset(&kd, 0, sizeof(KEYDATA));
	kd.keyRepeatCnt = 1;
	kd.keyScanCode = static_cast<BYTE>(MapVirtualKey(Key, 0));
	kd.flagExtendedKey = 0;
	kd.Reserved = 0;
	kd.flagAltDown = 0; // this has to be 0 else no lumping of messages
	kd.flagRepeat = 1; // this is necessary for proper execution in maplestory
	kd.flagUp = 0;
	DWORD dwVal;
	memcpy(&dwVal, &kd, sizeof(DWORD));

	return dwVal;
}

ref struct KeyMacro {
	int keyCode;
	MacroType macroType;

	bool operator<(const KeyMacro^ km) {
		return macroType < km->macroType;
	}

	// Funny thing is this is different from holding a key down and yet its different from holding and then releasing key down
	// basically this represents magical keyboard which repeatedly presses keys down but doesn't need any kind of going up with keys
	// thus this non-physical keyboard will cause certain skills like charge abilities to be permanently stuck charging
	static void SendKey(int Key) {
		PostMessage(GlobalVars::mapleWindow, WM_KEYDOWN, Key, createKeyData(Key));
		if (DBG_Macro) {
			Log::WriteLineToConsole("SendKey: ERROR failed to post message to process!");
			Log::WriteLineToConsole("Key: " + Key.ToString() + " MSG: WM_KEYDOWN " + "KeyData: " + createKeyData(Key));
		}
	}

	// this is a minor hack to boost speed of hotkeys
	// we still need much more control over message que and its data :(
	// if (keyRepeatCnt > 0 && flagRepeat = 1 && flagAltDown = 0)
	// then OS should lump these into one message and process it much faster
	static void SpamSendKey(int Key, int times) {	
		for (int i = 0; i < times; i++) {
			PostMessage(GlobalVars::mapleWindow, WM_KEYDOWN, Key, createKeyData(Key));
			 if (DBG_Macro) {
				 Log::WriteLineToConsole("SpamSendKey: ERROR failed to post message to process!");
				 Log::WriteLineToConsole("Key: " + Key.ToString() + " MSG: WM_KEYDOWN " + "KeyData: " + createKeyData(Key));
			 }
		}
	}

	// This simulates repeated bashing of a keystroke on a regular physical keyboard
	static void PressKey(int Key) {
		PostMessage(GlobalVars::mapleWindow, WM_KEYDOWN, Key, createKeyData(Key));
		if (DBG_Macro) {
			Log::WriteLineToConsole("PressKey: ERROR failed to post message to process!");
			Log::WriteLineToConsole("Key: " + Key.ToString() + " MSG: WM_KEYDOWN " + "KeyData: " + createKeyData(Key));
		}
		//WM_CHAR
		PostMessage(GlobalVars::mapleWindow, WM_KEYUP, Key, createKeyData(Key));
		if (DBG_Macro) {
			Log::WriteLineToConsole("PressKey: ERROR failed to post message to process!");
			Log::WriteLineToConsole("Key: " + Key.ToString() + " MSG: WM_KEYDOWN " + "KeyData: " + createKeyData(Key));
		}
	}

	// unused for now
	static void SpamPressKey(int Key, int times) {
		for (int i = 0; i < times; i++) {
			PostMessage(GlobalVars::mapleWindow, WM_KEYDOWN, Key, createKeyData(Key));
			//WM_CHAR
			PostMessage(GlobalVars::mapleWindow, WM_KEYUP, Key, createKeyData(Key));
		}
	}
};

ref class PriorityQueue {
public:
	static cliext::priority_queue<KeyMacro^>^ macroQueue = gcnew cliext::priority_queue<KeyMacro^>();
	static bool closeMacroQueue = false;

	static void MacroQueueWorker() {
		if (GlobalVars::mapleWindow == nullptr) 
			GlobalVars::mapleWindow = GetMSWindowHandle();

		while(!closeMacroQueue) {
			if (macroQueue == nullptr || macroQueue->empty())
				continue;
			
			Threading::Monitor::Enter(macroQueue);
			KeyMacro ^key = macroQueue->top();
			macroQueue->pop();		
			if (DBG_Macro) Log::WriteLineToConsole("MacroQueueSizeAfterPop: " + macroQueue->size());

			switch(key->macroType) {
				case MacroType::BUFFMACRO:
					if (HelperFuncs::IsInGame()) {
						KeyMacro::PressKey(key->keyCode);
					}
					break;
				case MacroType::HPPOTMACRO:
					if (MacrosEnabled::bMacroHP && HelperFuncs::IsInGame()) {
						KeyMacro::PressKey(key->keyCode);
					}
					break;
				case MacroType::MPPOTMACRO:
					if (MacrosEnabled::bMacroMP && HelperFuncs::IsInGame()) {
						KeyMacro::PressKey(key->keyCode);
					}
					break;
				case MacroType::LOOTMACRO:
					if (MacrosEnabled::bMacroLoot && HelperFuncs::ValidToLoot())
						KeyMacro::SpamSendKey(key->keyCode, 2);
					break;
				case MacroType::ATTACKMACRO:
					if (MacrosEnabled::bMacroAttack && HelperFuncs::ValidToAttack())
						KeyMacro::SpamSendKey(key->keyCode, 2);
					break;
			}
			Threading::Monitor::Exit(macroQueue);
		}
	}
};

ref class Macro {
	Threading::Timer^ timer;

	void SendKeyMacroToQueue() {
		if (DBG_Macro) Log::WriteLineToConsole("Entered SendKeyToQueue");

		KeyMacro^ keyMacro = gcnew KeyMacro();
		keyMacro->keyCode = keyCode;
		keyMacro->macroType = macroType;			

		switch (keyMacro->macroType) {
			// TODO: buffs need better handling
			case MacroType::BUFFMACRO:		
				if (DBG_Macro) Log::WriteLineToConsole("Pushing macro to queue: " + MacroTypeToStr(keyMacro->macroType));
				PriorityQueue::macroQueue->push(keyMacro);
			break;
			case MacroType::HPPOTMACRO:
				if (MacrosEnabled::bMacroHP && HelperFuncs::IsInGame()) {
					if (String::IsNullOrWhiteSpace(Timelapse::MainForm::TheInstance->tbHP->Text)) break;
					const int hpCntDrinkLimit = Convert::ToUInt32(Timelapse::MainForm::TheInstance->tbHP->Text);
					const int hpCntCurrent = Assembly::curHP;

					if (hpCntCurrent < hpCntDrinkLimit) {
						if (DBG_Macro) Log::WriteLineToConsole("Pushing macro to queue: " + MacroTypeToStr(keyMacro->macroType));
						PriorityQueue::macroQueue->push(keyMacro);
					}
				}
			break;
			case MacroType::MPPOTMACRO:
				if (MacrosEnabled::bMacroMP && HelperFuncs::IsInGame()) {
					if (String::IsNullOrWhiteSpace(Timelapse::MainForm::TheInstance->tbMP->Text)) break;
					const int mpCntDrinkLimit = Convert::ToUInt32(Timelapse::MainForm::TheInstance->tbMP->Text);
					const int mpCntCurrent = Assembly::curMP;

					if (mpCntCurrent < mpCntDrinkLimit) {
						if (DBG_Macro) Log::WriteLineToConsole("Pushing macro to queue: " + MacroTypeToStr(keyMacro->macroType));
						PriorityQueue::macroQueue->push(keyMacro);
					}
				}
			break;
			case MacroType::LOOTMACRO:
				if (MacrosEnabled::bMacroLoot && HelperFuncs::ValidToLoot()) {
					if (String::IsNullOrWhiteSpace(Timelapse::MainForm::TheInstance->tbLootItem->Text)) break;
					const int itemCntLootLimit = Convert::ToUInt32(Timelapse::MainForm::TheInstance->tbLootItem->Text);
					const int itemCntCurrent = ReadPointer(DropPoolBase, OFS_ItemCount);

					if (itemCntCurrent > itemCntLootLimit) {
						if (DBG_Macro) Log::WriteLineToConsole("Pushing macro to queue: " + MacroTypeToStr(keyMacro->macroType));
						PriorityQueue::macroQueue->push(keyMacro);
					}
				}
			break;
			case MacroType::ATTACKMACRO:
				if (MacrosEnabled::bMacroAttack && HelperFuncs::ValidToAttack()) {
					if (String::IsNullOrWhiteSpace(Timelapse::MainForm::TheInstance->tbAttackMob->Text)) break;
					const int mobCntAttackLimit = Convert::ToUInt32(Timelapse::MainForm::TheInstance->tbAttackMob->Text);
					const int mobCntCurrent = ReadPointer(MobPoolBase, OFS_MobCount);

					if (mobCntCurrent > mobCntAttackLimit) {
						if (DBG_Macro) Log::WriteLineToConsole("Pushing macro to queue: " + MacroTypeToStr(keyMacro->macroType));
						PriorityQueue::macroQueue->push(keyMacro);
					}
				}
			break;
		}
	}

	void TimerElapsed(Object^ state) {
		SendKeyMacroToQueue();
	}

	static void DebugMacro(Macro^ macro) {
		String^ macroKeyCode = ("\nMacro keyCode: " + macro->keyCode);
		String^ macroDelay = ("\nMacro delay: " + macro->delay);
		String^ macroType = ("\nMacro type: " + MacroTypeToStr(macro->macroType));
		Log::WriteLineToConsole("Macro: " + macroKeyCode + macroDelay + macroType);
	}

public:
	int keyCode, delay;
	MacroType macroType;

	static String^ MacroTypeToStr(MacroType macroType) {
		switch (macroType) {
			case MacroType::LOOTMACRO:
				return "LootMacro";
			case MacroType::ATTACKMACRO:
				return "AttackMacro";
			case MacroType::BUFFMACRO:
				return "BuffMacro";
			case MacroType::MPPOTMACRO:
				return "MpPotMacro";
			case MacroType::HPPOTMACRO:
				return "HpPotMacro";
			case MacroType::AUTOLOGIN:
				return "AutoLogin";
		}
		if (DBG_Macro) Log::WriteLineToConsole("Error when parsing enum MacroType, unknown type!");
		return nullptr;
	}

	Macro(int keyCode, int delay, MacroType macro) {
		this->keyCode = keyCode;
		this->delay = delay;
		this->macroType = macro;
		if (DBG_Macro) DebugMacro(this);
		Threading::TimerCallback^ TimerDelegate = gcnew Threading::TimerCallback(this, &Macro::TimerElapsed);
		timer = gcnew Threading::Timer(TimerDelegate, nullptr, Threading::Timeout::Infinite, delay);
		if (DBG_Macro) Log::WriteLineToConsole("new timer: " + timer);
	}

	// TODO: if buffMacro check if i have it present if not cast it for first time
	void Toggle(bool enable) {	
		if (enable) {
			timer->Change(delay, delay);			
		}				
		else {
			timer->Change(Threading::Timeout::Infinite, Threading::Timeout::Infinite);					
		}
	}
};