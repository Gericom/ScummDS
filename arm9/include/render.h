#ifndef __RENDER_H__
#define __RENDER_H__

extern uint16_t FrameBuffer[640 * 480];
extern uint16_t ResultFrameBuffer[512 * 256];

extern uint16_t FrameBuffers[1][640 * 480];

//extern uint16_t ActorFrameBuffer[160 * 120];

extern uint16_t CursorBuffer[32 * 32 * 2];

extern bool FrameBufferFree[1];

void InitFrameBuffers();
int GetFreeFrameBuffer();

void RenderFrame(int textureID);
void DoRender(int textureID);

#endif