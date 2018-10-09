#ifndef POINTERS_H
#define POINTERS_H

#include "Packet.h"

#pragma region HookAddresses
ULONG levelHookAddr = 0x004E2B28;
ULONG levelHookDecode = 0x004E807A;
ULONG levelHookAddrRet = levelHookAddr + 7;
ULONG jobHookAddr = 0x004E2B3E;
ULONG jobHookDecode = 0x004E80EB;
ULONG jobHookAddrRet = jobHookAddr + 7;
ULONG statHookAddr = 0x008D8581;
ULONG statHookAddrRet = statHookAddr + 5;
ULONG mesosHookAddr = 0x004E2D01;
ULONG mesosHookAddrRet = mesosHookAddr + 6;
ULONG mesosChangeHookAddr = 0x004E31FB;
ULONG mesosChangeHookAddrRet = mesosChangeHookAddr + 6;
ULONG mapNameHookAddr = 0x005CFA48;
ULONG mapNameHookAddrRet = mapNameHookAddr + 6;
ULONG itemVacAddr = 0x005047AA;
ULONG itemVacAddrRet = itemVacAddr + 7;
ULONG mouseFlyXAddr = 0x009B62ED;
ULONG mouseFlyXAddrRet = mouseFlyXAddr + 5;
ULONG mouseFlyYAddr = 0x009B6352;
ULONG mouseFlyYAddrRet = mouseFlyYAddr + 5;
ULONG mobFreezeAddr = 0x009BCA92;
ULONG mobFreezeAddrRet = mobFreezeAddr + 6;
ULONG mobAutoAggroAddr = 0x009BCAF7;
ULONG cVecCtrlWorkUpdateActiveCall = 0x009B19D0;
ULONG mobAutoAggroAddrRet = mobAutoAggroAddr + 5;
ULONG spawnPointAddr = 0x009B12A8;
ULONG spawnPointAddrRet = spawnPointAddr + 5;
#pragma endregion

#pragma region FunctionPointers
typedef int(__thiscall *lpfnCWvsContext__SendMigrateToShopRequest)(ULONG, int); // Enters Cash Shop
lpfnCWvsContext__SendMigrateToShopRequest CWvsContext__SendMigrateToShopRequest = (lpfnCWvsContext__SendMigrateToShopRequest)0x00A04DCA;

typedef int(__stdcall *lpfnCCashShop__SendTransferFieldPacket)(); // Exits Cash Shop
lpfnCCashShop__SendTransferFieldPacket CCashShop__SendTransferFieldPacket = (lpfnCCashShop__SendTransferFieldPacket)0x0047C108;

typedef int(__stdcall *lpfnCField__SendTransferChannelRequest)(int nTargetChannel); // Changes Channel
lpfnCField__SendTransferChannelRequest CField__SendTransferChannelRequest = (lpfnCField__SendTransferChannelRequest)0x5304AF;

typedef int(__stdcall *lpfnCWvsContext__GetCharacterLevel)(ULONG, LPVOID); // Retrieves Character Level
lpfnCWvsContext__GetCharacterLevel CWvsContext__GetCharacterLevel = (lpfnCWvsContext__GetCharacterLevel)0x004A8DE0; // TODO: Fix address

typedef char*(__cdecl *get_job_name)(int); // Retrieves Job name
get_job_name GetJobName = (get_job_name)0x004A77EF;

//typedef ZXString<char>*(__thiscall *lpfnCItemInfo__GetMapString)(ULONG, ZXString<char>*, UINT, const char*);
typedef char*(__thiscall *lpfnCItemInfo__GetMapString)(ULONG, char*);
lpfnCItemInfo__GetMapString CItemInfo__GetMapString = (lpfnCItemInfo__GetMapString)0x005CF792; // TODO: Fix func

