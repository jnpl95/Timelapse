﻿#include <Windows.h>
#include <cliext/vector>
#include <sstream>
#include "MainForm.h"
#include "Addresses.h"
#include "Macro.h"
#include "Packet.h"
#include "Structs.h"
#include "Settings.h"
#include "Log.h"
#include "Hooks.h"
#include "AutoLoginTimer.h"
#include <msclr\marshal_cppstd.h>

[assembly:System::Diagnostics::DebuggableAttribute(true, true)]; //For debugging purposes

using namespace msclr::interop;
using namespace Timelapse;

//Forward declarations
void AutoLogin();
static void loadMaps(); 
static void mapRush(int destMapID); 

//Managed Global Variables
ref struct GlobalRefs {
	static Macro ^macroHP, ^macroMP, ^macroAttack, ^macroLoot, ^autologin;
	static bool isChangingField = false, isMapRushing = false;
	static bool bClickTeleport = false, bMouseTeleport = false, bTeleport = false, bKami = false, bKamiLoot = false;
	static bool bWallVac = false, bDupeX = false, bMMC = false, bUEMI = false;
	static UINT cccsTimerTickCount = 0;
	static bool isDragging = false, isEmbedding = false;
	static Point dragOffset;
	static HWND hParent; 
	static double formOpacity;
	static Generic::List<MapData^>^ maps;
	static bool bSendPacketLog = false, bRecvPacketLog = false;
	//static String^ globalAutoLogin = "01 00 0C 00 54 68 72 61 73 68 65 72 31 32 37 36 0A 00 50 69 7A 7A 61 72 64 31 32 34 00 00 00 00 00 00 EA 61 29 77 00 00 00 00 9B 8D 00 00 00 00 02 00 00 00 00 00 00";
};

#pragma region General Form
[STAThread]
void Main() {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	Application::Run(gcnew MainForm);
	Application::Exit();
}

#pragma unmanaged
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, PVOID lpvReserved) {
	GlobalVars::hDLL = hModule;

	switch (dwReason) {
		case DLL_PROCESS_ATTACH:
			//GlobalVars::hDLL = hModule;
			CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)&Main, nullptr, 0, nullptr);
			break;
		case DLL_PROCESS_DETACH:
			FreeLibraryAndExitThread(hModule, 0);
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		default:
			break;
	}
	//__writefsdword(0x6B0, GetMSThreadID());

	return TRUE;
}

#pragma managed
void MainForm::MainForm_Load(Object^  sender, EventArgs^  e) {
	Log::WriteLineToConsole(":::::::::::::::::::::::::::::::::::::::::");
	Log::WriteLineToConsole(":::         Timelapse Trainer         :::");
	Log::WriteLineToConsole(":::::::::::::::::::::::::::::::::::::::::");
	Log::WriteLineToConsole("Use: Extreme Injector v3.7.2");
	Log::WriteLineToConsole("Initializing Timelapse trainer ....");
	RECT msRect;
	GetWindowRect(GetMSWindowHandle(), &msRect);
	this->Left = msRect.right;
	this->Top = msRect.top;
}

void MainForm::MainForm_Shown(Object^  sender, EventArgs^  e) {
	this->Refresh();

	for (double i = 0.100; i < 0.9;) {
		i += 0.1;
		this->Opacity = i;
		this->Refresh();
	}

	lbTitle->Text = "Timelapse Trainer - PID: " + GetMSProcID();
	Log::WriteLineToConsole("Creating and starting macro Thread ...");
	Threading::Thread^ macroThread = gcnew Threading::Thread(gcnew Threading::ThreadStart(PriorityQueue::MacroQueueWorker));
	macroThread->Start();
	Log::WriteLineToConsole("Loading Maps for mapRusher ......");
	loadMaps();
	Log::WriteLineToConsole("Initialized Timelapse trainer!");

	if (File::Exists(Settings::GetSettingsPath())) {
		if (MessageBox::Show("A save file has been detected, load save?", "Save File", MessageBoxButtons::YesNo) == System::Windows::Forms::DialogResult::Yes) {
			Settings::Deserialize(this, Settings::GetSettingsPath());
			AutoLogin();
		}
	}
}

void MainForm::MainForm_FormClosing(Object^  sender, Windows::Forms::FormClosingEventArgs^  e) {
	//Turn off all loops
	GlobalRefs::isChangingField = false, GlobalRefs::isMapRushing = false;
	GlobalRefs::bClickTeleport = false, GlobalRefs::bMouseTeleport = false, GlobalRefs::bTeleport = false;
	GlobalRefs::bWallVac = false, GlobalRefs::bDupeX = false, GlobalRefs::bMMC = false, GlobalRefs::bUEMI = false;
	GlobalRefs::bKami = false, GlobalRefs::bKamiLoot = false;
	GlobalRefs::isDragging = false;
	PriorityQueue::closeMacroQueue = true;

	for (double i = this->Opacity; i > 0;) {
		i -= 0.1;
		this->Opacity = i;
		this->Refresh();
	}

	Sleep(200);
	Log::WriteLine("Deinitialized Timelapse trainer!");
}

void MainForm::btnClose_Click(Object^  sender, EventArgs^  e) {
	Application::Exit();
}

void MainForm::btnMinimize_Click(Object^  sender, EventArgs^  e) {
	if (WindowState == FormWindowState::Normal) WindowState = FormWindowState::Minimized;
}

void MainForm::pnlFull_MouseDown(Object^  sender, Windows::Forms::MouseEventArgs^  e) {
	if (!GlobalRefs::isDragging) {
		GlobalRefs::isDragging = true;
		GlobalRefs::dragOffset = Point(e->X, e->Y);
		GlobalRefs::formOpacity = Math::Round(this->Opacity, 2);
		for (double i = this->Opacity; i > 0.80; ) {
			i -= 0.1;
			this->Opacity = i;
			this->Refresh();
		}
	}
}

void MainForm::pnlFull_MouseUp(Object^  sender, Windows::Forms::MouseEventArgs^  e) {
	if (GlobalRefs::isDragging) {
		for (double i = this->Opacity; i < GlobalRefs::formOpacity;) {
			i += 0.05;
			this->Opacity = i;
			this->Refresh();
		}
		GlobalRefs::isDragging = false;
	}
}

void MainForm::pnlFull_MouseMove(Object^  sender, Windows::Forms::MouseEventArgs^  e) {
	if (GlobalRefs::isDragging) {
		Point currentScreenPos = PointToScreen(e->Location);
		Location = Point(currentScreenPos.X - GlobalRefs::dragOffset.X, currentScreenPos.Y - GlobalRefs::dragOffset.Y);
	}
}
#pragma endregion

#pragma region ToolStrip
void MainForm::loadSettingsToolStripMenuItem_Click(Object^  sender, EventArgs^  e) {
	Settings::Deserialize(this, Settings::GetSettingsPath());
}

void MainForm::saveSettingsToolStripMenuItem_Click(Object^  sender, EventArgs^  e) {
	Settings::Serialize(this, Settings::GetSettingsPath());
}

void MainForm::closeMapleStoryToolStripMenuItem_Click(Object^  sender, EventArgs^  e) {
	TerminateProcess(GetCurrentProcess(), 0);
}

//TODO: Make MainForm panels dynamically move to expand to height of 625 (make sure UI looks ok) 
//TODO: Also make embedded MS window resizable (add TrackBar in options?), make timelapse icon and name at the top move to the right to replace MS border
void EmbedMS(HWND hWnd) {
	RECT msRect;
	GetWindowRect(GlobalVars::mapleWindow, &msRect);

	if (!GlobalRefs::isEmbedding) {
		if (GlobalVars::mapleWindow) {
			GlobalRefs::hParent = SetParent(GlobalVars::mapleWindow, hWnd);
			if (GlobalRefs::hParent) {
				GlobalRefs::isEmbedding = TRUE;
				MainForm::TheInstance->Left = msRect.left;
				MainForm::TheInstance->Top = msRect.top;
				SetWindowPos(GlobalVars::mapleWindow, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			}
		}
	} else {
		SetParent(GlobalVars::mapleWindow, GlobalRefs::hParent);
		GlobalRefs::isEmbedding = FALSE;
		MainForm::TheInstance->Left = msRect.right;
		MainForm::TheInstance->Top = msRect.top;
	}
}

void MainForm::embedMSWindowToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
	if (!GlobalRefs::isEmbedding) {
		this->Width = 1360;
		this->Height = 625;
		this->pnlFull->Location = Point(800, 0);
	} else {
		this->Width = 560;
		this->Height = 500;
		this->pnlFull->Location = Point(0, 0);
	}

	EmbedMS((HWND)this->Handle.ToPointer());
}

void MainForm::hideMSWindowToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
	if (this->hideMSWindowToolStripMenuItem->Text == "Hide MS Window") {
		this->hideMSWindowToolStripMenuItem->Text = "Show MS Window";
		ShowWindow(GlobalVars::mapleWindow, SW_HIDE);
	} 
	else {
		this->hideMSWindowToolStripMenuItem->Text = "Hide MS Window";
		ShowWindow(GlobalVars::mapleWindow, SW_SHOW);
	}
}

#include <tlhelp32.h>
void DoSuspendThread()
{
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (h != INVALID_HANDLE_VALUE) {
		THREADENTRY32 te;
		te.dwSize = sizeof(te);
		if (Thread32First(h, &te)) {
			do {
				if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID)) {
					// Suspend all threads EXCEPT the one we want to keep running
					if (te.th32ThreadID != GetCurrentThreadId() && te.th32OwnerProcessID == GetCurrentProcessId()) {
						HANDLE thread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
						if (thread != nullptr) {
							SuspendThread(thread);
							CloseHandle(thread);
						}
					}
				}
				te.dwSize = sizeof(te);
			} while (Thread32Next(h, &te));
		}
		CloseHandle(h);
	}
}

void DoResumeThread()
{
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (h != INVALID_HANDLE_VALUE) {
		THREADENTRY32 te;
		te.dwSize = sizeof(te);
		if (Thread32First(h, &te)) {
			do {
				if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID)) {
					// Suspend all threads EXCEPT the one we want to keep running
					if (te.th32ThreadID != GetCurrentThreadId() && te.th32OwnerProcessID == GetCurrentProcessId()) {
						HANDLE thread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
						if (thread != nullptr) {
							ResumeThread(thread);
							CloseHandle(thread);
						}
					}
				}
				te.dwSize = sizeof(te);
			} while (Thread32Next(h, &te));
		}
		CloseHandle(h);
	}
}

void MainForm::pauseMSToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
	Diagnostics::ProcessThreadCollection^ threads = Diagnostics::Process::GetCurrentProcess()->Threads;

	if (this->pauseMSToolStripMenuItem->Text == "Pause MS") {
		this->pauseMSToolStripMenuItem->Text = "Resume MS";
		DoSuspendThread();
	} 
	else {
		this->pauseMSToolStripMenuItem->Text = "Pause MS";
		DoResumeThread();
	}
}
#pragma endregion

#pragma region Pointers
//Display Char Job value on hover
void MainForm::lbJob_MouseHover(Object^  sender, EventArgs^  e) {
	ToolTip^ jobToolTip = gcnew ToolTip();
	jobToolTip->IsBalloon = true;
	jobToolTip->ShowAlways = true;
	jobToolTip->SetToolTip(lbJob, PointerFuncs::getCharJobID().ToString());
}

//Display Char HP values on hover
void MainForm::lbHP_MouseHover(Object^  sender, EventArgs^  e) {
	ToolTip^ hpToolTip = gcnew ToolTip();
	hpToolTip->IsBalloon = true;
	hpToolTip->ShowAlways = true;
	hpToolTip->SetToolTip(lbHP, "(" + Assembly::curHP.ToString() + "/" + Assembly::maxHP.ToString() + ")");
}

//Display Char MP values on hover
void MainForm::lbMP_MouseHover(Object^  sender, EventArgs^  e) {
	ToolTip^ mpToolTip = gcnew ToolTip();
	mpToolTip->IsBalloon = true;
	mpToolTip->ShowAlways = true;
	mpToolTip->SetToolTip(lbMP, "(" + Assembly::curMP.ToString() + "/" + Assembly::maxMP.ToString() + ")");
}

//Display Char EXP values on hover
void MainForm::lbEXP_MouseHover(Object^  sender, EventArgs^  e) {
	ToolTip^ expToolTip = gcnew ToolTip();
	expToolTip->IsBalloon = true;
	expToolTip->ShowAlways = true;
	expToolTip->SetToolTip(lbEXP, "(" + Assembly::curEXP.ToString() + "/" + Assembly::maxEXP.ToString() + ")");
}

//Display World value on hover
void MainForm::lbWorld_MouseHover(Object^  sender, EventArgs^  e) {
	ToolTip^ worldToolTip = gcnew ToolTip();
	worldToolTip->IsBalloon = true;
	worldToolTip->ShowAlways = true;
	worldToolTip->SetToolTip(lbWorld, ReadPointer(ServerBase, OFS_World).ToString());
}

//Timer that ticks every 200ms and updates every label
void MainForm::GUITimer_Tick(Object^  sender, EventArgs^  e) {
	lbThreadID->Text = "0x" + GetMSThreadID().ToString("X");
	
	if (GetMSThreadID()) {;
		lbActive->Visible = true;
		lbInactive->Visible = false;
	}
	else {
		lbActive->Visible = false;
		lbInactive->Visible = true;
	}

	if (HelperFuncs::IsInGame()) {
		lbMapName->Text = PointerFuncs::getMapName();

		lbCharName->Text = PointerFuncs::getCharName();
		lbLevel->Text = PointerFuncs::getCharLevel();
		lbJob->Text = PointerFuncs::getCharJob();
		lbHP->Text = PointerFuncs::getCharHP();
		lbMP->Text = PointerFuncs::getCharMP();
		lbEXP->Text = PointerFuncs::getCharEXP();
		lbMesos->Text = PointerFuncs::getCharMesos().ToString("N0");

		lbWorld->Text = PointerFuncs::getWorld();
		lbChannel->Text = PointerFuncs::getChannel();
		lbMapID->Text = PointerFuncs::getMapID();
		lbWalls->Text = PointerFuncs::getMapLeftWall() + " " + PointerFuncs::getMapRightWall() + " " + PointerFuncs::getMapTopWall() + " " + PointerFuncs::getMapBottomWall();
		
		lbCharFoothold->Text = PointerFuncs::getCharFoothold();
		lbCharAnimation->Text = PointerFuncs::getCharAnimation();
		lbCharPos->Text = PointerFuncs::getCharPos();
		lbMousePos->Text = PointerFuncs::getMousePos();

		lbAttackCount->Text = PointerFuncs::getAttackCount();
		lbBuffCount->Text = PointerFuncs::getBuffCount();
		lbBreathCount->Text = PointerFuncs::getBreathCount();
		lbPeopleCount->Text = PointerFuncs::getPeopleCount();
		lbMobCount->Text = PointerFuncs::getMobCount();
		lbItemCount->Text = PointerFuncs::getItemCount();
		lbPortalCount->Text = PointerFuncs::getPortalCount();
		lbNPCCount->Text = PointerFuncs::getNPCCount();
	}
}
#pragma endregion

#pragma region Main Tab
#pragma region Log
void MainForm::lbConsoleLog_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
	if (e->Control && e->KeyCode == Keys::C) {
		System::Text::StringBuilder^ copyBuf = gcnew System::Text::StringBuilder();
		for each(auto item in lbConsoleLog->SelectedItems)
			copyBuf->AppendLine(item->ToString());

		if (copyBuf->Length > 0)
			Clipboard::SetText(copyBuf->ToString());
	}
}
#pragma endregion

#pragma region Auto Login

