#pragma once
#include "Functions.h"

using namespace System;

#pragma region Helper Functions
bool IsInGame()
{
	int mapID = Convert::ToInt32(PointerFuncs::getMapID());

	if (!mapID == 0) //&& !PointerFuncs::getCharName()->Equals("CharName")
		return true;

	return false;
}

bool ValidToAttack()
{
	int attCnt = Convert::ToInt32(PointerFuncs::getAttackCount());

	// check for weapon and ammo
	if (!(attCnt > 99) && IsInGame())
		return true;

	return false;
}

bool ValidToLoot()
{
	int pplCnt = Convert::ToInt32(PointerFuncs::getPeopleCount());

	if (!(pplCnt > 0) && IsInGame())
		return true;

	return false;
}
#pragma endregion