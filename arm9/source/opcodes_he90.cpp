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

void _0x0A_Dup_n()
{
	int num;
	int args[16];

	push(fetchScriptWord());
	num = getStackList(args, ARRAYSIZE(args));

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < num; j++)
			push(args[j]);
	}
}

void _0x1C_WizImageOps()
{
	int a, b;

	int subOp = fetchScriptByte();
	//printf("Wiz (%d)\n", subOp);
	switch (subOp) {
	case 32: // HE99+
		_wizParams.processFlags |= kWPFUseDefImgWidth;
		_wizParams.resDefImgW = pop();
		break;
	case 33: // HE99+
		_wizParams.processFlags |= kWPFUseDefImgHeight;
		_wizParams.resDefImgH = pop();
		break;
	case 46:
		// Dummy case
		pop();
		break;
	case 47:
		//_wizParams.box.bottom = pop();
		//_wizParams.box.right = pop();
		//_wizParams.box.top = pop();
		//_wizParams.box.left = pop();
		pop();
		pop();
		pop();
		pop();
		break;
	case 48:
		_wizParams.processMode = 1;
		break;
	case 49:
		_wizParams.processFlags |= kWPFUseFile;
		_wizParams.processMode = 3;
		copyScriptString(_wizParams.filename, sizeof(_wizParams.filename));
		break;
	case 50:
		_wizParams.processFlags |= kWPFUseFile;
		_wizParams.processMode = 4;
		copyScriptString(_wizParams.filename, sizeof(_wizParams.filename));
		_wizParams.fileWriteMode = pop();
		break;
	case 51:
		_wizParams.processFlags |= kWPFClipBox | 0x100;
		_wizParams.processMode = 2;
		//_wizParams.box.bottom = pop();
		//_wizParams.box.right = pop();
		//_wizParams.box.top = pop();
		//_wizParams.box.left = pop();
		pop();
		pop();
		pop();
		pop();
		_wizParams.compType = pop();
		//adjustRect(_wizParams.box);
		break;
	case 52:
		_wizParams.processFlags |= kWPFNewState;
		_wizParams.img.state = pop();
		break;
	case 53:
		_wizParams.processFlags |= kWPFRotate;
		_wizParams.angle = pop();
		break;
	case 54:
		_wizParams.processFlags |= kWPFNewFlags;
		_wizParams.img.flags |= pop();
		break;
	case 56:
		_wizParams.img.flags = pop();
		_wizParams.img.state = pop();
		_wizParams.img.y1 = pop();
		_wizParams.img.x1 = pop();
		_wizParams.img.resNum = pop();
		displayWizImage(&_wizParams.img);
		break;
	case 57:
		_wizParams.img.resNum = pop();
		_wizParams.processMode = 0;
		_wizParams.processFlags = 0;
		_wizParams.remapNum = 0;
		_wizParams.img.flags = 0;
		_wizParams.params1 = 0;
		_wizParams.params2 = 0;
		_wizParams.spriteId = 0;
		_wizParams.spriteGroup = 0;
		break;
	case 62: // HE99+
		_wizParams.processFlags |= kWPFMaskImg;
		_wizParams.sourceImage = pop();
		break;
	case 65:
	case 154:
		_wizParams.processFlags |= kWPFSetPos;
		_wizParams.img.y1 = pop();
		_wizParams.img.x1 = pop();
		break;
	case 66:
	case 249: // HE98+
		b = pop();
		a = pop();
		_wizParams.processFlags |= kWPFRemapPalette;
		_wizParams.processMode = 6;
		if (_wizParams.remapNum == 0) {
			memset(_wizParams.remapIndex, 0, sizeof(_wizParams.remapIndex));
		}
		//assert(_wizParams.remapNum < ARRAYSIZE(_wizParams.remapIndex));
		_wizParams.remapIndex[_wizParams.remapNum] = a;
		_wizParams.remapColor[a] = b;
		_wizParams.remapNum++;
		break;
	case 67:
		_wizParams.processFlags |= kWPFClipBox;
		//_wizParams.box.bottom = pop();
		//_wizParams.box.right = pop();
		//_wizParams.box.top = pop();
		//_wizParams.box.left = pop();
		//adjustRect(_wizParams.box);
		pop();
		pop();
		pop();
		pop();
		break;
	case 86: // HE99+
		_wizParams.processFlags |= kWPFPaletteNum;
		_wizParams.img.palette = pop();
		break;
	case 92:
		_wizParams.processFlags |= kWPFScaled;
		_wizParams.scale = pop();
		break;
	case 98:
		_wizParams.processFlags |= kWPFShadow;
		_wizParams.img.shadow = pop();
		break;
	case 131: // HE99+
		_wizParams.processFlags |= 0x1000 | 0x100 | 0x2;
		_wizParams.processMode = 7;
		_wizParams.polygonId2 = pop();
		_wizParams.polygonId1 = pop();
		_wizParams.compType = pop();
		break;
	case 133: // HE99+
		_wizParams.processFlags |= kWPFFillColor | kWPFClipBox2;
		_wizParams.processMode = 9;
		_wizParams.fillColor = pop();
		//_wizParams.box2.bottom = pop();
		//_wizParams.box2.right = pop();
		//_wizParams.box2.top = pop();
		//_wizParams.box2.left = pop();
		//adjustRect(_wizParams.box2);
		pop();
		pop();
		pop();
		pop();
		break;
	case 134: // HE99+
		_wizParams.processFlags |= kWPFFillColor | kWPFClipBox2;
		_wizParams.processMode = 10;
		_wizParams.fillColor = pop();
		//_wizParams.box2.bottom = pop();
		//_wizParams.box2.right = pop();
		//_wizParams.box2.top = pop();
		//_wizParams.box2.left = pop();
		//adjustRect(_wizParams.box2);
		pop();
		pop();
		pop();
		pop();
		break;
	case 135: // HE99+
		_wizParams.processFlags |= kWPFFillColor | kWPFClipBox2;
		_wizParams.processMode = 11;
		_wizParams.fillColor = pop();
		//_wizParams.box2.top = _wizParams.box2.bottom = pop();
		//_wizParams.box2.left = _wizParams.box2.right = pop();
		//adjustRect(_wizParams.box2);
		pop();
		pop();
		break;
	case 136: // HE99+
		_wizParams.processFlags |= kWPFFillColor | kWPFClipBox2;
		_wizParams.processMode = 12;
		_wizParams.fillColor = pop();
		//_wizParams.box2.top = _wizParams.box2.bottom = pop();
		//_wizParams.box2.left = _wizParams.box2.right = pop();
		//adjustRect(_wizParams.box2);
		pop();
		pop();
		break;
	case 137: // HE99+
		_wizParams.processFlags |= kWPFDstResNum;
		_wizParams.dstResNum = pop();
		break;
	case 139: // HE99+
		_wizParams.processFlags |= kWPFParams;
		_wizParams.params1 = pop();
		_wizParams.params2 = pop();
		break;
	case 141: // HE99+
		_wizParams.processMode = 13;
		break;
	case 142: // HE99+
		_wizParams.field_239D = pop();
		_wizParams.field_2399 = pop();
		_wizParams.field_23A5 = pop();
		_wizParams.field_23A1 = pop();
		copyScriptString(_wizParams.string2, sizeof(_wizParams.string2));
		_wizParams.processMode = 15;
		break;
	case 143: // HE99+
		_wizParams.processMode = 16;
		_wizParams.field_23AD = pop();
		_wizParams.field_23A9 = pop();
		copyScriptString(_wizParams.string1, sizeof(_wizParams.string1));
		break;
	case 189: // HE99+
		_wizParams.processMode = 17;
		_wizParams.field_23CD = pop();
		_wizParams.field_23C9 = pop();
		_wizParams.field_23C5 = pop();
		_wizParams.field_23C1 = pop();
		_wizParams.field_23BD = pop();
		_wizParams.field_23B9 = pop();
		_wizParams.field_23B5 = pop();
		_wizParams.field_23B1 = pop();
		break;
	case 196: // HE99+
		_wizParams.processMode = 14;
		break;
	case 217: // HE99+
		_wizParams.processMode = 8;
		break;
	case 246:
		_wizParams.processFlags |= kWPFNewFlags | kWPFSetPos | 2;
		_wizParams.img.flags |= kWIFIsPolygon;
		_wizParams.polygonId1 = _wizParams.img.y1 = _wizParams.img.x1 = pop();
		break;
	case 255:
		if (_wizParams.img.resNum)
			processWizImage(&_wizParams);
		printf("Wiz!\n");
		break;
	default:
		printf("Error: o90_wizImageOps: unhandled case %d\n", subOp);
	} 
}