//TODO: Get Real Mac Address
String^ GetMac(bool generateFake) {
	String^ macAddress = "";

	if(generateFake) {
		for(int i = 0; i < 12; i++) {
			if (i != 0 && i % 2 == 0)
				macAddress += "-";
			macAddress += IntToHex(rand() % 16);
		}
	}

	return macAddress;
}

//TODO: Get Real HWID (CLogin::GetLocalMacAddressWithHDDSerialNo())
String^ GetHWID(bool generateFake, String^ mac) {
	String^ hwid = "";

	if(generateFake) {
		mac = mac->Replace("-", "");
		hwid += mac;
		hwid += "_";
		for (int i = 0; i < 8; i++)
			hwid += IntToHex(rand() % 16);
	}

	return hwid; 
}

void SendLoginPacket(String^ username, String^ password) {
	String^ packet = "";
	writeBytes(packet, gcnew array<BYTE>{0x01, 0x00}); //Login OpCode
	writeString(packet, username); //Username
	writeString(packet, password); //Password
	writeBytes(packet, gcnew array<BYTE>{0x00, 0x00}); //Login OpCode
	writeBytes(packet, gcnew array<BYTE>{0x00, 0x00,0x00}); //Login OpCode
	writeBytes(packet, gcnew array<BYTE>{0x00, 0xEA, 0x61, 0x29, 0x77}); //Login OpCode
	writeBytes(packet, gcnew array<BYTE>{0x00, 0x00, 0x00, 0x00, 0x9B, 0x8D, 0x00, 0x00, 0x00, 0x00 ,0x02 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00}); //Unknown bytes
	//rest of packet unused for now
	SendPacket(packet);
}

void SendCharListRequestPacket(int world, int channel) {
	String^ packet = "";
	writeBytes(packet, gcnew array<BYTE>{0x05, 0x00}); //Character List Request OpCode
	writeByte(packet, 0x02); //Unknown byte
	writeByte(packet, 0x00); //World
	writeByte(packet, channel); //Channel
	writeBytes(packet, gcnew array<BYTE>{ 0x0A, 0x00, 0x01, 0x1C}); //Unknown bytes
	SendPacket(packet);
}

void SendSelectCharPacket(int character, bool existsPIC) {
	String^ packet = "";
	String^ macAddress = GetMac(true);

	
	if(existsPIC) {
		String^ PIC = MainForm::TheInstance->tbAutoLoginPIC->Text;
		String^ loc_ign = MainForm::TheInstance->ign->Text;
		std::string normalized_ign = marshal_as<std::string>(loc_ign);
		std::vector<char> loc_ignB = HexToBytes(normalized_ign);
		writeBytes(packet, gcnew array<BYTE>{0x1E, 0x00});
		writeString(packet, PIC); //PIC
		//writeInt(packet, character); //Character Number (starts with 1)
		//writeByte(packet, (*(BYTE*)CharacterStatBase + OFS_Ign));
		//writeByte(packet, (*(BYTE*)CharacterStatBase + OFS_Ign + 1));
		//writeByte(packet, (*(BYTE*)CharacterStatBase + OFS_Ign + 2));
		//writeByte(packet, (*(BYTE*)CharacterStatBase + OFS_Ign + 3));
		//writeBytes(packet, gcnew array<BYTE>{0x48, 0x33, 0x0B, 0x00}); //IGN
		writeBytes(packet, gcnew array<BYTE>{loc_ignB[0], loc_ignB[1]});
		writeBytes(packet, gcnew array<BYTE>{0x0B, 0x00});
		writeString(packet, macAddress); //Mac Address
		writeString(packet, GetHWID(true, macAddress)); //HWID
		SendPacket(packet);
	}
	else {
		/*
		writeBytes(packet, gcnew array<BYTE>{0x13, 0x00}); //Character Select (Without PIC) OpCode
		writeInt(packet, character); //Character Number (starts with 1)
		writeString(packet, macAddress); //Mac Address
		writeString(packet, GetHWID(true, macAddress)); //HWID
		SendPacket(packet);
		*/
	}
}

//Only works for HeavenMS as of now, maybe on other private servers that doesn't check the validity of the fake hwid/mac address in the packets
void AutoLogin() {
	if (*(BYTE*)(*(ULONG*)LoginBase + OFS_LoginScreen) == 255) {
		Log::WriteLineToConsole("AutoLogin: Logging in...");
		String^ usernameStr = MainForm::TheInstance->tbAutoLoginUsername->Text;
		String^ passwordStr = MainForm::TheInstance->tbAutoLoginPassword->Text;
		SendLoginPacket(usernameStr, passwordStr);
		//SendPacket(GlobalRefs::globalAutoLogin);
		//SendPacket("01 00 0C 00 54 68 72 61 73 68 65 72 31 32 37 36 0A 00 50 69 7A 7A 61 72 64 31 32 34 00 00 00 00 00 00 EA 61 29 77 00 00 00 00 9B 8D 00 00 00 00 02 00 00 00 00 00 00");
		Sleep(2000);

		int world = MainForm::TheInstance->comboAutoLoginWorld->SelectedIndex;
		int channel = MainForm::TheInstance->comboAutoLoginChannel->SelectedIndex; 


		SendCharListRequestPacket(world, channel);
		Sleep(4000);

		//int character = MainForm::TheInstance->comboAutoLoginCharacter->SelectedIndex + 1;
		//bool existsPIC = MainForm::TheInstance->cbAutoLoginPic->Checked;
		////SendPacket("1E 00 06 00 37 32 30 36 31 33 48 33 0B 00 11 00 30 30 2D 31 33 2D 32 33 2D 45 37 2D 36 36 2D 39 36 15 00 30 30 31 33 32 33 45 37 36 36 39 36 5F 45 41 36 31 32 39 37 37");
		//SendSelectCharPacket(character, existsPIC);
		SendSelectCharPacket(1, true);
		Log::WriteLineToConsole("AutoLogin: Login Completed");
	}
}

void SetLoginHooks(bool enable) {
	Hooks::CLogin__OnRecommendWorldMessage_Hook(true);

	if (!enable) {
		//MessageBox::Show("False!");
		Hooks::CLogin__OnRecommendWorldMessage_Hook(false);
	}
	else {
		MessageBox::Show("true!");
		Hooks::CLogin__OnRecommendWorldMessage_Hook(true); //Crashes, wrong params maybe

	}
}

//TODO: Maybe just write it on load instead of on load save? Not sure
//Logo Skip (CLogo::UpdateLogo())
void MainForm::cbAutoLoginSkipLogo_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (this->cbAutoLoginSkipLogo->Checked)
		WriteMemory(logoSkipAddr, 1, 0x75); //jne 0062F2EB [first byte]
	else
		WriteMemory(logoSkipAddr, 1, 0x74); //je 0062F2EB [first byte]
}

void MainForm::cbAutoLogin_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if(this->cbAutoLogin->Checked)
	{
		//GlobalRefs::autologintime = gcnew AutoLoginTimer::starttimer(5000,&AutoLogin);
		timer1->Enabled = true;
		AutoLogin();
	}
	else {
		timer1->Enabled = true;
		SetLoginHooks(false);
	}
}
#pragma endregion

#pragma region Options
void MainForm::transparencyTrackBar_Scroll(Object^  sender, EventArgs^  e) {
	this->Opacity = transparencyTrackBar->Value * 0.01;
}
#pragma endregion
#pragma endregion

#pragma region Bots Tab
//Keycode based on index selected in combo boxes
static int keyCollection[] = { 0x10, 0x11, 0x12, 0x20, 0x2D, 0x2E, 0x24, 0x23, 0x21, 0x22, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51,
0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };

//Check if keypress is valid
static bool isKeyValid(Object^ sender, Windows::Forms::KeyPressEventArgs^ e, bool isSigned) {
	bool result = true;

	//If the character is not a digit or backspace, don't allow it
	if (!isdigit(e->KeyChar) && e->KeyChar != '\b') result = false;

	//If the textbox is supposed to contain a signed value, check to see if '-' should be permitted
	if (isSigned && e->KeyChar == '-') {
		Windows::Forms::TextBox^ textBox = safe_cast<Windows::Forms::TextBox^>(sender);

		//If the selected text does not start at the beginning of the text, don't allow the '-' keypress
		if (textBox->SelectionStart > 0) result = false;
		//If the selection starts at 0 and there exists a '-' in the complete text, and the selected text doesn't have '-', don't allow an additional '-' keypress
		else if (textBox->Text->IndexOf('-') > -1 && !textBox->SelectedText->Contains("-")) result = false;
		else result = true;
	}

	return result;
}
#pragma region Auto HP
void MainForm::cbHP_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbHP->Checked) {
		if (GlobalRefs::macroHP == nullptr)
			GlobalRefs::macroHP = gcnew Macro(keyCollection[this->comboHPKey->SelectedIndex], Convert::ToUInt32(HPPotDelay->Text), MacroType::HPPOTMACRO);
		GlobalRefs::macroHP->Toggle(true);
		MacrosEnabled::bMacroHP = true;
	}
	else {
		GlobalRefs::macroHP->Toggle(false);
		MacrosEnabled::bMacroHP = false;
	}
}

void MainForm::comboHPKey_SelectedIndexChanged(Object^  sender, EventArgs^  e) {
	if (GlobalRefs::macroHP != nullptr)
		GlobalRefs::macroHP->keyCode = keyCollection[this->comboHPKey->SelectedIndex];
}

void MainForm::tbHP_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
#pragma endregion

#pragma region Auto MP
void MainForm::cbMP_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbMP->Checked) {
		if (GlobalRefs::macroMP == nullptr){}
			GlobalRefs::macroMP = gcnew Macro(keyCollection[this->comboMPKey->SelectedIndex], Convert::ToUInt32(MPPotDelay->Text), MacroType::MPPOTMACRO);
		GlobalRefs::macroMP->Toggle(true);
		MacrosEnabled::bMacroMP = true;
	}
	else {
		GlobalRefs::macroMP->Toggle(false);
		MacrosEnabled::bMacroMP = false;
	}
}

void MainForm::comboMPKey_SelectedIndexChanged(Object^  sender, EventArgs^  e) {
	if (GlobalRefs::macroMP != nullptr)
		GlobalRefs::macroMP->keyCode = keyCollection[this->comboMPKey->SelectedIndex];
}

void MainForm::tbMP_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
#pragma endregion

#pragma region Auto Attack
void MainForm::cbAttack_CheckedChanged(Object^  sender, EventArgs^  e) {
	if(this->cbAttack->Checked) {
		if(GlobalRefs::macroAttack == nullptr) {
			if (String::IsNullOrWhiteSpace(tbAttackInterval->Text)) {
				MessageBox::Show("Error: Attack Interval textbox cannot be empty");
				this->cbAttack->Checked = false;
				return;
			}
		}
		this->tAutoAttack->Interval = Convert::ToInt32(tbAttackInterval->Text);
		this->tAutoAttack->Enabled = true; //cbAttack->Checked
		MacrosEnabled::bMacroAttack = true;
	}
	else {
		this->tAutoAttack->Enabled = false;
		MacrosEnabled::bMacroAttack = false;
	}
}

void MainForm::tbAttackInterval_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tAutoAttack_Tick(Object^ sender, EventArgs^ e) {
	if (HelperFuncs::ValidToAttack()) {
		KeyMacro::SpamPressKey(keyCollection[this->comboAttackKey->SelectedIndex], 2);
	}
}

void MainForm::tbAttackInterval_TextChanged(Object^  sender, EventArgs^  e) {
	if (GlobalRefs::macroAttack != nullptr) {
		if (String::IsNullOrWhiteSpace(tbAttackInterval->Text)) {
			MessageBox::Show("Error: Attack Interval textbox cannot be empty");
			return;
		}
		GlobalRefs::macroAttack->delay = Convert::ToUInt32(tbAttackInterval->Text);
	}
}

void MainForm::tbAttackMob_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::comboAttackKey_SelectedIndexChanged(Object^  sender, EventArgs^  e) {
	if (GlobalRefs::macroAttack != nullptr)
		GlobalRefs::macroAttack->keyCode = keyCollection[this->comboAttackKey->SelectedIndex];
}
#pragma endregion

#pragma region Auto Loot
void MainForm::cbLoot_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbLoot->Checked) {
		if (GlobalRefs::macroLoot == nullptr) {
			if (String::IsNullOrWhiteSpace(tbLootInterval->Text)) {
				MessageBox::Show("Error: Loot Interval textbox cannot be empty");
				this->cbLoot->Checked = false;
				return;
			}
		}
		this->tAutoLoot->Interval = Convert::ToInt32(tbAttackInterval->Text);
		this->tAutoLoot->Enabled = true; //cbLoot->Checked
		MacrosEnabled::bMacroLoot = true;
	}
	else {
		this->tAutoLoot->Enabled = false; 
		MacrosEnabled::bMacroLoot = false;
	}
}

void MainForm::tbLootInterval_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tAutoLoot_Tick(System::Object^  sender, System::EventArgs^  e) {
	if (HelperFuncs::ValidToLoot()) {
		KeyMacro::SpamPressKey(keyCollection[this->comboLootKey->SelectedIndex], 2);
	}
}

void MainForm::Timer1_Tick(System::Object^ sender, System::EventArgs^ e) {
	Log::WriteLineToConsole("timer1 tick!");
		if (this->cbAutoLogin->Checked)
		{
			AutoLogin();
		}
}

void MainForm::tbLootInterval_TextChanged(Object^  sender, EventArgs^  e) {
	if (GlobalRefs::macroLoot != nullptr) {
		if (String::IsNullOrWhiteSpace(tbLootInterval->Text)) {
			MessageBox::Show("Error: Loot Interval textbox cannot be empty");
			return;
		}
		GlobalRefs::macroLoot->delay = Convert::ToUInt32(tbLootInterval->Text);
	}
}

void MainForm::tbLootItem_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::comboLootKey_SelectedIndexChanged(Object^  sender, EventArgs^  e) {
	if (GlobalRefs::macroLoot != nullptr)
		GlobalRefs::macroLoot->keyCode = keyCollection[this->comboLootKey->SelectedIndex];
}
#pragma endregion

#pragma region Auto Buffs
void MainForm::bBuffAdd_Click(Object^  sender, EventArgs^  e) {
	if(String::IsNullOrWhiteSpace(tbBuffInterval->Text)) {
		MessageBox::Show("Error: Buff Interval textbox cannot be empty");
		return;
	}
	ListViewItem^ lvi = gcnew ListViewItem(gcnew array<String^>{tbBuffName->Text, comboBuffKey->Text, tbBuffInterval->Text});
	lvi->Tag = gcnew Macro(keyCollection[comboBuffKey->SelectedIndex], Convert::ToUInt32(tbBuffInterval->Text)*1000, MacroType::BUFFMACRO);
	lvi->Checked = true;
	lvBuff->Items->Add(lvi);
}

void MainForm::bBuffEnableAll_Click(Object^  sender, EventArgs^  e) {
	for each (ListViewItem^ lvi in lvBuff->Items) {
		Macro^ macro = (Macro^)lvi->Tag;
		macro->Toggle(true);
		lvi->Checked = true;
	}
}

void MainForm::bBuffDisableAll_Click(Object^  sender, EventArgs^  e) {
	for each (ListViewItem ^lvi in lvBuff->Items) {
		Macro^ macro = (Macro^)lvi->Tag;
		macro->Toggle(false);
		lvi->Checked = false;
	}
}

void MainForm::bBuffRemove_Click(Object^  sender, EventArgs^  e) {
	for each(ListViewItem^ lvi in lvBuff->SelectedItems) {
		Macro^ macro = (Macro^)lvi->Tag;
		macro->Toggle(false);
		lvBuff->Items->Remove(lvi);
	}
}