//ZRef<CharacterData> *__thiscall CWvsContext::GetCharacterData(CWvsContext *this, ZRef<CharacterData> *result) //Could I use pointer?? See ignbase
typedef void*(__thiscall *lpfnCWvsContext__GetCharacterData)(ULONG, PVOID);
lpfnCWvsContext__GetCharacterData CWvsContext__GetCharacterData = (lpfnCWvsContext__GetCharacterData)0x00425D0B;


#pragma endregion

#pragma region Pointers
ULONG PtInRectAddr = 0xBF0484;

ULONG GUIInfoBase = 0xBEC208; 
ULONG OFS_HP = 0xD18;
ULONG OFS_MP = OFS_HP + 4;
ULONG OFS_EXP = 0xBC8;

ULONG SettingsBase = 0xBEBF9C; // CUIStatusBar
ULONG OFS_HPAlert = 0x80;
ULONG OFS_MPAlert = OFS_HPAlert + 4;

ULONG InfoBase = 0xBED788; // CUIMiniMap
ULONG OFS_MapID = 0x668;

ULONG IgnBase = 0xBF3CD8; //GW_CharacterStat //CWvsContext::GetCharacterData() returns ZRef<CharacterData>; &CharacterData[0] = GW_CharacterStat
ULONG OFS_Ign = 0x4;

ULONG NPCBase = 0xBED780;
ULONG OFS_NPCCount = 0x24;

ULONG PortalBase = 0xBED768; // CPortalList 
ULONG OFS_PortalCount = 0x18;

ULONG ServerBase = 0xBE7918; // CWvsContext 
ULONG OFS_World = 0x2054;
ULONG OFS_Channel = 0x2058;
ULONG OFS_Tubi = 0x20A4;

ULONG CharBase = 0xBEBF98; // CUserLocal 
ULONG OFS_pID = 0x11A4;
ULONG OFS_Foothold = 0x1F0;
ULONG OFS_KB = 0x214;
ULONG OFS_KBX = 0x220;
ULONG OFS_KBY = 0x228;
ULONG OFS_Aggro = 0x250;
ULONG OFS_CharX = 0x3124;
ULONG OFS_CharY = OFS_CharX + 4;
ULONG OFS_AttackCount = 0x2B88;
ULONG OFS_Breath = 0x56C;
ULONG OFS_Morph = 528; //Change pointer to 9 and freeze
ULONG OFS_CharAnimation = 570;
ULONG OFS_Tele = 0x2B18;
ULONG OFS_TeleX = OFS_Tele + 8;
ULONG OFS_TeleY = OFS_TeleX + 4;
ULONG OFS_BuffCount = 0xBF4AD4; // Couldn't find it within CUserLocal, so static address


ULONG ItemBase = 0xBED6AC; // CDropPool 
ULONG OFS_ItemCount = 0x14;

ULONG MapBase = 0xBEBFA0; // CWvsPhysicalSpace2D 
ULONG OFS_LeftWall = 0x24;
ULONG OFS_RightWall = 0x2C;
ULONG OFS_UpWall = 0x28;
ULONG OFS_DownWall = 0x30;

ULONG PeopleBase = 0xBEBFA8; // CUserPool
ULONG OFS_PeopleCount = 0x18;

ULONG MouseBase = 0xBEC33C; // CInputSystem
ULONG OFS_MouseAnimation = 0x9B4;
ULONG OFS_MouseLocation = 0x978;
ULONG OFS_MouseX = 0x8C;
ULONG OFS_MouseY = OFS_MouseX + 4;

ULONG MobBase = 0xBEBFA4; // CMobPool
ULONG OFS_Mob1 = 0x28;
ULONG OFS_Mob2 = 0x4;
ULONG OFS_Mob3 = 0x120;
ULONG OFS_Mob4 = 0x24;
ULONG OFS_MobX = 0x60;
ULONG OFS_MobY = OFS_MobX + 4;
ULONG OFS_DeadMob = 0x7C; //Wrong
ULONG OFS_MobCount = 0x24;

ULONG CItemInfo = 0xBE78D8;
#pragma endregion

#endif