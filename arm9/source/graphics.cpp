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

#define READ_BIT (shift--, dataBit = data & 1, data >>= 1, dataBit)

#define BUFFERSIZE (4096 * 128)

static uint8_t buffer[BUFFERSIZE];
static int buffersize = 0;

static int _decomp_shr;
static int _decomp_mask;

void FILL_BITS(FILE* handle, uint32* data, int* shift, int n)
{
	if (*shift < n) 
	{
		uint8_t dst;
		if(buffersize == 0) 
		{
			readBytes(handle, &buffer[0], BUFFERSIZE);
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

void drawStripHE(FILE* handle, LFLF_t* Room, byte *dst, int dstPitch, int width, int height, const bool transpCheck)
{
	static const int delta_color[] = { -4, -3, -2, -1, 1, 2, 3, 4 };
	uint32 dataBit, data;
	byte color;
	int shift;

	readByte(handle, &color);
	readU24LE(handle, &data);
	//src += 3;
	shift = 24;

	int x = width;
	while (1) {
		//if (!transpCheck || color != _transparentColor)
			writeRoomColor(Room, dst, color);
		dst += 2;//_vm->_bytesPerPixel;
		--x;
		if (x == 0) {
			x = width;
			dst += dstPitch - width * 2;//_vm->_bytesPerPixel;
			--height;
			if (height == 0)
				return;
		}
		FILL_BITS(handle, &data, &shift, 1);
		if (READ_BIT) {
			FILL_BITS(handle, &data, &shift, 1);
			if (READ_BIT) {
				FILL_BITS(handle, &data, &shift, 3);
				color += delta_color[data & 7];
				shift -= 3;
				data >>= 3;
			} else {
				FILL_BITS(handle, &data, &shift, _decomp_shr);
				color = data & _decomp_mask;
				shift -= _decomp_shr;
				data >>= _decomp_shr;
			}
		}
	}
}

void ConvertRoomBackground(FILE* handle, LFLF_t* Room)
{
	fseek(handle, Room->RMIM.IM.BMAP.dataOffset, SEEK_SET);
	buffersize = 0;
	uint8_t code;
	readByte(handle, &code);
	_decomp_shr = code % 10;
	_decomp_mask = 0xFF >> (8 - _decomp_shr);
	switch (code) {
		case 134:
		case 135:
		case 136:
		case 137:
		case 138:
			drawStripHE(handle, Room, (uint8_t*)&FrameBuffer[0], /*vs->pitch*/Room->RMDA->RMHD.RoomWidth * 2, Room->RMDA->RMHD.RoomWidth, Room->RMDA->RMHD.RoomHeight, false);
			break;
		case 144:
		case 145:
		case 146:
		case 147:
		case 148:
			drawStripHE(handle, Room, (uint8_t*)&FrameBuffer[0], /*vs->pitch*/Room->RMDA->RMHD.RoomWidth * 2, Room->RMDA->RMHD.RoomWidth, Room->RMDA->RMHD.RoomHeight, true);
			break;
		case 150:
			//fill(dst, vs->pitch, *bmap_ptr, vs->w, vs->h, vs->format.bytesPerPixel);
			break;
	} 
}

void writePaletteColor(void* Palette, byte *dst, byte color)
{
	uint8_t* colordata = (uint8_t*)(Palette + color * 3);// (uint8_t*)(&Room->RMDA->PALS->WRAP->APAL->data[color * 3]);
	uint8_t r = colordata[0] >> 3;
	uint8_t g = colordata[1] >> 3;
	uint8_t b = colordata[2] >> 3;
	uint16_t c16 = r | (g << 5) | (b << 10);
	//printf("%x at 0x%x\n", color, (uint32)dst);

	*((uint16_t*)dst) = c16;
}

static uint8_t linebuffer[1024];

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
						writePaletteColor(palPtr, dstPtr, color);
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
						writePaletteColor(palPtr, dstPtr, color);
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
			decompressWizImage((uint8*)&FrameBuffers[GetFreeFrameBuffer()][0], 640 * 2, DataOffset, X, Y, Width, Height, Palette);
			//return //ConvertWizImage(DataOffset, Palette, Width, Height);
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
			//return //ConvertWizImage(DataOffset, Palette, Width, Height);
			break;
		default:
			break;
	}
}

void DecompressAKOSCodec1(uint32_t DataOffset, uint8_t* Dst, void* Palette, uint32_t PaletteLength, void* Colors, uint32_t Width, uint32_t Height)
		{
			int ColorShift;
			byte RepeatMask;
			if (PaletteLength == 32) ColorShift = 3;
			else if (PaletteLength == 64) ColorShift = 2;
			else ColorShift = 4;
			RepeatMask = (byte)~((0xFF >> ColorShift) << ColorShift);

			fseek(HE1_File, DataOffset, SEEK_SET);

			int x = 0;
			int y = 0;
			while (true)
			{
				uint8_t d;
				readByte(HE1_File, &d);
				int repeat = (int)(/*Data[Offset]*/d & RepeatMask);
				int color = (int)(/*Data[Offset++]*/d >> ColorShift);
				if (repeat == 0)
				{
					//repeat = Data[Offset++];
					readByte(HE1_File, &d);
					repeat = d;
				}
				for (int j = 0; j < repeat; j++)
				{
					if (color != 0) writePaletteColor(Colors, Dst + (y * Width * 2 + x * 2), ((uint8_t*)Palette)[color]);
					y++;
					if (y == Height)
					{
						y = 0;
						x++;
						if (x == Width)
						{
							return;
						}
					}
				}
			}
		}

void ConvertAKOSFrame(uint32_t DataOffset, uint8_t* Dst, void* Palette, uint32_t PaletteLength, void* Colors, uint32_t Width, uint32_t Height, uint32_t Codec)
{
	switch (Codec)
	{
		case 1: DecompressAKOSCodec1(DataOffset, Dst, Palette, PaletteLength, Colors, Width, Height); break;
		//case 16: return DecompressAKOSCodec16(Data, Offset, Palette, Colors, Width, Height);
		case 32: decompressWizImage(Dst, Width * 2, DataOffset, 0, 0, Width, Height, Colors); break;
	}
	//return null;
}