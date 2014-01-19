#include <nds.h>

#include <actor.h>
#include <akos.h>
#include <script.h>
#include <sounds.h>
#include <objects.h>


byte _curActor;
byte _numActors = 62;
Actor _actors[62];

byte _actorToPrintStrFor;
int _sentenceNum;
SentenceTab _sentence[NUM_SENTENCE];
StringTab _string[6];
byte _haveMsg = 0;
int16 _talkDelay;

bool _egoPositioned;


/**
* Convert an old style direction to a new style one (angle),
*/
int newDirToOldDir(int dir) {
	if (dir >= 71 && dir <= 109)
		return 1;
	if (dir >= 109 && dir <= 251)
		return 2;
	if (dir >= 251 && dir <= 289)
		return 0;
	return 3;
}

/**
* Convert an new style (angle) direction to an old style one.
*/
int oldDirToNewDir(int dir) {
	const int new_dir_table[4] = { 270, 90, 180, 0 };
	return new_dir_table[dir];
}

void initActor(Actor* a, int mode)
{
	a->_number = _curActor;
	if (mode == -1) {
		a->_top = a->_bottom = a->_left = a->_right = 0;
		a->_needRedraw = false;
		a->_needBgReset = false;
		a->_costumeNeedsInit = false;
		a->_visible = false;
		a->_flip = false;
		a->_speedx = 8;
		a->_speedy = 2;
		a->_frame = 0;
		a->_walkbox = 0;
		a->_animProgress = 0;
		a->_drawToBackBuf = false;
		//memset(_animVariable, 0, sizeof(_animVariable));
		//memset(_palette, 0, sizeof(_palette));
		//memset(_sound, 0, sizeof(_sound));
		//memset(&_cost, 0, sizeof(CostumeData));
		//memset(&_walkdata, 0, sizeof(ActorWalkData));
		//_walkdata.point3.x = 32000;
		a->_walkScript = 0;
	}

	if (mode == 1 || mode == -1) {
		a->_costume = 0;
		a->_room = 0;
		a->x = 0;
		a->y = 0;
		a->_facing = 180;
	} else if (mode == 2) {
		a->_facing = 180;
	}
	a->_elevation = 0;
	a->_width = 24;
	a->_talkColor = 15;
	a->_talkPosX = 0;
	a->_talkPosY = -80;
	a->_boxscale = a->_scaley = a->_scalex = 0xFF;
	a->_charset = 0;
	//memset(_sound, 0, sizeof(_sound));
	a->_targetFacing = a->_facing;

	a->_shadowMode = 0;
	a->_layer = 0;

	stopActorMoving(a);

	//setActorWalkSpeed(8, 2);

	a->_animSpeed = 0;
	a->_animProgress = 0;

	a->_ignoreBoxes = false;
	a->_forceClip = 0;
	a->_ignoreTurns = false;

	a->_talkFrequency = 256;
	a->_talkPan = 64;
	a->_talkVolume = 127;

	a->_initFrame = 1;
	a->_walkFrame = 2;
	a->_standFrame = 3;
	a->_talkStartFrame = 4;
	a->_talkStopFrame = 5;

	a->_walkScript = 0;
	a->_talkScript = 0;

	_classData[a->_number] = 0; 

	if (mode == -1) {
		a->_heOffsX = a->_heOffsY = 0;
		a->_heSkipLimbs = false;
		//memset(_heTalkQueue, 0, sizeof(_heTalkQueue));
	}

	if (mode == 1 || mode == -1) {
		a->_heCondMask = 1;
		a->_heNoTalkAnimation = 0;
		a->_heSkipLimbs = false;
	} else if (mode == 2) {
		a->_heCondMask = 1;
		a->_heSkipLimbs = false;
	}

	a->_heXmapNum = 0;
	a->_hePaletteNum = 0;
	a->_heFlags = 0;
	a->_heTalking = false;

	a->_flip = 0;

	//a->_clipOverride = ((ScummEngine_v60he *)_vm)->_actorClipOverride;

	//_auxBlock.reset(); 
}

void stopActorMoving(Actor* a) {
	if (a->_walkScript)
		stopScript(a->_walkScript);

	a->_moving = 0;
}

