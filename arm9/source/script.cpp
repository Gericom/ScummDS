#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <HE0.h>
#include <HE1.h>
#include <scumm.h>
#include <script.h>
#include <resources.h>
#include <room.h>
#include <opcodes.h>
#include <actor.h>

uint32_t _numGlobalScripts;
VirtualMachineState vm;

//uint8_t scriptdata[16384];

uint32 _localScriptOffsets[1024];
const byte *_scriptPointer;
byte *_scriptOrgPointer = NULL;// = &scriptdata[0];
//const byte * const *_lastCodePtr;
byte _opcode;
byte _currentScript;
int _scummStackPos;
int _vmStack[150];

int _stringLength;
byte _stringBuffer[4096];

int16 _varwatch;
int32 _roomVars[4096];
int32 _scummVars[1024];
//byte _bitVars[512];

uint32_t _numArray = 512;

ArrayHeader* _arrays[512];

static int done = 0;

//void* mallocs[1024];
//int mallocsoffset = 0;

/* Start executing script 'script' with the given parameters */
void runScript(int script, bool freezeResistant, bool recursive, int *lvarptr, int cycle) {
	ScriptSlot *s;
	//byte *scriptPtr;
	uint32 scriptOffs;
	byte scriptType;
	int slot;

	if (!script)
		return;

	if (!recursive)
		stopScript(script);

	uint16 number = (_currentScript != 0xFF) ? vm.slot[_currentScript].number : 0;

	if (script < _numGlobalScripts) {
		// Call getResourceAddress to ensure the resource is loaded & its usage count reset
		/*scriptPtr =*/
		//getResourceAddress(rtScript, script);
		//if(done < 3)
		//{
			 //getGlobalScriptPtr(script);
		//	 done++;
		//}
		scriptOffs = 0;//_resourceHeaderSize;
		scriptType = WIO_GLOBAL;

		printf("Debug: runScript(Global-%d) from %d-%d\n", script, number, _roomResource);
	} else {
		//if((script - _numGlobalScripts) >= RoomResource->RMDA->NrLocalScripts) printf("Error: Local script %d is not in room %d\n", script, _roomResource);
		scriptOffs = 0;//_localScriptOffsets[script - _numGlobalScripts];
		scriptType = WIO_LOCAL;

		printf("Debug: runScript(%d) from %d-%d\n", script, number, _roomResource);
		//while(1);
	}

	if (cycle == 0)
		cycle = VAR(VAR_SCRIPT_CYCLE);

	slot = getScriptSlot();

	s = &vm.slot[slot];
	s->number = script;
	s->offs = scriptOffs;
	s->status = ssRunning;
	s->where = scriptType;
	s->freezeResistant = freezeResistant;
	s->recursive = recursive;
	s->freezeCount = 0;
	s->delayFrameCount = 0;
	s->cycle = cycle;

	initializeLocals(slot, lvarptr);

	runScriptNested(slot);
} 

void initializeLocals(int slot, int *vars) {
	int i;
	if (!vars) {
		for (i = 0; i < NUM_SCRIPT_LOCAL; i++)
			vm.localvar[slot][i] = 0;
	} else {
		for (i = 0; i < NUM_SCRIPT_LOCAL; i++)
			vm.localvar[slot][i] = vars[i];
	}
} 

/* Stop script 'script' */
void stopScript(int script) {
	ScriptSlot *ss;
	int i;

	if (script == 0)
		return;

	ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (script == ss->number && ss->status != ssDead &&
			(ss->where == WIO_GLOBAL || ss->where == WIO_LOCAL)) {
			if (ss->cutsceneOverride) printf("Error: Script %d stopped with active cutscene/override\n", script);
			ss->number = 0;
			ss->status = ssDead;
			//nukeArrays(i);
			if (_currentScript == i)
				_currentScript = 0xFF;
		}
	}

	for (i = 0; i < vm.numNestedScripts; ++i) {
		if (vm.nest[i].number == script &&
				(vm.nest[i].where == WIO_GLOBAL || vm.nest[i].where == WIO_LOCAL)) {
			//nukeArrays(vm.nest[i].slot);
			vm.nest[i].number = 0;
			vm.nest[i].slot = 0xFF;
			vm.nest[i].where = 0xFF;
		}
	}
}

