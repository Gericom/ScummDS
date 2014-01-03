#include <nds.h>
#include <time.h>

#include <script.h>
#include <scumm.h>
#include <room.h>
#include <opcodes.h>
#include <sounds.h>
#include <wiz.h>
#include <render.h>
#include <actor.h>
#include <objects.h>
#include <sprite.h>

void _0xBD_StopObjectCode()
{
	stopObjectCode();
}

void _0xD9_CloseFile()
{
	int slot = pop();
	printf("CloseFile (%d)\n", slot);
	/*if (0 <= slot && slot < 17) {
		if (_hOutFileTable[slot]) {
			_hOutFileTable[slot]->finalize();
			delete _hOutFileTable[slot];
			_hOutFileTable[slot] = 0;
		}

		delete _hInFileTable[slot];
		_hInFileTable[slot] = 0;
	} */
}

void _0xE2_LocalizeArrayToScript()
{
	int slot = pop();
	//localizeArray(slot, _currentScript);
}