bool isInCurrentRoom(Actor* a) {
	return a->_room == _currentRoom;
} 

void putActor(Actor* a) {
	putActor(a, a->x, a->y, a->_room);
}

void putActor(Actor* a, int room) {
	putActor(a, a->x, a->y, room);
}

void putActor(Actor* a, int x, int y) {
	putActor(a, x, y, a->_room);
} 

void putActor(Actor* a, int dstX, int dstY, int newRoom) {
	if (a->_visible && _currentRoom != newRoom && getTalkingActor() == a->_number) {
		stopTalk();
	}

	a->x = dstX;
	a->y = dstY;
	a->_room = newRoom;
	a->_needRedraw = true;

	if (VAR(VAR_EGO) == a->_number) {
		_egoPositioned = true;
	}

	if (a->_visible) {
		if (isInCurrentRoom(a)) {
			if (a->_moving) {
				stopActorMoving(a);
				startAnimActor(a, a->_standFrame);
			}
			//adjustActorPos();
		} else {
			//((ScummEngine_v71he *)_vm)->queueAuxBlock((ActorHE *)this);
			hideActor(a);
		}
	} else {
		if (isInCurrentRoom(a))
			showActor(a);
	}
}

int getRoom(Actor* a) {
	return a->_room;
}

int getFacing(Actor* a) {
	return a->_facing;
}

void setFacing(Actor* a, int newFacing) {
	a->_facing = newFacing;
}

int getAnimVar(Actor* a, byte var) {
	return a->_animVariable[var];
}

void setAnimVar(Actor* a, byte var, int value) {
	a->_animVariable[var] = value;
}

void setAnimSpeed(Actor* a, byte newAnimSpeed) {
	a->_animSpeed = newAnimSpeed;
	a->_animProgress = 0;
}

int getAnimSpeed(Actor* a) {
	return a->_animSpeed;
}

int getAnimProgress(Actor* a) {
	return a->_animProgress;
}

int getElevation(Actor* a) {
	return a->_elevation;
}

void setElevation(Actor* a, int newElevation) {
	if (a->_elevation != newElevation) {
		a->_elevation = newElevation;
		a->_needRedraw = true;
	}
}

void setPalette(Actor* a, int idx, int val) {
	a->_palette[idx] = val;
	a->_needRedraw = true;
}

void setScale(Actor* a, int sx, int sy) {
	if (sx != -1)
		a->_scalex = sx;
	if (sy != -1)
		a->_scaley = sy;
	a->_needRedraw = true;
}


void animateActor(Actor* a, int anim) {
	int cmd, dir;

	cmd = anim / 4;
	dir = oldDirToNewDir(anim % 4);

	// Convert into old cmd code
	cmd = 0x3F - cmd + 2;


	switch (cmd) {
	case 2:				// stop walking
		startAnimActor(a, a->_standFrame);
		stopActorMoving(a);
		break;
	case 3:				// change direction immediatly
		a->_moving &= ~MF_TURN;
		//setDirection(a, dir);
		break;
	case 4:				// turn to new direction
		//turnToDirection(a, dir);
		break;
	case 64:
	default:
		startAnimActor(a, anim);
		break;
	}
} 

void startAnimActor(Actor* a, int f) {
		switch (f) {
		case 0x38:
			f = a->_initFrame;
			break;
		case 0x39:
			f = a->_walkFrame;
			break;
		case 0x3A:
			f = a->_standFrame;
			break;
		case 0x3B:
			f = a->_talkStartFrame;
			break;
		case 0x3C:
			f = a->_talkStopFrame;
			break;
		}

		//assert(f != 0x3E);

		if (isInCurrentRoom(a) && a->_costume != 0) {
			a->_animProgress = 0;
			a->_needRedraw = true;
			a->_cost.animCounter = 0;
			if (f == a->_initFrame) {
				a->_cost.reset();
				//((ActorHE *)this)->_auxBlock.reset();
			}
			costumeDecodeData(a, f, (uint) - 1);
			a->_frame = f;
		}
}

