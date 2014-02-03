#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <HE1.h>
#include <ioutil.h>
#include <graphics.h>
#include <render.h>
#include <scumm.h>
#include <room.h>

#define READ_BIT (shift--, dataBit = data & 1, data >>= 1, dataBit)

#define BUFFERSIZE (4096 * 8)

static uint8_t buffer[BUFFERSIZE];
static int buffersize = 0;

//static int _decomp_shr;
//static int _decomp_mask;

void FILL_BITS_he(uint32* data, int* shift, int n)
{
	if (*shift < n) 
	{
		uint8_t dst;
		if(buffersize == 0) 
		{
			readBytes(HE1_File, &buffer[0], BUFFERSIZE);
			buffersize = BUFFERSIZE;
		}
		dst = buffer[BUFFERSIZE - buffersize];
		buffersize--;
		//readByte(handle, &dst);
		*data |= dst << *shift;
		*shift += 8;
	}
}

void writeRoomColor(LFLF_t* Room, byte *dst, byte color)
{
	uint8_t* colordata = (uint8_t*)(&Room->RMDA->PALS->WRAP->APAL->data[color * 3]);
	uint8_t r = colordata[0] >> 3;
	uint8_t g = colordata[1] >> 3;
	uint8_t b = colordata[2] >> 3;
	uint16_t c16 = r | (g << 5) | (b << 10);
	//printf("%x at 0x%x\n", color, (uint32)dst);

	*((uint16_t*)dst) = c16;
}

void ConvertRoomBackground(FILE* handle, LFLF_t* Room)
{
	uint16_t* dst = &TempBuffer[0];

	fseek(handle, Room->RMIM.IM.BMAP.dataOffset, SEEK_SET);
	buffersize = 0;
	uint8_t code;
	readByte(handle, &code);
	int _decomp_shr = code % 10;
	int _decomp_mask = 0xFF >> (8 - _decomp_shr);
	switch (code) {
	case 134:
	case 135:
	case 136:
	case 137:
	case 138:
		drawStripHE_asm(handle, Room, (uint8_t*)&dst[0], Room->RMDA->RMHD.RoomHeight, Room->RMDA->RMHD.RoomWidth, Room->RMDA->RMHD.RoomWidth * 2, -1, _decomp_shr, _decomp_mask);
		break;
	case 144:
	case 145:
	case 146:
	case 147:
	case 148:
		drawStripHE_asm(handle, Room, (uint8_t*)&dst[0], Room->RMDA->RMHD.RoomHeight, Room->RMDA->RMHD.RoomWidth, Room->RMDA->RMHD.RoomWidth * 2, RoomResource->RMDA->TransClrIdx, _decomp_shr, _decomp_mask);
		break;
	case 150:
		printf("Fill!");
		while(1);
		//while(scanKeys(), keysHeld() == 0);
		//swiDelay(5000000);
		//fill(dst, vs->pitch, *bmap_ptr, vs->w, vs->h, vs->format.bytesPerPixel);
		break;
	} 
	for(int i = 0, i2 = 0; i < 480; i+=2, i2++)
	{
		for(int j = 0, j2 = 0; j < 640; j+=2, j2++)
		{
			BackgroundFrameBuffer[i2 * 320 + j2] = Merge4Pixels(dst[i * 640 + j], dst[i * 640 + j + 1], dst[(i + 1) * 640 + j], dst[(i + 1) * 640 + j + 1]);
		}
	}
}

