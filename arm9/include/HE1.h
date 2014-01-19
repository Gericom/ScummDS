#ifndef __HE1_H__
#define __HE1_H__
#include <stdio.h>

struct RMIH_t
{
	uint16_t NrZBuffer;
};

struct BMAP_t
{
	fpos_t dataOffset;
	//uint8_t data[1];
};

struct IM_t
{
	BMAP_t BMAP;
};

struct ZP_t
{
	uint8_t data[1];
};

struct RMIM_t
{
	RMIH_t RMIH;
	IM_t IM;
	//ZP_t* ZP[1];
};

struct RMHD_t
{
	uint16_t RoomWidth;
	uint16_t RoomHeight;
	uint16_t NrObjects;
};

struct OFFS_t
{
	uint32_t Offsets[1];
};

struct APAL_t
{
	uint8_t data[1];
};

struct WRAP_t
{
	OFFS_t* OFFS;
	APAL_t* APAL;
};

struct PALS_t
{
	WRAP_t* WRAP;
};

struct OCDH_t
{
	uint16_t ObjectID;
	int16_t X;
	int16_t Y;
	uint16_t Width;
	uint16_t Height;
	uint8_t Flags;
	uint8_t Parent;
	uint16_t XWalk;
	uint16_t YWalk;
	uint8_t ActorDirection;
};

struct OBCD_t
{
	uint16_t ObjectId;
	int16_t X;
	int16_t Y;
	uint16_t Width;
	uint16_t Height;
	uint8_t Flags;
	uint8_t Parent;
	fpos_t OCDHOffset;
	uint32_t VERBLength;
	fpos_t VERBOffset;
	fpos_t OBNAOffset;
};

struct RMDA_t
{
	RMHD_t RMHD;
	uint16_t TransClrIdx;
	PALS_t* PALS;
	OBCD_t** OBCD;
	fpos_t EXCD;
	fpos_t ENCD;
	uint16_t NrLocalScripts;
	uint16_t* LocalScriptIds;
	fpos_t* LocalScriptOffsets;
	uint32_t* LocalScriptLengths;
};

struct LFLF_t
{
	RMIM_t RMIM;
	RMDA_t* RMDA;
};

void freeLFLF(LFLF_t* Room);

void readRoom(FILE* handle, LFLF_t* LFLF, uint32_t offset);

#endif