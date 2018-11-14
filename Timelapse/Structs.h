#ifndef STRUCTS_H
#define STRUCTS_H

#include <Windows.h>

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


#endif