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

int getStackList(int *args, uint maxnum) {
	uint num, i;

	for (i = 0; i < maxnum; i++)
		args[i] = 0;

	num = pop();

	if (num > maxnum)
	{
		printf("Error: Too many items %d in stack list, max %d\n", num, maxnum);
	}

	i = num;
	while (i--) {
		args[i] = pop();
	}

	return num;
} 

int popRoomAndObj(int *room) {
	int obj;

	*room = pop();
	obj = pop();

	return obj;
}

void copyScriptString(byte *dst, int dstSize) {
	byte string[1024];
	byte chr;
	int pos = 0;

	int array = pop();
	if (array < 0) {
		if (_stringLength == 1)
			printf("Error: String stack underflow\n");

		_stringLength -= 2;
		while ((chr = _stringBuffer[_stringLength]) != 0) {
			string[pos] = chr;
			pos++;

			if (pos > dstSize)
				printf("Error: String too long to pop\n");

			_stringLength--;
		}

		string[pos] = 0;
		_stringLength++;

		// Reverse string
		int len = resStrLen(string);
		while (len--)
			*dst++ = string[len];
	} else {
		writeVar(0, array);
		while ((chr = readArray(0, 0, pos)) != 0) {
			*dst++ = chr;
			pos++;
		}
	}
	*dst = 0;
}

void decodeScriptString(byte *dst, bool scriptString) {
	const byte *src;
	int args[31];
	int num, len, val;
	byte chr, string[1024];
	memset(args, 0, sizeof(args));
	memset(string, 0, sizeof(string));

	// Get stack list, plus one
	num = pop();
	for (int i = num; i >= 0; i--)
		args[i] = pop();

	// Get string
	if (scriptString) {
		len = resStrLen(_scriptPointer) + 1;
		memcpy(string, _scriptPointer, len);
		_scriptPointer += len;
	} else {
		copyScriptString(string, sizeof(string));
		len = resStrLen(string) + 1;
	}

	// Decode string
	num = 0;
	val = 0;
	while (len--) {
		chr = string[num++];
		if (chr == '%') {
			chr = string[num++];
			switch (chr) {
			case 'b':
				val++;
				//dst += sprintf((char *)dst, "%b", args[val++]);
				break;
			case 'c':
				*dst++ = args[val++];
				break;
			case 'd':
				dst += sprintf((char *)dst, "%d", args[val++]);
				break;
			case 's':
				//val++;
				src = _arrays[args[val++] & ~0x33539000]->data;//getStringAddress(args[val++]);
				if (src) {
					while (*src != 0)
						*dst++ = *src++;
				}
				break;
			case 'x':
				dst += sprintf((char *)dst, "%x", args[val++]);
				break;
			default:
				*dst++ = '%';
				num--;
				break;
			}
		} else {
			*dst++ = chr;
		}
	}
	*dst = 0;
}

void decodeParseString(int m, int n) {
	Actor *a;
	int i, colors, size;
	int args[31];
	byte name[1024];

	byte b = fetchScriptByte();

	switch (b) {
	case 65:		// SO_AT
		_string[m].ypos = pop();
		_string[m].xpos = pop();
		_string[m].overhead = false;
		break;
	case 66:		// SO_COLOR
		_string[m].color = pop();
		break;
	case 67:		// SO_CLIPPED
		_string[m].right = pop();
		break;
	case 69:		// SO_CENTER
		_string[m].center = true;
		_string[m].overhead = false;
		break;
	case 71:		// SO_LEFT
		_string[m].center = false;
		_string[m].overhead = false;
		break;
	case 72:		// SO_OVERHEAD
		_string[m].overhead = true;
		_string[m].no_talk_anim = false;
		break;
	case 74:		// SO_MUMBLE
		_string[m].no_talk_anim = true;
		break;
	case 75:		// SO_TEXTSTRING
		printString(m, _scriptPointer);
		_scriptPointer += resStrLen(_scriptPointer) + 1;
		break;
	case 194:
		decodeScriptString(name, true);
		printString(m, name);
		break;
	case 0xE1:
		{
			pop();
			/*byte *dataPtr = getResourceAddress(rtTalkie, pop());
			byte *text = findWrappedBlock(MKTAG('T','E','X','T'), dataPtr, 0, 0);
			size = getResourceDataSize(text);
			memcpy(name, text, size);
			printString(m, name);*/
		}
		break;
	case 0xF9:
		colors = pop();
		if (colors == 1) {
			_string[m].color = pop();
		} else {
			push(colors);
			getStackList(args, ARRAYSIZE(args));
			//for (i = 0; i < 16; i++)
			//	_charsetColorMap[i] = _charsetData[_string[m]._default.charset][i] = (unsigned char)args[i];
			//_string[m].color = _charsetColorMap[0];
		}
		break;
	case 0xFE:
		_string[m].loadDefault();
		if (n) {
			_actorToPrintStrFor = pop();
			if (_actorToPrintStrFor != 0xFF) {
				a = &_actors[_actorToPrintStrFor];//derefActor(_actorToPrintStrFor, "decodeParseString");
				_string[m].color = a->_talkColor;
			}
		}
		break;
	case 0xFF:
		_string[m].saveDefault();
		break;
	default:
		printf("Error: decodeParseString: default case 0x%x\n", b);
	}
}

void printString(int m, const byte *msg) {
	switch (m) {
	case 0:
		//printf("actorTalk:\n%s\n", msg);
		actorTalk(msg);
		break;
	case 1:
		printf("drawString:\n%s\n", msg);
		//drawString(1, msg);
		break;
	case 2:
		printf("debugMessage:\n%s\n", msg);
		//debugMessage(msg);
		break;
	case 3:
		printf("showMessageDialog:\n%s\n", msg);
		//showMessageDialog(msg);
		break;
	}
}

int findFreeArrayId() {
	//const ResourceManager::ResTypeData &rtd = _res->_types[rtString];
	int i;

	for (i = 1; i < _numArray; i++) {
		if (_arrays[i] == NULL)
			return i;
	}
	printf("Error: Out of array pointers, %d max\n", _numArray);
	while(scanKeys(), keysHeld() == 0);
	swiDelay(5000000);
	return -1;
}

