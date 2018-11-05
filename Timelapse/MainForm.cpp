#include <Windows.h>
#include <cliext/vector>
#include <sstream>
#include "MainForm.h"
#include "Pointers.h"
#include "Functions.h"
#include "Macro.h"
#include "Packet.h"
#include "Structs.h"
#include "resource.h"

using namespace Timelapse;
static void loadMaps(); //Forward declaration
[assembly:System::Diagnostics::DebuggableAttribute(true, true)]; //For debugging purposes

//Managed Global Variables
ref struct GlobalRefs {
	static Macro ^macroHP, ^macroMP, ^macroAttack, ^macroLoot;
	static bool isChangingField = false, isMapRushing = false;
	static bool bClickTeleport = false, bMouseTeleport = false, bTeleport = false, bKami = false, bKamiLoot;
	static UINT cccsTimerTickCount = 0;
	static bool isDragging = false;
	static Point dragOffset;
	static double formOpacity;
	static Generic::List<MapData^>^ maps; 
};

#pragma region General Form
[STAThreadAttribute]
void Main() {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	Application::Run(gcnew MainForm);
	Application::Exit();
}

#pragma unmanaged
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved) {
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
	Threading::Thread^ macroThread = gcnew Threading::Thread(gcnew Threading::ThreadStart(PriorityQueue::MacroQueueWorker));
	macroThread->Start();
	loadMaps();
}

void MainForm::MainForm_FormClosing(Object^  sender, Windows::Forms::FormClosingEventArgs^  e) {
	PriorityQueue::closeMacroQueue = true;
	for (double i = this->Opacity; i > 0;) {
		i -= 0.1;
		this->Opacity = i;
		this->Refresh();
	}
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

#pragma region ToolStrip
void MainForm::closeMapleStoryToolStripMenuItem_Click(Object^  sender, EventArgs^  e) {
	TerminateProcess(GetCurrentProcess(), 0);
}
#pragma endregion
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
	hpToolTip->SetToolTip(lbHP, "(" + CodeCaves::curHP.ToString() + "/" + CodeCaves::maxHP.ToString() + ")");
}

//Display Char MP values on hover
void MainForm::lbMP_MouseHover(Object^  sender, EventArgs^  e) {
	ToolTip^ mpToolTip = gcnew ToolTip();
	mpToolTip->IsBalloon = true;
	mpToolTip->ShowAlways = true;
	mpToolTip->SetToolTip(lbMP, "(" + CodeCaves::curMP.ToString() + "/" + CodeCaves::maxMP.ToString() + ")");
}

//Display Char EXP values on hover
void MainForm::lbEXP_MouseHover(Object^  sender, EventArgs^  e) {
	ToolTip^ expToolTip = gcnew ToolTip();
	expToolTip->IsBalloon = true;
	expToolTip->ShowAlways = true;
	expToolTip->SetToolTip(lbEXP, "(" + CodeCaves::curEXP.ToString() + "/" + CodeCaves::maxEXP.ToString() + ")");
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
	lbDateTime->Text = DateTime::Now.ToString();
	lbThreadID->Text = "0x" + GetMSThreadID().ToString("X");
	if (GetMSThreadID()) {;
		lbActive->Visible = true;
		lbInactive->Visible = false;
	}
	else {
		lbActive->Visible = false;
		lbInactive->Visible = true;
	}

	if (!PointerFuncs::getMapID()->Equals("0")) {
		//if (PointerFuncs::getMapName()->Equals("Waiting..") && PointerFuncs::isHooked) 
			//CCToGetPointers();

		lbMapName->Text = PointerFuncs::getMapName();

		lbCharName->Text = PointerFuncs::getCharName();
		lbLevel->Text = PointerFuncs::getCharLevel();
		lbJob->Text = PointerFuncs::getCharJob();
		lbHP->Text = PointerFuncs::getCharHP();
		lbMP->Text = PointerFuncs::getCharMP();
		lbEXP->Text = PointerFuncs::getCharEXP();
		lbMesos->Text = PointerFuncs::getCharMesos();

		lbWorld->Text = PointerFuncs::getWorld();
		lbChannel->Text = PointerFuncs::getChannel();
		lbMapID->Text = PointerFuncs::getMapID();
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
#pragma region Auto Login
//From Old Trainer, need to fix dc if already logged in once


/*System::Text::StringBuilder usernameOutput, passwordOutput;
String^ username = this->textBox34->Text;
String^ password = this->textBox35->Text;
for each (System::Byte b in username) usernameOutput.AppendFormat("{0:X} ", b);
for each (System::Byte b in password) passwordOutput.AppendFormat("{0:X} ", b);

System::String^ usernameString = usernameOutput.ToString();
System::String^ passwordString = passwordOutput.ToString();
System::String^ loginPacket = "01 00 07 00" + usernameString + "08 00" + passwordString + "00 00 00 00 00 00 B4 43 8D 48 00 00 00 00 13 92 00 00 00 00 02 00 00 00 00 00 00";

msclr::interop::marshal_context mc;
SendPacket(loginPacket);
Sleep(2000);

SendPacket("0B 00");
Sleep(2000);

SendPacket("06 00 00 00");
Sleep(2000);

int channel;
if (this->comboBox10->Text->Equals("Random")) channel = rand() % getmax();
else channel = int::Parse(this->comboBox10->Text) - 1;
SendPacket(gcnew String("05 00 02 00" + gcnew String(intToHexL(channel, 2, false).c_str()) + "7F 00 00 01"));
Sleep(2000);

int character = int::Parse(this->comboBox11->Text) - 1; //" + gcnew String(intToHexL(character, 2, false).c_str()) + "
SendPacket(gcnew String("13 00 02 00 00 00 11 00 41 34 2D 33 34 2D 44 39 2D 34 38 2D 39 44 2D 46 36 15 00 35 30 37 42 39 44 35 46 31 38 37 34 5F 42 34 34 33 38 44 34 38"));*/

#pragma endregion

#pragma region Options
void MainForm::transparencyTrackBar_Scroll(Object^  sender, EventArgs^  e) {
	this->Opacity = transparencyTrackBar->Value * 0.01;
}
#pragma endregion
#pragma endregion

#pragma region Bots Tab
#pragma region Auto HP
void MainForm::cbHP_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbHP->Checked) {
		if (GlobalRefs::macroHP == nullptr)
			GlobalRefs::macroHP = gcnew Macro(keyCollection[this->comboHPKey->SelectedIndex], 200, MacroType::HPPOTMACRO);
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
			GlobalRefs::macroMP = gcnew Macro(keyCollection[this->comboMPKey->SelectedIndex], 200, MacroType::MPPOTMACRO);
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

			GlobalRefs::macroAttack = gcnew Macro(keyCollection[this->comboAttackKey->SelectedIndex], Convert::ToUInt32(tbAttackInterval->Text), MacroType::ATTACKMACRO);
		}
		GlobalRefs::macroAttack->Toggle(true);
		MacrosEnabled::bMacroAttack = true;
	}
	else {
		GlobalRefs::macroAttack->Toggle(false);
		MacrosEnabled::bMacroAttack = false;
	}
}