void _0x1D_Min()
{
	int a = pop();
	int b = pop();

	if (b < a) {
		push(b);
	} else {
		push(a);
	}
}

void _0x1E_Max()
{
	int a = pop();
	int b = pop();

	if (b > a) {
		push(b);
	} else {
		push(a);
	}
}

void _0x22_ATan2()
{
	int y = pop();
	int x = pop();
	/*int a = (int)(atan2((double)y, (double)x) * 180. / M_PI);
	if (a < 0) {
		a += 360;
	}*/
	push(/*a*/0);
}

void _0x25_GetSpriteInfo()
{
	int args[16];
	int spriteId, flags, groupId, type;
	int32 x, y;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 30:
		spriteId = pop();
		if (spriteId) {
			//_sprite->getSpritePosition(spriteId, x, y);
			push(/*x*/0);
		} else {
			push(0);
		}
		break;
	case 31:
		spriteId = pop();
		if (spriteId) {
			//_sprite->getSpritePosition(spriteId, x, y);
			push(/*y*/0);
		} else {
			push(0);
		}
		break;
	case 32:
		spriteId = pop();
		if (spriteId) {
			//_sprite->getSpriteImageDim(spriteId, x, y);
			push(/*x*/0);
		} else {
			push(0);
		}
		break;
	case 33:
		spriteId = pop();
		if (spriteId) {
			//_sprite->getSpriteImageDim(spriteId, x, y);
			push(/*y*/0);
		} else {
			push(0);
		}
		break;
	case 34:
		spriteId = pop();
		if (spriteId) {
			//_sprite->getSpriteDist(spriteId, x, y);
			push(/*x*/0);
		} else {
			push(0);
		}
		break;
	case 35:
		spriteId = pop();
		if (spriteId) {
			//_sprite->getSpriteDist(spriteId, x, y);
			push(/*y*/0);
		} else {
			push(0);
		}
		break;
	case 36:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteImageStateCount(spriteId)*/0);
		else
			push(0);
		break;
	case 37:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteGroup(spriteId)*/0);
		else
			push(0);
		break;
	case 38:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteDisplayX(spriteId)*/0);
		else
			push(0);
		break;
	case 39:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteDisplayY(spriteId)*/0);
		else
			push(0);
		break;
	case 42:
		flags = pop();
		spriteId = pop();
		if (spriteId) {
			switch (flags) {
			case 0:
				push(/*_sprite->getSpriteFlagXFlipped(spriteId)*/0);
				break;
			case 1:
				push(/*_sprite->getSpriteFlagYFlipped(spriteId)*/0);
				break;
			case 2:
				push(/*_sprite->getSpriteFlagActive(spriteId)*/0);
				break;
			case 3:
				push(/*_sprite->getSpriteFlagDoubleBuffered(spriteId)*/0);
				break;
			case 4:
				push(/*_sprite->getSpriteFlagRemapPalette(spriteId)*/0);
				break;
			default:
				push(0);
			}
		} else {
			push(0);
		}
		break;
	case 43:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpritePriority(spriteId)*/0);
		else
			push(0);
		break;
	case 45:
		//if (_game.heversion == 99) {
			flags = getStackList(args, ARRAYSIZE(args));
			type = pop();
			groupId = pop();
			y = pop();
			x = pop();
			push(/*_sprite->findSpriteWithClassOf(x, y, groupId, type, flags, args)*/0);
		/*} else if (_game.heversion == 98) {
			type = pop();
			groupId = pop();
			y = pop();
			x = pop();
			push(_sprite->findSpriteWithClassOf(x, y, groupId, type, 0, 0));
		} else {
			groupId = pop();
			y = pop();
			x = pop();
			push(_sprite->findSpriteWithClassOf(x, y, groupId, 0, 0, 0));
		}*/
		//printf("NOT SUPPORTED (OP 0x25, case 45)\n");
		//while(1);
		break;
	case 52:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteImageState(spriteId)*/0);
		else
			push(0);
		break;
	case 62:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteSourceImage(spriteId)*/0);
		else
			push(0);
		break;
	case 63:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteImage(spriteId)*/0);
		else
			push(0);
		break;
	case 68:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteFlagEraseType(spriteId)*/0);
		else
			push(1);
		break;
	case 82:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteFlagAutoAnim(spriteId)*/0);
		else
			push(0);
		break;
	case 86:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpritePalette(spriteId)*/0);
		else
			push(0);
		break;
	case 92:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteScale(spriteId)*/0);
		else
			push(0);
		break;
	case 97:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteAnimSpeed(spriteId)*/0);
		else
			push(1);
		break;
	case 98:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteShadow(spriteId)*/0);
		else
			push(0);
		break;
	case 124:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteFlagUpdateType(spriteId)*/0);
		else
			push(0);
		break;
	case 125:
		flags = getStackList(args, ARRAYSIZE(args));
		spriteId = pop();
		if (spriteId) {
			push(/*_sprite->getSpriteClass(spriteId, flags, args)*/0);
		} else {
			push(0);
		}
		break;
	case 139:
		flags = pop();
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteGeneralProperty(spriteId, flags)*/0);
		else
			push(0);
		break;
	case 140:
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteMaskImage(spriteId)*/0);
		else
			push(0);
		break;
	case 198:
		pop();
		spriteId = pop();
		if (spriteId)
			push(/*_sprite->getSpriteUserValue(spriteId)*/0);
		else
			push(0);
		break;
	default:
		printf("Error: o90_getSpriteInfo: Unknown case %d\n", subOp);
	}
}

