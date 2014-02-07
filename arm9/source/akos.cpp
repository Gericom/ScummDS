#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <ioutil.h>
#include <actor.h>
#include <scumm.h>
#include <akos.h>
#include <sounds.h>
#include <render.h>
#include <graphics.h>

struct AkosOffset {
	uint32 akcd;	// offset into the akcd data
	uint16 akci;	// offset into the akci data
};// PACKED_STRUCT;

enum AkosOpcodes {
	AKC_Return = 0xC001,
	AKC_SetVar = 0xC010,
	AKC_CmdQue3 = 0xC015,
	AKC_C016 = 0xC016,
	AKC_C017 = 0xC017,
	AKC_C018 = 0xC018,
	AKC_C019 = 0xC019,
	AKC_ComplexChan = 0xC020,
	AKC_C021 = 0xC021,
	AKC_C022 = 0xC022,
	AKC_ComplexChan2 = 0xC025,
	AKC_Jump = 0xC030,
	AKC_JumpIfSet = 0xC031,
	AKC_AddVar = 0xC040,
	AKC_C042 = 0xC042,
	AKC_C044 = 0xC044,
	AKC_C045 = 0xC045,
	AKC_C046 = 0xC046,
	AKC_C047 = 0xC047,
	AKC_C048 = 0xC048,
	AKC_Ignore = 0xC050,
	AKC_IncVar = 0xC060,
	AKC_CmdQue3Quick = 0xC061,
	AKC_JumpStart = 0xC070,
	AKC_JumpE = 0xC070,
	AKC_JumpNE = 0xC071,
	AKC_JumpL = 0xC072,
	AKC_JumpLE = 0xC073,
	AKC_JumpG = 0xC074,
	AKC_JumpGE = 0xC075,
	AKC_StartAnim = 0xC080,
	AKC_StartVarAnim = 0xC081,
	AKC_Random = 0xC082,
	AKC_SetActorClip = 0xC083,
	AKC_StartAnimInActor = 0xC084,
	AKC_SetVarInActor = 0xC085,
	AKC_HideActor = 0xC086,
	AKC_SetDrawOffs = 0xC087,
	AKC_JumpTable = 0xC088,
	AKC_SoundStuff = 0xC089,
	AKC_Flip = 0xC08A,
	AKC_Cmd3 = 0xC08B,
	AKC_Ignore3 = 0xC08C,
	AKC_Ignore2 = 0xC08D,
	AKC_C08E = 0xC08E,
	AKC_SkipStart = 0xC090,
	AKC_SkipE = 0xC090,
	AKC_SkipNE = 0xC091,
	AKC_SkipL = 0xC092,
	AKC_SkipLE = 0xC093,
	AKC_SkipG = 0xC094,
	AKC_SkipGE = 0xC095,
	AKC_ClearFlag = 0xC09F,
	AKC_C0A0 = 0xC0A0,
	AKC_C0A1 = 0xC0A1,
	AKC_C0A2 = 0xC0A2,
	AKC_C0A3 = 0xC0A3,
	AKC_C0A4 = 0xC0A4,
	AKC_C0A5 = 0xC0A5,
	AKC_C0A6 = 0xC0A6,
	AKC_C0A7 = 0xC0A7,
	AKC_EndSeq = 0xC0FF
};

struct CostumeInfo {
	uint16 width, height;
	//int16 rel_x, rel_y;
	//int16 move_x, move_y;
} ;//PACKED_STRUCT;

static bool akos_compare(int a, int b, byte cmd) {
	switch (cmd) {
	case 0:
		return a == b;
	case 1:
		return a != b;
	case 2:
		return a < b;
	case 3:
		return a <= b;
	case 4:
		return a > b;
	default:
		return a >= b;
	}
}

void freeAKOS(AKOS_t* AKOS)
{
	//free(AKOS->AKPL);
	//free(AKOS->RGBS);
	free(AKOS->AKSQ);
	//free(AKOS->AKCH);
	//free(AKOS->AKOF);
	//free(AKOS->AKCI);
	//free(AKOS->AKCD);
	//free(AKOS->AKST);
	//free(AKOS->AKSF);
	free(AKOS);
}

