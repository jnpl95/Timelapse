#pragma once

// Taken from destiny its minimal value(official GMS0.83 start slots) max value is about 180
/*`EquipmentSlots` tinyint(3) UNSIGNED NOT NULL DEFAULT '24',
`UsableSlots` tinyint(3) UNSIGNED NOT NULL DEFAULT '24',
`SetupSlots` tinyint(3) UNSIGNED NOT NULL DEFAULT '24',
`EtceteraSlots` tinyint(3) UNSIGNED NOT NULL DEFAULT '24',
`CashSlots` tinyint(3) UNSIGNED NOT NULL DEFAULT '48',*/

typedef struct TabSlot{
	int index;
	bool occupied;
	int itemId;
	int itemQuantity;
} TabSlot;

typedef struct InventoryTab{
	TabSlot slot1;
	TabSlot slot2;
	TabSlot slot3;
	TabSlot slot4;
	TabSlot slot5;
	TabSlot slot6;
	TabSlot slot7;
	TabSlot slot8;
	TabSlot slot9;
	TabSlot slot10;
	TabSlot slot11;
	TabSlot slot12;
	TabSlot slot13;
	TabSlot slot14;
	TabSlot slot15;
	TabSlot slot16;
	TabSlot slot17;
	TabSlot slot18;
	TabSlot slot19;
	TabSlot slot20;
	TabSlot slot21;
	TabSlot slot22;
	TabSlot slot23;
	TabSlot slot24;
} InventoryTab;

typedef struct Inventory {
	InventoryTab EQUIP;
	InventoryTab USE;
	InventoryTab SET_UP;
	InventoryTab ETC;
	InventoryTab CASH;
} Inventory;

class CharInventory{
public:
	CharInventory();
	~CharInventory();
};
