#pragma once
#include <Windows.h>
#include <tchar.h>
#include "memory.h"
//#include "Addresses.h"
#include "Assembly.h"

static HWND GetMSWindowHandle() {
	HWND msHandle = nullptr;
	TCHAR buf[256] = { 0 };
	ULONG procid;
	for (HWND hwnd = GetTopWindow(nullptr); hwnd != nullptr; hwnd = GetNextWindow(hwnd, GW_HWNDNEXT)) {
		GetWindowThreadProcessId(hwnd, &procid);
		if (procid != GetCurrentProcessId()) continue;
		if (!GetClassName(hwnd, buf, 256)) continue;
		if (_tcscmp(buf, _T("MapleStoryClass")) != 0) continue;
		msHandle = hwnd;
	}
	if (!msHandle) return nullptr;

	return msHandle;
}

#pragma managed
//Get MS ProcessID
static String^ GetMSProcID() {
	const std::string processID = std::to_string(GetCurrentProcessId());
	String^ procIdStr = ConvertStdToSystemStr(processID);

	return "0x" + procIdStr;
}

//Check if position is valid 
static bool isPosValid(int X, int Y) {
	return ((ReadPointerSigned(CWvsPhysicalSpace2DBase, OFS_LeftWall) <= X) && (ReadPointerSigned(CWvsPhysicalSpace2DBase, OFS_RightWall) >= X)
		&& (ReadPointerSigned(CWvsPhysicalSpace2DBase, OFS_TopWall) <= Y) && (ReadPointerSigned(CWvsPhysicalSpace2DBase, OFS_BottomWall) >= Y));
}

//Teleport to parameter position
static void Teleport(int X, int Y) {
	//if (isPosValid(X, Y)) {
	WritePointer(UserLocalBase, OFS_TeleX, X);
	WritePointer(UserLocalBase, OFS_TeleY, Y);
	WritePointer(UserLocalBase, OFS_Tele, 1);
	//WritePointer(UserLocalBase, OFS_Tele + 4, 1);
//}
}

//Teleport to parameter point
static void Teleport(POINT point) {
	if (isPosValid(point.x, point.y)) {
		WritePointer(UserLocalBase, OFS_TeleX, point.x);
		WritePointer(UserLocalBase, OFS_TeleY, point.y);
		WritePointer(UserLocalBase, OFS_Tele, 1);
		WritePointer(UserLocalBase, OFS_Tele + 4, 1);
	}
}
#pragma endregion

namespace PointerFuncs {
	bool isHooksEnabled = true; //For the future, disable pointers that requires Codecaves or function calls

	//Retrieve Char Level
	static String^ getCharLevel() {
		const UINT8 level = readCharValueZtlSecureFuse(*(ULONG*)CharacterStatBase + OFS_Level);
		if (level == 0) return "00";
		return Convert::ToString(level);
		/*if (isHooked)
			Jump(levelHookAddr, CodeCaves::LevelHook, 2);
		else
			WriteMemory(levelHookAddr, 7, 0x8A, 0xC8, 0xE8, 0x4B, 0x55, 0x00, 0x00);

		return CodeCaves::level.ToString();*/
	}

	//Retrieve Char Job ID
	static SHORT getCharJobID() {
		return readShortValueZtlSecureFuse(*(ULONG*)CharacterStatBase + OFS_JobID);
	}

	//Retrieve Char Job
	static String^ getCharJob() {
		/*if (isHooked)
			Jump(jobHookAddr, CodeCaves::JobHook, 2);
		else
			WriteMemory(levelHookAddr, 7, 0x8B, 0xC8, 0xE8, 0xA6, 0x55, 0x00, 0x00);

		if (CodeCaves::job == -1) return "Null";*/
		return gcnew String(GetJobName(getCharJobID()));
	}

	//Retrieve Char HP
	static String^ getCharHP() {
		if (isHooksEnabled)
			Jump(statHookAddr, Assembly::StatHook, 0);
		else
			WriteMemory(statHookAddr, 5, 0x8D, 0x0C, 0x80, 0x3B, 0xCB);

		if (Assembly::maxHP != 0)
			Assembly::hpPercent = ((double)Assembly::curHP / Assembly::maxHP) * 100.0;

		return Assembly::hpPercent.ToString("f2") + "%";
	}

	//Retrieve Char MP
	static String^ getCharMP() {
		if (isHooksEnabled)
			Jump(statHookAddr, Assembly::StatHook, 0);
		else
			WriteMemory(statHookAddr, 5, 0x8D, 0x0C, 0x80, 0x3B, 0xCB);

		if (Assembly::maxMP != 0)
			Assembly::mpPercent = ((double)Assembly::curMP / Assembly::maxMP) * 100.0;

		return Assembly::mpPercent.ToString("f2") + "%";
	}

