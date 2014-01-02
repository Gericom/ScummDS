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
				//dst += sprintf((char *)dst, "%b", args[val++]);
				break;
			case 'c':
				*dst++ = args[val++];
				break;
			case 'd':
				dst += sprintf((char *)dst, "%d", args[val++]);
				break;
			case 's':
				src = (byte*)_arrays[args[val++]]->data;//getStringAddress(args[val++]);
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
		printf("actorTalk:\n%s\n", msg);
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
	return -1;
}

static const int arrayDataSizes[] = { 0, 1, 4, 8, 8, 16, 32 };

byte *defineArray(int array, int type, int dim2start, int dim2end, int dim1start, int dim1end) {
		int id;
		int size;
		ArrayHeader *ah;

		//assert(dim2start >= 0 && dim2start <= dim2end);
		//assert(dim1start >= 0 && dim1start <= dim1end);
		//assert(0 <= type && type <= 6);


		if (type == kBitArray || type == kNibbleArray)
			type = kByteArray;

		nukeArray(array);

		id = findFreeArrayId();

		//printf("defineArray (array %d, dim2start %d, dim2end %d dim1start %d dim1end %d\n", id, dim2start, dim2end, dim1start, dim1end);

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

		ah = _arrays[id];//(ArrayHeader *)_res->createResource(rtString, id, size + sizeof(ArrayHeader));

		ah->type = type;
		ah->dim1start = dim1start;
		ah->dim1end = dim1end;
		ah->dim2start = dim2start;
		ah->dim2end = dim2end;

		return ah->data;
}

