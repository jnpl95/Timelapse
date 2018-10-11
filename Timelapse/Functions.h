#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <Windows.h>
#include "Pointers.h"
#include <msclr\marshal_cppstd.h>
#include <string>
#include <atlstr.h>
#include <stdlib.h>
#include "Structs.h"

#define NewThread(threadFunc) CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&threadFunc, NULL, NULL, NULL);
//#define jmp(frm, to) (int)(((int)to - (int)frm) - 5);
HWND mapleWindow = nullptr;

#pragma region General Functions
static void MakePageWritable(ULONG ulAddress, ULONG ulSize) {
	MEMORY_BASIC_INFORMATION* mbi = new MEMORY_BASIC_INFORMATION;
	VirtualQuery((PVOID)ulAddress, mbi, ulSize);
	if (mbi->Protect != PAGE_EXECUTE_READWRITE) {
		ULONG* ulProtect = new unsigned long;
		VirtualProtect((PVOID)ulAddress, ulSize, PAGE_EXECUTE_READWRITE, ulProtect);
		delete ulProtect;
	}
	delete mbi;
}

static void Jump(ULONG ulAddress, PVOID Function, unsigned Nops) {
	MakePageWritable(ulAddress, Nops + 5);
	*(UCHAR*)ulAddress = 0xE9; 
	*(ULONG*)(ulAddress + 1) = (int)(((int)Function - (int)ulAddress) - 5);
	memset((PVOID)(ulAddress + 5), 0x90, Nops);
}