static const int arrayDataSizes[] = { 0, 1, 4, 8, 8, 16, 32 };

byte *defineArray(int array, int type, int dim2start, int dim2end, int dim1start, int dim1end)
{
	int id;
	int size;
	//ArrayHeader *ah;

	//assert(dim2start >= 0 && dim2start <= dim2end);
	//assert(dim1start >= 0 && dim1start <= dim1end);
	//assert(0 <= type && type <= 6);


	if (type == kBitArray || type == kNibbleArray)
		type = kByteArray;

	nukeArray(array);

	id = findFreeArrayId();

	//printf("defineArray (array %d (%d), dim2start %d, dim2end %d dim1start %d dim1end %d\n", array, id, dim2start, dim2end, dim1start, dim1end);
	//while(scanKeys(), keysHeld() == 0);
	//swiDelay(5000000);

	if (array & 0x80000000) {
		printf("Error: Can't define bit variable as array pointer\n");
	}

	size = arrayDataSizes[type];

	//if (_game.heversion >= 80)
	id |= 0x33539000;

	writeVar(array, id);

	//if (_game.heversion >= 80)
	id &= ~0x33539000;

	size *= dim2end - dim2start + 1;
	size *= dim1end - dim1start + 1;
	size >>= 3;

	_arrays[id] = (ArrayHeader*)malloc(size + sizeof(ArrayHeader));
	for(int i = 0; i < size + sizeof(ArrayHeader); i++)
	{
		((uint8_t*)_arrays[id])[i] = 0;
	}

	//ah = _arrays[id];//(ArrayHeader *)_res->createResource(rtString, id, size + sizeof(ArrayHeader));

	_arrays[id]->type = type;
	_arrays[id]->dim1start = dim1start;
	_arrays[id]->dim1end = dim1end;
	_arrays[id]->dim2start = dim2start;
	_arrays[id]->dim2end = dim2end;

	return &_arrays[id]->data[0];
}

int readArray(int array, int idx2, int idx1) {
	//printf("readArray (array %d (%d), idx2 %d, idx1 %d)\n", array, readVar(array) & ~0x33539000, idx2, idx1);

	if (readVar(array) == 0)
	{
		printf("Error: readArray: Reference to zeroed array pointer (%d)\n", array);
		//while(scanKeys(), keysHeld() == 0);
		//	swiDelay(5000000);
		return 0;
	}

	ArrayHeader *ah = _arrays[readVar(array) & ~0x33539000];//(ArrayHeader *)getResourceAddress(rtString, readVar(array));

	//printf("ah->type = %d\n", ah->type);
	//printf("ah->dim1start = %d\n", ah->dim1start);
	//printf("ah->dim1end = %d\n", ah->dim1end);
	//printf("ah->dim2start = %d\n", ah->dim2start);
	//printf("ah->dim2end = %d\n", ah->dim2end);

	//while(scanKeys(), keysHeld() == 0);
	//swiDelay(5000000);

	if (ah == NULL)
	{
		printf("Error: readArray: invalid array %d (%d)\n", array, readVar(array) & ~0x33539000);
		//while(scanKeys(), keysHeld() == 0);
		//	swiDelay(5000000);
		return 0;
	}

	if (idx2 < (int)ah->dim2start || idx2 > (int)ah->dim2end ||
		idx1 < (int)ah->dim1start || idx1 > (int)ah->dim1end) {
			printf("Error: readArray: array %d out of bounds: [%d, %d] exceeds [%d..%d, %d..%d]\n", array, idx1, idx2, ah->dim1start, ah->dim1end, ah->dim2start, ah->dim2end);
			//while(scanKeys(), keysHeld() == 0);
			//swiDelay(5000000);
			return 0;
	}

	const int offset = (ah->dim1end - ah->dim1start + 1) *
		(idx2 - ah->dim2start) + (idx1 - ah->dim1start);

	switch (ah->type) {
	case kByteArray:
	case kStringArray:
		return ah->data[offset];

	case kIntArray:
		return (int16)(((uint16_t*)ah->data)[offset]);
		//return (int16)*((uint16_t*)(ah->data + offset * 2));

	case kDwordArray:
		return (int32)(((uint32_t*)ah->data)[offset]);
		//return (int32)*((uint32_t*)(ah->data + offset * 4));
	}

	return 0;
}

void writeArray(int array, int idx2, int idx1, int value) {
	//printf("writeArray (array %d, idx2 %d, idx1 %d, value %d)\n", readVar(array) & ~0x33539000, idx2, idx1, value);

	if (readVar(array) == 0)
	{
		printf("Error: writeArray: Reference to zeroed array pointer\n");
		//while(scanKeys(), keysHeld() == 0);
		//	swiDelay(5000000);
		return;
	}

	ArrayHeader *ah = _arrays[readVar(array) & ~0x33539000];//(ArrayHeader *)getResourceAddress(rtString, readVar(array));

	if (ah == NULL)
	{
		printf("Error: writeArray: Invalid array (%d) reference\n", readVar(array));
		//while(scanKeys(), keysHeld() == 0);
		//	swiDelay(5000000);
		return;
	}

	if (idx2 < (int)ah->dim2start || idx2 > (int)ah->dim2end ||
		idx1 < (int)ah->dim1start || idx1 > (int)ah->dim1end) {
			printf("Error: writeArray: array %d out of bounds: [%d, %d] exceeds [%d..%d, %d..%d]\n",
				array, idx1, idx2, ah->dim1start, ah->dim1end,
				ah->dim2start, ah->dim2end);
			//while(scanKeys(), keysHeld() == 0);
			//swiDelay(5000000);
			return;
	}

	const int offset = (ah->dim1end - ah->dim1start + 1) *
		(idx2 - ah->dim2start) - ah->dim1start + idx1;


	switch (ah->type) {
	case kByteArray:
	case kStringArray:
		ah->data[offset] = value;
		break;

	case kIntArray:
		((uint16_t*)ah->data)[offset] = value;
		//*((uint16_t*)(ah->data + offset * 2)) = value;
		break;

	case kDwordArray:
		((uint32_t*)ah->data)[offset] = value;
		//*((uint32_t*)(ah->data + offset * 4)) = value;
		break;
	}
}

