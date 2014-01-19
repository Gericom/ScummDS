#ifndef __ROOM_H__
#define __ROOM_H__
#include <HE1.h>

extern LFLF_t* RoomResource;
extern uint32_t _currentRoom;
extern uint32_t _roomResource;

struct Actor;

void startScene(int room, Actor *a, int objectNr);
void* getExitScript();
void* getEntryScript();

#endif