void _0x26_SetSpriteInfo()
{
	int args[16];
	int spriteId;
	int32 tmp[2];
	int n;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 34:
		args[0] = pop();
		/*if (_curSpriteId > _curMaxSpriteId)
		break;
		spriteId = _curSpriteId;
		if (!spriteId)
		spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++) {
		_sprite->getSpriteDist(spriteId, tmp[0], tmp[1]);
		_sprite->setSpriteDist(spriteId, args[0], tmp[1]);
		}*/
		break;
	case 35:
		args[0] = pop();
		/*if (_curSpriteId > _curMaxSpriteId)
		break;
		spriteId = _curSpriteId;
		if (!spriteId)
		spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++) {
		_sprite->getSpriteDist(spriteId, tmp[0], tmp[1]);
		_sprite->setSpriteDist(spriteId, tmp[0], args[0]);
		}*/
		break;
	case 37:
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpriteGroup(spriteId, args[0]);
		break;
	case 42:
		args[1] = pop();
		args[0] = pop();
		/*if (_curSpriteId > _curMaxSpriteId)
		break;
		spriteId = _curSpriteId;
		if (!spriteId)
		spriteId++;

		for (; spriteId <= _curMaxSpriteId; spriteId++)
		switch (args[1]) {
		case 0:
		_sprite->setSpriteFlagXFlipped(spriteId, args[0]);
		break;
		case 1:
		_sprite->setSpriteFlagYFlipped(spriteId, args[0]);
		break;
		case 2:
		_sprite->setSpriteFlagActive(spriteId, args[0]);
		break;
		case 3:
		_sprite->setSpriteFlagDoubleBuffered(spriteId, args[0]);
		break;
		case 4:
		_sprite->setSpriteFlagRemapPalette(spriteId, args[0]);
		break;
		default:
		break;
		}*/
		break;
	case 43:
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpritePriority(spriteId, args[0]);
		break;
	case 44:
		args[1] = pop();
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->moveSprite(spriteId, args[0], args[1]);
		break;
	case 52:
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpriteImageState(spriteId, args[0]);
		break;
	case 53:
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpriteAngle(spriteId, args[0]);
		break;
	case 57:
		/*if (_game.features & GF_HE_985 || _game.heversion >= 99) {
		_curMaxSpriteId = pop();
		_curSpriteId = pop();

		if (_curSpriteId > _curMaxSpriteId)
		SWAP(_curSpriteId, _curMaxSpriteId);
		} else {*/
		pop();
		//_curSpriteId = pop();
		//_curMaxSpriteId = _curSpriteId; // to make all functions happy
		//}
		break;
	case 62: // HE99+
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpriteSourceImage(spriteId, args[0]);
		break;
	case 63:
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpriteImage(spriteId, args[0]);
		break;
	case 65:
		args[1] = pop();
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpritePosition(spriteId, args[0], args[1]);
		break;
	case 68:
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpriteFlagEraseType(spriteId, args[0]);
		break;
	case 77:
		args[1] = pop();
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpriteDist(spriteId, args[0], args[1]);
		break;
	case 82:
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpriteFlagAutoAnim(spriteId, args[0]);
		break;
	case 86: // HE 98+
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpritePalette(spriteId, args[0]);
		break;
	case 92: // HE 99+
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpriteScale(spriteId, args[0]);
		break;
	case 97: // HE 98+
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpriteAnimSpeed(spriteId, args[0]);
		break;
	case 98:
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpriteShadow(spriteId, args[0]);
		break;
	case 124:
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpriteFlagUpdateType(spriteId, args[0]);
		break;
	case 125:
		n = getStackList(args, ARRAYSIZE(args));
		/*if (_curSpriteId != 0 && _curMaxSpriteId != 0 && n != 0) {
		int *p = &args[n - 1];
		do {
		int code = *p;
		if (code == 0) {
		for (int i = _curSpriteId; i <= _curMaxSpriteId; ++i) {
		_sprite->setSpriteResetClass(i);
		}
		} else if (code & 0x80) {
		for (int i = _curSpriteId; i <= _curMaxSpriteId; ++i) {
		_sprite->setSpriteSetClass(i, code & 0x7F, 1);
		}
		} else {
		for (int i = _curSpriteId; i <= _curMaxSpriteId; ++i) {
		_sprite->setSpriteSetClass(i, code & 0x7F, 0);
		}
		}
		--p;
		} while (--n);
		}*/
		break;
	case 139: // HE 99+
		args[1] = pop();
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpriteGeneralProperty(spriteId, args[0], args[1]);
		break;
	case 140: // HE 99+
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpriteMaskImage(spriteId, args[0]);
		break;
	case 158:
		//_sprite->resetTables(true);
		break;
	case 198:
		args[1] = pop();
		args[0] = pop();
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->setSpriteUserValue(spriteId, args[0], args[1]);
		break;
	case 217:
		//if (_curSpriteId > _curMaxSpriteId)
		//	break;
		//spriteId = _curSpriteId;
		//if (!spriteId)
		//	spriteId++;

		//for (; spriteId <= _curMaxSpriteId; spriteId++)
		//	_sprite->resetSprite(spriteId);
		break;
	default:
		printf("Error: o90_setSpriteInfo: Unknown case %d\n", subOp);
	}
}