/* Return a free script slot */
int getScriptSlot() {
	ScriptSlot *s;
	int i;

	for (i = 1; i < NUM_SCRIPT_SLOT; i++) {
		s = &vm.slot[i];
		if (s->status == ssDead)
			return i;
	}
	printf("Error: Too many scripts running, %d max\n", NUM_SCRIPT_SLOT);
	return -1;
}

/* Run script 'script' nested - eg, within the parent script.*/
void runScriptNested(int script) {
	NestedScript *nest;
	ScriptSlot *slot;

	updateScriptPtr();

	if (vm.numNestedScripts >= kMaxScriptNesting) printf("Error: Too many nested scripts\n");

	nest = &vm.nest[vm.numNestedScripts];

	if (_currentScript == 0xFF) {
		nest->number = 0;
		nest->where = 0xFF;
	} else {
		// Store information about the currently running script
		slot = &vm.slot[_currentScript];
		nest->number = slot->number;
		nest->where = slot->where;
		nest->slot = _currentScript;
		//
	}

	vm.numNestedScripts++;

	_currentScript = script;
	getScriptBaseAddress();
	resetScriptPointer();
	executeScript();

	//free((void*)_scriptOrgPointer);

	if (vm.numNestedScripts != 0)
		vm.numNestedScripts--;

	if (nest->number) {
		// Try to resume the script which called us, if its status has not changed
		// since it invoked us. In particular, we only resume it if it hasn't been
		// stopped in the meantime, and if it did not already move on.
		slot = &vm.slot[nest->slot];
		if (slot->number == nest->number && slot->where == nest->where &&
				slot->status != ssDead && slot->freezeCount == 0) {
			_currentScript = nest->slot;
			getScriptBaseAddress();
			resetScriptPointer();
			return;
		}
	}
	_currentScript = 0xFF;
}

void updateScriptPtr() {
	if (_currentScript == 0xFF)
		return;

	vm.slot[_currentScript].offs = _scriptPointer - _scriptOrgPointer;
}

/* Get the code pointer to a script */
void getScriptBaseAddress() {
	ScriptSlot *ss;
	int idx;

	//void* old = (void*)_scriptOrgPointer;

	if (_currentScript == 0xFF)
		return;


	ss = &vm.slot[_currentScript];
	switch (ss->where) {
	//case WIO_INVENTORY:					/* inventory script * */
	//	for (idx = 0; idx < _numInventory; idx++)
	//		if (_inventory[idx] == ss->number)
	//			break;
	//	_scriptOrgPointer = getResourceAddress(rtInventory, idx);
	//	assert(idx < _numInventory);
	//	_lastCodePtr = &_res->_types[rtInventory][idx]._address;
	//	break;

	case WIO_LOCAL:
		_scriptOrgPointer = (byte*)getLocalScriptPtr(ss->number);
		//_scriptOrgPointer = getResourceAddress(rtRoom, CurrentRoomId);
		//_lastCodePtr = &_res->_types[rtRoom][CurrentRoomId]._address;
		break;
	case WIO_ROOM:								/* room script */
		if (ss->number == 10001) _scriptOrgPointer = (byte*)getExitScript();//RoomResource->RMDA->EXCD;
		else if (ss->number == 10002) _scriptOrgPointer = (byte*)getEntryScript();//RoomResource->RMDA->ENCD;
		else 
		{
			printf("Error: Unknown WIO_ROOM Script %d\n", ss->number);
			while(1);
		}
		break;

	case WIO_GLOBAL:							/* global script */
		_scriptOrgPointer = (byte*)getGlobalScriptPtr(ss->number);//getResourceAddress(rtScript, ss->number);
		//_lastCodePtr = &_res->_types[rtScript][ss->number]._address;
		break;

	//case WIO_FLOBJECT:						/* flobject script */
	//	idx = getObjectIndex(ss->number);
	//	assert(idx != -1);
	//	idx = _objs[idx].fl_object_index;
	//	_scriptOrgPointer = getResourceAddress(rtFlObject, idx);
	//	assert(idx < _numFlObject);
	//	_lastCodePtr = &_res->_types[rtFlObject][idx]._address;
	//	break;
	default:
		printf("Error: Bad type while getting base address\n");
		while(1);
	}

	//mallocs[mallocsoffset++] = _scriptOrgPointer;

	//free(old);
} 

