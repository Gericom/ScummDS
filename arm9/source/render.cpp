#include <nds.h>

#include <render.h>
#include <actor.h>
#include <akos.h>
#include <scumm.h>
#include <objects.h>

int _drawObjectQueNr;
uint8_t _drawObjectQue[200]; 

uint16_t TempBuffer[640 * 480];

uint16_t BackgroundFrameBuffer[320 * 240];

uint16_t WIZFrameBuffer[320 * 240];

uint16_t ObjectFrameBuffer[320 * 240];

uint16_t ResultFrameBuffer[512 * 256];

uint16_t CursorBuffer[32 * 32 * 2];

static bool done = false;

#define VCOUNT				(*(vu16*)0x04000006)
#define VRAMCNT				(*(vu32*)0x04000240)

void InitFrameBuffers()
{
	for(int j = 0; j < 320 * 240; j++)
	{
		WIZFrameBuffer[j] = 0x8000;
	}
}

void ClearObjectFrameBuffer()
{
	for(int j = 0; j < 320 * 240; j++)
	{
		ObjectFrameBuffer[j] = 0x8000;
	}
}

static Actor* _sortedActors[62];

static bool init = false;

int numactors;

//640x480 -> 320x240 -> 512x256
void RenderFrame()
{
	for(int i = 0; i < 240; i++)
	{
		for(int j = 0; j < 320; j++)
		{
			ResultFrameBuffer[i * 512 + j] = BackgroundFrameBuffer[i * 320 + j] | 0x8000;
		}
	}

	for(int i = 0; i < 240; i++)
	{
		for(int j = 0; j < 320; j++)
		{
			if(WIZFrameBuffer[i * 320 + j] != 0x8000)
				ResultFrameBuffer[i * 512 + j] = WIZFrameBuffer[i * 320 + j] | 0x8000;
		}
	}

	//for(int i = 0; i < 240; i++)
	//{
	//	for(int j = 0; j < 320; j++)
	//	{
	//		if(ObjectFrameBuffer[i * 320 + j] != 0x8000)
	//			ResultFrameBuffer[i * 512 + j] = ObjectFrameBuffer[i * 320 + j] | 0x8000;
	//	}
	//}

	numactors = 0;

	// Make a list of all actors in this room
	for (int i = 0; i < _numActors; i++) {
		if (isInCurrentRoom(&_actors[i]) && _actors[i]._costume && _actors[i]._cost.AKOS != NULL) {
			_sortedActors[numactors++] = &_actors[i];
		}
	}
	if (numactors) {
		//if (_game.heversion >= 90) {
		for (int j = 0; j < numactors; ++j) {
			for (int i = 0; i < numactors; ++i) {
				int sc_actor1 = _sortedActors[j]->_layer;
				int sc_actor2 = _sortedActors[i]->_layer;
				if (sc_actor1 < sc_actor2) {
					SWAP(_sortedActors[i], _sortedActors[j]);
				} else if (sc_actor1 == sc_actor2) {
					sc_actor1 = _sortedActors[j]->y;
					sc_actor2 = _sortedActors[i]->y;
					if (sc_actor1 < sc_actor2) {
						SWAP(_sortedActors[i], _sortedActors[j]);
					}
				}
			}
		}
		//} else {
		/*	for (int j = 0; j < numactors; ++j) {
		for (int i = 0; i < numactors; ++i) {
		int sc_actor1 = _sortedActors[j]->getPos().y - _sortedActors[j]->_layer * 2000;
		int sc_actor2 = _sortedActors[i]->getPos().y - _sortedActors[i]->_layer * 2000;
		if (sc_actor1 < sc_actor2) {
		SWAP(_sortedActors[i], _sortedActors[j]);
		}
		}
		}
		}*/

		// Finally draw the now sorted actors
		Actor** end = _sortedActors + numactors;
		for (Actor** ac = _sortedActors; ac != end; ++ac) {
			Actor* a = *ac;

			//if (a->_costume && a->_cost.AKOS != NULL) {
				renderCostume(a);
				//a->drawActorCostume();
				//a->animateCostume();
			//}
		} 

	}

	if(_cursor_state > 0)
	{
		for(int y = 0; y < 32; y+=2)
		{
			for(int x = 0; x < 32; x+=2)
			{
				if(CursorBuffer[y * 32 + x] != 0x8000)
					ResultFrameBuffer[(_mouse_y / 2 + y / 2) * 512 + _mouse_x / 2 + x / 2] = Merge4Pixels(CursorBuffer[y * 32 + x], CursorBuffer[y * 32 + x + 1], CursorBuffer[(y + 1) * 32 + x], CursorBuffer[(y + 1) * 32 + x + 1]) | 0x8000;
			}
		}
	}

	if(!init){ glTexImage2D(0, 0, GL_RGBA, TEXTURE_SIZE_512, TEXTURE_SIZE_256, 0, TEXGEN_TEXCOORD, (uint8_t*)&ResultFrameBuffer[0]); init = true;}
	else
	{
		DC_FlushRange(&ResultFrameBuffer[0], 512 * 240 * 2);
		uint32_t vr = VRAMCNT;
		while(VCOUNT != 144);
		VRAMCNT &= ~0x303;
		dmaCopyWords(0, &ResultFrameBuffer[0], (uint16_t*)0x6800000, 512 * 256);
		VRAMCNT = vr;
		while(VCOUNT != 144);
		VRAMCNT &= ~0x303;
		dmaCopyWords(0, &ResultFrameBuffer[512 * 256 / 2], &((uint16_t*)0x6800000)[512 * 256 / 2], 512 * 256);
		VRAMCNT = vr;
	}
}