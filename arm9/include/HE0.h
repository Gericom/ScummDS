#ifndef __HE0_H__
#define __HE0_H__
#include <stdio.h>

enum {
	OF_OWNER_MASK = 0x0F,
	OF_STATE_MASK = 0xF0,

	OF_STATE_SHL = 4
};

/*typedef struct
{
	uint16_t NrRooms;
	uint8_t RoomIds[1];
	uint32_t FileNumbers[1];
	uint32_t RoomDataOffsets[1];
} DIRI_t;

typedef struct
{
	uint16_t NrRooms;
	uint8_t RoomIds[1];
	uint32_t RoomDataOffsets[1];
	uint32_t RoomDataLengths[1];
} DIRR_t;

typedef struct
{
	uint16_t NrScripts;
	uint8_t RoomIds[1];
	uint32_t ScriptDataOffsets[1];
	uint32_t ScriptDataLengths[1];
} DIRS_t;

typedef struct
{
	uint16_t NrSounds;
	uint8_t RoomIds[1];
	uint32_t SoundOffsets[1];
	uint32_t SoundLengths[1];
} DIRN_t;

typedef struct
{
	uint16_t NrCostumes;
	uint8_t RoomIds[1];
	uint32_t CostumeOffsets[1];
	uint32_t CostumeLengths[1];
} DIRC_t;

typedef struct
{
	uint16_t NrCharsets;
	uint8_t RoomIds[1];
	uint32_t CharsetOffsets[1];
	uint32_t CharsetLengths[1];
} DIRF_t;

typedef struct
{
	uint16_t NrAWIZ;
	uint8_t RoomIds[1];
	uint32_t AWIZOffsets[1];
	uint32_t AWIZLengths[1];
} DIRM_t;

typedef struct
{
	uint16_t NrTalkies;
	uint8_t RoomIds[1];
	uint32_t TalkieOffsets[1];
	uint32_t TalkieLengths[1];
} DIRT_t;

typedef struct
{
	uint16_t NrSKs;
	uint8_t RoomIds[1];
} DISK_t;

typedef struct
{
	uint16_t NrLittleFileLists;
	uint32_t LittleFileListOffsets[1];
} DLFL_t;*/

struct HE0_t
{
	fpos_t MAXS;
	fpos_t DIRI;
	fpos_t DIRR;
	fpos_t DIRS;
	fpos_t DIRN;
	fpos_t DIRC;
	fpos_t DIRF;
	fpos_t DIRM;
	fpos_t DIRT;
	fpos_t DISK;
	fpos_t DLFL;
};

extern uint32_t HE0_Start;
extern uint32_t HE0_Length;
extern uint32_t HE1_Start;
extern uint32_t HE2_Start;
extern uint32_t HE4_Start;

extern uint32_t HE8_Start;

void readHE0(FILE* handle, HE0_t* HE0);

uint16_t getCostumeCount(FILE* handle, HE0_t* HE0);
uint32_t getCostumeOffset(FILE* handle, HE0_t* HE0, int idx);

uint16_t getAWIZCount(FILE* handle, HE0_t* HE0);
uint32_t getAWIZOffset(FILE* handle, HE0_t* HE0, int idx);

uint16_t getSoundCount(FILE* handle, HE0_t* HE0);
uint32_t getSoundOffset(FILE* handle, HE0_t* HE0, int idx);

uint16_t getScriptCount(FILE* handle, HE0_t* HE0);
uint32_t getScriptOffset(FILE* handle, HE0_t* HE0, int idx);
uint32_t getScriptLength(FILE* handle, HE0_t* HE0, int idx);

uint16_t getLFLFCount(FILE* handle, HE0_t* HE0);
uint32_t getLFLFOffset(FILE* handle, HE0_t* HE0, int idx);

#endif