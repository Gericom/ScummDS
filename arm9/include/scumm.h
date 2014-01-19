#ifndef __SCUMM_H__
#define __SCUMM_H__
#include <HE0.h>

#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

template<typename T> inline void SWAP(T &a, T &b) { T tmp = a; a = b; b = tmp; }

//#define printf my_printf

//void my_printf(char* a, ...);

enum WhereIsObject {
	WIO_NOT_FOUND = -1,
	WIO_INVENTORY = 0,
	WIO_ROOM = 1,
	WIO_GLOBAL = 2,
	WIO_LOCAL = 3,
	WIO_FLOBJECT = 4
};

extern FILE* HE0_File;
extern FILE* HE1_File;
extern FILE* HE2_File;
extern FILE* HE4_File;

extern FILE* HEx_File;

extern HE0_t HE0_Data;

extern uint32_t _heTimers[16];

extern uint16_t _mouse_x;
extern uint16_t _mouse_y;

extern uint16_t _cursor_state;
extern uint16_t _userPut;

extern uint32_t _hInFileTable[17];
extern uint32_t _hOutFileTable[17];

extern uint16_t gameHEVersion;

int getMillis();
int getHETimer(int timer);
void setHETimer(int timer);

#endif