#include <nds.h>

#include <stdio.h>

#include <HE1.h>
#include <room.h>
#include <render.h>
#include <scumm.h>
#include <wiz.h>
#include <ioutil.h>
#include <graphics.h>

uint8_t* _objectOwnerTable;
uint8_t* _objectStateTable;
uint8_t* _objectRoomTable;
uint32_t* _classData;
uint32_t _numGlobalObjects;

static const uint32 IMxx_tags[] = {
	MKTAG('I','M','0','0'),
	MKTAG('I','M','0','1'),
	MKTAG('I','M','0','2'),
	MKTAG('I','M','0','3'),
	MKTAG('I','M','0','4'),
	MKTAG('I','M','0','5'),
	MKTAG('I','M','0','6'),
	MKTAG('I','M','0','7'),
	MKTAG('I','M','0','8'),
	MKTAG('I','M','0','9'),
	MKTAG('I','M','0','A'),
	MKTAG('I','M','0','B'),
	MKTAG('I','M','0','C'),
	MKTAG('I','M','0','D'),
	MKTAG('I','M','0','E'),
	MKTAG('I','M','0','F'),
	MKTAG('I','M','1','0')
};

bool getClass(int obj, int cls)
{
	if(obj >= _numGlobalObjects || obj < 1) return 0;
	cls &= 0x7F;

	return (_classData[obj] & (1 << (cls - 1))) != 0;
}

void putClass(int obj, int cls, bool set)
{
	if(obj >= _numGlobalObjects || obj < 1) return;
	cls &= 0x7F;

	if (set)
		_classData[obj] |= (1 << (cls - 1));
	else
		_classData[obj] &= ~(1 << (cls - 1));
} 

int getOwner(int obj)
{
	if(obj >= _numGlobalObjects || obj < 1) return 0;
	return _objectOwnerTable[obj];
}

void putOwner(int obj, int owner) 
{
	if(obj >= _numGlobalObjects || obj < 1) return;
	_objectOwnerTable[obj] = owner;
}

int getState(int obj) 
{
	if(obj >= _numGlobalObjects || obj < 1) return 0;
	return _objectStateTable[obj];
}

void putState(int obj, int state) 
{
	if(obj >= _numGlobalObjects || obj < 1) return;
	_objectStateTable[obj] = state;
}

int getObjectIndex(int object) {
	int i;

	if (object < 1)
		return -1;

	for (i = (RoomResource->RMDA->RMHD.NrObjects - 1); i >= 0; i--) {
		if (RoomResource->RMDA->OBCD[i]->ObjectId == object)
			return i;
	}
	return -1;
}

//int findObject(int x, int y)
//{
//	int i, b;
//	byte a;
//	const int mask = 0xF;
//
//	for (i = /*1*/0; i < /*_numLocalObjects*/RoomResource->RMDA->RMHD.NrObjects; i++) {
//		if ((RoomResource->RMDA->OBCD[i]->ObjectId < 1) || getClass(RoomResource->RMDA->OBCD[i]->ObjectId, kObjectClassUntouchable))
//			continue;
//
//		b = i;
//		do {
//			a = ((RoomResource->RMDA->OBCD[b]->Flags == 0x80) ? 1 : (RoomResource->RMDA->OBCD[b]->Flags & 0xF));
//			b = RoomResource->RMDA->OBCD[b]->Parent;
//			if (b == 0) {
//				//if (_game.heversion >= 71) {
//					//if (((ScummEngine_v71he *)this)->_wiz->polygonHit(_objs[i].obj_nr, x, y))
//					//	return _objs[i].obj_nr;
//				//}
//				if (RoomResource->RMDA->OBCD[i]->X <= x && RoomResource->RMDA->OBCD[i]->Width + RoomResource->RMDA->OBCD[i]->X > x &&
//				    RoomResource->RMDA->OBCD[i]->Y <= y && RoomResource->RMDA->OBCD[i]->Height + RoomResource->RMDA->OBCD[i]->Y > y)
//					return _objs[i].obj_nr;
//				break;
//			}
//		} while ((RoomResource->RMDA->OBCD[i]->state & mask) == a);
//	}
//
//	return 0;
//} 