void MainForm::tbAttackInterval_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
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
			GlobalRefs::macroLoot = gcnew Macro(keyCollection[this->comboLootKey->SelectedIndex], Convert::ToUInt32(tbLootInterval->Text), MacroType::LOOTMACRO);
		}
		GlobalRefs::macroLoot->Toggle(true);
		MacrosEnabled::bMacroLoot = true;
	}
	else {
		GlobalRefs::macroLoot->Toggle(false);
		MacrosEnabled::bMacroLoot = true;
	}
}

void MainForm::tbLootInterval_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
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
	lvi->Tag = gcnew Macro(keyCollection[comboBuffKey->SelectedIndex], Convert::ToUInt32(tbBuffInterval->Text), MacroType::BUFFMACRO);
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
void CallCCFunc(int channel) {
	if (PointerFuncs::getMapID()->Equals("0")) return;
	WritePointer(UserLocalBase, OFS_Breath, 0);
	CField__SendTransferChannelRequest(channel);
	Sleep(200);
}

void CallCSFunc() {
	if (PointerFuncs::getMapID()->Equals("0")) return;
	WritePointer(UserLocalBase, OFS_Breath, 0);
	CWvsContext__SendMigrateToShopRequest(*(PVOID*)ServerBase, (PVOID)0x2FDFE1D, 0);
	Sleep(Convert::ToUInt32(MainForm::TheInstance->tbCSDelay->Text));
	//CCashShop__SendTransferFieldPacket();
	Sleep(1000);
}

void _stdcall AutoCC(int toChannel) {
	if (GlobalRefs::isMapRushing) return;
	int channel;
	if (toChannel == -1) channel = rand() % 19;
	else channel = toChannel;

	if (MainForm::TheInstance->rbFunction->Checked) //Call Auto CC Function
		CallCCFunc(channel);
	else //Send Auto CC Packet
		SendPacket(gcnew String("27 00 " + channel.ToString("X2") + " ** ** ** 00"));
}

