#pragma once
#include "Structs.h"

/*  
 *  Addresses defined here were found using the following programs: Cheat Engine, Hex-Rays IDA, ReClassEx
 *  Default Image Base Address for a 32-bit exe in Windows, when loaded into Memory is 0x400000
 *  So all the addresses defined here already have 0x400000 added for convenience purposes 
 *  These addresses are all virtual addresses defined in MapleStory.exe's virtual address space
 */

#pragma region Hack Addresses
//Main Tab
ULONG logoSkipAddr = 0x0062F2D6;

//Hacks Tabs
ULONG fullGodmodeAddr = 0x009581D5;
ULONG missGodmodeAddr = 0x009582E9;
ULONG blinkGodmodeAddr = 0x00932501;
ULONG swimInAirAddr = 0x00704704;
ULONG unlimitedAttackAddr = 0x009536E0; 
ULONG fullAccuracyAddr = 0x00AFE7F8; //Client sided only version?? This is incomplete acc hack.
ULONG noBreathAddr = 0x00452316; //Fake/imperfect no breath?
ULONG noPlayerKnocbackAddr = 0x007A637F;
ULONG noPlayerDeathAddr = 0x009506C6;
ULONG jumpDownAnywhereAddr = 0x0094C6EE;
ULONG noSkillEffectAddr = 0x00933990;
ULONG noAttackDelayAddr = 0x0092EDB2;
ULONG noPlayerNameTagAddr = 0x00942DCC;
ULONG attackDelayAddr = 0x00454795;
ULONG instantDropItemsAddr = 0x00AF0E1C;
ULONG instantLootItemsAddr = 0x004417E3;
ULONG tubiAddr = 0x00485C01;
ULONG noMobReactionAddr = 0x0066B05E;
ULONG noMobDeathEffectAddr = 0x00663995;
ULONG noMobKnockbackAddr = 0x00668C9E;
ULONG mobDisarmAddr = 0x00667A00;
ULONG noMapBackgroundAddr = 0x00639CB6;
ULONG noMapObjectsAddr = 0x00639CAF;
ULONG noMapTitlesAddr = 0x00639CA8;
ULONG noMapFadeEffect = 0x00776E65;
ULONG mapSpeedUpAddr = 0x009B21A0;
ULONG removeSpamFilterAddr1 = 0x00490607; 
ULONG removeSpamFilterAddr2 = 0x00490651;
ULONG infiniteChatboxAddr1 = 0x004CAA09;
ULONG infiniteChatboxAddr2 = 0x004CAA84;
ULONG noBlueBoxesAddr = 0x009929DD;
ULONG walkingFrictionAddr = 0x009B4365; // je -> jne

//Vac Tabs
ULONG fullMapAttackAddr = 0x006785CA;
ULONG zzVacAddr1 = 0x009B17A0; 
ULONG zzVacAddr2 = 0x009B17B0;
ULONG vacForceRightAddr = 0x009B2C1E; // jae -> jna	
ULONG vacRightNoForce = 0x009B2C32; // jae -> jna
ULONG vacJumpLeftAddr = 0x009B4632; // jae -> je
ULONG vacJumpRightAddr = 0x009B46A1; // jna -> je		
ULONG vacJumpUpAddr = 0x009B4732; // jae -> je	
ULONG vacLeftAddr = 0x009B4819;  // jae -> je	
ULONG vacRightAddr= 0x009B4896; // jbe -> je	
ULONG fangorVacAddr1 = 0x009B2B98; // fld(0) -> fld(1) //vac into left wall		
ULONG fangorVacAddr2 = 0x009B43BE; // fld(0) -> fld(1) //vac into top left corner		
ULONG pVacAddr = 0x009B1E43; // codecave

//Unimplemented Hacks
ULONG multiClientAddr = 0x00949BC7; // jne -> jmp
ULONG lagHackAddr = 0x009B16F2; // je -> jne
ULONG accuracyHackAddr1 = 0x00424D22; // codecave
ULONG accuracyHackAddr2 = 0x00AFE7F8; // codecave
ULONG accuracyHackAddr3 = 0x005E2AAA; // codecave
ULONG mpRegenTickTimeAddr = 0x00A031F5; // cmp ebx,00002710 -> cmp ebx, [BYTE_VALUE]
ULONG sitAnywhereAddr = 0x009506E9; // je -> 2x nop
ULONG speedWalkAddr = 0x009B268D; // je -> 6x nop
ULONG mouseCSEAXVacYAddr = 0x009B6352; // codecave
ULONG mouseCSEAXVacXAddr = 0x009B62ED; // codecave
ULONG mesoDropCap = 0x0081DAC7; // 4 bytes
ULONG magicAttCap = 0x00780621; // 4 bytes
ULONG gravity = 0x00AF0DE0; // double
ULONG bringYourOwnRopeAddr = 0x00A45B03; // codecave
ULONG MSCRCBypassAddr1 = 0x004A27E7; // codecave
ULONG MSCRCBypassAddr2 = 0x004A27EC; // codecave
ULONG slideRightAddr = 0x009B2C0A; // jna -> jne
ULONG itsRainingMobsAddr = 0x009B1E8E; // F1 -> F2, bugged diassembly??
ULONG attackUnrandommizerAddr = 0x0076609C; // codecave
ULONG etcExplosionAddr = 0x00505806; // 6x nop
ULONG useRechargableItemsAfterDepletionAddr1 = 0x009516BA; // 6x nop
ULONG useRechargableItemsAfterDepletionAddr2 = 0x009516C2; // je -> jns
ULONG chargeSkillsNoChargingAddr1 = 0x009B2154; //jne -> jmp		
// ULONG chargeSkillsNoChargingAddr2 = ??????????		
ULONG noJumpingMobAndPlayerAddr = 0x009B44D4; //je -> jmp		
ULONG vacLeftAddr2 = 0x009B2441; //??		
ULONG jmpRelatedstub = 0x009B2BF7; //??		
ULONG gravityrelated = 0x009B2BF5; //jae -> jmp for monster fly up on jump
ULONG mapGlideAddr = 0x009B2BE8; // jna 009B2C11 jna->jae monsters/players glide like in Elnath