void resetScriptPointer() {
	if (_currentScript == 0xFF)
		return;
	_scriptPointer = _scriptOrgPointer + vm.slot[_currentScript].offs;
}

/**
 * This method checks whether the resource that contains the active script
 * moved, and if so, updates the script pointer accordingly.
 *
 * The script resource may have moved because it might have been garbage
 * collected by ResourceManager::expireResources.
 */
void refreshScriptPointer() {
	/*if (*_lastCodePtr != _scriptOrgPointer) {
		long oldoffs = _scriptPointer - _scriptOrgPointer;
		getScriptBaseAddress();
		_scriptPointer = _scriptOrgPointer + oldoffs;
	}*/
}

/** Execute a script - Read opcode, and execute it from the table */
void executeScript() {
	int c;
	while (_currentScript != 0xFF) {

		_opcode = fetchScriptByte();
		vm.slot[_currentScript].didexec = true;
		//printf("Script %d, offset 0x%x: [%X]\n", vm.slot[_currentScript].number, (uint)(_scriptPointer - _scriptOrgPointer) - 1, _opcode);

		executeOpcode(_opcode);

		/*This enables stepping through the code with the A button
		while(1)
		{
			swiDelay(1000000); //Make this bigger for slower stepping
			scanKeys();
			u16 keys = keysHeld();
			if((keys & KEY_A))
			{
				break;
			}
		}*/
	}
}