void _stdcall AutoCS() {
	if (GlobalRefs::isMapRushing) return;
	if (MainForm::TheInstance->rbFunction->Checked) //Call Auto CS Function
		CallCSFunc();
	else if (MainForm::TheInstance->rbPacket->Checked) { //Send Auto CS Packet
		if(String::IsNullOrWhiteSpace(MainForm::TheInstance->tbCSDelay->Text)) {
			MessageBox::Show("Error: CS Delay textbox cannot be empty");
			return;
		}
		SendPacket(gcnew String("28 00 ** ** ** 00"));
		Sleep(Convert::ToUInt32(MainForm::TheInstance->tbCSDelay->Text));
		SendPacket(gcnew String("26 00"));
		Sleep(500);
	}
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
	if(cbCCCSTime->Checked) {
		GlobalRefs::cccsTimerTickCount += 250;
		if ((GlobalRefs::cccsTimerTickCount/1000) >= Convert::ToUInt32(tbCCCSTime->Text)) {
			if (rbCC->Checked) AutoCC(-1);
			else AutoCS();
			GlobalRefs::cccsTimerTickCount = 0;
		}
	}

	if (cbCCCSPeople->Checked) {
		if (ReadPointer(UserPoolBase, OFS_PeopleCount) >= Convert::ToUInt32(tbCCCSPeople->Text)) {
			if (rbCC->Checked) AutoCC(-1);
			else AutoCS();
		}
	}

	if (cbCCCSAttack->Checked) {
		if(ReadPointer(UserLocalBase, OFS_AttackCount) >= Convert::ToUInt32(tbCCCSAttack->Text)) {
			if (rbCC->Checked) AutoCC(-1);
			else AutoCS();
		}
	}

	if (cbCCCSMob->Checked) {
		if (ReadPointer(MobPoolBase, OFS_MobCount) <= Convert::ToUInt32(tbCCCSMob->Text)) {
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
#pragma endregion

#pragma region Hacks I Tab
//Full Godmode (CUserLocal::SetDamaged())
void MainForm::cbFullGodmode_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbFullGodmode->Checked)
		WriteMemory(0x009581D5, 2, 0x0F, 0x84); //je 009596F7 [first 2 bytes]
	else
		WriteMemory(0x009581D5, 2, 0x0F, 0x85); //jne 009596F7 [first 2 bytes]
}

//Miss Godmode (CUserLocal::SetDamaged())
void MainForm::cbMissGodmode_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbMissGodmode->Checked)
		WriteMemory(0x009582E9, 8, 0xC7, 0x06, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90); //mov [esi],00000000; nop; nop;
	else
		WriteMemory(0x009582E9, 8, 0x89, 0x06, 0x83, 0xC6, 0x04, 0xFF, 0x4D, 0xC4); //mov [esi],eax; add esi,04; dec [ebp-3C];
}

//Blink Godmode (CUser::Update())
void MainForm::cbBlinkGodmode_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbBlinkGodmode->Checked)
		WriteMemory(0x00932501, 2, 0x83, 0xC7); //add edi,1E
	else
		WriteMemory(0x00932501, 2, 0x83, 0xEF); //sub edi,1E
}

void ClickTeleport() {
	while (GlobalRefs::bClickTeleport) {
		if (ReadPointer(InputBase, OFS_MouseAnimation) == 12)
			Teleport(ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseX), ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseY));
		Sleep(Convert::ToUInt32(MainForm::TheInstance->tbClickTeleport->Text));
	}
}

void MouseTeleport() {
	while (GlobalRefs::bMouseTeleport) {
		if (ReadPointer(InputBase, OFS_MouseAnimation) == 00)
			Teleport(ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseX), ReadMultiPointerSigned(InputBase, 2, OFS_MouseLocation, OFS_MouseY));
		Sleep(Convert::ToUInt32(MainForm::TheInstance->tbMouseTeleport->Text));
	}
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
		Jump(mouseFlyXAddr, CodeCaves::MouseFlyXHook, 0); //MouseFlyXHook CodeCave
		Jump(mouseFlyYAddr, CodeCaves::MouseFlyYHook, 0); //MouseFlyYHook CodeCave
	}
	else {
		WriteMemory(mouseFlyXAddr, 5, 0x89, 0x03, 0x8B, 0x7D, 0x10); //mov [ebx],eax; mov edi,[ebp+10]
		WriteMemory(mouseFlyYAddr, 5, 0x89, 0x07, 0x8B, 0x5D, 0x14); //mov [edi],eax; mov ebx,[ebp+14]
	}
}

//Swim in Air (CVecCtrl::IsSwimming())
void MainForm::cbSwimInAir_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbSwimInAir->Checked)
		WriteMemory(0x00704704, 2, 0x74, 0x04); //je 0070470A
	else
		WriteMemory(0x00704704, 2, 0x75, 0x04); //jne 0070470A
}

//Speed Attack (CAvatar::PrepareActionLayer())
void MainForm::cbSpeedAttack_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbSpeedAttack->Checked)
		WriteMemory(0x0045478F, 2, 0x0F, 0x8F); //jg 0045483D [first 2 bytes]
	else
		WriteMemory(0x0045478F, 2, 0x0F, 0x8E); //jng 0045483D [first 2 bytes]
}

//Unlimited Attack (CAntiRepeat::TryRepeat())
void MainForm::cbUnlimitedAttack_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbUnlimitedAttack->Checked)
		WriteMemory(0x009536E0, 1, 0xEB); //jmp 0095370C [first byte]
	else
		WriteMemory(0x009536E0, 1, 0x7E); //jle 0095370C [first byte]
}

//Full Accuracy
void MainForm::cbFullAccuracy_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbFullAccuracy->Checked)
		WriteMemory(0x00AFE7F8, 8, 0x00, 0x00, 0x00, 0xE0, 0xCF, 0x12, 0x63, 0x41); //add [eax],al; add al,ah; iretd; adc ah,byte ptr [ebx+41];
	else
		WriteMemory(0x00AFE7F8, 8, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0xE6, 0x3F); //out 3F,al
}