void MainForm::bBuffClear_Click(Object^  sender, EventArgs^  e) {
	for each(ListViewItem^ lvi in lvBuff->Items) {
		Macro^ macro = (Macro^)lvi->Tag;
		macro->Toggle(false);
		lvBuff->Items->Remove(lvi);
	}
}

void MainForm::lvBuff_ItemChecked(Object^  sender, Windows::Forms::ItemCheckedEventArgs^  e) {
	Macro^ macro = (Macro^)e->Item->Tag;
	macro->Toggle(e->Item->Checked);
}

void MainForm::tbBuffInterval_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
#pragma endregion

#pragma region Auto CC/CS
//TODO: add option to whitelist or blacklist channels

void CallCSFunc() {
	if (PointerFuncs::getMapID()->Equals("0")) return;
	WritePointer(UserLocalBase, OFS_Breath, 0);
	CWvsContext__SendMigrateToShopRequest(*(PVOID*)ServerBase, (PVOID)0x2FDFE1D, 0);
	Sleep(Convert::ToUInt32(MainForm::TheInstance->tbCSDelay->Text));
	//CCashShop__SendTransferFieldPacket();
	Sleep(1000);
}

void _stdcall AutoCC(int toChannel) {
	int channel;
	//List<int> excludedChannels = {};
	// read string from settings	
	//String excludedChannelsText = MainForm::TheInstance->tbCSDelay->Text;
	//int i;
	//for (i=0, i<sizeof(excludedChannelsText), i++)
	// parse string by comma into excludedChanList
	//String firstchar = excludedChannelsText[1];

	if (toChannel == -1) channel = rand() % 19;
	else channel = toChannel;

	if (MainForm::TheInstance->rbPacket->Checked)
		SendPacket(gcnew String("27 00 " + channel.ToString("X2") + " ** ** ** 00")); //Send Auto CC Packet
	else
		Hooks::ChangeChannel(channel); //CallCCFunc(channel); //Call Auto CC Function

	Sleep(200); 
}

void _stdcall AutoCS() {
	if (MainForm::TheInstance->rbPacket->Checked) { 
		if(String::IsNullOrWhiteSpace(MainForm::TheInstance->tbCSDelay->Text)) {
			MessageBox::Show("Error: CS Delay textbox cannot be empty");
			return;
		}
		SendPacket(gcnew String("28 00 ** ** ** 00")); //Send go to CS packet
		Sleep(Convert::ToUInt32(MainForm::TheInstance->tbCSDelay->Text));
		SendPacket(gcnew String("26 00")); //Send transfer back to field packet
		Sleep(500);
	}
	else 
		CallCSFunc(); //Call CS Function and return to field Function
}

void MainForm::rbFunction_CheckedChanged(Object^  sender, EventArgs^  e) {
	cbCCCSTime->Checked = false;
	cbCCCSPeople->Checked = false;
	cbCCCSAttack->Checked = false;
	cbCCCSMob->Checked = false;
	GlobalRefs::cccsTimerTickCount = 0;
}

void MainForm::rbCC_CheckedChanged(Object^  sender, EventArgs^  e) {
	cbCCCSTime->Checked = false;
	cbCCCSPeople->Checked = false;
	cbCCCSAttack->Checked = false;
	cbCCCSMob->Checked = false;
	GlobalRefs::cccsTimerTickCount = 0;
}

void MainForm::AutoCCCSTimer_Tick(Object^  sender, EventArgs^  e) {
	if (GlobalRefs::isMapRushing) return;

	if(cbCCCSTime->Checked) {
		GlobalRefs::cccsTimerTickCount += 250;
		if ((GlobalRefs::cccsTimerTickCount/1000) > Convert::ToUInt32(tbCCCSTime->Text)) {
			if (rbCC->Checked) AutoCC(-1);
			else AutoCS();
			GlobalRefs::cccsTimerTickCount = 0;
		}
	}

	if (cbCCCSPeople->Checked) {
		if (ReadPointer(UserPoolBase, OFS_PeopleCount) > Convert::ToUInt32(tbCCCSPeople->Text)) {
			if (rbCC->Checked) AutoCC(-1);
			else AutoCS();
		}
	}

	if (cbCCCSAttack->Checked) {
		if(ReadPointer(UserLocalBase, OFS_AttackCount) > Convert::ToUInt32(tbCCCSAttack->Text)) {
			if (rbCC->Checked) AutoCC(-1);
			else AutoCS();
		}
	}

	if (cbCCCSMob->Checked) {
		if (ReadPointer(MobPoolBase, OFS_MobCount) < Convert::ToUInt32(tbCCCSMob->Text)) {
			if (rbCC->Checked) AutoCC(-1);
			else AutoCS();
		}
	}
}

void MainForm::cbCCCSTime_CheckedChanged(Object^  sender, EventArgs^  e) {
	if(this->cbCCCSTime->Checked) GlobalRefs::cccsTimerTickCount = 0;
}

void MainForm::bCC_Click(Object^  sender, EventArgs^  e) {
	if (!GlobalRefs::isMapRushing && !GlobalRefs::isChangingField) {
		GlobalRefs::isChangingField = true;
		WritePointer(UserLocalBase, OFS_Breath, 0);
		AutoCC(this->comboChannelKey->SelectedIndex);
		GlobalRefs::isChangingField = false;
	}
}

void MainForm::bRandomCC_Click(Object^  sender, EventArgs^  e) {
	if (!GlobalRefs::isMapRushing && !GlobalRefs::isChangingField) {
		GlobalRefs::isChangingField = true;
		WritePointer(UserLocalBase, OFS_Breath, 0);
		AutoCC(-1);
		GlobalRefs::isChangingField = false;
	}
}

void MainForm::bCS_Click(Object^  sender, EventArgs^  e) {
	if (!GlobalRefs::isMapRushing && !GlobalRefs::isChangingField) {
		GlobalRefs::isChangingField = true;
		WritePointer(UserLocalBase, OFS_Breath, 0);
		AutoCS();
		GlobalRefs::isChangingField = false;
	}
}

void MainForm::tbCCCSTime_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbCCCSPeople_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbCCCSAttack_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbCCCSMob_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbCSDelay_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
#pragma endregion 

#pragma region Auto Sell Tab
void MainForm::cbSellAll_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (this->cbSellAll->Checked) {
		//if (HelperFuncs::IsInventoryFull()) {
			//deactivate hacks and macro
			if (this->cbZzVac->Checked)
				cbZzVac->Checked = false;
			if (this->cbKami->Checked)
				cbKami->Checked = false;
			if (this->cbVacLeft->Checked)
				cbVacLeft->Checked = false;
			if (this->cbVacRight->Checked)
				cbVacRight->Checked = false;

			MacrosEnabled::bMacroLoot = false;
			MacrosEnabled::bMacroAttack = false;
			MacrosEnabled::bMacroHP = false;
			MacrosEnabled::bMacroMP = false;
			// TODO: rush to mapID	   
			SellAtEquipMapId(220050300); //TESTING autosell at lubridum path of time
			// TODO: reverse rushpath back
		    // activate hack and begin again
		//}
	}
	else {
		MacrosEnabled::bMacroLoot = true;
		MacrosEnabled::bMacroAttack = true;
		MacrosEnabled::bMacroHP = true;
		MacrosEnabled::bMacroMP = true;
	}
}
#pragma endregion
#pragma endregion

#pragma region Hacks I Tab
//Full Godmode (CUserLocal::SetDamaged())
void MainForm::cbFullGodmode_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbFullGodmode->Checked)
		WriteMemory(fullGodmodeAddr, 2, 0x0F, 0x84); //je 009596F7 [first 2 bytes]
	else
		WriteMemory(fullGodmodeAddr, 2, 0x0F, 0x85); //jne 009596F7 [first 2 bytes]
}

//TODO: Add number of misses
//Miss Godmode (CUserLocal::SetDamaged())
void MainForm::cbMissGodmode_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbMissGodmode->Checked)
		WriteMemory(missGodmodeAddr, 8, 0xC7, 0x06, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90); //mov [esi],00000000; nop; nop;
	else
		WriteMemory(missGodmodeAddr, 8, 0x89, 0x06, 0x83, 0xC6, 0x04, 0xFF, 0x4D, 0xC4); //mov [esi],eax; add esi,04; dec [ebp-3C];
}

//TODO: Add number of blinks 
//Blink Godmode (CUser::Update())
void MainForm::cbBlinkGodmode_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbBlinkGodmode->Checked)
		WriteMemory(blinkGodmodeAddr, 2, 0x83, 0xC7); //add edi,1E
	else
		WriteMemory(blinkGodmodeAddr, 2, 0x83, 0xEF); //sub edi,1E
}

void ClickTeleport() {
	while (GlobalRefs::bClickTeleport) {
		if (ReadPointer(InputBase, OFS_MouseAnimation) == 12)
			Teleport(ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseX), ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseY));
		Sleep(Convert::ToUInt32(MainForm::TheInstance->tbClickTeleport->Text));
	}
	ExitThread(0);
}

void MouseTeleport() {
	while (GlobalRefs::bMouseTeleport) {
		if (ReadPointer(InputBase, OFS_MouseAnimation) == 00)
			Teleport(ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseX), ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseY));
		Sleep(Convert::ToUInt32(MainForm::TheInstance->tbMouseTeleport->Text));
	}
	ExitThread(0);
}

void MainForm::cbClickTeleport_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbClickTeleport->Checked) {
		GlobalRefs::bClickTeleport = true;
		NewThread(ClickTeleport);
	}
	else
		GlobalRefs::bClickTeleport = false;
}
 
void MainForm::cbMouseTeleport_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbMouseTeleport->Checked) {
		GlobalRefs::bMouseTeleport = true;
		NewThread(MouseTeleport);
	}
	else
		GlobalRefs::bMouseTeleport = false;
}

void MainForm::tbClickTeleport_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbMouseTeleport_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

//Mouse Fly (CVecCtrl::raw_GetSnapshot())
void MainForm::cbMouseFly_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbMouseFly->Checked) {
		Jump(mouseFlyXAddr, Assembly::MouseFlyXHook, 0); //MouseFlyXHook CodeCave
		Jump(mouseFlyYAddr, Assembly::MouseFlyYHook, 0); //MouseFlyYHook CodeCave
	}
	else {
		WriteMemory(mouseFlyXAddr, 5, 0x89, 0x03, 0x8B, 0x7D, 0x10); //mov [ebx],eax; mov edi,[ebp+10]
		WriteMemory(mouseFlyYAddr, 5, 0x89, 0x07, 0x8B, 0x5D, 0x14); //mov [edi],eax; mov ebx,[ebp+14]
	}
}

//Swim in Air (CVecCtrl::IsSwimming())
void MainForm::cbSwimInAir_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbSwimInAir->Checked)
		WriteMemory(swimInAirAddr, 2, 0x74, 0x04); //je 0070470A
	else
		WriteMemory(swimInAirAddr, 2, 0x75, 0x04); //jne 0070470A
}

//Unlimited Attack (CAntiRepeat::TryRepeat())
void MainForm::cbUnlimitedAttack_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbUnlimitedAttack->Checked)
		WriteMemory(unlimitedAttackAddr, 1, 0xEB); //jmp 0095370C [first byte]
	else
		WriteMemory(unlimitedAttackAddr, 1, 0x7E); //jle 0095370C [first byte]
}

//Full Accuracy
void MainForm::cbFullAccuracy_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbFullAccuracy->Checked)
		WriteMemory(fullAccuracyAddr, 8, 0x00, 0x00, 0x00, 0xE0, 0xCF, 0x12, 0x63, 0x41); //add [eax],al; add al,ah; iretd; adc ah,byte ptr [ebx+41];
	else
		WriteMemory(fullAccuracyAddr, 8, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0xE6, 0x3F); //out 3F,al
}

//No Breath (CAvatar::Update())
void MainForm::cbNoBreath_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoBreath->Checked)
		WriteMemory(noBreathAddr, 1, 0x7C); //jl 0045233D [first byte]
	else
		WriteMemory(noBreathAddr, 1, 0x7D); //jnl 0045233D [first byte]
}

//No Player Knockback (CVecCtrl::SetImpactNext())
void MainForm::cbNoPlayerKnockback_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoPlayerKnockback->Checked)
		WriteMemory(noPlayerKnocbackAddr, 3, 0xC7, 0x00, 0x00); //mov [eax],00000000 [first 3 bytes] 
	else
		WriteMemory(noPlayerKnocbackAddr, 3, 0xC7, 0x00, 0x01); //mov [eax],00000001 [first 3 bytes]
}

//No Player Death (CUserLocal::OnResolveMoveAction())
void MainForm::cbNoPlayerDeath_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoPlayerDeath->Checked)
		WriteMemory(noPlayerDeathAddr, 2, 0xEB, 0x0B); //jmp 009506D3
	else
		WriteMemory(noPlayerDeathAddr, 2, 0x74, 0x0B); //je 009506D3
}

//Jump Down Any Tile (CUserLocal::FallDown())
void MainForm::cbJumpDownAnyTile_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbJumpDownAnyTile->Checked)
		WriteMemory(jumpDownAnywhereAddr, 2, 0x90, 0x90); //nop; nop;
	else
		WriteMemory(jumpDownAnywhereAddr, 2, 0x74, 0x1E); //je 0094C70E
}

//No Skill Effects (CUser::ShowSkillEffect())
void MainForm::cbNoSkillEffects_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoSkillEffects->Checked)
		WriteMemory(noSkillEffectAddr, 5, 0xC2, 0x14, 0x00, 0x90, 0x90); //ret 0014; nop; nop; nop;
	else
		WriteMemory(noSkillEffectAddr, 5, 0xB8, 0x34, 0xC3, 0xAD, 0x00); //mov eax,00ADC334
}

//No Attack Delay (CUser::SetAttackAction())
void MainForm::cbNoAttackDelay_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoAttackDelay->Checked)
		WriteMemory(noAttackDelayAddr, 10, 0x6A, 0x01, 0x58, 0xC2, 0x10, 0x00, 0x90, 0x90, 0x90, 0x90); //push 01; pop eax; ret 0010; nop; nop; nop; nop;
	else
		WriteMemory(noAttackDelayAddr, 10, 0xB8, 0x88, 0xB7, 0xAD, 0x00, 0xE8, 0xDC, 0x1C, 0x13, 0x00); //mov eax,00ADB788; call 00A60B98;
}

//No Player Name Tag (CUser::DrawNameTags())
void MainForm::cbNoPlayerNameTag_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoPlayerNameTag->Checked)
		WriteMemory(noPlayerNameTagAddr, 5, 0xC3, 0x90, 0x90, 0x90, 0x90); //ret; nop; nop; nop; nop;
	else
		WriteMemory(noPlayerNameTagAddr, 5, 0xB8, 0x14, 0xDA, 0xAD, 0x00); //mov eax,00ADDA14
}

void MainForm::tbAttackDelay_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbAttackDelay_TextChanged(Object^ sender, EventArgs^ e) {
	String^ animDelayStr = tbAttackDelay->Text;
	if (String::IsNullOrWhiteSpace(animDelayStr)) {
		MessageBox::Show("Error: Attack delay textbox cannot be empty");
		return;
	}

	const int animDelayInt = Convert::ToInt32(animDelayStr);
	if (animDelayInt < -128) {
		tbAttackDelay->Text = "-128";
		Assembly::animDelay = -128;
	} else if (animDelayInt > 127) {
		tbAttackDelay->Text = "127";
		Assembly::animDelay = 127;
	}
	Assembly::animDelay = animDelayInt;
}