void executeOpcode(byte i)
{
	switch(i)
	{
	case 0x00: _0x00_PushByte(); break;
	case 0x01: _0x01_PushWord(); break;
	case 0x02: _0x02_PushDWord(); break;
	case 0x03: _0x03_PushWordVar(); break;
	case 0x04: _0x04_GetScriptString(); break;
	case 0x07: _0x07_WordArrayRead(); break;
	case 0x0A: _0x0A_Dup_n(); break;
	case 0x0B: _0x0B_WordArrayIndexedRead(); break;
	case 0x0C: _0x0C_Dup(); break;
	case 0x0D: _0x0D_Not(); break;
	case 0x0E: _0x0E_Eq(); break;
	case 0x0F: _0x0F_Neq(); break;
	case 0x10: _0x10_Gt(); break;
	case 0x11: _0x11_Lt(); break;
	case 0x12: _0x12_Le(); break;
	case 0x13: _0x13_Ge(); break;
	case 0x14: _0x14_Add(); break;
	case 0x15: _0x15_Sub(); break;
	case 0x16: _0x16_Mul(); break;
	case 0x17: _0x17_Div(); break;
	case 0x18: _0x18_Land(); break;
	case 0x19: _0x19_Lor(); break;
	case 0x1A: _0x1A_Pop(); break;
	case 0x1B: _0x1B_IsAnyOf(); break;
	case 0x1C: _0x1C_WizImageOps(); break;
	case 0x1D: _0x1D_Min(); break;
	case 0x1E: _0x1E_Max(); break;
	case 0x22: _0x22_ATan2(); break;
	case 0x25: _0x25_GetSpriteInfo(); break;
	case 0x26: _0x26_SetSpriteInfo(); break;
	case 0x28: _0x28_SetSpriteGroupInfo(); break;
	case 0x34: _0x34_FindAllObjectsWithClassOf(); break;
	case 0x37: _0x37_Dim2Dim2Array(); break;
	case 0x43: _0x43_WriteWordVar(); break;
	case 0x46: _0x46_ByteArrayWrite(); break;
	case 0x47: _0x47_WordArrayWrite(); break;
	case 0x4B: _0x4B_WordArrayIndexedWrite(); break;
	case 0x4F: _0x4F_WordVarInc(); break;
	case 0x53: _0x53_WordArrayInc(); break;
	case 0x57: _0x57_WordVarDec(); break;
	case 0x58: _0x58_GetTimer(); break;
	case 0x59: _0x59_SetTimer(); break;
	case 0x5C: _0x5C_If(); break;
	case 0x5D: _0x5D_IfNot(); break;
	case 0x5E: _0x5E_StartScript(); break;
	case 0x61: _0x61_DrawObject(); break;
	case 0x64: _0x64_GetNumFreeArrays(); break;
	case 0x65: _0x65_StopObjectCode(); break;
	case 0x66: _0x66_StopObjectCode(); break;
	case 0x6B: _0x6B_CursorCommand(); break;
	case 0x6C: _0x6C_BreakHere(); break;
	case 0x6D: _0x6D_IfClassOfIs(); break;
	case 0x6E: _0x6E_SetClass(); break;
	case 0x6F: _0x6F_GetState(); break;
	case 0x70: _0x70_SetState(); break;
	case 0x73: _0x73_Jump(); break;
	case 0x74: _0x74_StartSound(); break;
	case 0x75: _0x75_StopSound(); break;
	case 0x7B: _0x7B_LoadRoom(); break;
	case 0x7C: _0x7C_StopScript(); break;
	case 0x7F: _0x7F_PutActorAtXY(); break;
	case 0x82: _0x82_AnimateActor(); break;
	case 0x87: _0x87_GetRandomNumber(); break;
	case 0x88: _0x88_GetRandomNumberRange(); break;
	case 0x8B: _0x8B_IsScriptRunning(); break;
	case 0x8C: _0x8C_GetActorRoom(); break;
	case 0x8E: _0x8E_GetObjectY(); break;
	case 0x91: _0x91_GetActorCostume(); break;
	case 0x92: _0x92_FindInventory(); break;
	case 0x95: _0x95_BeginOverride(); break;
	case 0x96: _0x96_EndOverride(); break;
	case 0x98: _0x98_IsSoundRunning(); break;
	case 0x9B: _0x9B_ResourceRoutines(); break;
	case 0x9C: _0x9C_RoomOps(); break;
	case 0x9D: _0x9D_ActorOps(); break;
	case 0x9F: _0x9F_GetActorFromXY(); break;
	case 0xA0: _0xA0_FindObject(); break;
	case 0xA4: _0xA4_ArrayOps(); break;
	case 0xA5: _0xA5_FontUnk(); break;
	case 0xA6: _0xA6_DrawBox(); break;
	case 0xA7: _0xA7_Pop(); break;
	case 0xA9: _0xA9_Wait(); break;
	case 0xAD: _0xAD_IsAnyOf(); break;
	case 0xAE: _0xAE_SystemOps(); break;
	case 0xB0: _0xB0_Delay(); break;
	case 0xB1: _0xB1_DelaySeconds(); break;
	case 0xB3: _0xB3_StopSentence(); break;
	case 0xB4: _0xB4_PrintLine(); break;
	case 0xB5: _0xB5_PrintText(); break;
	case 0xB6: _0xB6_PrintDebug(); break;
	case 0xB7: _0xB7_PrintSystem(); break;
	case 0xB8: _0xB8_PrintActor(); break;
	case 0xB9: _0xB9_PrintEgo(); break;
	case 0xBC: _0xBC_DimArray(); break;
	case 0xBD: _0xBD_StopObjectCode(); break;
	case 0xBF: _0xBF_StartScriptQuick2(); break;
	case 0xC0: _0xC0_Dim2DimArray(); break;
	case 0xC1: _0xC1_TraceStatus(); break;
	case 0xC9: _0xC9_KernelSetFunctions(); break;
	case 0xCA: _0xCA_DelayFrames(); break;
	case 0xCB: _0xCB_PickOneOf(); break;
	case 0xCE: _0xCE_DrawWizImage(); break;
	case 0xD0: _0xD0_GetDateTime(); break;
	case 0xD1: _0xD1_StopTalking(); break;
	case 0xD2: _0xD2_GetAnimateVariable(); break;
	case 0xD6: _0xD6_BAnd(); break;
	case 0xD7: _0xD7_BOr(); break;
	case 0xD9: _0xD9_CloseFile(); break;
	case 0xDA: _0xDA_OpenFile(); break;
	case 0xDD: _0xDD_FindAllObjects(); break;
	case 0xDE: _0xDE_DeleteFile(); break;
	case 0xE1: _0xE1_GetPixel(); break;
	case 0xE2: _0xE2_LocalizeArrayToScript(); break;
	case 0xE3: _0xE3_PickVarRandom(); break;
	case 0xEE: _0xEE_GetStringLength(); break;
	case 0xF2: _0xF2_IsResourceLoaded(); break;
	case 0xF3: _0xF3_ReadINI(); break;
	case 0xF4: _0xF4_WriteINI(); break;
	case 0xF9: _0xF9_CreateDirectory(); break;
	case 0xFA: _0xFA_SetSystemMessage(); break;
	case 0xFB: _0xFB_PolygonOps(); break;
	default:
		printf("Script %d, offset 0x%x: [%X]\n", vm.slot[_currentScript].number, (uint)(_scriptPointer - _scriptOrgPointer) - 1, _opcode);
		printf("Unknown Opcode\n");
		while(1);
		break;
	}
}