int setupStringArray(int size) {
	writeVar(0, 0);
	defineArray(0, kStringArray, 0, 0, 0, size + 1);
	writeArray(0, 0, 0, 0);
	return readVar(0);
} 

void nukeArray(int a) {
	int data;

	data = readVar(a);

	//if (_game.heversion >= 80)
	data &= ~0x33539000;

	if (data)
	{
		free(_arrays[data]);
		_arrays[data] = NULL;
	}
	//if (_game.heversion >= 60)
	//_arraySlot[data] = 0;

	writeVar(a, 0);
}

void redimArray(int arrayId, int newDim2start, int newDim2end,
	int newDim1start, int newDim1end, int type) {
		int newSize, oldSize;

		if (readVar(arrayId) == 0)
		{
			printf("Error: redimArray: Reference to zeroed array pointer\n");
			return;
		}

		ArrayHeader *ah = _arrays[readVar(arrayId) & ~0x33539000];//(ArrayHeader *)getResourceAddress(rtString, readVar(arrayId));

		if (ah == NULL)
		{
			printf("Error: redimArray: Invalid array (%d) reference\n", readVar(arrayId));
			return;
		}

		newSize = arrayDataSizes[type];
		oldSize = arrayDataSizes[ah->type];

		newSize *= (newDim1end - newDim1start + 1) * (newDim2end - newDim2start + 1);
		oldSize *= (ah->dim1end - ah->dim1start + 1) *
			(ah->dim2end - ah->dim2start + 1);

		newSize >>= 3;
		oldSize >>= 3;

		if (newSize != oldSize)
		{
			printf("Error: redimArray: array %d redim mismatch\n", readVar(arrayId));
			return;
		}

		ah->type = type;
		ah->dim1start = newDim1start;
		ah->dim1end = newDim1end;
		ah->dim2start = newDim2start;
		ah->dim2end = newDim2end;
}

void checkArrayLimits(int array, int dim2start, int dim2end, int dim1start, int dim1end) {
	if (dim1end < dim1start) {
		printf("Error: Across max %d smaller than min %d\n", dim1end, dim1start);
		return;
	}
	if (dim2end < dim2start) {
		printf("Error: Down max %d smaller than min %d\n", dim2end, dim2start);
		return;
	}
	ArrayHeader *ah = _arrays[readVar(array) & ~0x33539000];//(ArrayHeader *)getResourceAddress(rtString, readVar(array));
	//assert(ah);
	if ((int)ah->dim2start > dim2start || (int)ah->dim2end < dim2end || (int)ah->dim1start > dim1start || (int)ah->dim1end < dim1end) {
		printf("Error: Invalid array access (%d,%d,%d,%d) limit (%d,%d,%d,%d)\n", dim2start, dim2end, dim1start, dim1end, ah->dim2start, ah->dim2end, ah->dim1start, ah->dim1end);
		return;
	}
}

void copyArray(int array1, int a1_dim2start, int a1_dim2end, int a1_dim1start, int a1_dim1end,
	int array2, int a2_dim2start, int a2_dim2end, int a2_dim1start, int a2_dim1end)
{
	byte *dst, *src;
	int dstPitch, srcPitch;
	int rowSize;
	checkArrayLimits(array1, a1_dim2start, a1_dim2end, a1_dim1start, a1_dim1end);
	checkArrayLimits(array2, a2_dim2start, a2_dim2end, a2_dim1start, a2_dim1end);
	int a12_num = a1_dim2end - a1_dim2start + 1;
	int a11_num = a1_dim1end - a1_dim1start + 1;
	int a22_num = a2_dim2end - a2_dim2start + 1;
	int a21_num = a2_dim1end - a2_dim1start + 1;
	if (a22_num != a12_num || a21_num != a11_num) {
		printf("Error: Operation size mismatch (%d vs %d)(%d vs %d)\n", a12_num, a22_num, a11_num, a21_num);
		return;
	}

	if (array1 != array2) {
		ArrayHeader *ah1 = _arrays[readVar(array1) & ~0x33539000];// (ArrayHeader *)getResourceAddress(rtString, readVar(array1));
		//assert(ah1);
		ArrayHeader *ah2 = _arrays[readVar(array2) & ~0x33539000];//(ArrayHeader *)getResourceAddress(rtString, readVar(array2));
		//assert(ah2);
		if (ah1->type == ah2->type) {
			copyArrayHelper(ah1, a1_dim2start, a1_dim1start, a1_dim1end, &dst, &dstPitch, &rowSize);
			copyArrayHelper(ah2, a2_dim2start, a2_dim1start, a2_dim1end, &src, &srcPitch, &rowSize);
			for (; a1_dim2start <= a1_dim2end; ++a1_dim2start) {
				memcpy(dst, src, rowSize);
				dst += dstPitch;
				src += srcPitch;
			}
		} else {
			for (; a1_dim2start <= a1_dim2end; ++a1_dim2start, ++a2_dim2start) {
				int a2dim1 = a2_dim1start;
				int a1dim1 = a1_dim1start;
				for (; a1dim1 <= a1_dim1end; ++a1dim1, ++a2dim1) {
					int val = readArray(array2, a2_dim2start, a2dim1);
					writeArray(array1, a1_dim2start, a1dim1, val);
				}
			}
		}
	} else {
		if (a2_dim2start != a1_dim2start || a2_dim1start != a1_dim1start) {
			ArrayHeader *ah = _arrays[readVar(array1) & ~0x33539000];//(ArrayHeader *)getResourceAddress(rtString, readVar(array1));
			//assert(ah);
			if (a2_dim2start > a1_dim2start) {
				copyArrayHelper(ah, a1_dim2start, a1_dim1start, a1_dim1end, &dst, &dstPitch, &rowSize);
				copyArrayHelper(ah, a2_dim2start, a2_dim1start, a2_dim1end, &src, &srcPitch, &rowSize);
			} else {
				// start at the end, so we copy backwards (in case the indices overlap)
				copyArrayHelper(ah, a1_dim2end, a1_dim1start, a1_dim1end, &dst, &dstPitch, &rowSize);
				copyArrayHelper(ah, a2_dim2end, a2_dim1start, a2_dim1end, &src, &srcPitch, &rowSize);
				dstPitch = -dstPitch;
				srcPitch = -srcPitch;
			}
			for (; a1_dim2start <= a1_dim2end; ++a1_dim2start) {
				memcpy(dst, src, rowSize);
				dst += dstPitch;
				src += srcPitch;
			}
		}
	}
}