//TODO: the value is a byte thus should be settable in range of -128_127
//Attack Delay (CAvatar::PrepareActionLayer())
//No Attack Delay Method 2 (skips animation, same function) Address: 0045478F Changed Memory: 0F8E->0F8F //jg 0045483D [first 2 bytes]
void MainForm::cbAttackDelay_CheckedChanged(Object^ sender, EventArgs^ e) {
	if (this->cbAttackDelay->Checked)
		WriteMemory(attackDelayAddr, 3, 0x83, 0xC1, Assembly::animDelay); //add ecx, [animDelay]
	else
		WriteMemory(attackDelayAddr, 3, 0x83, 0xC0, 0x0A); //add eax,0a
}

//Instant Drop Items
void MainForm::cbInstantDropItems_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbInstantDropItems->Checked)
		WriteMemory(instantDropItemsAddr, 4, 0x00, 0x00, 0x00, 0x00); //add [eax],al; add [eax],al;
	else
		WriteMemory(instantDropItemsAddr, 4, 0x00, 0x40, 0x8F, 0x40); //add [eax-71],al; inc eax;
}

//Instant Loot Items (CAnimationDisplayer::ABSORBITEM::Update())
void MainForm::cbInstantLootItems_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbInstantLootItems->Checked)
		WriteMemory(instantLootItemsAddr, 6, 0x81, 0xFB, 0x00, 0x00, 0x00, 0x00); //cmp ebx,00000000
	else
		WriteMemory(instantLootItemsAddr, 6, 0x81, 0xFB, 0xBC, 0x02, 0x00, 0x00); //cmp ebx,000002BC
}

//Tubi (CWvsContext::CanSendExclRequest())
void MainForm::cbTubi_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbTubi->Checked)
		WriteMemory(tubiAddr, 2, 0x90, 0x90); //nop; nop;
	else
		WriteMemory(tubiAddr, 2, 0x75, 0x36); //jne 00485C39
}

//Item Vac (CDropPool::TryPickUpDrop())
void MainForm::cbItemVac_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbItemVac->Checked)
		Jump(itemVacAddr, Assembly::ItemVacHook, 2);
	else
		WriteMemory(itemVacAddr, 7, 0x50, 0xFF, 0x75, 0xDC, 0x8D, 0x45, 0xCC);
}

//No Mob Reaction (CMob::AddDamageInfo())
void MainForm::cbNoMobReaction_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoMobReaction->Checked)
		WriteMemory(noMobReactionAddr, 5, 0xC2, 0x44, 0x00, 0x90, 0x90); //ret 0044; nop; nop;
	else
		WriteMemory(noMobReactionAddr, 5, 0xB8, 0x50, 0x1A, 0xAA, 0x00); //mov eax,00AA1A50
}

//No Mob Death Effect (CMob::OnDie())
void MainForm::cbNoMobDeathEffect_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoMobDeathEffect->Checked)
		WriteMemory(noMobDeathEffectAddr, 5, 0xC3, 0x90, 0x90, 0x90, 0x90); //ret; nop; nop; nop; nop;
	else
		WriteMemory(noMobDeathEffectAddr, 5, 0xB8, 0xD4, 0x13, 0xAA, 0x00); //mov eax,00AA13D4
}

//No Mob Knockback (CMob::OnHit())
void MainForm::cbNoMobKnockback_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoMobKnockback->Checked)
		WriteMemory(noMobKnockbackAddr, 2, 0x90, 0x90); //nop; nop;
	else
		WriteMemory(noMobKnockbackAddr, 2, 0x74, 0x20); //je 00668CC0
}

//Mob Freeze (CVecCtrlMob::WorkUpdateActive())
void MainForm::cbMobFreeze_CheckedChanged(Object^  sender, EventArgs^  e)
{
	if (this->cbMobFreeze->Checked)
		Jump(mobFreezeAddr, Assembly::MobFreezeHook, 1); //MobFreezeHook CodeCave
	else
		WriteMemory(mobFreezeAddr, 6, 0x8B, 0x86, 0x48, 0x02, 0x00, 0x00); //mov eax,[esi+00000248]
}

//Mob Disarm (CMob::Update())
void MainForm::cbMobDisarm_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbMobDisarm->Checked)
		WriteMemory(mobDisarmAddr, 9, 0xE9, 0xFD, 0x01, 0x00, 0x00, 0x90, 0x90, 0x90, 0x90); //jmp 00667C02; nop; nop; nop; nop;
	else
		WriteMemory(mobDisarmAddr, 9, 0x75, 0x0E, 0x8B, 0xCB, 0xE8, 0x24, 0x69, 0x00, 0x00); //jne 00667A10; mov ecx,ebx; call 0066E32D;
}

//Mob Auto Aggro (CVecCtrlMob::WorkUpdateActive())
void MainForm::cbMobAutoAggro_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbMobAutoAggro->Checked)
		Jump(mobAutoAggroAddr, Assembly::MobAutoAggroHook, 0); //MobAutoAggroHook CodeCave
	else
		WriteMemory(mobAutoAggroAddr, 5, 0xE8, 0xD4, 0x4E, 0xFF, 0xFF); //call 009B19D0 (calls CVecCtrl::WorkUpdateActive())
}

//No Map Background (CMapReloadable::LoadMap())
void MainForm::cbNoMapBackground_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoMapBackground->Checked)
		WriteMemory(noMapBackgroundAddr, 5, 0x90, 0x90, 0x90, 0x90, 0x90); //nop; nop; nop; nop; nop;
	else
		WriteMemory(noMapBackgroundAddr, 5, 0xE8, 0xFF, 0x2E, 0x00, 0x00); //call 0063CBBA
}

//No Map Objects (CMapReloadable::LoadMap())
void MainForm::cbNoMapObjects_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoMapObjects->Checked)
		WriteMemory(noMapObjectsAddr, 5, 0x90, 0x90, 0x90, 0x90, 0x90); //nop; nop; nop; nop; nop;
	else
		WriteMemory(noMapObjectsAddr, 5, 0xE8, 0xCA, 0x0D, 0x00, 0x00); //call 0063AA7E
}

//No Map Tiles (CMapReloadable::LoadMap())
void MainForm::cbNoMapTiles_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoMapTiles->Checked)
		WriteMemory(noMapTitlesAddr, 5, 0x90, 0x90, 0x90, 0x90, 0x90); //nop; nop; nop; nop; nop;
	else
		WriteMemory(noMapTitlesAddr, 5, 0xE8, 0x53, 0x04, 0x00, 0x00); //call 0063A100
}

//No Map Fade Effect (CStage::FadeOut())
void MainForm::cbNoMapFadeEffect_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoMapFadeEffect->Checked)
		WriteMemory(noMapFadeEffect, 5, 0xC3, 0x90, 0x90, 0x90, 0x90); //ret; nop; nop; nop; nop;
	else
		WriteMemory(noMapFadeEffect, 5, 0xB8, 0x44, 0x61, 0xAB, 0x00); //mov eax,00AB6144
}

//Map Speed Up (max_walk_speed())
void MainForm::cbMapSpeedUp_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbMapSpeedUp->Checked)
		WriteMemory(mapSpeedUpAddr, 3, 0x8D, 0x48, 0x0C); //lea ecx,[eax+0C]
	else
		WriteMemory(mapSpeedUpAddr, 3, 0x8D, 0x48, 0x24); //lea ecx,[eax+24]
}

//Infinite & Uncensored Chat
void MainForm::cbInfiniteChat_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbInfiniteChat->Checked) {
		WriteMemory(removeSpamFilterAddr1, 2, 0xEB, 0x27); //jmp 00490630
		WriteMemory(removeSpamFilterAddr2, 2, 0xEB, 0x1D); //jmp 00490670
		WriteMemory(infiniteChatboxAddr1, 2, 0xEB, 0x7E); //jmp 004CAA89
		WriteMemory(infiniteChatboxAddr2, 2, 0xEB, 0x03); //jmp 004CAA89
	}
	else {
		WriteMemory(removeSpamFilterAddr1, 2, 0x74, 0x27); //je 00490630
		WriteMemory(removeSpamFilterAddr2, 2, 0x73, 0x1D); //jae 00490670
		WriteMemory(infiniteChatboxAddr1, 2, 0x7E, 0x7E); //jle 004CAA89
		WriteMemory(infiniteChatboxAddr2, 2, 0x7E, 0x03); //jle 004CAA89
	}
}

//No Blue Boxes (CUtilDlg::Notice())
void MainForm::cbNoBlueBoxes_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoBlueBoxes->Checked)
		WriteMemory(noBlueBoxesAddr, 5, 0xC3, 0x90, 0x90, 0x90, 0x90); //ret; nop; nop; nop; nop;
	else
		WriteMemory(noBlueBoxesAddr, 5, 0xB8, 0x92, 0x21, 0xAE, 0x00); //mov eax,00AE2192
}

//No Walking Friction (TODO: find function this is in, its named sub_9B3FD1 in v83 idb)
void MainForm::cbNoWalkingFriction_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (this->cbNoWalkingFriction->Checked)
		WriteMemory(walkingFrictionAddr, 2, 0x75, 0x05); //jne 009B436C
	else
		WriteMemory(walkingFrictionAddr, 2, 0x74, 0x05); //je 009B436C	
}
#pragma endregion

#pragma region Hacks II Tab
#pragma region Teleport
void TeleportLoop() {
	int index = 0;
	while (GlobalRefs::bTeleport) {
		if (index < MainForm::TheInstance->lvTeleport->Items->Count) {
			Teleport(Convert::ToInt32(MainForm::TheInstance->lvTeleport->Items[index]->Text), Convert::ToInt32(MainForm::TheInstance->lvTeleport->Items[index]->SubItems[1]->Text));
			index++;
		}
		else
			index = 0;

		Sleep(Convert::ToUInt32(MainForm::TheInstance->tbTeleportLoopDelay->Text));
	}
	ExitThread(0);
}

void MainForm::bTeleportGetCurrentLocation_Click(Object^  sender, EventArgs^  e) {
	tbTeleportX->Text = PointerFuncs::getCharPosX();
	tbTeleportY->Text = PointerFuncs::getCharPosY();
}

void MainForm::bTeleportAdd_Click(Object^  sender, EventArgs^  e) {
	if (String::IsNullOrWhiteSpace(tbTeleportX->Text) || String::IsNullOrWhiteSpace(tbTeleportY->Text)) {
		MessageBox::Show("Error: The teleport x and y textboxes cannot be empty");
		return;
	}

	array<String^>^ row = {tbTeleportX->Text, tbTeleportY->Text};
	ListViewItem^ lvi = gcnew ListViewItem(row);
	lvTeleport->Items->Add(lvi);
}

void MainForm::bTeleportDelete_Click(Object^  sender, EventArgs^  e) {
	for each(ListViewItem^ lvi in lvTeleport->SelectedItems)
		lvTeleport->Items->Remove(lvi);
}

void MainForm::bTeleport_Click(Object^  sender, EventArgs^  e) {
	if (lvTeleport->Items->Count > 0 && lvTeleport->SelectedItems->Count != 0)
		Teleport(Convert::ToInt32(lvTeleport->SelectedItems[0]->Text), Convert::ToInt32(lvTeleport->SelectedItems[0]->SubItems[1]->Text));
}

void MainForm::lvTeleport_MouseDoubleClick(Object^  sender, Windows::Forms::MouseEventArgs^  e) {
	if (lvTeleport->Items->Count > 0 && lvTeleport->SelectedItems->Count != 0)
		Teleport(Convert::ToInt32(lvTeleport->SelectedItems[0]->Text), Convert::ToInt32(lvTeleport->SelectedItems[0]->SubItems[1]->Text));
}

void MainForm::bTeleportLoop_Click(Object^  sender, EventArgs^  e) {
	if (!bTeleportLoop->Text->Equals("Stop Loop")) {
		bTeleportLoop->Text = "Stop Loop";
		bTeleportGetCurrentLocation->Enabled = false;
		bTeleportAdd->Enabled = false;
		bTeleportDelete->Enabled = false;
		bTeleport->Enabled = false;
		lvTeleport->Enabled = false;
		GlobalRefs::bTeleport = true;
		NewThread(TeleportLoop);
	}
	else {
		bTeleportLoop->Text = "Loop";
		bTeleportGetCurrentLocation->Enabled = true;
		bTeleportAdd->Enabled = true;
		bTeleportDelete->Enabled = true;
		bTeleport->Enabled = true;
		lvTeleport->Enabled = true;
		GlobalRefs::bTeleport = false;
	}
}

void MainForm::tbTeleportX_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, true)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbTeleportY_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, true)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbTeleportLoopDelay_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
#pragma endregion

#pragma region Spawn Control
void MainForm::bSpawnControlGetCurrentLocation_Click(Object^  sender, EventArgs^  e) {
	tbSpawnControlX->Text = PointerFuncs::getCharPosX();
	tbSpawnControlY->Text = PointerFuncs::getCharPosY();
	tbSpawnControlMapID->Text = PointerFuncs::getMapID();
}

void MainForm::bSpawnControlAdd_Click(Object^  sender, EventArgs^  e) {
	if (String::IsNullOrWhiteSpace(tbSpawnControlMapID->Text) || String::IsNullOrWhiteSpace(tbSpawnControlX->Text) || String::IsNullOrWhiteSpace(tbSpawnControlY->Text)) {
		MessageBox::Show("Error: Spawn Control Map ID, x, and y textboxes cannot be empty");
		return;
	}
	
	if(Convert::ToUInt32(tbSpawnControlMapID->Text) == 0) {
		MessageBox::Show("Error: Map ID cannot be 0"); 
		return;
	}

	for each(ListViewItem^ lvi in lvSpawnControl->Items) {
		if (lvi->SubItems[0]->Text->Equals(tbSpawnControlMapID->Text)) {
			MessageBox::Show("Error: Two spawn points can not exist for the same map ID."); 
			return;
		}
	}

	array<String^>^ row = { tbSpawnControlMapID->Text, tbSpawnControlX->Text, tbSpawnControlY->Text };
	ListViewItem^ lvi = gcnew ListViewItem(row);
	lvSpawnControl->Items->Add(lvi);

	SpawnControlData* sps = new SpawnControlData(Convert::ToUInt32(tbSpawnControlMapID->Text), Convert::ToInt32(tbSpawnControlX->Text), Convert::ToInt32(tbSpawnControlY->Text));
	Assembly::spawnControl->push_back(sps);
}

void MainForm::bSpawnControlDelete_Click(Object^  sender, EventArgs^  e) {
	for each(ListViewItem^ lvi in lvSpawnControl->SelectedItems) {
		for (std::vector<SpawnControlData*>::const_iterator i = Assembly::spawnControl->begin(); i != Assembly::spawnControl->end(); ++i) {
			if (Convert::ToString((*i)->mapID)->Equals(lvi->SubItems[0]->Text)) {
				Assembly::spawnControl->erase(i);
				break;
			}
		}
		lvSpawnControl->Items->Remove(lvi);
	}
}

void MainForm::bSpawnControl_Click(Object^  sender, EventArgs^  e) {
	if (!bSpawnControl->Text->Equals("Disable Spawn Control")) { //Enabled
		bSpawnControl->Text = "Disable Spawn Control";
		bSpawnControlGetCurrentLocation->Enabled = false;
		bSpawnControlAdd->Enabled = false;
		bSpawnControlDelete->Enabled = false;
		lvSpawnControl->Enabled = false;
		Jump(spawnPointAddr, Assembly::SpawnPointHook, 0);
	}
	else {
		bSpawnControl->Text = "Enable Spawn Control";
		bSpawnControlGetCurrentLocation->Enabled = true;
		bSpawnControlAdd->Enabled = true;
		bSpawnControlDelete->Enabled = true;
		lvSpawnControl->Enabled = true;
		WriteMemory(spawnPointAddr, 5, 0xB8, 0xF4, 0x56, 0xAE, 0x00);
	}
}

void MainForm::tbSpawnControlMapID_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbSpawnControlX_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, true)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbSpawnControlY_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, true)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
#pragma endregion
#pragma endregion

