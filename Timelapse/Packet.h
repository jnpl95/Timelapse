#pragma once
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