//No Breath (CAvatar::Update())
void MainForm::cbNoBreath_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoBreath->Checked)
		WriteMemory(0x00452316, 1, 0x7C); //jl 0045233D [first byte]
	else
		WriteMemory(0x00452316, 1, 0x7D); //jnl 0045233D [first byte]
}

//No Player Kickback (CVecCtrl::SetImpactNext())
void MainForm::cbNoPlayerKickback_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoPlayerKickback->Checked)
		WriteMemory(0x007A637F, 3, 0xC7, 0x00, 0x00); //mov [eax],00000000 [first 3 bytes] 
	else
		WriteMemory(0x007A637F, 3, 0xC7, 0x00, 0x01); //mov [eax],00000001 [first 3 bytes]
}

//No Player Death (CUserLocal::OnResolveMoveAction())
void MainForm::cbNoPlayerDeath_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoPlayerDeath->Checked)
		WriteMemory(0x009506C6, 2, 0xEB, 0x0B); //jmp 009506D3
	else
		WriteMemory(0x009506C6, 2, 0x74, 0x0B); //je 009506D3
}

//Jump Down Any Tile (CUserLocal::FallDown())
void MainForm::cbJumpDownAnyTile_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbJumpDownAnyTile->Checked)
		WriteMemory(0x0094C6EE, 2, 0x90, 0x90); //nop; nop;
	else
		WriteMemory(0x0094C6EE, 2, 0x74, 0x1E); //je 0094C70E
}

//No Skill Effects (CUser::ShowSkillEffect())
void MainForm::cbNoSkillEffects_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoSkillEffects->Checked)
		WriteMemory(0x00933990, 5, 0xC2, 0x14, 0x00, 0x90, 0x90); //ret 0014; nop; nop; nop;
	else
		WriteMemory(0x00933990, 5, 0xB8, 0x34, 0xC3, 0xAD, 0x00); //mov eax,00ADC334
}

//No Attack Delay (CUser::SetAttackAction())
void MainForm::cbNoAttackDelay_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoAttackDelay->Checked)
		WriteMemory(0x0092EDB2, 10, 0x6A, 0x01, 0x58, 0xC2, 0x10, 0x00, 0x90, 0x90, 0x90, 0x90); //push 01; pop eax; ret 0010; nop; nop; nop; nop;
	else
		WriteMemory(0x0092EDB2, 10, 0xB8, 0x88, 0xB7, 0xAD, 0x00, 0xE8, 0xDC, 0x1C, 0x13, 0x00); //mov eax,00ADB788; call 00A60B98;
}

//No Player Name Tag (CUser::DrawNameTags())
void MainForm::cbNoPlayerNameTag_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoPlayerNameTag->Checked)
		WriteMemory(0x00942DCC, 5, 0xC3, 0x90, 0x90, 0x90, 0x90); //ret; nop; nop; nop; nop;
	else
		WriteMemory(0x00942DCC, 5, 0xB8, 0x14, 0xDA, 0xAD, 0x00); //mov eax,00ADDA14
}

//Instant Drop Items
void MainForm::cbInstantDropItems_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbInstantDropItems->Checked)
		WriteMemory(0x00AF0E1C, 4, 0x00, 0x00, 0x00, 0x00); //add [eax],al; add [eax],al;
	else
		WriteMemory(0x00AF0E1C, 4, 0x00, 0x40, 0x8F, 0x40); //add [eax-71],al; inc eax;
}

//Instant Loot Items (CAnimationDisplayer::ABSORBITEM::Update())
void MainForm::cbInstantLootItems_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbInstantLootItems->Checked)
		WriteMemory(0x004417E3, 6, 0x81, 0xFB, 0x00, 0x00, 0x00, 0x00); //cmp ebx,00000000
	else
		WriteMemory(0x004417E3, 6, 0x81, 0xFB, 0xBC, 0x02, 0x00, 0x00); //cmp ebx,000002BC
}

//Fast Loot Items (CWvsContext::CanSendExclRequest())
void MainForm::cbFastLootItems_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbFastLootItems->Checked)
		WriteMemory(0x00485C01, 2, 0x90, 0x90); //nop; nop;
	else
		WriteMemory(0x00485C01, 2, 0x75, 0x36); //jne 00485C39
}

//No Mob Reaction (CMob::AddDamageInfo())
void MainForm::cbNoMobReaction_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoMobReaction->Checked)
		WriteMemory(0x0066B05E, 5, 0xC2, 0x44, 0x00, 0x90, 0x90); //ret 0044; nop; nop;
	else
		WriteMemory(0x0066B05E, 5, 0xB8, 0x50, 0x1A, 0xAA, 0x00); //mov eax,00AA1A50
}

//No Mob Death Effect (CMob::OnDie())
void MainForm::cbNoMobDeathEffect_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoMobDeathEffect->Checked)
		WriteMemory(0x00663995, 5, 0xC3, 0x90, 0x90, 0x90, 0x90); //ret; nop; nop; nop; nop;
	else
		WriteMemory(0x00663995, 5, 0xB8, 0xD4, 0x13, 0xAA, 0x00); //mov eax,00AA13D4
}

