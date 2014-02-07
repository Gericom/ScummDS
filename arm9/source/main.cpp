#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <scumm.h>
#include <HE0.h>
#include <HE1.h>
#include <graphics.h>
#include <render.h>
#include <script.h>
#include <room.h>
#include <sounds.h>
#include <actor.h>
#include <resources.h>
#include <objects.h>
#include <opcodes.h>
#include <verbs.h>
#include <akos.h>

#include <sound_io.h>

#include <HEP.h>

ITCM_CODE void waitForTimer(int msec_delay) {
	uint32 start_time;

	//if (_fastMode & 2)
	//	msec_delay = 0;
	//else if (_fastMode & 1)
	//	msec_delay = 10;

	start_time = getMillis();

	while (true) {
		//_sound->updateCD(); // Loop CD Audio if needed
		//parseEvents();

		//_system->updateScreen();
		if (getMillis() >= start_time + msec_delay)
			break;
		//delayMillis(10);
	}
}

static int lastpressed = 0;

ITCM_CODE void processInput()
{
	touchPosition touch;
	scanKeys();
	touchRead(&touch);
	int pressed = keysDown();
	int held = keysHeld();

	if(pressed & KEY_START)
	{
		//menu here!
		//How to reset to main menu?
	}

	if(pressed & KEY_R) abortCutscene();

	VAR(VAR_KEY_STATE) = 0;

	if(held & KEY_LEFT) VAR(VAR_KEY_STATE) |= 1;
	else if(held & KEY_RIGHT) VAR(VAR_KEY_STATE) |= 2;
	else if(held & KEY_UP) VAR(VAR_KEY_STATE) |= 4;
	else if(held & KEY_DOWN) VAR(VAR_KEY_STATE) |= 8;

	if(lastpressed & KEY_TOUCH && !(pressed & KEY_TOUCH) && !(held & KEY_TOUCH))
	{ 
		VAR(VAR_LEFTBTN_HOLD) = 1;
		printf("Left Mouse!\n");
	}
	else VAR(VAR_LEFTBTN_HOLD) = 0;

	lastpressed = pressed;

	if (!(held & KEY_TOUCH) || touch.rawx == 0 || touch.rawy == 0) return;
	_mouse_x = touch.px * 2.5;
	_mouse_y = touch.py * 2.5;
}

static char* hep;

static int render = 0;

static int textureID;

