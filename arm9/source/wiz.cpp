#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <ioutil.h>
#include <HE0.h>
#include <scumm.h>
#include <wiz.h>
#include <graphics.h>
#include <render.h>
#include <room.h>

WizParameters _wizParams;

void freeAWIZ(AWIZ_t* AWIZ)
{
	free(AWIZ->RGBS);
	free(AWIZ);
}

AWIZ_t* readAWIZ()
{
	uint32_t sig;
	uint32_t size;
	fpos_t pos;
	readU32LE(HE1_File, &sig);
	//printf("WIZ: %c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
	readU32LE(HE1_File, &size);
	fgetpos(HE1_File, &pos);
	uint32_t end = pos + SWAP_CONSTANT_32(size) - 8;
	AWIZ_t* AWIZ = (AWIZ_t*)malloc(sizeof(AWIZ_t));
	AWIZ->RGBS = NULL;
	while(pos < (end - 4))
	{
		readU32LE(HE1_File, &sig);
		//printf("%c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
		readU32LE(HE1_File, &size);
		switch(sig)
		{
		case MKTAG('R', 'G', 'B', 'S'):
			AWIZ->RGBS = malloc(SWAP_CONSTANT_32(size) - 8);
			readBytes(HE1_File, (uint8_t*)AWIZ->RGBS, SWAP_CONSTANT_32(size) - 8);
			break;
		case MKTAG('W', 'I', 'Z', 'H'):
			readU32LE(HE1_File, &AWIZ->Compression);
			readU32LE(HE1_File, &AWIZ->Width);
			readU32LE(HE1_File, &AWIZ->Height);
			break;
		case MKTAG('W', 'I', 'Z', 'D'):
			fgetpos(HE1_File, &AWIZ->WIZD);
			fseek(HE1_File, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		default:
			//printf("Unknown Signature\n");
			fseek(HE1_File, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		}
		fgetpos(HE1_File, &pos);
	}
	return AWIZ;
}

void displayWizImage(WizImage *pwi) {
	/*if (_vm->_fullRedraw) {
	assert(_imagesNum < ARRAYSIZE(_images));
	WizImage *wi = &_images[_imagesNum];
	wi->resNum = pwi->resNum;
	wi->x1 = pwi->x1;
	wi->y1 = pwi->y1;
	wi->zorder = 0;
	wi->state = pwi->state;
	wi->flags = pwi->flags;
	wi->shadow = 0;
	wi->field_390 = 0;
	wi->palette = 0;
	++_imagesNum;
	} else if (pwi->flags & kWIFIsPolygon) {
	drawWizPolygon(pwi->resNum, pwi->state, pwi->x1, pwi->flags, 0, 0, 0);
	} else {
	const Common::Rect *r = NULL;
	drawWizImage(pwi->resNum, pwi->state, 0, 0, pwi->x1, pwi->y1, 0, 0, 0, r, pwi->flags, 0, _vm->getHEPaletteSlot(0));
	}*/
	//if(pwi->flags)
	{
		uint32_t offs = getAWIZOffset(HE0_File, &HE0_Data, pwi->resNum);
		//printf("displayWizImage: %d (Offs: 0x%X)\n", pwi->resNum, offs);
		fseek(HE1_File, offs, SEEK_SET);
		AWIZ_t* AWIZ = readAWIZ();
		if(AWIZ->RGBS != NULL) ConvertWIZImage(AWIZ->WIZD, AWIZ->RGBS, pwi->x1, pwi->y1, AWIZ->Width, AWIZ->Height, AWIZ->Compression);
		else ConvertWIZImage(AWIZ->WIZD, &RoomResource->RMDA->PALS->WRAP->APAL->data, pwi->x1, pwi->y1, AWIZ->Width, AWIZ->Height, AWIZ->Compression);
		freeAWIZ(AWIZ);
		//while(1);
	}
}

void processWizImage(const WizParameters *params) {
	byte buffer[260];

	printf("processWizImage: processMode %d\n", params->processMode);
	switch (params->processMode) {
	case 0:
		// Used in racedemo
		break;
	case 1:
		{
			//if(params->img.flags)
			//{
			uint32_t offs = getAWIZOffset(HE0_File, &HE0_Data, params->img.resNum);
			//printf("displayWizImage: %d (Offs: 0x%X)\n", params->img.resNum, offs);
			fseek(HE1_File, offs, SEEK_SET);
			AWIZ_t* AWIZ = readAWIZ();
			if(AWIZ->RGBS != NULL) ConvertWIZImage(AWIZ->WIZD, AWIZ->RGBS, params->img.x1, params->img.y1, AWIZ->Width, AWIZ->Height, AWIZ->Compression);
			else ConvertWIZImage(AWIZ->WIZD, &RoomResource->RMDA->PALS->WRAP->APAL->data, params->img.x1, params->img.y1, AWIZ->Width, AWIZ->Height, AWIZ->Compression);
			freeAWIZ(AWIZ);
			//}
			//displayWizComplexImage(params);
		}
		break;
	case 2:
		//captureWizImage(params->img.resNum, params->box, (params->img.flags & kWIFBlitToFrontVideoBuffer) != 0, params->compType);
		break;
	case 3:
		/*if (params->processFlags & kWPFUseFile) {
		Common::SeekableReadStream *f = NULL;
		memcpy(buffer, params->filename, 260);
		const char *filename = (char *)buffer + _vm->convertFilePath(buffer, sizeof(buffer));

		if (!_vm->_saveFileMan->listSavefiles(filename).empty()) {
		f = _vm->_saveFileMan->openForLoading(filename);
		} else {
		f = SearchMan.createReadStreamForMember(filename);
		}

		if (f) {
		uint32 id = f->readUint32BE();
		if (id == MKTAG('A','W','I','Z') || id == MKTAG('M','U','L','T')) {
		uint32 size = f->readUint32BE();
		f->seek(0, SEEK_SET);
		byte *p = _vm->_res->createResource(rtImage, params->img.resNum, size);
		if (f->read(p, size) != size) {
		_vm->_res->nukeResource(rtImage, params->img.resNum);
		error("i/o error when reading '%s'", filename);
		_vm->VAR(_vm->VAR_GAME_LOADED) = -2;
		_vm->VAR(119) = -2;
		} else {
		_vm->_res->setModified(rtImage, params->img.resNum);
		_vm->VAR(_vm->VAR_GAME_LOADED) = 0;
		_vm->VAR(119) = 0;
		}
		} else {
		_vm->VAR(_vm->VAR_GAME_LOADED) = -1;
		_vm->VAR(119) = -1;
		}
		delete f;
		} else {
		_vm->VAR(_vm->VAR_GAME_LOADED) = -3;
		_vm->VAR(119) = -3;
		debug(0, "Unable to open for read '%s'", filename);
		}
		}*/
		break;
	case 4:
		/*if (params->processFlags & kWPFUseFile) {
		Common::OutSaveFile *f;
		memcpy(buffer, params->filename, 260);
		const char *filename = (char *)buffer + _vm->convertFilePath(buffer, sizeof(buffer));

		switch (params->fileWriteMode) {
		case 2:
		_vm->VAR(119) = -1;
		break;
		case 1:
		// TODO Write image to file
		break;
		case 0:
		if (!(f = _vm->_saveFileMan->openForSaving(filename))) {
		debug(0, "Unable to open for write '%s'", filename);
		_vm->VAR(119) = -3;
		} else {
		byte *p = _vm->getResourceAddress(rtImage, params->img.resNum);
		uint32 size = READ_BE_UINT32(p + 4);
		if (f->write(p, size) != size) {
		error("i/o error when writing '%s'", filename);
		_vm->VAR(119) = -2;
		} else {
		_vm->VAR(119) = 0;
		}
		f->finalize();
		delete f;
		}
		break;
		default:
		error("processWizImage: processMode 4 unhandled fileWriteMode %d", params->fileWriteMode);
		}
		}*/
		break;
	case 6:
		//remapWizImagePal(params);
		break;
		// HE 99+
	case 7:
		//captureWizPolygon(params->img.resNum, params->sourceImage, (params->processFlags & kWPFNewState) ? params->img.state : 0, params->polygonId1, params->polygonId2, params->compType);
		break;
	case 8: {
		/*	int img_w = 640;
		if (params->processFlags & kWPFUseDefImgWidth) {
		img_w = params->resDefImgW;
		}
		int img_h = 480;
		if (params->processFlags & kWPFUseDefImgHeight) {
		img_h = params->resDefImgH;
		}
		int img_x = 0;
		int img_y = 0;
		if (params->processFlags & 1) {
		img_x = params->img.x1;
		img_y = params->img.y1;
		}
		if (params->processFlags & kWPFParams) {
		debug(0, "Compression %d Color Depth %d", params->params1, params->params2);
		}
		createWizEmptyImage(params->img.resNum, img_x, img_y, img_w, img_h);*/
			}
			break;
	case 9:
		//fillWizRect(params);
		break;
	case 10:
		//fillWizLine(params);
		break;
	case 11:
		//fillWizPixel(params);
		break;
	case 12:
		//fillWizFlood(params);
		break;
	case 13:
		// Used for text in FreddisFunShop/PuttsFunShop/SamsFunShop
		// TODO: Start Font
		break;
	case 14:
		// Used for text in FreddisFunShop/PuttsFunShop/SamsFunShop
		// TODO: End Font
		break;
	case 15:
		// Used for text in FreddisFunShop/PuttsFunShop/SamsFunShop
		// TODO: Create Font
		break;
	case 16:
		// TODO: Render Font String
		printf("Error: Render Font String\n");
		break;
	case 17:
		// Used in to draw circles in FreddisFunShop/PuttsFunShop/SamsFunShop
		// TODO: Ellipse
		//_vm->_res->setModified(rtImage, params->img.resNum);
		break;
	default:
		printf("Error: Unhandled processWizImage mode %d\n", params->processMode);
	}
} 

void loadWizCursor(int resId, int palette) {
	/*int32 x, y;
	getWizImageSpot(resId, 0, x, y);
	if (x < 0) {
	x = 0;
	} else if (x > 32) {
	x = 32;
	}
	if (y < 0) {
	y = 0;
	} else if (y > 32) {
	y = 32;
	}

	const Common::Rect *r = NULL;
	_cursorImage = true;
	uint8 *cursor = drawWizImage(resId, 0, 0, 0, 0, 0, 0, 0, 0, r, kWIFBlitToMemBuffer, 0, _vm->getHEPaletteSlot(palette));
	_cursorImage = false;

	int32 cw, ch;
	getWizImageDim(resId, 0, cw, ch);
	_vm->setCursorHotspot(x, y);
	_vm->setCursorFromBuffer(cursor, cw, ch, cw * _vm->_bytesPerPixel);

	// Since we set up cursor palette for default cursor, disable it now
	CursorMan.disableCursorPalette(true);

	free(cursor);*/

	uint32_t offs = getAWIZOffset(HE0_File, &HE0_Data, resId);
	//printf("WizCursor: %d (Offs: 0x%X)\n", resId, offs);
	fseek(HE1_File, offs, SEEK_SET);
	AWIZ_t* AWIZ = readAWIZ();
	for(int i = 0; i < 32 * 32; i++)
	{
		CursorBuffer[i] = 0x8000;
	}
	if(AWIZ->RGBS != NULL) ConvertWIZCursor(AWIZ->WIZD, (uint8_t*)&CursorBuffer[0], AWIZ->RGBS, AWIZ->Width, AWIZ->Height, AWIZ->Compression);
	else ConvertWIZCursor(AWIZ->WIZD, (uint8_t*)&CursorBuffer[0], &RoomResource->RMDA->PALS->WRAP->APAL->data, AWIZ->Width, AWIZ->Height, AWIZ->Compression);
	freeAWIZ(AWIZ);
}

WizPolygon _polygons[NUM_POLYGONS]; 

void polygonStore(int id, bool flag, int vert1x, int vert1y, int vert2x, int vert2y, int vert3x, int vert3y, int vert4x, int vert4y)
{
	WizPolygon *wp = NULL;
	for (int i = 0; i < ARRAYSIZE(_polygons); ++i) {
		if (_polygons[i].id == 0) {
			wp = &_polygons[i];
			break;
		}
	}
	if (!wp) {
		printf("Error: Wiz::polygonStore: out of polygon slot, max = %d\n", ARRAYSIZE(_polygons));
	}

	wp->vert[0].x = vert1x;
	wp->vert[0].y = vert1y;
	wp->vert[1].x = vert2x;
	wp->vert[1].y = vert2y;
	wp->vert[2].x = vert3x;
	wp->vert[2].y = vert3y;
	wp->vert[3].x = vert4x;
	wp->vert[3].y = vert4y;
	wp->vert[4].x = vert1x;
	wp->vert[4].y = vert1y;
	wp->id = id;
	wp->numVerts = 5;
	wp->flag = flag;

	polygonCalcBoundBox(wp->vert, wp->numVerts, wp->bound);
} 

void polygonCalcBoundBox(Point *vert, int numVerts, Rect &bound)
{
	bound.left = 10000;
	bound.top = 10000;
	bound.right = -10000;
	bound.bottom = -10000;

	// compute bounding box
	for (int j = 0; j < numVerts; j++) {
		Rect r(vert[j].x, vert[j].y, vert[j].x + 1, vert[j].y + 1);
		bound.extend(r);
	}
}

void polygonErase(int fromId, int toId) {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++) {
		if (_polygons[i].id >= fromId && _polygons[i].id <= toId)
			memset(&_polygons[i], 0, sizeof(WizPolygon));
	}
}

int polygonHit(int id, int x, int y) {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++) {
		if ((id == 0 || _polygons[i].id == id) && _polygons[i].bound.contains(x, y)) {
			if (polygonContains(_polygons[i], x, y)) {
				return _polygons[i].id;
			}
		}
	}
	return 0;
} 

