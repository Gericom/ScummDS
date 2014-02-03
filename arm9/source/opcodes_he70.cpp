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

void _0x74_StartSound()
{
	int var, value;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 9:
		_heSndFlags |= 4;
		break;
	case 23:
		value = pop();
		var = pop();
		_heSndSoundId = pop();
		//((SoundHE *)_sound)->setSoundVar(_heSndSoundId, var, value);
		break;
	case 25:
		value = pop();
		_heSndSoundId = pop();
		addSoundToQueue(_heSndSoundId, 0, 0, 8);
	case 56:
		_heSndFlags |= 16;
		break;
	case 164:
		_heSndFlags |= 2;
		break;
	case 222:
		// WORKAROUND: For errors in room script 240 (room 4) of maze
		break;
	case 224:
		_heSndSoundFreq = pop();
		break;
	case 230:
		_heSndChannel = pop();
		break;
	case 231:
		_heSndOffset = pop();
		break;
	case 232:
		_heSndSoundId = pop();
		_heSndOffset = 0;
		_heSndSoundFreq = 11025;
		_heSndChannel = VAR(VAR_SOUND_CHANNEL);
		break;
	case 245:
		_heSndFlags |= 1;
		break;
	case 255:
		addSoundToQueue(_heSndSoundId, _heSndOffset, _heSndChannel, _heSndFlags);
		_heSndFlags = 0;
		break;

	default:
		printf("Error: o70_startSound invalid case %d\n", subOp);
	} 
}

void _0x8C_GetActorRoom()
{
	int act = pop();

	if (act == 0) {
		// This case occurs at the very least in COMI. That's because in COMI's script 28,
		// there is a check which looks as follows:
		//   if (((VAR_TALK_ACTOR != 0) && (VAR_HAVE_MSG == 1)) &&
		//        (getActorRoom(VAR_TALK_ACTOR) == VAR_ROOM))
		// Due to the way this is represented in bytecode, the engine cannot
		// short circuit. Hence, even though this would be perfectly fine code
		// in C/C++, here it can (and does) lead to getActorRoom(0) being
		// invoked. We silently ignore this.
		push(0);
		return;
	}

	if (act == 255) {
		// This case also occurs in COMI...
		push(0);
		return;
	}

	Actor *a = &_actors[act];
	push(a->_room); 
}