void _0x27_GetSpriteGroupInfo()
{
	int32 tx, ty;
	int spriteGroupId, type;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 8: // HE 99+
		spriteGroupId = pop();
		//if (spriteGroupId)
		//	push(getGroupSpriteArray(spriteGroupId));
		//else
			push(0);
		break;
	case 30:
		spriteGroupId = pop();
		//if (spriteGroupId) {
		//	_sprite->getGroupPosition(spriteGroupId, tx, ty);
		//	push(tx);
		//} else {
			push(0);
		//}
		break;
	case 31:
		spriteGroupId = pop();
		//if (spriteGroupId) {
		//	_sprite->getGroupPosition(spriteGroupId, tx, ty);
		//	push(ty);
		//} else {
			push(0);
		//}
		break;
	case 42: // HE 99+
		type = pop();
		spriteGroupId = pop();
		/*if (spriteGroupId) {
			switch (type) {
			case 0:
				push(_sprite->getGroupXMul(spriteGroupId));
				break;
			case 1:
				push(_sprite->getGroupXDiv(spriteGroupId));
				break;
			case 2:
				push(_sprite->getGroupYMul(spriteGroupId));
				break;
			case 3:
				push(_sprite->getGroupYDiv(spriteGroupId));
				break;
			default:
				push(0);
			}
		} else {*/
			push(0);
		//}
		break;
	case 43:
		spriteGroupId = pop();
		//if (spriteGroupId)
		//	push(_sprite->getGroupPriority(spriteGroupId));
		//else
			push(0);
		break;
	case 63: // HE 99+
		spriteGroupId = pop();
		//if (spriteGroupId)
		//	push(_sprite->getGroupDstResNum(spriteGroupId));
		//else
			push(0);
		break;
	case 139: // HE 99+
		// dummy case
		pop();
		pop();
		push(0);
		break;
	default:
		printf("Error: o90_getSpriteGroupInfo: Unknown case %d\n", subOp);
	}
}