int readArray(int array, int idx2, int idx1) {
	//printf("readArray (array %d, idx2 %d, idx1 %d)\n", readVar(array), idx2, idx1);

	if (readVar(array) == 0)
	{
		printf("Error: readArray: Reference to zeroed array pointer\n");
		return 0;
	}

	ArrayHeader *ah = _arrays[readVar(array) & ~0x33539000];//(ArrayHeader *)getResourceAddress(rtString, readVar(array));

	if (!ah)
	{
		printf("Error: readArray: invalid array %d (%d)\n", array, readVar(array));
		return 0;
	}

	if (idx2 < (int)ah->dim2start || idx2 > (int)ah->dim2end ||
		idx1 < (int)ah->dim1start || idx1 > (int)ah->dim1end) {
			printf("Error: readArray: array %d out of bounds: [%d, %d] exceeds [%d..%d, %d..%d]\n",
				array, idx1, idx2, ah->dim1start, ah->dim1end,
				ah->dim2start, ah->dim2end);
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
	//printf("writeArray (array %d, idx2 %d, idx1 %d, value %d)\n", readVar(array), idx2, idx1, value);

	if (readVar(array) == 0)
	{
		printf("Error: writeArray: Reference to zeroed array pointer\n");
		return;
	}

	ArrayHeader *ah = _arrays[readVar(array) & ~0x33539000];//(ArrayHeader *)getResourceAddress(rtString, readVar(array));

	if (!ah)
	{
		printf("Error: writeArray: Invalid array (%d) reference\n", readVar(array));
		return;
	}

	if (idx2 < (int)ah->dim2start || idx2 > (int)ah->dim2end ||
		idx1 < (int)ah->dim1start || idx1 > (int)ah->dim1end) {
			printf("Error: writeArray: array %d out of bounds: [%d, %d] exceeds [%d..%d, %d..%d]\n",
				array, idx1, idx2, ah->dim1start, ah->dim1end,
				ah->dim2start, ah->dim2end);
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
			printf("Error: redimArray: Reference to zeroed array pointer\n");

		ArrayHeader *ah = _arrays[readVar(arrayId) & ~0x33539000];//(ArrayHeader *)getResourceAddress(rtString, readVar(arrayId));

		if (!ah)
			printf("Error: redimArray: Invalid array (%d) reference\n", readVar(arrayId));

		newSize = arrayDataSizes[type];
		oldSize = arrayDataSizes[ah->type];

		newSize *= (newDim1end - newDim1start + 1) * (newDim2end - newDim2start + 1);
		oldSize *= (ah->dim1end - ah->dim1start + 1) *
			(ah->dim2end - ah->dim2start + 1);

		newSize >>= 3;
		oldSize >>= 3;

		if (newSize != oldSize)
			printf("Error: redimArray: array %d redim mismatch\n", readVar(arrayId));

		ah->type = type;
		ah->dim1start = newDim1start;
		ah->dim1end = newDim1end;
		ah->dim2start = newDim2start;
		ah->dim2end = newDim2end;
}

void checkArrayLimits(int array, int dim2start, int dim2end, int dim1start, int dim1end) {
	if (dim1end < dim1start) {
		printf("Error: Across max %d smaller than min %d\n", dim1end, dim1start);
	}
	if (dim2end < dim2start) {
		printf("Error: Down max %d smaller than min %d\n", dim2end, dim2start);
	}
	ArrayHeader *ah = _arrays[readVar(array) & ~0x33539000];//(ArrayHeader *)getResourceAddress(rtString, readVar(array));
	//assert(ah);
	if ((int)ah->dim2start > dim2start || (int)ah->dim2end < dim2end || (int)ah->dim1start > dim1start || (int)ah->dim1end < dim1end) {
		printf("Error: Invalid array access (%d,%d,%d,%d) limit (%d,%d,%d,%d)\n", dim2start, dim2end, dim1start, dim1end, ah->dim2start, ah->dim2end, ah->dim1start, ah->dim1end);
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

void _0x00_PushByte() {
	push(fetchScriptByte());
}

void _0x01_PushWord() {
	push(fetchScriptWordSigned());
}

void _0x02_PushDWord()
{
	push(fetchScriptDWordSigned());
}

void _0x03_PushWordVar()
{
	push(readVar(fetchScriptWord()));
}

void _0x04_GetScriptString()
{
	byte chr;

	while ((chr = fetchScriptByte()) != 0) {
		_stringBuffer[_stringLength] = chr;
		_stringLength++;

		if (_stringLength >= 4096)
			printf("Error: String stack overflow\n");
	}

	_stringBuffer[_stringLength] = 0;
	_stringLength++;
}

void _0x07_WordArrayRead()
{
	int base = pop();
	push(readArray(fetchScriptWord(), 0, base)); 
}

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

void _0x0B_WordArrayIndexedRead()
{
	int base = pop();
	int idx = pop();
	push(readArray(fetchScriptWord(), idx, base)); 
}

void _0x0C_Dup()
{
	int a = pop();
	push(a);
	push(a);
}

void _0x0D_Not() {
	push(pop() == 0);
}

void _0x0E_Eq() {
	push(pop() == pop());
} 

void _0x0F_Neq() {
	push(pop() != pop());
}

void _0x10_Gt() {
	int a = pop();
	push(pop() > a);
}

void _0x11_Lt() {
	int a = pop();
	push(pop() < a);
}

void _0x12_Le()
{
	int a = pop();
	push(pop() <= a);
}

void _0x13_Ge() {
	int a = pop();
	push(pop() >= a);
}

void _0x14_Add()
{
	int a = pop();
	push(pop() + a);
}

void _0x15_Sub()
{
	int a = pop();
	push(pop() - a);
}

void _0x16_Mul()
{
	int a = pop();
	push(pop() * a);
}

void _0x17_Div()
{
	int a = pop();
	push(pop() / a);
}

void _0x18_Land() {
	int a = pop();
	push(pop() && a);
}

void _0x19_Lor() {
	int a = pop();
	push(pop() || a);
}

void _0x1A_Pop() {
	pop();
}

void _0x1B_IsAnyOf()
{
	int args[128];
	int num, value;

	num = getStackList(args, ARRAYSIZE(args));
	value = pop();

	for (int i = 0; i < num; i++) {
		if (args[i] == value) {
			push(1);
			return;
		}
	}

	push(0);
}

void _0x1C_WizImageOps()
{
	int a, b;

	int subOp = fetchScriptByte();
	printf("Wiz (%d)\n", subOp);
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

void _0x43_WriteWordVar()
{
	writeVar(fetchScriptWord(), pop());
}

void _0x4B_WordArrayIndexedWrite()
{
	int val = pop();
	int base = pop();
	writeArray(fetchScriptWord(), pop(), base, val); 
}

void _0x46_ByteArrayWrite() 
{
	int a = pop();
	writeArray(fetchScriptByte(), 0, pop(), a); 
}

void _0x47_WordArrayWrite()
{
	int a = pop();
	writeArray(fetchScriptWord(), 0, pop(), a); 
}

void _0x4F_WordVarInc()
{
	int var = fetchScriptWord();
	writeVar(var, readVar(var) + 1);
}

void _0x53_WordArrayInc()
{
	int var = fetchScriptWord();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) + 1); 
}

void _0x57_WordVarDec()
{
	int var = fetchScriptWord();
	writeVar(var, readVar(var) - 1); 
}

void _0x58_GetTimer()
{
	int timer = pop();
	byte cmd = fetchScriptByte();

	if (cmd == 10 || cmd == 50) {
		push(getHETimer(timer));
	} else {
		push(0);
	}
}

void _0x59_SetTimer()
{
	int timer = pop();
	byte cmd = fetchScriptByte();

	if (cmd == 158 || cmd == 61) {
		setHETimer(timer);
	} else {
		printf("Error: TIMER command %d?\n", cmd);
	} 
}

void _0x5C_If()
{
	if (pop()) _0x73_Jump();
	else fetchScriptWord();
}

void _0x5D_IfNot()
{
	if (!pop()) _0x73_Jump();
	else fetchScriptWord();
}

void _0x5E_StartScript()
{
	int args[25];
	int script;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();

	runScript(script, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args); 
}

void _0x61_DrawObject()
{
	byte subOp = fetchScriptByte();
	int state, y, x;

	switch (subOp) {
	case 62:
		state = pop();
		y = pop();
		x = pop();
		break;
	case 63:
		state = pop();
		if (state == 0)
			state = 1;
		y = x = -100;
		break;
	case 65:
		state = 1;
		y = pop();
		x = pop();
		break;
	default:
		printf("Error: o72_drawObject: default case %d\n", subOp);
	}

	int object = pop();
	int objnum = getObjectIndex(object);
	if (objnum == -1)
	return;

	if (y != -100 && x != -100) {
		//_objs[objnum].x_pos = x * 8;
		//_objs[objnum].y_pos = y * 8;
	}

	if (state != -1) {
		//addObjectToDrawQue(objnum);
		//putState(object, state);
	}
}

void _0x64_GetNumFreeArrays()
{
	int i, num = 0;

	for (i = 1; i < _numArray; i++) {
		if (_arrays[i] == NULL)
			num++;
	}

	push(num);
}

void _0x65_StopObjectCode()
{
	stopObjectCode();
}

void _0x66_StopObjectCode()
{
	stopObjectCode();
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
		printf("Cursor!\n");
		//while(scanKeys(), keysHeld() == 0);
		//swiDelay(5000000);
		break;
	case 0x3C:
		b = pop();
		a = pop();
		loadWizCursor(a, b);
		printf("Cursor!\n");
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

void _0x6C_BreakHere()
{
	updateScriptPtr();
	_currentScript = 0xFF;
}

void _0x6D_IfClassOfIs()
{
	int args[16];
	int num, obj, cls;
	bool b;
	int cond = 1;

	num = getStackList(args, ARRAYSIZE(args));
	obj = pop();

	if (num == 0) {
		push(/*_classData[obj]*/0);
		return;
	}

	while (--num >= 0) {
		cls = args[num];
		b = 0;//getClass(obj, cls);
		if ((cls & 0x80 && !b) || (!(cls & 0x80) && b))
			cond = 0;
	}
	push(cond);
}

void _0x6E_SetClass()
{
	int args[16];
	int num, obj, cls;

	num = getStackList(args, ARRAYSIZE(args));
	obj = pop();

	/*while (--num >= 0) {
	cls = args[num];
	if (cls == 0)
	_classData[num] = 0;
	else if (cls & 0x80)
	putClass(obj, cls, 1);
	else
	putClass(obj, cls, 0);
	} */
}

void _0x6F_GetState()
{
	int obj = pop();
	push(/*getState(obj)*/0); 
}

void _0x70_SetState()
{
	int state = pop();
	int obj = pop();

	//putState(obj, state);
	//markObjectRectAsDirty(obj);
	//if (_bgNeedsRedraw)
	//	clearDrawObjectQueue();
}

void _0x73_Jump()
{
	_scriptPointer += fetchScriptWordSigned();
}

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

void _0x75_StopSound()
{
	int sound = pop();
	printf("Stop Sound: %d\n", sound);
	stopSound(sound);
}

void _0x7B_LoadRoom()
{
	int room = pop();
	startScene(room, 0, 0);
	//setCameraAt(camera._cur.x, 0);
	//_fullRedraw = true;
}

void _0x7C_StopScript()
{
	int script = pop();
	if (script == 0)
		stopObjectCode();
	else
		stopScript(script);
}

void _0x7F_PutActorAtXY()
{
	int room, x, y, act;
	Actor *a;

	room = pop();
	y = pop();
	x = pop();
	act = pop();
	a = &_actors[act];//derefActor(act, "o6_putActorAtXY");

	if (room == 0xFF || room == 0x7FFFFFFF) {
		room = a->_room;
	} else {
		//if (a->_visible && _currentRoom != room && getTalkingActor() == a->_number) {
		//stopTalk();
		//}
		if (room != 0)
			a->_room = room;
	}
	putActor(a, x, y, room);
}

void _0x82_AnimateActor()
{
	int anim = pop();
	int act = pop();

	Actor *a = &_actors[act];//derefActor(act, "o6_animateActor");
	animateActor(a, anim); 
}

void _0x87_GetRandomNumber()
{
	int rnd = rand() % abs(pop());
	VAR(VAR_RANDOM_NR) = rnd;
	push(rnd); 
}

void _0x88_GetRandomNumberRange()
{
	int max = pop();
	int min = pop();
	int rnd = rand() % (max - min) + min;
	VAR(VAR_RANDOM_NR) = rnd;
	push(rnd); 
}

void _0x8B_IsScriptRunning()
{
	push(isScriptRunning(pop()));
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

	Actor *a = &_actors[act];//derefActor(act, "o6_getActorRoom");
	push(a->_room); 
}

void _0x8E_GetObjectY()
{
	int obj = pop();
	push(/*getObjY(pop())*/0);
} 

void _0x91_GetActorCostume()
{
	Actor *a = &_actors[pop()];//derefActor(pop(), "o6_getActorCostume");
	push(a->_costume); 
}

void _0x92_FindInventory()
{
	int idx = pop();
	int owner = pop();
	push(/*findInventory(owner, idx)*/0);
} 

void _0x95_BeginOverride()
{
	beginOverride();
	//_skipVideo = 0;
}

void _0x96_EndOverride()
{
	endOverride();
}

void _0x98_IsSoundRunning()
{
	int snd = pop();

	if (snd) snd = isSoundRunning(snd);

	push(snd); 
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

void _0x9C_RoomOps()
{
	int a, b, c, d, e;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 172:		// SO_ROOM_SCROLL
		b = pop();
		a = pop();
		/*if (a < (_screenWidth / 2))
		a = (_screenWidth / 2);
		if (b < (_screenWidth / 2))
		b = (_screenWidth / 2);
		if (a > _roomWidth - (_screenWidth / 2))
		a = _roomWidth - (_screenWidth / 2);
		if (b > _roomWidth - (_screenWidth / 2))
		b = _roomWidth - (_screenWidth / 2);
		VAR(VAR_CAMERA_MIN_X) = a;
		VAR(VAR_CAMERA_MAX_X) = b;*/
		break;

	case 174:		// SO_ROOM_SCREEN
		b = pop();
		a = pop();
		//initScreens(a, _screenHeight);
		break;

	case 175:		// SO_ROOM_PALETTE
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		//setPalColor(d, a, b, c);
		break;

	case 179:		// SO_ROOM_INTENSITY
		c = pop();
		b = pop();
		a = pop();
		//darkenPalette(a, a, a, b, c);
		break;

	case 180:		// SO_ROOM_SAVEGAME
		pop();
		pop();
		//_saveTemporaryState = true;
		//_saveLoadSlot = pop();
		//_saveLoadFlag = pop();
		break;

	case 181:		// SO_ROOM_FADE
		// Defaults to 1 but doesn't use fade effects
		a = pop();
		break;

	case 182:		// SO_RGB_ROOM_INTENSITY
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		//darkenPalette(a, b, c, d, e);
		break;

	case 213:		// SO_ROOM_NEW_PALETTE
		a = pop();
		//setCurrentPalette(a);
		break;

	case 220:
		a = pop();
		b = pop();
		//copyPalColor(a, b);
		break;

	case 221:
		byte buffer[256];

		copyScriptString((byte *)buffer, sizeof(buffer));

		/*_saveLoadFileName = (char *)buffer + convertFilePath(buffer, sizeof(buffer));
		printf("o72_roomOps: case 221: filename %s\n", _saveLoadFileName.c_str());

		_saveLoadFlag = pop();
		_saveLoadSlot = 255;
		_saveTemporaryState = true;*/
		break;

	case 234:
		b = pop();
		a = pop();
		//swapObjects(a, b);
		break;

	case 236:
		b = pop();
		a = pop();
		//setRoomPalette(a, b);
		break;

	default:
		printf("Error: o72_roomOps: default case %d\n", subOp);
	} 
}

void _0x9D_ActorOps()
{
	Actor* a;
	int i, j, k;
	int args[32];
	byte string[256];

	byte subOp = fetchScriptByte();
	if (subOp == 197) {
		_curActor = pop();
		return;
	}

	a = &_actors[_curActor];//(ActorHE *)derefActorSafe(_curActor, "o72_actorOps");
	//if (!a)
	//	return;

	switch (subOp) {
	case 21: // HE 80+
		k = getStackList(args, ARRAYSIZE(args));
		//for (i = 0; i < k; ++i) {
		//	a->setUserCondition(args[i] & 0x7F, args[i] & 0x80);
		//}
		break;
	case 24: // HE 80+
		k = pop();
		//if (k == 0)
		//	k = _rnd.getRandomNumberRng(1, 10);
		a->_heNoTalkAnimation = 1;
		//a->setTalkCondition(k);
		break;
	case 43: // HE 90+
		a->_layer = pop();
		a->_needRedraw = true;
		break;
	case 64:
		pop();
		pop();
		pop();
		pop();
		//_actorClipOverride.bottom = pop();
		//_actorClipOverride.right = pop();
		//_actorClipOverride.top = pop();
		//_actorClipOverride.left = pop();
		//adjustRect(_actorClipOverride);
		break;
	case 65: // HE 98+
		j = pop();
		i = pop();
		putActor(a, i, j);
		break;
	case 67: // HE 99+
		pop();
		pop();
		pop();
		pop();
		//a->_clipOverride.bottom = pop();
		//a->_clipOverride.right = pop();
		//a->_clipOverride.top = pop();
		//a->_clipOverride.left = pop();
		//adjustRect(a->_clipOverride);
		break;
	case 68: // HE 90+
		k = pop();
		//a->setHEFlag(1, k);
		break;
	case 76:		// SO_COSTUME
		setActorCostume(a, pop());
		break;
	case 77:		// SO_STEP_DIST
		j = pop();
		i = pop();
		//a->setActorWalkSpeed(i, j);
		break;
	case 78:		// SO_SOUND
		//printf("SO_SOUND:\n");
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; i++)
		{
			a->_sound[i] = args[i];
			//printf("%d\n", args[i]);
		}
		//while(scanKeys(), keysHeld() == 0);
		//swiDelay(5000000);
		break;
	case 79:		// SO_WALK_ANIMATION
		a->_walkFrame = pop();
		break;
	case 80:		// SO_TALK_ANIMATION
		a->_talkStopFrame = pop();
		a->_talkStartFrame = pop();
		break;
	case 81:		// SO_STAND_ANIMATION
		a->_standFrame = pop();
		break;
	case 82:		// SO_ANIMATION
		// dummy case in scumm6
		pop();
		pop();
		pop();
		break;
	case 83:		// SO_DEFAULT
		initActor(a, 0);
		break;
	case 84:		// SO_ELEVATION
		setElevation(a, pop());
		break;
	case 85:		// SO_ANIMATION_DEFAULT
		a->_initFrame = 1;
		a->_walkFrame = 2;
		a->_standFrame = 3;
		a->_talkStartFrame = 4;
		a->_talkStopFrame = 5;
		break;
	case 86:		// SO_PALETTE
		j = pop();
		i = pop();
		//assertRange(0, i, 255, "palette slot");
		//a->remapActorPaletteColor(i, j);
		a->_needRedraw = true;
		break;
	case 87:		// SO_TALK_COLOR
		a->_talkColor = pop();
		break;
	case 88:		// SO_ACTOR_NAME
		copyScriptString(string, sizeof(string));
		//loadPtrToResource(rtActorName, a->_number, string);
		break;
	case 89:		// SO_INIT_ANIMATION
		a->_initFrame = pop();
		break;
	case 91:		// SO_ACTOR_WIDTH
		a->_width = pop();
		break;
	case 92:		// SO_SCALE
		i = pop();
		setScale(a, i, i);
		break;
	case 93:		// SO_NEVER_ZCLIP
		a->_forceClip = 0;
		break;
	case 94:		// SO_ALWAYS_ZCLIP
		a->_forceClip = pop();
		break;
	case 95:		// SO_IGNORE_BOXES
		a->_ignoreBoxes = 1;
		a->_forceClip = 0;
		if (isInCurrentRoom(a))
			putActor(a);
		break;
	case 96:		// SO_FOLLOW_BOXES
		a->_ignoreBoxes = 0;
		a->_forceClip = 0;
		if (isInCurrentRoom(a))
			putActor(a);
		break;
	case 97:		// SO_ANIMATION_SPEED
		setAnimSpeed(a, pop());
		break;
	case 98:		// SO_SHADOW
		a->_heXmapNum = pop();
		a->_needRedraw = true;
		break;
	case 99:		// SO_TEXT_OFFSET
		a->_talkPosY = pop();
		a->_talkPosX = pop();
		break;
	case 156:		// HE 72+
		a->_charset = pop();
		break;
	case 175:		// HE 99+
		a->_hePaletteNum = pop();
		a->_needRedraw = true;
		break;
	case 198:		// SO_ACTOR_VARIABLE
		i = pop();
		setAnimVar(a, pop(), i);
		break;
	case 215:		// SO_ACTOR_IGNORE_TURNS_ON
		a->_ignoreTurns = true;
		break;
	case 216:		// SO_ACTOR_IGNORE_TURNS_OFF
		a->_ignoreTurns = false;
		break;
	case 217:		// SO_ACTOR_NEW
		initActor(a, 2);
		break;
	case 218:
		//a->drawActorToBackBuf(a->getPos().x, a->getPos().y);
		break;
	case 219:
		a->_drawToBackBuf = false;
		a->_needRedraw = true;
		a->_needBgReset = true;
		break;
	case 225:
		{
			copyScriptString(string, sizeof(string));
			int slot = pop();

			printf("Actor: %s\n", string);

			int len = resStrLen(string) + 1;
			memcpy(a->_heTalkQueue[slot].sentence, string, len);

			a->_heTalkQueue[slot].posX = a->_talkPosX;
			a->_heTalkQueue[slot].posY = a->_talkPosY;
			a->_heTalkQueue[slot].color = a->_talkColor;
			break;
		}
	default:
		printf("Error: o72_actorOps: default case %d\n", subOp);
	}
}

void _0x9F_GetActorFromXY()
{
	int y = pop();
	int x = pop();
	int r = getActorFromPos(x, y);
	push(r);
}  

void _0xA0_FindObject() 
{
	int y = pop();
	int x = pop();
	int r = 0;//findObject(x, y, 0, 0);
	push(r);
} 

void _0xA1_PseudoRoom()
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

void _0xA4_ArrayOps()
{
	byte *data;
	byte string[1024];
	int dim1end, dim1start, dim2end, dim2start;
	int id, len, b, c, list[128];
	int offs, tmp, tmp2;
	uint tmp3;

	byte subOp = fetchScriptByte();
	int array = fetchScriptWord();
	printf("o72_arrayOps: array %d case %d\n", array, subOp);

	switch (subOp) {
	case 7:			// SO_ASSIGN_STRING
		copyScriptString(string, sizeof(string));
		len = resStrLen(string);
		data = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(data, string, len);
		break;

	case 126:
		len = getStackList(list, ARRAYSIZE(list));
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, dim2start, dim2end, dim1start, dim1end);
		}
		checkArrayLimits(array, dim2start, dim2end, dim1start, dim1end);

		tmp2 = 0;
		while (dim2start <= dim2end) {
			tmp = dim1start;
			while (tmp <= dim1end) {
				writeArray(array, dim2start, tmp, list[tmp2++]);
				if (tmp2 == len)
					tmp2 = 0;
				tmp++;
			}
			dim2start++;
		}
		break;
	case 127:
		{
			int a2_dim1end = pop();
			int a2_dim1start = pop();
			int a2_dim2end = pop();
			int a2_dim2start = pop();
			int array2 = fetchScriptWord();
			int a1_dim1end = pop();
			int a1_dim1start = pop();
			int a1_dim2end = pop();
			int a1_dim2start = pop();
			if (a1_dim1end - a1_dim1start != a2_dim1end - a2_dim1start || a2_dim2end - a2_dim2start != a1_dim2end - a1_dim2start) {
				printf("Error: Source and dest ranges size are mismatched\n");
			}
			copyArray(array, a1_dim2start, a1_dim2end, a1_dim1start, a1_dim1end, array2, a2_dim2start, a2_dim2end, a2_dim1start, a2_dim1end);
		}
		break;
	case 128:
		b = pop();
		c = pop();
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, dim2start, dim2end, dim1start, dim1end);
		}
		checkArrayLimits(array, dim2start, dim2end, dim1start, dim1end);

		offs = (b >= c) ? 1 : -1;
		tmp2 = c;
		tmp3 = c - b + 1;
		while (dim2start <= dim2end) {
			tmp = dim1start;
			while (tmp <= dim1end) {
				writeArray(array, dim2start, tmp, tmp2);
				if (--tmp3 == 0) {
					tmp2 = c;
					tmp3 = c - b + 1;
				} else {
					tmp2 += offs;
				}
				tmp++;
			}
			dim2start++;
		}
		break;
	case 194:
		decodeScriptString(string);
		len = resStrLen(string);
		data = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(data, string, len);
		break;
	case 208:		// SO_ASSIGN_INT_LIST
		b = pop();
		c = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, 0, 0, 0, b + c - 1);
		}
		while (c--) {
			writeArray(array, 0, b + c, pop());
		}
		break;
	case 212:		// SO_ASSIGN_2DIM_LIST
		printf("SO_ASSIGN_2DIM_LIST\n");
		len = getStackList(list, ARRAYSIZE(list));
		id = readVar(array);
		if (id == 0) printf("Error: Must DIM a two dimensional array before assigning\n");
		c = pop();
		while (--len >= 0) {
			writeArray(array, c, len, list[len]);
		}
		break;
	default:
		printf("Error: o72_arrayOps: default case %d (array %d)\n", subOp, array);
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

