#include "Packet.h"
#include "Structs.h"

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

void writeByte(System::String^ %packet, BYTE byte) {
	packet += byte.ToString("X2") + " ";
}

void writeBytes(System::String^ %packet, array<BYTE>^ bytes) {
	for each(BYTE byte in bytes) packet += byte.ToString("X2") + " ";
}

void writeString(System::String^ %packet, System::String^ str) {
	writeByte(packet, str->Length);
	writeByte(packet, 0);
	writeBytes(packet, System::Text::Encoding::UTF8->GetBytes(str));
}

void writeInt(System::String^ %packet, int num) {
	writeByte(packet, (BYTE)num);
	writeByte(packet, (BYTE)((UINT)num >> 8 & 0xFF));
	writeByte(packet, (BYTE)((UINT)num >> 16 & 0xFF));
	writeByte(packet, (BYTE)((UINT)num >> 24 & 0xFF));
}

void writeShort(System::String^ %packet, short num) {
	writeByte(packet, (BYTE)num);
	writeByte(packet, (BYTE)((UINT)num >> 8 & 0xFF));
}

void writeUnsignedShort(System::String^ %packet, USHORT num) {
	writeByte(packet, (BYTE)num);
	writeByte(packet, (BYTE)((UINT)num >> 8 & 0xFF));
}

inline PUCHAR atohx(PUCHAR szDestination, LPCSTR szSource)
{
	const PUCHAR szReturn = szDestination;
	for (int lsb, msb; *szSource; szSource += 2)
	{
		msb = tolower(*szSource);
		lsb = tolower(*(szSource + 1));
		msb -= isdigit(msb) ? 0x30 : 0x57;
		lsb -= isdigit(lsb) ? 0x30 : 0x57;
		if ((msb < 0x0 || msb > 0xf) || (lsb < 0x0 || lsb > 0xf))
		{
			*szReturn = 0;
			return nullptr;
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
	const LPCSTR lpcszPacket = (LPCSTR)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(rawPacket).ToPointer());

	Packet.Size = strlen(lpcszPacket) / 2;
	Packet.lpvData = atohx(tmpPacketStr, lpcszPacket);

	try {
		Recv(*ClientSocket, &Packet);
		return true;
	}
	catch (...) { return false; }
}
