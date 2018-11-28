#pragma once
#include "Structs.h"
#include "resource.h"
#include "GeneralFunctions.h"

#define CodeCave(name) static void __declspec(naked) ##name() { _asm
#define EndCodeCave } 

namespace GlobalVars {
	static HMODULE hDLL;
	static HWND mapleWindow = nullptr;
}

namespace Assembly {
ULONG curHP = 0, maxHP = 0, curMP = 0, maxMP = 0, curEXP = 0, maxEXP = 0, mapNameAddr = 0x0;
int ItemX = 0, ItemY = 0, animDelay = 10; //TODO:: actually animDelay should be a Double
double hpPercent = 0.00, mpPercent = 0.00, expPercent = 0.00;
bool isItemLoggingEnabled = false, isItemFilterEnabled = false, isItemFilterWhiteList = true;
bool isMobLoggingEnabled = false, isMobFilterEnabled = false, isMobFilterWhiteList = true;
ULONG itemLogged = 0, itemFilterMesos = 0, mobLogged = 0;
static std::vector<ULONG> *itemList = new std::vector<ULONG>(), *mobList = new std::vector<ULONG>();
static std::vector<SpawnControlData*> *spawnControl = new std::vector<SpawnControlData*>();

//Find item name using item ID in the ItemsList resource
static String^ findItemNameFromID(int itemID) {
	try {
		std::string result = "", tmpStr = "";
		HRSRC hRes = FindResource(GlobalVars::hDLL, MAKEINTRESOURCE(ItemsList), _T("TEXT"));
		if (hRes == nullptr) return "";
		const HGLOBAL hGlob = LoadResource(GlobalVars::hDLL, hRes);
		if (hGlob == nullptr) return "";
		const CHAR* pData = reinterpret_cast<const CHAR*>(LockResource(hGlob));
		std::istringstream file(pData);

		while (file.good()) {
			std::getline(file, tmpStr);
			if (tmpStr.find(std::to_string(itemID)) == 0) {
				tmpStr = tmpStr.substr(tmpStr.find('[') + 1, tmpStr.find(']'));
				tmpStr = tmpStr.substr(0, tmpStr.length() - 2);
				result = tmpStr;
			}
		}
		UnlockResource(hRes);
		return ConvertStdToSystemStr(result);
	}
	catch (...) { return "Error"; }
}

//Find mob name using mob ID in the MobsList resource
static String^ findMobNameFromID(int mobID) {
	try {
		std::string result = "", tmpStr = "";
		HRSRC hRes = FindResource(GlobalVars::hDLL, MAKEINTRESOURCE(MobsList), _T("TEXT"));
		if (hRes == nullptr) return "";
		const HGLOBAL hGlob = LoadResource(GlobalVars::hDLL, hRes);
		if (hGlob == nullptr) return "";
		const CHAR* pData = reinterpret_cast<const CHAR*>(LockResource(hGlob));
		std::istringstream file(pData);

		while (file.good()) {
			std::getline(file, tmpStr);
			if (tmpStr.find(std::to_string(mobID)) == 0) {
				tmpStr = tmpStr.substr(tmpStr.find('[') + 1, tmpStr.find(']'));
				tmpStr = tmpStr.substr(0, tmpStr.length() - 2);
				result = tmpStr;
			}
		}
		UnlockResource(hRes);
		return ConvertStdToSystemStr(result);
	}
	catch (...) { return "Error"; }
}

static SpawnControlData* __stdcall getSpawnControlStruct() {
	if (spawnControl->size() == 0) return nullptr;
	for (SpawnControlData *spawnControlStruct : *spawnControl)
		if (spawnControlStruct->mapID == ReadPointer(UIMiniMapBase, OFS_MapID))
			return spawnControlStruct;
	return nullptr;
}

inline void __stdcall itemLog() {
	if (itemLogged < 50000) return; //Ignore mesos
		String^ itemName = findItemNameFromID(itemLogged);
	if (itemLogged > 0 && !Timelapse::MainForm::TheInstance->lbItemSearchLog->Items->Contains(itemName + " (" + itemLogged.ToString() + ")"))
		Timelapse::MainForm::TheInstance->lbItemSearchLog->Items->Add(itemName + " (" + itemLogged.ToString() + ")");
}

inline bool __stdcall shouldItemBeFiltered() {
	if (isItemFilterWhiteList) {
		for (ULONG item : *itemList)
			if (item == itemLogged) return false;
		return true;
	} 
	for (ULONG item : *itemList)
		if (item == itemLogged) return true;
	return false;	
}

inline void __stdcall mobLog() {
	String^ mobName = findMobNameFromID(mobLogged);
	if (mobLogged > 0 && !Timelapse::MainForm::TheInstance->lbMobSearchLog->Items->Contains(mobName + " (" + mobLogged.ToString() + ")"))
		Timelapse::MainForm::TheInstance->lbMobSearchLog->Items->Add(mobName + " (" + mobLogged.ToString() + ")");
}

inline bool __stdcall shouldMobBeFiltered() {
	if (isMobFilterWhiteList) {
		for (ULONG mob : *mobList)
			if (mob == mobLogged) return false;
		return true;
	}
	for (ULONG mob : *mobList)
		if (mob == mobLogged) return true;
	return false;
}

#pragma unmanaged

