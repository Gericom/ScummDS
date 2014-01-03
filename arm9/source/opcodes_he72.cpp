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

void _0x02_PushDWord()
{
	push(fetchScriptDWordSigned());
}

void _0x04_GetScriptString()
{
	byte chr;

	while ((chr = fetchScriptByte()) != 0) {
		_stringBuffer[_stringLength] = chr;
		_stringLength++;

		if (_stringLength >= 4096)
			printf("Error: String stack overflow\n");
	}

	_stringBuffer[_stringLength] = 0;
	_stringLength++;
}

void _0x1B_IsAnyOf()
{
	int args[128];
	int num, value;

	num = getStackList(args, ARRAYSIZE(args));
	value = pop();

	for (int i = 0; i < num; i++) {
		if (args[i] == value) {
			push(1);
			return;
		}
	}

	push(0);
}

void _0x58_GetTimer()
{
	int timer = pop();
	byte cmd = fetchScriptByte();

	if (cmd == 10 || cmd == 50) {
		push(getHETimer(timer));
	} else {
		push(0);
	}
}

void _0x59_SetTimer()
{
	int timer = pop();
	byte cmd = fetchScriptByte();

	if (cmd == 158 || cmd == 61) {
		setHETimer(timer);
	} else {
		printf("Error: TIMER command %d?\n", cmd);
	} 
}

void _0x5E_StartScript()
{
	int args[25];
	int script;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();

	runScript(script, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args); 
}

void _0x61_DrawObject()
{
	byte subOp = fetchScriptByte();
	int state, y, x;

	switch (subOp) {
	case 62:
		state = pop();
		y = pop();
		x = pop();
		break;
	case 63:
		state = pop();
		if (state == 0)
			state = 1;
		y = x = -100;
		break;
	case 65:
		state = 1;
		y = pop();
		x = pop();
		break;
	default:
		printf("Error: o72_drawObject: default case %d\n", subOp);
	}

	int object = pop();
	printf("Draw Object %d at (%d, %d) with state %d\n", object, x, y, state);
	int objnum = getObjectIndex(object);
	if (objnum == -1) return;

	if (y != -100 && x != -100) {
		//_objs[objnum].x_pos = x * 8;
		//_objs[objnum].y_pos = y * 8;
	}

	if (state != -1) {
		//addObjectToDrawQue(objnum);
		//putState(object, state);
	}
}

void _0x64_GetNumFreeArrays()
{
	int i, num = 0;

	for (i = 1; i < _numArray; i++) {
		if (_arrays[i] == NULL)
			num++;
	}

	push(num);
}

void _0x9C_RoomOps()
{
	int a, b, c, d, e;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 172:		// SO_ROOM_SCROLL
		b = pop();
		a = pop();
		/*if (a < (_screenWidth / 2))
		a = (_screenWidth / 2);
		if (b < (_screenWidth / 2))
		b = (_screenWidth / 2);
		if (a > _roomWidth - (_screenWidth / 2))
		a = _roomWidth - (_screenWidth / 2);
		if (b > _roomWidth - (_screenWidth / 2))
		b = _roomWidth - (_screenWidth / 2);
		VAR(VAR_CAMERA_MIN_X) = a;
		VAR(VAR_CAMERA_MAX_X) = b;*/
		break;

	case 174:		// SO_ROOM_SCREEN
		b = pop();
		a = pop();
		//initScreens(a, _screenHeight);
		break;

	case 175:		// SO_ROOM_PALETTE
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		//setPalColor(d, a, b, c);
		break;

	case 179:		// SO_ROOM_INTENSITY
		c = pop();
		b = pop();
		a = pop();
		//darkenPalette(a, a, a, b, c);
		break;

	case 180:		// SO_ROOM_SAVEGAME
		pop();
		pop();
		//_saveTemporaryState = true;
		//_saveLoadSlot = pop();
		//_saveLoadFlag = pop();
		break;

	case 181:		// SO_ROOM_FADE
		// Defaults to 1 but doesn't use fade effects
		a = pop();
		break;

	case 182:		// SO_RGB_ROOM_INTENSITY
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		//darkenPalette(a, b, c, d, e);
		break;

	case 213:		// SO_ROOM_NEW_PALETTE
		a = pop();
		//setCurrentPalette(a);
		break;

	case 220:
		a = pop();
		b = pop();
		//copyPalColor(a, b);
		break;

	case 221:
		byte buffer[256];

		copyScriptString((byte *)buffer, sizeof(buffer));

		/*_saveLoadFileName = (char *)buffer + convertFilePath(buffer, sizeof(buffer));
		printf("o72_roomOps: case 221: filename %s\n", _saveLoadFileName.c_str());

		_saveLoadFlag = pop();
		_saveLoadSlot = 255;
		_saveTemporaryState = true;*/
		break;

	case 234:
		b = pop();
		a = pop();
		//swapObjects(a, b);
		break;

	case 236:
		b = pop();
		a = pop();
		//setRoomPalette(a, b);
		break;

	default:
		printf("Error: o72_roomOps: default case %d\n", subOp);
	} 
}

