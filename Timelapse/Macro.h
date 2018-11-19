#ifndef MACRO_H
#define MACRO_H

#include <cliext\queue>
#include "Functions.h"
#include "MainForm.h"
#include <Windows.h>
#include "HelperFunctions.h"

enum class MacroType { LOOTMACRO = 1, ATTACKMACRO = 2, BUFFMACRO = 3, MPPOTMACRO = 4, HPPOTMACRO = 5};
ref struct MacrosEnabled { static bool bMacroHP = false, bMacroMP = false, bMacroAttack = false, bMacroLoot = false; };

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
		PostMessage(GlobalVars::mapleWindow, WM_KEYDOWN, Key, MapVirtualKey(Key, 0) << 16);
	}

	// This simulates repeated bashing of a keystroke on a regular physical keyboard
	static void SpamKey(int Key) {
		PostMessage(GlobalVars::mapleWindow, WM_KEYDOWN, Key, MapVirtualKey(Key, 0) << 16);
		PostMessage(GlobalVars::mapleWindow, WM_KEYUP, Key, MapVirtualKey(Key, 0) << 16);
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
			if(macroQueue == nullptr || macroQueue->empty()) {
				Sleep(50); 
				continue; 
			}
			
			System::Threading::Monitor::Enter(PriorityQueue::macroQueue);
			KeyMacro ^key = macroQueue->top();
			macroQueue->pop();

			switch(key->macroType) {
				case MacroType::LOOTMACRO:
					if (MacrosEnabled::bMacroLoot && ValidToLoot()) {
						if (System::String::IsNullOrWhiteSpace(Timelapse::MainForm::TheInstance->tbLootItem->Text)) break;
						if (ReadPointer(DropPoolBase, OFS_ItemCount) > System::Convert::ToUInt32(Timelapse::MainForm::TheInstance->tbLootItem->Text))
							KeyMacro::SendKey(key->keyCode);
					}
					break;

				case MacroType::ATTACKMACRO:
					if (MacrosEnabled::bMacroAttack && ValidToAttack()) {
						if (System::String::IsNullOrWhiteSpace(Timelapse::MainForm::TheInstance->tbAttackMob->Text)) break;
						if (ReadPointer(MobPoolBase, OFS_MobCount) > System::Convert::ToUInt32(Timelapse::MainForm::TheInstance->tbAttackMob->Text))
							KeyMacro::SpamKey(key->keyCode);
					}
					break;

				case MacroType::BUFFMACRO:
					// Using Sleep(X) in a one threaded code is a very bad idea, the entire thread goes to sleeping
					// Sleep(15); 
					KeyMacro::SpamKey(key->keyCode);
					// Sleep(50);
					break;

				case MacroType::MPPOTMACRO:
					if(MacrosEnabled::bMacroMP && IsInGame()) {
						if (System::String::IsNullOrWhiteSpace(Timelapse::MainForm::TheInstance->tbMP->Text)) break;
						if (CodeCaves::curMP < System::Convert::ToUInt32(Timelapse::MainForm::TheInstance->tbMP->Text))
							KeyMacro::SendKey(key->keyCode);
					}
						
					break;
				case MacroType::HPPOTMACRO:
					if (MacrosEnabled::bMacroHP && IsInGame()) {
						if (System::String::IsNullOrWhiteSpace(Timelapse::MainForm::TheInstance->tbHP->Text)) break;
						if (CodeCaves::curHP < System::Convert::ToUInt32(Timelapse::MainForm::TheInstance->tbHP->Text))
							KeyMacro::SendKey(key->keyCode);
					}
					break;
			}

			System::Threading::Monitor::Exit(PriorityQueue::macroQueue);
		}
	}

};

ref class Macro {
	System::Threading::Timer^ timer;

	void TimerElapsed(System::Object^ state) {
		//if ((macroType == MacroType::LOOTMACRO || macroType == MacroType::ATTACKMACRO) && PriorityQueue::macroQueue->size() > 20) return;
		KeyMacro^ keyMacro = gcnew KeyMacro();
		keyMacro->keyCode = keyCode;
		keyMacro->macroType = macroType;
		PriorityQueue::macroQueue->push(keyMacro);
	}
	
public:
	int keyCode, delay;
	MacroType macroType;

	Macro(int keyCode, int delay, MacroType macro) {
		this->keyCode = keyCode;
		this->delay = delay;
		this->macroType = macro;
		System::Threading::TimerCallback^ TimerDelegate = gcnew System::Threading::TimerCallback(this, &Macro::TimerElapsed);
		timer = gcnew System::Threading::Timer(TimerDelegate, nullptr, System::Threading::Timeout::Infinite, delay);
	}

	void Toggle(bool enable) {
		if (enable)
			timer->Change(delay, delay);
		else
			timer->Change(System::Threading::Timeout::Infinite, System::Threading::Timeout::Infinite);
	}
};

#endif