void ConvertStripeMap(FILE* handle, uint32_t DataOffset, uint8 *dst, int Width, int Height, int TransparentColorIndex)
{
	fseek(handle, DataOffset, SEEK_SET);
	uint32_t NrStrips;
	readU32LE(handle, &NrStrips);
	NrStrips = (NrStrips - 8) / 4;
	for (int i = 0; i < NrStrips; i++)
	{
		fseek(handle, DataOffset + i * 4, SEEK_SET);
		buffersize = 0;
		uint32_t Offset;
		readU32LE(handle, &Offset);
		Offset -= 8;
		fseek(handle, DataOffset + Offset, SEEK_SET);
		uint8_t code;
		readByte(handle, &code);
		int _decomp_shr = code % 10;
		int _decomp_mask = 0xFF >> (8 - _decomp_shr);
		switch (code)
		{
		case 1:
			drawStripRaw(RoomResource, dst + i * 16, Height, Width * 2, -1);
			break;
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
			drawStripBasicV(RoomResource, dst + i * 16, Height, Width * 2, -1, _decomp_shr, _decomp_mask);
			break;
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
			drawStripBasicH(RoomResource, dst + i * 16, Height, Width * 2, -1, _decomp_shr, _decomp_mask);
			break;
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
			drawStripBasicV(RoomResource, dst + i * 16, Height, Width * 2, RoomResource->RMDA->TransClrIdx, _decomp_shr, _decomp_mask);
			break;
		case 44:
		case 45:
		case 46:
		case 47:
		case 48:
			drawStripBasicH(RoomResource, dst + i * 16, Height, Width * 2, RoomResource->RMDA->TransClrIdx, _decomp_shr, _decomp_mask);
			break;
		case 64:
		case 65:
		case 66:
		case 67:
		case 68:
		case 104:
		case 105:
		case 106:
		case 107:
		case 108:
			drawStripComplex(RoomResource, dst + i * 16, Height, Width * 2, -1, _decomp_shr, _decomp_mask);
			break;
		case 84:
		case 85:
		case 86:
		case 87:
		case 88:
		case 124:
		case 125:
		case 126:
		case 127:
		case 128:
			drawStripComplex(RoomResource, dst + i * 16, Height, Width * 2, RoomResource->RMDA->TransClrIdx, _decomp_shr, _decomp_mask);
			break;
		case 134:
		case 135:
		case 136:
		case 137:
		case 138:
			drawStripHE_asm(handle, RoomResource, dst + i * 16, Height, 8, Width * 2, -1, _decomp_shr, _decomp_mask);
			break;
		case 144:
		case 145:
		case 146:
		case 147:
		case 148:
			drawStripHE_asm(handle, RoomResource, dst + i * 16, Height, 8, Width * 2, RoomResource->RMDA->TransClrIdx, _decomp_shr, _decomp_mask);
			break;
		case 150:
			printf("Fill!\n");
			while(1);
			break;
		default:
			printf("Error: Code not supported: %d\n", code);
			break;
		}
	}
}

#undef READ_BIT

#define READ_BIT (cl--, bit = bits & 1, bits >>= 1, bit)
void FILL_BITS(byte* cl, uint* bits)
{
	if (*cl <= 8)
	{     
		if(buffersize == 0) 
		{
			readBytes(HE1_File, &buffer[0], BUFFERSIZE);
			buffersize = BUFFERSIZE;
		}
		*bits |= (buffer[BUFFERSIZE - buffersize] << *cl); 
		buffersize--;
		*cl += 8;                
	}
}

void drawStripComplex(LFLF_t* Room, byte *dst, int height, int dstPitch, int transpColor, int _decomp_shr, int _decomp_mask)
{
	byte color = readByteBuffer();
	uint bits = readByteBuffer();

	byte cl = 8;
	byte bit;
	byte incm, reps;

	do {
		int x = 8;
		do {
			FILL_BITS(&cl, &bits);
			if (color != transpColor)
				writeRoomColor(Room, dst, color);
			dst += 2;

		againPos:
			if (!READ_BIT) {
			} else if (!READ_BIT) {
				FILL_BITS(&cl, &bits);
				color = bits & _decomp_mask;
				bits >>= _decomp_shr;
				cl -= _decomp_shr;
			} else {
				incm = (bits & 7) - 4;
				cl -= 3;
				bits >>= 3;
				if (incm) {
					color += incm;
				} else {
					FILL_BITS(&cl, &bits);
					reps = bits & 0xFF;
					do {
						if (!--x) {
							x = 8;
							dst += dstPitch - 16;
							if (!--height)
								return;
						}
						if (color != transpColor)
							writeRoomColor(Room, dst, color);
						dst += 2;
					} while (--reps);
					bits >>= 8;
					bits |= readByteBuffer() << (cl - 8);
					goto againPos;
				}
			}
		} while (--x);
		dst += dstPitch - 16;
	} while (--height);
}