void _0xA6_DrawBox()
{
	int x, y, x2, y2, color;
	color = pop();
	y2 = pop();
	x2 = pop();
	y = pop();
	x = pop();
	//drawBox(x, y, x2, y2, color);
}

void _0xA7_Pop()
{
	pop();
}

void _0xA9_Wait()
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
		//printf("VAR(VAR_HAVE_MSG) = %X\n", VAR(VAR_HAVE_MSG));
		if (VAR(VAR_HAVE_MSG))
		{
			if(_haveMsg >= 64) _haveMsg-=64;
			else _haveMsg = 0;
			
			break;
		}
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

void _0xAD_IsAnyOf()
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

void _0xAE_SystemOps()
{
	byte string[1024];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 22: // HE80+
		printf("clearDrawObjectQueue\n");
		InitFrameBuffers();
		//while(1);
		//clearDrawObjectQueue();
		break;
	case 26: // HE80+
		printf("restoreBackgroundHE\n");
		InitFrameBuffers();
		//while(1);
		//restoreBackgroundHE(Common::Rect(_screenWidth, _screenHeight));
		//updatePalette();
		break;
	case 158:
		printf("Restart\n");
		while(1);
		//restart();
		break;
	case 160:
		// Confirm shutdown
		printf("Confirm Shutdown\n");
		while(1);
		//confirmExitDialog();
		break;
	case 244:
		printf("Quit\n");
		while(1);
		//quitGame();
		break;
	case 251:
		copyScriptString(string, sizeof(string));
		printf("Start executable (%s)\n", string);
		while(1);
		break;
	case 252:
		copyScriptString(string, sizeof(string));
		printf("Start game (%s)\n", string);
		while(1);
		break;
	default:
		printf("Error: o72_systemOps invalid case %d\n", subOp);
		while(1);
		break;
	}
}

