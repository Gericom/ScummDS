#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <ioutil.h>

//#define XOR

bool readByte(FILE* handle, uint8_t* dst)
{
	if(fread(dst, 1, 1, handle))
	{
#ifdef XOR
		*dst ^= 0x69;
#endif
		return true;
	}
	return false;
}

bool readBytes(FILE* handle, uint8_t* dst, int count)
{
	if(fread(dst, 1, count, handle))
	{
#ifdef XOR
		for(int i = 0; i < count; i++)
		{
			dst[i] ^= 0x69;
		}
#endif
		return true;
	}
	return false;
}

bool readChar(FILE* handle, char* dst)
{
	if(fread(dst, 1, 1, handle))
	{
#ifdef XOR
		*dst ^= 0x69;
#endif
		return true;
	}
	return false;
}

bool readChars(FILE* handle, char* dst, int count)
{
	if(fread(dst, 1, count, handle))
	{
#ifdef XOR
		char* end = dst + count;
		for(char* dst2 = dst; dst2 < end; dst2++)
		{
			*dst2 ^= 0x69;
		}
#endif
		return true;
	}
	return false;
}

bool readU16LE(FILE* handle, uint16_t* dst)
{
	if(fread(dst, 2, 1, handle))
	{
#ifdef XOR
		*dst ^= 0x6969;
#endif
		return true;
	}
	return false;
}

bool readU24LE(FILE* handle, uint32_t* dst)
{
	if(fread(dst, 3, 1, handle))
	{
#ifdef XOR
		*dst ^= 0x696969;
#endif
		return true;
	}
	return false;
}

bool readU32LE(FILE* handle, uint32_t* dst)
{
	if(fread(dst, 4, 1, handle))
	{
#ifdef XOR
		*dst ^= 0x69696969;
#endif
		return true;
	}
	return false;
}

bool readU32sLE(FILE* handle, uint32_t* dst, int count)
{
	if(fread(dst, 4, count, handle))
	{
#ifdef XOR
		uint32_t* end = dst + count * 4;
		for(uint32_t* dst2 = dst; dst2 < end; dst2++)
		{
			*dst2 ^= 0x69696969;
		}
#endif
		return true;
	}
	return false;
}