void _0x28_SetSpriteGroupInfo() 
{
	int type, value1, value2, value3, value4;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 37:
		type = pop() - 1;
		switch (type) {
		case 0:
			value2 = pop();
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			//_sprite->moveGroupMembers(_curSpriteGroupId, value1, value2);
			break;
		case 1:
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			//_sprite->setGroupMembersPriority(_curSpriteGroupId, value1);
			break;
		case 2:
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			//_sprite->setGroupMembersGroup(_curSpriteGroupId, value1);
			break;
		case 3:
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			//_sprite->setGroupMembersUpdateType(_curSpriteGroupId, value1);
			break;
		case 4:
			if (!_curSpriteGroupId)
				break;

			//_sprite->setGroupMembersResetSprite(_curSpriteGroupId);
			break;
		case 5:
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			//_sprite->setGroupMembersAnimationSpeed(_curSpriteGroupId, value1);
			break;
		case 6:
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			//_sprite->setGroupMembersAutoAnimFlag(_curSpriteGroupId, value1);
			break;
		case 7:
			value1 = pop();
			if (!_curSpriteGroupId)
				break;

			//_sprite->setGroupMembersShadow(_curSpriteGroupId, value1);
			break;
		default:
			printf("Error: o90_setSpriteGroupInfo subOp 0: Unknown case %d\n", subOp);
		}
		break;
	case 42:
		type = pop();
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		switch (type) {
		case 0:
			//_sprite->setGroupXMul(_curSpriteGroupId, value1);
			break;
		case 1:
			//_sprite->setGroupXDiv(_curSpriteGroupId, value1);
			break;
		case 2:
			//_sprite->setGroupYMul(_curSpriteGroupId, value1);
			break;
		case 3:
			//_sprite->setGroupYDiv(_curSpriteGroupId, value1);
			break;
		default:
			printf("Error: o90_setSpriteGroupInfo subOp 5: Unknown case %d\n", subOp);
		}
		break;
	case 43:
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		//_sprite->setGroupPriority(_curSpriteGroupId, value1);
		break;
	case 44:
		value2 = pop();
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		//_sprite->moveGroup(_curSpriteGroupId, value1, value2);
		break;
	case 57:
		_curSpriteGroupId = pop();
		break;
	case 63:
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		//_sprite->setGroupImage(_curSpriteGroupId, value1);
		break;
	case 65:
		value2 = pop();
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		//_sprite->setGroupPosition(_curSpriteGroupId, value1, value2);
		break;
	case 67:
		value4 = pop();
		value3 = pop();
		value2 = pop();
		value1 = pop();
		if (!_curSpriteGroupId)
			break;

		//_sprite->setGroupBounds(_curSpriteGroupId, value1, value2, value3, value4);
		break;
	case 93:
		if (!_curSpriteGroupId)
			break;

		//_sprite->resetGroupBounds(_curSpriteGroupId);
		break;
	case 217:
		if (!_curSpriteGroupId)
			break;

		//_sprite->resetGroup(_curSpriteGroupId);
		break;
	default:
		printf("Error: o90_setSpriteGroupInfo: Unknown case %d\n", subOp);
	}
} 