void _0xB0_Delay()
{
	uint32 delay = (uint16)pop();
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = ssPaused;
	_0x6C_BreakHere();
}

void _0xB1_DelaySeconds()
{
	uint32 delay = (uint32)pop();
	delay = delay * 60;
	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = ssPaused;
	_0x6C_BreakHere(); 
}

void _0xB3_StopSentence()
{
	//_sentenceNum = 0;
	stopScript(VAR(VAR_SENTENCE_SCRIPT));
	//clearClickedStatus();
}

void _0xB4_PrintLine() 
{
	_actorToPrintStrFor = 0xFF;
	decodeParseString(0, 0);
}


void _0xB5_PrintText() 
{
	decodeParseString(1, 0);
}

void _0xB6_PrintDebug() 
{
	decodeParseString(2, 0);
}

void _0xB7_PrintSystem() 
{
	decodeParseString(3, 0);
}

void _0xB8_PrintActor() 
{
	decodeParseString(0, 1);
}

void _0xB9_PrintEgo() 
{
	push(VAR(VAR_EGO));
	decodeParseString(0, 1);
}

void _0xBC_DimArray()
{
	int data;

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
	case 204:		// SO_UNDIM_ARRAY
		nukeArray(fetchScriptWord());
		return;
	default:
		printf("Error: o72_dimArray: default case %d\n", subOp);
	}

	defineArray(fetchScriptWord(), data, 0, 0, 0, pop());
}