void _0x9D_ActorOps()
{
	Actor* a;
	int i, j, k;
	int args[32];
	byte string[256];

	byte subOp = fetchScriptByte();
	if (subOp == 197) {
		_curActor = pop();
		return;
	}

	a = &_actors[_curActor];//(ActorHE *)derefActorSafe(_curActor, "o72_actorOps");
	//if (!a)
	//	return;

	switch (subOp) {
	case 21: // HE 80+
		k = getStackList(args, ARRAYSIZE(args));
		//for (i = 0; i < k; ++i) {
		//	a->setUserCondition(args[i] & 0x7F, args[i] & 0x80);
		//}
		break;
	case 24: // HE 80+
		k = pop();
		//if (k == 0)
		//	k = _rnd.getRandomNumberRng(1, 10);
		a->_heNoTalkAnimation = 1;
		//a->setTalkCondition(k);
		break;
	case 43: // HE 90+
		a->_layer = pop();
		a->_needRedraw = true;
		break;
	case 64:
		pop();
		pop();
		pop();
		pop();
		//_actorClipOverride.bottom = pop();
		//_actorClipOverride.right = pop();
		//_actorClipOverride.top = pop();
		//_actorClipOverride.left = pop();
		//adjustRect(_actorClipOverride);
		break;
	case 65: // HE 98+
		j = pop();
		i = pop();
		putActor(a, i, j);
		break;
	case 67: // HE 99+
		pop();
		pop();
		pop();
		pop();
		//a->_clipOverride.bottom = pop();
		//a->_clipOverride.right = pop();
		//a->_clipOverride.top = pop();
		//a->_clipOverride.left = pop();
		//adjustRect(a->_clipOverride);
		break;
	case 68: // HE 90+
		k = pop();
		//a->setHEFlag(1, k);
		break;
	case 76:		// SO_COSTUME
		setActorCostume(a, pop());
		break;
	case 77:		// SO_STEP_DIST
		j = pop();
		i = pop();
		//a->setActorWalkSpeed(i, j);
		break;
	case 78:		// SO_SOUND
		//printf("SO_SOUND:\n");
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; i++)
		{
			a->_sound[i] = args[i];
			//printf("%d\n", args[i]);
		}
		//while(scanKeys(), keysHeld() == 0);
		//swiDelay(5000000);
		break;
	case 79:		// SO_WALK_ANIMATION
		a->_walkFrame = pop();
		break;
	case 80:		// SO_TALK_ANIMATION
		a->_talkStopFrame = pop();
		a->_talkStartFrame = pop();
		break;
	case 81:		// SO_STAND_ANIMATION
		a->_standFrame = pop();
		break;
	case 82:		// SO_ANIMATION
		// dummy case in scumm6
		pop();
		pop();
		pop();
		break;
	case 83:		// SO_DEFAULT
		initActor(a, 0);
		break;
	case 84:		// SO_ELEVATION
		setElevation(a, pop());
		break;
	case 85:		// SO_ANIMATION_DEFAULT
		a->_initFrame = 1;
		a->_walkFrame = 2;
		a->_standFrame = 3;
		a->_talkStartFrame = 4;
		a->_talkStopFrame = 5;
		break;
	case 86:		// SO_PALETTE
		j = pop();
		i = pop();
		//assertRange(0, i, 255, "palette slot");
		//a->remapActorPaletteColor(i, j);
		a->_needRedraw = true;
		break;
	case 87:		// SO_TALK_COLOR
		a->_talkColor = pop();
		break;
	case 88:		// SO_ACTOR_NAME
		copyScriptString(string, sizeof(string));
		//loadPtrToResource(rtActorName, a->_number, string);
		break;
	case 89:		// SO_INIT_ANIMATION
		a->_initFrame = pop();
		break;
	case 91:		// SO_ACTOR_WIDTH
		a->_width = pop();
		break;
	case 92:		// SO_SCALE
		i = pop();
		setScale(a, i, i);
		break;
	case 93:		// SO_NEVER_ZCLIP
		a->_forceClip = 0;
		break;
	case 94:		// SO_ALWAYS_ZCLIP
		a->_forceClip = pop();
		break;
	case 95:		// SO_IGNORE_BOXES
		a->_ignoreBoxes = 1;
		a->_forceClip = 0;
		if (isInCurrentRoom(a))
			putActor(a);
		break;
	case 96:		// SO_FOLLOW_BOXES
		a->_ignoreBoxes = 0;
		a->_forceClip = 0;
		if (isInCurrentRoom(a))
			putActor(a);
		break;
	case 97:		// SO_ANIMATION_SPEED
		setAnimSpeed(a, pop());
		break;
	case 98:		// SO_SHADOW
		a->_heXmapNum = pop();
		a->_needRedraw = true;
		break;
	case 99:		// SO_TEXT_OFFSET
		a->_talkPosY = pop();
		a->_talkPosX = pop();
		break;
	case 156:		// HE 72+
		a->_charset = pop();
		break;
	case 175:		// HE 99+
		a->_hePaletteNum = pop();
		a->_needRedraw = true;
		break;
	case 198:		// SO_ACTOR_VARIABLE
		i = pop();
		setAnimVar(a, pop(), i);
		break;
	case 215:		// SO_ACTOR_IGNORE_TURNS_ON
		a->_ignoreTurns = true;
		break;
	case 216:		// SO_ACTOR_IGNORE_TURNS_OFF
		a->_ignoreTurns = false;
		break;
	case 217:		// SO_ACTOR_NEW
		initActor(a, 2);
		break;
	case 218:
		//a->drawActorToBackBuf(a->getPos().x, a->getPos().y);
		break;
	case 219:
		a->_drawToBackBuf = false;
		a->_needRedraw = true;
		a->_needBgReset = true;
		break;
	case 225:
		{
			copyScriptString(string, sizeof(string));
			int slot = pop();

			printf("Actor: %s\n", string);

			int len = resStrLen(string) + 1;
			memcpy(a->_heTalkQueue[slot].sentence, string, len);

			a->_heTalkQueue[slot].posX = a->_talkPosX;
			a->_heTalkQueue[slot].posY = a->_talkPosY;
			a->_heTalkQueue[slot].color = a->_talkColor;
			break;
		}
	default:
		printf("Error: o72_actorOps: default case %d\n", subOp);
	}
}