byte fetchScriptByte() {
	//refreshScriptPointer();
	return *_scriptPointer++;
}

uint16 fetchScriptWord() {
	//refreshScriptPointer();
	return (fetchScriptByte() | fetchScriptByte() << 8);
	//uint a = *((uint16_t*)_scriptPointer);
	//_scriptPointer += 2;
	//return a;
}

int16 fetchScriptWordSigned() {
	return (int16)fetchScriptWord();
}

uint fetchScriptDWord() {
	return (fetchScriptByte() | (fetchScriptByte() << 8) | (fetchScriptByte() << 16) | (fetchScriptByte() << 24));
	//refreshScriptPointer();
	//uint a = *((uint32_t*)_scriptPointer);
	//_scriptPointer += 4;
	//return a;
}

int fetchScriptDWordSigned() {
	return (int32)fetchScriptDWord();
} 

int readVar(uint var) {
	int a;

	//printf("readvar(%d)\n", var);


	if (!(var & 0xF000)) {

		/*if (VAR_SUBTITLES != 0xFF && var == VAR_SUBTITLES) {
			return ConfMan.getBool("subtitles");
		}
		if (VAR_NOSUBTITLES != 0xFF && var == VAR_NOSUBTITLES) {
			return !ConfMan.getBool("subtitles");
		}*/

		//assertRange(0, var, _numVariables - 1, "variable (reading)");
		return _scummVars[var];
	}

	if (var & 0x8000)
	{
		//printf("roomvar?\n");
		//while(1);
		//if (_game.heversion >= 80) {
			var &= 0xFFF;
		//	assertRange(0, var, _numRoomVariables - 1, "room variable (reading)");
			return _roomVars[var];

		//}  else {*/
			//var &= 0x7FFF;

			//assertRange(0, var, _numBitVariables - 1, "variable (reading)");
		//	return (_bitVars[var >> 3] & (1 << (var & 7))) ? 1 : 0;
		//}
	}

	if (var & 0x4000) {
		//if (_game.features & GF_FEW_LOCALS) {
		//	var &= 0xF;
		//} else {
			var &= 0xFFF;
		//}

		//if (_game.heversion >= 80)
		//	assertRange(0, var, 25, "local variable (reading)");
		//else
		//	assertRange(0, var, 20, "local variable (reading)");
		return vm.localvar[_currentScript][var];
	}

	printf("Error: Illegal varbits (r)\n");
	return -1;
}