void _0xBD_StopObjectCode()
{
	stopObjectCode();
}

void _0xBF_StartScriptQuick2()
{
	int args[25];
	int script;
	getStackList(args, ARRAYSIZE(args));
	script = pop();
	runScript(script, 0, 1, args); 
}

void _0xC0_Dim2DimArray()
{
	int data, dim1end, dim2end;

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
		printf("Error: o72_dim2dimArray: default case %d\n", subOp);
	}

	dim1end = pop();
	dim2end = pop();
	defineArray(fetchScriptWord(), data, 0, dim2end, 0, dim1end); 
}

void _0xC1_TraceStatus()
{
	byte string[80];

	copyScriptString(string, sizeof(string));
	pop();
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

void _0xCA_DelayFrames()
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

void _0xCB_PickOneOf()
{
	int args[100];
	int i, num;

	num = getStackList(args, ARRAYSIZE(args));
	i = pop();
	if (i < 0 || i > num)
		printf("Error: o6_pickOneOf: %d out of range (0, %d)\n", i, num - 1);
	push(args[i]); 
}

void _0xCE_DrawWizImage() {
	WizImage wi;
	wi.flags = pop();
	wi.y1 = pop();
	wi.x1 = pop();
	wi.resNum = pop();
	wi.state = 0;
	displayWizImage(&wi);
}

void _0xD0_GetDateTime()
{
	time_t ut = time(NULL);
	tm* t = localtime(&ut);
	VAR(VAR_TIMEDATE_YEAR) = t->tm_year;
	VAR(VAR_TIMEDATE_MONTH) = t->tm_mon;
	VAR(VAR_TIMEDATE_DAY) = t->tm_mday;
	VAR(VAR_TIMEDATE_HOUR) = t->tm_hour;
	VAR(VAR_TIMEDATE_MINUTE) = t->tm_min; 
}

void _0xD1_StopTalking()
{
	//stopTalk();
}

void _0xD2_GetAnimateVariable()
{
	int var = pop();
	Actor *a = &_actors[pop()];//derefActor(pop(), "o6_getAnimateVariable");
	push(getAnimVar(a, var));
	//if(getAnimVar(a, var) == 0) setAnimVar(a, var, 1);
	//else setAnimVar(a, var, 0);
}

void _0xD5_JumpToScript()
{
	int args[25];
	int script;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();
	stopObjectCode();
	runScript(script, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args);
}

void _0xD6_BAnd()
{
	int a = pop();
	push(pop() & a); 
}

void _0xD7_BOr()
{
	int a = pop();
	push(pop() | a); 
}

void _0xD9_CloseFile()
{
	int slot = pop();
	printf("CloseFile (%d)\n", slot);
	/*if (0 <= slot && slot < 17) {
		if (_hOutFileTable[slot]) {
			_hOutFileTable[slot]->finalize();
			delete _hOutFileTable[slot];
			_hOutFileTable[slot] = 0;
		}

		delete _hInFileTable[slot];
		_hInFileTable[slot] = 0;
	} */
}

void _0xDA_OpenFile()
{
	int mode, slot, i;
	byte buffer[256];

	mode = pop();
	copyScriptString(buffer, sizeof(buffer));
	printf("Original filename %s\n", buffer);

	//const char *filename = (char *)buffer + convertFilePath(buffer, sizeof(buffer));
	//debug("Final filename to %s\n", filename);

	/*slot = -1;
	for (i = 1; i < 17; i++) {
		if (_hInFileTable[i] == 0 && _hOutFileTable[i] == 0) {
			slot = i;
			break;
		}
	}

	if (slot != -1) {
		switch (mode) {
		case 1:   // Read mode
			if (!_saveFileMan->listSavefiles(filename).empty()) {
				_hInFileTable[slot] = _saveFileMan->openForLoading(filename);
			} else {
				_hInFileTable[slot] = SearchMan.createReadStreamForMember(filename);
			}
			break;
		case 2:   // Write mode
			if (!strchr(filename, '/')) {
				_hOutFileTable[slot] = _saveFileMan->openForSaving(filename);
			}
			break;
		case 6: { // Append mode
			if (strchr(filename, '/'))
				break;

			// First check if the file already exists
			Common::InSaveFile *initialState = 0;
			if (!_saveFileMan->listSavefiles(filename).empty())
				initialState = _saveFileMan->openForLoading(filename);
			else
				initialState = SearchMan.createReadStreamForMember(filename);

			// Read in the data from the initial file
			uint32 initialSize = 0;
			byte *initialData = 0;
			if (initialState) {
				initialSize = initialState->size();
				initialData = new byte[initialSize];
				initialState->read(initialData, initialSize);
				delete initialState;
			}

			// Attempt to open a save file
			_hOutFileTable[slot] = _saveFileMan->openForSaving(filename);

			// Begin us off with the data from the previous file
			if (_hOutFileTable[slot] && initialData) {
				_hOutFileTable[slot]->write(initialData, initialSize);
				delete[] initialData;
			}

			} break;
		default:
			printf("Error: o72_openFile(): wrong open file mode %d\n", mode);
		}

		if (_hInFileTable[slot] == 0 && _hOutFileTable[slot] == 0)
			slot = -1;

	}
	printf("o72_openFile: slot %d, mode %d\n", slot, mode);*/
	push(/*slot*/1); 
}

void _0xDD_FindAllObjects()
{
	int room = pop();
	int i;

	if (room != _currentRoom)
		printf("Error: o72_findAllObjects: current room is not %d\n", room);

	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, /*_numLocalObjects*/RoomResource->RMDA->RMHD.NrObjects);
	writeArray(0, 0, 0, /*_numLocalObjects*/RoomResource->RMDA->RMHD.NrObjects);

	for (i = /*1*/0; i < /*_numLocalObjects*/RoomResource->RMDA->RMHD.NrObjects; i++) {
		writeArray(0, 0, i, /*_objs[i].obj_nr*/RoomResource->RMDA->OBCD[i]->ObjectId);
	}

	push(readVar(0));
}