AKOS_t* readAKOS()
{
	uint32_t sig;
	uint32_t size;
	fpos_t pos;
	readU32LE(HE1_File, &sig);
	//printf("Costume: %c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
	readU32LE(HE1_File, &size);
	fgetpos(HE1_File, &pos);
	uint32_t end = pos + SWAP_CONSTANT_32(size) - 8;
	AKOS_t* AKOS = (AKOS_t*)malloc(sizeof(AKOS_t));
	memset(AKOS, 0, sizeof(AKOS_t));
	//AKOS->AKPL = NULL;
	//AKOS->RGBS = NULL;
	//AKOS->AKST = NULL;
	//AKOS->AKCTOffset = 0;
	//memset(AKOS->RGBS, 0, sizeof(AKOS->RGBS));
	AKOS->HasRGBS = false;
	while(pos < (end - 4))
	{
		readU32LE(HE1_File, &sig);
		//printf("%c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
		readU32LE(HE1_File, &size);
		switch(sig)
		{
		case MKTAG('A', 'K', 'H', 'D'):
			fseek(HE1_File, 2, SEEK_CUR);
			//readU16LE(HE1_File, &AKOS->AKHD.Unknown1);
			readByte(HE1_File, &AKOS->AKHD.Flags);
			fseek(HE1_File, 1, SEEK_CUR);
			//readByte(HE1_File, &AKOS->AKHD.Unknown2);
			readU16LE(HE1_File, &AKOS->AKHD.NrAnims);
			readU16LE(HE1_File, &AKOS->AKHD.NrFrames);
			readU16LE(HE1_File, &AKOS->AKHD.Codec);
			fseek(HE1_File, 2, SEEK_CUR);
			//readU16LE(HE1_File, &AKOS->AKHD.Unknown3);
			break;
		case MKTAG('A', 'K', 'P', 'L'):
			AKOS->AKPLLength = SWAP_CONSTANT_32(size) - 8;
			readBytes(HE1_File, &AKOS->AKPL[0], SWAP_CONSTANT_32(size) - 8);
			break;
		case MKTAG('R', 'G', 'B', 'S'):
			//AKOS->RGBS = malloc(SWAP_CONSTANT_32(size) - 8);
			readBytes(HE1_File, &AKOS->RGBS[0], SWAP_CONSTANT_32(size) - 8);
			AKOS->HasRGBS = true;
			break;
		case MKTAG('A', 'K', 'S', 'Q'):
			//AKOS->AKSQLength = SWAP_CONSTANT_32(size) - 8;
			AKOS->AKSQ = malloc(SWAP_CONSTANT_32(size) - 8);
			readBytes(HE1_File, (uint8_t*)AKOS->AKSQ, SWAP_CONSTANT_32(size) - 8);
			break;
		case MKTAG('A', 'K', 'C', 'H'):
			fgetpos(HE1_File, &AKOS->AKCHOffset);
			//AKOS->AKCH = malloc(SWAP_CONSTANT_32(size) - 8);
			//readBytes(HE1_File, (uint8_t*)AKOS->AKCH, SWAP_CONSTANT_32(size) - 8);
			fseek(HE1_File, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		case MKTAG('A', 'K', 'O', 'F'):
			fgetpos(HE1_File, &AKOS->AKOFOffset);
			fseek(HE1_File, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		case MKTAG('A', 'K', 'C', 'I'):
			fgetpos(HE1_File, &AKOS->AKCIOffset);
			fseek(HE1_File, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		case MKTAG('A', 'K', 'C', 'D'):
			fgetpos(HE1_File, &AKOS->AKCDOffset);
			fseek(HE1_File, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		case MKTAG('A', 'K', 'C', 'T'):
			fgetpos(HE1_File, &AKOS->AKCTOffset);
			fseek(HE1_File, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		case MKTAG('A', 'K', 'S', 'T'):
			AKOS->AKSTLength = SWAP_CONSTANT_32(size) - 8;
			fgetpos(HE1_File, &AKOS->AKSTOffset);
			fseek(HE1_File, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			//AKOS->AKSTLength = SWAP_CONSTANT_32(size) - 8;
			//AKOS->AKST = malloc(SWAP_CONSTANT_32(size) - 8);
			//readBytes(HE1_File, (uint8_t*)AKOS->AKST, SWAP_CONSTANT_32(size) - 8);
			break;
		case MKTAG('A', 'K', 'S', 'F'):
			AKOS->AKSFLength = SWAP_CONSTANT_32(size) - 8;
			fgetpos(HE1_File, &AKOS->AKSFOffset);
			fseek(HE1_File, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			//AKOS->AKSF = malloc(SWAP_CONSTANT_32(size) - 8);
			//readBytes(HE1_File, (uint8_t*)AKOS->AKSF, SWAP_CONSTANT_32(size) - 8);
			break;
		default:
			//printf("Unknown Signature\n");
			fseek(HE1_File, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		}
		fgetpos(HE1_File, &pos);
	}
	return AKOS;

}

//static int nrakos = 0;

void costumeDecodeData(Actor *a, int frame, uint usemask)
{
	//return;
	uint anim;
	uint8_t* r;
	//const AkosHeader *akhd;
	uint16 offs;
	int i;
	byte code;
	uint16 start, len;
	uint16 mask;

	if(a->_cost.AKOS != NULL)
	{
		freeAKOS(a->_cost.AKOS);
		a->_cost.AKOS = NULL;
	}

	if (a->_costume == 0)
		return;

	uint32_t offs_ = getCostumeOffset(HE0_File, &HE0_Data, a->_costume);
	fseek(HE1_File, offs_, SEEK_SET);

	AKOS_t* AKOS = readAKOS();

	anim = newDirToOldDir(getFacing(a)) + frame * 4;

	if (anim >= AKOS->AKHD.NrAnims)
	{
		freeAKOS(AKOS);
		printf("anim (%d) >= AKOS->AKHD.NrAnims (%d)\n", anim, AKOS->AKHD.NrAnims);
		while(1);
		return;
	}

	fseek(HE1_File, AKOS->AKCHOffset + anim * 2, SEEK_SET);

	readU16LE(HE1_File, &offs);
	if (offs == 0)
	{
		freeAKOS(AKOS);
		//printf("offs == 0\n");
		//while(1);
		return;
	}

	//while(scanKeys(), keysHeld() == 0);
	//swiDelay(5000000);

	fseek(HE1_File, AKOS->AKCHOffset + offs, SEEK_SET);

	//r += offs;

	uint8* akstPtr = NULL;//(uint8_t*)AKOS->AKST;
	//uint8* aksfPtr = (uint8_t*)AKOS->AKSF;

	i = 0;
	readU16LE(HE1_File, &mask);
	do {
		if (mask & 0x8000) {
			uint8* akst = akstPtr;
			//uint8* aksf = aksfPtr;

			readByte(HE1_File, &code);
			if (usemask & 0x8000) {
				switch (code) {
				case 1:
					a->_cost.active[i] = 0;
					a->_cost.frame[i] = frame;
					a->_cost.end[i] = 0;
					a->_cost.start[i] = 0;
					a->_cost.curpos[i] = 0;
					a->_cost.heCondMaskTable[i] = 0;

					if(AKOS->AKSTOffset != 0)
					{
						fpos_t curpos;
						fgetpos(HE1_File, &curpos);
						fseek(HE1_File, AKOS->AKSTOffset, SEEK_SET);
						int size = AKOS->AKSTLength / 8;
						if (size > 0) {
							bool found = false;
							while (size--) {
								uint32_t tmp;
								readU32LE(HE1_File, &tmp);
								if (/*(akst[0] | akst[1] << 8 | akst[2] << 16 | akst[3] << 24)*/tmp == 0) {
									//a->_cost.heCondMaskTable[i] = (akst[4] | akst[5] << 8 | akst[6] << 16 | akst[7] << 24);//READ_LE_UINT32(akst + 4);
									readU32LE(HE1_File, &a->_cost.heCondMaskTable[i]);
									found = true;
									break;
								}
								//akst += 8;
							}
							if (!found) {
								printf("Error: Sequence not found in actor %p costume %d\n", (void *)a, a->_costume);
							}
						}
						fseek(HE1_File, curpos, SEEK_SET);
					}

					/*if (akst) {
						int size = AKOS->AKSTLength / 8;
						if (size > 0) {
							bool found = false;
							while (size--) {
								if ((akst[0] | akst[1] << 8 | akst[2] << 16 | akst[3] << 24) == 0) {
									a->_cost.heCondMaskTable[i] = (akst[4] | akst[5] << 8 | akst[6] << 16 | akst[7] << 24);//READ_LE_UINT32(akst + 4);
									found = true;
									break;
								}
								akst += 8;
							}
							if (!found) {
								printf("Error: Sequence not found in actor %p costume %d\n", (void *)a, a->_costume);
							}
						}
					}*/
					break;
				case 4:
					a->_cost.stopped |= 1 << i;
					break;
				case 5:
					a->_cost.stopped &= ~(1 << i);
					break;
				default:
					readU16LE(HE1_File, &start);
					readU16LE(HE1_File, &len);
					//start = (r[0] | r[1] << 8); r += 2;
					//len = (r[0] | r[1] << 8); r += 2;

					/*printf("i = %d\n", i);
					printf("code = %d\n", code);
					printf("start = %d\n", start);
					printf("len = %d\n", len);

					while(scanKeys(), keysHeld() == 0);
					swiDelay(5000000);*/

					a->_cost.heJumpOffsetTable[i] = 0;
					a->_cost.heJumpCountTable[i] = 0;

					if(AKOS->AKSFOffset != 0)
					{
						fpos_t curpos;
						fgetpos(HE1_File, &curpos);
						fseek(HE1_File, AKOS->AKSFOffset, SEEK_SET);
						int size = AKOS->AKSFLength / 6;
						if (size > 0) {
							bool found = false;
							while (size--) {
								uint16_t tmp;
								readU16LE(HE1_File, &tmp);
								if (/*(aksf[0] | aksf[1] << 8)*/tmp == start) {
									//a->_cost.heJumpOffsetTable[i] = (aksf[2] | aksf[3] << 8);
									//a->_cost.heJumpCountTable[i] =  (aksf[4] | aksf[5] << 8);
									readU16LE(HE1_File, &a->_cost.heJumpOffsetTable[i]);
									readU16LE(HE1_File, &a->_cost.heJumpCountTable[i]);
									found = true;
									break;
								}
								//aksf += 6;
							}
							if (!found) {
								printf("Error: Sequence not found in actor %p costume %d\n", (void *)a, a->_costume);
							}
						}
						fseek(HE1_File, curpos, SEEK_SET);
					}

					/*if (aksf) {
					int size = AKOS->AKSFLength / 6;
					if (size > 0) {
					bool found = false;
					while (size--) {
					if ((aksf[0] | aksf[1] << 8) == start) {
					a->_cost.heJumpOffsetTable[i] = (aksf[2] | aksf[3] << 8);
					a->_cost.heJumpCountTable[i] =  (aksf[4] | aksf[5] << 8);
					found = true;
					break;
					}
					aksf += 6;
					}
					if (!found) {
					printf("Error: Sequence not found in actor %p costume %d\n", (void *)a, a->_costume);
					}
					}
					}*/

					a->_cost.active[i] = code;
					a->_cost.frame[i] = frame;
					a->_cost.end[i] = start + len;
					a->_cost.start[i] = start;
					a->_cost.curpos[i] = start;
					a->_cost.heCondMaskTable[i] = 0;

					if(AKOS->AKSTOffset != 0)
					{
						fpos_t curpos;
						fgetpos(HE1_File, &curpos);
						fseek(HE1_File, AKOS->AKSTOffset, SEEK_SET);
						int size = AKOS->AKSTLength / 8;
						if (size > 0) {
							bool found = false;
							while (size--) {
								uint32_t tmp;
								readU32LE(HE1_File, &tmp);
								if (/*(akst[0] | akst[1] << 8 | akst[2] << 16 | akst[3] << 24)*/tmp == start) {
									//a->_cost.heCondMaskTable[i] = (akst[4] | akst[5] << 8 | akst[6] << 16 | akst[7] << 24);//READ_LE_UINT32(akst + 4);
									readU32LE(HE1_File, &a->_cost.heCondMaskTable[i]);
									found = true;
									break;
								}
								//akst += 8;
							}
							if (!found) {
								printf("Error: Sequence not found in actor %p costume %d\n", (void *)a, a->_costume);
							}
						}
						fseek(HE1_File, curpos, SEEK_SET);
					}

					/*if (akst) {
						int size = AKOS->AKSTLength / 8;
						if (size > 0) {
							bool found = false;
							while (size--) {
								if ((akst[0] | akst[1] << 8 | akst[2] << 16 | akst[3] << 24) == start) {
									a->_cost.heCondMaskTable[i] = (akst[4] | akst[5] << 8 | akst[6] << 16 | akst[7] << 24);
									found = true;
									break;
								}
								akst += 8;
							}
							if (!found) {
								printf("Error: Sequence not found in actor %p costume %d\n", (void *)a, a->_costume);
							}
						}
					}*/
					break;
				}
			} else {
				if (code != 1 && code != 4 && code != 5)
					fseek(HE1_File, 4, SEEK_CUR);
			}
		}
		i++;
		mask <<= 1;
		usemask <<= 1;
	} while ((uint16)mask);
	a->_cost.AKOS = AKOS;
}

byte drawLimb(const Actor *a, int limb) {
	//from baserender:
	int _xmove, _ymove;
	uint16_t _width, _height;
	const byte *_srcptr;
	//end

	//from akos.h
	uint16 _codec = a->_cost.AKOS->AKHD.Codec;
	//end

	uint code;
	const byte *p;
	//const AkosOffset *off;
	//const CostumeData &cost = a->_cost;
	//const CostumeInfo *costumeInfo;
	uint i, extra;
	byte result = 0;
	int xmoveCur, ymoveCur;
	uint32 heCondMaskIndex[32];
	bool useCondMask;
	int lastDx, lastDy;

	lastDx = lastDy = 0;
	for (i = 0; i < 32; ++i) {
		heCondMaskIndex[i] = i;
	}

	//if (_skipLimbs)
	//	return 0;

	if (a->_cost.active[limb] == 8)
		return 0;

	//if (!a->_cost.active[limb]/* || a->_cost.stopped & (1 << limb)*/)
	//	return 0;

	//printf("Starting Drawing!\n");

	//while(scanKeys(), keysHeld() == 0);
	//swiDelay(5000000);

	useCondMask = false;
	p = (uint8_t*)(a->_cost.AKOS->AKSQ + a->_cost.curpos[limb]);

	code = p[0];
	if (code & 0x80)
		code = (*p << 8 | *(p + 1));//READ_BE_UINT16(p);

	//_shadow_mode = 0;

	if (code == AKC_C021 || code == AKC_C022) {
		uint16 s = a->_cost.curpos[limb] + 4;
		uint j = 0;
		extra = p[3];
		uint8 n = extra;
		//assert(n <= ARRAYSIZE(heCondMaskIndex));
		while (n--) {
			heCondMaskIndex[j++] = ((uint8_t*)a->_cost.AKOS->AKSQ)[s++];
		}
		useCondMask = true;
		p += extra + 2;
		code = (code == AKC_C021) ? AKC_ComplexChan : AKC_ComplexChan2;
	}

	if (code == AKC_Return || code == AKC_EndSeq)
		return 0;

	if (code != AKC_ComplexChan && code != AKC_ComplexChan2) {
		//off = (AkosOffset*)(a->_cost.AKOS->AKOF + (code & 0xFFF));

		fseek(HE1_File, a->_cost.AKOS->AKOFOffset + (code & 0xFFF), SEEK_SET);
		uint32_t off_akcd;
		uint16_t off_akci;
		readU32LE(HE1_File, &off_akcd);
		readU16LE(HE1_File, &off_akci);

		//assert((code & 0xFFF) * 6 < READ_BE_UINT32((const byte *)akof - 4) - 8);
		//assert((code & 0x7000) == 0);

		//_srcptr = (uint8_t*)(a->_cost.AKOS->AKCD + off->akcd);
		//costumeInfo = (const CostumeInfo *) (a->_cost.AKOS->AKCI + off->akci);

		fseek(HE1_File, a->_cost.AKOS->AKCIOffset + off_akci, SEEK_SET);

		//_width = costumeInfo->width;
		//_height = costumeInfo->height;

		readU16LE(HE1_File, &_width);
		readU16LE(HE1_File, &_height);

		uint16_t rel_x;
		uint16_t rel_y;

		readU16LE(HE1_File, &rel_x);
		readU16LE(HE1_File, &rel_y);

		uint16_t move_x;
		uint16_t move_y;

		readU16LE(HE1_File, &move_x);
		readU16LE(HE1_File, &move_y);

		xmoveCur = _xmove + (int16)rel_x + a->x - _width;// + 320 - _width;// + (int16)READ_LE_UINT16(&costumeInfo->rel_x);
		ymoveCur = _ymove + (int16)rel_y + a->y - _height / 2;// + 240;// + 240;// + (int16)READ_LE_UINT16(&costumeInfo->rel_y);

		/*printf("_xmove = %d\n", _xmove);
		printf("_ymove = %d\n", _ymove);

		printf("rel_x = %d\n", rel_x);
		printf("rel_y = %d\n", rel_y);*/

		_xmove += (int16)move_x;//(int16)READ_LE_UINT16(&costumeInfo->move_x);
		_ymove -= (int16)move_y;//(int16)READ_LE_UINT16(&costumeInfo->move_y);

		for(int i = 0; i < _height / 2; i++)
		{
			if((i * 2) + ymoveCur < 0) break;
			if((i * 2) + ymoveCur >= 480) break;
			for(int j = 0; j < _width / 2; j++)
			{
				if((j * 2) + xmoveCur < 0) break;
				if((j * 2) + xmoveCur >= 640) break;
				ResultFrameBuffer[(ymoveCur / 2 + i) * 512 + xmoveCur / 2 + j] = 0x001F;
			}
		}

		switch (_codec) {
		case 1:
			//result |= codec1(xmoveCur, ymoveCur);
			break;
		case 5:
			//result |= codec5(xmoveCur, ymoveCur);
			break;
		case 16:
			//result |= codec16(xmoveCur, ymoveCur);
			break;
		default:
			printf("Error: akos_drawLimb: invalid _codec %d\n", _codec);
		}
	} else {
		if (code == AKC_ComplexChan2)  {
			lastDx = (int16)(*(p + 2) | *(p + 3) << 8);//READ_LE_UINT16(p + 2);
			lastDy = (int16)(*(p + 4) | *(p + 5) << 8);//READ_LE_UINT16(p + 4);
			p += 4;
		}

		extra = p[2];
		p += 3;
		uint32 decflag = heCondMaskIndex[0];

		for (i = 0; i != extra; i++) {
			code = p[4];
			if (code & 0x80)
				code = (*(p + 4) << 8 | *(p + 5));//READ_BE_UINT16(p + 4);
			//off = (AkosOffset*)(a->_cost.AKOS->AKOF + (code & 0xFFF));

			fseek(HE1_File, a->_cost.AKOS->AKOFOffset + (code & 0xFFF), SEEK_SET);
			uint32_t off_akcd;
			uint16_t off_akci;
			readU32LE(HE1_File, &off_akcd);
			readU16LE(HE1_File, &off_akci);

			//_srcptr = (uint8_t*)(a->_cost.AKOS->AKCD + off->akcd);
			//costumeInfo = (const CostumeInfo *) (a->_cost.AKOS->AKCI + off->akci);

			fseek(HE1_File, a->_cost.AKOS->AKCIOffset + off_akci, SEEK_SET);

			//_width = costumeInfo->width;
			//_height = costumeInfo->height;

			readU16LE(HE1_File, &_width);
			readU16LE(HE1_File, &_height);

			xmoveCur = _xmove + (int16)(*p | *(p + 1) << 8) + a->x;// + 320;
			ymoveCur = _ymove + (int16)(*(p + 2) | *(p + 3) << 8) + a->y;// + 240;

			printf("xmoveCur = %d\n", xmoveCur);
			printf("ymoveCur = %d\n", ymoveCur);

			if (i == extra - 1) {
				_xmove += lastDx;
				_ymove -= lastDy;
			}

			uint16 shadowMask = 0;

			/*if (!useCondMask || !akct) {*/
			decflag = 1;
			/*} else {
			uint32 cond = READ_LE_UINT32(akct + cost.heCondMaskTable[limb] + heCondMaskIndex[i] * 4);
			if (cond == 0) {
			decflag = 1;
			} else {
			uint32 type = cond & ~0x3FFFFFFF;
			cond &= 0x3FFFFFFF;
			if (_vm->_game.heversion >= 90) {
			shadowMask = cond & 0xE000;
			cond &= ~0xE000;
			}
			if (_vm->_game.heversion >= 90 && cond == 0) {
			decflag = 1;
			} else if (type == 0x40000000) { // restored_bit
			decflag = (a->_heCondMask & cond) == cond ? 1 : 0;
			} else if (type == 0x80000000) { // dirty_bit
			decflag = (a->_heCondMask & cond) ? 0 : 1;
			} else {
			decflag = (a->_heCondMask & cond) ? 1 : 0;
			}
			}
			}*/

			for(int i = 0; i < _height / 2; i++)
			{
				if((i * 2) + ymoveCur >= 480) break;
				for(int j = 0; j < _width / 2; j++)
				{
					if((j * 2) + xmoveCur >= 640) break;
					ResultFrameBuffer[(ymoveCur / 2 + i) * 512 + xmoveCur / 2 + j] = 0x001F;
				}
			}

			p += (p[4] & 0x80) ? 6 : 5;

			if (decflag == 0)
				continue;

			/*if (_vm->_game.heversion >= 90) {
			if (_vm->_game.heversion >= 99)
			_shadow_mode = 0;
			if (xmap && (shadowMask & 0x8000))
			_shadow_mode = 3;
			}*/

			switch (_codec) {
			case 1:
				//result |= codec1(xmoveCur, ymoveCur);
				break;
			case 5:
				//result |= codec5(xmoveCur, ymoveCur);
				break;
			case 16:
				//result |= codec16(xmoveCur, ymoveCur);
				break;
			case 32:
				//result |= codec32(xmoveCur, ymoveCur);
				break;
			default:
				printf("Error: akos_drawLimb: invalid _codec %d\n", _codec);
			}
		}
	}

	return result;
}

#define GW(o) ((int16)(*((uint8_t*)(a->_cost.AKOS->AKSQ+curpos+(o))) | *((uint8_t*)(a->_cost.AKOS->AKSQ+curpos+(o) + 1)) << 8))
#define GUW(o) (*((uint8_t*)(a->_cost.AKOS->AKSQ+curpos+(o))) | *((uint8_t*)(a->_cost.AKOS->AKSQ+curpos+(o) + 1)) << 8)
#define GB(o) ((uint8_t*)a->_cost.AKOS->AKSQ)[curpos+(o)]

bool akos_increaseAnim(Actor *a, int chan) {
	byte active;
	uint old_curpos, curpos, end;
	uint code;
	bool flag_value, needRedraw;
	int tmp, tmp2;

	active = a->_cost.active[chan];
	end = a->_cost.end[chan];
	old_curpos = curpos = a->_cost.curpos[chan];
	flag_value = false;
	needRedraw = false;

	do {

		code = ((uint8_t*)a->_cost.AKOS->AKSQ)[curpos];
		if (code & 0x80)
			code = (*((uint8_t*)(a->_cost.AKOS->AKSQ + curpos)) << 8 | *((uint8_t*)(a->_cost.AKOS->AKSQ + curpos + 1)));

		switch (active) {
		case 6:
		case 8:
			switch (code) {
			case AKC_JumpIfSet:
			case AKC_AddVar:
			case AKC_SetVar:
			case AKC_SkipGE:
			case AKC_SkipG:
			case AKC_SkipLE:
			case AKC_SkipL:

			case AKC_SkipNE:
			case AKC_SkipE:
			case AKC_C016:
			case AKC_C017:
			case AKC_C018:
			case AKC_C019:
				curpos += 5;
				break;
			case AKC_JumpTable:
			case AKC_SetActorClip:
			case AKC_Ignore3:
			case AKC_Ignore2:
			case AKC_Ignore:
			case AKC_StartAnim:
			case AKC_StartVarAnim:
			case AKC_CmdQue3:
			case AKC_C042:
			case AKC_C044:
			case AKC_C0A3:
				curpos += 3;
				break;
			case AKC_SoundStuff:
				//if (_game.heversion >= 61)
				curpos += 6;
				//else
				//	curpos += 8;
				break;
			case AKC_Cmd3:
			case AKC_SetVarInActor:
			case AKC_SetDrawOffs:
				curpos += 6;
				break;
			case AKC_ClearFlag:
			case AKC_HideActor:
			case AKC_IncVar:
			case AKC_CmdQue3Quick:
			case AKC_Return:
			case AKC_EndSeq:
				curpos += 2;
				break;
			case AKC_JumpGE:
			case AKC_JumpG:
			case AKC_JumpLE:
			case AKC_JumpL:
			case AKC_JumpNE:
			case AKC_JumpE:
			case AKC_Random:
				curpos += 7;
				break;
			case AKC_Flip:
			case AKC_Jump:
			case AKC_StartAnimInActor:
			case AKC_C0A0:
			case AKC_C0A1:
			case AKC_C0A2:
				curpos += 4;
				break;
			case AKC_ComplexChan2:
				curpos += 4;
				// Fall through
			case AKC_ComplexChan:
				curpos += 3;
				tmp = ((uint8_t*)a->_cost.AKOS->AKSQ)[curpos - 1];
				while (--tmp >= 0) {
					curpos += 4;
					curpos += (((uint8_t*)a->_cost.AKOS->AKSQ)[curpos] & 0x80) ? 2 : 1;
				}
				break;
			case AKC_C021:
			case AKC_C022:
			case AKC_C045:
			case AKC_C046:
			case AKC_C047:
			case AKC_C048:
				needRedraw = 1;
				curpos += ((uint8_t*)a->_cost.AKOS->AKSQ)[curpos + 2];
				break;
			case AKC_C08E:
				//akos_queCommand(7, a, GW(2), 0);
				curpos += 4;
				break;
			default:
				curpos += (code & 0x8000) ? 2 : 1;
				break;
			}
			break;
		case 2:
			curpos += (code & 0x8000) ? 2 : 1;
			if (curpos > end)
				curpos = a->_cost.start[chan];
			break;
		case 3:
			if (curpos != end)
				curpos += (code & 0x8000) ? 2 : 1;
			break;
		}

		code = ((uint8_t*)a->_cost.AKOS->AKSQ)[curpos];
		if (code & 0x80)
			code = (*((uint8_t*)(a->_cost.AKOS->AKSQ + curpos)) << 8 | *((uint8_t*)(a->_cost.AKOS->AKSQ + curpos + 1)));

		if (flag_value && code != AKC_ClearFlag)
			continue;

		switch (code) {
		case AKC_StartAnimInActor:
			//akos_queCommand(4, derefActor(a->getAnimVar(GB(2)), "akos_increaseAnim:29"), a->getAnimVar(GB(3)), 0);
			continue;

		case AKC_Random:
			//setAnimVar(a, GB(6), _rnd.getRandomNumberRng(GW(2), GW(4)));
			continue;
		case AKC_JumpGE:
		case AKC_JumpG:
		case AKC_JumpLE:
		case AKC_JumpL:
		case AKC_JumpNE:
		case AKC_JumpE:
			if (akos_compare(getAnimVar(a, GB(4)), GW(5), code - AKC_JumpStart) != 0) {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_IncVar:
			setAnimVar(a, 0, getAnimVar(a, 0) + 1);
			continue;
		case AKC_SetVar:
			setAnimVar(a, GB(4), GW(2));
			continue;
		case AKC_AddVar:
			setAnimVar(a, GB(4), getAnimVar(a, GB(4)) + GW(2));
			continue;
		case AKC_Flip:
			a->_flip = GW(2) != 0;
			continue;
		case AKC_CmdQue3:
			//if (_game.heversion >= 61)
			tmp = GB(2);
			//else
			//	tmp = GB(2) - 1;
			//if ((uint) tmp < 24)
			//	akos_queCommand(3, a, a->_sound[tmp], 0);
			continue;
		case AKC_CmdQue3Quick:
			//akos_queCommand(3, a, a->_sound[0], 0);
			continue;
		case AKC_StartAnim:
			//akos_queCommand(4, a, GB(2), 0);
			continue;
		case AKC_StartVarAnim:
			//akos_queCommand(4, a, a->getAnimVar(GB(2)), 0);
			continue;
		case AKC_SetVarInActor:
			//derefActor(a->getAnimVar(GB(2)), "akos_increaseAnim:9")->setAnimVar(GB(3), GW(4));
			continue;
		case AKC_HideActor:
			//akos_queCommand(1, a, 0, 0);
			continue;
		case AKC_SetActorClip:
			//akos_queCommand(5, a, GB(2), 0);
			continue;
		case AKC_SoundStuff:
			//if (_game.heversion >= 61)
			continue;
			//tmp = GB(2) - 1;
			//if (tmp >= 8)
			//	continue;
			//tmp2 = GB(4);
			//if (tmp2 < 1 || tmp2 > 3)
			//	error("akos_increaseAnim:8 invalid code %d", tmp2);
			//akos_queCommand(tmp2 + 6, a, a->_sound[tmp], GB(6));
			//continue;
		case AKC_SetDrawOffs:
			//akos_queCommand(6, a, GW(2), GW(4));
			continue;
		case AKC_JumpTable:
			/*if (akfo == NULL)
			error("akos_increaseAnim: no AKFO table");
			tmp = a->getAnimVar(GB(2)) - 1;
			if (_game.heversion >= 80) {
			if (tmp < 0 || tmp > a->_cost.heJumpCountTable[chan] - 1)
			error("akos_increaseAnim: invalid jump value %d", tmp);
			curpos = READ_LE_UINT16(akfo + a->_cost.heJumpOffsetTable[chan] + tmp * 2);
			} else {
			if (tmp < 0 || tmp > numakfo - 1)
			error("akos_increaseAnim: invalid jump value %d", tmp);
			curpos = READ_LE_UINT16(&akfo[tmp]);
			}*/
			break;
		case AKC_JumpIfSet:
			if (!getAnimVar(a, GB(4)))
				continue;
			setAnimVar(a, GB(4), 0);
			curpos = GUW(2);
			break;

		case AKC_ClearFlag:
			flag_value = false;
			continue;

		case AKC_Jump:
			curpos = GUW(2);
			break;

		case AKC_Return:
		case AKC_EndSeq:
		case AKC_ComplexChan:
		case AKC_C08E:
		case AKC_ComplexChan2:
			break;

		case AKC_C021:
		case AKC_C022:
			needRedraw = 1;
			break;

		case AKC_Cmd3:
		case AKC_Ignore:
		case AKC_Ignore3:
			continue;

		case AKC_Ignore2:
			//akos_queCommand(3, a, a->_sound[a->getAnimVar(GB(2))], 0);
			continue;

		case AKC_SkipE:
		case AKC_SkipNE:
		case AKC_SkipL:
		case AKC_SkipLE:
		case AKC_SkipG:
		case AKC_SkipGE:
			if (akos_compare(getAnimVar(a, GB(4)), GW(2), code - AKC_SkipStart) == 0)
				flag_value = true;
			continue;
		case AKC_C016:
			if (isSoundRunning(a->_sound[getAnimVar(a, GB(4))]))  {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_C017:
			if (!isSoundRunning(a->_sound[getAnimVar(a, GB(4))])) {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_C018:
			if (isSoundRunning(a->_sound[GB(4)])) {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_C019:
			if (!isSoundRunning(a->_sound[GB(4)])) {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_C042:
			//akos_queCommand(9, a, a->_sound[GB(2)], 0);
			continue;
		case AKC_C044:
			//akos_queCommand(9, a, a->_sound[a->getAnimVar(GB(2))], 0);
			continue;
		case AKC_C045:
			//((ActorHE *)a)->setUserCondition(GB(3), a->getAnimVar(GB(4)));
			continue;
		case AKC_C046:
			//setAnimVar(a, GB(4), ((ActorHE *)a)->isUserConditionSet(GB(3)));
			continue;
		case AKC_C047:
			//((ActorHE *)a)->setTalkCondition(GB(3));
			continue;
		case AKC_C048:
			//a->setAnimVar(GB(4), ((ActorHE *)a)->isTalkConditionSet(GB(3)));
			continue;
		case AKC_C0A0:
			//akos_queCommand(8, a, GB(2), 0);
			continue;
		case AKC_C0A1:
			if (a->_heTalking != 0) {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_C0A2:
			if (a->_heTalking == 0) {
				curpos = GUW(2);
				break;
			}
			continue;
		case AKC_C0A3:
			//akos_queCommand(8, a, a->getAnimVar(GB(2)), 0);
			continue;
		case AKC_C0A4:
			//if (VAR(VAR_TALK_ACTOR) != 0) {
			//	curpos = GUW(2);
			//	break;
			//}
			continue;
		case AKC_C0A5:
			//if (VAR(VAR_TALK_ACTOR) == 0) {
			//	curpos = GUW(2);
			//	break;
			//}
			continue;
		default:
			if ((code & 0xC000) == 0xC000)
				printf("Error: Undefined uSweat token %X\n", code);
		}
		break;
	} while (1);

	int code2 = ((uint8_t*)a->_cost.AKOS->AKSQ)[curpos];
	if (code2 & 0x80)
		code2 = (*((uint8_t*)(a->_cost.AKOS->AKSQ + curpos)) << 8 | *((uint8_t*)(a->_cost.AKOS->AKSQ + curpos + 1)));

	if ((code2 & 0xC000) == 0xC000 && code2 != AKC_ComplexChan && code2 != AKC_Return && code2 != AKC_EndSeq && code2 != AKC_C08E && code2 != AKC_ComplexChan2 && code2 != AKC_C021 && code2 != AKC_C022)
		printf("Error: Ending with undefined uSweat token %X\n", code2);

	a->_cost.curpos[chan] = curpos;

	if (needRedraw)
		return 1;
	else
		return curpos != old_curpos;
} 

//void costumeDecodeData(Actor* a, int frame, uint usemask)
//{
//	return;
//	uint anim;
//	const byte *r;
//	uint offs;
//	int i;
//	byte code;
//	uint16 start, len;
//	uint16 mask;
//
//	if(a->_cost.AKOS != NULL)
//	{
//		freeAKOS(a->_cost.AKOS);
//		a->_cost.AKOS = NULL;
//	}
//
//	if (a->_costume == 0)
//	{
//		if(a->_cost.AKOS != NULL) freeAKOS(a->_cost.AKOS);
//		a->_cost.AKOS = NULL;
//		return;
//	}
//
//	printf("Costume: %d\n", a->_costume);
//
//	uint32_t offs_ = getCostumeOffset(HE0_File, &HE0_Data, a->_costume);
//	fseek(HE1_File, offs_, SEEK_SET);
//
//	AKOS_t* AKOS = readAKOS();
//
//	//while(scanKeys(), keysHeld() == 0);
//
//	anim = /*newDirToOldDir(getFacing(a)) + */frame * 4;
//
//	if (anim >= AKOS->AKHD.NrAnims)
//	{
//		printf("anim >= AKOS->AKHD.NrAnims\n");
//		freeAKOS(AKOS);
//		while(1);
//		return;
//	}
//
//	r = (uint8_t*)AKOS->AKCH;
//
//	for(int i = 0; i < AKOS->AKHD.NrAnims; i++)
//	{
//		offs = ((uint16_t*)r)[i];
//		if(offs != 0) break;
//	}
//
//	/*offs = ((uint16_t*)r)[anim];
//	//offs = *((uint16_t*)(r + (anim * 4) * sizeof(uint16)));
//	if(offs == 0) offs = ((uint16_t*)r)[anim - 1];//*((uint16_t*)(r + (anim - 1) * 2));//offs = *((uint16_t*)(r + ((anim  * 4) - 1) * sizeof(uint16)));*/
//	if (offs == 0)
//	{
//		printf("frame = %d\n", frame);
//		printf("anim = %d\n", anim);
//		printf("offs == 0\n");
//		freeAKOS(AKOS);
//		//while(1);
//		return;
//	}
//
//	printf("frame = %d\n", frame);
//	printf("anim = %d\n", anim);
//	printf("offs = %x\n", offs);
//
//	r += offs;
//
//	//const uint8 *akstPtr = (uint8_t*)AKOS->AKST;
//	//const uint8 *aksfPtr = (uint8_t*)AKOS->AKSF;
//
//	//printf("%x %x %x %x\n", *r++, *r++ ,*r++ ,*r++);
//
//	//r -= 4;
//
//	i = 0;
//	mask = (*r++ | *r++ << 8);
//
//	printf("mask = %x\n", mask);
//
//	byte mode = *r++;
//
//	printf("mode = %d\n", mode);
//
//
//	if(mode != 1 && mode != 4 && mode != 5)
//	{
//		uint16_t start = (*r++ | *r++ << 8);//*((uint16_t*)r); r += 2;
//		uint16_t length = *((uint16_t*)r); r += 2;
//		//a->_cost.end[0] = start + length;
//		a->_cost.start[0] = start;
//		a->_cost.curpos[0] = start;
//
//		printf("start = %x\n", start);
//		//printf("length = %x\n", length);
//
//		if(start > AKOS->AKSQLength) 
//		{
//			printf("start >= AKOS->AKSQLength\n");
//			freeAKOS(AKOS);
//			while(1);
//			return;
//		}
//
//	}
//
//	//while(scanKeys(), keysHeld() == 0);
//	//swiDelay(5000000);
//
//	/*do {
//	if (mask & 0x8000) {
//	const uint8 *akst = akstPtr;
//	const uint8 *aksf = aksfPtr;
//
//	code = *r++;
//	if (usemask & 0x8000) {
//	switch (code) {
//	case 1:
//	a->_cost.active[i] = 0;
//	a->_cost.frame[i] = frame;
//	a->_cost.end[i] = 0;
//	a->_cost.start[i] = 0;
//	a->_cost.curpos[i] = 0;
//	a->_cost.heCondMaskTable[i] = 0;
//
//	if (akst) {
//	int size = AKOS->AKSTLength / 8;
//	if (size > 0) {
//	bool found = false;
//	while (size--) {
//	if (*((uint32*)akst) == 0) {
//	a->_cost.heCondMaskTable[i] = *((uint32*)akst + 4);
//	found = true;
//	break;
//	}
//	akst += 8;
//	}
//	if (!found) {
//	printf("Error: Sequence not found in actor %p costume %d\n", (void *)a, a->_costume);
//	}
//	}
//	}
//	break;
//	case 4:
//	a->_cost.stopped |= 1 << i;
//	break;
//	case 5:
//	a->_cost.stopped &= ~(1 << i);
//	break;
//	default:
//	start = *((uint16_t*)r); r += 2;
//	len = *((uint16_t*)r); r += 2;
//
//	a->_cost.heJumpOffsetTable[i] = 0;
//	a->_cost.heJumpCountTable[i] = 0;
//	if (aksf) {
//	int size = AKOS->AKSFLength / 6;
//	if (size > 0) {
//	bool found = false;
//	while (size--) {
//	if (*((uint16_t*)aksf) == start) {
//	a->_cost.heJumpOffsetTable[i] = *((uint16_t*)aksf + 2);
//	a->_cost.heJumpCountTable[i] = *((uint16_t*)aksf + 4);
//	found = true;
//	break;
//	}
//	aksf += 6;
//	}
//	if (!found) {
//	printf("Error: Sequence not found in actor %p costume %d\n", (void *)a, a->_costume);
//	}
//	}
//	}
//
//	a->_cost.active[i] = code;
//	a->_cost.frame[i] = frame;
//	a->_cost.end[i] = start + len;
//	a->_cost.start[i] = start;
//	a->_cost.curpos[i] = start;
//	a->_cost.heCondMaskTable[i] = 0;
//	if (akst) {
//	int size = AKOS->AKSTLength / 8;
//	if (size > 0) {
//	bool found = false;
//	while (size--) {
//	if (*((uint32*)akst) == start) {
//	a->_cost.heCondMaskTable[i] = *((uint32*)akst + 4);
//	found = true;
//	break;
//	}
//	akst += 8;
//	}
//	if (!found) {
//	printf("Error: Sequence not found in actor %p costume %d\n", (void *)a, a->_costume);
//	}
//	}
//	}
//	break;
//	}
//	} else {
//	if (code != 1 && code != 4 && code != 5)
//	r += sizeof(uint16) * 2;
//	}
//	}
//	i++;
//	mask <<= 1;
//	usemask <<= 1;
//	} while ((uint16)mask);*/
//	//freeAKOS(AKOS);
//	free(AKOS->AKCH);
//	//free(AKOS->AKST);
//	//free(AKOS->AKSF);
//	a->_cost.AKOS = AKOS;
//}
//
ITCM_CODE void renderCostume(Actor* a, bool render)
{
	if(!render && (a->_animProgress + 1) < a->_animSpeed)
	{
		a->_animProgress++;
		return;
	}
	uint heCondMaskIndex[32];// = new uint[32];
	bool useCondMask = false;
	int xmove = a->x;//0;
	int ymove = a->y;//0;
	int x = 0;
	int y = 0;
	int lastDx = 0;
	int lastDy = 0;
	int limb = 0;	

	int minx = 640 * 2;
	int miny = 480 * 2;
	int maxx = 0;
	int maxy = 0;

	for(int i = 0; i < 16; i++)
	{
		if(a->_cost.active[i] == 6) 
		{
			limb = i;
			break;
		}
		else if(a->_cost.active[i] != 0)
		{
			printf("a->_cost.active[%d] = %d\n", i, a->_cost.active[i]);
			while(scanKeys(), keysHeld() == 0);
			swiDelay(5000000);
		}
	}
	if(((int16)a->_cost.curpos[limb]) == -2) return;

	if(((int16)a->_cost.curpos[limb]) == -1) 
	{
		a->_cost.curpos[limb] = 0;
	}
	//else if(((int16)a->_cost.curpos[limb]) != 0)
	//{
	//xmove = a->_cost.xmove[limb];// = xmove;
	//ymove = a->_cost.ymove[limb];// = ymove;
	//lastDx = a->_cost.lastDx[limb];
	//lastDy = a->_cost.lastDy[limb];
	//}

	for (int i = 0; i < 32; i++)
	{
		heCondMaskIndex[i] = i;
	}

	//printf("Draw!\n");

	//if((int16)a->_cost.curpos[limb] == -1) return;

	//if(a->_cost.curpos[limb] >= a->_cost.end[limb]) return;

	//fseek(HE1_File, a->_cost.AKOS->AKSQOffset, SEEK_SET);

	uint8_t* seqstart = (uint8_t*)a->_cost.AKOS->AKSQ;//(uint8_t*)malloc(a->_cost.AKOS->AKSQLength);
	//readBytes(HE1_File, seqstart, a->_cost.AKOS->AKSQLength);

	uint8_t* seq = seqstart + a->_cost.curpos[limb];//(uint8_t*)a->_cost.AKOS->AKSQ + a->_cost.curpos[limb];
	//printf("Start: %x\n", a->_cost.curpos[limb]);
	while (true)
	{
		//while(scanKeys(), keysHeld() == 0);
		//swiDelay(5000000);
		ushort code = *seq++;//Data[Offset++];
		if (code & 0x80)
			code = (ushort)((code << 8) | *seq++);

		//printf("%d: Code (%X)\n", (uint32_t)seq - (uint32_t)seqstart, code);

		if ((code >> 8) != 0xC0)
		{
			printf("Bitmap (%d)\n", code & 0xFFF);
			continue;
		}

		switch (code)
		{
		case AKC_Return:
			goto ret;
		case AKC_SetVar://Set Variable
			{
				int16_t a_ = (int16)(seq[0] | seq[1] << 8);
				seq += 2;
				byte v = *seq++;
				a->_animVariable[v] = a_;
				break;
			}
		case AKC_CmdQue3://Start Sound
			{
				byte sound = *seq++;
				//printf("0xC015: Start Sound (%d = %d)\n", sound, a->_sound[sound]);
				addSoundToQueue2(a->_sound[sound], 0, 0, 8);
			}
			break;
		case AKC_C018://ifSoundIsRunning(uword jmp,byte snd) 
			{
				ushort o = (seq[0] | seq[1] << 8);
				seq += 2;
				if(isSoundRunning(*seq++)) seq = (uint8_t*)seqstart + o;
				break;
			}
		case AKC_C019://ifSoundIsNotRunning
			{
				ushort o = (seq[0] | seq[1] << 8); seq += 2;
				if(!isSoundRunning(*seq++)) seq = (uint8_t*)seqstart + o;
				break;
			}
		case AKC_ComplexChan:
		case AKC_ComplexChan2:
_0xC020:
			{
				if (code == 0xC025)
				{
					lastDx = (seq[0] | seq[1] << 8); seq += 2;
					lastDy = (seq[0] | seq[1] << 8); seq += 2;
				}
				int extra = *seq++;
				uint decflag = heCondMaskIndex[0];
				for (int i = 0; i < extra; i++)
				{
					code = *(seq + 4);
					if ((code & 0x80) != 0)
						code = (ushort)((code << 8) | *(seq + 5));
					int offset = code & 0xFFF;

					x = xmove + (int16)(seq[0] | (seq[1] << 8));
					y = ymove + (int16)(seq[2] | (seq[3] << 8)) - a->_elevation;

					if (i == extra - 1)
					{
						xmove += lastDx;
						ymove -= lastDy;
					}

					ushort shadowMask = 0;
					if (!useCondMask || a->_cost.AKOS->AKCTOffset == 0) {
						decflag = 1;
					} else {
						fseek(HE1_File, a->_cost.AKOS->AKCTOffset + a->_cost.heCondMaskTable[limb] + heCondMaskIndex[i] * 4, SEEK_SET);
						uint32 cond;// = READ_LE_UINT32(akct + cost.heCondMaskTable[limb] + heCondMaskIndex[i] * 4);
						readU32LE(HE1_File, &cond);
						if (cond == 0) {
							decflag = 1;
						} else {
							uint32 type = cond & ~0x3FFFFFFF;
							cond &= 0x3FFFFFFF;
							//if (_vm->_game.heversion >= 90) {
							shadowMask = cond & 0xE000;
							cond &= ~0xE000;
							//}
							if (/*_vm->_game.heversion >= 90 &&*/ cond == 0) {
								decflag = 1;
							} else if (type == 0x40000000) { // restored_bit
								decflag = (a->_heCondMask & cond) == cond ? 1 : 0;
							} else if (type == 0x80000000) { // dirty_bit
								decflag = (a->_heCondMask & cond) ? 0 : 1;
							} else {
								decflag = ((a->_heCondMask & cond) ? 1 : 0);
							}
						}
					}

					if (decflag != 0 && render && a->_cost.AKOS->HasRGBS)
					{
						fseek(HE1_File, a->_cost.AKOS->AKOFOffset + offset * 6, SEEK_SET);
						uint32_t off_akcd;
						uint16_t off_akci;
						readU32LE(HE1_File, &off_akcd);
						readU16LE(HE1_File, &off_akci);

						fseek(HE1_File, a->_cost.AKOS->AKCIOffset + off_akci, SEEK_SET);

						uint16_t width;
						uint16_t height;

						readU16LE(HE1_File, &width);
						readU16LE(HE1_File, &height);

						//if((a->_cost.AKOS->AKHD.Codec == 32 || a->_cost.AKOS->AKHD.Codec == 1) && a->_cost.AKOS->RGBS != NULL)
						//{
							uint16_t* dst = &TempBuffer[0];
							for(int i = 0; i < width * height; i++)
							{
								dst[i] = 0x8000;
							}
							ConvertAKOSFrame(a->_cost.AKOS->AKCDOffset + off_akcd, (uint8_t*)dst, a->_cost.AKOS->AKPL, a->_cost.AKOS->AKPLLength, a->_cost.AKOS->RGBS, width, height, a->_cost.AKOS->AKHD.Codec);
							for(int i = 0; i < height / 2; i++)
							{
								if((i * 2) + y < 0) continue;
								if((i * 2) + y >= 480) break;
								for(int j = 0; j < width / 2; j++)
								{
									if((j * 2) + x < 0) continue;
									if((j * 2) + x >= 640) break;
									if(dst[i * 2 * width + j * 2] != 0x8000)
										ResultFrameBuffer[(y / 2 + i) * 512 + x / 2 + j] = Merge4Pixels(dst[i * 2 * width + j * 2], dst[i * 2 * width + j * 2 + 1], dst[(i * 2 + 1) * width + j * 2], dst[(i * 2 + 1) * width + j * 2 + 1]) | 0x8000;
								}
							}
						/*}
						else
						{
							for(int i = 0; i < height / 2; i++)
							{
								if((i * 2) + y < 0) continue;
								if((i * 2) + y >= 480) break;
								for(int j = 0; j < width / 2; j++)
								{
									if((j * 2) + x < 0) continue;
									if((j * 2) + x >= 640) break;
									ResultFrameBuffer[(y / 2 + i) * 512 + x / 2 + j] = 0x001F;
								}
							}
						}*/

						if(minx > x) minx = x;
						if(maxx < (x + width)) maxx = x + width;
						if(miny > y) miny = y;
						if(maxy < (y + height)) maxy = y + height;
					}

					seq += (((*(seq+4)) & 0x80) != 0) ? 6 : 5;
				}
				goto ret;
			}
			break;
		case AKC_C021:
		case AKC_C022:
			{
				seq++;
				int extra = *seq++;
				for (int i = 0; i < extra; i++)
				{
					heCondMaskIndex[i] = *seq++;
				}
				useCondMask = true;
				code = (ushort)((code == AKC_C021) ? AKC_ComplexChan : AKC_ComplexChan2);
				goto _0xC020;
			}
			break;
		case AKC_Jump://Jump (used as a looping sometimes)
			{
				ushort o = (seq[0] | seq[1] << 8);
				seq += 2;
				seq = (uint8_t*)seqstart + o;
				break;
			}
		case AKC_AddVar:
			{
				short a_ = (int16)(seq[0] | seq[1] << 8);
				seq += 2;
				byte v = *seq++;
				a->_animVariable[v] += a_;
				break;
			}
		case AKC_C042://Start Sound
			{
				byte sound = *seq++;
				//printf("0xC042: Start Sound (%d)\n", sound);
				addSoundToQueue2(a->_sound[sound], 0, 0, 8);
			}
			break;
		case AKC_JumpE:
			{
				ushort o = (seq[0] | seq[1] << 8);
				seq += 2;
				byte a_ = *seq++;
				short b_ = (int16)(seq[0] | seq[1] << 8);
				seq += 2;
				if (a->_animVariable[a_] == b_) seq = (uint8_t*)seqstart + o;
				break;
			}
		case AKC_JumpNE:
			{
				ushort o = (seq[0] | seq[1] << 8);
				seq += 2;
				byte a_ = *seq++;
				short b_ = (int16)(seq[0] | seq[1] << 8);
				seq += 2;
				if (a->_animVariable[a_] != b_) seq = (uint8_t*)seqstart + o;
				break;
			}
		case AKC_JumpL:
			{
				ushort o = (seq[0] | seq[1] << 8);
				seq += 2;
				byte a_ = *seq++;
				short b_ = (int16)(seq[0] | seq[1] << 8);
				seq += 2;
				if (a->_animVariable[a_] < b_) seq = (uint8_t*)seqstart + o;
				break;
			}
			case AKC_JumpLE:
			{
				ushort o = (seq[0] | seq[1] << 8);
				seq += 2;
				byte a_ = *seq++;
				short b_ = (int16)(seq[0] | seq[1] << 8);
				seq += 2;
				if (a->_animVariable[a_] <= b_) seq = (uint8_t*)seqstart + o;
				break;
			}
		case AKC_JumpG:
			{
				ushort o = (seq[0] | seq[1] << 8);
				seq += 2;
				byte a_ = *seq++;
				short b_ = (int16)(seq[0] | seq[1] << 8);
				seq += 2;
				if (a->_animVariable[a_] > b_) seq = (uint8_t*)seqstart + o;
				break;
			}
		case AKC_JumpGE:
			{
				ushort o = (seq[0] | seq[1] << 8);
				seq += 2;
				byte a_ = *seq++;
				short b_ = (int16)(seq[0] | seq[1] << 8);
				seq += 2;
				if (a->_animVariable[a_] >= b_) seq = (uint8_t*)seqstart + o;
				break;
			}
		case AKC_StartAnim:
			{
				byte a_ = *seq++;
				startAnimActor(a, a_);
				break;
			}
		case AKC_Random://random(word min,word max,byte *ret) 
			{
				short min = (int16)(seq[0] | seq[1] << 8);
				seq += 2;
				short max = (int16)(seq[0] | seq[1] << 8);
				seq += 2;
				byte v = *seq++;
				a->_animVariable[v] = rand() % (max - min) + min;
				break;
			}
		case AKC_HideActor:
			{
				putActor(a, 0, 0, 0);
				break;
			}
		case AKC_Ignore2://ignore2
			{
				byte a_ = *seq++;
				addSoundToQueue2(a->_sound[a->_animVariable[a_]], 0, 0, 8);
				break;
			}
		case AKC_C0A0:
			{
				uint8_t a_ = *seq++;
				uint8_t b_ = *seq++;

				_actorToPrintStrFor = a->_number;

				a->_talkPosX = a->_heTalkQueue[a_].posX;
				a->_talkPosY = a->_heTalkQueue[a_].posY;
				a->_talkColor = a->_heTalkQueue[a_].color;

				_string[0].loadDefault();
				_string[0].color = a->_talkColor;

				actorTalk(a->_heTalkQueue[a_].sentence);
			}
			break;
		case AKC_C0A1://Jump if Talking
			{
				ushort o = (seq[0] | seq[1] << 8);
				seq += 2;
				if(a->_heTalking) seq = (uint8_t*)seqstart + o;
			}
			break;
		case AKC_C0A2://jumpIfNotTalking
			{
				ushort o = (seq[0] | seq[1] << 8);
				seq += 2;
				if(!a->_heTalking) seq = (uint8_t*)seqstart + o;
			}
			break;
		case AKC_C0A3://resetVarPan
			{
				uint8_t a_ = a->_animVariable[*seq++];

				_actorToPrintStrFor = a->_number;

				a->_talkPosX = a->_heTalkQueue[a_].posX;
				a->_talkPosY = a->_heTalkQueue[a_].posY;
				a->_talkColor = a->_heTalkQueue[a_].color;

				_string[0].loadDefault();
				_string[0].color = a->_talkColor;

				actorTalk(a->_heTalkQueue[a_].sentence);
			}
			break;
		case AKC_EndSeq:
			a->_cost.curpos[limb] = (uint32_t)seq - (uint32_t)seqstart - 2;
			a->_cost.active[limb] = 0;
			//printf("EndSeq\n");
			//while(scanKeys(), keysHeld() == 0);
			//swiDelay(5000000);
			return;
		default:
			printf("Unknown (%X)\n", code);
			while(scanKeys(), keysHeld() == 0);
			swiDelay(5000000);
			break;
			//return new Bitmap(1, 1);
		}
	}
ret:
	a->_animProgress++;
	if(a->_animProgress >= a->_animSpeed)
	{
		a->_animProgress = 0;
		a->_cost.curpos[limb] = (uint32_t)seq - (uint32_t)seqstart;
	}
	//a->_cost.xmove[limb] = xmove;
	//a->_cost.ymove[limb] = ymove;
	//a->_cost.lastDx[limb] = 0;//lastDx;
	//a->_cost.lastDy[limb] = 0;//lastDy;
	a->_top = miny;
	a->_bottom = maxy;
	a->_left = minx;
	a->_right = maxx;
	//free(seqstart);
}