void copyArrayHelper(ArrayHeader *ah, int idx2, int idx1, int len1, byte **data, int *size, int *num) {
	const int pitch = ah->dim1end - ah->dim1start + 1;
	const int offset = pitch * (idx2 - ah->dim2start) + idx1 - ah->dim1start;

	switch (ah->type) {
	case kByteArray:
	case kStringArray:
		*num = len1 - idx1 + 1;
		*size = pitch;
		*data = ah->data + offset;
		break;
	case kIntArray:
		*num = (len1 - idx1) * 2 + 2;
		*size = pitch * 2;
		*data = ah->data + offset * 2;
		break;
	case kDwordArray:
		*num = (len1 - idx1) * 4 + 4;
		*size = pitch * 4;
		*data = ah->data + offset * 4;
		break;
	default:
		printf("Error: Invalid array type %d\n", ah->type);
	}
}

void shuffleArray(int num, int minIdx, int maxIdx) {
	int range = maxIdx - minIdx;
	int count = range * 2;

	// Shuffle the array 'num'
	while (count--) {
		// Determine two random elements...
		int rand1 = rand() % range + minIdx;
		int rand2 = rand() % range + minIdx;

		// ...and swap them
		int val1 = readArray(num, 0, rand1);
		int val2 = readArray(num, 0, rand2);
		writeArray(num, 0, rand1, val2);
		writeArray(num, 0, rand2, val1);
	}
} 

void getArrayDim(int array, int *dim2start, int *dim2end, int *dim1start, int *dim1end) {
	ArrayHeader *ah = _arrays[readVar(array) & ~0x33539000];//(ArrayHeader *)getResourceAddress(rtString, readVar(array));
	//assert(ah);
	if (dim2start && *dim2start == -1) {
		*dim2start = ah->dim2start;
	}
	if (dim2end && *dim2end == -1) {
		*dim2end = ah->dim2end;
	}
	if (dim1start && *dim1start == -1) {
		*dim1start = ah->dim1start;
	}
	if (dim1end && *dim1end == -1) {
		*dim1end = ah->dim1end;
	}
}

static int sortArrayOffset;

static int compareByteArray(const void *a, const void *b) {
	int va = *((const uint8 *)a + sortArrayOffset);
	int vb = *((const uint8 *)b + sortArrayOffset);
	return va - vb;
}

static int compareByteArrayReverse(const void *a, const void *b) {
	int va = *((const uint8 *)a + sortArrayOffset);
	int vb = *((const uint8 *)b + sortArrayOffset);
	return vb - va;
}

static int compareIntArray(const void *a, const void *b) {
	int va = (int16)(((const uint16*)a)[sortArrayOffset]);
	int vb = (int16)(((const uint16*)b)[sortArrayOffset]);
	return va - vb;
}

static int compareIntArrayReverse(const void *a, const void *b) {
	int va = (int16)(((const uint16*)a)[sortArrayOffset]);
	int vb = (int16)(((const uint16*)b)[sortArrayOffset]);
	return vb - va;
}

static int compareDwordArray(const void *a, const void *b) {
	int va = (int32)(((const uint32*)a)[sortArrayOffset]);
	int vb = (int32)(((const uint32*)b)[sortArrayOffset]);
	return va - vb;
}

static int compareDwordArrayReverse(const void *a, const void *b) {
	int va = (int32)(((const uint32*)a)[sortArrayOffset]);
	int vb = (int32)(((const uint32*)b)[sortArrayOffset]);
	return vb - va;
}


/**
* Sort a row range in a two-dimensional array by the value in a given column.
*
* We sort the data in the row range [dim2start..dim2end], according to the value
* in column dim1start == dim1end.
*/
void sortArray(int array, int dim2start, int dim2end, int dim1start, int dim1end, int sortOrder) {
	//debug(9, "sortArray(%d, [%d,%d,%d,%d], %d)", array, dim2start, dim2end, dim1start, dim1end, sortOrder);

	//assert(dim1start == dim1end);
	checkArrayLimits(array, dim2start, dim2end, dim1start, dim1end);
	ArrayHeader *ah = _arrays[readVar(array) & ~0x33539000];//(ArrayHeader *)getResourceAddress(rtString, readVar(array));
	//assert(ah);

	const int num = dim2end - dim2start + 1;	// number of rows to sort
	const int pitch = ah->dim1end - ah->dim1start + 1;	// length of a row = number of columns in it
	const int offset = pitch * (dim2start - ah->dim2start);	// memory offset to the first row to be sorted
	sortArrayOffset = dim1start - ah->dim1start;	// offset to the column by which we sort

	// Now we just have to invoke qsort on the appropriate row range. We
	// need to pass sortArrayOffset as an implicit parameter to the
	// comparison functions, which makes it necessary to use a global
	// (albeit local to this file) variable.
	// This could be avoided by using qsort_r or a self-written portable
	// analog (this function passes an additional, user determined
	// parameter to the comparison function).
	// Another idea would be to use Common::sort, but that only is
	// suitable if you sort objects of fixed size, which must be known
	// during compilation time; clearly this not the case here.
	switch (ah->type) {
	case kByteArray:
	case kStringArray:
		if (sortOrder <= 0) {
			qsort(ah->data + offset, num, pitch, compareByteArray);
		} else {
			qsort(ah->data + offset, num, pitch, compareByteArrayReverse);
		}
		break;
	case kIntArray:
		if (sortOrder <= 0) {
			qsort(ah->data + offset * 2, num, pitch * 2, compareIntArray);
		} else {
			qsort(ah->data + offset * 2, num, pitch * 2, compareIntArrayReverse);
		}
		break;
	case kDwordArray:
		if (sortOrder <= 0) {
			qsort(ah->data + offset * 4, num, pitch * 4, compareDwordArray);
		} else {
			qsort(ah->data + offset * 4, num, pitch * 4, compareDwordArrayReverse);
		}
		break;
	default:
		printf("Error: Invalid array type %d\n", ah->type);
	}
}