//No Mob Kickback (CMob::OnHit())
void MainForm::cbNoMobKickback_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoMobKickback->Checked)
		WriteMemory(0x00668C9E, 2, 0x90, 0x90); //nop; nop;
	else
		WriteMemory(0x00668C9E, 2, 0x74, 0x20); //je 00668CC0
}

//Mob Freeze (CVecCtrlMob::WorkUpdateActive())
void MainForm::cbMobFreeze_CheckedChanged(Object^  sender, EventArgs^  e)
{
	if (this->cbMobFreeze->Checked)
		Jump(mobFreezeAddr, CodeCaves::MobFreezeHook, 1); //MobFreezeHook CodeCave
	else
		WriteMemory(mobFreezeAddr, 6, 0x8B, 0x86, 0x48, 0x02, 0x00, 0x00); //mov eax,[esi+00000248]
}

//Mob Disarm (CMob::Update())
void MainForm::cbMobDisarm_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbMobDisarm->Checked)
		WriteMemory(0x00667A00, 9, 0xE9, 0xFD, 0x01, 0x00, 0x00, 0x90, 0x90, 0x90, 0x90); //jmp 00667C02; nop; nop; nop; nop;
	else
		WriteMemory(0x00667A00, 9, 0x75, 0x0E, 0x8B, 0xCB, 0xE8, 0x24, 0x69, 0x00, 0x00); //jne 00667A10; mov ecx,ebx; call 0066E32D;
}

//Mob Auto Aggro (CVecCtrlMob::WorkUpdateActive())
void MainForm::cbMobAutoAggro_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbMobAutoAggro->Checked)
		Jump(mobAutoAggroAddr, CodeCaves::MobAutoAggroHook, 0); //MobAutoAggroHook CodeCave
	else
		WriteMemory(mobAutoAggroAddr, 5, 0xE8, 0xD4, 0x4E, 0xFF, 0xFF); //call 009B19D0 (calls CVecCtrl::WorkUpdateActive())
}

//No Map Background (CMapReloadable::LoadMap())
void MainForm::cbNoMapBackground_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoMapBackground->Checked)
		WriteMemory(0x00639CB6, 5, 0x90, 0x90, 0x90, 0x90, 0x90); //nop; nop; nop; nop; nop;
	else
		WriteMemory(0x00639CB6, 5, 0xE8, 0xFF, 0x2E, 0x00, 0x00); //call 0063CBBA
}

//No Map Objects (CMapReloadable::LoadMap())
void MainForm::cbNoMapObjects_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoMapObjects->Checked)
		WriteMemory(0x00639CAF, 5, 0x90, 0x90, 0x90, 0x90, 0x90); //nop; nop; nop; nop; nop;
	else
		WriteMemory(0x00639CAF, 5, 0xE8, 0xCA, 0x0D, 0x00, 0x00); //call 0063AA7E
}

//No Map Tiles (CMapReloadable::LoadMap())
void MainForm::cbNoMapTiles_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoMapTiles->Checked)
		WriteMemory(0x00639CA8, 5, 0x90, 0x90, 0x90, 0x90, 0x90); //nop; nop; nop; nop; nop;
	else
		WriteMemory(0x00639CA8, 5, 0xE8, 0x53, 0x04, 0x00, 0x00); //call 0063A100
}

//No Map Fade Effect (CStage::FadeOut())
void MainForm::cbNoMapFadeEffect_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoMapFadeEffect->Checked)
		WriteMemory(0x00776E65, 5, 0xC3, 0x90, 0x90, 0x90, 0x90); //ret; nop; nop; nop; nop;
	else
		WriteMemory(0x00776E65, 5, 0xB8, 0x44, 0x61, 0xAB, 0x00); //mov eax,00AB6144
}

//Map Speed Up (max_walk_speed())
void MainForm::cbMapSpeedUp_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbMapSpeedUp->Checked)
		WriteMemory(0x009B21A0, 3, 0x8D, 0x48, 0x0C); //lea ecx,[eax+0C]
	else
		WriteMemory(0x009B21A0, 3, 0x8D, 0x48, 0x24); //lea ecx,[eax+24]
}

//Infinite & Uncensored Chat
void MainForm::cbInfiniteChat_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbInfiniteChat->Checked) {
		WriteMemory(0x00490607, 2, 0xEB, 0x27); //jmp 00490630
		WriteMemory(0x00490651, 2, 0xEB, 0x1D); //jmp 00490670
		WriteMemory(0x004CAA09, 2, 0xEB, 0x7E); //jmp 004CAA89
		WriteMemory(0x004CAA84, 2, 0xEB, 0x03); //jmp 004CAA89
	}
	else {
		WriteMemory(0x00490607, 2, 0x74, 0x27); //je 00490630
		WriteMemory(0x00490651, 2, 0x73, 0x1D); //jae 00490670
		WriteMemory(0x004CAA09, 2, 0x7E, 0x7E); //jle 004CAA89
		WriteMemory(0x004CAA84, 2, 0x7E, 0x03); //jle 004CAA89
	}
}