void _0xA0_FindObject() 
{
	int y = pop();
	int x = pop();
	int r = 0;//findObject(x, y, 0, 0);
	push(r);
} 

void _0xA4_ArrayOps()
{
	byte *data;
	byte string[1024];
	int dim1end, dim1start, dim2end, dim2start;
	int id, len, b, c, list[128];
	int offs, tmp, tmp2;
	uint tmp3;

	byte subOp = fetchScriptByte();
	int array = fetchScriptWord();
	printf("o72_arrayOps: array %d case %d\n", array, subOp);

	switch (subOp) {
	case 7:			// SO_ASSIGN_STRING
		copyScriptString(string, sizeof(string));
		len = resStrLen(string);
		data = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(data, string, len);
		break;

	case 126:
		len = getStackList(list, ARRAYSIZE(list));
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, dim2start, dim2end, dim1start, dim1end);
		}
		checkArrayLimits(array, dim2start, dim2end, dim1start, dim1end);

		tmp2 = 0;
		while (dim2start <= dim2end) {
			tmp = dim1start;
			while (tmp <= dim1end) {
				writeArray(array, dim2start, tmp, list[tmp2++]);
				if (tmp2 == len)
					tmp2 = 0;
				tmp++;
			}
			dim2start++;
		}
		break;
	case 127:
		{
			int a2_dim1end = pop();
			int a2_dim1start = pop();
			int a2_dim2end = pop();
			int a2_dim2start = pop();
			int array2 = fetchScriptWord();
			int a1_dim1end = pop();
			int a1_dim1start = pop();
			int a1_dim2end = pop();
			int a1_dim2start = pop();
			if (a1_dim1end - a1_dim1start != a2_dim1end - a2_dim1start || a2_dim2end - a2_dim2start != a1_dim2end - a1_dim2start) {
				printf("Error: Source and dest ranges size are mismatched\n");
			}
			copyArray(array, a1_dim2start, a1_dim2end, a1_dim1start, a1_dim1end, array2, a2_dim2start, a2_dim2end, a2_dim1start, a2_dim1end);
		}
		break;
	case 128:
		b = pop();
		c = pop();
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, dim2start, dim2end, dim1start, dim1end);
		}
		checkArrayLimits(array, dim2start, dim2end, dim1start, dim1end);

		offs = (b >= c) ? 1 : -1;
		tmp2 = c;
		tmp3 = c - b + 1;
		while (dim2start <= dim2end) {
			tmp = dim1start;
			while (tmp <= dim1end) {
				writeArray(array, dim2start, tmp, tmp2);
				if (--tmp3 == 0) {
					tmp2 = c;
					tmp3 = c - b + 1;
				} else {
					tmp2 += offs;
				}
				tmp++;
			}
			dim2start++;
		}
		break;
	case 194:
		decodeScriptString(string);
		len = resStrLen(string);
		data = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(data, string, len);
		break;
	case 208:		// SO_ASSIGN_INT_LIST
		b = pop();
		c = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, 0, 0, 0, b + c - 1);
		}
		while (c--) {
			writeArray(array, 0, b + c, pop());
		}
		break;
	case 212:		// SO_ASSIGN_2DIM_LIST
		printf("SO_ASSIGN_2DIM_LIST\n");
		len = getStackList(list, ARRAYSIZE(list));
		id = readVar(array);
		if (id == 0) printf("Error: Must DIM a two dimensional array before assigning\n");
		c = pop();
		while (--len >= 0) {
			writeArray(array, c, len, list[len]);
		}
		break;
	default:
		printf("Error: o72_arrayOps: default case %d (array %d)\n", subOp, array);
	}
}