#pragma region Vacs Tab
//TODO: Every hack has a "get current location" to get x,y. Reuse that shit
//TODO: calls to kami teleport cause crash when character drops to floor(slow call of teleport interval, however fast calling crashes of itself) while vac right/left is active, prolly too many concurent calls or something overflows
#pragma region Kami
void KamiLoop() {
	while (GlobalRefs::bKami || GlobalRefs::bKamiLoot) {
		if (GlobalRefs::bKami && GlobalRefs::bKamiLoot) {
			if(ReadPointer(DropPoolBase, OFS_ItemCount) > Convert::ToUInt32(MainForm::TheInstance->tbKamiLootItem->Text)) {
				if (!GlobalRefs::isChangingField && !GlobalRefs::isMapRushing) {}
					Teleport(Assembly::ItemX, Assembly::ItemY + 10);
				Sleep(Convert::ToUInt32(MainForm::TheInstance->tbKamiLootInterval->Text));
			}
			else if (ReadPointer(MobPoolBase, OFS_MobCount) > Convert::ToUInt32(MainForm::TheInstance->tbKamiMob->Text)) {
				POINT telePoint;
				telePoint.x = ReadMultiPointerSigned(MobPoolBase, 5, OFS_Mob1, OFS_Mob2, OFS_Mob3, OFS_Mob4, OFS_MobX) - Convert::ToInt32(MainForm::TheInstance->tbKamiX->Text);
				telePoint.y = ReadMultiPointerSigned(MobPoolBase, 5, OFS_Mob1, OFS_Mob2, OFS_Mob3, OFS_Mob4, OFS_MobY) - Convert::ToInt32(MainForm::TheInstance->tbKamiY->Text);

				if (!GlobalRefs::isChangingField && !GlobalRefs::isMapRushing)
					Teleport(telePoint);

				Sleep(Convert::ToUInt32(MainForm::TheInstance->tbKamiInterval->Text));
			}
		}
		else if (GlobalRefs::bKami) {
			if(ReadPointer(MobPoolBase, OFS_MobCount) > Convert::ToUInt32(MainForm::TheInstance->tbKamiMob->Text)) {
				POINT telePoint;
				telePoint.x = ReadMultiPointerSigned(MobPoolBase, 5, OFS_Mob1, OFS_Mob2, OFS_Mob3, OFS_Mob4, OFS_MobX) - Convert::ToInt32(MainForm::TheInstance->tbKamiX->Text);
				telePoint.y = ReadMultiPointerSigned(MobPoolBase, 5, OFS_Mob1, OFS_Mob2, OFS_Mob3, OFS_Mob4, OFS_MobY) - Convert::ToInt32(MainForm::TheInstance->tbKamiY->Text);

				if (!GlobalRefs::isChangingField && !GlobalRefs::isMapRushing)
					Teleport(telePoint);
			}
			Sleep(Convert::ToUInt32(MainForm::TheInstance->tbKamiInterval->Text));
		}
		else if (GlobalRefs::bKamiLoot) {
			if (ReadPointer(DropPoolBase, OFS_ItemCount) > Convert::ToUInt32(MainForm::TheInstance->tbKamiLootItem->Text)) {
				if (!GlobalRefs::isChangingField && !GlobalRefs::isMapRushing) {}
					Teleport(Assembly::ItemX, Assembly::ItemY+10); //MessageBox::Show("ItemX: " + Assembly::ItemX.ToString() + " ItemY: " + Assembly::ItemY.ToString());
			}
			Sleep(Convert::ToUInt32(MainForm::TheInstance->tbKamiLootInterval->Text));
		}
	}
	ExitThread(0);
}

void MainForm::cbKami_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if(this->cbKami->Checked) {
		tbKamiX->Enabled = false;
		tbKamiY->Enabled = false;
		tbKamiInterval->Enabled = false;
		tbKamiMob->Enabled = false;
		GlobalRefs::bKami = true;
		if(!GlobalRefs::bKamiLoot)
			NewThread(KamiLoop);
	}
	else {
		GlobalRefs::bKami = false;
		tbKamiX->Enabled = true;
		tbKamiY->Enabled = true;
		tbKamiInterval->Enabled = true;
		tbKamiMob->Enabled = true;
	}
}

void MainForm::cbKamiLoot_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (this->cbKamiLoot->Checked) {
		tbKamiLootInterval->Enabled = false;
		tbKamiLootItem->Enabled = false;
		GlobalRefs::bKamiLoot = true;
		cbLoot->Checked = true; //Enable Auto Loot (Required for call to PtInRect)
		*(ULONG*)PtInRectAddr = (ULONG)Assembly::ItemHook;
		if (!GlobalRefs::bKami)
			NewThread(KamiLoop);
	}
	else {
		GlobalRefs::bKamiLoot = false;
		cbLoot->Checked = false; //Disable Auto Loot 
		*(ULONG*)PtInRectAddr = (ULONG)PtInRect;
		tbKamiLootInterval->Enabled = true;
		tbKamiLootItem->Enabled = true;
	}
}

void MainForm::tbKamiX_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, true)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbKamiY_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, true)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbKamiInterval_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbKamiMob_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbKamiLootInterval_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbKamiLootItem_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
#pragma endregion

#pragma region Wall Vac
//TODO: Textbox error checking
void WallVacLoop() {
	int vacXPos = 0, vacYPos = 0, rangeX = 15, rangeY = 0;
	vacXPos = Convert::ToInt32(MainForm::TheInstance->tbWallVacX->Text);
	vacYPos =  Convert::ToInt32(MainForm::TheInstance->tbWallVacY->Text);
	rangeX = Convert::ToInt32(MainForm::TheInstance->tbWallVacRangeX->Text);
	rangeY = Convert::ToInt32(MainForm::TheInstance->tbWallVacRangeY->Text);

	while (GlobalRefs::bWallVac) {
		//Set X Walls
		WritePointer(CWvsPhysicalSpace2DBase, OFS_LeftWall, vacXPos - rangeX);
		Sleep(50);
		WritePointer(CWvsPhysicalSpace2DBase, OFS_RightWall, vacXPos + rangeX);
		Sleep(50);

		//Set Y Walls
		WritePointer(CWvsPhysicalSpace2DBase, OFS_TopWall, vacYPos - rangeY);
		Sleep(50);
		WritePointer(CWvsPhysicalSpace2DBase, OFS_BottomWall, vacYPos + rangeY);
		Sleep(50);

		Sleep(500); //Every half a second, re-write pointer to set wall values TODO: Allow users to enter a delay
	}
	ExitThread(0);
}

void MainForm::cbWallVac_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if(this->cbWallVac->Checked) {
		tbWallVacX->Enabled = false;
		tbWallVacY->Enabled = false;
		tbWallVacRangeX->Enabled = false;
		tbWallVacRangeY->Enabled = false;
		GlobalRefs::bWallVac = true;
		NewThread(WallVacLoop);
	}
	else {
		tbWallVacX->Enabled = true;
		tbWallVacY->Enabled = true;
		tbWallVacRangeX->Enabled = true;
		tbWallVacRangeY->Enabled = true;
		GlobalRefs::bWallVac = false;
	}
}

void MainForm::bWallVacGetCurrentLocation_Click(System::Object^  sender, System::EventArgs^  e) {
	tbWallVacX->Text = PointerFuncs::getCharPosX();
	tbWallVacY->Text = PointerFuncs::getCharPosY();
}

void MainForm::tbWallVacX_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, true)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbWallVacY_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, true)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbWallVacRangeX_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, true)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbWallVacRangeY_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, true)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
#pragma endregion

#pragma region DupeX
/* Old code from old trainer, need to refactor/make better

DWORD dupeX = 0x009B495D; //89 BE 14 01 00 00 89 86 18 01 00 00 39 41 04 74
DWORD dupeXRet = dupeX+6;
DWORD dupeXPlat = 0;
DWORD dupeXRunFlag = 0;

void __declspec(naked) DupeXAsm() {
	__asm {
		pushfd
		push eax
		push ecx
		mov eax,[0x00BEBF98] //CharBase
		mov eax, [eax]
		test eax,eax
		je NullPlat
		mov eax,[eax+0x000011A4] //pID Offset
		lea ecx,[eax-0x0c] //account id offset?
		test ecx,ecx
		je NullPlat
		mov eax,[ecx+0x00000114] //kb offset?
		test eax,eax
		je NullPlat
		cmp dword ptr [dupeXRunFlag],0x01
		je DoVac
		mov dword ptr [dupeXPlat],eax
		inc dword ptr [dupeXRunFlag]
		jmp DoVac

		DoVac:
		cmp esi,ecx
		je Normal
		mov edi, [dupeXPlat]
		jmp Normal

		NullPlat:
		mov dword ptr [dupeXPlat],0x00
		mov dword ptr [dupeXRunFlag],0x00
		jmp Normal

		Normal:
		pop ecx
		pop eax
		popfd
		mov [esi+0x00000114],edi
		jmp dword ptr[dupeXRet]
	}
}

Jump(dupeX, DupeXAsm, 1);
WriteMemory(0x009B495D, 6, 0x89, 0xBE, 0x14, 0x01, 0x00, 0x00);

Reset Click Event: 
dupeXRunFlag = 0;

 */

void MainForm::cbDupeX_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (this->cbDupeX->Checked) {
		tbDupeXFoothold->Enabled = false;
		bDupeXGetFoothold->Enabled = false;
		MessageBox::Show("Warning: Bans");
		Jump(dupeXAddr, Assembly::DupeXHook, 1);
	} 
	else {
		tbDupeXFoothold->Enabled = true;
		bDupeXGetFoothold->Enabled = true;
		WriteMemory(dupeXAddr, 6, 0x89, 0xBE, 0x14, 0x01, 0x00, 0x00);
	}
}

void MainForm::bDupeXGetFoothold_Click(System::Object^  sender, System::EventArgs^  e) {
	tbDupeXFoothold->Text = Convert::ToString(ReadMultiPointerSigned(UserLocalBase, 2, OFS_pID, OFS_Foothold)); 
}

void MainForm::tbDupeXFoothold_TextChanged(System::Object^  sender, System::EventArgs^  e) {
	if (tbDupeXFoothold->Text != "") Assembly::dupeXFoothold = Convert::ToInt32(tbDupeXFoothold->Text); 
}

void MainForm::tbDupeXFoothold_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, true)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
#pragma endregion

#pragma region MMC
void MainForm::tbMMCX_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, true)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbMMCY_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, true)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
#pragma endregion

#pragma region uEMI
//TODO: Textbox error checking
void UEMILoop() {
	int uEMIXPos = 0, uEMIYPos = 0;
	uEMIXPos = Convert::ToInt32(MainForm::TheInstance->tbUEMIx->Text);
	uEMIYPos = Convert::ToInt32(MainForm::TheInstance->tbUEMIy->Text);

	while (GlobalRefs::bUEMI) {
		int *mobXPos = (int*)(*(ULONG*)((*(ULONG*)((*(ULONG*)(0xBEBFA4)) + 0x28)) + 0x4) + 0x510);
		int *mobYPos = (int*)(*(ULONG*)((*(ULONG*)((*(ULONG*)(0xBEBFA4)) + 0x28)) + 0x4) + 0x514);

		*mobXPos = uEMIXPos;
		*mobYPos = uEMIYPos;
		Sleep(50); //TODO: Allow users to enter a delay
	}
	ExitThread(0);
}


void MainForm::cbUEMI_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if(this->cbUEMI->Checked) {
		tbUEMIx->Enabled = false;
		tbUEMIy->Enabled = false;
		GlobalRefs::bUEMI = true;
		NewThread(UEMILoop);
	}
	else {
		tbUEMIx->Enabled = true;
		tbUEMIy->Enabled = true;
		GlobalRefs::bUEMI = false;
	}
}

void MainForm::bUEMIGetCurrentLocation_Click(System::Object^  sender, System::EventArgs^  e) {
	tbUEMIx->Text = PointerFuncs::getCharPosX();
	tbUEMIy->Text = PointerFuncs::getCharPosY();
}
#pragma endregion

//Full Map Attack (CMobPool::FindHitMobInRect())
void MainForm::cbFullMapAttack_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbFullMapAttack->Checked)
		WriteMemory(fullMapAttackAddr, 2, 0x90, 0x90); //nop; nop;
	else
		WriteMemory(fullMapAttackAddr, 2, 0x74, 0x22); //je 006785EE
}

//ZZ Vac
//TODO: Is it possible to change the coordinates from (0,0) to custom coordinates?
//TODO: Find func in idb using pdb
void MainForm::cbZzVac_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbZzVac->Checked)
	{
		WriteMemory(zzVacAddr1, 3, 0x90, 0x90, 0x90); //nop; nop; nop;
		WriteMemory(zzVacAddr2, 3, 0x90, 0x90, 0x90); //nop; nop; nop;			
	}
	else
	{
		WriteMemory(zzVacAddr1, 3, 0xDD, 0x45, 0xF0); //fld qword ptr [ebp-10]
		WriteMemory(zzVacAddr2, 3, 0xDD, 0x45, 0xE8); //fld qword ptr [ebp-18]		
	}
}

//TODO: Find func in idb using pdb
void MainForm::cbVacForceRight_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (this->cbVacForceRight->Checked)
		WriteMemory(vacForceRightAddr, 2, 0x76, 0x18); //jna
	else
		WriteMemory(vacForceRightAddr, 2, 0x73, 0x18); //jae
}

//TODO: Find func in idb using pdb
void MainForm::cbVacRight_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (this->cbVacRight->Checked)
		WriteMemory(vacRightAddr, 2, 0x0F, 0x84); //je	
	else
		WriteMemory(vacRightAddr, 2, 0x0F, 0x86); //jbe
}

//TODO: Find func in idb using pdb
void MainForm::cbVacLeft_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (this->cbVacLeft->Checked)
		WriteMemory(vacLeftAddr, 2, 0x74, 0x66); //je
	else
		WriteMemory(vacLeftAddr, 2, 0x73, 0x66); //jae
}

//TODO: Find func in idb using pdb
void MainForm::cbVacJumpRight_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (this->cbVacJumpRight->Checked)
		WriteMemory(vacJumpRightAddr, 2, 0x74, 0x72); //je
	else
		WriteMemory(vacJumpRightAddr, 2, 0x76, 0x72); //jna
}

//TODO: Find func in idb using pdb
void MainForm::cbVacJumpLeft_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (this->cbVacJumpLeft->Checked)
		WriteMemory(vacJumpLeftAddr, 2, 0x74, 0x66); //je
	else
		WriteMemory(vacJumpLeftAddr, 2, 0x73, 0x66); //jae
}
#pragma endregion

#pragma region Filters Tab
#pragma region ItemFilter
//Find items in ItemsList resource with names starting with string passed in
static void findItemsStartingWithStr(String^ str) {
	try {
		if (String::IsNullOrEmpty(str)) return;
		std::string tmpStr = "", itemID = "", itemStr = ConvertSystemToStdStr(str); //TODO
		HRSRC hRes = FindResource(GlobalVars::hDLL, MAKEINTRESOURCE(ItemsList), _T("TEXT"));
		if (hRes == nullptr) return;
		HGLOBAL hGlob = LoadResource(GlobalVars::hDLL, hRes);
		if (hGlob == nullptr) return;
		const CHAR* pData = reinterpret_cast<const CHAR*>(::LockResource(hGlob));
		std::istringstream File(pData);

		while (File.good()) {
			std::getline(File, tmpStr);
			itemID = tmpStr.substr(0, tmpStr.find(' '));
			tmpStr = tmpStr.substr(tmpStr.find('[') + 1, tmpStr.find(']'));
			tmpStr = tmpStr.substr(0, tmpStr.length() - 2);

			if (tmpStr.find(itemStr) != std::string::npos) {
				std::string tmpLine = tmpStr + " (" + itemID.c_str() + ")";
				String^ result = gcnew String(tmpLine.c_str());
				if (!MainForm::TheInstance->lbItemSearchLog->Items->Contains(result))
					MainForm::TheInstance->lbItemSearchLog->Items->Add(result);
			}
		}
		UnlockResource(hRes);
	}
	catch (...) {}
}