void _0x34_FindAllObjectsWithClassOf() {
	int args[16];
	int cond, num, cls, tmp;
	bool b;

	num = getStackList(args, ARRAYSIZE(args));
	int room = pop();
	int numObjs = 0;

	if (room != _currentRoom)
		printf("Error: o90_findAllObjectsWithClassOf: current room is not %d\n", room);

	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, RoomResource->RMDA->RMHD.NrObjects);
	/*for (int i = 1; i < _numLocalObjects; i++) {
		cond = 1;
		tmp = num;
		while (--tmp >= 0) {
			cls = args[tmp];
			b = getClass(_objs[i].obj_nr, cls);
			if ((cls & 0x80 && !b) || (!(cls & 0x80) && b))
				cond = 0;
		}

		if (cond) {
			numObjs++;
			writeArray(0, 0, numObjs, _objs[i].obj_nr);
		}
	}*/

	writeArray(0, 0, 0, numObjs);

	push(readVar(0));
} 

void _0x37_Dim2Dim2Array()
{
	int data, dim1start, dim1end, dim2start, dim2end; 

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
		printf("Error: o90_dim2dimArray: default case %d\n", subOp);
	}

	if (pop() == 2) {
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
	} else {
		dim2end = pop();
		dim2start = pop();
		dim1end = pop();
		dim1start = pop();
	}

	defineArray(fetchScriptWord(), data, dim2start, dim2end, dim1start, dim1end); 
}

