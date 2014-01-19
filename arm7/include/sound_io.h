//777777777777777777
#ifndef __SOUND_IO7_H__
#define __SOUND_IO7_H__

#define FIFO_SOUND_IO	FIFO_USER_08

#define SOUNDxCNT(x)	(*(vu32*)(0x04000400 + (x << 4)))
#define SOUNDxSAD(x)	(*(vu32*)(0x04000404 + (x << 4)))
#define SOUNDxTMR(x)	(*(vu16*)(0x04000408 + (x << 4)))
#define SOUNDxPNT(x)	(*(vu16*)(0x0400040A + (x << 4)))
#define SOUNDxLEN(x)	(*(vu32*)(0x0400040C + (x << 4)))

#define SOUNDCNT		(*(vu32*)0x04000500)
#define SOUNDBIAS		(*(vu32*)0x04000504)

typedef enum
{
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


void installSound_IOFIFO();

#endif