void _0x9B_ResourceRoutines()
{
	int objidx, resid;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 100:		// SO_LOAD_SCRIPT
		resid = pop();
		printf("SO_LOAD_SCRIPT: %d\n", resid);
		//ensureResourceLoaded(rtScript, resid);
		break;
	case 101:		// SO_LOAD_SOUND
		resid = pop();
		printf("SO_LOAD_SOUND: %d\n", resid);
		//ensureResourceLoaded(rtSound, resid);
		break;
	case 102:		// SO_LOAD_COSTUME
		resid = pop();
		printf("SO_LOAD_COSTUME: %d\n", resid);
		//ensureResourceLoaded(rtCostume, resid);
		break;
	case 103:		// SO_LOAD_ROOM
		resid = pop();
		printf("SO_LOAD_ROOM: %d\n", resid);
		//ensureResourceLoaded(rtRoomImage, resid);
		//ensureResourceLoaded(rtRoom, resid);
		break;
	case 104:		// SO_NUKE_SCRIPT
		resid = pop();
		printf("SO_NUKE_SCRIPT: %d\n", resid);
		//_res->nukeResource(rtScript, resid);
		break;
	case 105:		// SO_NUKE_SOUND
		resid = pop();
		printf("SO_NUKE_SOUND: %d\n", resid);
		//_res->nukeResource(rtSound, resid);
		break;
	case 106:		// SO_NUKE_COSTUME
		resid = pop();
		printf("SO_NUKE_COSTUME: %d\n", resid);
		//_res->nukeResource(rtCostume, resid);
		break;
	case 107:		// SO_NUKE_ROOM
		resid = pop();
		printf("SO_NUKE_ROOM: %d\n", resid);
		//_res->nukeResource(rtRoom, resid);
		//_res->nukeResource(rtRoomImage, resid);
		break;
	case 108:		// SO_LOCK_SCRIPT
		resid = pop();
		printf("SO_LOCK_SCRIPT: %d\n", resid);
		if (resid >= _numGlobalScripts)
			break;
		//_res->lock(rtScript, resid);
		break;
	case 109:		// SO_LOCK_SOUND
		resid = pop();
		printf("SO_LOCK_SOUND: %d\n", resid);
		//_res->lock(rtSound, resid);
		break;
	case 110:		// SO_LOCK_COSTUME
		resid = pop();
		printf("SO_LOCK_COSTUME: %d\n", resid);
		//_res->lock(rtCostume, resid);
		break;
	case 111:		// SO_LOCK_ROOM
		resid = pop();
		printf("SO_LOCK_ROOM: %d\n", resid);
		//if (_game.heversion <= 71 && resid > 0x7F)
		//	resid = _resourceMapper[resid & 0x7F];
		//_res->lock(rtRoom, resid);
		//_res->lock(rtRoomImage, resid);
		break;
	case 112:		// SO_UNLOCK_SCRIPT
		resid = pop();
		printf("SO_UNLOCK_SCRIPT: %d\n", resid);
		if (resid >= _numGlobalScripts)
			break;
		//_res->unlock(rtScript, resid);
		break;
	case 113:		// SO_UNLOCK_SOUND
		resid = pop();
		printf("SO_UNLOCK_SOUND: %d\n", resid);
		//_res->unlock(rtSound, resid);
		break;
	case 114:		// SO_UNLOCK_COSTUME
		resid = pop();
		printf("SO_UNLOCK_COSTUME: %d\n", resid);
		//_res->unlock(rtCostume, resid);
		break;
	case 115:		// SO_UNLOCK_ROOM
		resid = pop();
		printf("SO_UNLOCK_ROOM: %d\n", resid);
		//if (_game.heversion <= 71 && resid > 0x7F)
		//	resid = _resourceMapper[resid & 0x7F];
		//_res->unlock(rtRoom, resid);
		//_res->unlock(rtRoomImage, resid);
		break;
	case 116:
		printf("Clear Heap\n");
		// TODO: Clear Heap
		break;
	case 117:		// SO_LOAD_CHARSET
		resid = pop();
		printf("SO_LOAD_CHARSET: %d\n", resid);
		//loadCharset(resid);
		break;
	case 118:		// SO_NUKE_CHARSET
		resid = pop();
		printf("SO_NUKE_CHARSET: %d\n", resid);
		//nukeCharset(resid);
		break;
	case 119:		// SO_LOAD_OBJECT
		{
			int obj = pop();
			printf("SO_LOAD_OBJECT: %d\n", obj);
			//int room = getObjectRoom(obj);
			//loadFlObject(obj, room);
			break;
		}
	case 120:
		resid = pop();
		if (resid >= _numGlobalScripts)
			break;
		break;
	case 121:
		resid = pop();
		break;
	case 122:
		resid = pop();
		break;
	case 123:
		resid = pop();
		break;
	case 159:
		resid = pop();
		//_res->unlock(rtImage, resid);
		break;
	case 192:
		resid = pop();
		//_res->nukeResource(rtImage, resid);
		break;
	case 201:
		resid = pop();
		//ensureResourceLoaded(rtImage, resid);
		break;
	case 202:
		resid = pop();
		//_res->lock(rtImage, resid);
		break;
	case 203:
		resid = pop();
		break;
	case 233:
		resid = pop();
		//objidx = getObjectIndex(resid);
		//if (objidx == -1)
		//	break;
		//_res->lock(rtFlObject, _objs[objidx].fl_object_index);
		break;
	case 235:
		resid = pop();
		break;
	case 239:
		// Used in airport
		break;
	default:
		printf("Error: o70_resourceRoutines: default case %d\n", subOp);
	}
}

void _0xEE_GetStringLength()
{
	int id, len;
	byte *addr;

	id = pop();

	addr = (byte*)_arrays[id & ~0x33539000]->data;
	if (!addr)
		printf("Error: o70_getStringLen: Reference to zeroed array pointer (%d)\n", id);

	len = resStrLen((byte*)_arrays[id & ~0x33539000]->data);
	push(len); 
}

void _0xF2_IsResourceLoaded() {
	// Reports percentage of resource loaded by queue
	//ResType type;

	byte subOp = fetchScriptByte();
	int idx = pop();

	/*switch (subOp) {
	case 18:
		type = rtImage;
		break;
	case 226:
		type = rtRoom;
		break;
	case 227:
		type = rtCostume;
		break;
	case 228:
		type = rtSound;
		break;
	case 229:
		type = rtScript;
		break;
	default:
		printf("Error: o70_isResourceLoaded: default case %d\n", subOp);
	}
	printf("o70_isResourceLoaded(%d,%d)\n", type, idx);*/

	push(100);
} 