// OLDSKOOL STUFF
// pin typer v0.62 004A0A6B: //83 FA 6F 0F 86 ?? ?? ?? ?? 83 FA 7B 
// jbe -> je
// take one dmg v0.62 00670090: // DF E0 9E 72 04 DD D8 D9 E8 DC 15
// jb -> ja
// ZPVac v0.62 007F2A18: // C3 DD D8 5D C3 55 8B EC
// fstp st(0) -> fstp st(6)
// Unlimited summon v0.62 004D6D95: //74 2D 8B 7C 24 0C 8B 07 6A 05 50 E8 ?? ?? ?? ??
// je -> jne
#pragma endregion

#pragma region CodeCave Addresses
ULONG statHookAddr = 0x008D8581; //Inside CUIStatusBar::SetNumberValue
ULONG statHookAddrRet = statHookAddr + 5;
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
ULONG cOutPacketAddr = 0x0049637B;
ULONG cOutPacketAddrRet = cOutPacketAddr + 5;
ULONG dupeXAddr = 0x009B495D;
ULONG dupeXAddrRet = dupeXAddr + 6;
#pragma endregion

#pragma region MapleStory Function Hook Addresses
//Hook Addresses
ULONG enterCSAddr = 0x00A04DCA;
ULONG exitCSAddr = 0x0047C108;
ULONG ccAddr = 0x005304AF;
ULONG jobNameAddr = 0x004A77EF;
ULONG charDataAddr = 0x00425D0B;
ULONG userlocalUpdateAddr = 0x0094A144;
ULONG cloginOnRecommendWorldAddr = 0x005F8340;
ULONG cloginSendLoginPacketAddr = 0x005F6D6A;

//Hooks
typedef void(__stdcall *pfnCWvsContext__SendMigrateToShopRequest)(PVOID, PVOID, int); // Enters Cash Shop
auto CWvsContext__SendMigrateToShopRequest = reinterpret_cast<pfnCWvsContext__SendMigrateToShopRequest>(enterCSAddr);

typedef void(__stdcall *pfnCCashShop__SendTransferFieldPacket)(); // Exits Cash Shop
auto CCashShop__SendTransferFieldPacket = reinterpret_cast<pfnCCashShop__SendTransferFieldPacket>(exitCSAddr);

typedef char*(__cdecl *pfnGet_Job_Name)(int); // Retrieves Job name
auto GetJobName = reinterpret_cast<pfnGet_Job_Name>(jobNameAddr);

typedef void*(__thiscall *pfnCWvsContext__GetCharacterData)(ULONG, PVOID);
auto CWvsContext__GetCharacterData = reinterpret_cast<pfnCWvsContext__GetCharacterData>(charDataAddr);

//typedef ZXString<char>*(__fastcall* StringPool__GetString_t)(void *StringPool, void *edx, ZXString<char> *result, unsigned int nIdx);
//auto StringPool__GetString = (StringPool__GetString_t)0x0049B330; //
//void **ms_pInstance_StringPool = (void **)0x01C1C200; pointer that gets all Strings in StringPool
#pragma endregion

#pragma region Pointer Addresses & Offsets
ULONG PtInRectAddr = 0xBF0484;

ULONG LoginBase = 0xBEDED4; //CLogin
ULONG OFS_LoginStep = 0x168; //0 = login screen or logged in, 1 = Select World/Channel, 2 = Select Char
ULONG OFS_LoginScreen = 0x174; //255 == login screen, 1 == loggin in, 0 = logged in

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
ULONG OFS_Morph = 0x528; //Change pointer to 9 and freeze
ULONG OFS_CharAnimation = 0x570;
ULONG OFS_Tele = 0x2B18;
ULONG OFS_TeleX = OFS_Tele + 8;
ULONG OFS_TeleY = OFS_TeleX + 4;
ULONG OFS_BuffCount = 0xBF4AD4; // Couldn't find it within CUserLocal, so static address
ULONG OFS_ComboCount = 0x3220;

ULONG DropPoolBase = 0xBED6AC; // CDropPool 
ULONG OFS_ItemCount = 0x14;

ULONG CWvsPhysicalSpace2DBase = 0xBEBFA0; // CWvsPhysicalSpace2D 
ULONG OFS_LeftWall = 0x24;
ULONG OFS_RightWall = 0x2C;
ULONG OFS_TopWall = 0x28;
ULONG OFS_BottomWall = 0x30;

ULONG UserPoolBase = 0xBEBFA8; // CUserPool
ULONG OFS_PeopleCount = 0x18;

ULONG InputBase = 0xBEC33C; // CInputSystem
ULONG OFS_MouseAnimation = 0x9B4;
ULONG OFS_MouseLocation = 0x978;
ULONG OFS_MouseX = 0x8C;
ULONG OFS_MouseY = OFS_MouseX + 4;

//TODO: Get valid names according to struct
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