void _0xDE_DeleteFile()
{
	byte buffer[256];

	copyScriptString(buffer, sizeof(buffer));
	//const char *filename = (char *)buffer + convertFilePath(buffer, sizeof(buffer));

	printf("o72_deleteFile(%s)\n", buffer);

	//if (!_saveFileMan->listSavefiles(filename).empty()) {
	//	_saveFileMan->removeSavefile(filename);
	//}
}

void _0xE1_GetPixel()
{
	uint16 area;

	int y = pop();
	int x = pop();
	byte subOp = fetchScriptByte();

	push(-1);

	//VirtScreen *vs = findVirtScreen(y);
	//if (vs == NULL || x > _screenWidth - 1 || x < 0) {
	//	push(-1);
	//	return;
	//}

	/*switch (subOp) {
	case 9: // HE 100
	case 218:
		if (_game.features & GF_16BIT_COLOR)
			area = READ_UINT16(vs->getBackPixels(x, y - vs->topline));
		else
			area = *vs->getBackPixels(x, y - vs->topline);
		break;
	case 8: // HE 100
	case 219:
		if (_game.features & GF_16BIT_COLOR)
			area = READ_UINT16(vs->getPixels(x, y - vs->topline));
		else
			area = *vs->getPixels(x, y - vs->topline);
		break;
	default:
		error("o72_getPixel: default case %d", subOp);
	}
	push(area);*/
} 