void _0xAE_SystemOps()
{
	byte string[1024];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 22: // HE80+
		printf("clearDrawObjectQueue\n");
		InitFrameBuffers();
		//while(1);
		//clearDrawObjectQueue();
		break;
	case 26: // HE80+
		printf("restoreBackgroundHE\n");
		InitFrameBuffers();
		//while(1);
		//restoreBackgroundHE(Common::Rect(_screenWidth, _screenHeight));
		//updatePalette();
		break;
	case 158:
		printf("Restart\n");
		while(1);
		//restart();
		break;
	case 160:
		// Confirm shutdown
		printf("Confirm Shutdown\n");
		while(1);
		//confirmExitDialog();
		break;
	case 244:
		printf("Quit\n");
		while(1);
		//quitGame();
		break;
	case 251:
		copyScriptString(string, sizeof(string));
		printf("Start executable (%s)\n", string);
		while(1);
		break;
	case 252:
		copyScriptString(string, sizeof(string));
		printf("Start game (%s)\n", string);
		while(1);
		break;
	default:
		printf("Error: o72_systemOps invalid case %d\n", subOp);
		while(1);
		break;
	}
}

void _0xBA_TalkActor()
{
	Actor *a;

	int act = pop();

	_string[0].loadDefault();

	// A value of 225 can occur when examining the gold in the mine of pajama, after mining the gold.
	// This is a script bug, the script should set the subtitle color, not actor number.
	// This script bug was fixed in the updated version of pajama.
	if (act == 225) {
		_string[0].color = act;
	} else {
		_actorToPrintStrFor = act;
		if (_actorToPrintStrFor != 0xFF) {
			a = &_actors[_actorToPrintStrFor];//derefActor(_actorToPrintStrFor, "o72_talkActor");
			_string[0].color = a->_talkColor;
		}
	}

	printf("actorTalk:\n");
	printf("%s\n", _scriptPointer);
	actorTalk(_scriptPointer);

	_scriptPointer += resStrLen(_scriptPointer) + 1;
}

