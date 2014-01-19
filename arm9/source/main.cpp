#include <nds.h>
#include <fat.h>
#include <maxmod9.h>

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

#include <sound_io.h>

#include <HEP.h>

void waitForTimer(int msec_delay) {
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

void processInput()
{
	touchPosition touch;
	scanKeys();
	touchRead(&touch);
	int pressed = keysDown();
	int held = keysHeld();

	if(pressed & KEY_R) abortCutscene();

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

//const char* hep = "/scumm/PJS2DEMO.hep";

//const char* he0 = "/scumm/PJS2DEMO.HE0";
//const char* he1 = "/scumm/PJS2DEMO.(A)";
//const char* he4 = "/scumm/PJS2DEMO.HE4";

//const char* hep = "/scumm/F4-DEMO.hep";

//const char* he0 = "/scumm/F4-DEMO.HE0";
//const char* he1 = "/scumm/F4-DEMO.(A)";

//const char* hep = "/scumm/SokkenSoep.hep";

//const char* he0 = "/scumm/SokkenSoep.HE0";
//const char* he1 = "/scumm/SokkenSoep.(a)";

const char* hep = "/scumm/DOOLHOF.hep";

//const char* he0 = "/scumm/DOOLHOF.HE0";
//const char* he1 = "/scumm/DOOLHOF.(A)";

//const char* he0 = "/scumm/ZOODEMO.HE0";//op
//const char* he1 = "/scumm/ZOODEMO.HE1";

//const char* hep = "/scumm/SPYDEMO.hep";

//const char* he0 = "/scumm/SPYDEMO.HE0";
//const char* he1 = "/scumm/SPYDEMO.(A)";

//const char* hep = "/scumm/sf2demo.hep";

//const char* he0 = "/scumm/sf2demo.he0";
//const char* he1 = "/scumm/sf2demo.(a)";

//const char* hep = "/scumm/PAJAMA.hep";

//const char* he0 = "/scumm/sam/PAJAMA.HE0";
//const char* he1 = "/scumm/sam/PAJAMA.HE1";

//const char* hep = "/scumm/PAJAMA2.hep";

//const char* he0 = "/scumm/sam2/PAJAMA2.HE0";//op
//const char* he1 = "/scumm/sam2/PAJAMA2.(A)";

//const char* he0 = "/scumm/spy1/SPYFox.HE0";
//const char* he1 = "/scumm/spy1/SPYFox.(A)";

//const char* hep = "/scumm/pj3demo.hep";

//const char* he0 = "/scumm/pj3demo.he0";
//const char* he1 = "/scumm/pj3demo.(a)";

//const char* hep = "/scumm/Pajama3.hep";

//const char* hep = "/scumm/SPYFox.hep";

//const char* hep = "/scumm/Spyfox2.hep";

//const char* hep = "/scumm/PUTTTIJD.hep";

//const char* hep = "/scumm/FREDDI4.hep";

//static int framenr = 0;

int main()
{
	defaultExceptionHandler();
	consoleDemoInit();
	lcdMainOnBottom(); 
	TIMER0_CR = TIMER_ENABLE|TIMER_DIV_1024;
	TIMER1_CR = TIMER_ENABLE|TIMER_CASCADE;

	soundIO_InstallFIFO();
	soundIO_Enable();

/*	mm_ds_system sys;
	sys.mod_count 			= 0;
	sys.samp_count			= 0;
	sys.mem_bank			= 0;
	sys.fifo_channel		= FIFO_MAXMOD;
	mmInit( &sys ); */

	//irqInit();
	
	glInit();
	videoSetMode(MODE_0_3D);
	glEnable(GL_TEXTURE_2D);
	int textureID;
	glGenTextures(1, &textureID);
	glViewport(0,0,255,191);
	glClearColor(0,0,0,31);
	glClearPolyID(63);
	glClearDepth(0x7FFF);
	vramSetBankA(VRAM_A_TEXTURE_SLOT0);
	vramSetBankB(VRAM_B_TEXTURE_SLOT1);
	//vramSetBankC(VRAM_C_TEXTURE_SLOT2);
	vramSetBankD(VRAM_D_TEXTURE_SLOT2);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); 
	glOrthof32(0, 256, 192, 0, 410, 409600);
	glStoreMatrix(0);

	printf("Loading FAT...");
	if(!fatInitDefault())
	{
		printf("Failed!\n");
		return 1;
	}
	printf("Done!\n");

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

	/*printf("Loading HE0...");
	HE0_File = fopen(he0, "rb");
	if(HE0_File == NULL)
	{
		printf("Failed!\n");
		return 1;
	}
	printf("Done!\n");
	readHE0(HE0_File, &HE0_Data);

	printf("Loading HE1...");
	HE1_File = fopen(he1, "rb");
	if(HE1_File == NULL)
	{
		printf("Failed!\n");
		return 1;
	}
	printf("Done!\n");*/

	/*printf("Loading HE...");
	HE0_File = fopen(he, "rb");
	if(HE0_File == NULL)
	{
		printf("Failed!\n");
		return 1;
	}
	printf("Done!\n");
	HE1_File = HE0_File;
	readHE0(HE0_File, &HE0_Data);*/

	/*if(he4 != NULL)
	{
		printf("Loading HE4...");
		HE4_File = fopen(he4, "rb");
		if(HE4_File == NULL)
		{
			printf("Failed!\n");
			return 1;
		}
		printf("Done!\n");
		//fclose(HE4_File);
	}*/
	
	_numGlobalScripts = getScriptCount(HE0_File, &HE0_Data);

	//GlobalScripts = (void**)malloc(_numGlobalScripts * 4);

	//for(int i = 0; i < _numGlobalScripts; i++)
	//{
	//	GlobalScripts[i] = NULL;
	//}

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

	//malloc(512);
	//malloc(512);
	//malloc(512);

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
			/*glMatrixMode(GL_MODELVIEW);
			glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
			glLoadIdentity();

			glBindTexture(0, textureID);
			RenderFrame(textureID);

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
			glFlush(GL_TRANS_MANUALSORT);*/

				glMatrixMode(GL_MODELVIEW);
				glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
				glLoadIdentity();

				glBindTexture(0, textureID);
				RenderFrame();//textureID, true);

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

				//framenr++;

		}

		doSound();
		//ReadStream();

		//camera._last = camera._cur;

		//_res->increaseExpireCounter();

		//animateCursor();

		/* show or hide mouse */
		//CursorMan.showMouse(_cursor.state > 0); 

		diff = getMillis() - diff; 
	}

	/*CurrentRoom = readRoom(HE1_File, getLFLFOffset(HE0_File, &HE0_Data, 1));
	ConvertRoomBackground(HE1_File, CurrentRoom);


	int minroomnr = 1;
	int maxroomnr = getLFLFCount(HE0_File, &HE0_Data) - 1;
	int roomnr = minroomnr;

	while(true)
	{
	glMatrixMode(GL_MODELVIEW);
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
	glLoadIdentity();

	glBindTexture(0, textureID);
	RenderFrame(textureID);

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

	while(true)
	{
	scanKeys();
	u16 keys = keysHeld();

	if((keys & KEY_LEFT))
	{
	freeLFLF(CurrentRoom);
	roomnr--;
	if(roomnr < minroomnr) roomnr = minroomnr;
	CurrentRoom = readRoom(HE1_File, getLFLFOffset(HE0_File, &HE0_Data, roomnr));
	ConvertRoomBackground(HE1_File, CurrentRoom);
	break;
	}
	else if((keys & KEY_RIGHT))
	{
	freeLFLF(CurrentRoom);
	roomnr++;
	if(roomnr > maxroomnr) roomnr = maxroomnr;
	CurrentRoom = readRoom(HE1_File, getLFLFOffset(HE0_File, &HE0_Data, roomnr));
	ConvertRoomBackground(HE1_File, CurrentRoom);
	break;
	}

	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
	}
	}*/

	while(1);
	return 1;
}