ITCM_CODE int main()
{
	defaultExceptionHandler();
	videoSetMode(MODE_3_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_BG);
	vramSetBankC(VRAM_C_MAIN_BG); 

	consoleInit(NULL, 2, BgType_Text4bpp, BgSize_T_256x256, 0, 3, true, true);
	consoleSetWindow(NULL, 3, 3, 29, 16); 
	lcdMainOnBottom();	

	int bg3 = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 8, 0); 
	bgSetPriority (bg3, 0);

	printf("Loading FAT...");
	if(!fatInitDefault())
	{
		printf("Failed!\n");
		return 1;
	}
	printf("Done!\n");

	iprintf("\x1b[2J");

	FILE* bg = fopen("/scumm/resources/bottomscreen.bin", "rb");
	fread(bgGetGfxPtr(bg3), 2, 256 * 192, bg);
	fclose(bg);

	iprintf("\x1b[30m");

	int num = 0;

	DIR *pdir;
	struct dirent *pent;

	pdir=opendir("/scumm");

	while ((pent=readdir(pdir))!=NULL) {
		if(strcmp(".", pent->d_name) == 0 || strcmp("..", pent->d_name) == 0 || pent->d_type == DT_DIR)
			continue;
		iprintf("  %s\n", pent->d_name);
		num++;
	}

	closedir(pdir);


	int line = 0;
	iprintf("\x1b[%d;0H>", line);
	while(1)
	{
		ushort keys;
		while(scanKeys(), keysHeld() == 0);
		keys = keysHeld();
		if(keys & KEY_DOWN && line + 1 < num)
		{
			iprintf("\x1b[%d;0H ", line);
			iprintf("\x1b[%d;0H>", ++line);
		}
		else if(keys & KEY_UP && line - 1 >= 0)
		{
			iprintf("\x1b[%d;0H ", line);
			iprintf("\x1b[%d;0H>", --line);
		}
		else if(keys & KEY_A) break;
		swiDelay(5000000);
	}
	num = 0;
	pdir=opendir("/scumm");

	while ((pent=readdir(pdir))!=NULL) {
		if(strcmp(".", pent->d_name) == 0 || strcmp("..", pent->d_name) == 0 || pent->d_type == DT_DIR)
			continue;
		if(num == line)
		{
			char outp[512] = "/scumm/";
			memcpy(&outp[7], pent->d_name, 256);
			hep = &outp[0];
			break;
		}
		num++;
	}

	closedir(pdir); 

	consoleDemoInit();
	TIMER0_CR = TIMER_ENABLE|TIMER_DIV_1024;
	TIMER1_CR = TIMER_ENABLE|TIMER_CASCADE;

	soundIO_InstallFIFO();
	soundIO_Enable();

	glInit();
	videoSetMode(MODE_0_3D);
	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &textureID);
	glViewport(0,0,255,191);
	glClearColor(0,0,0,31);
	glClearPolyID(63);
	glClearDepth(0x7FFF);
	vramSetBankA(VRAM_A_TEXTURE_SLOT0);
	vramSetBankB(VRAM_B_TEXTURE_SLOT1);
	//vramSetBankD(VRAM_D_TEXTURE_SLOT2);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); 
	glOrthof32(0, 256, 192, 0, 410, 409600);
	glStoreMatrix(0);

	printf("Loading HEP...");
	FILE* HEP_F = fopen(hep, "rb");
	if(HEP_F == NULL)
	{
		printf("Failed!\n");
		return 1;
	}
	printf("Done!\n");
	readHEP(HEP_F);
	readHE0(HE0_File, &HE0_Data);

	//prerendering
	{
		glMatrixMode(GL_MODELVIEW);
		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
		glLoadIdentity();
		glBindTexture(0, textureID);
		RenderFrame();
		glColor3b(255, 255, 255);
		glBegin(GL_QUAD);
		glTexCoord2t16(0, 0);
		glVertex3v16(0, 0, -7 * 4096);
		glTexCoord2t16(320 * 16, 0);
		glVertex3v16(256, 0, -7 * 4096);
		glTexCoord2t16(320 * 16, 240 * 16);
		glVertex3v16(256, 192, -7 * 4096);
		glTexCoord2t16(0, 240 * 16);
		glVertex3v16(0, 192, -7 * 4096);
		glEnd();
		glFlush(GL_TRANS_MANUALSORT);
	}

	_numGlobalScripts = getScriptCount(HE0_File, &HE0_Data);

	for(int i = 0; i < _numArray; i++)
	{
		_arrays[i] = NULL;
	}

	for(int i = 0; i < 16; i++)
	{
		CurrentPlayingSounds[i] = NULL;
	}

	VAR(VAR_NUM_SOUND_CHANNELS) = 16;
	VAR(VAR_SOUND_CHANNEL) = 1;
	VAR(VAR_TALK_CHANNEL) = 2;

	VAR(VAR_PLATFORM) = 1;
	VAR(VAR_SCRIPT_CYCLE) = 1;
	VAR(VAR_NUM_SCRIPT_CYCLES) = 1;

	VAR(VAR_PLATFORM) = 1;

	// Array 132 is set to game path
	defineArray(132, kStringArray, 0, 0, 0, 0);

	ClearObjectFrameBuffer();

	setupHE90();

	//Execution Starts Here!
	runScript(1, 0, 0, NULL, 0);
	int diff = 0;
	while(1)
	{
		rand();
		//printf("Refresh!\n");
		VAR(VAR_TIMER) = diff * 60 / 1000; 
		//VAR(VAR_TIMER_TOTAL) += diff * 60 / 1000;

		int delta = VAR(VAR_TIMER_NEXT);
		if (delta < 1) delta = 1;

		waitForTimer(delta * 1000 / 60 - diff); 

		diff = getMillis();

		VAR(VAR_TMR_1) += delta;
		VAR(VAR_TMR_2) += delta;
		VAR(VAR_TMR_3) += delta; 

		if (delta > 15)
			delta = 15;

		decreaseScriptDelay(delta);

		int oldEgo = 0;
		oldEgo = VAR(VAR_EGO);

		processInput();


		//VAR(VAR_CAMERA_POS_X) = camera._cur.x;

		VAR(VAR_HAVE_MSG) = _haveMsg;

		VAR(VAR_VIRT_MOUSE_X) = _mouse_x;//_virtualMouse.x;
		VAR(VAR_VIRT_MOUSE_Y) = _mouse_y;//_virtualMouse.y;
		VAR(VAR_MOUSE_X) = _mouse_x;
		VAR(VAR_MOUSE_Y) = _mouse_y;


		VAR(VAR_GAME_LOADED) = 0;

		//scummLoop_handleSaveLoad();

		//((SoundHE *)_sound)->processSoundCode();
		runAllScripts();
		checkExecVerbs();
		checkAndRunSentenceScript();
		//if (shouldQuit())
		//	return;

		if (_currentRoom == 0) {}
		else 
		{
			processDrawQue();

			if(!render)
			{
				RenderFrame();
			}
			else
			{
				numactors = 0;
				for (int i = 0; i < _numActors; i++) {
					if (isInCurrentRoom(&_actors[i]) && _actors[i]._costume && _actors[i]._cost.AKOS != NULL) {
						renderCostume(&_actors[i], false);
						numactors++;
					}
				}
			}

			if(!render)
			{
				glTexCoord2t16(0, 0);
				glVertex3v16(0, 0, -7 * 4096);
				glTexCoord2t16(320 * 16, 0);
				glVertex3v16(256, 0, -7 * 4096);
				glTexCoord2t16(320 * 16, 240 * 16);
				glVertex3v16(256, 192, -7 * 4096);
				glTexCoord2t16(0, 240 * 16);
				glVertex3v16(0, 192, -7 * 4096);
				glFlush(GL_TRANS_MANUALSORT);
			}

			//render++;
			//if(render >= (((numactors * 3) > 5) ? 5: (numactors * 3))) render = 0;
			render++;
			if(render >= (((numactors * 3) > 5) ? 5: (numactors * 3))) render = 0;
		}

		doSound();
		soundIO_FillStreamBuffers();

		//camera._last = camera._cur;

		//_res->increaseExpireCounter();

		//animateCursor();

		/* show or hide mouse */
		//CursorMan.showMouse(_cursor.state > 0); 

		diff = getMillis() - diff; 
	}

	while(1);
	return 1;
}