void drawStripBasicH(LFLF_t* Room, byte *dst, int height, int dstPitch, int transpColor, int _decomp_shr, int _decomp_mask)
{
	byte color = readByteBuffer();
	uint bits = readByteBuffer();

	byte cl = 8;
	byte bit;
	int8 inc = -1;

	do {
		int x = 8;
		do {
			FILL_BITS(&cl, &bits);
			if (color != transpColor)
				writeRoomColor(Room, dst, color);
			dst += 2;
			if (!READ_BIT) {
			} else if (!READ_BIT) {
				FILL_BITS(&cl, &bits);
				color = bits & _decomp_mask;
				bits >>= _decomp_shr;
				cl -= _decomp_shr;
				inc = -1;
			} else if (!READ_BIT) {
				color += inc;
			} else {
				inc = -inc;
				color += inc;
			}
		} while (--x);
		dst += dstPitch - 16;
	} while (--height);
}

void drawStripBasicV(LFLF_t* Room, byte *dst, int height, int dstPitch, int transpColor, int _decomp_shr, int _decomp_mask)
{
	byte color = readByteBuffer();
	uint bits = readByteBuffer();

	byte cl = 8;
	byte bit;
	int8 inc = -1;

	int x = 8;
	do {
		int h = height;
		do {
			FILL_BITS(&cl, &bits);
			if (color != transpColor)
				writeRoomColor(Room, dst, color);
			dst += dstPitch;
			if (!READ_BIT) {
			} else if (!READ_BIT) {
				FILL_BITS(&cl, &bits);
				color = bits & _decomp_mask;
				bits >>= _decomp_shr;
				cl -= _decomp_shr;
				inc = -1;
			} else if (!READ_BIT) {
				color += inc;
			} else {
				inc = -inc;
				color += inc;
			}
		} while (--h);
		dst -= height * dstPitch - 2;
	} while (--x);
}

void drawStripRaw(LFLF_t* Room, byte *dst, int height, int dstPitch, int transpColor)
{
	do {
		for (int x = 0; x < 8; x ++) {
			byte color = readByteBuffer();
			if (color != transpColor)
				writeRoomColor(Room, dst + x * 2, color);
		}
		dst += dstPitch;
	} while (--height);
}

static uint8_t linebuffer[2048];

void decompressWizImage(uint8 *dst, int dstPitch, uint32_t DataOffset, int X, int Y, int Width, int Height, void *palPtr) {
	//const *dataPtrNext;
	uint8 code, *dstPtr, *dstPtrNext;
	int h, w, xoff, dstInc;
	uint8_t* line;

	fseek(HE1_File, DataOffset, SEEK_SET);

	//if (type == kWizXMap) {
	//		assert(xmapPtr != 0);
	//}
	//if (type == kWizRMap) {
	//	assert(palPtr != 0);
	//}

	dstPtr = dst + Y * dstPitch + X * 2;
	//dataPtr = src;

	// Skip over the first 'srcRect->top' lines in the data
	h = 0;//srcRect.top;
	/*while (h--) {
	dataPtr += READ_LE_UINT16(dataPtr) + 2;
	}*/
	h = Height;//srcRect.height();
	w = Width;//srcRect.width();
	if (h <= 0 || w <= 0)
		return;

	//if (flags & kWIFFlipY) {
	//	dstPtr += (h - 1) * dstPitch;
	//	dstPitch = -dstPitch;
	//}
	dstInc = 2;//bitDepth;
	//if (flags & kWIFFlipX) {
	//	dstPtr += (w - 1) * bitDepth;
	//	dstInc = -bitDepth;
	//}

	while (h--) {
		xoff = 0;//srcRect.left;
		w = Width;
		uint16 lineSize;
		readU16LE(HE1_File, &lineSize);
		readBytes(HE1_File, &linebuffer[0], lineSize);
		line = &linebuffer[0];
		dstPtrNext = dstPtr + dstPitch;
		if (lineSize != 0) {
			while (w > 0) {
				code = *line++;
				if (code & 1) {
					code >>= 1;
					if (xoff > 0) {
						xoff -= code;
						if (xoff >= 0)
							continue;

						code = -xoff;
					}
					dstPtr += dstInc * code;
					w -= code;
				} else if (code & 2) {
					code = (code >> 2) + 1;
					if (xoff > 0) {
						xoff -= code;
						++line;
						if (xoff >= 0)
							continue;

						code = -xoff;
						--line;
					}
					w -= code;
					if (w < 0) {
						code += w;
					}
					uint8_t color = *line++;
					while (code--) {
						writePaletteColor_asm(palPtr, dstPtr, color);
						//write8BitColor<type>(dstPtr, dataPtr, dstType, palPtr, xmapPtr, bitDepth);
						dstPtr += dstInc;
					}
				} else {
					code = (code >> 2) + 1;
					if (xoff > 0) {
						xoff -= code;
						line += code;
						if (xoff >= 0)
							continue;

						code = -xoff;
						line += xoff;
					}
					w -= code;
					if (w < 0) {
						code += w;
					}
					while (code--) {
						uint8_t color = *line++;
						writePaletteColor_asm(palPtr, dstPtr, color);
						//write8BitColor<type>(dstPtr, dataPtr, dstType, palPtr, xmapPtr, bitDepth);
						dstPtr += dstInc;
					}
				}
			}
		}
		dstPtr = dstPtrNext;
	}
}