//No Blue Boxes (CUtilDlg::Notice())
void MainForm::cbNoBlueBoxes_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbNoBlueBoxes->Checked)
		WriteMemory(0x009929DD, 5, 0xC3, 0x90, 0x90, 0x90, 0x90); //ret; nop; nop; nop; nop;
	else
		WriteMemory(0x009929DD, 5, 0xB8, 0x92, 0x21, 0xAE, 0x00); //mov eax,00AE2192
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
	CodeCaves::spawnControl->push_back(sps);
}

void MainForm::bSpawnControlDelete_Click(Object^  sender, EventArgs^  e) {
	for each(ListViewItem^ lvi in lvSpawnControl->SelectedItems) {
		for (std::vector<SpawnControlData*>::const_iterator i = CodeCaves::spawnControl->begin(); i != CodeCaves::spawnControl->end(); ++i) {
			if (Convert::ToString((*i)->mapID)->Equals(lvi->SubItems[0]->Text)) {
				CodeCaves::spawnControl->erase(i);
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
		Jump(spawnPointAddr, CodeCaves::SpawnPointHook, 0);
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
#pragma region Kami
void KamiLoop() {
	while (GlobalRefs::bKami || GlobalRefs::bKamiLoot) {
		if (GlobalRefs::bKami && GlobalRefs::bKamiLoot) {
			if(ReadPointer(DropPoolBase, OFS_ItemCount) > Convert::ToUInt32(MainForm::TheInstance->tbKamiLootItem->Text)) {
				if (!GlobalRefs::isChangingField && !GlobalRefs::isMapRushing) {}
					Teleport(CodeCaves::ItemX, CodeCaves::ItemY + 10);
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
					MessageBox::Show("ItemX: " + CodeCaves::ItemX.ToString() + " ItemY: " + CodeCaves::ItemY.ToString()); //Teleport(CodeCaves::ItemX, CodeCaves::ItemY+10);
			}
			Sleep(Convert::ToUInt32(MainForm::TheInstance->tbKamiLootInterval->Text));
		}
	}
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
		*(ULONG*)PtInRectAddr = (ULONG)CodeCaves::ItemHook;
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
void MainForm::tbDupeXMob_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
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

//Full Map Attack (CMobPool::FindHitMobInRect())
void MainForm::cbFullMapAttack_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbFullMapAttack->Checked)
		WriteMemory(0x006785CA, 2, 0x90, 0x90); //nop; nop;
	else
		WriteMemory(0x006785CA, 2, 0x74, 0x22); //je 006785EE
}

void MainForm::cbItemVac_CheckedChanged(Object^  sender, EventArgs^  e) {
	if (this->cbItemVac->Checked)
		Jump(itemVacAddr, CodeCaves::ItemVacHook, 2);
	else
		WriteMemory(itemVacAddr, 7, 0x50, 0xFF, 0x75, 0xDC, 0x8D, 0x45, 0xCC);
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

		CodeCaves::isItemFilterEnabled = 1;
		if (CodeCaves::isItemLoggingEnabled == 0)
			Jump(itemFilterAddr, CodeCaves::ItemFilterHook, 1);
	}
	else {
		bItemFilter->Text = "Enable Item Filter";
		CodeCaves::isItemFilterEnabled = 0;
		if(CodeCaves::isItemLoggingEnabled == 0)
			WriteMemory(itemFilterAddr, 6, 0x89, 0x47, 0x34, 0x8B, 0x7D, 0xEC); //mov [edi+34],eax; mov edi,[ebp-14];
	}
}

//Change Item Filter type (either White List or Black List)
void MainForm::rbItemFilterWhiteList_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (rbItemFilterWhiteList->Checked)
		CodeCaves::isItemFilterWhiteList = 1;
	else
		CodeCaves::isItemFilterWhiteList = 0;
}

//Enable Item Filter Log
void MainForm::cbItemFilterLog_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if(cbItemFilterLog->Checked) {
		CodeCaves::isItemLoggingEnabled = 1;
		if (CodeCaves::isItemFilterEnabled == 0)
			Jump(itemFilterAddr, CodeCaves::ItemFilterHook, 1);
	}
	else {
		CodeCaves::isItemLoggingEnabled = 0;
		if (CodeCaves::isItemFilterEnabled == 0)
			WriteMemory(itemFilterAddr, 6, 0x89, 0x47, 0x34, 0x8B, 0x7D, 0xEC); //mov [edi+34],eax; mov edi,[ebp-14];
	}
}