//Enable Item Filter
void MainForm::bItemFilter_Click(System::Object^  sender, System::EventArgs^  e) {
	if(bItemFilter->Text->Equals("Enable Item Filter")) {
		bItemFilter->Text = "Disable Item Filter";
		if (Convert::ToUInt32(tbItemFilterMesos->Text) > 50000) MessageBox::Show("Please enter mesos value ranging from 0 to 50,000. Default: 0");

		Assembly::isItemFilterEnabled = 1;
		if (Assembly::isItemLoggingEnabled == 0)
			Jump(itemFilterAddr, Assembly::ItemFilterHook, 1);
	}
	else {
		bItemFilter->Text = "Enable Item Filter";
		Assembly::isItemFilterEnabled = 0;
		if(Assembly::isItemLoggingEnabled == 0)
			WriteMemory(itemFilterAddr, 6, 0x89, 0x47, 0x34, 0x8B, 0x7D, 0xEC); //mov [edi+34],eax; mov edi,[ebp-14];
	}
}

//Change Item Filter type (either White List or Black List)
void MainForm::rbItemFilterWhiteList_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (rbItemFilterWhiteList->Checked)
		Assembly::isItemFilterWhiteList = 1;
	else
		Assembly::isItemFilterWhiteList = 0;
}

//Enable Item Filter Log
void MainForm::cbItemFilterLog_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if(cbItemFilterLog->Checked) {
		Assembly::isItemLoggingEnabled = 1;
		if (Assembly::isItemFilterEnabled == 0)
			Jump(itemFilterAddr, Assembly::ItemFilterHook, 1);
	}
	else {
		Assembly::isItemLoggingEnabled = 0;
		if (Assembly::isItemFilterEnabled == 0)
			WriteMemory(itemFilterAddr, 6, 0x89, 0x47, 0x34, 0x8B, 0x7D, 0xEC); //mov [edi+34],eax; mov edi,[ebp-14];
	}
}

//Add item to Item Filter ListBox
void MainForm::bItemFilterAdd_Click(System::Object^  sender, System::EventArgs^  e) {
	if(tbItemFilterID->TextLength > 0) {
		try {
			UINT itemID = Convert::ToUInt32(tbItemFilterID->Text);
			String^ item = Assembly::findItemNameFromID(itemID);

			if(itemID > 0 && !lbItemFilter->Items->Contains(item + " (" + itemID.ToString() + ")")) {
				lbItemFilter->Items->Add(item + " (" + itemID.ToString() + ")");
				tbItemFilterID->Text = "";
				lbItemFilter->SelectedIndex = -1;
				Assembly::itemList->push_back(itemID);
			}
		}
		catch (...) { MessageBox::Show("Item ID not found"); }
	}
}

//Delete item from Item Filter ListBox
void MainForm::lbItemFilter_MouseDoubleClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
	if(lbItemFilter->SelectedItem != nullptr) {
		String ^itemStr = lbItemFilter->GetItemText(lbItemFilter->SelectedItem);
		int startIndex = itemStr->IndexOf('(') + 1, endIndex = itemStr->IndexOf(')');

		String^ itemIDStr = itemStr->Substring(startIndex, endIndex - startIndex);
		Assembly::itemList->erase(std::find(Assembly::itemList->begin(), Assembly::itemList->end(), Convert::ToUInt32(itemIDStr)));

		lbItemFilter->Items->Remove(lbItemFilter->SelectedItem);
		lbItemFilter->SelectedIndex = -1;
	}
}

//Transfer item from Item Search Log ListBox to Item Filter ListBox
void MainForm::lbItemSearchLog_MouseDoubleClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
	if(lbItemSearchLog->SelectedItem != nullptr && lbItemSearchLog->SelectedItem->ToString()->Length > 0) {
		String ^itemStr = lbItemSearchLog->GetItemText(lbItemSearchLog->SelectedItem);
		int startIndex = itemStr->IndexOf('(')+1, endIndex = itemStr->IndexOf(')');

		String^ itemIDStr = itemStr->Substring(startIndex, endIndex - startIndex);
		Assembly::itemList->push_back(Convert::ToUInt32(itemIDStr));

		lbItemFilter->Items->Add(lbItemSearchLog->SelectedItem);
		lbItemSearchLog->SelectedIndex = -1;
		lbItemFilter->SelectedIndex = -1;
	}
}

//Clear items in Item Search Log
void MainForm::bItemSearchLogClear_Click(System::Object^  sender, System::EventArgs^  e) {
	lbItemSearchLog->Items->Clear();
}

//Find items in ItemsList resource with names starting with text entered so far
void MainForm::tbItemFilterSearch_TextChanged(System::Object^  sender, System::EventArgs^  e) {
	lbItemSearchLog->Items->Clear();
	findItemsStartingWithStr(tbItemFilterSearch->Text);
}

//Changes limit for Mesos (Range: 0<=limit<=50000)
void MainForm::tbItemFilterMesos_TextChanged(System::Object^  sender, System::EventArgs^  e) {
	if (!String::IsNullOrEmpty(tbItemFilterMesos->Text)) {
		ULONG mesosLimit = Convert::ToUInt32(tbItemFilterMesos->Text);
		if (mesosLimit >= 0 && mesosLimit <= 50000)
			Assembly::itemFilterMesos = mesosLimit;
	}
}

void MainForm::tbItemFilterID_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbItemFilterMesos_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
#pragma endregion

#pragma region MobFilter
//Find mobs in MobsList resource with names starting with string passed in
static void findMobsStartingWithStr(String^ str) {
	try {
		if (String::IsNullOrEmpty(str)) return;
		std::string tmpStr = "", mobID = "", mobStr = ConvertSystemToStdStr(str); //TODO
		HRSRC hRes = FindResource(GlobalVars::hDLL, MAKEINTRESOURCE(MobsList), _T("TEXT"));
		if (hRes == nullptr) return;
		HGLOBAL hGlob = LoadResource(GlobalVars::hDLL, hRes);
		if (hGlob == nullptr) return;
		const CHAR* pData = reinterpret_cast<const CHAR*>(::LockResource(hGlob));
		std::istringstream File(pData);

		while (File.good()) {
			std::getline(File, tmpStr);
			mobID = tmpStr.substr(0, tmpStr.find(' '));
			tmpStr = tmpStr.substr(tmpStr.find('[') + 1, tmpStr.find(']'));
			tmpStr = tmpStr.substr(0, tmpStr.length() - 2);

			if (tmpStr.find(mobStr) != std::string::npos) {
				std::string tmpLine = tmpStr + " (" + mobID.c_str() + ")";
				String^ result = gcnew String(tmpLine.c_str());
				if (!MainForm::TheInstance->lbMobSearchLog->Items->Contains(result))
					MainForm::TheInstance->lbMobSearchLog->Items->Add(result);
			}
		}
		UnlockResource(hRes);
	}
	catch (...) {}
}

//Enable Mob Filter
void MainForm::bMobFilter_Click(System::Object^  sender, System::EventArgs^  e) {
	if (bMobFilter->Text->Equals("Enable Mob Filter")) {
		bMobFilter->Text = "Disable Mob Filter";
		Assembly::isMobFilterEnabled = 1;
		if (Assembly::isMobLoggingEnabled == 0) {
			Jump(mobFilter1Addr, Assembly::MobFilter1Hook, 0);
			Jump(mobFilter2Addr, Assembly::MobFilter2Hook, 0);
		}
	}
	else {
		bMobFilter->Text = "Enable Mob Filter";
		Assembly::isMobFilterEnabled = 0;
		if (Assembly::isMobLoggingEnabled == 0) {
			WriteMemory(mobFilter1Addr, 5, 0xE8, 0xF7, 0xE2, 0xD8, 0xFF); //call 00406629
			WriteMemory(mobFilter2Addr, 5, 0xE8, 0x98, 0xD1, 0xD8, 0xFF); //call 00406629
		}
	}
}

//Change Mob Filter type (either White List or Black List)
void MainForm::rbMobFilterWhiteList_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (rbMobFilterWhiteList->Checked)
		Assembly::isMobFilterWhiteList = 1;
	else
		Assembly::isMobFilterWhiteList = 0;
}

//Enable Mob Filter Log
void MainForm::cbMobFilterLog_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (cbMobFilterLog->Checked) {
		Assembly::isMobLoggingEnabled = 1;
		if (Assembly::isMobFilterEnabled == 0) {
			Jump(mobFilter1Addr, Assembly::MobFilter1Hook, 0);
			Jump(mobFilter2Addr, Assembly::MobFilter2Hook, 0);
		}
	}
	else {
		Assembly::isMobLoggingEnabled = 0;
		if (Assembly::isMobFilterEnabled == 0) {
			WriteMemory(mobFilter1Addr, 5, 0xE8, 0xF7, 0xE2, 0xD8, 0xFF); //call 00406629
			WriteMemory(mobFilter2Addr, 5, 0xE8, 0x98, 0xD1, 0xD8, 0xFF); //call 00406629
		}
	}
}

//Add mob to Mob Filter ListBox
void MainForm::bMobFilterAdd_Click(System::Object^  sender, System::EventArgs^  e) {
	if (tbMobFilterID->TextLength > 0) {
		try {
			UINT mobID = Convert::ToUInt32(tbMobFilterID->Text);
			String^ mob = Assembly::findMobNameFromID(mobID);

			if (mobID > 0 && !lbMobFilter->Items->Contains(mob + " (" + mobID.ToString() + ")")) {
				lbMobFilter->Items->Add(mob + " (" + mobID.ToString() + ")");
				tbMobFilterID->Text = "";
				lbMobFilter->SelectedIndex = -1;
				Assembly::mobList->push_back(mobID);
			}
		}
		catch (...) { MessageBox::Show("Mob ID not found"); }
	}
}

//Delete mob from Mob Filter ListBox
void MainForm::lbMobFilter_MouseDoubleClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
	if (lbMobFilter->SelectedItem != nullptr) {
		String ^mobStr = lbMobFilter->GetItemText(lbMobFilter->SelectedItem);
		int startIndex = mobStr->IndexOf('(') + 1, endIndex = mobStr->IndexOf(')');

		String^ mobIDStr = mobStr->Substring(startIndex, endIndex - startIndex);
		Assembly::mobList->erase(std::find(Assembly::mobList->begin(), Assembly::mobList->end(), Convert::ToUInt32(mobIDStr)));

		lbMobFilter->Items->Remove(lbMobFilter->SelectedItem);
		lbMobFilter->SelectedIndex = -1;
	}
}

//Transfer mob from Mob Search Log ListBox to Mob Filter ListBox
void MainForm::lbMobSearchLog_MouseDoubleClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
	if (lbMobSearchLog->SelectedItem != nullptr && lbMobSearchLog->SelectedItem->ToString()->Length > 0) {
		String ^mobStr = lbMobSearchLog->GetItemText(lbMobSearchLog->SelectedItem);
		int startIndex = mobStr->IndexOf('(') + 1, endIndex = mobStr->IndexOf(')');

		String^ mobIDStr = mobStr->Substring(startIndex, endIndex - startIndex);
		Assembly::mobList->push_back(Convert::ToUInt32(mobIDStr));

		lbMobFilter->Items->Add(lbMobSearchLog->SelectedItem);
		lbMobSearchLog->SelectedIndex = -1;
		lbMobFilter->SelectedIndex = -1;
	}
}

//Clear mobs in Mob Search Log
void MainForm::bMobSearchLogClear_Click(System::Object^  sender, System::EventArgs^  e) {
	lbMobSearchLog->Items->Clear();
}

//Searches MobList resource for mobs starting with text entered so far
void MainForm::tbMobFilterSearch_TextChanged(System::Object^  sender, System::EventArgs^  e) {
	lbMobSearchLog->Items->Clear();
	findMobsStartingWithStr(tbMobFilterSearch->Text);
}

void MainForm::tbMobFilterID_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
#pragma endregion
#pragma endregion

#pragma region Packets Tab
#pragma region Send Packets
void MainForm::bSendPacket_Click(Object^  sender, EventArgs^  e) {
	SendPacket(tbSendPacket->Text);
	//SendPacket(gcnew String("28 00 ** ** ** 00"));
}

void MainForm::tbSendSpamDelay_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::bSendLog_Click(System::Object^  sender, System::EventArgs^  e) {
	if(bSendLog->Text->Equals("Enable Log")) {
		bSendLog->Text = "Disable Log";
		GlobalRefs::bSendPacketLog = true;
		this->tPacketLog->Enabled = true;
		Jump(cOutPacketAddr, Assembly::SendPacketLogHook, 0);
	}
	else {
		bSendLog->Text = "Enable Log";
		GlobalRefs::bSendPacketLog = false;
		WriteMemory(cOutPacketAddr, 5, 0xB8, 0x6C, 0x12, 0xA8, 0x00);
	}
}
#pragma endregion

#pragma region Receive Packets
void MainForm::bRecvPacket_Click(Object^  sender, EventArgs^  e) {
	RecvPacket(tbRecvPacket->Text);
}
#pragma endregion

#pragma region Predefined Packets
int scrollId = 2030000;  // L"Nearest"
int useSlotG = 00;

void MainForm::comboToTown_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
	const int townStr = comboToTown->SelectedIndex;
	//int scrollId = 0;

	switch (townStr) {
	case 1: // L"Nearest"
		scrollId = 2030000;
		break;
	case 2: //L"LithHarbor"
		scrollId = 2030001;
		break;
	case 3: //L"Ellinia"
		scrollId = 2030002;
		break; 
	case 4: //L"Perion"
		scrollId = 2030003;
		break;
	case 5: //L"Henesys"	
		scrollId = 2030004;
		break;
	case 6: //L"KerningCity"
		scrollId = 2030005;
		break;
	case 7: //L"Sleepywood"
		scrollId = 2030006;
		break;
	case 8: //L"DeadMine"
		scrollId = 2030007;
		break;
	default: 
		Log::WriteLineToConsole("comboToTown:: ERROR unknown town selected!");
	}	
}

void MainForm::comboInUseSlot_TextChanged(System::Object^  sender, System::EventArgs^  e) {
	if (String::IsNullOrWhiteSpace(comboInUseSlot->Text)) return;
	const int useSlot = Convert::ToInt32(comboInUseSlot->Text);
	useSlotG = useSlot;
}

String^ CreateRtrnScrollPacket(int scrollId, int useSlot) {
	String^ rtrnPacket = "";
	String^ slotStr;
	if (useSlotG < 99) slotStr = useSlotG.ToString();		
	else slotStr = useSlotG.ToString("X");
 
	// TODO: IDtoHex then reverse order // B0 F9 1E = 2030000
	switch (scrollId) {
	case 2030000: // L"Nearest"
		rtrnPacket = "55 00 * * * 3A XX 00 B0 F9 1E 00"; 
		break;
	case 2030001: //L"LithHarbor"
		break;
	case 2030002: //L"Ellinia"
		break;
	case 2030003: //L"Perion"
		break;
	case 2030004: //L"Henesys"	
		break;
	case 2030005: //L"KerningCity"
		break;
	case 2030006: //L"Sleepywood"
		break;
	case 2030007: //L"DeadMine"
		break;
	default:
		Log::WriteLineToConsole("CreateRtrnScrollPacket:: ERROR unknown scrollId!");
	}
	rtrnPacket->Replace("XX", slotStr);
	Log::WriteLineToConsole("Sending RTRN packet: " + rtrnPacket);

	return rtrnPacket;
}

