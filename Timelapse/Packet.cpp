#include "Packet.h"
#include <string>
#include <Windows.h>
#include "MainForm.h"

//TODO: Define in Pointers.h instead of here
LPVOID* ClientSocket = (LPVOID*)0x00BE7914;
typedef void(__thiscall *PacketSend)(PVOID clientSocket, COutPacket* packet); //Send packet from client to server
PacketSend Send = (PacketSend)0x0049637B;

typedef void(__thiscall *PacketRecv)(PVOID clientSocket, CInPacket* packet); //Receive packet from client to server
PacketRecv Recv = (PacketRecv)0x004965F1;

inline PUCHAR atohx(PUCHAR szDestination, LPCSTR szSource)
{
	PUCHAR szReturn = szDestination;
	for (int lsb, msb; *szSource; szSource += 2)
	{
		msb = tolower(*szSource);
		lsb = tolower(*(szSource + 1));
		msb -= isdigit(msb) ? 0x30 : 0x57;
		lsb -= isdigit(lsb) ? 0x30 : 0x57;
		if ((msb < 0x0 || msb > 0xf) || (lsb < 0x0 || lsb > 0xf))
		{
			*szReturn = 0;
			return NULL;
		}
		*szDestination++ = (char)(lsb | (msb << 4));
	}
	*szDestination = 0;
	return szReturn;
}

bool SendPacket(System::String^ packetStr)
{
	COutPacket Packet;
	SecureZeroMemory(&Packet, sizeof(COutPacket));
	System::String^ rawPacket = packetStr->Replace(" ", System::String::Empty)->Replace("*", (rand() % 16).ToString("X"));
	byte tmpPacketStr[150];
	LPCSTR lpcszPacket = (LPCSTR)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(rawPacket).ToPointer());
	
	Packet.Size = strlen(lpcszPacket) / 2;
	Packet.Data = atohx(tmpPacketStr, lpcszPacket);
	
	try {
		Send(*ClientSocket, &Packet);
		return true;
	}
	catch (...) { return false; }
}

bool RecvPacket(System::String^ packetStr)
{
	CInPacket Packet;
	SecureZeroMemory(&Packet, sizeof(CInPacket));
	System::String^ rawPacket = packetStr->Replace(" ", System::String::Empty)->Replace("*", (rand() % 16).ToString("X"));
	byte tmpPacketStr[150];
	LPCSTR lpcszPacket = (LPCSTR)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(rawPacket).ToPointer());

	Packet.Size = strlen(lpcszPacket) / 2;
	Packet.lpvData = atohx(tmpPacketStr, lpcszPacket);

	try {
		Recv(*ClientSocket, &Packet);
		return true;
	}
	catch (...) { return false; }
}

/* WORK IN PROGRESS
void __stdcall RecordPacketsSent(COutPacket packet) {
	array<unsigned char> ^sDestination = gcnew array<unsigned char>(packet.Size);
	System::Runtime::InteropServices::Marshal::Copy(System::IntPtr((void*)packet.Data), sDestination, 0, packet.Size);
	System::String ^sResult = System::Text::Encoding::ASCII->GetString(sDestination);

	array<System::String^>^ row = { sResult };
	System::Windows::Forms::ListViewItem^ lvi = gcnew System::Windows::Forms::ListViewItem(row);
	Timelapse::MainForm::TheInstance->lvSend->Items->Add(lvi);
}

DWORD dwSendHookRet = 0x0049637B + 5;

#pragma unmanaged
void __stdcall SendPacketHook() {
	__asm {
		pushad
		push [esp+0x04] //Packet Struct? Need to find the correct pointer on the stack I think
		call RecordPacketsSent
		popad
		mov eax, 0x00A8126C //original code
		jmp dword ptr[dwSendHookRet]
	}
}*/