#ifndef __AKOS_H__
#define __AKOS_H__
#include <actor.h>

struct AKHD_t
{
	uint16_t Unknown1;
	uint8_t Flags;
	uint8_t Unknown2;
	uint16_t NrAnims;
	uint16_t NrFrames;
	uint16_t Codec;
	uint16_t Unknown3;
};

struct AKOS_t
{
	AKHD_t AKHD;
	//void* RGBS;
	uint32_t AKSQLength;
	//fpos_t AKSQOffset;
	void* AKSQ;
	void* AKCH;
	fpos_t AKOFOffset;
	//void* AKOF;
	fpos_t AKCIOffset;
	//void* AKCI;
	fpos_t AKCDOffset;
	//void* AKCD;
	uint32_t AKSTLength;
	void* AKST;
	uint32_t AKSFLength;
	void* AKSF;
};

void freeAKOS(AKOS_t* AKOS);
void costumeDecodeData(Actor* a, int frame, uint usemask);
byte drawLimb(const Actor *a, int limb);
bool akos_increaseAnim(Actor *a, int chan);
void renderCostume(Actor* a);

#endif