void _0xBC_DimArray()
{
	int data;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 2:		// SO_BIT_ARRAY
		data = kBitArray;
		break;
	case 3:		// SO_NIBBLE_ARRAY
		data = kNibbleArray;
		break;
	case 4:		// SO_BYTE_ARRAY
		data = kByteArray;
		break;
	case 5:		// SO_INT_ARRAY
		data = kIntArray;
		break;
	case 6:
		data = kDwordArray;
		break;
	case 7:		// SO_STRING_ARRAY
		data = kStringArray;
		break;
	case 204:		// SO_UNDIM_ARRAY
		nukeArray(fetchScriptWord());
		return;
	default:
		printf("Error: o72_dimArray: default case %d\n", subOp);
	}

	defineArray(fetchScriptWord(), data, 0, 0, 0, pop());
}

void _0xC0_Dim2DimArray()
{
	int data, dim1end, dim2end;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 2:		// SO_BIT_ARRAY
		data = kBitArray;
		break;
	case 3:		// SO_NIBBLE_ARRAY
		data = kNibbleArray;
		break;
	case 4:		// SO_BYTE_ARRAY
		data = kByteArray;
		break;
	case 5:		// SO_INT_ARRAY
		data = kIntArray;
		break;
	case 6:
		data = kDwordArray;
		break;
	case 7:		// SO_STRING_ARRAY
		data = kStringArray;
		break;
	default:
		printf("Error: o72_dim2dimArray: default case %d\n", subOp);
	}

	dim1end = pop();
	dim2end = pop();
	defineArray(fetchScriptWord(), data, 0, dim2end, 0, dim1end); 
}

void _0xC1_TraceStatus()
{
	byte string[80];

	copyScriptString(string, sizeof(string));
	pop();
} 

void _0xCE_DrawWizImage() {
	WizImage wi;
	wi.flags = pop();
	wi.y1 = pop();
	wi.x1 = pop();
	wi.resNum = pop();
	wi.state = 0;
	displayWizImage(&wi);
}

void _0xD5_JumpToScript()
{
	int args[25];
	int script;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();
	stopObjectCode();
	runScript(script, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args);
}

