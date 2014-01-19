#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <ioutil.h>
#include <HE0.h>
#include <HE1.h>
#include <script.h>
#include <scumm.h>
#include <actor.h>
#include <room.h>
#include <graphics.h>
#include <render.h>
#include <resources.h>

LFLF_t Room;
LFLF_t* RoomResource = NULL;
uint32_t _currentRoom;
uint32_t _roomResource;

/**
 * Start a 'scene' by loading the specified room with the given main actor.
 * The actor is placed next to the object indicated by objectNr.
 */
void startScene(int room, Actor* a, int objectNr)
{
	int i, where;

	printf("Loading room %d\n", room);

	stopTalk();

	//fadeOut(_switchRoomEffect2);
	//_newEffect = _switchRoomEffect;

	ScriptSlot *ss = &vm.slot[_currentScript];

	if (_currentScript != 0xFF) {
		if (ss->where == WIO_ROOM || ss->where == WIO_FLOBJECT) {
			if (ss->cutsceneOverride)
				printf("Error: Object %d stopped with active cutscene/override in exit\n", ss->number);

			//nukeArrays(_currentScript);
			_currentScript = 0xFF;
		} else if (ss->where == WIO_LOCAL) {
			if (ss->cutsceneOverride)
				printf("Error: Script %d stopped with active cutscene/override in exit\n", ss->number);

			//nukeArrays(_currentScript);
			_currentScript = 0xFF;
		}
	}

	VAR(VAR_NEW_ROOM) = room;

	runExitScript();

	killScriptsAndResources();


	InitFrameBuffers();

	// For HE80+ games
	for (i = 0; i < 4096; i++)
		_roomVars[i] = 0;

	//nukeArrays(0xFF);

	for (i = 1; i < _numActors; i++) {
		hideActor(&_actors[i]);
	}

		/*for (i = 0; i < 256; i++) {
			_roomPalette[i] = i;
			if (_shadowPalette)
				_shadowPalette[i] = i;
		}*/

	VAR(VAR_ROOM) = room;
	//_fullRedraw = true;

	//_res->increaseResourceCounters();

	_currentRoom = room;
	VAR(VAR_ROOM) = room;

	//if (room >= 0x80 && _game.version < 7 && _game.heversion <= 71)
	//	_roomResource = _resourceMapper[room & 0x7F];
	//else
	_roomResource = room;

	VAR(VAR_ROOM_RESOURCE) = _roomResource;

	//if (room != 0)
	//	ensureResourceLoaded(rtRoom, room);

	//clearRoomObjects();

	if(RoomResource != NULL) freeLFLF(RoomResource);

	if (_currentRoom == 0) {
		RoomResource = NULL;
		//_ENCD_offs = _EXCD_offs = 0;
		//_numObjectsInRoom = 0;
		return;
	}
	else 
	{
		readRoom(HE1_File, &Room, getLFLFOffset(HE0_File, &HE0_Data, room));
		RoomResource = &Room;
		ConvertRoomBackground(HE1_File, RoomResource);
	}

	//setupRoomSubBlocks();
	//resetRoomSubBlocks();

	//initBGBuffers(_roomHeight);

	//resetRoomObjects();

	VAR(VAR_ROOM_WIDTH) = 640;//_roomWidth;
	VAR(VAR_ROOM_HEIGHT) = 480;//_roomHeight;


	/*if (VAR_CAMERA_MIN_X != 0xFF)
		VAR(VAR_CAMERA_MIN_X) = _screenWidth / 2;
	if (VAR_CAMERA_MAX_X != 0xFF)
		VAR(VAR_CAMERA_MAX_X) = _roomWidth - (_screenWidth / 2);

		camera._mode = kNormalCameraMode;
		if (_game.version > 2)
			camera._cur.x = camera._dest.x = _screenWidth / 2;
		camera._cur.y = camera._dest.y = _screenHeight / 2;*/

	if (_roomResource == 0)
		return;

	//memset(gfxUsageBits, 0, sizeof(gfxUsageBits));

	if (a) {
		//where = whereIsObject(objectNr);
		//if (where != WIO_ROOM && where != WIO_FLOBJECT)
		//	printf("Error: startScene: Object %d is not in room %d\n", objectNr,
		//			_currentRoom);
		/*int x, y, dir;
		getObjectXYPos(objectNr, x, y, dir);
		a->putActor(x, y, _currentRoom);
		a->setDirection(dir + 180);
		a->stopActorMoving();
		*/
	}

	showActors();

	_egoPositioned = false;

	runEntryScript();
	
		/*if (a && !_egoPositioned) {
			int x, y;
			getObjectXYPos(objectNr, x, y);
			a->putActor(x, y, _currentRoom);
			a->_moving = 0;
		}*/

	//_doEffect = true;

}

void* getExitScript()
{
	fseek(HE1_File, RoomResource->RMDA->EXCD - 4, SEEK_SET);
	uint32_t length;
	readU32LE(HE1_File, &length);
	length = SWAP_CONSTANT_32(length) - 8;
	void* data = malloc(length);
	readBytes(HE1_File, (uint8_t*)data, length);
	return data;
}

void* getEntryScript()
{
	fseek(HE1_File, RoomResource->RMDA->ENCD - 4, SEEK_SET);
	uint32_t length;
	readU32LE(HE1_File, &length);
	length = SWAP_CONSTANT_32(length) - 8;
	void* data = malloc(length);
	readBytes(HE1_File, (uint8_t*)data, length);
	return data;
}