int findObject(int x, int y, int num, int *args)
{
	//printf("findObject(x %d, y %d, num %d, ...)\n", x, y, num);
	int b, cls, i, result;

	for (i = /*1*/0; i < /*_numLocalObjects*/RoomResource->RMDA->RMHD.NrObjects; i++) {
		result = 0;
		if ((RoomResource->RMDA->OBCD[i]->ObjectId < 1) || getClass(RoomResource->RMDA->OBCD[i]->ObjectId, kObjectClassUntouchable))
			continue;

		// Check polygon bounds
		if (polygonDefined(RoomResource->RMDA->OBCD[i]->ObjectId)) {
			if (polygonHit(RoomResource->RMDA->OBCD[i]->ObjectId, x, y))
				result = RoomResource->RMDA->OBCD[i]->ObjectId;
			else if (VAR(VAR_POLYGONS_ONLY))
				continue;
		}

		if (!result) {
			// Check object bounds
			if (RoomResource->RMDA->OBCD[i]->X <= x && RoomResource->RMDA->OBCD[i]->Width + RoomResource->RMDA->OBCD[i]->X > x &&
				RoomResource->RMDA->OBCD[i]->Y <= y && RoomResource->RMDA->OBCD[i]->Height + RoomResource->RMDA->OBCD[i]->Y > y)
				result = RoomResource->RMDA->OBCD[i]->ObjectId;
		}

		if (result) {
			if (!num)
				return result;

			// Check object class
			cls = args[0];
			b = getClass(RoomResource->RMDA->OBCD[i]->ObjectId, cls);
			if ((cls & 0x80 && b) || (!(cls & 0x80) && !b))
				return result;
		}
	}

	return 0;
}

int convertFilePath(byte *dst, int dstSize)
{
	printf("convertFilePath: original filePath is %s\n", dst);

	int len = resStrLen(dst);

	// Switch all \ to / for portablity
	for (int i = 0; i < len; i++)
		if (dst[i] == '\\')
			dst[i] = '/';

	/*if (_game.platform == Common::kPlatformMacintosh) {
	// Remove : prefix in HE71 games
	if (dst[0] == ':') {
	len -= 1;
	memmove(dst, dst + 1, len);
	dst[len] = 0;
	}

	// Switch all : to / for portablity
	for (int i = 0; i < len; i++) {
	if (dst[i] == ':')
	dst[i] = '/';
	}
	}*/

	// Strip path
	int r = 0;
	if (dst[len - 3] == 's' && dst[len - 2] == 'g') { // Save Game File
		// Change filename prefix to target name, for save game files.
		//const char c = dst[len - 1];
		//snprintf((char *)dst, dstSize, "%s.sg%c", _targetName.c_str(), c);
	} else if (dst[0] == '.' && dst[1] == '/') { // Game Data Path
		// The default game data path is set to './' by ScummVM
		r = 2;
	} else if (dst[0] == '*' && dst[1] == '/') { // Save Game Path (Windows HE72 - HE100)
		// The default save game path is set to '*/' by ScummVM
		r = 2;
	} else if (dst[0] == '*' && dst[1] == ':') { // Save Game Path (Macintosh HE72 - HE100)
		// The default save game path is set to ':/' by ScummVM
		r = 2;
	} else if (dst[0] == 'c' && dst[1] == ':') { // Save Game Path (HE60 - HE71)
		// The default save path is game path (DOS) or 'c:/hegames/' (Windows)
		for (r = len; r != 0; r--) {
			if (dst[r - 1] == '/')
				break;
		}
	} else if (dst[0] == 'u' && dst[1] == 's') { // Save Game Path (Moonbase Commander)
		// The default save path is 'user/'
		r = 5;
	}

	printf("convertFilePath: converted filePath is %s\n", dst + r);
	return r;
}


ITCM_CODE static void _0x00_PushByte() {
	push(fetchScriptByte());
}

ITCM_CODE static void _0x01_PushWord() {
	push(fetchScriptWordSigned());
}

ITCM_CODE static void _0x03_PushWordVar()
{
	push(readVar(fetchScriptWord()));
}

ITCM_CODE static void _0x06_ByteArrayRead()
{
	int base = pop();
	push(readArray(fetchScriptByte(), 0, base));
} 

ITCM_CODE static void _0x07_WordArrayRead()
{
	int base = pop();
	push(readArray(fetchScriptWord(), 0, base)); 
}

ITCM_CODE static void _0x0B_WordArrayIndexedRead()
{
	int base = pop();
	int idx = pop();
	push(readArray(fetchScriptWord(), idx, base)); 
}

static void _0x0C_Dup()
{
	int a = pop();
	push(a);
	push(a);
}

static void _0x0D_Not() {
	push(pop() == 0);
}

static void _0x0E_Eq() {
	push(pop() == pop());
} 

static void _0x0F_Neq() {
	push(pop() != pop());
}

static void _0x10_Gt() {
	int a = pop();
	push(pop() > a);
}

static void _0x11_Lt() {
	int a = pop();
	push(pop() < a);
}

static void _0x12_Le()
{
	int a = pop();
	push(pop() <= a);
}

static void _0x13_Ge() {
	int a = pop();
	push(pop() >= a);
}

static void _0x14_Add()
{
	int a = pop();
	push(pop() + a);
}

static void _0x15_Sub()
{
	int a = pop();
	push(pop() - a);
}

static void _0x16_Mul()
{
	int a = pop();
	push(pop() * a);
}

static void _0x17_Div()
{
	int a = pop();
	push(pop() / a);
}

static void _0x18_Land() {
	int a = pop();
	push(pop() && a);
}

static void _0x19_Lor() {
	int a = pop();
	push(pop() || a);
}

static void _0x1A_Pop() {
	pop();
}

static void _0x43_WriteWordVar()
{
	writeVar(fetchScriptWord(), pop());
}

/*static void _0x46_ByteArrayWrite() 
{
int a = pop();
writeArray(fetchScriptByte(), 0, pop(), a); 
}*/

