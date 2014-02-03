#ifndef __ACTOR_H__
#define __ACTOR_H__
#include <stdio.h>
#include <room.h>

enum {
	NUM_SENTENCE = 6,
	NUM_SHADOW_PALETTE = 8
};

struct SentenceTab {
	byte verb;
	byte preposition;
	uint16 objectA;
	uint16 objectB;
	uint8 freezeCount;
};


struct StringSlot {
	int16 xpos;
	int16 ypos;
	int16 right;
	int16 height;
	byte color;
	byte charset;
	bool center;
	bool overhead;
	bool no_talk_anim;
	bool wrapping;
}; 

struct StringTab : StringSlot {
	// The 'default' values for this string slot. This is used so that the
	// string slot can temporarily be set to different values, and then be
	// easily reset to a previously set default.
	StringSlot _default;

	void saveDefault() {
		StringSlot &s = *this;
		_default = s;
	}

	void loadDefault() {
		StringSlot &s = *this;
		s = _default;
	}
}; 

enum MoveFlags {
	MF_NEW_LEG = 1,
	MF_IN_LEG = 2,
	MF_TURN = 4,
	MF_LAST_LEG = 8,
	MF_FROZEN = 0x80
};

struct AKOS_t;

struct CostumeData {
	byte active[16];
	uint16 animCounter;
	byte soundCounter;
	byte soundPos;
	uint16 stopped;
	uint16 curpos[16];
	uint16 start[16];
	uint16 end[16];
	uint16 frame[16];

	uint16 xmove[16];
	uint16 ymove[16];

	uint16 lastDx[16];
	uint16 lastDy[16];

	/* HE specific */
	uint16 heJumpOffsetTable[16];
	uint16 heJumpCountTable[16];
	uint32 heCondMaskTable[16];

	AKOS_t* AKOS;

	void reset() {
		stopped = 0;
		for (int i = 0; i < 16; i++) {
			active[i] = 0;
			curpos[i] = start[i] = end[i] = frame[i] = xmove[i] = ymove[i] = lastDx[i] = lastDy[i] = 0xFFFF;
		}
	}
};

struct Actor
{
	int _top, _bottom;
	uint _width;
	byte _number;
	uint16 _costume;
	byte _room;

	byte _talkColor;
	int _talkFrequency;
	byte _talkPan;
	byte _talkVolume;
	uint16 _boxscale;
	byte _scalex, _scaley;
	byte _charset;
	byte _moving;
	bool _ignoreBoxes;
	byte _forceClip;

	byte _initFrame;
	byte _walkFrame;
	byte _standFrame;
	byte _talkStartFrame;
	byte _talkStopFrame;

	bool _needRedraw, _needBgReset, _visible;
	byte _shadowMode;
	bool _flip;
	byte _frame;
	byte _walkbox;
	int16 _talkPosX, _talkPosY;
	uint16 _talkScript, _walkScript;
	bool _ignoreTurns;
	bool _drawToBackBuf;
	int32 _layer;
	uint16 _sound[32];
	CostumeData _cost;

	/* HE specific */
	int _heOffsX, _heOffsY;
	bool _heSkipLimbs;
	uint32 _heCondMask;
	uint32 _hePaletteNum;
	uint32 _heXmapNum;

	uint16 _palette[256];
	int _elevation;
	uint16 _facing;
	uint16 _targetFacing;
	uint _speedx, _speedy;
	byte _animProgress, _animSpeed;
	bool _costumeNeedsInit;
	//ActorWalkData _walkdata;
	int16 _animVariable[27]; 

	/** This rect is used to clip actor drawing. */
	//Common::Rect _clipOverride;

	bool _heNoTalkAnimation;
	bool _heTalking;
	byte _heFlags;

	//AuxBlock _auxBlock;

	struct {
		int16 posX;
		int16 posY;
		int16 color;
		byte sentence[128];
	} _heTalkQueue[16]; 

	int16 x;
	int16 y;

	int _left, _right;
 
};

extern byte _curActor;
extern byte _numActors;
extern Actor _actors[62];


extern byte _actorToPrintStrFor;
extern int _sentenceNum;
extern SentenceTab _sentence[NUM_SENTENCE];
extern StringTab _string[6];
extern byte _haveMsg;
extern int16 _talkDelay;

extern bool _egoPositioned;

int newDirToOldDir(int dir);
int oldDirToNewDir(int dir);

void initActor(Actor* a, int mode);
void stopActorMoving(Actor* a);
bool isInCurrentRoom(Actor* a);
void putActor(Actor* a);
void putActor(Actor* a, int room);
void putActor(Actor* a, int x, int y);
void putActor(Actor* a, int dstX, int dstY, int newRoom);
int getRoom(Actor* a);
int getFacing(Actor* a);
void setFacing(Actor* a, int newFacing);
int getAnimVar(Actor* a, byte var);
void setAnimVar(Actor* a, byte var, int value);
void setAnimSpeed(Actor* a, byte newAnimSpeed);
int getAnimSpeed(Actor* a);
int getAnimProgress(Actor* a);
int getElevation(Actor* a);
void setElevation(Actor* a, int newElevation);
void setPalette(Actor* a, int idx, int val);
void setScale(Actor* a, int sx, int sy);
void animateActor(Actor* a, int anim);
void startAnimActor(Actor* a, int f);
void setActorCostume(Actor* a, int c);
void setHEFlag(Actor* a, int bit, int set);
void setUserCondition(Actor* a, int slot, int set);
bool isUserConditionSet(Actor* a, int slot);
void setTalkCondition(Actor* a, int slot);
bool isTalkConditionSet(Actor* a, int slot);
int getTalkingActor();
void setTalkingActor(int i);
void actorTalk(const byte *msg);
void stopTalk();
int getActorFromPos(int x, int y);
void hideActor(Actor* a);
void showActor(Actor* a);
void showActors();
void drawActorToBackBuf(Actor* a, int x, int y);
void remapActorPaletteColor(Actor* a, int color, int new_color);


#endif
