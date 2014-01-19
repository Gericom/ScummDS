#include <nds.h>

#include <render.h>
#include <actor.h>
#include <akos.h>
#include <scumm.h>

uint16_t TempBuffer[640 * 480];

uint16_t BackgroundFrameBuffer[320 * 240];

uint16_t WIZFrameBuffer[320 * 240];

uint16_t ResultFrameBuffer[512 * 256];

uint16_t CursorBuffer[32 * 32 * 2];

static bool done = false;

#define VCOUNT				(*(vu16*)0x04000006)

void InitFrameBuffers()
{
	for(int j = 0; j < 640 * 480; j++)
	{
		WIZFrameBuffer[j] = 0x8000;
	}
}

static Actor* _sortedActors[62];

//640x480 -> 320x240 -> 512x256
void RenderFrame()
{
	for(int i = 0; i < 240; i++)
	{
		memcpy(&ResultFrameBuffer[i * 512], &BackgroundFrameBuffer[i * 320], 320 * 2);
	}

	for(int i = 0; i < 240; i++)
	{
		for(int j = 0; j < 320; j++)
		{
			if(WIZFrameBuffer[i * 320 + j] != 0x8000)
				ResultFrameBuffer[i * 512 + j] = WIZFrameBuffer[i * 320 + j];
		}
	}

	int numactors = 0;

	// Make a list of all actors in this room
	for (int i = 0; i < _numActors; i++) {
		if (isInCurrentRoom(&_actors[i])) {
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

			if (a->_costume && a->_cost.AKOS != NULL) {
				renderCostume(a);
				//a->drawActorCostume();
				//a->animateCostume();
			}
		} 

	}


	/*for(int i = 0; i < _numActors; i++)
	{
	if(_actors[i]._cost.AKOS != NULL && _actors[i]._visible && isInCurrentRoom(&_actors[i]))
	{
	renderCostume(&_actors[i]);
	}
	}*/

	if(_cursor_state > 0)
	{
		for(int y = 0; y < 32; y+=2)
		{
			for(int x = 0; x < 32; x+=2)
			{
				if(CursorBuffer[y * 32 + x] != 0x8000)
					ResultFrameBuffer[(_mouse_y / 2 + y / 2) * 512 + _mouse_x / 2 + x / 2] = Merge4Pixels(CursorBuffer[y * 32 + x], CursorBuffer[y * 32 + x + 1], CursorBuffer[(y + 1) * 32 + x], CursorBuffer[(y + 1) * 32 + x + 1]);
			}
		}
	}

	glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_512, TEXTURE_SIZE_256, 0, TEXGEN_TEXCOORD, (uint8_t*)&ResultFrameBuffer[0]);
}