static void _0x47_WordArrayWrite()
{
	int a = pop();
	writeArray(fetchScriptWord(), 0, pop(), a); 
}

static void _0x4B_WordArrayIndexedWrite()
{
	int val = pop();
	int base = pop();
	writeArray(fetchScriptWord(), pop(), base, val); 
}

static void _0x4F_WordVarInc()
{
	int var = fetchScriptWord();
	writeVar(var, readVar(var) + 1);
}

static void _0x53_WordArrayInc()
{
	int var = fetchScriptWord();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) + 1); 
}

static void _0x57_WordVarDec()
{
	int var = fetchScriptWord();
	writeVar(var, readVar(var) - 1); 
}

static void _0x5C_If()
{
	if (pop()) _scriptPointer += fetchScriptWordSigned();
	else fetchScriptWord();
}

static void _0x5D_IfNot()
{
	if (!pop()) _scriptPointer += fetchScriptWordSigned();
	else fetchScriptWord();
}

static void _0x65_StopObjectCode()
{
	stopObjectCode();
}

static void _0x66_StopObjectCode()
{
	stopObjectCode();
}

static void _0x6C_BreakHere()
{
	updateScriptPtr();
	_currentScript = 0xFF;
}

static void _0x6D_IfClassOfIs()
{
	int args[16];
	int num, obj, cls;
	bool b;
	int cond = 1;

	num = getStackList(args, ARRAYSIZE(args));
	obj = pop();

	if (num == 0) {
		push(_classData[obj]);
		return;
	}

	while (--num >= 0) {
		cls = args[num];
		b = getClass(obj, cls);
		if ((cls & 0x80 && !b) || (!(cls & 0x80) && b))
			cond = 0;
	}
	push(cond);
}

static void _0x6E_SetClass()
{
	int args[16];
	int num, obj, cls;

	num = getStackList(args, ARRAYSIZE(args));
	obj = pop();

	while (--num >= 0) {
		cls = args[num];
		if (cls == 0)
			_classData[num] = 0;
		else if (cls & 0x80)
			putClass(obj, cls, 1);
		else
			putClass(obj, cls, 0);
	}
}

static void _0x6F_GetState()
{
	int obj = pop();
	push(getState(obj)); 
}

static void _0x72_GetOwner()
{
	push(getOwner(pop()));
}

static void _0x73_Jump()
{
	_scriptPointer += fetchScriptWordSigned();
}

static void _0x75_StopSound()
{
	int sound = pop();
	printf("Stop Sound: %d\n", sound);
	stopSound(sound);
}

static void _0x7B_LoadRoom()
{
	int room = pop();
	startScene(room, 0, 0);
	//setCameraAt(camera._cur.x, 0);
	//_fullRedraw = true;
}

static void _0x7C_StopScript()
{
	int script = pop();
	if (script == 0)
		stopObjectCode();
	else
		stopScript(script);
}

static void _0x7F_PutActorAtXY()
{
	int room, x, y, act;
	Actor *a;

	room = pop();
	y = pop();
	x = pop();
	act = pop();
	a = &_actors[act];

	if (room == 0xFF || room == 0x7FFFFFFF) {
		room = a->_room;
	} else {
		if (a->_visible && _currentRoom != room && getTalkingActor() == a->_number) {
			stopTalk();
		}
		if (room != 0)
			a->_room = room;
	}
	putActor(a, x, y, room);
}

static void _0x82_AnimateActor()
{
	int anim = pop();
	int act = pop();

	Actor *a = &_actors[act];
	animateActor(a, anim); 
}

static void _0x87_GetRandomNumber()
{
	int rnd = rand() % /*abs(*/(pop() + 1)/*)*/;
	VAR(VAR_RANDOM_NR) = rnd;
	push(rnd); 
}

static void _0x88_GetRandomNumberRange()
{
	int max = pop();
	int min = pop();
	int rnd = rand() % (max - min + 1) + min;
	VAR(VAR_RANDOM_NR) = rnd;
	push(rnd); 
}

static void _0x8B_IsScriptRunning()
{
	push(isScriptRunning(pop()));
}

static void _0x8E_GetObjectY()
{
	int obj = pop();
	push(/*getObjY(pop())*/0);
} 

static void _0x91_GetActorCostume()
{
	Actor *a = &_actors[pop()];//derefActor(pop(), "o6_getActorCostume");
	push(a->_costume); 
}

static void _0x92_FindInventory()
{
	int idx = pop();
	int owner = pop();
	push(/*findInventory(owner, idx)*/0);
} 

static void _0x95_BeginOverride()
{
	beginOverride();
	//_skipVideo = 0;
}

static void _0x96_EndOverride()
{
	endOverride();
}

static void _0x98_IsSoundRunning()
{
	int snd = pop();

	if (snd) snd = isSoundRunning(snd);

	push(snd); 
}

static void _0x9F_GetActorFromXY()
{
	int y = pop();
	int x = pop();
	int r = getActorFromPos(x, y);
	push(r);
} 

static void _0xA1_PseudoRoom()
{
	int list[100];
	int num, a, value;

	num = getStackList(list, ARRAYSIZE(list));
	value = pop();

	while (--num >= 0) {
		a = list[num];
		//if (a > 0x7F)
		//	_resourceMapper[a & 0x7F] = value;
	}
} 

static void _0xA3_GetVerbEntrypoint()
{
	int e = pop();
	int v = pop();
	push(getVerbEntrypoint(v, e));
}

static void _0xA6_DrawBox()
{
	int x, y, x2, y2, color;
	color = pop();
	y2 = pop();
	x2 = pop();
	y = pop();
	x = pop();
	//drawBox(x, y, x2, y2, color);
}

static void _0xA7_Pop()
{
	pop();
}