void writeVar(uint var, int value) {
	//printf("writeVar(%d, %d)\n", var, value);

	if (!(var & 0xF000)) {
		//assertRange(0, var, _numVariables - 1, "variable (writing)");

		/*if (VAR_SUBTITLES != 0xFF && var == VAR_SUBTITLES) {
			// Ignore default setting in HE72-74 games
			if (_game.heversion <= 74 && vm.slot[_currentScript].number == 1)
				return;
			assert(value == 0 || value == 1);
			ConfMan.setBool("subtitles", (value != 0));
		}
		if (VAR_NOSUBTITLES != 0xFF && var == VAR_NOSUBTITLES) {
			// Ignore default setting in HE60-71 games
			if (_game.heversion >= 60 && vm.slot[_currentScript].number == 1)
				return;
			assert(value == 0 || value == 1);
			ConfMan.setBool("subtitles", !value);
		}*/

		/*if (var == VAR_CHARINC) {
			// Did the user override the talkspeed manually? Then use that.
			// Otherwise, use the value specified by the game script.
			// Note: To determine whether there was a user override, we only
			// look at the target specific settings, assuming that any global
			// value is likely to be bogus. See also bug #2251765.
			if (ConfMan.hasKey("talkspeed", _targetName)) {
				value = getTalkSpeed();
			} else {
				// Save the new talkspeed value to ConfMan
				setTalkSpeed(value);
			}
		}*/

		_scummVars[var] = value;

		//if ((_varwatch == (int)var) || (_varwatch == 0)) {
		//	if (vm.slot[_currentScript].number < 100)
		//		printf("vars[%d] = %d (via script-%d)\n", var, value, vm.slot[_currentScript].number);
		//	else
		//		printf("vars[%d] = %d (via room-%d-%d)\n", var, value, _currentRoom,
		//					vm.slot[_currentScript].number);
		//}
		return;
	}

	if (var & 0x8000) {
		//if (_game.heversion >= 80) {
			var &= 0xFFF;
		//	assertRange(0, var, _numRoomVariables - 1, "room variable (writing)");
			_roomVars[var] = value;

		//}* else {
			//var &= 0x7FFF;
			//assertRange(0, var, _numBitVariables - 1, "bit variable (writing)");

			//if (value)
			//	_bitVars[var >> 3] |= (1 << (var & 7));
			//else
			//	_bitVars[var >> 3] &= ~(1 << (var & 7));
		//}
		return;
	}

	if (var & 0x4000) {
		//if (_game.features & GF_FEW_LOCALS) {
		//	var &= 0xF;
		//} else {
			var &= 0xFFF;
		//}

		//if (_game.heversion >= 80)
		//	assertRange(0, var, 25, "local variable (writing)");
		//else
		//	assertRange(0, var, 20, "local variable (writing)");

		vm.localvar[_currentScript][var] = value;
		return;
	}

	printf("Error: Illegal varbits (w)\n");
}

void push(int a) {
	_vmStack[_scummStackPos++] = a;
}

int pop() {
	return _vmStack[--_scummStackPos];
}

