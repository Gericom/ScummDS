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

void decompressWizImage(uint8 *dst, int dstPitch, uint32_t DataOffset, int X, int Y, int Width, int Height, void *palPtr) {
	//const *dataPtrNext;
	uint8 code, *dstPtr, *dstPtrNext;
	int h, w, xoff, dstInc;

	fseek(HE1_File, DataOffset, SEEK_SET);

	//if (type == kWizXMap) {
//		assert(xmapPtr != 0);
	//}
	//if (type == kWizRMap) {
	//	assert(palPtr != 0);
	//}

	dstPtr = dst;
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
		xoff = X;//srcRect.left;
		w = Width;
		uint16 lineSize;// = READ_LE_UINT16(dataPtr); dataPtr += 2;
		readU16LE(HE1_File, &lineSize);
		dstPtrNext = dstPtr + dstPitch;
		//dataPtrNext = dataPtr + lineSize;
		if (lineSize != 0) {
			while (w > 0) {
				readByte(HE1_File, &code);
				//code = *dataPtr++;
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
						fseek(HE1_File, 1, SEEK_CUR);
						//++dataPtr;
						if (xoff >= 0)
							continue;

						code = -xoff;
						fseek(HE1_File, -1, SEEK_CUR);
						//--dataPtr;
					}
					w -= code;
					if (w < 0) {
						code += w;
					}
					uint8_t color;
					readByte(HE1_File, &color);
					while (code--) {
						writePaletteColor(palPtr, dstPtr, color);
						//write8BitColor<type>(dstPtr, dataPtr, dstType, palPtr, xmapPtr, bitDepth);
						dstPtr += dstInc;
					}
					//fseek(HE1_File, 1, SEEK_CUR);
					//dataPtr++;
				} else {
					code = (code >> 2) + 1;
					if (xoff > 0) {
						xoff -= code;
						fseek(HE1_File, code, SEEK_CUR);
						//dataPtr += code;
						if (xoff >= 0)
							continue;

						code = -xoff;
						fseek(HE1_File, xoff, SEEK_CUR);
						//dataPtr += xoff;
					}
					w -= code;
					if (w < 0) {
						code += w;
					}
					while (code--) {
						uint8_t color;
						readByte(HE1_File, &color);
						writePaletteColor(palPtr, dstPtr, color);
						//write8BitColor<type>(dstPtr, dataPtr, dstType, palPtr, xmapPtr, bitDepth);
						//fseek(HE1_File, 1, SEEK_CUR);
						//dataPtr++;
						dstPtr += dstInc;
					}
				}
			}
		}
		//dataPtr = dataPtrNext;
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
	}
}