void _0x39_GetLinesIntersectionPoint() 
{
	int var_ix = fetchScriptWord();
	int var_iy = fetchScriptWord();
	int line2_y2 = pop();
	int line2_x2 = pop();
	int line2_y1 = pop();
	int line2_x1 = pop();
	int line1_y2 = pop();
	int line1_x2 = pop();
	int line1_y1 = pop();
	int line1_x1 = pop();

	int result = 0;
	int ix = 0;
	int iy = 0;

	bool isLine1Point = (line1_x1 == line1_x2 && line1_y1 == line1_y2);
	bool isLine2Point = (line2_x1 == line2_x2 && line2_y1 == line2_y2);

	if (isLine1Point) {
		if (isLine2Point) {
			if (line1_x1 == line2_x1 && line1_y1 == line2_y2) {
				ix = line1_x1;
				iy = line2_x1;
				result = 1;
			}
		} else {
			// 1 point and 1 line
			int dx2 = line2_x2 - line2_x1;
			if (dx2 != 0) {
				int dy2 = line2_y2 - line2_y1;
				float y = (float)dy2 / dx2 * (line1_x1 - line2_x1) + line2_y1 + .5f;
				if (line1_y1 == (int)y) {
					ix = line1_x1;
					iy = line1_y1;
					result = 1;
				}
			} else {
				// vertical line
				if (line1_x1 == line2_x1) {
					if (line2_y1 > line2_y2) {
						if (line1_y1 >= line2_y2 && line1_y1 <= line2_y1) {
							ix = line1_x1;
							iy = line1_y1;
							result = 1;
						}
					} else {
						if (line1_y1 >= line2_y1 && line1_y1 <= line2_y2) {
							ix = line1_x1;
							iy = line1_y1;
							result = 1;
						}
					}
				}
			}
		}
	} else {
		if (isLine2Point) {
			// 1 point and 1 line
			int dx1 = line1_x2 - line1_x1;
			if (dx1 != 0) {
				int dy1 = line1_y2 - line1_y1;
				float y = (float)dy1 / dx1 * (line2_x1 - line1_x1) + line1_y1 + .5f;
				if (line2_y1 == (int)y) {
					ix = line2_x1;
					iy = line2_y1;
					result = 1;
				}
			} else {
				// vertical line
				if (line2_x1 == line1_x1) {
					if (line1_y1 > line1_y2) {
						if (line2_y1 >= line1_y2 && line2_y1 <= line1_y1) {
							ix = line2_x1;
							iy = line2_y1;
							result = 1;
						}
					} else {
						if (line2_y1 >= line1_y1 && line2_y1 <= line1_y2) {
							ix = line2_x2;
							iy = line2_y1;
							result = 1;
						}
					}
				}
			}
		} else {
			// 2 lines
			int dy1 = line1_y2 - line1_y1;
			int dx1 = line1_x2 - line1_x1;
			int dy2 = line2_y2 - line2_y1;
			int dx2 = line2_x2 - line2_x1;
			int det = dx1 * dy2 - dx2 * dy1;
			int cross_p1 = dx1 * (line1_y1 - line2_y1) - dy1 * (line1_x1 - line2_x1);
			int cross_p2 = dx2 * (line1_y1 - line2_y1) - dy2 * (line1_x1 - line2_x1);
			if (det == 0) {
				// parallel lines
				if (cross_p2 == 0) {
					ix = abs(line2_x2 + line2_x1) / 2;
					iy = abs(line2_y2 + line2_y1) / 2;
					result = 2;
				}
			} else {
				float rcp1 = (float)cross_p1 / det;
				float rcp2 = (float)cross_p2 / det;
				if (rcp1 >= 0 && rcp1 <= 1 && rcp2 >= 0 && rcp2 <= 1) {
					ix = (int)(dx1 * rcp2 + line1_x1 + .5f);
					iy = (int)(dy1 * rcp2 + line1_y1 + .5f);
					result = 1;
				}
			}
		}
	}

	writeVar(var_ix, ix);
	writeVar(var_iy, iy);
	push(result);
} 

void _0x3A_SortArray() {
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 129:
	case 134: // HE100
		{
			int array = fetchScriptWord();
			int sortOrder = pop();
			int dim1end = pop();
			int dim1start = pop();
			int dim2end = pop();
			int dim2start = pop();
			getArrayDim(array, &dim2start, &dim2end, &dim1start, &dim1end);
			sortArray(array, dim2start, dim2end, dim1start, dim1end, sortOrder);
		}
		break;
	default:
		printf("Error: o90_sortArray: Unknown case %d\n", subOp);
	}
} 

