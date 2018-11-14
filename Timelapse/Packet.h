#ifndef PACKET_H
#define PACKET_H

#include <Windows.h>

bool SendPacket(System::String^ packetStr);
bool RecvPacket(System::String^ packetStr);
void __stdcall SendPacketHook();
//void __stdcall RecordPacketsSent(COutPacket packet);
void writeByte(System::String^ %packet, BYTE byte);
void writeBytes(System::String^ %packet, array<BYTE>^ bytes);
void writeString(System::String^ %packet, System::String^ str);
void writeInt(System::String^ %packet, int num);
void writeShort(System::String^ %packet, short num);

struct COutPacket {
	int Loopback;
	union {
		PUCHAR Data;
		PVOID Unk;
		PUSHORT Header;
	};
	ULONG Size;
	UINT Offset;
	int EncryptedByShanda;
};

struct CInPacket {
	bool fLoopback;
	int iState;
	union {
		PVOID lpvData;
		struct {
			ULONG dw;
			USHORT wHeader;
		} *pHeader;
		struct {
			ULONG dw;
			PUCHAR Data;
		} *pData;
	};
	ULONG Size;
	USHORT usRawSeq;
	USHORT usDataLen;
	USHORT usUnknown;
	UINT uOffset;
	PVOID lpv;
};

#endif