void MainForm::bUseRtrnScroll_Click(System::Object^  sender, System::EventArgs^  e) {	
	SendPacket(CreateRtrnScrollPacket(scrollId, useSlotG));
}

void MainForm::bSendSuicide_Click(System::Object^  sender, System::EventArgs^  e) {
	SendPacket("30 00 72 76 9D 00 FD 00 00 BB 00 00 00 00 00");
}

void MainForm::bSendRevive_Click(System::Object^  sender, System::EventArgs^  e) {
	SendPacket("26 00 00 00 00 00 00 00 00 00 00 00");
}

void MainForm::bSendMount_Click(System::Object^  sender, System::EventArgs^  e) {
	SendPacket("5B 00 C8 7D 91 0A EC 03 00 00 01 00 00");
}

void MainForm::bSendDrop10_Click(System::Object^  sender, System::EventArgs^  e) {
	SendPacket("5E 00 FF 20 C0 03 0A 00 00 00");
}

void MainForm::bSendDrop1000_Click(System::Object^  sender, System::EventArgs^  e) {
	SendPacket("5E 00 FF 20 C0 03 E8 03 00 00");
}

void MainForm::bSendDrop10000_Click(System::Object^  sender, System::EventArgs^  e) {
	SendPacket("5E 00 FF 20 C0 03 10 27 00 00");
}

void MainForm::bSendDrop50000_Click(System::Object^  sender, System::EventArgs^  e) {
	SendPacket("5E 00 FF 20 C0 03 50 C3 00 00");
}

void MainForm::bSendRestore127Health_Click(System::Object^  sender, System::EventArgs^  e) {
	SendPacket("59 00 A1 7F F7 08 00 14 00 00 7F 00 00 00 00");
}
#pragma endregion

#pragma region AutoAP
void MainForm::tbAPLevel_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
void MainForm::tbAPLevel_TextChanged(System::Object^ sender, System::EventArgs^ e){
}

void MainForm::tbAPHP_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
void MainForm::tbAPHP_TextChanged(System::Object^ sender, System::EventArgs^ e) {
}

void MainForm::tbAPMP_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
void MainForm::tbAPMP_TextChanged(System::Object^ sender, System::EventArgs^ e) {
}

void MainForm::tbAPSTR_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
void MainForm::tbAPSTR_TextChanged(System::Object^ sender, System::EventArgs^ e) {
}

void MainForm::tbAPDEX_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
void MainForm::tbAPDEX_TextChanged(System::Object^ sender, System::EventArgs^ e) {
}

void MainForm::tbAPINT_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
void MainForm::tbAPINT_TextChanged(System::Object^ sender, System::EventArgs^ e) {
}

void MainForm::tbAPLUK_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
void MainForm::tbAPLUK_TextChanged(System::Object^ sender, System::EventArgs^ e) {
}

System::Void MainForm::cbAP_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
}
#pragma endregion

void MainForm::tPacketLog_Tick(System::Object^  sender, System::EventArgs^  e) {
	if (!GlobalRefs::bSendPacketLog && !GlobalRefs::bRecvPacketLog)
		this->tPacketLog->Enabled = false;

	//std::vector<COutPacket*> *sentPacketQueue = Assembly::sendPacketLogQueue;
	if(GlobalRefs::bSendPacketLog && !Assembly::sendPacketQueue->empty()) {
		COutPacket *packet = (COutPacket*)Assembly::sendPacketQueue->front();
		
		String^ packetHeader = "";
		writeUnsignedShort(packetHeader, *packet->Header);
		Log::WriteLineToConsole(packetHeader);
		
		Assembly::sendPacketQueue->pop();
	}

	/*if(!GlobalRefs::bSendPacketLog) {
		for (std::vector<COutPacket>::const_iterator i = Assembly::sendPacketLogQueue->begin(); i != Assembly::sendPacketLogQueue->end(); ++i) {
			COutPacket packet = *i;

			String^ packetHeader = "";
			writeUnsignedShort(packetHeader, *packet.Header);
			Log::WriteLineToConsole(packetHeader);

			/*String^ packetData = "";
			BYTE* packetBytes = packet->Data;
			int packetSize = packet->Size - 2;
			for (int i = 0; i < packetSize; i++)
				writeByte(packetData, packetBytes[i]);#1#


			/*Windows::Forms::TreeNode^ headerNode = gcnew Windows::Forms::TreeNode(packetHeader);
			headerNode->Name = packetHeader;

			Windows::Forms::TreeNode^ packetNode = gcnew Windows::Forms::TreeNode(packetData);
			packetNode->Name = packetData;

			Timelapse::MainForm::TheInstance->tvSendPackets->Nodes->Add(headerNode);#1#

			/*if(Timelapse::MainForm::TheInstance->tvSendPackets->Nodes->ContainsKey(packetHeader)) {
				//Header exists in tree
			}
			else {
				Windows::Forms::TreeNode^ headerNode = gcnew Windows::Forms::TreeNode(packetHeader);
				headerNode->Name = packetHeader;
				//headerNode->Nodes->Add(packetNode);
				Timelapse::MainForm::TheInstance->tvSendPackets->BeginUpdate();
				Timelapse::MainForm::TheInstance->tvSendPackets->Nodes->Add(headerNode);
				Timelapse::MainForm::TheInstance->tvSendPackets->EndUpdate();
			}#1#
		}
		Assembly::sendPacketLogQueue->clear();
	}*/
}
#pragma endregion

#pragma region Map Rusher Tab
//Get map id for special maps, manually found
static int getSpecialMapID(int mapID, String^ portalName) {
	if (mapID == 101000000 && portalName->Equals("in04")) return 101000400;
	return 999999999;
}

//Load all maps into GlobalRefs::maps & load into TreeView in Map Rusher tab
static void loadMaps() {
	GlobalRefs::maps = gcnew Generic::List<MapData^>();
	try {
		HRSRC hRes = FindResource(GlobalVars::hDLL, MAKEINTRESOURCE(MapsList), _T("TEXT"));
		HGLOBAL hGlob = LoadResource(GlobalVars::hDLL, hRes);
		const char* pData = reinterpret_cast<const char*>(::LockResource(hGlob));
		IO::StringReader^ strReader = gcnew IO::StringReader(gcnew String(pData));

		while(strReader->Peek() != -1) {
			MapData^ tempMapData = gcnew MapData();
			String^ tempString = "";
			int startIndex = 0, endIndex = 0;
			int numPortals = 0; 

			//Get Map ID
			tempString = strReader->ReadLine();
			if (!tempString->Contains("[")) continue;
			startIndex = tempString->IndexOf('[') + 1;
			endIndex = tempString->IndexOf(']');
			tempMapData->mapID = Convert::ToInt32(tempString->Substring(startIndex, endIndex - startIndex));

			//Get Map's Island Name
			tempString = strReader->ReadLine();
			tempMapData->islandName = tempString->Substring(7);

			//Get Map's Street Name
			tempString = strReader->ReadLine();
			tempMapData->streetName = tempString->Substring(11);

			//Get Map's Street Name
			tempString = strReader->ReadLine();
			tempMapData->mapName = tempString->Substring(8);

			//Get the number of portals in Map
			tempString = strReader->ReadLine();
			numPortals = Convert::ToInt32(tempString->Substring(6));

			//Loop through all portals, and add to tempMapData's Portals
			Generic::List<PortalData^>^ tempPortals = gcnew Generic::List<PortalData^>();
			for(int i = 0; i < numPortals; i++) {
				PortalData tempPortalData;
				tempString = strReader->ReadLine();
				array<String^>^ tempArray = tempString->Split(' ');

				tempPortalData.portalName = tempArray[0];
				tempPortalData.xPos = Convert::ToInt32(tempArray[1]);
				tempPortalData.yPos = Convert::ToInt32(tempArray[2]);
				tempPortalData.portalType = Convert::ToInt32(tempArray[3]);
				if (tempPortalData.portalType == 7) 
					tempPortalData.toMapID = getSpecialMapID(tempMapData->mapID, tempPortalData.portalName);
				else 
					tempPortalData.toMapID = Convert::ToInt32(tempArray[4]);

				tempPortals->Add(%tempPortalData); //Add portal to tempPortals
			}
			tempMapData->portals = tempPortals; //Insert portals to temp map
			GlobalRefs::maps->Add(tempMapData); //Add temp map
		}

		UnlockResource(hRes);
	}
	catch (...) { MessageBox::Show("Error: Couldn't load map data"); }

	//Load all maps into the tree view in Map Rusher tab
	for each (MapData^ map in GlobalRefs::maps) {
		TreeNode^ islandNode = gcnew TreeNode(map->islandName); 
		TreeNode^ streetNode = gcnew TreeNode(map->streetName); 
		TreeNode^ mapNode = gcnew TreeNode(map->mapName);
		TreeNode^ mapIDNode = gcnew TreeNode(Convert::ToString(map->mapID));;
		islandNode->Name = map->islandName;
		streetNode->Name = map->streetName;
		mapNode->Name = map->mapName;
		mapNode->Tag = map;
		mapIDNode->Name = Convert::ToString(map->mapID);
		mapIDNode->Tag = "MapID";
		mapNode->Nodes->Add(mapIDNode);

		if(MainForm::TheInstance->tvMapRusherSearch->Nodes->ContainsKey(islandNode->Name)) {
			TreeNode^ tempIslandNode = MainForm::TheInstance->tvMapRusherSearch->Nodes[islandNode->Name];
			MainForm::TheInstance->tvMapRusherSearch->BeginUpdate();

			if(tempIslandNode->Nodes->ContainsKey(streetNode->Name)) {
				tempIslandNode->Nodes[streetNode->Name]->Nodes->Add(mapNode);
			}
			else {
				tempIslandNode->Nodes->Add(streetNode);
				streetNode->Nodes->Add(mapNode);
			}

			MainForm::TheInstance->tvMapRusherSearch->EndUpdate();
		}
		else {
			MainForm::TheInstance->tvMapRusherSearch->BeginUpdate();
			MainForm::TheInstance->tvMapRusherSearch->Nodes->Add(islandNode);
			islandNode->Nodes->Add(streetNode);
			streetNode->Nodes->Add(mapNode);
			MainForm::TheInstance->tvMapRusherSearch->EndUpdate();
		}
	}
}

//Get's MapData of the passed in mapID. Callee function checks if nullptr is returned
static MapData^ getMap(int mapID) {
	for each(MapData^ map in GlobalRefs::maps)
		if (map->mapID == mapID)
			return map;
	return nullptr;
}

//Recursive Depth First Search (DFS) to find path
void existsInNextMapDFS(int currMapID, int startMapID, int destMapID, int numRecursions, cliext::vector<MapPath^>^ searchList, cliext::vector<MapPath^>^ finalPath) {
	if (currMapID == destMapID) {
		if ((int)(finalPath->size()) == 0 || finalPath->size() > searchList->size()) 
			*finalPath = searchList; //Current path is the shortest path to destination map
		
		return; //Returning so that no further maps from this one are searched
	}

	if (getMap(currMapID)->portals->Count == 0 || numRecursions > 300) 
		return; //If current map is an endpoint or if number of recursions are over 300, no further maps are searched

	for each(PortalData^ portalData in getMap(currMapID)->portals) {
		bool existsInSearchList = false;
		for each(MapPath^ mapData in searchList) 
			if (mapData->mapID == portalData->toMapID) existsInSearchList = true;

		if (getMap(portalData->toMapID) == nullptr) continue; //Skips portals where the portal's map is not found
		if (existsInSearchList) continue; //Skip portals where it goes to maps already in search path to prevent loop backs

		MapPath^ mapPath = gcnew MapPath(currMapID, portalData);
		searchList->push_back(mapPath);
		existsInNextMapDFS(portalData->toMapID, startMapID, destMapID, numRecursions + 1, searchList, finalPath); //Recursive call
		searchList->pop_back();
	}
}

//Uses recursive existsInNextMapDFS() to generate a path
cliext::vector<MapPath^>^ generatePath(int startMapID, int destMapID) {
	cliext::vector<MapPath^> ^searchList = gcnew cliext::vector<MapPath^>(), ^finalPath = gcnew cliext::vector<MapPath^>();
	existsInNextMapDFS(startMapID, startMapID, destMapID, 0, searchList, finalPath); //Gets shortest path and puts it into finalPath (if there exists a path)
	return finalPath;
}

//Returns correct portal data (reading client's mem) in the case the stored values are incorrect
PortalData^ findPortal(int toMapID) {
	short portalZRef = 0x4; //First portal
	int portalIndex = ReadMultiPointerSigned(PortalListBase, 3, 0x4, portalZRef, 0x0);
	if (portalIndex != 0) return nullptr; //Check if First Portal Exists
	bool nextPortalExists = true;

	while(nextPortalExists) {
		int currMap = ReadMultiPointerSigned(PortalListBase, 3, 0x4, portalZRef, 0x1C);
		if(currMap == toMapID) {
			PortalData^ newPortalData = gcnew PortalData();
			char* portalName = ReadMultiPointerString(PortalListBase, 3, 0x4, portalZRef, 0x4);
			newPortalData->portalName = gcnew System::String(portalName);
			newPortalData->portalType = ReadMultiPointerSigned(PortalListBase, 3, 0x4, portalZRef, 0x8);
			newPortalData->xPos = ReadMultiPointerSigned(PortalListBase, 3, 0x4, portalZRef, 0xC);
			newPortalData->yPos = ReadMultiPointerSigned(PortalListBase, 3, 0x4, portalZRef, 0x10);
			return newPortalData;
		}
		
		//Check next portal
		portalZRef += 0x8;
		int prevIndex = portalIndex;
		portalIndex = ReadMultiPointerSigned(PortalListBase, 3, 0x4, portalZRef, 0x0);
		if (portalIndex != (prevIndex + 1)) nextPortalExists = false;
	}

	return nullptr;
}

//Classifies each island by the first 2 digits of the Map Ids within the island
int getIsland(int mapID) {
	//0=Maple, 10=Victoria, 11=Florina Beach, 13=Ereve, 14=Rien, 20=Orbis, 21=El Nath, 22=Ludus Lake, 23=Aquarium, 24=Minar Forest, 
	//25=Mu Lung Garden, 26=Nihal Desert, 27=Temple of Time, 30=Elin, 54=Singapore, 55=Malaysia, 60=Masteria, 68=Amoria, 80=Zipangu

	if (mapID < 100000000) return 0; //Maple
	return mapID / 10000000; //Returns first 2 digits of mapID as the island
}

void SendNPCPacket(int npcID, int xPos, int yPos) {
	String^ packet = "";
	writeBytes(packet, gcnew array<BYTE>{0x3A, 0x00}); //NPC Talk OpCode
	writeInt(packet, npcID);
	writeShort(packet, xPos); //Char x pos when npc is clicked, not really important
	writeShort(packet, yPos); //Char y pos when npc is clicked, not really important
	SendPacket(packet);
}

