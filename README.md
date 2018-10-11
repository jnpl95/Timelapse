# Timelapse
A MapleStory v83 Trainer

![alt text](https://i.imgur.com/vLhLOQv.png)

TODO By Trainer Tab: 
ToolBar:
	- Code Save/Load for all options, Enable/Disable All
	- Code Hide/Show MS Window (already coded in old trainer)
	- Code Embed MS (already coded in old trainer)
	- Code inject dll (already coded in old trainer)
Main:
	AutoLogin:
		- (ADD) AutoLogin
		- Code Skip Logo (already found in .CT)
	Options: 
		- Code disable pointers
Bots: 
	Main: 
		- (ADD) Auto Turn? 
	AutoCC/CS: 
		- (ADD) Add option for auto face left after cc/cs
		- Fix AutoCC tbCCCSTime, tbCCCSPeople, tbCCCSAttack, tbCCCSMob empty/null (add error message) :: if time == 0, change to 1 second
		- Fix Auto CS function call (doesn't work as intended, get rid of blue box popup on MigrateToCashShop Call)
		- Make Auto CC/CS run on threads instead of timers. Timers cause the trainer to hang up 
	Advanced Bot(ADD): 
		- (ADD)Flat bot
		- (ADD) climbing ladders/rope (CVecCtrl::GetLadderOrRope/CVecCtrl::IsOnLadder/CVecCtrl::IsOnRope 
		- (ADD)Find nearest mob/most grouped mobs in a platform (spiral algo with rings started from inside (for loop with 2 for loops inside)
		- (ADD) level based map rush
Hacks: 
	- (ADD) Char Walk & Jump Speed editing(double), damage, and other physics, 
	- Auto Assign AP (make sure packet auto ap not enabled) 
Vacs: 
	- Code DupeX/MMC/Wall Vac (already coded in old trainer)
	- (ADD) Vami, Portal Kami, dEMi, CSEAX vac if possible

Filters: 
	- Code mob filters/item filters & corresponding data (already coded in old trainer)
	- Modifications?
Packets: 
	- (ADD) code all block packets
	- (ADD) code logging send/recv packets
	- (ADD) code multi packet for send & recv
	- (ADD) code/find defined packets including Auto AP, Auto-Sell, intercontinental packets if possible 
Map Rusher: 
	- Code Map Rusher (already coded in old trainer)
	- (ADD) Make Map Rush intercontinental
	- (ADD) Code Auto-Rushback
	- (ADD) Code allowing for both packet based and teleport based map rush
Other: 
	- Look into making sure trainer can be injected into multipleinstances with no interference between
	- (ADD) check for auto-loot to see if inventory is full, add option to sell useless shit and buy pots 
	- (ADD) Make a label visible for a bit (or log) all options in trainer
	- (ADD) Make Tooltips for everything
	- (ADD) Logger for mesos picked up/mobs killed/# of levels leveled up during hacking session
	- (ADD) countdown for Auto CC/CS on bottom of trainer (look into replacing useless time label on bottom right)
	- (ADD) chat messages for style points hehe

NOTES: 
	- Use Stealth Inject option in Settings for Extreme Injector
	- For all new number only textboxes, double check to see if the keypress event (in MainForm.h) is defined


Temp Unsorted TODO:  

- Fix ClickTeleport() tbClickTeleport, MouseTeleport() tbMouseTeleport, TeleportLoop() tbTeleportLoopDelay being empty
- Check and fix all textbox error messages
- Add spawn point dynamic stuff to Codecave and see if Strucs.h/SpawnControlStruct is necessary ??? maybe in helper function in codecave::, loop through listviews and return a dynamically constructed struct/array
- Look into seeing if there is a pointer to see if map has reloaded (for map rush & auto cc/cs), otherwise users can click multiple times, causing the function to complete multiple times before cc'ing is actually finished
- Fix null pointer exception for reading char for level pointer



