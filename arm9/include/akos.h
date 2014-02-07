#ifndef __AKOS_H__
#define __AKOS_H__
#include <actor.h>

struct AKHD_t
{
	//uint16_t Unknown1;
	uint8_t Flags;
	//uint8_t Unknown2;
	uint16_t NrAnims;
	uint16_t NrFrames;
	uint16_t Codec;
	//uint16_t Unknown3;
};

struct AKOS_t
{
	AKHD_t AKHD;
	uint32_t AKPLLength;
	uint8_t AKPL[64];
	bool HasRGBS;
	uint8_t	RGBS[256 * 3];
	void* AKSQ;
	fpos_t AKCHOffset;
	fpos_t AKOFOffset;
	fpos_t AKCIOffset;
	fpos_t AKCDOffset;
	fpos_t AKCTOffset;
	uint32_t AKSTLength;
	fpos_t AKSTOffset;
	uint32_t AKSFLength;
	fpos_t AKSFOffset;
};

void freeAKOS(AKOS_t* AKOS);
void costumeDecodeData(Actor* a, int frame, uint usemask);
byte drawLimb(const Actor *a, int limb);
bool akos_increaseAnim(Actor *a, int chan);
void renderCostume(Actor* a, bool render = true);

#endif