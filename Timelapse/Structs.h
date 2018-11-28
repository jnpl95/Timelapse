#pragma once
#include <Windows.h>

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

struct SpawnControlData {
	UINT mapID;
	INT spawnX;
	INT spawnY;

	SpawnControlData(UINT mapID, INT spawnX, INT spawnY) {
		this->mapID = mapID;
		this->spawnX = spawnX;
		this->spawnY = spawnY;
	}
};

//Managed struct for holding Portal data for each map in MapData
ref struct PortalData {
	System::String^ portalName;
	int portalType;
	int xPos;
	int yPos;
	int toMapID;
};

//Managed struct for holding Map data for map rusher
ref struct MapData {
	int mapID;
	System::String^ islandName;
	System::String^ streetName;
	System::String^ mapName;
	System::Collections::Generic::List<PortalData^>^ portals;
};

//Managed struct for holding Map Route data for map rusher
ref struct MapPath {
	int mapID;
	PortalData^ portal;
	MapPath(int mapID, PortalData^ portal) {
		this->mapID = mapID;
		this->portal = portal;
	}
};