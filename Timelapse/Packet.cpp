#include "Packet.h"
#include "Structs.h"
#include "GeneralFunctions.h"
#include "Log.h"

// TODO: rewrite in c++ WinSock Style
// Hooking client may not be available however hooking windows sockets api will be

// Addresses
ULONG clientSocketAddr = 0x00BE7914;
ULONG COutPacketAddr = 0x0049637B;
ULONG CInPacketAddr = 0x004965F1;
// Hooks
PVOID* ClientSocket = reinterpret_cast<PVOID*>(clientSocketAddr);
typedef void(__thiscall *PacketSend)(PVOID clientSocket, COutPacket* packet); //Send packet from client to server
PacketSend Send = reinterpret_cast<PacketSend>(COutPacketAddr);
typedef void(__thiscall *PacketRecv)(PVOID clientSocket, CInPacket* packet); //Receive packet from client to server
PacketRecv Recv = reinterpret_cast<PacketRecv>(CInPacketAddr);

void writeByte(String^ %packet, BYTE byte) {
	packet += byte.ToString("X2") + " ";
}

void writeBytes(String^ %packet, array<BYTE>^ bytes) {
	for each(BYTE byte in bytes) packet += byte.ToString("X2") + " ";
}

void writeString(String^ %packet, String^ str) {
	writeByte(packet, str->Length);
	writeByte(packet, 0);
	writeBytes(packet, Text::Encoding::UTF8->GetBytes(str));
}

void writeInt(String^ %packet, int num) {
	writeByte(packet, (BYTE)num);
	writeByte(packet, (BYTE)((UINT)num >> 8 & 0xFF));
	writeByte(packet, (BYTE)((UINT)num >> 16 & 0xFF));
	writeByte(packet, (BYTE)((UINT)num >> 24 & 0xFF));
}

void writeShort(String^ %packet, short num) {
	writeByte(packet, (BYTE)num);
	writeByte(packet, (BYTE)((UINT)num >> 8 & 0xFF));
}

void writeUnsignedShort(System::String^ %packet, USHORT num) {
	writeByte(packet, (BYTE)num);
	writeByte(packet, (BYTE)((UINT)num >> 8 & 0xFF));
}

bool IsValidRawPacket(String^ rawPacket) {
	if (String::IsNullOrEmpty(rawPacket)) {
		Log::WriteLineToConsole("SendPacket::ERROR: Packet is Empty!");
		return false;
	}
	for (int i = 0; i < rawPacket->Length; i++) {
		if (rawPacket[i] >= '0' && rawPacket[i] <= '9') continue;
		if (rawPacket[i] >= 'A' && rawPacket[i] <= 'F') continue;
		if (rawPacket[i] == '*') continue;

		Log::WriteLineToConsole("SendPacket::ERROR: Invalid character detected in packet: It contains a \"" + rawPacket[i] + "\"");
		return false;
	}

	return true;
}

// TODO: more error checking/thread safety
bool SendPacket(String^ packetStr)
{
	COutPacket Packet;
	SecureZeroMemory(&Packet, sizeof(COutPacket));
	// Clean whitespace
	String^ rawPacket = packetStr->Replace(" ", String::Empty);
	if (!IsValidRawPacket(rawPacket)) return false;
	// Create random bytes for "*"
	String^ processedPacket = rawPacket->Replace("*", (rand() % 16).ToString("X"));
	// Temp buffer
	BYTE tmpPacketStr[150];
	// 32-bit pointer to a constant null-terminated string of 8-bit Windows (ANSI) characters
	const LPCSTR lpcszPacket = static_cast<LPCSTR>(Runtime::InteropServices::Marshal::StringToHGlobalAnsi(processedPacket).ToPointer());
	// enforcing that packet size is a multiple of 2
	Packet.Size = strlen(lpcszPacket) / 2;
	if (Packet.Size % 2 == 1) {
		Log::WriteLineToConsole("SendPacket::ERROR: Packet size is not a multiple of 2!");
		return false;
	}
	// enforce only hex characters
	Packet.Data = atohx(tmpPacketStr, lpcszPacket);

	//try sending packet via Maplestory client packet hook
	try {
		Send(*ClientSocket, &Packet);
		return true;
	}
	catch (...) { return false; }
}

bool RecvPacket(String^ packetStr)
{
	CInPacket Packet;
	SecureZeroMemory(&Packet, sizeof(CInPacket));
	String^ rawPacket = packetStr->Replace(" ", String::Empty)->Replace("*", (rand() % 16).ToString("X"));
	BYTE tmpPacketStr[150];
	const LPCSTR lpcszPacket = (LPCSTR)(Runtime::InteropServices::Marshal::StringToHGlobalAnsi(rawPacket).ToPointer());

	Packet.Size = strlen(lpcszPacket) / 2;
	Packet.lpvData = atohx(tmpPacketStr, lpcszPacket);

	try {
		Recv(*ClientSocket, &Packet);
		return true;
	}
	catch (...) { return false; }
}
