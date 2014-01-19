#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <HE0.h>
#include <ioutil.h>
#include <HEP.h>
#include <scumm.h>

void readHEP(FILE* handle)
{
	uint32_t sig;
	uint32_t size;
	readU32LE(handle, &sig);
	readU32LE(handle, &size);
	readU16LE(handle, &gameHEVersion);
	uint16_t nrfiles;
	readU16LE(handle, &nrfiles);
	printf("nrfiles: %d\n", nrfiles);
	for(int i = 0; i < nrfiles; i++)
	{
		uint32_t id;
		readU32LE(handle, &id);
		uint32_t offset;
		readU32LE(handle, &offset);
		uint32_t length;
		readU32LE(handle, &length);
		printf("id: %d\n", id);
		printf("offset: %x\n", offset);
		printf("length: %x\n", length);
		switch(id)
		{
			case 0: HE0_Start = offset; HE0_Length = length; break;
			case 1: HE1_Start = offset; break;
			case 2: HE2_Start = offset; break;
			case 4: HE4_Start = offset; break;

			case 8: HE8_Start = offset; break;
		}
	}
	HE0_File = handle;
	HE1_File = handle;
	HE2_File = handle;
	HE4_File = handle;

	HEx_File = handle;
}