#ifndef __IOUTIL_H__
#define __IOUTIL_H__

#define MKTAG(a0,a1,a2,a3) ((uint32)((a0) | ((a1) << 8) | ((a2) << 16) | ((a3) << 24)))
#define SWAP_CONSTANT_32(a) \
	((uint32)((((a) >> 24) & 0x00FF) | \
	          (((a) >>  8) & 0xFF00) | \
	          (((a) & 0xFF00) <<  8) | \
	          (((a) & 0x00FF) << 24) ))

extern "C" bool readByte(FILE* handle, uint8_t* dst);
bool readBytes(FILE* handle, uint8_t* dst, int count);
bool readChar(FILE* handle, char* dst);
bool readChars(FILE* handle, char* dst, int count);
bool readU16LE(FILE* handle, uint16_t* dst);
extern "C" bool readU24LE(FILE* handle, uint32_t* dst);
bool readU32LE(FILE* handle, uint32_t* dst);
bool readU32sLE(FILE* handle, uint32_t* dst, int count);

#endif