	//Retrieve Char EXP
	static String^ getCharEXP() {
		if (isHooksEnabled)
			Jump(statHookAddr, Assembly::StatHook, 0);
		else
			WriteMemory(statHookAddr, 5, 0x8D, 0x0C, 0x80, 0x3B, 0xCB);

		if (Assembly::maxEXP != 0)
			Assembly::expPercent = ((double)Assembly::curEXP / Assembly::maxEXP) * 100.0;

		return Assembly::expPercent.ToString("f2") + "%";
	}

	//Retrieve Char Mesos
	static UINT getCharMesos() {
		return readLongValueZtlSecureFuse((ULONG*)(*(ULONG*)CharacterStatBase + OFS_Mesos));
	}

	//Retrieve Map Name
	static String^ getMapName() {
		if (isHooksEnabled)
			Jump(mapNameHookAddr, Assembly::MapNameHook, 1);
		else
			WriteMemory(mapNameHookAddr, 6, 0x89, 0x7D, 0xD8, 0x8D, 0x4D, 0xEC);

		if (Assembly::mapNameAddr == 0x0) return "Waiting..";

		char *mapNameBuff = (char*)(Assembly::mapNameAddr + 12);
		return gcnew String(mapNameBuff);
	}

	//Retrieve Left Wall coord
	static String^ getMapLeftWall() {
		return ReadPointerSignedInt(CWvsPhysicalSpace2DBase, OFS_LeftWall).ToString();
	}

	//Retrieve Right Wall coord
	static String^ getMapRightWall() {
		return ReadPointerSignedInt(CWvsPhysicalSpace2DBase, OFS_RightWall).ToString();
	}

	//Retrieve Top Wall coord
	static String^ getMapTopWall() {
		return ReadPointerSignedInt(CWvsPhysicalSpace2DBase, OFS_TopWall).ToString();
	}

	//Retrieve Bottom Wall coord
	static String^ getMapBottomWall() {
		return ReadPointerSignedInt(CWvsPhysicalSpace2DBase, OFS_BottomWall).ToString();
	}

	//Retrieve Char Name
	static String^ getCharName() {
		String^ charName = Runtime::InteropServices::Marshal::PtrToStringAnsi((IntPtr)(void*)(ReadPointerString(CharacterStatBase, OFS_Ign)));
		if (String::IsNullOrEmpty(charName)) return "CharName";
		return charName;
	}

	//Retrieve World
	static String^ getWorld() {
		const int worldID = ReadPointer(ServerBase, OFS_World);
		if (getCharName()->Equals("CharName")) return "Null";
		switch (worldID) {
		case 0:
			return("Scania");
		case 1:
			return("Bera");
		case 2:
			return("Broa");
		case 3:
			return("Windia");
		case 4:
			return("Khaini");
		case 5:
			return("Bellocan");
		case 6:
			return("Mardia");
		case 7:
			return("Kradia");
		case 8:
			return("Yellonde");
		case 9:
			return("Demethos");
		case 10:
			return("Galicia");
		case 11:
			return("El Nido");
		case 12:
			return("Zenith");
		case 13:
			return("Arcania");
		case 14:
			return("Chaos");
		case 15:
			return("Nova");
		case 16:
			return("Regenades");
		default:
			return("Null");
		}
	}

	//Retrieve Channel
	static String^ getChannel() {
		return (ReadPointer(ServerBase, OFS_Channel) + 1).ToString();
	}

	//Retrieve MapID
	static String^ getMapID() {
		return ReadPointerSignedInt(UIMiniMapBase, OFS_MapID).ToString();
	}

	//Retrieve Char Position
	static String^ getCharPos() {
		return "(" + ReadPointerSignedInt(UserLocalBase, OFS_CharX).ToString() + ", " + ReadPointerSignedInt(UserLocalBase, OFS_CharY).ToString() + ")";
	}

	//Retrieve Char X Position
	static String^ getCharPosX() {
		return ReadPointerSignedInt(UserLocalBase, OFS_CharX).ToString();
	}

	//Retrieve Char Y Position
	static String^ getCharPosY() {
		return ReadPointerSignedInt(UserLocalBase, OFS_CharY).ToString();
	}