void stopObjectCode() {
	ScriptSlot *ss;

	ss = &vm.slot[_currentScript];
		if (ss->where != WIO_GLOBAL && ss->where != WIO_LOCAL) {
			if (ss->cutsceneOverride)
				printf("Error: Object %d ending with active cutscene/override (%d)\n", ss->number, ss->cutsceneOverride);
		} else {
			if (ss->cutsceneOverride)
				printf("Error: Script %d ending with active cutscene/override (%d)\n", ss->number, ss->cutsceneOverride);
		}
		ss->number = 0;
		ss->status = ssDead;

	//nukeArrays(_currentScript);
	_currentScript = 0xFF;
} 

void runAllScripts() {
	int i;

	for (i = 0; i < NUM_SCRIPT_SLOT; i++)
		vm.slot[i].didexec = false;

	_currentScript = 0xFF;
	int numCycles = VAR(VAR_NUM_SCRIPT_CYCLES);

	for (int cycle = 1; cycle <= numCycles; cycle++) {
		for (i = 0; i < NUM_SCRIPT_SLOT; i++) {
			if (vm.slot[i].cycle == cycle && vm.slot[i].status == ssRunning && !vm.slot[i].didexec) {
				_currentScript = (byte)i;
				getScriptBaseAddress();
				resetScriptPointer();
				executeScript();
				//free((void*)_scriptOrgPointer);
			}
		}
	}
}

void runExitScript() {
	if (VAR(VAR_EXIT_SCRIPT))
		runScript(VAR(VAR_EXIT_SCRIPT), 0, 0, 0);
	if (RoomResource != NULL) {
		int slot = getScriptSlot();
		vm.slot[slot].status = ssRunning;
		vm.slot[slot].number = 10001;
		vm.slot[slot].where = WIO_ROOM;
		vm.slot[slot].offs = 0;
		vm.slot[slot].freezeResistant = false;
		vm.slot[slot].recursive = false;
		vm.slot[slot].freezeCount = 0;
		vm.slot[slot].delayFrameCount = 0;
		vm.slot[slot].cycle = 1;

		initializeLocals(slot, 0);
		runScriptNested(slot);
	}
	if (VAR(VAR_EXIT_SCRIPT2))
		runScript(VAR(VAR_EXIT_SCRIPT2), 0, 0, 0);
}

void runEntryScript() {
	if (VAR(VAR_ENTRY_SCRIPT))
		runScript(VAR(VAR_ENTRY_SCRIPT), 0, 0, 0);
	if (RoomResource != NULL) {
		int slot = getScriptSlot();
		vm.slot[slot].status = ssRunning;
		vm.slot[slot].number = 10002;
		vm.slot[slot].where = WIO_ROOM;
		vm.slot[slot].offs = 0;
		vm.slot[slot].freezeResistant = false;
		vm.slot[slot].recursive = false;
		vm.slot[slot].freezeCount = 0;
		vm.slot[slot].delayFrameCount = 0;
		vm.slot[slot].cycle = 1;
		initializeLocals(slot, 0);
		runScriptNested(slot);
	}
	if (VAR(VAR_ENTRY_SCRIPT2))
		runScript(VAR(VAR_ENTRY_SCRIPT2), 0, 0, 0);
}

void killScriptsAndResources() {
	ScriptSlot *ss;
	int i;

	ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (ss->where == WIO_ROOM || ss->where == WIO_FLOBJECT) {
			if (ss->cutsceneOverride) {
				printf("Warning: Object %d stopped with active cutscene/override in exit\n", ss->number);
				ss->cutsceneOverride = 0;
			}
			//nukeArrays(i);
			ss->status = ssDead;
		} else if (ss->where == WIO_LOCAL) {
			if (ss->cutsceneOverride) {
				printf("Warning: Script %d stopped with active cutscene/override in exit\n", ss->number);
				ss->cutsceneOverride = 0;
			}
			//nukeArrays(i);
			ss->status = ssDead;
		}
	}
}