void drawObject(int obj, int arg)
{
	int id = RoomResource->RMDA->OBCD[obj]->ObjectId;
	int x = RoomResource->RMDA->OBCD[obj]->X;
	int y = RoomResource->RMDA->OBCD[obj]->Y;
	int width = RoomResource->RMDA->OBCD[obj]->Width;
	int height = RoomResource->RMDA->OBCD[obj]->Height;

	int state = getState(id);
	int obim = -1;
	for(int i = 0; i < RoomResource->RMDA->RMHD.NrObjects; i++)
	{
		if(RoomResource->RMDA->OBIM[i]->ObjectId == id)
		{
			obim = i;
			break;
		}
	}
	if(obim == -1 || RoomResource->RMDA->OBIM[obim]->NrImages == 0) return;

	fseek(HE1_File, RoomResource->RMDA->OBIM[obim]->IMHDOffset + 16, SEEK_SET);
	uint16_t NrHotSpots;
	readU16LE(HE1_File, &NrHotSpots);
	fseek(HE1_File, NrHotSpots * 4, SEEK_CUR);

	//uint8_t* data;
	bool found = false;
	for(int i = 0; i < RoomResource->RMDA->OBIM[obim]->NrImages; i++)
	{
		uint32_t sig;
		uint32_t size;
		readU32LE(HE1_File, &sig);
		readU32LE(HE1_File, &size);
		if(sig == IMxx_tags[state])
		{
			readU32LE(HE1_File, &sig);
			readU32LE(HE1_File, &size);
			if(sig == MKTAG('B', 'M', 'A', 'P'))
			{
				printf("BMAP\n");
				while(scanKeys(), keysHeld() == 0);
				swiDelay(5000000);
			}
			else if(sig == MKTAG('S', 'M', 'A', 'P'))
			{
				//data = (uint8_t*)malloc(SWAP_CONSTANT_32(size) - 8);
				//readBytes(HE1_File, &data[0], SWAP_CONSTANT_32(size) - 8);
				found = true;
				break;
			}
			else
			{
				printf("Unk Image Signature: %c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
				while(scanKeys(), keysHeld() == 0);
				swiDelay(5000000);
			}
		}
		else fseek(HE1_File, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
	}
	if(!found) return;

	uint16_t* dst = &TempBuffer[0];
	for(int i = 0; i < width * height; i++)
	{
		dst[i] = 0x8000;
	}

	fpos_t pos;

	fgetpos(HE1_File, &pos);

	ConvertStripeMap(HE1_File, (uint32_t)pos, (uint8_t*)dst, width, height, -1);

	for(int i = 0; i < height / 2; i++)
	{
		if((i * 2) + y < 0) continue;
		if((i * 2) + y >= 480) break;
		for(int j = 0; j < width / 2; j++)
		{
			if((j * 2) + x < 0) continue;
			if((j * 2) + x >= 640) break;
			if(dst[i * 2 * width + j * 2] != 0x8000)
				WIZFrameBuffer[(y / 2 + i) * 320 + x / 2 + j] = Merge4Pixels(dst[i * 2 * width + j * 2], dst[i * 2 * width + j * 2 + 1], dst[(i * 2 + 1) * width + j * 2], dst[(i * 2 + 1) * width + j * 2 + 1]);
		}
	}
}

void processDrawQue()
{
	for (int i = 0; i < _drawObjectQueNr; i++)
	{
		if (_drawObjectQue[i] != 0xFF) drawObject(_drawObjectQue[i], 0);
	}
	_drawObjectQueNr = 0;
}

void addObjectToDrawQue(int object)
{
	if ((unsigned int)_drawObjectQueNr >= ARRAYSIZE(_drawObjectQue)) printf("Error: Draw Object Que overflow\n");
	_drawObjectQue[_drawObjectQueNr++] = object;
}

void removeObjectFromDrawQue(int object)
{
	if (_drawObjectQueNr <= 0)
		return;

	int i;
	for (i = 0; i < _drawObjectQueNr; i++)
	{
		if (_drawObjectQue[i] == object) _drawObjectQue[i] = 0xFF;
	}
}

void clearDrawObjectQueue()
{
	_drawObjectQueNr = 0;
}

void clearDrawQueues()
{
	clearDrawObjectQueue();
	//_blastObjectQueuePos = 0;
	polygonClear();
	//clearWizBuffer();
	InitFrameBuffers();
}