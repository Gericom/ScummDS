#ifndef __SCRIPT_H__
#define __SCRIPT_H__
#include <stdio.h>

enum
{
	VAR_KEYPRESS = 0,
	VAR_DEBUGMODE = 1,
	VAR_TIMER_NEXT = 2,
	VAR_OVERRIDE = 3,
	VAR_WALKTO_OBJ = 4,
	VAR_RANDOM_NR = 5,

	VAR_GAME_LOADED = 8,
	VAR_EGO = 9,
	VAR_NUM_ACTOR = 10,

	VAR_VIRT_MOUSE_X = 13,
	VAR_VIRT_MOUSE_Y = 14,
	VAR_MOUSE_X = 15,
	VAR_MOUSE_Y = 16,
	VAR_LEFTBTN_HOLD = 17,
	VAR_RIGHTBTN_HOLD = 18,

	VAR_CURSORSTATE = 19,
	VAR_USERPUT = 20,
	VAR_ROOM = 21,
	VAR_ROOM_WIDTH = 22,
	VAR_ROOM_HEIGHT = 23,
	VAR_CAMERA_POS_X = 24,
	VAR_CAMERA_MIN_X = 25,
	VAR_CAMERA_MAX_X = 26,
	VAR_ROOM_RESOURCE = 27,
	VAR_SCROLL_SCRIPT = 28,
	VAR_ENTRY_SCRIPT = 29,
	VAR_ENTRY_SCRIPT2 = 30,
	VAR_EXIT_SCRIPT = 31,
	VAR_EXIT_SCRIPT2 = 32,
	VAR_VERB_SCRIPT = 33,
	VAR_SENTENCE_SCRIPT = 34,
	VAR_INVENTORY_SCRIPT = 35,
	VAR_CUTSCENE_START_SCRIPT = 36,
	VAR_CUTSCENE_END_SCRIPT = 37,

	VAR_RESTART_KEY = 42,
	VAR_PAUSE_KEY = 43,
	VAR_CUTSCENEEXIT_KEY = 44,
	VAR_TALKSTOP_KEY = 45,
	VAR_HAVE_MSG = 46,
	VAR_SUBTITLES = 47,
	VAR_CHARINC = 48,
	VAR_TALK_ACTOR = 49,
	VAR_LAST_SOUND = 50,
	VAR_TALK_CHANNEL = 51,
	VAR_SOUND_CHANNEL = 52,

	VAR_MEMORY_PERFORMANCE = 57,
	VAR_VIDEO_PERFORMANCE = 58,
	VAR_NEW_ROOM = 59,
	VAR_TMR_1 = 60,
	VAR_TMR_2 = 61,
	VAR_TMR_3 = 62,
	VAR_TIMEDATE_HOUR = 63,
	VAR_TIMEDATE_MINUTE = 64,
	VAR_TIMEDATE_DAY = 65,
	VAR_TIMEDATE_MONTH = 66,
	VAR_TIMEDATE_YEAR = 67,

	VAR_NUM_ROOMS = 68,
	VAR_NUM_SCRIPTS = 69,
	VAR_NUM_SOUNDS = 70,
	VAR_NUM_COSTUMES = 71,
	VAR_NUM_IMAGES = 72,
	VAR_NUM_CHARSETS = 73,
	VAR_NUM_GLOBAL_OBJS = 74,
	VAR_MOUSE_STATE = 75,
	VAR_POLYGONS_ONLY = 76,

	VAR_SOUND_ENABLED = 54,
	VAR_NUM_SOUND_CHANNELS = 56,
	VAR_PLATFORM = 78,

	VAR_TIMER = 97,
	VAR_SCRIPT_CYCLE = 103,
	VAR_NUM_SCRIPT_CYCLES = 104
};

enum {
	NUM_SCRIPT_SLOT = 80,
	NUM_SCRIPT_LOCAL = 25
};

/* Script status type (slot.status) */
enum {
	ssDead = 0,
	ssPaused = 1,
	ssRunning = 2
};

struct ScriptSlot {
	uint32 offs;
	int32 delay;
	uint16 number;
	uint16 delayFrameCount;
	bool freezeResistant, recursive;
	bool didexec;
	byte status;
	byte where;
	byte freezeCount;
	byte cutsceneOverride;
	byte cycle;
};

struct NestedScript {
	uint16 number;
	uint8 where;
	uint8 slot;
}; 

enum {
	/**
	* The maximal number of cutscenes that can be active
	* in parallel (i.e. nested).
	*/
	kMaxCutsceneNum = 5,

	/**
	* The maximal 'nesting' level for scripts.
	*/
	kMaxScriptNesting = 15
};

struct VirtualMachineState {
	uint32 cutScenePtr[kMaxCutsceneNum];
	byte cutSceneScript[kMaxCutsceneNum];
	int16 cutSceneData[kMaxCutsceneNum];
	int16 cutSceneScriptIndex;
	byte cutSceneStackPointer;
	ScriptSlot slot[NUM_SCRIPT_SLOT];
	// Why does localvar have space for one extra local variable?
	int32 localvar[NUM_SCRIPT_SLOT][NUM_SCRIPT_LOCAL + 1];

	NestedScript nest[kMaxScriptNesting];
	byte numNestedScripts;
};

enum ArrayType {
		kBitArray = 1,
		kNibbleArray = 2,
		kByteArray = 3,
		kStringArray = 4,
		kIntArray = 5,
		kDwordArray = 6
	};

struct ArrayHeader {
		int32 type;      //0
		int32 dim1start; //4
		int32 dim1end;   //8
		int32 dim2start; //0C
		int32 dim2end;   //10
		byte data[1];    //14
};


extern uint32_t _numGlobalScripts;
extern VirtualMachineState vm;

extern byte _currentScript;

extern int _stringLength;
extern byte _stringBuffer[4096];

extern const byte *_scriptPointer;

extern int16 _varwatch;
extern int32 _roomVars[4096];
extern int32 _scummVars[1024];
//extern byte _bitVars[512]; 

extern uint32_t _numArray;

extern ArrayHeader* _arrays[8192];

//extern uint8_t scriptdata[16384];

//extern void* mallocs[1024];
//extern int mallocsoffset;

#define VAR(x)	_scummVars[x]

void runScript(int script, bool freezeResistant, bool recursive, int *lvarptr, int cycle = 0);
void runObjectScript(int object, int entry, bool freezeResistant, bool recursive, int *vars, int slot = -1, int cycle = 0);
void initializeLocals(int slot, int *vars);
int getVerbEntrypoint(int obj, int entry);
void stopScript(int script);
void stopObjectScript(int script);
int getScriptSlot();
void runScriptNested(int script);
void updateScriptPtr();
void getScriptBaseAddress();
void resetScriptPointer();
void refreshScriptPointer();
void executeScript();
inline void executeOpcode(byte i);
byte fetchScriptByte();
uint16 fetchScriptWord();
int16 fetchScriptWordSigned();
uint fetchScriptDWord();
int fetchScriptDWordSigned();
int readVar(uint var);
void writeVar(uint var, int value);
void push(int a);
int pop();
void stopObjectCode();
void runAllScripts();
void runExitScript();
void runEntryScript();
void killScriptsAndResources();
void checkAndRunSentenceScript();
void runInputScript(int clickArea, int val, int mode);
void decreaseScriptDelay(int amount);
bool isScriptInUse(int script);
bool isScriptRunning(int script);
int resStrLen(const byte *src);
void abortCutscene();
void beginOverride();
void endOverride();

#endif