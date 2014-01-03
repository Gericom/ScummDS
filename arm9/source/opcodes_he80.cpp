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

void _0x6B_CursorCommand() {
	int a, b, i;
	int args[16];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 0x13:
	case 0x14:
		a = pop();
		loadWizCursor(a, 0);
		printf("Cursor!\n");
		//while(scanKeys(), keysHeld() == 0);
		//swiDelay(5000000);
		break;
	case 0x3C:
		b = pop();
		a = pop();
		loadWizCursor(a, b);
		printf("Cursor!\n");
		//while(scanKeys(), keysHeld() == 0);
		//swiDelay(5000000);
		break;
	case 0x90:		// SO_CURSOR_ON Turn cursor on
		_cursor_state = 1;
		break;
	case 0x91:		// SO_CURSOR_OFF Turn cursor off
		_cursor_state = 0;
		break;
	case 0x92:		// SO_USERPUT_ON
		_userPut = 1;
		break;
	case 0x93:		// SO_USERPUT_OFF
		_userPut = 0;
		break;
	case 0x94:		// SO_CURSOR_SOFT_ON Turn soft cursor on
		_cursor_state++;
		if (_cursor_state > 1)
			printf("Error: Cursor state greater than 1 in script\n");
		break;
	case 0x95:		// SO_CURSOR_SOFT_OFF Turn soft cursor off
		_cursor_state--;
		break;
	case 0x96:		// SO_USERPUT_SOFT_ON
		_userPut++;
		break;
	case 0x97:		// SO_USERPUT_SOFT_OFF
		_userPut--;
		break;
	case 0x9C:		// SO_CHARSET_SET
		pop();
		//initCharset(pop());
		break;
	case 0x9D:		// SO_CHARSET_COLOR
		getStackList(args, ARRAYSIZE(args));
		//for (i = 0; i < 16; i++)
		//	_charsetColorMap[i] = _charsetData[_string[1]._default.charset][i] = (unsigned char)args[i];
		break;
	default:
		printf("Error: o80_cursorCommand: default case %x\n", subOp);
	}

	VAR(VAR_CURSORSTATE) = _cursor_state;
	VAR(VAR_USERPUT) = _userPut; 
}

void _0x70_SetState()
{
	int state = pop();
	int obj = pop();

	//state &= 0x7FFF;
	//putState(obj, state);
	//removeObjectFromDrawQue(obj); 
}

void _0xE3_PickVarRandom()
{
	int num;
	int args[100];
	int32 dim1end;

	num = getStackList(args, ARRAYSIZE(args));
	int value = fetchScriptWord();

	if (readVar(value) == 0) {
		defineArray(value, kDwordArray, 0, 0, 0, num);
		//if (value & 0x8000)
		//	localizeArray(readVar(value), 0xFF);
		//else if (value & 0x4000)
		//	localizeArray(readVar(value), _currentScript);

		if (num > 0) {
			int16 counter = 0;
			do {
				writeArray(value, 0, counter + 1, args[counter]);
			} while (++counter < num);
		}

		shuffleArray(value, 1, num);
		writeArray(value, 0, 0, 2);
		push(readArray(value, 0, 1));
		return;
	}

	num = readArray(value, 0, 0);

	ArrayHeader *ah = _arrays[readVar(value)];//(ArrayHeader *)getResourceAddress(rtString, readVar(value));
	dim1end = ah->dim1end;

	if (dim1end < num) {
		int32 var_2 = readArray(value, 0, num - 1);
		shuffleArray(value, 1, dim1end);
		num = 1;
		if (readArray(value, 0, 1) == var_2 && dim1end >= 3) {
			int32 tmp = readArray(value, 0, 2);
			writeArray(value, 0, num, tmp);
			writeArray(value, 0, 2, var_2);
		}
	}

	writeArray(value, 0, 0, num + 1);
	push(readArray(value, 0, num)); 
}