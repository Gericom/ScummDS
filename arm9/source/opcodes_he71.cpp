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

/*void _0xEC_CopyString()
{
	int dst, size;
	int src = pop();

	size = resStrLen(getStringAddress(src)) + 1;
	dst = setupStringArray(size);

	appendSubstring(dst, src, -1, -1);

	push(dst);
} */

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