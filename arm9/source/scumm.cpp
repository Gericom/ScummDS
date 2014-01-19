#include <nds.h>

#include <HE0.h>
#include <scumm.h>

FILE* HE0_File;
FILE* HE1_File;
FILE* HE2_File;
FILE* HE4_File;

FILE* HEx_File;

HE0_t HE0_Data;

uint32_t _heTimers[16];

uint16_t _mouse_x = 320;
uint16_t _mouse_y = 240;

uint16_t _cursor_state;
uint16_t _userPut;

uint32_t _hInFileTable[17];
uint32_t _hOutFileTable[17]; 

uint16_t gameHEVersion;

int getMillis()
{
	return ((TIMER1_DATA*(1<<16))+TIMER0_DATA)/32.7285;
}

int getHETimer(int timer) {
	int time = getMillis() - _heTimers[timer];
	return time;
}

void setHETimer(int timer) {
	_heTimers[timer] = getMillis();
}

void my_printf(char* a, ...){}