#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__
#include <HE1.h>

void ConvertRoomBackground(FILE* handle, LFLF_t* Room);
void ConvertWIZImage(uint32_t DataOffset, void* Palette, int X, int Y, uint32_t Width, uint32_t Height, uint32_t Compression);
void ConvertWIZCursor(uint32_t DataOffset, uint8_t* Dst, void* Palette, uint32_t Width, uint32_t Height, uint32_t Compression);

#endif