void _0xDA_OpenFile()
{
	int mode, slot, i;
	byte buffer[256];

	mode = pop();
	copyScriptString(buffer, sizeof(buffer));
	printf("Original filename %s\n", buffer);

	//const char *filename = (char *)buffer + convertFilePath(buffer, sizeof(buffer));
	//debug("Final filename to %s\n", filename);

	/*slot = -1;
	for (i = 1; i < 17; i++) {
		if (_hInFileTable[i] == 0 && _hOutFileTable[i] == 0) {
			slot = i;
			break;
		}
	}

	if (slot != -1) {
		switch (mode) {
		case 1:   // Read mode
			if (!_saveFileMan->listSavefiles(filename).empty()) {
				_hInFileTable[slot] = _saveFileMan->openForLoading(filename);
			} else {
				_hInFileTable[slot] = SearchMan.createReadStreamForMember(filename);
			}
			break;
		case 2:   // Write mode
			if (!strchr(filename, '/')) {
				_hOutFileTable[slot] = _saveFileMan->openForSaving(filename);
			}
			break;
		case 6: { // Append mode
			if (strchr(filename, '/'))
				break;

			// First check if the file already exists
			Common::InSaveFile *initialState = 0;
			if (!_saveFileMan->listSavefiles(filename).empty())
				initialState = _saveFileMan->openForLoading(filename);
			else
				initialState = SearchMan.createReadStreamForMember(filename);

			// Read in the data from the initial file
			uint32 initialSize = 0;
			byte *initialData = 0;
			if (initialState) {
				initialSize = initialState->size();
				initialData = new byte[initialSize];
				initialState->read(initialData, initialSize);
				delete initialState;
			}

			// Attempt to open a save file
			_hOutFileTable[slot] = _saveFileMan->openForSaving(filename);

			// Begin us off with the data from the previous file
			if (_hOutFileTable[slot] && initialData) {
				_hOutFileTable[slot]->write(initialData, initialSize);
				delete[] initialData;
			}

			} break;
		default:
			printf("Error: o72_openFile(): wrong open file mode %d\n", mode);
		}

		if (_hInFileTable[slot] == 0 && _hOutFileTable[slot] == 0)
			slot = -1;

	}
	printf("o72_openFile: slot %d, mode %d\n", slot, mode);*/
	push(/*slot*/1); 
}

void _0xDD_FindAllObjects()
{
	int room = pop();
	int i;

	if (room != _currentRoom)
		printf("Error: o72_findAllObjects: current room is not %d\n", room);

	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, /*_numLocalObjects*/RoomResource->RMDA->RMHD.NrObjects);
	writeArray(0, 0, 0, /*_numLocalObjects*/RoomResource->RMDA->RMHD.NrObjects);

	for (i = /*1*/0; i < /*_numLocalObjects*/RoomResource->RMDA->RMHD.NrObjects; i++) {
		writeArray(0, 0, i, /*_objs[i].obj_nr*/RoomResource->RMDA->OBCD[i]->ObjectId);
	}

	push(readVar(0));
}

void _0xDE_DeleteFile()
{
	byte buffer[256];

	copyScriptString(buffer, sizeof(buffer));
	//const char *filename = (char *)buffer + convertFilePath(buffer, sizeof(buffer));

	printf("o72_deleteFile(%s)\n", buffer);

	//if (!_saveFileMan->listSavefiles(filename).empty()) {
	//	_saveFileMan->removeSavefile(filename);
	//}
}

void _0xE1_GetPixel()
{
	uint16 area;

	int y = pop();
	int x = pop();
	byte subOp = fetchScriptByte();

	push(-1);

	//VirtScreen *vs = findVirtScreen(y);
	//if (vs == NULL || x > _screenWidth - 1 || x < 0) {
	//	push(-1);
	//	return;
	//}

	/*switch (subOp) {
	case 9: // HE 100
	case 218:
		if (_game.features & GF_16BIT_COLOR)
			area = READ_UINT16(vs->getBackPixels(x, y - vs->topline));
		else
			area = *vs->getBackPixels(x, y - vs->topline);
		break;
	case 8: // HE 100
	case 219:
		if (_game.features & GF_16BIT_COLOR)
			area = READ_UINT16(vs->getPixels(x, y - vs->topline));
		else
			area = *vs->getPixels(x, y - vs->topline);
		break;
	default:
		error("o72_getPixel: default case %d", subOp);
	}
	push(area);*/
} 