static ULONG ReadPointer(ULONG ulBase, int iOffset) {
	__try { return *(ULONG*)(*(ULONG*)ulBase + iOffset); }
	__except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

static LONG ReadPointerSigned(ULONG ulBase, int iOffset) {
	__try { return *(LONG*)(*(ULONG*)ulBase + iOffset); }
	__except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

static double ReadPointerDouble(ULONG ulBase, int iOffset) {
	__try { return *(double*)(*(ULONG*)ulBase + iOffset); }
	__except (EXCEPTION_EXECUTE_HANDLER) { return NULL; }
}

static LPCSTR ReadPointerString(ULONG ulBase, int iOffset) {
	__try { return (LPCSTR)(*(ULONG*)ulBase + iOffset); }
	__except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
}

static UINT8 readCharValueZtlSecureFuse(int at) {
	UCHAR v2;
	UINT8 result;
	try {
		v2 = *(PUCHAR)at;
		at = *(PUINT8)(at + 1);
		result = at ^ v2;
	}
	catch (...) { return 0; }
	return result;
}

static INT16 readShortValueZtlSecureFuse(int a1) {
	PUINT8 v2 = (PUINT8)(a1 + 2);
	DWORD v4 = (DWORD)&a1 - a1;

	v2[v4] = *v2 ^ *(v2 - 2);
	v2++;
	v2[v4] = *v2 ^ *(v2 - 2);

	return HIWORD(a1);
}

unsigned int readLongValueZtlSecureFuse(ULONG *a1) {
	return *a1 ^ _rotl(a1[1], 5);
}

#pragma unmanaged
static LONG_PTR ReadMultiPointerSigned(LONG_PTR ulBase, int level, ...) {
	LONG_PTR ulResult = 0;
	va_list vaarg;
	va_start(vaarg, level);

	if (!IsBadReadPtr((PVOID)ulBase, sizeof(LONG_PTR))) {
		ulBase = *(LONG_PTR*)ulBase;
		for (int i = 0; i < level; i++) {
			const int offset = va_arg(vaarg, int);
			if (IsBadReadPtr((PVOID)(ulBase + offset), sizeof(LONG_PTR))) {
				va_end(vaarg);
				return 0;
			}
			ulBase = *(LONG_PTR*)(ulBase + offset);
		}
		ulResult = ulBase;
	}

	va_end(vaarg);
	return ulResult;
}

static void WriteMemory(ULONG ulAddress, ULONG ulAmount, ...) {
	va_list va;
	va_start(va, ulAmount);

	MakePageWritable(ulAddress, ulAmount);
	for (ULONG ulIndex = 0; ulIndex < ulAmount; ulIndex++)
		*(UCHAR*)(ulAddress + ulIndex) = va_arg(va, UCHAR);

	va_end(va);
}

#pragma managed
static bool WritePointer(ULONG ulBase, int iOffset, int iValue) {
	__try { *(int*)(*(ULONG*)ulBase + iOffset) = iValue; return true; }
	__except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

//Convert String^ to std::string
static std::string ConvertSystemToStdStr(System::String^ str1) {
	return msclr::interop::marshal_as<std::string>(str1);
}

//Convert std::string to String^
static System::String^ ConvertStdToSystemStr(std::string str1) {
	return gcnew System::String(str1.c_str());
}

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

//Get MS ThreadID
static ULONG GetMSThreadID() {
	if(mapleWindow == nullptr)
		mapleWindow = GetMSWindowHandle();
	return GetWindowThreadProcessId(mapleWindow, nullptr);
}

//Get MS ProcessID
static System::String^ GetMSProcID() {
	return "0x" + GetCurrentProcessId().ToString("X");
}

//Get DLL Module Handle
static HMODULE GetCurrentModule() {
	HMODULE hModule = nullptr;
	GetModuleHandleEx( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)GetCurrentModule, &hModule);
	return hModule;
}

//Keycode based on index selected in combo boxes
static int keyCollection[] = { 0x10, 0x11, 0x12, 0x20, 0x2D, 0x2E, 0x24, 0x23, 0x21, 0x22, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51,
0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };

//Check if position is valid 
static bool isPosValid(int X, int Y) {
	return ((ReadPointerSigned(CWvsPhysicalSpace2DBase, OFS_LeftWall) <= X) && (ReadPointerSigned(CWvsPhysicalSpace2DBase, OFS_RightWall) >= X) 
		&& (ReadPointerSigned(CWvsPhysicalSpace2DBase, OFS_UpWall) <= Y) && (ReadPointerSigned(CWvsPhysicalSpace2DBase, OFS_DownWall) >= Y));
}

//Teleport to parameter position
static void Teleport(int X, int Y) {
	if (isPosValid(X, Y)) {
		WritePointer(UserLocalBase, OFS_TeleX, X);
		WritePointer(UserLocalBase, OFS_TeleY, Y);
		WritePointer(UserLocalBase, OFS_Tele, 1);
		WritePointer(UserLocalBase, OFS_Tele + 4, 1);
	}
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

//Check if keypress is valid
static bool isKeyValid(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e, bool isSigned) {
	bool result = true;

	//If the character is not a digit or backspace, don't allow it
	if (!isdigit(e->KeyChar) && e->KeyChar != '\b') result = false;

	//If the textbox is supposed to contain a signed value, check to see if '-' should be permitted
	if (isSigned && e->KeyChar == '-') {
		System::Windows::Forms::TextBox^ textBox = safe_cast<System::Windows::Forms::TextBox^>(sender);

		//If the selected text does not start at the beginning of the text, don't allow the '-' keypress
		if (textBox->SelectionStart > 0) result = false;
		//If the selection starts at 0 and there exists a '-' in the complete text, and the selected text doesn't have '-', don't allow an additional '-' keypress
		else if (textBox->Text->IndexOf('-') > -1 && !textBox->SelectedText->Contains("-")) result = false;
		else result = true;
	}

	return result;
}

#pragma endregion

namespace CodeCaves {
	ULONG curHP = 0, maxHP = 0, curMP = 0, maxMP = 0, curEXP = 0, maxEXP = 0, mapNameAddr = 0x0;
	int ItemX = 0, ItemY = 0;
	double hpPercent = 0.00, mpPercent = 0.00, expPercent = 0.00;
	static std::vector<SpawnControlStruct*>* spawnControl = new std::vector<SpawnControlStruct*>();

	static SpawnControlStruct* __stdcall getSpawnControlStruct() {
		if (spawnControl->size() == 0) return nullptr;
		for (std::vector<SpawnControlStruct*>::const_iterator i = spawnControl->begin(); i != spawnControl->end(); ++i)
			if ((*i)->mapID == ReadPointer(UIMiniMapBase, OFS_MapID))
				return (*i);
		return nullptr;
	}

#pragma unmanaged
	/*BYTE level = 0x00;
	__declspec(naked) static void __stdcall LevelHook() {
		__asm {
			mov [level], al
			mov cl, al
			call dword ptr [levelHookDecode]
			jmp dword ptr [levelHookAddrRet]
		}
	}*/

	/*SHORT job = -1;
	__declspec(naked) static void __stdcall JobHook() {
		__asm {
			mov [job], ax
			mov ecx, eax
			call dword ptr [jobHookDecode]
			jmp dword ptr [jobHookAddrRet]
		}
	}*/

	__declspec(naked) static void __stdcall StatHook() {
		__asm {
			push eax
			mov eax, [ebp + 0x08]
			mov [curHP], eax
			mov eax, [ebp + 0x0C]
			mov [maxHP], eax
			mov eax, [ebp + 0x10]
			mov [curMP], eax
			mov eax, [ebp + 0x14]
			mov [maxMP], eax
			mov eax, [ebp + 0x18]
			mov [curEXP], eax
			mov eax, [ebp + 0x1C]
			mov [maxEXP], eax
			pop eax
			lea ecx, [eax + eax * 0x4]
			cmp ecx, ebx
			jmp dword ptr [statHookAddrRet]
		}
	}

	/*
	ULONG mesos = 0;
	__declspec(naked) static void __stdcall MesosHook() {
		__asm {
			mov [mesos], eax
			lea edx, [esi + 0xA5]
			jmp dword ptr [mesosHookAddrRet]
		}
	}

	__declspec(naked) static void __stdcall MesosChangeHook() {
		__asm {
			mov [mesos], eax
			lea edx, [esi + 0xA5]
			jmp dword ptr [mesosChangeHookAddrRet]
		}
	}
	*/

	__declspec(naked) static void __stdcall MapNameHook() {
		__asm {
			mov [mapNameAddr], ecx
			mov [ebp - 0x28], edi
			lea ecx, [ebp - 0x14]
			jmp dword ptr [mapNameHookAddrRet]
		}
	}

	__declspec(naked) static void __stdcall ItemVacHook() {
		__asm {
			pushad
			mov ecx, [ebp + 0x8]
			mov ebx, [ebp - 0x24]
			mov [ecx], ebx
			mov [ecx + 0x4], eax
			mov ecx, eax
			mov eax, ebx

			lea edx, [eax - 0x19]
			mov [ebp - 0x34], edx
			lea edx, [ecx - 0x32]
			add eax, 0x19
			add ecx, 0xA
			mov [ebp - 0x30], edx
			mov [ebp - 0x2C], eax
			mov [ebp - 0x28], ecx
			popad

			push eax
			push dword ptr ss : [ebp - 0x24]
			lea eax, dword ptr ss : [ebp - 0x34]
			jmp dword ptr [itemVacAddrRet]
		}
	}

	__declspec(naked) static void __stdcall MouseFlyXHook() {
		__asm {
			push eax
			push ecx
			mov eax, [UserLocalBase]
			mov eax, [eax]
			mov ecx, [OFS_pID]
			mov eax, [eax + ecx]
			cmp esi, eax
			pop eax
			jne ReturnX
			mov eax, [InputBase]
			mov eax, [eax]
			mov ecx, [OFS_MouseLocation]
			mov eax, [eax + ecx]
			mov ecx, [OFS_MouseX]
			mov eax, [eax + ecx]
			ReturnX:
			pop ecx
			mov [ebx], eax
			mov edi, [ebp + 0x10]
			jmp dword ptr [mouseFlyXAddrRet]
		}
	}

	__declspec(naked) static void __stdcall MouseFlyYHook() {
		__asm {
			push eax
			push ecx
			mov eax, [UserLocalBase]
			mov eax, [eax]
			mov ecx, [OFS_pID]
			mov eax, [eax + ecx]
			cmp esi, eax
			pop eax
			jne ReturnY
			mov eax, [InputBase]
			mov eax, [eax]
			mov ecx, [OFS_MouseLocation]
			mov eax, [eax + ecx]
			mov ecx, [OFS_MouseY]
			mov eax, [eax + ecx]
			ReturnY:
			pop ecx
			mov [edi], eax
			mov ebx, [ebp + 0x14]
			jmp dword ptr [mouseFlyYAddrRet]
		}
	}

	__declspec(naked) static void __stdcall MobFreezeHook() {
		__asm {
			mov [esi + 0x00000248], 0x06
			mov eax, [esi + 0x00000248]
			jmp dword ptr [mobFreezeAddrRet]
		}
	}

	__declspec(naked) static void __stdcall MobAutoAggroHook() {
		__asm {
			call dword ptr [cVecCtrlWorkUpdateActiveCall] //calls CVecCtrl::WorkUpdateActive()
			push eax
			mov edx, [UserLocalBase]
			mov edx, [edx]
			mov eax, [OFS_pID]
			mov edx, [edx + eax]
			mov edx, [edx + 0x8]
			mov eax, [OFS_Aggro]
			mov [esi + eax], edx //Aggro Offset (first cmp before CVecCtrl::ChaseTarget)
			pop eax
			jmp dword ptr [mobAutoAggroAddrRet]
		}
	}

	__declspec(naked) static void __stdcall SpawnPointHook() {
		__asm {
			push ecx
			push edx
			cmp dword ptr [esp + 0x8], 0x009BBD5D
			je Return //If Spawning Mob, skip
			cmp dword ptr [esp + 0x8], 0x009C1D90
			je Return //If Spawning NPC, skip
			call getSpawnControlStruct //returns SpawnControlStruct* with the mapID == current mapID
			cmp eax, 0 
			je Return //If end of SpawnControl and no match, skip
			mov edx, [eax + 4]
			mov dword ptr[esp + 0x10], edx //x
			mov edx, [eax + 8]
			mov dword ptr[esp + 0x14], edx //y

			Return:
			pop edx
			pop ecx
			mov eax, 0x00AE56F4 //original code
			jmp dword ptr[spawnPointAddrRet]
		}
	}

	static void __declspec(naked) _stdcall ItemHook()
	{
		__asm
		{
			cmp dword ptr[esp], 0x005047B8
			jne NormalAPICall //If return not in CDropPool::TryPickUpDrop, skip
			push eax
			mov eax, [esp + 0x0C]
			mov [ItemX], eax
			mov eax, [esp + 0x10]
			mov [ItemY], eax
			pop eax

			NormalAPICall :
			jmp dword ptr PtInRect
		}
	}

#pragma managed
}

namespace PointerFuncs {
	bool isHooked = true;

	//Retrieve Char Level
	static System::String^ getCharLevel() {
		UINT8 level = readCharValueZtlSecureFuse(*(ULONG*)CharacterStatBase + OFS_Level);
		if (level == 0) return "00";
		return System::Convert::ToString(level);
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
	static System::String^ getCharJob() {
		/*if (isHooked)
			Jump(jobHookAddr, CodeCaves::JobHook, 2);
		else
			WriteMemory(levelHookAddr, 7, 0x8B, 0xC8, 0xE8, 0xA6, 0x55, 0x00, 0x00);

		if (CodeCaves::job == -1) return "Null";*/
		return gcnew System::String(GetJobName(getCharJobID()));
	}

	//Retrieve Char HP
	static System::String^ getCharHP() {
		if (isHooked)
			Jump(statHookAddr, CodeCaves::StatHook, 0);
		else
			WriteMemory(statHookAddr, 5, 0x8D, 0x0C, 0x80, 0x3B, 0xCB);
		if (CodeCaves::maxHP != 0)
			CodeCaves::hpPercent = ((double)CodeCaves::curHP / CodeCaves::maxHP) * 100.0;
		return CodeCaves::hpPercent.ToString("f2") + "%";
	}

	//Retrieve Char MP
	static System::String^ getCharMP() {
		if (isHooked)
			Jump(statHookAddr, CodeCaves::StatHook, 0);
		else
			WriteMemory(statHookAddr, 5, 0x8D, 0x0C, 0x80, 0x3B, 0xCB);
		if (CodeCaves::maxMP != 0)
			CodeCaves::mpPercent = ((double)CodeCaves::curMP / CodeCaves::maxMP) * 100.0;
		return CodeCaves::mpPercent.ToString("f2") + "%";
	}

	//Retrieve Char EXP
	static System::String^ getCharEXP() {
		if (isHooked)
			Jump(statHookAddr, CodeCaves::StatHook, 0);
		else
			WriteMemory(statHookAddr, 5, 0x8D, 0x0C, 0x80, 0x3B, 0xCB);
		if (CodeCaves::maxEXP != 0)
			CodeCaves::expPercent = ((double)CodeCaves::curEXP / CodeCaves::maxEXP) * 100.0;
		return CodeCaves::expPercent.ToString("f2") + "%";
	}

	//Retrieve Char Mesos
	static System::String^ getCharMesos() {
		/*if (isHooked) {
			Jump(mesosHookAddr, CodeCaves::MesosHook, 1);
			Jump(mesosChangeHookAddr, CodeCaves::MesosChangeHook, 1);
		}
		else {
			WriteMemory(mesosHookAddr, 6, 0x8D, 0x96, 0xA5, 0x00, 0x00, 0x00);
			WriteMemory(mesosChangeHookAddr, 6, 0x8D, 0x96, 0xA5, 0x00, 0x00, 0x00);
		}*/

		return readLongValueZtlSecureFuse((ULONG*)(*(ULONG*)CharacterStatBase + OFS_Mesos)).ToString("N0");
	}

	//Retrieve Map Name
	static System::String^ getMapName() {
		if (isHooked)
			Jump(mapNameHookAddr, CodeCaves::MapNameHook, 1);
		else
			WriteMemory(mapNameHookAddr, 6, 0x89, 0x7D, 0xD8, 0x8D, 0x4D, 0xEC);

		if (CodeCaves::mapNameAddr == 0x0) return "Waiting..";

		char *mapNameBuff = (char*)(CodeCaves::mapNameAddr + 12);
		return gcnew System::String(mapNameBuff);
	}

	//Retrieve Char Name
	static System::String^ getCharName() {
		System::String^ charName = System::Runtime::InteropServices::Marshal::PtrToStringAnsi((System::IntPtr)(void*)(ReadPointerString(CharacterStatBase, OFS_Ign)));
		if (System::String::IsNullOrEmpty(charName)) return "CharName";
		return charName;
	}

	//Retrieve World
	static System::String^ getWorld() {
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
	static System::String^ getChannel() {
		return ReadPointer(ServerBase, OFS_Channel).ToString();
	}

	//Retrieve MapID
	static System::String^ getMapID() {
		return ReadPointer(UIMiniMapBase, OFS_MapID).ToString();
	}

	//Retrieve Char Position
	static System::String^ getCharPos() {
		return "(" + ReadPointerSigned(UserLocalBase, OFS_CharX).ToString() + ", " + ReadPointerSigned(UserLocalBase, OFS_CharY).ToString() + ")";
	}

	//Retrieve Char X Position
	static System::String^ getCharPosX() {
		return ReadPointerSigned(UserLocalBase, OFS_CharX).ToString();
	}

	//Retrieve Char Y Position
	static System::String^ getCharPosY() {
		return ReadPointerSigned(UserLocalBase, OFS_CharY).ToString();
	}

	//Retrieve Mouse Position
	static System::String^ getMousePos() {
		return "(" + ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseX).ToString() + ", " + ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseY).ToString() + ")";
	}

	//Retrieve Mouse X Position
	static System::String^ getMousePosX() {
		return ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseX).ToString();
	}

	//Retrieve Mouse Y Position
	static System::String^ getMousePosY() {
		return ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseY).ToString();
	}

	//Retrieve Attack Count
	static System::String^ getAttackCount() {
		return ReadPointer(UserLocalBase, OFS_AttackCount).ToString();
	}

	//Retrieve Buff Count
	static System::String^ getBuffCount() {
		return (*(ULONG*)OFS_BuffCount).ToString();
	}

	//Retrieve Breath Count
	static System::String^ getBreathCount() {
		return ReadPointer(UserLocalBase, OFS_Breath).ToString();
	}

	//Retrieve People Count
	static System::String^ getPeopleCount() {
		return ReadPointer(UserPoolBase, OFS_PeopleCount).ToString();
	}

	//Retrieve Mob Count
	static System::String^ getMobCount() {
		return ReadPointer(MobPoolBase, OFS_MobCount).ToString();
	}

	//Retrieve Item Count
	static System::String^ getItemCount() {
		return ReadPointer(DropPoolBase, OFS_ItemCount).ToString();
	}

	//Retrieve Portal Count
	static System::String^ getPortalCount() {
		return ReadPointer(PortalListBase, OFS_PortalCount).ToString();
	}

	//Retrieve NPC Count
	static System::String^ getNPCCount() {
		return ReadPointer(NPCBase, OFS_NPCCount).ToString();
	}
}

#endif