void setActorCostume(Actor* a, int c) {
	if (c == -1  || c == -2) {
		a->_heSkipLimbs = (c == -1);
		a->_needRedraw = true;
		return;
	}

	//((ScummEngine_v71he *)_vm)->queueAuxBlock(this);
	//_auxBlock.reset();
	if (a->_visible) {
		a->_needRedraw = true;
	}

	int i;

	a->_costumeNeedsInit = true;

	//memset(_animVariable, 0, sizeof(_animVariable));

	a->_costume = c;
	a->_cost.reset();

	if (a->_visible) {
		if (a->_costume) {
			//_vm->ensureResourceLoaded(rtCostume, _costume);
		}
		startAnimActor(a, a->_initFrame);
	}




	for (i = 0; i < 256; i++)
		a->_palette[i] = 0xFF;


	//if (_vm->_game.heversion >= 71 && _vm->getTalkingActor() == _number) {
	//	if (_vm->_game.heversion <= 95 || (_vm->_game.heversion >= 98 && _vm->VAR(_vm->VAR_SKIP_RESET_TALK_ACTOR) == 0)) {
	//		_vm->setTalkingActor(0);
	//	}
	//}
} 

void setHEFlag(Actor* a, int bit, int set)
{
	// Note that condition is inverted
	if (!set) {
		a->_heFlags |= bit;
	} else {
		a->_heFlags &= ~bit;
	}
}

void setUserCondition(Actor* a, int slot, int set)
{
	const int condMaskCode = /*(_vm->_game.heversion >= 85) ? */0x1FFF;// : 0x3FF;
	if (set == 0) {
		a->_heCondMask &= ~(1 << (slot + 0xF));
	} else {
		a->_heCondMask |= 1 << (slot + 0xF);
	}
	if (a->_heCondMask & condMaskCode) {
		a->_heCondMask &= ~1;
	} else {
		a->_heCondMask |= 1;
	}
}

bool isUserConditionSet(Actor* a, int slot)
{
	return (a->_heCondMask & (1 << (slot + 0xF))) != 0;
}

void setTalkCondition(Actor* a, int slot)
{
	const int condMaskCode = /*(_vm->_game.heversion >= 85) ?*/ 0x1FFF;// : 0x3FF;
	a->_heCondMask = (a->_heCondMask & ~condMaskCode) | 1;
	if (slot != 1) {
		a->_heCondMask |= 1 << (slot - 1);
		if (a->_heCondMask & condMaskCode) {
			a->_heCondMask &= ~1;
		} else {
			a->_heCondMask |= 1;
		}
	}
}

bool isTalkConditionSet(Actor* a, int slot)
{
	return (a->_heCondMask & (1 << (slot - 1))) != 0;
}

int getTalkingActor()
{
	return VAR(VAR_TALK_ACTOR);
}

void setTalkingActor(int i)
{
	if (i == 255) {
		//_system->clearFocusRectangle();
	} else {
		// Work out the screen co-ordinates of the actor
		//int x = _actors[i]->x - (camera._cur.x - (_screenWidth >> 1));
		//int y = _actors[i]->_top - (camera._cur.y - (_screenHeight >> 1));

		// Set the focus area to the calculated position
		// TODO: Make the size adjust depending on what it's focusing on.
		//_system->setFocusRectangle(Common::Rect::center(x, y, 192, 128));
	}

	VAR(VAR_TALK_ACTOR) = i;
} 

