#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <ioutil.h>

//#define XOR

ITCM_CODE bool readByte(FILE* handle, uint8_t* dst)
{
	if(fread(dst, 1, 1, handle)) return true;
	return false;
}

ITCM_CODE bool readBytes(FILE* handle, uint8_t* dst, int count)
{
	if(fread(dst, 1, count, handle)) return true;
	return false;
}

ITCM_CODE bool readChar(FILE* handle, char* dst)
{
	if(fread(dst, 1, 1, handle)) return true;
	return false;
}

ITCM_CODE bool readChars(FILE* handle, char* dst, int count)
{
	if(fread(dst, 1, count, handle)) return true;
	return false;
}

ITCM_CODE bool readU16LE(FILE* handle, uint16_t* dst)
{
	if(fread(dst, 2, 1, handle)) return true;
	return false;
}

ITCM_CODE bool readU24LE(FILE* handle, uint32_t* dst)
{
	if(fread(dst, 3, 1, handle)) return true;
	return false;
}

ITCM_CODE bool readU32LE(FILE* handle, uint32_t* dst)
{
	if(fread(dst, 4, 1, handle)) return true;
	return false;
}

ITCM_CODE bool readU32sLE(FILE* handle, uint32_t* dst, int count)
{
	if(fread(dst, 4, count, handle)) return true;
	return false;
}
