//999999999999999999
#ifndef __SOUND_IO9_H__
#define __SOUND_IO9_H__

#define FIFO_SOUND_IO	FIFO_USER_08

#define SOUNDxCNT(x)	(0x04000400 + (x << 4))
#define SOUNDxSAD(x)	(0x04000404 + (x << 4))
#define SOUNDxTMR(x)	(0x04000408 + (x << 4))
#define SOUNDxPNT(x)	(0x0400040A + (x << 4))
#define SOUNDxLEN(x)	(0x0400040C + (x << 4))

#define SOUNDCNT		(0x04000500)
#define SOUNDBIAS		(0x04000504)

typedef enum {
   	SOUND_IO_MASTER_DISABLE = 0,
	SOUND_IO_MASTER_ENABLE	= 1
} SoundIOCommand7;

typedef enum
{
	SOUND_IO_WRITE_REG16 = 0,
	SOUND_IO_WRITE_REG32 = 1
} SoundIOMessage7;

typedef struct SoundIOMessage7_t
{
	u8 command;
	union
	{
		struct 
		{
			u32 reg;
			u16 val;
		} WriteReg16;

		struct 
		{
			u32 reg;
			u32 val;
		} WriteReg32;
	}; 
} ALIGN(4) SoundIOMessage7_t;

void soundIO_Disable();
void soundIO_Enable();
void soundIO_PlaySound(int channel, void* data, SoundFormat format, u32 dataSize, u16 freq, u8 volume, u8 pan, bool loop, u16 loopPoint);
void soundIO_StopSound(int channel);
void soundIO_InstallFIFO();

#endif