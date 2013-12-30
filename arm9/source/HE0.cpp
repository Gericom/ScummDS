#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <HE0.h>
#include <ioutil.h>

#define HE1_Start 0//0xE5B0//sam2demo, otherwise 0

void readHE0(FILE* handle, HE0_t* HE0)
{
	uint32_t sig;
	uint32_t size;
	uint16_t a;
	//HE0_t* HE0 = (HE0_t*)malloc(sizeof(HE0_t));
	while(readU32LE(handle, &sig))
	{
		//printf((char*)&sig, 4);
		//printf("\n");
		//printf("%c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
		readU32LE(handle, &size);
		switch(sig)
		{
		case MKTAG('M', 'A', 'X', 'S'):
			fgetpos(handle, &HE0->MAXS);
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		case MKTAG('D', 'I', 'R', 'I'):
			fgetpos(handle, &HE0->DIRI);
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		case MKTAG('D', 'I', 'R', 'R'):
			fgetpos(handle, &HE0->DIRR);
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		case MKTAG('D', 'I', 'R', 'S'):
			fgetpos(handle, &HE0->DIRS);
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		case MKTAG('D', 'I', 'R', 'N'):
			fgetpos(handle, &HE0->DIRN);
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		case MKTAG('D', 'I', 'R', 'C'):
			fgetpos(handle, &HE0->DIRC);
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		case MKTAG('D', 'I', 'R', 'F'):
			fgetpos(handle, &HE0->DIRF);
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		case MKTAG('D', 'I', 'R', 'M'):
			fgetpos(handle, &HE0->DIRM);
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		case MKTAG('D', 'I', 'R', 'T'):
			fgetpos(handle, &HE0->DIRT);
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		case MKTAG('D', 'I', 'S', 'K'):
			fgetpos(handle, &HE0->DISK);
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		case MKTAG('D', 'L', 'F', 'L'):
			fgetpos(handle, &HE0->DLFL);
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		default:
			printf("Unknown Signature\n");
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		}
	}
	//return HE0;
}

uint16_t getCostumeCount(FILE* handle, HE0_t* HE0)
{
	fsetpos(handle, &HE0->DIRC);
	uint16_t num;
	readU16LE(handle, &num);
	return num;
}

uint32_t getCostumeOffset(FILE* handle, HE0_t* HE0, int idx)
{
	fsetpos(handle, &HE0->DIRC);
	uint16_t num;
	readU16LE(handle, &num);
	if(idx >= num) 
	{
		printf("Incorrect Costume Id: %d\n",idx);
		while(1);
		return 0xFFFFFFFF;
	}
	fseek(handle, idx, SEEK_CUR);
	uint8_t room;
	readByte(handle, &room);
	fsetpos(handle, &HE0->DIRC);
	fseek(handle, num + idx * 4 + 2, SEEK_CUR);
	uint32_t res;
	readU32LE(handle, &res);
	return res + getLFLFOffset(handle, HE0, room) + 8;
}

uint16_t getAWIZCount(FILE* handle, HE0_t* HE0)
{
	fsetpos(handle, &HE0->DIRM);
	uint16_t num;
	readU16LE(handle, &num);
	return num;
}

uint32_t getAWIZOffset(FILE* handle, HE0_t* HE0, int idx)
{
	fsetpos(handle, &HE0->DIRM);
	uint16_t num;
	readU16LE(handle, &num);
	if(idx >= num) 
	{
		printf("Incorrect AWIZ Id: %d\n",idx);
		while(1);
		return 0xFFFFFFFF;
	}
	fseek(handle, idx, SEEK_CUR);
	uint8_t room;
	readByte(handle, &room);
	fsetpos(handle, &HE0->DIRM);
	fseek(handle, num + idx * 4 + 2, SEEK_CUR);
	uint32_t res;
	readU32LE(handle, &res);
	return res + getLFLFOffset(handle, HE0, room) + 8;
}

uint16_t getSoundCount(FILE* handle, HE0_t* HE0)
{
	fsetpos(handle, &HE0->DIRN);
	uint16_t num;
	readU16LE(handle, &num);
	return num;
}

uint32_t getSoundOffset(FILE* handle, HE0_t* HE0, int idx)
{
	fsetpos(handle, &HE0->DIRN);
	uint16_t num;
	readU16LE(handle, &num);
	if(idx >= num) 
	{
		printf("Incorrect Sound Id: %d\n",idx);
		while(1);
		return 0xFFFFFFFF;
	}
	fseek(handle, idx, SEEK_CUR);
	uint8_t room;
	readByte(handle, &room);
	fsetpos(handle, &HE0->DIRN);
	fseek(handle, num + idx * 4 + 2, SEEK_CUR);
	uint32_t res;
	readU32LE(handle, &res);
	return res + getLFLFOffset(handle, HE0, room) + 8;
}

uint16_t getScriptCount(FILE* handle, HE0_t* HE0)
{
	fsetpos(handle, &HE0->DIRS);
	uint16_t num;
	readU16LE(handle, &num);
	return num;
}

uint32_t getScriptOffset(FILE* handle, HE0_t* HE0, int idx)
{
	fsetpos(handle, &HE0->DIRS);
	uint16_t num;
	readU16LE(handle, &num);
	if(idx >= num) return 0xFFFFFFFF;
	fseek(handle, idx, SEEK_CUR);
	uint8_t room;
	readByte(handle, &room);
	fsetpos(handle, &HE0->DIRS);
	fseek(handle, num + idx * 4 + 2, SEEK_CUR);
	uint32_t res;
	readU32LE(handle, &res);
	return res + getLFLFOffset(handle, HE0, room) + 8;
}

uint32_t getScriptLength(FILE* handle, HE0_t* HE0, int idx)
{
	fsetpos(handle, &HE0->DIRS);
	uint16_t num;
	readU16LE(handle, &num);
	if(idx >= num) return 0xFFFFFFFF;
	fseek(handle, num * 5 + idx * 4, SEEK_CUR);
	uint32_t res;
	readU32LE(handle, &res);
	return res;
}

uint16_t getLFLFCount(FILE* handle, HE0_t* HE0)
{
	fsetpos(handle, &HE0->DLFL);
	uint16_t num;
	readU16LE(handle, &num);
	return num;
}

uint32_t getLFLFOffset(FILE* handle, HE0_t* HE0, int idx)
{
	fsetpos(handle, &HE0->DLFL);
	uint16_t num;
	readU16LE(handle, &num);
	if(idx >= num) return 0xFFFFFFFF;
	fseek(handle, idx * 4, SEEK_CUR);
	uint32_t res;
	readU32LE(handle, &res);
	return res - 8 + HE1_Start;
}