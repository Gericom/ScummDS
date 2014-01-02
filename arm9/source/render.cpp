#include <nds.h>

#include <render.h>
#include <actor.h>
#include <akos.h>
#include <scumm.h>

uint16_t FrameBuffer[640 * 480];

uint16_t FrameBuffers[1][640 * 480];

bool FrameBufferFree[1];

uint16_t ResultFrameBuffer[512 * 256];

//uint16_t ActorFrameBuffer[160 * 120];

uint16_t CursorBuffer[32 * 32 * 2];

static bool done = false;

#define VRAMCNT				(*(vu32*)0x04000240)

void InitFrameBuffers()
{
	for(int i = 0; i < 1; i++)
	{
		FrameBufferFree[i] = true;
		for(int j = 0; j < 640 * 480; j++)
		{
			FrameBuffers[i][j] = 0x8000;
		}
	}
}

int GetFreeFrameBuffer()
{
	for(int i = 0; i < 1; i++)
	{
		if(FrameBufferFree[i] == true)
		{
			FrameBufferFree[i] = false;
			return i;
		}
	}
	printf("No framebuffers Left!\n");
	//while(scanKeys(), keysHeld() == 0);
	//swiDelay(5000000);
	return 0;
}

uint16_t Merge4Pixels(uint16_t a, uint16_t b, uint16_t c, uint16_t d)
{
	uint16_t R = ((a & 0x1F) + (b & 0x1F) + (c & 0x1F) + (d & 0x1F)) >> 2;
	uint16_t G = (((a & 0x3E0) + (b & 0x3E0) + (c & 0x3E0) + (d & 0x3E0)) >> 2) & 0x3E0;
	uint16_t B = (((a & 0x7C00) + (b & 0x7C00) + (c & 0x7C00) + (d & 0x7C00)) >> 2) & 0x7C00;

	return R | G | B;
}

//640x480 -> 320x240 -> 512x256
void RenderFrame(int textureID)
{
	for(int i = 0, i2 = 0; i < 480; i+=2, i2++)
	{
		for(int j = 0, j2 = 0; j < 640; j+=2, j2++)
		{
			/*uint16_t a = FrameBuffer[i * 640 + j];
			uint16_t b = FrameBuffer[i * 640 + j + 1];
			uint16_t c = FrameBuffer[(i + 1) * 640 + j];
			uint16_t d = FrameBuffer[(i + 1) * 640 + j + 1];

			uint16_t R = ((a & 0x1F) + (b & 0x1F) + (c & 0x1F) + (d & 0x1F)) >> 2;
			uint16_t G = (((a & 0x3E0) + (b & 0x3E0) + (c & 0x3E0) + (d & 0x3E0)) >> 2) & 0x3E0;
			uint16_t B = (((a & 0x7C00) + (b & 0x7C00) + (c & 0x7C00) + (d & 0x7C00)) >> 2) & 0x7C00;*/

			ResultFrameBuffer[i2 * 512 + j2] = Merge4Pixels(FrameBuffer[i * 640 + j], FrameBuffer[i * 640 + j + 1], FrameBuffer[(i + 1) * 640 + j], FrameBuffer[(i + 1) * 640 + j + 1]);//R | G | B;//FrameBuffer[i * 640 + j];
		}
	}

	for(int a = 0; a < 1; a++)
	{
		if(!FrameBufferFree[a])
		{
			for(int i = 0, i2 = 0; i < 480; i+=2, i2++)
			{
				for(int j = 0, j2 = 0; j < 640; j+=2, j2++)
				{
					if(FrameBuffers[a][i * 640 + j] != 0x8000)
						ResultFrameBuffer[i2 * 512 + j2] = Merge4Pixels(FrameBuffers[a][i * 640 + j], FrameBuffers[a][i * 640 + j + 1], FrameBuffers[a][(i + 1) * 640 + j], FrameBuffers[a][(i + 1) * 640 + j + 1]);//R | G | B;//FrameBuffer[i * 640 + j];
				}
			}
		}
	}

	/*for(int i = 0; i < 160 * 120; i++)
	{
		ActorFrameBuffer[i] = 0x8000;
	}*/

	for(int i = 0; i < _numActors; i++)
	{
		if(_actors[i]._cost.AKOS != NULL && _actors[i]._visible)
		{
			/*for(int j = 0; j < 16; j++)
			{
					if(drawLimb(&_actors[i], j)) 
					{
						//akos_increaseAnim(&_actors[i], j);
						break;
					}
			}*/
			renderCostume(&_actors[i]);
			
			//printf("None Active!\n");

			//int q = 0;
			//while(drawLimb(&_actors[i], q) == 0) q++;
		}
	}

	/*for(int i = 0, i2 = 0; i < 160; i++, i2+=2)
	{
		for(int j = 0, j2 = 0; j < 120; j++, j2+=2)
		{
			if(ActorFrameBuffer[i * 160 + j] != 0x8000)
			{
				ResultFrameBuffer[i2 * 512 + j2] = ActorFrameBuffer[i * 160 + j];
				ResultFrameBuffer[i2 * 512 + j2 + 1] = ActorFrameBuffer[i * 160 + j];
				ResultFrameBuffer[(i2 + 1) * 512 + j2] = ActorFrameBuffer[i * 160 + j];
				ResultFrameBuffer[(i2 + 1) * 512 + j2 + 1] = ActorFrameBuffer[i * 160 + j];
			}
		}
	}*/

	if(_cursor_state > 0)
	{
		for(int y = 0; y < 32; y+=2)
		{
			for(int x = 0; x < 32; x+=2)
			{
				if(CursorBuffer[y * 32 + x] != 0x0000)
					ResultFrameBuffer[(_mouse_y / 2 + y / 2) * 512 + _mouse_x / 2 + x / 2] = Merge4Pixels(CursorBuffer[y * 32 + x], CursorBuffer[y * 32 + x + 1], CursorBuffer[(y + 1) * 32 + x], CursorBuffer[(y + 1) * 32 + x + 1]);
			}
		}
	}

	/*glMatrixMode(GL_MODELVIEW);
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
	glLoadIdentity();

	glBindTexture(0, textureID);*/
	glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_512, TEXTURE_SIZE_256, 0, TEXGEN_TEXCOORD, (uint8_t*)&ResultFrameBuffer[0]);
	/*

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
}