static void _0xA9_Wait()
{
	int actnum;
	int offs = -2;
	Actor *a;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 168:		// SO_WAIT_FOR_ACTOR Wait for actor
		offs = fetchScriptWordSigned();
		actnum = pop();
		printf("SO_WAIT_FOR_ACTOR\n");
		while(1);
		a = &_actors[actnum];//derefActor(actnum, "o6_wait:168");

		if (a->_moving)
			break;
		return;
	case 169:		// SO_WAIT_FOR_MESSAGE Wait for message
		if (VAR(VAR_HAVE_MSG)) break;
		return;
	case 170:		// SO_WAIT_FOR_CAMERA Wait for camera
		//if (camera._cur.x / 8 != camera._dest.x / 8)
		//	break;
		printf("SO_WAIT_FOR_CAMERA\n");
		while(1);
		return;
	case 171:		// SO_WAIT_FOR_SENTENCE
		/*if (_sentenceNum) {
		if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
		return;
		break;
		}
		if (!isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))*/
		//return;
		printf("SO_WAIT_FOR_SENTENCE\n");
		while(1);
		break;
	case 226:		// SO_WAIT_FOR_ANIMATION
		offs = fetchScriptWordSigned();
		actnum = pop();
		a = &_actors[actnum];//derefActor(actnum, "o6_wait:226");
		if (isInCurrentRoom(a) && a->_needRedraw)
			break;
		printf("SO_WAIT_FOR_ANIMATION\n");
		while(1);
		return;
	case 232:		// SO_WAIT_FOR_TURN
		// WORKAROUND for bug #744441: An angle will often be received as the
		// actor number due to script bugs in The Dig. In all cases where this
		// occurs, _curActor is set just before it, so we can use it instead.
		//
		// For now, if the value passed in is divisible by 45, assume it is an
		// angle, and use _curActor as the actor to wait for.
		offs = fetchScriptWordSigned();
		actnum = pop();
		if (actnum % 45 == 0) {
			actnum = _curActor;
		}
		a = &_actors[actnum];//derefActor(actnum, "o6_wait:232b");
		if (isInCurrentRoom(a) && a->_moving & MF_TURN)
			break;
		printf("SO_WAIT_FOR_TURN\n");
		while(1);
		return;
	default:
		printf("Error: o6_wait: default case 0x%x\n", subOp);
		break;
	}

	_scriptPointer += offs;
	_0x6C_BreakHere();
}

static void _0xAD_IsAnyOf()
{
	int list[100];
	int num;
	int32 val;

	num = getStackList(list, ARRAYSIZE(list));
	val = pop();

	while (--num >= 0) {
		if (list[num] == val) {
			push(1);
			return;
		}
	}

	push(0); 
}

static void _0xB0_Delay()
{
	uint32 delay = (uint16)pop();
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = ssPaused;
	_0x6C_BreakHere();
}

static void _0xB1_DelaySeconds()
{
	uint32 delay = (uint32)pop();
	delay = delay * 60;
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = ssPaused;
	_0x6C_BreakHere(); 
}

static void _0xB3_StopSentence()
{
	//_sentenceNum = 0;
	stopScript(VAR(VAR_SENTENCE_SCRIPT));
	//clearClickedStatus();
}

static void _0xB4_PrintLine() 
{
	_actorToPrintStrFor = 0xFF;
	decodeParseString(0, 0);
}


static void _0xB5_PrintText() 
{
	decodeParseString(1, 0);
}

static void _0xB6_PrintDebug() 
{
	decodeParseString(2, 0);
}

static void _0xB7_PrintSystem() 
{
	decodeParseString(3, 0);
}

static void _0xB8_PrintActor() 
{
	decodeParseString(0, 1);
}

static void _0xB9_PrintEgo() 
{
	push(VAR(VAR_EGO));
	decodeParseString(0, 1);
}

static void _0xBF_StartScriptQuick2()
{
	int args[25];
	int script;
	getStackList(args, ARRAYSIZE(args));
	script = pop();
	runScript(script, 0, 1, args); 
}

static void _0xC4_Abs()
{
	int a = pop();
	push((a >= 0) ? a : -a);
}

static void _0xCA_DelayFrames()
{
	ScriptSlot *ss = &vm.slot[_currentScript];
	if (ss->delayFrameCount == 0) {
		ss->delayFrameCount = pop();
	} else {
		ss->delayFrameCount--;
	}
	if (ss->delayFrameCount) {
		_scriptPointer--;
		_0x6C_BreakHere();
	}
}

static void _0xCB_PickOneOf()
{
	int args[100];
	int i, num;

	num = getStackList(args, ARRAYSIZE(args));
	i = pop();
	if (i < 0 || i > num)
		printf("Error: o6_pickOneOf: %d out of range (0, %d)\n", i, num - 1);
	push(args[i]); 
}

static void _0xCD_StampObject()
{
	int object, x, y, state;

	state = pop();
	y = pop();
	x = pop();
	object = pop();

	if (state == 0)
		state = 1;

	int objnum = getObjectIndex(object);
	if (objnum == -1)
		return;

	if (x != -1) {
		//_objs[objnum].x_pos = x * 8;
		//_objs[objnum].y_pos = y * 8;
	}

	putState(object, state);
	drawObject(objnum, 0);
}

static void _0xD0_GetDateTime()
{
	time_t ut = time(NULL);
	tm* t = localtime(&ut);
	VAR(VAR_TIMEDATE_YEAR) = t->tm_year;
	VAR(VAR_TIMEDATE_MONTH) = t->tm_mon;
	VAR(VAR_TIMEDATE_DAY) = t->tm_mday;
	VAR(VAR_TIMEDATE_HOUR) = t->tm_hour;
	VAR(VAR_TIMEDATE_MINUTE) = t->tm_min; 
}

static void _0xD1_StopTalking()
{
	stopTalk();
}

static void _0xD2_GetAnimateVariable()
{
	int var = pop();
	Actor *a = &_actors[pop()];
	push(getAnimVar(a, var));
}

static void _0xD4_Shuffle()
{
	int b = pop();
	int a = pop();
	shuffleArray(fetchScriptWord(), a, b);
}

static void _0xD6_BAnd()
{
	int a = pop();
	push(pop() & a); 
}

static void _0xD7_BOr()
{
	int a = pop();
	push(pop() | a); 
}