void actorTalk(const byte *msg) {
	Actor *a;

	printf("Actor: %s\n", (char*)msg);

	//convertMessageToString(msg, _charsetBuffer, sizeof(_charsetBuffer));

	if (_actorToPrintStrFor == 0xFF) {
		//if (!_keepText) {
			stopTalk();
		//}
		setTalkingActor(0xFF);
	} else {
		int oldact;

		a = &_actors[_actorToPrintStrFor];// derefActor(_actorToPrintStrFor, "actorTalk");
		if (!isInCurrentRoom(a)) {
			oldact = 0xFF;
		} else {
			//if (!_keepText) {
				stopTalk();
			//}
			setTalkingActor(a->_number);
			a->_heTalking = true;
			if (!_string[0].no_talk_anim) {
				//runActorTalkScript(a, a->_talkStartFrame);
				//_useTalkAnims = true;
			}
			oldact = getTalkingActor();
		}
		if (oldact >= 0x80)
			return;
	}

	if (getTalkingActor() > 0x7F) ;
	//	_charsetColor = (byte)_string[0].color;
	//_charsetBufPos = 0;
	//_talkDelay = 0;
	_haveMsg = 0xFF;
	VAR(VAR_HAVE_MSG) = 0xFF;
	//_haveActorSpeechMsg = true;

	char* str = (char*)msg;
	str += 2;

	int i = 0;
	char c = *str++;
	char value[32]; 
	while (c != 44)
	{
		value[i] = c;
		c = *str++;
		i++;
	}
	value[i] = 0;
	int talk_sound_a = atoi(value); 
	//printf("sound: %d\n", talk_sound_a);

	addSoundToQueue2(0x7FFF, talk_sound_a, 2, 8);

	//CHARSET_1();
} 

void stopTalk()
{
	int act;

	stopSound(-1);//_sound->stopTalkSound();

	_haveMsg = 0;
	//_talkDelay = 0;

	act = getTalkingActor();
	if (act && act < 0x80) {
		Actor *a = &_actors[act];//derefActor(act, "stopTalk");
		/*if ((_game.version >= 7 && !_string[0].no_talk_anim) ||
			(_game.version <= 6 && a->isInCurrentRoom() && _useTalkAnims)) {
			a->runActorTalkScript(a->_talkStopFrame);
			_useTalkAnims = false;
		}
		if (_game.version <= 7 && _game.heversion == 0)
			setTalkingActor(0xFF);
		if (_game.heversion != 0)*/
			a->_heTalking = false;
	}

	setTalkingActor(0);

	//_keepText = false;
	//restoreCharsetBg();
}

int getActorFromPos(int x, int y) {
	int curActor, i;

	//if (!testGfxAnyUsageBits(x / 8))
	//	return 0;

	curActor = 0;
	for (i = 1; i < _numActors; i++) {
		//if (/*testGfxUsageBit(x / 8, i) && !getClass(i, kObjectClassUntouchable)
		//	&&*/ y >= _actors[i]._top && y <= _actors[i]._bottom
		//	&& (_actors[i].y > _actors[curActor].y || curActor == 0))
		//		curActor = i;
		if(_actors[i]._visible && y >= _actors[i]._top && y <= _actors[i]._bottom && x >= _actors[i]._left && x <= _actors[i]._right) curActor = i;
	}

	return curActor;
}

void hideActor(Actor* a) {
	if (!a->_visible)
		return;

	if (a->_moving) {
		stopActorMoving(a);
		startAnimActor(a, a->_standFrame);
	}
	a->_visible = false;
	a->_cost.soundCounter = 0;
	a->_cost.soundPos = 0;
	a->_needRedraw = false;
	a->_needBgReset = true;
	//a->_auxBlock.reset();
}

void showActor(Actor* a) {
	if (_currentRoom == 0 || a->_visible)
		return;

	//adjustActorPos();

	//_vm->ensureResourceLoaded(rtCostume, _costume);

	if (a->_costumeNeedsInit) {
		startAnimActor(a, a->_initFrame);
		a->_costumeNeedsInit = false;
	}

	stopActorMoving(a);
	a->_visible = true;
	a->_needRedraw = true;
}

void showActors() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (isInCurrentRoom(&_actors[i]))
			showActor(&_actors[i]);
	}
} 

void drawActorToBackBuf(Actor* a, int x, int y)
{
	//int curTop = a->_top;
	//int curBottom = a->_bottom;

	a->x = x;
	a->y = y;

	a->_drawToBackBuf = true;
	a->_needRedraw = true;
	//drawActorCostume(a);

	a->_drawToBackBuf = false;
	a->_needRedraw = true;
	//drawActorCostume(a);
	a->_needRedraw = false;

	//if (a->_top > curTop)
	//	a->_top = curTop;
	//if (a->_bottom < curBottom)
	//	a->_bottom = curBottom;
}