void ConvertWIZImage(uint32_t DataOffset, void* Palette, int X, int Y, uint32_t Width, uint32_t Height, uint32_t Compression)
{
	switch (Compression)
	{
	case 1:
		{
			uint16_t* dst = &TempBuffer[0];
			for(int i = 0; i < 640 * 480; i++)
			{
				dst[i] = 0x8000;
			}
			decompressWizImage((uint8*)dst, 640 * 2, DataOffset, X, Y, Width, Height, Palette);
			for(int i = 0, i2 = 0; i < 480; i+=2, i2++)
			{
				for(int j = 0, j2 = 0; j < 640; j+=2, j2++)
				{
					if(dst[i * 640 + j] != 0x8000)
						WIZFrameBuffer[i2 * 320 + j2] = Merge4Pixels(dst[i * 640 + j], dst[i * 640 + j + 1], dst[(i + 1) * 640 + j], dst[(i + 1) * 640 + j + 1]);
				}
			}
		}
		break;
	default:
		break;
	}
}

void ConvertWIZCursor(uint32_t DataOffset, uint8_t* Dst, void* Palette, uint32_t Width, uint32_t Height, uint32_t Compression)
{
	switch (Compression)
	{
	case 1:
		decompressWizImage(Dst, 32 * 2, DataOffset, 0, 0, Width, Height, Palette);
		break;
	default:
		break;
	}
}

uint8_t readByteBuffer()
{
	if(buffersize == 0) 
	{
		readBytes(HE1_File, &buffer[0], BUFFERSIZE);
		buffersize = BUFFERSIZE;
	}
	uint8_t dst = buffer[BUFFERSIZE - buffersize];
	buffersize--;
	return dst;
}

void clearByteBuffer()
{
	buffersize = 0;
}

void ConvertAKOSFrame(uint32_t DataOffset, uint8_t* Dst, void* Palette, uint32_t PaletteLength, void* Colors, uint32_t Width, uint32_t Height, uint32_t Codec)
{
	buffersize = 0;
	switch (Codec)
	{
	case 1:  DecompressAKOSCodec1_asm(Dst, Palette, Colors, Width, Height, DataOffset, PaletteLength); break;
		//case 1: DecompressAKOSCodec1(DataOffset, Dst, Palette, PaletteLength, Colors, Width, Height); break;
		//case 16: return DecompressAKOSCodec16(Data, Offset, Palette, Colors, Width, Height);
	case 32: decompressWizImage(Dst, Width * 2, DataOffset, 0, 0, Width, Height, Colors); break;
	}
	//return null;
}