/*WriteMemory(0x0074661D, 1, 0xEB); //Enables multiple open dialogs (in CScriptMan::OnScriptMessage)
typedef int(__stdcall *pfnCUtilDlgEx__ForcedRet)(); //Close Dialogs (not the yes/no dialogue :sadface:)
auto CUtilDlgEx__ForcedRet = (pfnCUtilDlgEx__ForcedRet)0x009A3C2C;
CUtilDlgEx__ForcedRet();*/
/*SendPacket("3C 00 01 01"); Sleep(200); //Click Yes
SendPacket("3C 00 00 01 "); Sleep(200); //Click Next*/
static void SendKey(BYTE keyCode) {
	PostMessage(GlobalVars::mapleWindow, WM_KEYDOWN, keyCode, MapVirtualKey(keyCode, MAPVK_VK_TO_VSC) << 16);
}
//Rushes to next island to route to Destination Map's island
int rushNextIsland(int startMapID, int destMapID) {
	int startIsland = getIsland(startMapID), destIsland = getIsland(destMapID);
	switch (startIsland) {
		case 0: //Rush to Victoria
			if (PointerFuncs::getCharMesos() < 150) {
				MainForm::TheInstance->lbMapRusherStatus->Text = "Status: You need 150 mesos to rush out of Maple Island";
				GlobalRefs::isMapRushing = false;
				return -1;
			}
			if (startMapID != 2000000) mapRush(2000000); //Rush to the map that links to Victoria
			Sleep(1000);
			SendNPCPacket(1000000003, 3366, -112); Sleep(500); //NPC Shanks 
			SendKey(VK_RIGHT); Sleep(500); //Send Right Arrow to select yes
			SendKey(VK_RETURN); Sleep(500); //Send Enter to press yes
			SendKey(VK_RETURN); Sleep(500); //Send Enter to press next
			SendKey(VK_RETURN); Sleep(500); //Send Enter to press next
			return 104000000;
		case 10:
			if (destIsland == 11) {
				if (PointerFuncs::getCharMesos() < 1500) {
					MainForm::TheInstance->lbMapRusherStatus->Text = "Status: You need 1500 mesos to rush to Florina Beach";
					GlobalRefs::isMapRushing = false;
					return -1;
				}
				if (startMapID != 104000000) mapRush(104000000); //Rush to the map that links to Florina Beach
				Sleep(1000);
				SendNPCPacket(1000000008, 1746, 647); Sleep(500); //NPC Pason
				SendKey(VK_RETURN); Sleep(500); //Send Enter to press yes
				return 110000000;
			}
		case 11:
			if (startMapID != 110000000) mapRush(110000000); //Rush to the map that links to Victoria
			Sleep(1000);
			SendNPCPacket(1000000004, -273, 151); Sleep(500);
			SendKey(VK_RETURN); Sleep(500); //Send Enter to press next
			SendKey(VK_RIGHT); Sleep(500); //Send Right Arrow to select yes
			SendKey(VK_RETURN); Sleep(500); //Send Enter to press yes
			return 104000000;
		case 13:
			if (startMapID != 101000400) mapRush(101000400);
			Sleep(1000);
			SendNPCPacket(1000000002, 97, 80); Sleep(500);
			SendKey(VK_RETURN); Sleep(500); //Send Enter to press next
			SendPacket("26 00 00 FF FF FF FF 04 00 68 64 30 30 7E FE 9C FE 00 00 00");
			return 130000210;

	}

	return 0;
}

//Checks if path exists to Destination Map's island
bool existsInterIslandPath(int startMapID, int destMapID) {
	int startIsland = getIsland(startMapID), destIsland = getIsland(destMapID);
	if (destIsland == 0) return false; //Cannot travel to Maple island from anywhere
	
	switch(startIsland) { 
		case 0:
			if (destIsland == 10 || destIsland == 11) return true;
			break;
		case 10:
			if (destIsland == 11) return true;
			break;
		case 11:
			if (destIsland == 10) return true;
			break;
	}

	return false;
}

//Enables hacks to make map rush faster
void toggleFastMapRushHacks(bool isChecked) {
	if(isChecked) {
		//A way to save the original state of the hacks to restore later without needing to create global vars
		if (MainForm::TheInstance->cbNoMapFadeEffect->Checked) MainForm::TheInstance->cbNoMapFadeEffect->ForeColor = Color::Green;
		if (MainForm::TheInstance->cbNoMapBackground->Checked) MainForm::TheInstance->cbNoMapBackground->ForeColor = Color::Green;
		if (MainForm::TheInstance->cbNoMapTiles->Checked) MainForm::TheInstance->cbNoMapTiles->ForeColor = Color::Green;
		if (MainForm::TheInstance->cbNoMapObjects->Checked) MainForm::TheInstance->cbNoMapObjects->ForeColor = Color::Green;
		
		MainForm::TheInstance->cbNoMapFadeEffect->Checked = true;
		MainForm::TheInstance->cbNoMapBackground->Checked = true;
		MainForm::TheInstance->cbNoMapTiles->Checked = true;
		MainForm::TheInstance->cbNoMapObjects->Checked = true;
	}
	else {
		if(MainForm::TheInstance->cbNoMapFadeEffect->ForeColor != Color::Green) MainForm::TheInstance->cbNoMapFadeEffect->Checked = false;
		if(MainForm::TheInstance->cbNoMapBackground->ForeColor != Color::Green) MainForm::TheInstance->cbNoMapBackground->Checked = false;
		if(MainForm::TheInstance->cbNoMapTiles->ForeColor != Color::Green) MainForm::TheInstance->cbNoMapTiles->Checked = false;
		if(MainForm::TheInstance->cbNoMapObjects->ForeColor != Color::Green) MainForm::TheInstance->cbNoMapObjects->Checked = false;

		MainForm::TheInstance->cbNoMapFadeEffect->ForeColor = Color::White;
		MainForm::TheInstance->cbNoMapBackground->ForeColor = Color::White;
		MainForm::TheInstance->cbNoMapTiles->ForeColor = Color::White;
		MainForm::TheInstance->cbNoMapObjects->ForeColor = Color::White;
	}
}

//Map Rush
static void mapRush(int destMapID) {
	GlobalRefs::isMapRushing = true;
	int startMapID = ReadPointer(UIMiniMapBase, OFS_MapID);
	if (startMapID == destMapID) {
		MainForm::TheInstance->lbMapRusherStatus->Text = "Status: Cannot Map Rush to same map";
		GlobalRefs::isMapRushing = false;
		return;
	}

	MainForm::TheInstance->lbMapRusherStatus->Text = "Status: Calculating a path to Destination Map ID";
	if (getIsland(startMapID) != getIsland(destMapID)) {
		for(int i = 0; i < 5; i++) { //Max islands to travel to has to be at max 5 islands
			if (!existsInterIslandPath(startMapID, destMapID)) break; //Check if path between islands exists
			startMapID = rushNextIsland(startMapID, destMapID); //Rushes to next island to dest map's island, return val is new starting map id
			if (startMapID == -1) return; //Error ocurred, rushNextIsland() handles error message
			if (startMapID == 0) break; //End of rushNextIsland() reached, shouldn't happen because of existsInterIslandPath()

			//If first map on new island is the destination, finish
			if (startMapID == destMapID) {
				MainForm::TheInstance->lbMapRusherStatus->Text = "Status: Map Rushing Complete";
				GlobalRefs::isMapRushing = false;
				return;
			} 
		}

		//Couldn't rush to same island as Destination Map
		if (getIsland(startMapID) != getIsland(destMapID)) {
			MainForm::TheInstance->lbMapRusherStatus->Text = "Status: Cannot find a path to Destination Map ID";
			GlobalRefs::isMapRushing = false;
			return;
		}
	}

	cliext::vector<MapPath^>^ mapPath = generatePath(startMapID, destMapID);
	if (mapPath->size() == 0) {
		MainForm::TheInstance->lbMapRusherStatus->Text = "Status: Cannot find a path to Destination Map ID";
		GlobalRefs::isMapRushing = false;
		return;
	}

	int remainingMapCount = mapPath->size(), delay = Convert::ToInt32(MainForm::TheInstance->tbMapRusherDelay->Text);
	if (delay <= 0 || delay > 999999) delay = 500;
	toggleFastMapRushHacks(true); //Enables No Map Background, Fade, Tiles, & Objects for quicker map rush
	int oldChannel = ReadPointer(ServerBase, OFS_Channel);

	std::vector<SpawnControlData*> *oldSpawnControl = Assembly::spawnControl; //Save old spawn control list
	Assembly::spawnControl = new std::vector<SpawnControlData*>(); //Create a new spawn control list for map rushing
	Jump(spawnPointAddr, Assembly::SpawnPointHook, 0); //Enable spawn control 

	for (auto i = mapPath->begin(); i != mapPath->end(); ++i) {
		MapPath^ mapData = *i;
		PortalData^ foundPortal = findPortal(mapData->portal->toMapID); //Find portal in mem in case wz files are different in private server
		if (foundPortal != nullptr) mapData->portal = foundPortal;

		//If first map, add spawn point to spawnControl & CC to new channel to enable hacks
		if (i == mapPath->begin()) {
			Assembly::spawnControl->push_back(new SpawnControlData((*i)->mapID, (*i)->portal->xPos, (*i)->portal->yPos - 10));
			if (oldChannel == 1) AutoCC(2); else AutoCC(1);
			Sleep(delay);
		}

		//Add next map's spawn point to spawnControl
		if((i+1) != mapPath->end()) Assembly::spawnControl->push_back(new SpawnControlData((*(i+1))->mapID, (*(i+1))->portal->xPos, (*(i+1))->portal->yPos - 10));

		//Construct Packet
		String^ packet = "";
		if (mapData->portal->portalType == 7) {
			writeBytes(packet, gcnew array<BYTE>{0x64, 0x00}); //Change Map Special OpCode
			writeByte(packet, 0); // 0 = Change Map through Regular Portals, 1 = Change Map From Dying
			writeString(packet, mapData->portal->portalName); // Portal Name
			writeShort(packet, (short)mapData->portal->xPos); //Portal x Position
			writeShort(packet, (short)mapData->portal->yPos); //Portal y Position
		} else {
			writeBytes(packet, gcnew array<BYTE>{0x26, 0x00}); //Change Map OpCode
			writeByte(packet, 0); // 0 = Change Map through Regular Portals, 1 = Change Map From Dying
			writeInt(packet, -1); // Target Map ID, only not -1 when character is dead, a GM, or for certain maps like Aran Introduction, Intro Map, Adventurer Intro, etc.
			writeString(packet, mapData->portal->portalName); // Portal Name
			writeShort(packet, (short)mapData->portal->xPos); //Portal x Position
			writeShort(packet, (short)mapData->portal->yPos); //Portal y Position
			writeByte(packet, 0); //Unknown
			writeShort(packet, 0); //Wheel of Destiny (item that revtestives char in same map)
		}

		//Spawn in next map
		SendPacket(packet);

		for(int n = 0; n < 50; n++) {
			Sleep(delay);
			if (ReadPointer(UIMiniMapBase, OFS_MapID) != mapData->mapID) break;
			SendPacket(packet);
			if (n % 3 == 0) Teleport(mapData->portal->xPos, mapData->portal->yPos - 20);
		}

		//Check to see if next map is loaded, try max 20 attempts
		/*for(int n = 0; n < 50; n++) {
			Sleep(25);
			if (ReadPointer(UIMiniMapBase, OFS_MapID) != mapData->mapID) break;
			if (n % 5 == 0) SendPacket(packet);
			if (n == 20) Teleport(mapData->portal->xPos, mapData->portal->yPos - 20);
		}*/
		
		remainingMapCount--;
		MainForm::TheInstance->lbMapRusherStatus->Text = "Status: Map Rushing, Remaining Maps: " + Convert::ToString(remainingMapCount);
	}

	Assembly::spawnControl = oldSpawnControl; //Restore old spawn control list
	toggleFastMapRushHacks(false); //Restores hacks to original state
	AutoCC(oldChannel); //CC back to original channel
	Sleep(delay);

	if (ReadPointer(UIMiniMapBase, OFS_MapID) != destMapID) 
		MainForm::TheInstance->lbMapRusherStatus->Text = "Status: An error has occurred, try setting delay higher";
	else 
		MainForm::TheInstance->lbMapRusherStatus->Text = "Status: Map Rushing Complete";

	GlobalRefs::isMapRushing = false;
}

//Find maps with names starting with text entered so far
static void findMapNamesStartingWithStr(String^ str) {
	MainForm::TheInstance->lvMapRusherSearch->BeginUpdate();
	for each(MapData^ map in GlobalRefs::maps) {
		if(map->mapName->ToLower()->StartsWith(str->ToLower())) {
			array<String^>^ row = { map->mapName, Convert::ToString(map->mapID) };
			ListViewItem^ lvi = gcnew ListViewItem(row);
			MainForm::TheInstance->lvMapRusherSearch->Items->Add(lvi);
		}
	}
	MainForm::TheInstance->lvMapRusherSearch->EndUpdate();
}
//#include <chrono>
//Starts Map Rush when clicked
void MainForm::bMapRush_Click(System::Object^  sender, System::EventArgs^  e) {
	//auto start = std::chrono::high_resolution_clock::now();
	if(!GlobalRefs::isMapRushing && !PointerFuncs::getMapID()->Equals("0")) {
		int mapDestID = 0;
		if (INT::TryParse(tbMapRusherDestination->Text, mapDestID))
			if (mapDestID >= 0 && mapDestID <= 999999999)
				mapRush(mapDestID);
	}
	//auto finish = std::chrono::high_resolution_clock::now();
	//std::chrono::duration<double> elapsed = finish - start;
	//MessageBox::Show("Elapsed Time: " + Convert::ToString(elapsed.count()));
}

//Adds tree view selected map's mapID to tbMapRusherDestination textbox
void MainForm::tvMapRusherSearch_MouseDoubleClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
	if (tvMapRusherSearch->SelectedNode == nullptr) return;
	if(dynamic_cast<MapData^>(tvMapRusherSearch->SelectedNode->Tag) != nullptr)
		tbMapRusherDestination->Text = tvMapRusherSearch->SelectedNode->Nodes[0]->Name;

	if(tvMapRusherSearch->SelectedNode->Tag != nullptr && tvMapRusherSearch->SelectedNode->Tag->Equals("MapID"))
		tbMapRusherDestination->Text = tvMapRusherSearch->SelectedNode->Name;

	tvMapRusherSearch->SelectedNode = nullptr;
}

//Adds list view selected map's mapID to tbMapRusherDestination textbox
void MainForm::lvMapRusherSearch_MouseDoubleClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
	tbMapRusherDestination->Text = lvMapRusherSearch->SelectedItems[0]->SubItems[1]->Text;
	lvMapRusherSearch->SelectedItems->Clear();
}

//Find maps with names starting with text entered so far
void MainForm::tbMapRusherSearch_TextChanged(System::Object^  sender, System::EventArgs^  e) {
	lvMapRusherSearch->Items->Clear();

	if(tbMapRusherSearch->Text != "")
		findMapNamesStartingWithStr(tbMapRusherSearch->Text);
}

//Changes color of text to show that the text has changed
void MainForm::lbMapRusherStatus_TextChanged(System::Object^  sender, System::EventArgs^  e) {	
	lbMapRusherStatus->ForeColor = Color::DimGray;
	Application::DoEvents();
	Sleep(50);
	lbMapRusherStatus->ForeColor = Color::White;
	Application::DoEvents();
}
#pragma endregion

//Remove at the end, but for now use it to test stuff out (test button on main form)
#pragma region testing

//typedef int(__stdcall *pfnCLogin__GetLocalMacAddressWithSerialNo)(void*);
//auto CLogin__GetLocalMacAddressWithSerialNo = (pfnCLogin__GetLocalMacAddressWithSerialNo)0x005FCDED;

void Timelapse::MainForm::bTestButton_Click(System::Object^  sender, System::EventArgs^  e) {
	/*void** result;
	CLogin__GetLocalMacAddressWithSerialNo(result);
	String^ test = gcnew String((char*)result); 
	if (String::IsNullOrEmpty(test))
		MessageBox::Show("Error! Empty string was returned");
	else
		MessageBox::Show(test);
		*/

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
		MessageBox::Show(test);*/
}

/*//Start of testing stuff
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
auto CItemInfo__GetMapString = (pfnCItemInfo__GetMapString)0x005CF792;*/

#pragma endregion