bool polygonDefined(int id) {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++)
		if (_polygons[i].id == id)
			return true;
	return false;
}

bool polygonContains(const WizPolygon &pol, int x, int y) {
	int pi = pol.numVerts - 1;
	bool diry = (y < pol.vert[pi].y);
	bool curdir;
	bool r = false;

	for (int i = 0; i < pol.numVerts; i++) {
		curdir = (y < pol.vert[i].y);

		if (curdir != diry) {
			if (((pol.vert[pi].y - pol.vert[i].y) * (pol.vert[i].x - x) <
				(pol.vert[pi].x - pol.vert[i].x) * (pol.vert[i].y - y)) == diry)
				r = !r;
		}

		pi = i;
		diry = curdir;
	}

	// HE80+
	int a, b;
	pi = pol.numVerts - 1;
	if (r == 0) {
		for (int i = 0; i < pol.numVerts; i++) {
			if (pol.vert[i].y == y && pol.vert[i].y == pol.vert[pi].y) {

				a = pol.vert[i].x;
				b = pol.vert[pi].x;

				if (pol.vert[i].x >= pol.vert[pi].x)
					a = pol.vert[pi].x;

				if (pol.vert[i].x > pol.vert[pi].x)
					b = pol.vert[i].x;

				if (x >= a && x <= b)
					return 1;

			} else if (pol.vert[i].x == x && pol.vert[i].x == pol.vert[pi].x) {

				a = pol.vert[i].y;
				b = pol.vert[i].y;

				if (pol.vert[i].y >= pol.vert[pi].y)
					a = pol.vert[pi].y;

				if (pol.vert[i].y <= pol.vert[pi].y)
					b = pol.vert[pi].y;

				if (y >= a && y <= b)
					return 1;
			}
			pi = i;
		}
	}

	return r;
}