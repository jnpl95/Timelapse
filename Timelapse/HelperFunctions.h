#pragma once
#include "Functions.h"

using namespace System;

namespace HelperFuncs {

	static void SetMapleWindowToForeground()
	{
		SetForegroundWindow(GlobalVars::mapleWindow);
	}

	static RECT GetMapleWindowRect()
	{
		RECT msRect;
		GetWindowRect(GlobalVars::mapleWindow, &msRect);

		return  msRect;
	}

	static bool IsInGame()
	{
		const int mapID = Convert::ToInt32(PointerFuncs::getMapID());

		if (!mapID == 0) //&& !PointerFuncs::getCharName()->Equals("CharName")
			return true;

		return false;
	}

	static bool ValidToAttack()
	{
		const int attCnt = Convert::ToInt32(PointerFuncs::getAttackCount());

		// check for weapon and ammo
		if (!(attCnt > 99) && IsInGame())
			return true;

		return false;
	}

	static bool ValidToLoot()
	{
		const int pplCnt = Convert::ToInt32(PointerFuncs::getPeopleCount());

		if (!(pplCnt > 0) && IsInGame())
			return true;

		return false;
	}

	// Check if item count > 50 and AutoLoot is Checked
	static bool IsInventoryFull()
	{
		const int itemCnt = Convert::ToInt32(PointerFuncs::getItemCount());

		if (itemCnt > 50) // TODO: check if looting
			return true;

		return false;
	}
}