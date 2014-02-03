#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__
#include <HE1.h>

void ConvertRoomBackground(FILE* handle, LFLF_t* Room);
void ConvertStripeMap(FILE* handle, uint32_t DataOffset, uint8 *dst, int Width, int Height, int TransparentColorIndex);
void drawStripComplex(LFLF_t* Room, byte *dst, int height, int dstPitch, int transpColor, int _decomp_shr, int _decomp_mask);
void drawStripBasicH(LFLF_t* Room, byte *dst, int height, int dstPitch, int transpColor, int _decomp_shr, int _decomp_mask);
void drawStripBasicV(LFLF_t* Room, byte *dst, int height, int dstPitch, int transpColor, int _decomp_shr, int _decomp_mask);
void drawStripRaw(LFLF_t* Room, byte *dst, int height, int dstPitch, int transpColor);
void ConvertWIZImage(uint32_t DataOffset, void* Palette, int X, int Y, uint32_t Width, uint32_t Height, uint32_t Compression);
void ConvertWIZCursor(uint32_t DataOffset, uint8_t* Dst, void* Palette, uint32_t Width, uint32_t Height, uint32_t Compression);
void ConvertAKOSFrame(uint32_t DataOffset, uint8_t* Dst, void* Palette, uint32_t PaletteLength, void* Colors, uint32_t Width, uint32_t Height, uint32_t Codec);

//extern "C" void debugwait();
extern "C" void FILL_BITS_he(uint32* data, int* shift, int n);
extern "C" void writeRoomColor(LFLF_t* Room, byte *dst, byte color);
//extern "C" void writePaletteColor(void* Palette, byte *dst, byte color);
extern "C" void clearByteBuffer();
extern "C" uint8_t readByteBuffer();/*FILE* handle,*/// uint8_t* dst);
extern "C"
{
	void drawStripHE_asm(FILE* handle, LFLF_t* Room, byte *dst, int height, int width, int dstPitch, int transpColor, int _decomp_shr, int _decomp_mask);
	void writePaletteColor_asm(void* Palette, byte *dst, byte color);
	void DecompressAKOSCodec1_asm(uint8_t* Dst, void* Palette, void* Colors, uint32_t Width, uint32_t Height, uint32_t DataOffset, uint32_t PaletteLength);
}

#endif