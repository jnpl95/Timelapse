#ifndef MACRO_H
#define MACRO_H

#include <cliext\queue>
#include "Functions.h"
#include "MainForm.h"
#include <Windows.h>

enum class MacroType { LOOTMACRO = 1, ATTACKMACRO = 2, BUFFMACRO = 3, MPPOTMACRO = 4, HPPOTMACRO = 5};
ref struct MacrosEnabled { static bool bMacroHP = false, bMacroMP = false, bMacroAttack = false, bMacroLoot = false; };

ref struct KeyMacro {
	int keyCode;
	MacroType macroType;
	bool operator<(const KeyMacro^ km) {
		return macroType < km->macroType;
	}
};

ref class PriorityQueue {
public:
	static cliext::priority_queue<KeyMacro^>^ macroQueue = gcnew cliext::priority_queue<KeyMacro^>();
	static bool closeMacroQueue = false;

	static void MacroQueueWorker() {
		if (mapleWindow == nullptr) mapleWindow = GetMSWindowHandle();
		while(!closeMacroQueue) {
			if(macroQueue == nullptr || macroQueue->empty()) { Sleep(50); continue; }
			
			System::Threading::Monitor::Enter(PriorityQueue::macroQueue);
			KeyMacro ^key = macroQueue->top();
			macroQueue->pop();

			switch(key->macroType) {
				case MacroType::LOOTMACRO:
					if (MacrosEnabled::bMacroLoot) {
						if (System::String::IsNullOrWhiteSpace(Timelapse::MainForm::TheInstance->tbLootItem->Text)) break;
						if (ReadPointer(ItemBase, OFS_ItemCount) > System::Convert::ToUInt32(Timelapse::MainForm::TheInstance->tbLootItem->Text))
							PostMessage(mapleWindow, WM_KEYDOWN, key->keyCode, MapVirtualKey(key->keyCode, MAPVK_VK_TO_VSC) << 16);
					}
					break;
				case MacroType::ATTACKMACRO:
					if (MacrosEnabled::bMacroAttack) {
						if (System::String::IsNullOrWhiteSpace(Timelapse::MainForm::TheInstance->tbAttackMob->Text)) break;
						if (ReadPointer(MobBase, OFS_MobCount) > System::Convert::ToUInt32(Timelapse::MainForm::TheInstance->tbAttackMob->Text))
							PostMessage(mapleWindow, WM_KEYDOWN, key->keyCode, MapVirtualKey(key->keyCode, MAPVK_VK_TO_VSC) << 16);
					}
					break;
				case MacroType::BUFFMACRO:
					Sleep(15);
					PostMessage(mapleWindow, WM_KEYDOWN, key->keyCode, MapVirtualKey(key->keyCode, MAPVK_VK_TO_VSC) << 16);
					PostMessage(mapleWindow, WM_KEYUP, key->keyCode, MapVirtualKey(key->keyCode, MAPVK_VK_TO_VSC) << 16);
					Sleep(50);
					break;
				case MacroType::MPPOTMACRO:
					if(MacrosEnabled::bMacroMP) {
						if (System::String::IsNullOrWhiteSpace(Timelapse::MainForm::TheInstance->tbMP->Text)) break;
						if (CodeCaves::curMP < System::Convert::ToUInt32(Timelapse::MainForm::TheInstance->tbMP->Text))
							PostMessage(mapleWindow, WM_KEYDOWN, key->keyCode, MapVirtualKey(key->keyCode, MAPVK_VK_TO_VSC) << 16);
					}
						
					break;
				case MacroType::HPPOTMACRO:
					if (MacrosEnabled::bMacroHP) {
						if (System::String::IsNullOrWhiteSpace(Timelapse::MainForm::TheInstance->tbHP->Text)) break;
						if (CodeCaves::curHP < System::Convert::ToUInt32(Timelapse::MainForm::TheInstance->tbHP->Text))
							PostMessage(mapleWindow, WM_KEYDOWN, key->keyCode, MapVirtualKey(key->keyCode, MAPVK_VK_TO_VSC) << 16);
					}
					break;
			}
			System::Threading::Monitor::Exit(PriorityQueue::macroQueue);
		}
	}

};

ref class Macro {
private:
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