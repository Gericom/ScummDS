#include <nds.h>

#include <HE1.h>
#include <room.h>

uint8_t* _objectOwnerTable;
uint8_t* _objectStateTable;
uint8_t* _objectRoomTable;
uint32_t* _classData;
uint32_t _numGlobalObjects;

bool getClass(int obj, int cls)
{
	if(obj >= _numGlobalObjects || obj < 1) return 0;
	cls &= 0x7F;

	return (_classData[obj] & (1 << (cls - 1))) != 0;
}

void putClass(int obj, int cls, bool set)
{
	if(obj >= _numGlobalObjects || obj < 1) return;
	cls &= 0x7F;

	if (set)
		_classData[obj] |= (1 << (cls - 1));
	else
		_classData[obj] &= ~(1 << (cls - 1));
} 

int getOwner(int obj)
{
	if(obj >= _numGlobalObjects || obj < 1) return 0;
	return _objectOwnerTable[obj];
}

void putOwner(int obj, int owner) 
{
	if(obj >= _numGlobalObjects || obj < 1) return;
	_objectOwnerTable[obj] = owner;
}

int getState(int obj) 
{
	if(obj >= _numGlobalObjects || obj < 1) return 0;
	return _objectStateTable[obj];
}

void putState(int obj, int state) 
{
	if(obj >= _numGlobalObjects || obj < 1) return;
	_objectStateTable[obj] = state;
}

int getObjectIndex(int object) {
	int i;

	if (object < 1)
		return -1;

	for (i = (RoomResource->RMDA->RMHD.NrObjects - 1); i > 0; i--) {
		if (RoomResource->RMDA->OBCD[i]->ObjectId == object)
			return i;
	}
	return -1;
}

//int findObject(int x, int y)
//{
//	int i, b;
//	byte a;
//	const int mask = 0xF;
//
//	for (i = /*1*/0; i < /*_numLocalObjects*/RoomResource->RMDA->RMHD.NrObjects; i++) {
//		if ((RoomResource->RMDA->OBCD[i]->ObjectId < 1) || getClass(RoomResource->RMDA->OBCD[i]->ObjectId, kObjectClassUntouchable))
//			continue;
//
//		b = i;
//		do {
//			a = ((RoomResource->RMDA->OBCD[b]->Flags == 0x80) ? 1 : (RoomResource->RMDA->OBCD[b]->Flags & 0xF));
//			b = RoomResource->RMDA->OBCD[b]->Parent;
//			if (b == 0) {
//				//if (_game.heversion >= 71) {
//					//if (((ScummEngine_v71he *)this)->_wiz->polygonHit(_objs[i].obj_nr, x, y))
//					//	return _objs[i].obj_nr;
//				//}
//				if (RoomResource->RMDA->OBCD[i]->X <= x && RoomResource->RMDA->OBCD[i]->Width + RoomResource->RMDA->OBCD[i]->X > x &&
//				    RoomResource->RMDA->OBCD[i]->Y <= y && RoomResource->RMDA->OBCD[i]->Height + RoomResource->RMDA->OBCD[i]->Y > y)
//					return _objs[i].obj_nr;
//				break;
//			}
//		} while ((RoomResource->RMDA->OBCD[i]->state & mask) == a);
//	}
//
//	return 0;
//} 