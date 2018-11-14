#ifndef ADDRESSES_H
#define ADDRESSES_H

#include "Packet.h"

/*  
 *  Addresses defined here were found using the following programs: Cheat Engine, Hex-Rays IDA, ReClassEx
 *  Default Image Base Address for a 32-bit exe in Windows, when loaded into Memory is 0x400000
 *  So all the addresses defined here already have 0x400000 added for convenience purposes 
 *  These addresses are all virtual addresses defined in MapleStory.exe's virtual address space
 */

#pragma region CodeCave Addresses
/*ULONG levelHookAddr = 0x004E2B28; //Inside GW_CharacterStat::Decode()
ULONG levelHookDecode = 0x004E807A; //Start of _ZtrlSecureTear<unsigned char>
ULONG levelHookAddrRet = levelHookAddr + 7;
ULONG jobHookAddr = 0x004E2B3E; //Inside GW_CharacterStat::Decode()
ULONG jobHookDecode = 0x004E80EB; //Inside _ZtlSecureTear<short>
ULONG jobHookAddrRet = jobHookAddr + 7;*/
ULONG statHookAddr = 0x008D8581; //Inside CUIStatusBar::SetNumberValue
ULONG statHookAddrRet = statHookAddr + 5;
/*ULONG mesosHookAddr = 0x004E2D01; //Inside GW_CharacterStat::DecodeMoney()
ULONG mesosHookAddrRet = mesosHookAddr + 6;
ULONG mesosChangeHookAddr = 0x004E31FB; //Inside GW_CharacterStat::DecodeChangeStat()
ULONG mesosChangeHookAddrRet = mesosChangeHookAddr + 6;*/
ULONG mapNameHookAddr = 0x005CFA48; //Inside CItemInfo::GetMapString()
ULONG mapNameHookAddrRet = mapNameHookAddr + 6;
ULONG itemVacAddr = 0x005047AA; //Inside CDropPool::TryPickUpDrop()
ULONG itemVacAddrRet = itemVacAddr + 7;
ULONG mouseFlyXAddr = 0x009B62ED; //Inside CVecCtrl::raw__GetSnapshot()
ULONG mouseFlyXAddrRet = mouseFlyXAddr + 5;
ULONG mouseFlyYAddr = 0x009B6352; //Inside CVecCtrl::raw__GetSnapshot()
ULONG mouseFlyYAddrRet = mouseFlyYAddr + 5;
ULONG mobFreezeAddr = 0x009BCA92; //Inside CVecCtrlMob::WorkUpdateActive()
ULONG mobFreezeAddrRet = mobFreezeAddr + 6;
ULONG mobAutoAggroAddr = 0x009BCAF7; //Inside CVecCtrlMob::WorkUpdateActive() (call to CVecCtrl::WorkUpdateActive())
ULONG cVecCtrlWorkUpdateActiveCall = 0x009B19D0; //Start of CVecCtrl::WorkUpdateActive()
ULONG mobAutoAggroAddrRet = mobAutoAggroAddr + 5;
ULONG spawnPointAddr = 0x009B12A8; //Start of CVecCtrl::SetActive()
ULONG spawnPointAddrRet = spawnPointAddr + 5;
ULONG itemFilterAddr = 0x005059CC; //Inside CDropPool::OnDropEnterField()
ULONG itemFilterAddrRet = itemFilterAddr + 6;
ULONG mobFilter1Addr = 0x0067832D; //Inside CMobPool::SetLocalMob()
ULONG mobFilter1AddrRet = mobFilter1Addr + 5;
ULONG mobFilter1JmpAddr = 0x006783E2; //mov ecx,[ebp-0C] above the ret 0004 at the end of function
ULONG mobFilter2Addr = 0x0067948C; //Inside CMobPool::OnMobEnterField()
ULONG mobFilter2AddrRet = mobFilter2Addr + 5;
ULONG mobFilter2JmpAddr = 0x0067957F; //mov ecx,[ebp-0C] above the ret 0004 at the end of function
ULONG cInPacketDecode4Addr = 0x00406629; //Start of CInPacket::Decode4()
#pragma endregion

#pragma region MapleStory Function Addresses
typedef void(__stdcall *pfnCWvsContext__SendMigrateToShopRequest)(PVOID, PVOID, int); // Enters Cash Shop
auto CWvsContext__SendMigrateToShopRequest = (pfnCWvsContext__SendMigrateToShopRequest)0x00A04DCA;

typedef void(__stdcall *pfnCCashShop__SendTransferFieldPacket)(); // Exits Cash Shop
auto CCashShop__SendTransferFieldPacket = (pfnCCashShop__SendTransferFieldPacket)0x0047C108;