void _0xE2_LocalizeArrayToScript()
{
	int slot = pop();
	//localizeArray(slot, _currentScript);
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

	ArrayHeader *ah = _arrays[readVar(value)];//(ArrayHeader *)getResourceAddress(rtString, readVar(value));
	dim1end = ah->dim1end;

	if (dim1end < num) {
		int32 var_2 = readArray(value, 0, num - 1);
		shuffleArray(value, 1, dim1end);
		if (readArray(value, 0, 1) == var_2) {
			num = 2;
		} else {
			num = 1;
		}
	}

	writeArray(value, 0, 0, num + 1);
	push(readArray(value, 0, num)); 
}

/*void _0xEC_CopyString()
{
	int dst, size;
	int src = pop();

	size = resStrLen(getStringAddress(src)) + 1;
	dst = setupStringArray(size);

	appendSubstring(dst, src, -1, -1);

	push(dst);
} */

void _0xEE_GetStringLength()
{
	int id, len;
	byte *addr;

	id = pop();

	addr = (byte*)_arrays[id]->data;//getStringAddress(id);
	if (!addr)
		printf("Error: o70_getStringLen: Reference to zeroed array pointer (%d)\n", id);

	len = resStrLen((byte*)_arrays[id]->data);
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

void _0xF3_ReadINI()
{
	byte option[128];
	byte *data;

	copyScriptString(option, sizeof(option));
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 43: // HE 100
	case 6: // number
		if (!strcmp((char *)option, "DisablePrinting") || !strcmp((char *)option, "NoPrinting")) {
			push(1);
		} else if (!strcmp((char *)option, "TextOn")) {
			push(/*ConfMan.getBool("subtitles")*/1);
		} else{// if(!strcmp((char*)option, "BlackAndWhiteCursors") || !strcmp((char*)option, "SputmDebug") || !strcmp((char*)option, "NeverQuit") || !strcmp((char*)option, "NeverRestartAtEnd") || !strcmp((char*)option, "MouseOverride") || !strcmp((char*)option, "NoControlPanel") || !strcmp((char*)option, "Demo") || ) {
			push(0);
			//push(ConfMan.getInt((char *)option));
		}
		//else 
		//{
		//	printf("o72_readINI: %d Option %s\n", subOp, option); 
		//	while(1);
		//}
		break;
	case 77: // HE 100
	case 7: // string
		writeVar(0, 0);
		/*if (!strcmp((char *)option, "HE3File")) {
		Common::String fileName = generateFilename(-3);
		int len = resStrLen((const byte *)fileName.c_str());
		data = defineArray(0, kStringArray, 0, 0, 0, len);
		memcpy(data, fileName.c_str(), len);
		} else if (!strcmp((char *)option, "GameResourcePath") || !strcmp((char *)option, "SaveGamePath")) {
		// We set SaveGamePath in order to detect where it used
		// in convertFilePath and to avoid warning about invalid
		// path in Macintosh verisons.
		data = defineArray(0, kStringArray, 0, 0, 0, 2);
		if (_game.platform == Common::kPlatformMacintosh)
		memcpy(data, (const char *)"*:", 2);
		else
		memcpy(data, (const char *)"*\\", 2);
		} else {
		const char *entry = (ConfMan.get((char *)option).c_str());
		int len = resStrLen((const byte *)entry);
		data = defineArray(0, kStringArray, 0, 0, 0, len);
		memcpy(data, entry, len);
		}*/
		push(/*readVar(0)*/0);
		break;
	default:
		printf("Error: o72_readINI: default type %d\n", subOp);
	}

	printf("o72_readINI: %d Option %s\n", subOp, option); 
	//while(1);
}