	CodeCave(AnimDelay) {
		add eax, [animDelay]
		jmp dword ptr[attackDelayAddrRet]
	} EndCodeCave

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

	CodeCave(StatHook) {
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
		jmp dword ptr[statHookAddrRet]
	} EndCodeCave

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

	CodeCave(MapNameHook) {
		mov [mapNameAddr], ecx
		mov [ebp - 0x28], edi
		lea ecx, [ebp - 0x14]
		jmp dword ptr[mapNameHookAddrRet]
	} EndCodeCave

	CodeCave(ItemVacHook) {
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
		jmp dword ptr[itemVacAddrRet]
	} EndCodeCave

	CodeCave(MouseFlyXHook) {
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
		jmp dword ptr[mouseFlyXAddrRet]
	} EndCodeCave

	CodeCave(MouseFlyYHook) {
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
		jmp dword ptr[mouseFlyYAddrRet]
	} EndCodeCave

	CodeCave(MobFreezeHook) {
		mov [esi + 0x00000248], 0x06
		mov eax, [esi + 0x00000248]
		jmp dword ptr[mobFreezeAddrRet]
	} EndCodeCave

	CodeCave(MobAutoAggroHook) {
		call dword ptr[cVecCtrlWorkUpdateActiveCall] //calls CVecCtrl::WorkUpdateActive()
		push eax
		mov edx, [UserLocalBase]
		mov edx, [edx]
		mov eax, [OFS_pID]
		mov edx, [edx + eax]
		mov edx, [edx + 0x8]
		mov eax, [OFS_Aggro]
		mov [esi + eax], edx //Aggro Offset (first cmp before CVecCtrl::ChaseTarget)
		pop eax
		jmp dword ptr[mobAutoAggroAddrRet]
	} EndCodeCave

	CodeCave(SpawnPointHook) {
		push ecx
		push edx
		cmp dword ptr[esp + 0x8], 0x009BBD5D
		je Return //If Spawning Mob, skip
		cmp dword ptr[esp + 0x8], 0x009C1D90
		je Return //If Spawning NPC, skip
		call getSpawnControlStruct //returns SpawnControlStruct* with the mapID == current mapID
		cmp eax, 0
		je Return //If end of SpawnControl and no match, skip
		mov edx, [eax + 4]
		mov dword ptr[esp + 0x10], edx //x
		mov edx, [eax + 8]
		mov dword ptr[esp + 0x14], edx //y

		Return :
		pop edx
		pop ecx
		mov eax, 0x00AE56F4 //original code
		jmp dword ptr[spawnPointAddrRet]
	} EndCodeCave

	CodeCave(ItemHook) {
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
	} EndCodeCave

	CodeCave(ItemFilterHook) {
		push ebx
		cmp byte ptr[isItemLoggingEnabled], 0
		je Continue //Skip Logging Items
		push eax
		mov [itemLogged], eax
		call itemLog
		pop eax

		Continue :
		cmp byte ptr[isItemFilterEnabled], 0
		je EndItemFilter //Skip if Item Filter is disabled
		mov ebx, [itemFilterMesos]
		cmp eax, ebx //Assumes item is mesos because there the mesos drop limit is 50,000 whereas the smallest item id is greater than that
		jle RemoveMesos //Remove mesos if item id is less than or equal to the user set limit. 
		push eax
		mov [itemLogged], eax
		call shouldItemBeFiltered
		cmp eax, 0 //Item shouldn't be filtered
		pop eax
		je EndItemFilter
		mov eax, 0x00 //Remove Item
		jmp EndItemFilter

		RemoveMesos :
		mov [edi + 0x30], 0x00 //Remove Mesos

		EndItemFilter :
		pop ebx
		mov [edi + 0x34], eax
		mov edi, [ebp - 0x14]
		jmp [itemFilterAddrRet]
	} EndCodeCave


