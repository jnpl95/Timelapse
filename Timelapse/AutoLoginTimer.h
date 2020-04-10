#pragma once
#pragma once
#include <cliext/queue>
#include "MainForm.h"
#include <Windows.h>
#include <winuser.h>
#include "Log.h"
#include "MapleFunctions.h"
#include "Addresses.h"

typedef void (*funcptr)();

ref class AutoLoginTimer {
	Threading::Timer^ timer;

	void TimerElapsed(Object^ state) {
		if (*(BYTE*)(*(ULONG*)LoginBase + OFS_LoginScreen) == 255) {
			funcptr();
		}
	}

public:
	int delay;
	void (*funcptr)();

	AutoLoginTimer(int delay,void (*funcptr)())
	{
		this->funcptr = funcptr;
		Threading::TimerCallback^ TimerDelegate = gcnew Threading::TimerCallback(this, &AutoLoginTimer::TimerElapsed);
		timer = gcnew Threading::Timer(TimerDelegate, nullptr, Threading::Timeout::Infinite, delay);
	}
	// TODO: if buffMacro check if i have it present if not cast it for first time
};