//Add item to Item Filter ListBox
void MainForm::bItemFilterAdd_Click(System::Object^  sender, System::EventArgs^  e) {
	if(tbItemFilterID->TextLength > 0) {
		try {
			UINT itemID = Convert::ToUInt32(tbItemFilterID->Text);
			String^ item = findItemNameFromID(itemID);

			if(itemID > 0 && !lbItemFilter->Items->Contains(item + " (" + itemID.ToString() + ")")) {
				lbItemFilter->Items->Add(item + " (" + itemID.ToString() + ")");
				tbItemFilterID->Text = "";
				lbItemFilter->SelectedIndex = -1;
				CodeCaves::itemList->push_back(itemID);
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
		CodeCaves::itemList->erase(std::find(CodeCaves::itemList->begin(), CodeCaves::itemList->end(), Convert::ToUInt32(itemIDStr)));

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
		CodeCaves::itemList->push_back(Convert::ToUInt32(itemIDStr));

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
			CodeCaves::itemFilterMesos = mesosLimit;
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
		CodeCaves::isMobFilterEnabled = 1;
		if (CodeCaves::isMobLoggingEnabled == 0) {
			Jump(mobFilter1Addr, CodeCaves::MobFilter1Hook, 0);
			Jump(mobFilter2Addr, CodeCaves::MobFilter2Hook, 0);
		}
	}
	else {
		bMobFilter->Text = "Enable Mob Filter";
		CodeCaves::isMobFilterEnabled = 0;
		if (CodeCaves::isMobLoggingEnabled == 0) {
			WriteMemory(mobFilter1Addr, 5, 0xE8, 0xF7, 0xE2, 0xD8, 0xFF); //call 00406629
			WriteMemory(mobFilter2Addr, 5, 0xE8, 0x98, 0xD1, 0xD8, 0xFF); //call 00406629
		}
	}
}

//Change Mob Filter type (either White List or Black List)
void MainForm::rbMobFilterWhiteList_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (rbMobFilterWhiteList->Checked)
		CodeCaves::isMobFilterWhiteList = 1;
	else
		CodeCaves::isMobFilterWhiteList = 0;
}

//Enable Mob Filter Log
void MainForm::cbMobFilterLog_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (cbMobFilterLog->Checked) {
		CodeCaves::isMobLoggingEnabled = 1;
		if (CodeCaves::isMobFilterEnabled == 0) {
			Jump(mobFilter1Addr, CodeCaves::MobFilter1Hook, 0);
			Jump(mobFilter2Addr, CodeCaves::MobFilter2Hook, 0);
		}
	}
	else {
		CodeCaves::isMobLoggingEnabled = 0;
		if (CodeCaves::isMobFilterEnabled == 0) {
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
			String^ mob = findMobNameFromID(mobID);

			if (mobID > 0 && !lbMobFilter->Items->Contains(mob + " (" + mobID.ToString() + ")")) {
				lbMobFilter->Items->Add(mob + " (" + mobID.ToString() + ")");
				tbMobFilterID->Text = "";
				lbMobFilter->SelectedIndex = -1;
				CodeCaves::mobList->push_back(mobID);
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
		CodeCaves::mobList->erase(std::find(CodeCaves::mobList->begin(), CodeCaves::mobList->end(), Convert::ToUInt32(mobIDStr)));

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
		CodeCaves::mobList->push_back(Convert::ToUInt32(mobIDStr));

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
//Send Suicide Packet: 30 00 00 00 00 00 FD 00 00 FF 00 00 00 00 00
//Send Changes map (if alive, teleport to starting map, if dead, revives you in nearest town) 26 00 00 00 00 00 00 00 00 00 00 00
//Send Restores health: 59 00 A1 7F F7 08 00 14 00 00 0A 00 00 00 00
void MainForm::bSendPacket_Click(Object^  sender, EventArgs^  e) {
	SendPacket(tbSendPacket->Text);
	//SendPacket(gcnew String("28 00 ** ** ** 00"));
}
void MainForm::bRecvPacket_Click(Object^  sender, EventArgs^  e) {
	RecvPacket(tbRecvPacket->Text);
}

void MainForm::tbSendSpamDelay_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::bSendLog_Click(System::Object^  sender, System::EventArgs^  e) {

	/*if(bSendLog->Text->Equals("Enable Log")) {
		bSendLog->Text = "Disable Log";
		//GlobalRefs::isPacketsSentHooked = true;
		Jump(0x0049637B, SendPacketHook, 0);
	}
	else {
		bSendLog->Text = "Enable Log";
		//GlobalRefs::isPacketsSentHooked = false;
		WriteMemory(0x0049637B, 5, 0xB8, 0x6C, 0x12, 0xA8, 0x00);
	}*/
}

#pragma region Predefined Packets
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

#pragma region AutoAP
void MainForm::tbAPLevel_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbAPHP_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbAPMP_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbAPSTR_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbAPDEX_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbAPINT_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}

void MainForm::tbAPLUK_KeyPress(Object^  sender, Windows::Forms::KeyPressEventArgs^  e) {
	if (!isKeyValid(sender, e, false)) e->Handled = true; //If key is not valid, do nothing and indicate that it has been handled
}
#pragma endregion
#pragma endregion

#pragma endregion

#pragma region Map Rusher Tab
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
		//if (portalData->toMapID == startMapID || portalData->toMapID == currMapID) continue; //Skip portals that come back to starting or current map
		if (existsInSearchList) continue; //Skip portals where it goes to maps already in search path to prevent loop backs

		MapPath^ mapPath = gcnew MapPath(portalData->toMapID, portalData);
		searchList->push_back(mapPath);
		existsInNextMapDFS(portalData->toMapID, startMapID, destMapID, numRecursions + 1, searchList, finalPath); //Recursive call
		searchList->pop_back();
	}
}

//Uses recursive existsInNextMapDFS() to generate a path
cliext::vector<MapPath^>^ generatePath(int startMapID, int destMapID) {
	cliext::vector<MapPath^> ^searchList = gcnew cliext::vector<MapPath^>(), ^finalPath = gcnew cliext::vector<MapPath^>();
	existsInNextMapDFS(startMapID, startMapID, destMapID, 0, searchList, finalPath); //
	return finalPath;
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
	cliext::vector<MapPath^>^ mapPath = generatePath(startMapID, destMapID);

	if (mapPath->size() == 0) {
		MainForm::TheInstance->lbMapRusherStatus->Text = "Status: Cannot find a path to Destination Map ID";
		GlobalRefs::isMapRushing = false;
		return;
	}

	int remainingMapCount = mapPath->size();
	MainForm::TheInstance->lbMapRusherStatus->Text = "Status: Map Rushing, Remaining Maps: " + Convert::ToString(remainingMapCount);

	String^ testPortalData = "";
	for each(MapPath^ mapData in mapPath) {
		testPortalData += "Rushing to " + mapData->mapID.ToString() + " Map Name: " + getMap(mapData->mapID)->mapName + "\n";
		remainingMapCount--;
		MainForm::TheInstance->lbMapRusherStatus->Text = "Status: Map Rushing, Remaining Maps: " + Convert::ToString(remainingMapCount);
	}
	MessageBox::Show(testPortalData);

	MainForm::TheInstance->lbMapRusherStatus->Text = "Status: Map Rushing Complete";
	GlobalRefs::isMapRushing = false;
}

//Find maps with names starting with text entered so far
static void findMapNamesStartingWithStr(String^ str) {
	MainForm::TheInstance->lvMapRusherSearch->BeginUpdate();
	for each(MapData^ map in GlobalRefs::maps) {
		if(map->mapName->StartsWith(str)) {
			array<String^>^ row = { map->mapName, Convert::ToString(map->mapID) };
			ListViewItem^ lvi = gcnew ListViewItem(row);
			MainForm::TheInstance->lvMapRusherSearch->Items->Add(lvi);
		}
	}
	MainForm::TheInstance->lvMapRusherSearch->EndUpdate();
}

//Starts Map Rush when clicked
void MainForm::bMapRush_Click(System::Object^  sender, System::EventArgs^  e) {
	if(!GlobalRefs::isMapRushing && !PointerFuncs::getMapID()->Equals("0")) {
		int mapDestID = 0;
		if (INT::TryParse(tbMapRusherDestination->Text, mapDestID))
			if (mapDestID >= 0 && mapDestID <= 999999999)
				mapRush(mapDestID);
	}
}

//Adds tree view selected map's mapID to tbMapRusherDestination textbox
void MainForm::tvMapRusherSearch_MouseDoubleClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
	if (tvMapRusherSearch->SelectedNode == nullptr) return;
	if(dynamic_cast<MapData^>(tvMapRusherSearch->SelectedNode->Tag) != nullptr)
		tbMapRusherDestination->Text = tvMapRusherSearch->SelectedNode->Nodes[0]->Name;

	if(tvMapRusherSearch->SelectedNode->Tag->Equals("MapID"))
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

#pragma region testing
//Start of testing stuff
ULONG getStringValHookAddr = 0x0079E9A3;
ULONG getStringRetValHookAddr = 0x0079EA58;
char* maplestring;
__declspec(naked) static void __stdcall GetStringHook() {
	__asm {
		mov [ebp + 0x0C], 0x2
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
		jmp [getStringRetValHookAddr]
	}
} //Working

typedef char**(__stdcall* StringPool__GetString_t)(PVOID, PVOID, char**, UINT, UINT);	//typedef ZXString<char>*(__stdcall* StringPool__GetString_t)(PVOID, PVOID, ZXString<char>*, UINT);
StringPool__GetString_t StringPool__GetString = (StringPool__GetString_t)0x0079E993;	//0x00406455;

typedef char**(__stdcall *lpfnCItemInfo__GetMapString)(PVOID, PVOID, char*, UINT, const char*);
lpfnCItemInfo__GetMapString CItemInfo__GetMapString = (lpfnCItemInfo__GetMapString)0x005CF792;

//Test stuff out //https://pastebin.com/aULY72tG
void MainForm::button1_Click(System::Object^  sender, System::EventArgs^  e) {
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

//CMobPool UML Diagram
//V95 00C687B8+28]+4]+10C]+28]+60 = mob x || v83 00BEBFA4+28]4]120]24]60 = mob x
//&v3->m_pvcHead.m_pInterface->vfptr;
//vfptr[8].addRef
//What I know: in v95, 00C687B8 is a CMobPool pointer, +28 = ZList<ZRef<CMob*>>, + 4 = ZList<ZRef<CMob*>>.pHead (CMob) + 10C = m_pvcHead
//m_pvcHead.m_pInterface->vpptr is type IUnknown* (Microsoft's Component object model) 
//that points to some class (maybe CVecCtrlMob) but the offsets 28 and 60 doesn't seem point to an offset in CVecCtrl or CVecCtrl mob that is the xy coordinates of a mob
#pragma endregion
