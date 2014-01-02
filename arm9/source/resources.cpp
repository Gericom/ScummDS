#include <nds.h>

#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <ioutil.h>
#include <scumm.h>
#include <HE0.h>
#include <script.h>
#include <room.h>

//void** GlobalScripts = NULL;

//static bool ScriptInitialized = false;

//Here a memory leak is created by using malloc, but never freeing again.
void* getGlobalScriptPtr(int id)
{
	//if(GlobalScripts != NULL && GlobalScripts[id] != NULL) return GlobalScripts[id];
	uint32_t offs = getScriptOffset(HE0_File, &HE0_Data, id);//SCRP
	printf("SCRP Offs: 0x%x\n", offs);
	uint32_t length = getScriptLength(HE0_File, &HE0_Data, id);//Total Length (including header)
	offs += 8;
	length -= 8;
	fseek(HE1_File, offs, SEEK_SET);
	void* data = malloc(length);
	readBytes(HE1_File, (uint8_t*)data, length);
	//if(GlobalScripts != NULL) GlobalScripts[id] = data;
	return data;
}

//void** LocalScripts = NULL;

void* getLocalScriptPtr(int id)
{
	for(int i = 0; i < RoomResource->RMDA->NrLocalScripts; i++)
	{
		//printf("%d\n", RoomResource->RMDA->LocalScriptIds[i]);
		if(RoomResource->RMDA->LocalScriptIds[i] == id)
		{
			fseek(HE1_File, RoomResource->RMDA->LocalScriptOffsets[i], SEEK_SET);
			void* data = malloc(RoomResource->RMDA->LocalScriptLengths[i]);
			readBytes(HE1_File, (uint8_t*)data, RoomResource->RMDA->LocalScriptLengths[i]);
			return data;
		}
	}
	printf("Error: Local Script (%d) not in Room!\n", id);
	while(1);
	return NULL;
}