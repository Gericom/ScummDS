#include <nds.h>

#include <script.h>
#include <scumm.h>
#include <verbs.h>

void checkExecVerbs()
{
	/*VAR(VAR_MOUSE_STATE) = 0;

	if (_userPut <= 0 || _mouseAndKeyboardStat == 0)
		return;

	VAR(VAR_MOUSE_STATE) = _mouseAndKeyboardStat;

	int i, over;
	VerbSlot *vs;

	if (_userPut <= 0 || _mouseAndKeyboardStat == 0)
		return;

	if (_mouseAndKeyboardStat < MBS_MAX_KEY)
	{
		runInputScript(kKeyClickArea, _mouseAndKeyboardStat, 1);
	}
	else if (_mouseAndKeyboardStat & MBS_MOUSE_MASK)
	{
		//VirtScreen *zone = findVirtScreen(_mouse.y);
		const byte code = _mouseAndKeyboardStat & MBS_LEFT_CLICK ? 1 : 2;

		// This could be kUnkVirtScreen.
		// Fixes bug #1536932: "MANIACNES: Crash on click in speechtext-area"
		if (!zone)
			return;

		over = findVerbAtPos(_mouse.x, _mouse.y);
		if (over != 0) {
			// Verb was clicked
			runInputScript(kVerbClickArea, _verbs[over].verbid, code);
		} else {
			// Scene was clicked
			runInputScript((zone->number == kMainVirtScreen) ? kSceneClickArea : kVerbClickArea, 0, code);
		}
	} */
	if (_userPut <= 0) return;
	if(VAR(VAR_LEFTBTN_HOLD) == 1)
	{
		runInputScript(kSceneClickArea, 0, 1);
	}
} 