	CodeCave(MobFilter1Hook) {
		push ebx
		call [cInPacketDecode4Addr] //CInPacket::Decode4()
		cmp byte ptr[isMobFilterEnabled], 0
		je EndMobFilter //Skip if Mob Filter is disabled
		push eax
		mov [mobLogged], eax
		call shouldMobBeFiltered
		cmp eax, 0 //Mob shouldn't be filtered
		pop eax
		je EndMobFilter
		pop ebx
		jmp [mobFilter1JmpAddr]

		EndMobFilter:
		pop ebx
		jmp [mobFilter1AddrRet]
	} EndCodeCave

	CodeCave(MobFilter2Hook) {
		push ebx
		call [cInPacketDecode4Addr] //CInPacket::Decode4()
		cmp byte ptr[isMobLoggingEnabled], 0
		je Continue //Skip Logging Mobs
		push eax
		mov [mobLogged], eax
		call mobLog
		pop eax

		Continue :
		cmp byte ptr[isMobFilterEnabled], 0
		je EndMobFilter //Skip if Mob Filter is disabled
		push eax
		mov [mobLogged], eax
		call shouldMobBeFiltered
		cmp eax, 0 //Mob shouldn't be filtered
		pop eax
		je EndMobFilter
		pop ebx
		jmp [mobFilter2JmpAddr]

		EndMobFilter:
		pop ebx
		jmp [mobFilter2AddrRet]
	} EndCodeCave
}

#pragma region testing
/*
//Start of testing stuff
ULONG getStringValHookAddr = 0x0079E9A3;
ULONG getStringRetValHookAddr = 0x0079EA58;
char* maplestring;
__declspec(naked) static void __stdcall GetStringHook() {
	__asm {
		mov[ebp + 0x0C], 0x2
		push ecx
		and dword ptr[ebp - 0x10], 0x00
		jmp[getStringValHookAddr]
	}
} //Non working

__declspec(naked) static void __stdcall GetStringRetValHook() {
	__asm {
		push ebx
		mov ebx, [eax]
		mov[maplestring], ebx
		pop ebx
		mov eax, edi
		pop edi
		pop esi
		pop ebx
		jmp[getStringRetValHookAddr]
	}
} //Working

typedef char**(__stdcall* pfnStringPool__GetString)(PVOID, PVOID, char**, UINT, UINT);	//typedef ZXString<char>*(__stdcall* StringPool__GetString_t)(PVOID, PVOID, ZXString<char>*, UINT);
auto StringPool__GetString = (pfnStringPool__GetString)0x0079E993;	//0x00406455;

typedef char**(__stdcall *pfnCItemInfo__GetMapString)(PVOID, PVOID, char*, UINT, const char*);
auto CItemInfo__GetMapString = (pfnCItemInfo__GetMapString)0x005CF792;

//Test stuff out //https://pastebin.com/aULY72tG
void Timelapse::MainForm::button1_Click(System::Object^  sender, System::EventArgs^  e) {


	/*char result[300];
	char* str = *CItemInfo__GetMapString(*(PVOID*)CItemInfo, NULL, result, 100000000, 0);
	String^ test = Convert::ToString(str);
	if (String::IsNullOrEmpty(test))
		MessageBox::Show("Error! Empty string was returned");
	else
		MessageBox::Show(test); */


		/*//Displays 0th string, but crashes shortly after. What I wanted was the 2nd maplestring
		char** result;
		char* str = *StringPool__GetString(*(PVOID*)StringPool, nullptr, (char**)result, 2, 0);
		String^ test = gcnew String(str);

		if (String::IsNullOrEmpty(test))
			MessageBox::Show("Error! Empty string was returned");
		else
			MessageBox::Show(test);*/


			//char result[256];
			//Jump(0x0079E99E, GetStringHook, 0);
			/*Jump(0x0079EA53, GetStringRetValHook, 0);
			String^ test = gcnew String(maplestring);
			if (String::IsNullOrEmpty(test))
				MessageBox::Show("Error! Empty string was returned");
			else
				MessageBox::Show(test);
}

//CMobPool UML Diagram
//V95 00C687B8+28]+4]+10C]+28]+60 = mob x || v83 00BEBFA4+28]4]120]24]60 = mob x
//&v3->m_pvcHead.m_pInterface->vfptr;
//vfptr[8].addRef
//What I know: in v95, 00C687B8 is a CMobPool pointer, +28 = ZList<ZRef<CMob*>>, + 4 = ZList<ZRef<CMob*>>.pHead (CMob) + 10C = m_pvcHead
//m_pvcHead.m_pInterface->vpptr is type IUnknown* (Microsoft's Component object model) 
//that points to some class (maybe CVecCtrlMob) but the offsets 28 and 60 doesn't seem point to an offset in CVecCtrl or CVecCtrl mob that is the xy coordinates of a mob
*/
#pragma endregion