void _0xF4_WriteINI()
{
	int value;
	byte option[256], string[1024];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 43: // HE 100
	case 6: // number
		value = pop();
		copyScriptString(option, sizeof(option));
		printf("o72_writeINI: %s = %d\n", option, value);

		//ConfMan.setInt((char *)option, value);
		break;
	case 77: // HE 100
	case 7: // string
		copyScriptString(string, sizeof(string));
		copyScriptString(option, sizeof(option));
		printf("o72_writeINI: %s = %s\n", option, string);

		// Filter out useless setting
		if (!strcmp((char *)option, "HETest"))
			return;

		// Filter out confusing subtitle setting
		if (!strcmp((char *)option, "TextOn"))
			return;

		// Filter out confusing path settings
		if (!strcmp((char *)option, "DownLoadPath") || !strcmp((char *)option, "GameResourcePath") || !strcmp((char *)option, "SaveGamePath"))
			return;

		//ConfMan.set((char *)option, (char *)string);
		break;
	default:
		printf("Error: o72_writeINI: default type %d\n", subOp);
	}

	//ConfMan.flushToDisk(); 
}

void _0xF9_CreateDirectory()
{
	byte directoryName[255];

	copyScriptString(directoryName, sizeof(directoryName));
	printf("o72_createDirectory: %s\n", directoryName);
}

void _0xFA_SetSystemMessage()
{
	byte name[1024];

	copyScriptString(name, sizeof(name));
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 240:
		printf("Debug: o72_setSystemMessage: (%d) %s\n", subOp, name);
		break;
	case 241: // Set Version
		printf("Debug: o72_setSystemMessage: (%d) %s\n", subOp, name);
		break;
	case 242:
		printf("Debug: o72_setSystemMessage: (%d) %s\n", subOp, name);
		break;
	case 243: // Set Window Caption
		printf("Debug: o72_setSystemMessage: (%d) %s\n", subOp, name);
		break;
	default:
		printf("Error: o72_setSystemMessage: default case %d\n", subOp);
	}
}

void _0xFB_PolygonOps()
{
	int vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y;
	int id, fromId, toId;
	bool flag;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 68: // HE 100
	case 69: // HE 100
	case 246:
	case 248:
		vert4y = pop();
		vert4x = pop();
		vert3y = pop();
		vert3x = pop();
		vert2y = pop();
		vert2x = pop();
		vert1y = pop();
		vert1x = pop();
		flag = (subOp == 69 || subOp == 248);
		id = pop();
		//_wiz->polygonStore(id, flag, vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y);
		break;
	case 28: // HE 100
	case 247:
		toId = pop();
		fromId = pop();
		//_wiz->polygonErase(fromId, toId);
		break;
	default:
		printf("Error: o71_polygonOps: default case %d\n", subOp);
	} 
}