static void _0xE4_SetBoxSet()
{
	int arg = pop() - 1;

	/*const byte *room = getResourceAddress(rtRoom, _roomResource);
	const byte *boxd = NULL, *boxm = NULL;
	int32 dboxSize, mboxSize;
	int i;

	ResourceIterator boxds(room, false);
	for (i = 0; i < arg; i++)
	boxd = boxds.findNext(MKTAG('B','O','X','D'));

	if (!boxd)
	error("ScummEngine_v6::o6_setBoxSet: Can't find dboxes for set %d", arg);

	dboxSize = READ_BE_UINT32(boxd + 4) - 8;
	byte *matrix = _res->createResource(rtMatrix, 2, dboxSize);

	assert(matrix);
	memcpy(matrix, boxd + 8, dboxSize);

	ResourceIterator boxms(room, false);
	for (i = 0; i < arg; i++)
	boxm = boxms.findNext(MKTAG('B','O','X','M'));

	if (!boxm)
	error("ScummEngine_v6::o6_setBoxSet: Can't find mboxes for set %d", arg);

	mboxSize = READ_BE_UINT32(boxm + 4) - 8;
	matrix = _res->createResource(rtMatrix, 1, mboxSize);

	assert(matrix);
	memcpy(matrix, boxm + 8, mboxSize);

	if (_game.version == 7)
	putActors();*/
}

void setupScumm6()
{
	for(int i = 0; i < 256; i++)
	{
		_opcodes[i] = unknownOpcode;
	}
	_opcodes[0x00] = _0x00_PushByte;
	_opcodes[0x01] = _0x01_PushWord;
	_opcodes[0x03] = _0x03_PushWordVar;
	_opcodes[0x06] = _0x06_ByteArrayRead;
	_opcodes[0x07] = _0x07_WordArrayRead;
	_opcodes[0x0B] = _0x0B_WordArrayIndexedRead;
	_opcodes[0x0C] = _0x0C_Dup;
	_opcodes[0x0D] = _0x0D_Not;
	_opcodes[0x0E] = _0x0E_Eq;
	_opcodes[0x0F] = _0x0F_Neq;
	_opcodes[0x10] = _0x10_Gt;
	_opcodes[0x11] = _0x11_Lt;
	_opcodes[0x12] = _0x12_Le;
	_opcodes[0x13] = _0x13_Ge;
	_opcodes[0x14] = _0x14_Add;
	_opcodes[0x15] = _0x15_Sub;
	_opcodes[0x16] = _0x16_Mul;
	_opcodes[0x17] = _0x17_Div;
	_opcodes[0x18] = _0x18_Land;
	_opcodes[0x19] = _0x19_Lor;
	_opcodes[0x1A] = _0x1A_Pop;
	_opcodes[0x43] = _0x43_WriteWordVar;
	_opcodes[0x47] = _0x47_WordArrayWrite;
	_opcodes[0x4B] = _0x4B_WordArrayIndexedWrite;
	_opcodes[0x4F] = _0x4F_WordVarInc;
	_opcodes[0x53] = _0x53_WordArrayInc;
	_opcodes[0x57] = _0x57_WordVarDec;
	_opcodes[0x5C] = _0x5C_If;
	_opcodes[0x5D] = _0x5D_IfNot;
	_opcodes[0x65] = _0x65_StopObjectCode;
	_opcodes[0x66] = _0x66_StopObjectCode;
	_opcodes[0x6C] = _0x6C_BreakHere;
	_opcodes[0x6D] = _0x6D_IfClassOfIs;
	_opcodes[0x6E] = _0x6E_SetClass;
	_opcodes[0x6F] = _0x6F_GetState;
	_opcodes[0x72] = _0x72_GetOwner;
	_opcodes[0x73] = _0x73_Jump;
	_opcodes[0x75] = _0x75_StopSound;
	_opcodes[0x7B] = _0x7B_LoadRoom;
	_opcodes[0x7C] = _0x7C_StopScript;
	_opcodes[0x7F] = _0x7F_PutActorAtXY;
	_opcodes[0x82] = _0x82_AnimateActor;
	_opcodes[0x87] = _0x87_GetRandomNumber;
	_opcodes[0x88] = _0x88_GetRandomNumberRange;
	_opcodes[0x8B] = _0x8B_IsScriptRunning;
	_opcodes[0x8E] = _0x8E_GetObjectY;
	_opcodes[0x91] = _0x91_GetActorCostume;
	_opcodes[0x92] = _0x92_FindInventory;
	_opcodes[0x95] = _0x95_BeginOverride;
	_opcodes[0x96] = _0x96_EndOverride;
	_opcodes[0x98] = _0x98_IsSoundRunning;
	_opcodes[0x9F] = _0x9F_GetActorFromXY;
	_opcodes[0xA1] = _0xA1_PseudoRoom;
	_opcodes[0xA3] = _0xA3_GetVerbEntrypoint;
	_opcodes[0xA6] = _0xA6_DrawBox;
	_opcodes[0xA7] = _0xA7_Pop;
	_opcodes[0xA9] = _0xA9_Wait;
	_opcodes[0xAD] = _0xAD_IsAnyOf;
	_opcodes[0xB0] = _0xB0_Delay;
	_opcodes[0xB1] = _0xB1_DelaySeconds;
	_opcodes[0xB3] = _0xB3_StopSentence;
	_opcodes[0xB4] = _0xB4_PrintLine;
	_opcodes[0xB5] = _0xB5_PrintText;
	_opcodes[0xB6] = _0xB6_PrintDebug;
	_opcodes[0xB7] = _0xB7_PrintSystem;
	_opcodes[0xB8] = _0xB8_PrintActor;
	_opcodes[0xB9] = _0xB9_PrintEgo;
	_opcodes[0xBF] = _0xBF_StartScriptQuick2;
	_opcodes[0xC4] = _0xC4_Abs;
	_opcodes[0xCA] = _0xCA_DelayFrames;
	_opcodes[0xCB] = _0xCB_PickOneOf;
	_opcodes[0xCD] = _0xCD_StampObject;
	_opcodes[0xD0] = _0xD0_GetDateTime;
	_opcodes[0xD1] = _0xD1_StopTalking;
	_opcodes[0xD2] = _0xD2_GetAnimateVariable;
	_opcodes[0xD4] = _0xD4_Shuffle;
	_opcodes[0xD6] = _0xD6_BAnd;
	_opcodes[0xD7] = _0xD7_BOr;
	_opcodes[0xE4] = _0xE4_SetBoxSet;
}