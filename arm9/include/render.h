#ifndef __RENDER_H__
#define __RENDER_H__

extern int numactors;

extern int _drawObjectQueNr;
extern uint8_t _drawObjectQue[200]; 

extern uint16_t TempBuffer[640 * 480];

extern uint16_t BackgroundFrameBuffer[320 * 240];
extern uint16_t ResultFrameBuffer[512 * 256];

extern uint16_t WIZFrameBuffer[320 * 240];

extern uint16_t ObjectFrameBuffer[320 * 240];

extern uint16_t CursorBuffer[32 * 32 * 2];

void InitFrameBuffers();
void ClearObjectFrameBuffer();

extern "C"
{
	uint16_t Merge4Pixels(uint16_t a, uint16_t b, uint16_t c, uint16_t d);
}

void RenderFrame();//int textureID, bool render);
//void DoRender(int textureID);

#endif