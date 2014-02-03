#ifndef __OBJECTS_H__
#define __OBJECTS_H__

extern uint8_t* _objectOwnerTable;
extern uint8_t* _objectStateTable;
extern uint8_t* _objectRoomTable;
extern uint32_t* _classData;
extern uint32_t _numGlobalObjects;

enum ObjectClass {
	kObjectClassNeverClip = 20,
	kObjectClassAlwaysClip = 21,
	kObjectClassIgnoreBoxes = 22,
	kObjectClassYFlip = 29,
	kObjectClassXFlip = 30,
	kObjectClassPlayer = 31,	// Actor is controlled by the player
	kObjectClassUntouchable = 32
};

bool getClass(int obj, int cls);
void putClass(int obj, int cls, bool set);
int getOwner(int obj);
void putOwner(int obj, int owner);
int getState(int obj);
void putState(int obj, int state);
int getObjectIndex(int object);
//int findObject(int x, int y);

void drawObject(int obj, int arg);
void processDrawQue();
void addObjectToDrawQue(int object);
void removeObjectFromDrawQue(int object);
void clearDrawObjectQueue();
void clearDrawQueues();

#endif