void _0x9E_PaletteOps() 
{
	int a, b, c, d, e;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 57:
		pop();
		//_hePaletteNum = pop();
		break;
	case 63:
		b = pop();
		a = pop();
		//if (_hePaletteNum != 0) {
			//setHEPaletteFromImage(_hePaletteNum, a, b);
		//}
		break;
	case 66:
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		//if (_hePaletteNum != 0) {
		//	for (; a <= b; ++a) {
				//setHEPaletteColor(_hePaletteNum, a, c, d, e);
		//	}
		//}
		break;
	case 70:
		c = pop();
		b = pop();
		a = pop();
		//if (_hePaletteNum != 0) {
		//	for (; a <= b; ++a) {
				//copyHEPaletteColor(_hePaletteNum, a, c);
		//	}
		//}
		break;
	case 76: //HE99+
		a = pop();
		//if (_hePaletteNum != 0) {
			//setHEPaletteFromCostume(_hePaletteNum, a);
		//}
		break;
	case 86:
		a = pop();
		//if (_hePaletteNum != 0) {
			//copyHEPalette(_hePaletteNum, a);
		//}
		break;
	case 175:
		b = pop();
		a = pop();
		//if (_hePaletteNum != 0) {
			//setHEPaletteFromRoom(_hePaletteNum, a, b);
		//}
		break;
	case 217:
		//if (_hePaletteNum != 0) {
			//restoreHEPalette(_hePaletteNum);
		//}
		break;
	case 255:
		//_hePaletteNum = 0;
		break;
	default:
		printf("Error: o90_paletteOps: Unknown case %d\n", subOp);
	}
} 

void _0xA5_FontUnk() {
	// Font related
	byte string[80];
	int a;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 60:	// HE100
	case 42:
		a = pop();
		if (a == 2) {
			copyScriptString(string, sizeof(string));
			push(-1);
		} else if (a == 1) {
			pop();
			writeVar(0, 0);
			defineArray(0, kStringArray, 0, 0, 0, 0);
			writeArray(0, 0, 0, 0);
			push(readVar(0));
		}
		break;
	case 0:		// HE100
	case 57:
		push(1);
		break;
	default:
		printf("Error: o90_fontUnk: Unknown case %d\n", subOp);
	}

	printf("o90_fontUnk stub (%d)\n", subOp);
} 

void _0xC9_KernelSetFunctions()
{
	int args[29];
	int num, tmp;
	Actor *a;

	num = getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 20:
		a = &_actors[args[1]];//(ActorHE *)derefActor(args[1], "o90_kernelSetFunctions: 20");
		//queueAuxBlock(a);
		break;
	case 21:
		//_skipDrawObject = 1;
		break;
	case 22:
		//_skipDrawObject = 0;
		break;
	case 23:
		//clearCharsetMask();
		//_fullRedraw = true;
		break;
	case 24:
		//_skipProcessActors = 1;
		//redrawAllActors();
		break;
	case 25:
		//_skipProcessActors = 0;
		//redrawAllActors();
		break;
	case 27:
		// Used in readdemo
		break;
	case 42:
		//_wiz->_rectOverrideEnabled = true;
		//_wiz->_rectOverride.left = args[1];
		//_wiz->_rectOverride.top = args[2];
		//_wiz->_rectOverride.right = args[3];
		//_wiz->_rectOverride.bottom = args[4];
		//adjustRect(_wiz->_rectOverride);
		break;
	case 43:
		//_wiz->_rectOverrideEnabled = false;
		break;
	case 714:
		//setResourceOffHeap(args[1], args[2], args[3]);
		break;
	case 1492:
		// Remote start script function
		break;
	case 1969:
		a = &_actors[args[1]];//(ActorHE *)derefActor(args[1], "o90_kernelSetFunctions: 1969");
		tmp = a->_heCondMask;
		tmp ^= args[2];
		tmp &= 0x7FFF0000;
		a->_heCondMask ^= tmp;
		break;
	case 2001:
		//_logicHE->dispatch(args[1], num - 2, (int32 *)&args[2]);
		break;
	case 201102: // Used in puttzoo iOS
	case 20111014: // Used in spyfox iOS
		break;
	default:
		printf("Error: o90_kernelSetFunctions: default case %d (param count %d)\n", args[0], num);
	}
}