void checkAndRunSentenceScript() {
	int i;
	int localParamList[NUM_SCRIPT_LOCAL];
	const ScriptSlot *ss;
	int sentenceScript;

	sentenceScript = VAR(VAR_SENTENCE_SCRIPT);

	memset(localParamList, 0, sizeof(localParamList));
	if (isScriptInUse(sentenceScript)) {
		ss = vm.slot;
		for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++)
			if (ss->number == sentenceScript && ss->status != ssDead && ss->freezeCount == 0)
				return;
	}

	if (!_sentenceNum || _sentence[_sentenceNum - 1].freezeCount)
		return;

	_sentenceNum--;
	SentenceTab &st = _sentence[_sentenceNum];

	if (st.preposition && st.objectB == st.objectA)
		return;

	
	localParamList[0] = st.verb;
	localParamList[1] = st.objectA;
	localParamList[2] = st.objectB;

	_currentScript = 0xFF;
	if (sentenceScript)
		runScript(sentenceScript, 0, 0, localParamList);
}

void decreaseScriptDelay(int amount) {
	ScriptSlot *ss = vm.slot;
	int i;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (ss->status == ssPaused) {
			ss->delay -= amount;
			if (ss->delay < 0) {
				ss->status = ssRunning;
				ss->delay = 0;
			}
		}
	}
}

bool isScriptInUse(int script) {
	int i;
	const ScriptSlot *ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++)
		if (ss->number == script)
			return true;
	return false;
} 

bool isScriptRunning(int script) {
	int i;
	const ScriptSlot *ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++)
		if (ss->number == script && (ss->where == WIO_GLOBAL || ss->where == WIO_LOCAL) && ss->status != ssDead)
			return true;
	return false;
} 

/**
 * Given a pointer to a Scumm string, this function returns the total
 * byte length of the string data in that resource. To do so it has to
 * understand certain special characters embedded into the string. The
 * reason for this function is that sometimes this embedded data
 * contains zero bytes, thus we can't just use strlen.
 */
int resStrLen(const byte *src) {
	int num = 0;
	byte chr;
	if (src == NULL) {
		refreshScriptPointer();
		src = _scriptPointer;
	}
	while ((chr = *src++) != 0) {
		num++;
		/*if (_game.heversion <= 71 && chr == 0xFF) {
			chr = *src++;
			num++;

			if (chr != 1 && chr != 2 && chr != 3 && chr != 8) {
					src += 2;
					num += 2;
			}
		}*/
	}
	return num;
} 

void abortCutscene() {
	const int idx = vm.cutSceneStackPointer;
	//assert(0 <= idx && idx < kMaxCutsceneNum);

	uint32 offs = vm.cutScenePtr[idx];
	if (offs) {
		ScriptSlot *ss = &vm.slot[vm.cutSceneScript[idx]];
		ss->offs = offs;
		ss->status = ssRunning;
		ss->freezeCount = 0;

		if (ss->cutsceneOverride > 0)
			ss->cutsceneOverride--;

		VAR(VAR_OVERRIDE) = 1;
		vm.cutScenePtr[idx] = 0;
	}
}

void beginOverride() {
	const int idx = vm.cutSceneStackPointer;
	//assert(0 <= idx && idx < kMaxCutsceneNum);

	vm.cutScenePtr[idx] = _scriptPointer - _scriptOrgPointer;
	vm.cutSceneScript[idx] = _currentScript;

	// Skip the jump instruction following the override instruction
	// (the jump is responsible for "skipping" cutscenes, and the reason
	// why we record the current script position in vm.cutScenePtr).
	fetchScriptByte();
	fetchScriptWord();

	VAR(VAR_OVERRIDE) = 0;
}

void endOverride() {
	const int idx = vm.cutSceneStackPointer;
	//assert(0 <= idx && idx < kMaxCutsceneNum);

	vm.cutScenePtr[idx] = 0;
	vm.cutSceneScript[idx] = 0;

	VAR(VAR_OVERRIDE) = 0;
}