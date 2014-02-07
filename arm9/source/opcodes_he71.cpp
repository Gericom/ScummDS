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

void appendSubstring(int dst, int src, int srcOffs, int len)
{
	int dstOffs, value;
	int i = 0;

	if (len == -1) {
		len = resStrLen((byte*)_arrays[src & ~0x33539000]->data);
		srcOffs = 0;
	}

	dstOffs = resStrLen((byte*)_arrays[dst & ~0x33539000]->data);

	len -= srcOffs;
	len++;

	while (i < len) {
		writeVar(0, src);
		value = readArray(0, 0, srcOffs + i);
		writeVar(0, dst);
		writeArray(0, 0, dstOffs + i, value);
		i++;
	}

	writeArray(0, 0, dstOffs + i, 0);
}

/*static void _0xEC_CopyString()
{
	int dst, size;
	int src = pop();

	size = resStrLen(getStringAddress(src)) + 1;
	dst = setupStringArray(size);

	appendSubstring(dst, src, -1, -1);

	push(dst);
} */

static void _0xEF_AppendString()
{
	int dst, size;

	int len = pop();
	int srcOffs = pop();
	int src = pop();

	size = len - srcOffs + 2;
	dst = setupStringArray(size);

	appendSubstring(dst, src, srcOffs, len);

	push(dst);
}

static void _0xF1_CompareString()
{
	int result;

	int array1 = pop();
	int array2 = pop();

	byte *string1 = (byte*)_arrays[array1 & ~0x33539000]->data;
	if (!string1)
		printf("Error: o71_compareString: Reference to zeroed array pointer (%d)\n", array1);

	byte *string2 = (byte*)_arrays[array2 & ~0x33539000]->data;
	if (!string2)
		printf("Error: o71_compareString: Reference to zeroed array pointer (%d)\n", array2);

	while (*string1 == *string2) {
		if (*string2 == 0) {
			push(0);
			return;
		}

		string1++;
		string2++;
	}

	result = (*string1 > *string2) ? -1 : 1;
	push(result);
}

static void _0xF6_GetCharIndexInString()
{
	int array, end, len, pos, value;

	value = pop();
	end = pop();
	pos = pop();
	array = pop();

	if (end >= 0) {
		len = resStrLen((byte*)_arrays[array & ~0x33539000]->data);
		if (len < end)
			end = len;
	} else {
		end = 0;
	}

	if (pos < 0)
		pos = 0;

	writeVar(0, array);
	if (end > pos) {
		while (end >= pos) {
			if (readArray(0, 0, pos) == value) {
				push(pos);
				return;
			}
			pos++;
		}
	} else {
		while (end <= pos) {
			if (readArray(0, 0, pos) == value) {
				push(pos);
				return;
			}
			pos--;
		}
	}

	push(-1);
}
 

static void _0xFB_PolygonOps()
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
		polygonStore(id, flag, vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y);
		break;
	case 28: // HE 100
	case 247:
		toId = pop();
		fromId = pop();
		polygonErase(fromId, toId);
		break;
	default:
		printf("Error: o71_polygonOps: default case %d\n", subOp);
	} 
}

static void _0xFC_PolygonHit()
{
	int y = pop();
	int x = pop();
	push(polygonHit(0, x, y));
} 

void setupHE71()
{
	setupHE70();
	_opcodes[0xEF] = _0xEF_AppendString;
	_opcodes[0xF1] = _0xF1_CompareString;
	_opcodes[0xF6] = _0xF6_GetCharIndexInString;
	_opcodes[0xFB] = _0xFB_PolygonOps;
	_opcodes[0xFC] = _0xFC_PolygonHit;
}