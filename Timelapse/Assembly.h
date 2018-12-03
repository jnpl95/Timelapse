#pragma once
#include "Structs.h"
#include "resource.h"
#include "GeneralFunctions.h"
#include "Packet.h"

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
SendPacketData *sendPacketData;
ULONG dupeXFoothold = 0;

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

inline void __stdcall addSendPacket() {
	COutPacket* packet = sendPacketData->packet;

	String^ header = "";
	writeUnsignedShort(header, *packet->Header);

	String^ packetData = "";
	BYTE* packetBytes = packet->Data;
	int packetSize = packet->Size - 2;
	for (int i = 0; i < packetSize; i++)
		writeByte(packetData, packetBytes[i]);

	Log::WriteLineToConsole("  " + header + " " + packetData);
	//TODO: Add packet to queue, and flush queue to GUI after a certain interval (use timer) 

	/*Windows::Forms::TreeNode^ packetNode = gcnew Windows::Forms::TreeNode(packetData);
	packetNode->Name = packetData;*/

	//Windows::Forms::TreeNode^ headerNode = gcnew Windows::Forms::TreeNode(header);
	//headerNode->Name = header;
	//Timelapse::MainForm::TheInstance->tvSendPackets->Nodes->Add(headerNode);

	/*if(Timelapse::MainForm::TheInstance->tvSendPackets->Nodes->ContainsKey(header)) {
		//Header exists in tree
	}
	else {
		Windows::Forms::TreeNode^ headerNode = gcnew Windows::Forms::TreeNode(header);
		headerNode->Name = header;
		//headerNode->Nodes->Add(packetNode);
		Timelapse::MainForm::TheInstance->tvSendPackets->BeginUpdate();
		Timelapse::MainForm::TheInstance->tvSendPackets->Nodes->Add(headerNode); 
		Timelapse::MainForm::TheInstance->tvSendPackets->EndUpdate();
	}*/
}

#pragma unmanaged

	/*CodeCave(AnimDelay) {
		add eax, [animDelay]
		jmp dword ptr[animDelayAddrRet]
	} EndCodeCave*/

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

	CodeCave(SendPacketLogHook) {
		mov dword ptr [sendPacketData], esp
		pushad
		call [addSendPacket]
		popad
		jmp [cOutPacketAddrRet]
	} EndCodeCave

	CodeCave(DupeXHook) {
		push eax
		push ecx
		mov eax, [UserLocalBase]
		mov eax, [eax]
		mov ecx, [OFS_pID]
		mov eax, [eax + ecx]
		lea ecx, [eax - 0x0c] //account id offset?
		mov eax, [ecx + 0x00000114] //kb offset?
		mov dword ptr[dupeXFoothold], eax
		mov edi, [dupeXFoothold]
		pop ecx
		pop eax
		mov[esi + 0x00000114], edi
		jmp dword ptr[dupeXAddrRet]

		//mov edi, [dupeXFoothold]
		//mov[esi + 0x00000114], edi
		//jmp dword ptr[dupeXAddrRet]
	} EndCodeCave
}