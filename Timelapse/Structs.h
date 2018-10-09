#ifndef STRUCTS_H
#define STRUCTS_H

#include <Windows.h>

struct SpawnControlStruct {
	UINT mapID;
	INT spawnX;
	INT spawnY;

	SpawnControlStruct(UINT mapID, INT spawnX, INT spawnY) {
		this->mapID = mapID;
		this->spawnX = spawnX;
		this->spawnY = spawnY;
	}
};

#endif