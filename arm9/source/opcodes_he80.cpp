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

void _0x45_CreateSound()
{
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 27:
		pop();
		//createSound(_heSndResId, pop());
		break;
	case 217:
		//createSound(_heSndResId, -1);
		break;
	case 232:
		pop();
		//_heSndResId = pop();
		break;
	case 255:
		// dummy case
		break;
	default:
		printf("Error: o80_createSound: default case %d\n", subOp);
	}
}

static int _LOL_TESTcurrent_level = 0;

void _0x4D_ReadConfigFile()
{
	byte option[128], section[128], filename[256];
	byte *data;
	//String entry;
	int len, r;

	copyScriptString(option, sizeof(option));
	copyScriptString(section, sizeof(section));
	copyScriptString(filename, sizeof(filename));

	r = convertFilePath(filename, sizeof(filename));

	/*if (!strcmp((const char *)filename, "map (i)")) {
		// Mac resource fork config file
		// (as used by only mustard mac for map data?)
		Common::MacResManager resFork;

		if (!resFork.open((const char *)filename) || !resFork.hasResFork())
			error("Could not open '%s'", filename);

		Common::String prefResName = Common::String::format("Pref:%s.%s", (const char *)section, (const char *)option);
		Common::SeekableReadStream *res = resFork.getResource(prefResName);

		if (res) {
			// The string is inside the resource as a pascal string
			byte length = res->readByte();
			for (byte i = 0; i < length; i++)
				entry += (char)res->readByte();

			delete res;
		}
	} else {
		// Normal Windows INI files
		Common::INIFile confFile;
		if (!strcmp((char *)filename + r, "map.ini"))
			confFile.loadFromFile((const char *)filename + r);
		else
			confFile.loadFromSaveFile((const char *)filename + r);

		confFile.getKey((const char *)option, (const char *)section, entry);
	}*/

	//while(scanKeys(), keysHeld() == 0);
	//swiDelay(5000000);

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 43: // HE 100
	case 6: // number
		if (!strcmp((char *)option, "Benchmark")) push(2);
		else if (!strcmp((char*)option, "_LOL-TESTextra-dog")) push(3);
		else if (!strcmp((char*)option, "_LOL-TESTmouse-use")) push(10);
		else if (!strcmp((char *)option, "_LOL-TESTcurrent-level")) push(_LOL_TESTcurrent_level);
		else
		{
			push(/*atoi(entry.c_str())*/0);
			printf("o80_readConfigFile: Filename %s Section %s Option %s Value %s\n", filename, section, option, "");
			//while(scanKeys(), keysHeld() == 0);
			//swiDelay(5000000);
		}
		break;
	case 77: // HE 100
	case 7: // string
		{
			writeVar(0, 0);
			char* ent;
			if (!strcmp((char*)option, "_KidNames")) ent = "LOL-TEST";
			else if (!strcmp((char*)option, "_KidNamesLOL-TESTColor")) ent = "bbbggggg";
			else
			{
				ent = "";
				printf("o80_readConfigFile: Filename %s Section %s Option %s Value %s\n", filename, section, option, ent);
				//while(scanKeys(), keysHeld() == 0);
				//swiDelay(5000000);
			}
			len = resStrLen((byte*)ent);//len = resStrLen((const byte *)entry.c_str());
			data = defineArray(0, kStringArray, 0, 0, 0, len);
			memcpy(data, ent, len);//memcpy(data, entry.c_str(), len);
			push(readVar(0));
		}
		break;
	default:
		printf("Error: o80_readConfigFile: default type %d\n", subOp);
	}

	//printf("o80_readConfigFile: Filename %s Section %s Option %s Value %s\n", filename, section, option, /*entry.c_str()*/"");
	//while(scanKeys(), keysHeld() == 0);
	//swiDelay(5000000);
}

void _0x4E_WriteConfigFile()
{
	byte filename[256], section[256], option[256], string[1024];
	int r, value;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 43: // HE 100
	case 6: // number
		value = pop();
		sprintf((char *)string, "%d", value);
		copyScriptString(option, sizeof(option));
		copyScriptString(section, sizeof(section));
		copyScriptString(filename, sizeof(filename));
		if(!strcmp((char *)option, "_LOL-TESTcurrent-level")) _LOL_TESTcurrent_level = value;
		break;
	case 77: // HE 100
	case 7: // string
		copyScriptString(string, sizeof(string));
		copyScriptString(option, sizeof(option));
		copyScriptString(section, sizeof(section));
		copyScriptString(filename, sizeof(filename));
		break;
	default:
		printf("Error: o80_writeConfigFile: default type %d\n", subOp);
	}

	//r = convertFilePath(filename, sizeof(filename));

	//Common::INIFile ConfFile;
	//ConfFile.loadFromSaveFile((const char *)filename + r);
	//ConfFile.setKey((char *)option, (char *)section, (char *)string);
	//ConfFile.saveToSaveFile((const char *)filename + r);

	printf("o80_writeConfigFile: Filename %s Section %s Option %s String %s\n", filename, section, option, string);
} 

void _0x6B_CursorCommand() {
	int a, b, i;
	int args[16];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 0x13:
	case 0x14:
		a = pop();
		loadWizCursor(a, 0);
		//printf("Cursor!\n");
		//while(scanKeys(), keysHeld() == 0);
		//swiDelay(5000000);
		break;
	case 0x3C:
		b = pop();
		a = pop();
		loadWizCursor(a, b);
		//printf("Cursor!\n");
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

	state &= 0x7FFF;
	putState(obj, state);
	removeObjectFromDrawQue(obj); 
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

	ArrayHeader *ah = _arrays[readVar(value)  & ~0x33539000];//(ArrayHeader *)getResourceAddress(rtString, readVar(value));
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