typedef int(__stdcall *pfnCField__SendTransferChannelRequest)(int nTargetChannel); // Changes Channel
auto CField__SendTransferChannelRequest = (pfnCField__SendTransferChannelRequest)0x5304AF;

typedef int(__stdcall *pfnCWvsContext__GetCharacterLevel)(ULONG, PVOID); // Retrieves Character Level
auto CWvsContext__GetCharacterLevel = (pfnCWvsContext__GetCharacterLevel)0x004A8DE0; // TODO: Fix address

typedef char*(__cdecl *pfnGet_Job_Name)(int); // Retrieves Job name
auto GetJobName = (pfnGet_Job_Name)0x004A77EF;

typedef void*(__thiscall *pfnCWvsContext__GetCharacterData)(ULONG, PVOID);
auto CWvsContext__GetCharacterData = (pfnCWvsContext__GetCharacterData)0x00425D0B;

//typedef ZXString<char>*(__fastcall* StringPool__GetString_t)(void *StringPool, void *edx, ZXString<char> *result, unsigned int nIdx);
//auto StringPool__GetString = (StringPool__GetString_t)0x0049B330; //
//void **ms_pInstance_StringPool = (void **)0x01C1C200; pointer that gets all Strings in StringPool

#pragma endregion

#pragma region Pointers Addresses & Offsets
ULONG PtInRectAddr = 0xBF0484;

ULONG UIInfoBase = 0xBEC208; 
ULONG OFS_HP = 0xD18;
ULONG OFS_MP = OFS_HP + 4;
ULONG OFS_EXP = 0xBC8;

ULONG UIStatusBarBase = 0xBEBF9C; // CUIStatusBar
ULONG OFS_HPAlert = 0x80;
ULONG OFS_MPAlert = OFS_HPAlert + 4;

ULONG UIMiniMapBase = 0xBED788; // CUIMiniMap
ULONG OFS_MapID = 0x668;

ULONG CharacterStatBase = 0xBF3CD8; //GW_CharacterStat //CWvsContext::GetCharacterData() returns ZRef<CharacterData>; &CharacterData[0] = GW_CharacterStat
ULONG OFS_Ign = 0x4;
ULONG OFS_Level = 0x33;
ULONG OFS_JobID = 0x39;
ULONG OFS_Mesos = 0xA5;

ULONG NPCPoolBase = 0xBED780; //CNPCPool
ULONG OFS_NPCCount = 0x24;

ULONG PortalListBase = 0xBED768; // CPortalList 
ULONG OFS_PortalCount = 0x18;

ULONG ServerBase = 0xBE7918; // CWvsContext 
ULONG OFS_World = 0x2054;
ULONG OFS_Channel = 0x2058;
ULONG OFS_Tubi = 0x20A4;
ULONG OFS_CharacterCount = 0x20A0;
ULONG OFS_ZRef_CharacterData = 0x20B8; //CharacterStatBase*

ULONG UserLocalBase = 0xBEBF98; // CUserLocal 
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


ULONG DropPoolBase = 0xBED6AC; // CDropPool 
ULONG OFS_ItemCount = 0x14;

ULONG CWvsPhysicalSpace2DBase = 0xBEBFA0; // CWvsPhysicalSpace2D 
ULONG OFS_LeftWall = 0x24;
ULONG OFS_RightWall = 0x2C;
ULONG OFS_UpWall = 0x28;
ULONG OFS_DownWall = 0x30;

ULONG UserPoolBase = 0xBEBFA8; // CUserPool
ULONG OFS_PeopleCount = 0x18;

ULONG InputBase = 0xBEC33C; // CInputSystem
ULONG OFS_MouseAnimation = 0x9B4;
ULONG OFS_MouseLocation = 0x978;
ULONG OFS_MouseX = 0x8C;
ULONG OFS_MouseY = OFS_MouseX + 4;

ULONG MobPoolBase = 0xBEBFA4; // CMobPool
ULONG OFS_Mob1 = 0x28;
ULONG OFS_Mob2 = 0x4;
ULONG OFS_Mob3 = 0x120;
ULONG OFS_Mob4 = 0x24;
ULONG OFS_MobX = 0x60;
ULONG OFS_MobY = OFS_MobX + 4;
ULONG OFS_DeadMob = 0x7C; //Wrong
ULONG OFS_MobCount = 0x24;

ULONG CItemInfo = 0xBE78D8;
ULONG StringPool = 0xBF0D0C;
#pragma endregion

#endif