	//Retrieve Mouse Position
	static String^ getMousePos() {
		return "(" + ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseX).ToString() + ", " + ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseY).ToString() + ")";
	}

	//Retrieve Mouse X Position
	static String^ getMousePosX() {
		return ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseX).ToString();
	}

	//Retrieve Mouse Y Position
	static String^ getMousePosY() {
		return ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseY).ToString();
	}

	//Retrieve Char Animation
	static String^ getCharAnimation() {
		return ReadPointerSignedInt(UserLocalBase, OFS_CharAnimation).ToString();
	}

	//Retrieve Char Foothold
	static String^ getCharFoothold() {
		return ReadPointerSignedInt(UserLocalBase, OFS_Foothold).ToString();
	}

	//Retrieve Attack Count
	static String^ getAttackCount() {
		return ReadPointerSignedInt(UserLocalBase, OFS_AttackCount).ToString();
	}

	//Retrieve Buff Count
	static String^ getBuffCount() {
		return (*(ULONG*)OFS_BuffCount).ToString();
	}

	//Retrieve Breath Count
	static String^ getBreathCount() {
		return ReadPointerSignedInt(UserLocalBase, OFS_Breath).ToString();
	}

	//Retrieve People Count
	static String^ getPeopleCount() {
		return ReadPointerSignedInt(UserPoolBase, OFS_PeopleCount).ToString();
	}

	//Retrieve Mob Count
	static String^ getMobCount() {
		return ReadPointerSignedInt(MobPoolBase, OFS_MobCount).ToString();
	}

	//Retrieve Item Count
	static String^ getItemCount() {
		return ReadPointerSignedInt(DropPoolBase, OFS_ItemCount).ToString();
	}

	//Retrieve Portal Count
	static String^ getPortalCount() {
		return ReadPointerSignedInt(PortalListBase, OFS_PortalCount).ToString();
	}

	//Retrieve NPC Count
	static String^ getNPCCount() {
		return ReadPointerSignedInt(NPCPoolBase, OFS_NPCCount).ToString();
	}
}

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

/*  TESTING AREA
static void OpenInventory() {
	const int iKey = 73;
	// use i hotkey
	SetMapleWindowToForeground();
	KeyboardInput::Keyboard::spamKey(iKey, 1);
	// wait till inventory opens
}

// Heuristics to sell all EQUIP
static void ReturnToTownAndSell(bool UseScroll)
{
	int delay = 150; //miliseconds
	SetMapleWindowToForeground();
	int leftMSBorder = GetMapleWindowRect().left;
	int topMSBorder = GetMapleWindowRect().top;

	// save current mapID and charPos for later use to return
	int oldMapID = Convert::ToInt32(PointerFuncs::getMapID());
	int oldCharPosX = Convert::ToInt32(PointerFuncs::getCharPosX());
	int oldCharPosy = Convert::ToInt32(PointerFuncs::getCharPosY());

	if (UseScroll) {
		// TODO: use packet?
		OpenInventory();
		// move to use tab and click
		//1612 - 1425 = 75 + 112 = 187
		//192 - 60 = 132
		const int useTabX = oldCharPosX - 185;
		const int useTabY = oldCharPosy - 130;
		MouseInput::Mouse::moveTo(leftMSBorder + useTabX, topMSBorder + useTabY);
		MouseInput::Mouse::leftClick();
		Sleep(delay);
		// move to first use slot and click
		int firstSlotX = useTabX - 45;
		int firstSlotY = useTabY + 25;
		MouseInput::Mouse::moveTo(leftMSBorder + firstSlotX, topMSBorder + firstSlotY);
		MouseInput::Mouse::leftClick();
		Sleep(delay);
	}
	else {
		// TODO: mapRush to town
	}

	// check where i am and teleport accordingly before npc
	int townMapID = Convert::ToInt32(PointerFuncs::getMapID());
	if (townMapID == 261000000) { //magatia sunset road
		// TODO: find npc coords
		const int npcX = 1328;
		const int npcY = 192;
		// teleport before npc's X,Y
		Teleport(npcX - 40, npcY);
		Sleep(delay);
		// double click npc to open sell dialog
		MouseInput::Mouse::moveTo(leftMSBorder + npcX, topMSBorder + npcY);
		MouseInput::Mouse::leftClick();
		Sleep(150);
	}
	else if (townMapID == 0000613) {
		// teleport to X1,Y1
		//int npcX;
		//int npcY;
		// double click npc to open sell dialog
		//MouseInput::Mouse::rightDoubleClickAt(npcX, npcY);
	}
	else if (townMapID <= 0) {
		Log::WriteLine("ReturnToTownAndSell: Error townMapID <= 0 after using return scroll!" + " townMapID: " + townMapID.ToString());
		return;
	}

	// click at tradeUI and drag it few pixels down
	// this aligns the confirmation box OK with items
	const int tradeUiX = 1340;
	const int tradeUiY = 30;
	MouseInput::Mouse::moveTo(leftMSBorder + tradeUiX, tradeUiY);
	MouseInput::Mouse::leftDragClickTo(leftMSBorder + tradeUiX, topMSBorder + tradeUiY + 60);
	Sleep(delay);

	// move mouse to position to spam double click
	const int confirmBoxOkX = 1360;
	const int confirmBoxOkY = 175;
	MouseInput::Mouse::moveTo(leftMSBorder + confirmBoxOkX, topMSBorder + confirmBoxOkY);
	Sleep(delay);

	for (int i = 0; i < 40; i++) {
		MouseInput::Mouse::doubleLeftClick();
		Sleep(100);
	}
	// map rush back to saved MapID and Position
	// enable hacks & macro continue hacking
}
*/