void _0xF3_ReadINI()
{
	byte option[128];
	byte *data;

	copyScriptString(option, sizeof(option));
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 43: // HE 100
	case 6: // number
		if (!strcmp((char *)option, "DisablePrinting") || !strcmp((char *)option, "NoPrinting")) {
			push(1);
		} else if (!strcmp((char *)option, "TextOn")) {
			push(/*ConfMan.getBool("subtitles")*/1);
		} else{// if(!strcmp((char*)option, "BlackAndWhiteCursors") || !strcmp((char*)option, "SputmDebug") || !strcmp((char*)option, "NeverQuit") || !strcmp((char*)option, "NeverRestartAtEnd") || !strcmp((char*)option, "MouseOverride") || !strcmp((char*)option, "NoControlPanel") || !strcmp((char*)option, "Demo") || ) {
			push(0);
			//push(ConfMan.getInt((char *)option));
		}
		//else 
		//{
		//	printf("o72_readINI: %d Option %s\n", subOp, option); 
		//	while(1);
		//}
		break;
	case 77: // HE 100
	case 7: // string
		writeVar(0, 0);
		/*if (!strcmp((char *)option, "HE3File")) {
		Common::String fileName = generateFilename(-3);
		int len = resStrLen((const byte *)fileName.c_str());
		data = defineArray(0, kStringArray, 0, 0, 0, len);
		memcpy(data, fileName.c_str(), len);
		} else if (!strcmp((char *)option, "GameResourcePath") || !strcmp((char *)option, "SaveGamePath")) {
		// We set SaveGamePath in order to detect where it used
		// in convertFilePath and to avoid warning about invalid
		// path in Macintosh verisons.
		data = defineArray(0, kStringArray, 0, 0, 0, 2);
		if (_game.platform == Common::kPlatformMacintosh)
		memcpy(data, (const char *)"*:", 2);
		else
		memcpy(data, (const char *)"*\\", 2);
		} else {
		const char *entry = (ConfMan.get((char *)option).c_str());
		int len = resStrLen((const byte *)entry);
		data = defineArray(0, kStringArray, 0, 0, 0, len);
		memcpy(data, entry, len);
		}*/
		push(/*readVar(0)*/0);
		break;
	default:
		printf("Error: o72_readINI: default type %d\n", subOp);
	}

	printf("o72_readINI: %d Option %s\n", subOp, option); 
	//while(1);
}

void _0xF4_WriteINI()
{
	int value;
	byte option[256], string[1024];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 43: // HE 100
	case 6: // number
		value = pop();
		copyScriptString(option, sizeof(option));
		printf("o72_writeINI: %s = %d\n", option, value);

		//ConfMan.setInt((char *)option, value);
		break;
	case 77: // HE 100
	case 7: // string
		copyScriptString(string, sizeof(string));
		copyScriptString(option, sizeof(option));
		printf("o72_writeINI: %s = %s\n", option, string);

		// Filter out useless setting
		if (!strcmp((char *)option, "HETest"))
			return;

		// Filter out confusing subtitle setting
		if (!strcmp((char *)option, "TextOn"))
			return;

		// Filter out confusing path settings
		if (!strcmp((char *)option, "DownLoadPath") || !strcmp((char *)option, "GameResourcePath") || !strcmp((char *)option, "SaveGamePath"))
			return;

		//ConfMan.set((char *)option, (char *)string);
		break;
	default:
		printf("Error: o72_writeINI: default type %d\n", subOp);
	}

	//ConfMan.flushToDisk(); 
}

void _0xF9_CreateDirectory()
{
	byte directoryName[255];

	copyScriptString(directoryName, sizeof(directoryName));
	printf("o72_createDirectory: %s\n", directoryName);
}

void _0xFA_SetSystemMessage()
{
	byte name[1024];

	copyScriptString(name, sizeof(name));
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 240:
		printf("Debug: o72_setSystemMessage: (%d) %s\n", subOp, name);
		break;
	case 241: // Set Version
		printf("Debug: o72_setSystemMessage: (%d) %s\n", subOp, name);
		break;
	case 242:
		printf("Debug: o72_setSystemMessage: (%d) %s\n", subOp, name);
		break;
	case 243: // Set Window Caption
		printf("Debug: o72_setSystemMessage: (%d) %s\n", subOp, name);
		break;
	default:
		printf("Error: o72_setSystemMessage: default case %d\n", subOp);
	}
}