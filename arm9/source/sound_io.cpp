//999999999999999999
#include <nds.h>
#include <nds/fifocommon.h>
#include <nds/fifomessages.h>

#include <stdio.h>

#include <ioutil.h>
#include <sound_io.h>

#define STREAM_BUFFER_SIZE (11025 * 4)

#define STREAM_BUFFER_A 0
#define STREAM_BUFFER_B 1

static u8 streamBufferA[STREAM_BUFFER_SIZE];
static u8 streamBufferB[STREAM_BUFFER_SIZE];

static u8 streamDstBuffer[STREAM_BUFFER_SIZE];

static int streamChannel = -1;

static FILE* streamHandle;
static uint32_t streamOffset;

static bool streamNeedData = false;

static int streamDstBufferId = STREAM_BUFFER_A;

static bool streamTimerUpdate = false;

void soundIO_WriteReg16(uint32_t address, uint16_t value)
{
	SoundIOMessage7_t msg;
	msg.command = SOUND_IO_WRITE_REG16;
	msg.WriteReg16.reg = address;
	msg.WriteReg16.val = value;
	fifoSendDatamsg(FIFO_SOUND_IO, sizeof(msg), (u8*)&msg); 
}

void soundIO_WriteReg32(uint32_t address, uint32_t value)
{
	SoundIOMessage7_t msg;
	msg.command = SOUND_IO_WRITE_REG32;
	msg.WriteReg32.reg = address;
	msg.WriteReg32.val = value;
	fifoSendDatamsg(FIFO_SOUND_IO, sizeof(msg), (u8*)&msg); 
}

void soundIO_Disable()
{
	fifoSendValue32(FIFO_SOUND_IO, SOUND_IO_MASTER_DISABLE); 
}

void soundIO_Enable()
{
	fifoSendValue32(FIFO_SOUND_IO, SOUND_IO_MASTER_ENABLE); 
}

void soundIO_PlaySound(int channel, void* data, SoundFormat format, u32 dataSize, u16 freq, u8 volume, u8 pan, bool loop, u16 loopPoint)
{
	soundIO_WriteReg32(SOUNDxSAD(channel), (uint32_t)data);
	soundIO_WriteReg16(SOUNDxPNT(channel), loopPoint);
	soundIO_WriteReg32(SOUNDxLEN(channel), dataSize >> 2);
	soundIO_WriteReg16(SOUNDxTMR(channel), (uint16_t)((-16756991 / freq)));
	soundIO_WriteReg32(SOUNDxCNT(channel), (1 << 31) | volume | (pan << 16) | (format << 29) | (1 << (loop ? 27 : 28))); 
}

void soundIO_StopSound(int channel)
{
	soundIO_WriteReg32(SOUNDxCNT(channel), 0); 
	if(channel == streamChannel)
	{
			timerStop(2);
			streamChannel = -1;
			streamHandle = NULL;
			streamOffset = 0;
			streamTimerUpdate = false;
			streamNeedData = false;
			streamDstBufferId = STREAM_BUFFER_A;
	}
}

static void soundIO_UpdateStream()
{
	if(!streamTimerUpdate)
	{
		streamTimerUpdate = true;
		return;
	}
	if(streamDstBufferId == STREAM_BUFFER_A)
	{
		for(int i = 0; i < STREAM_BUFFER_SIZE; i++)
		{
			streamDstBuffer[i] = streamBufferB[i] ^ 0x7F;
		}
		streamDstBufferId = STREAM_BUFFER_B;
		streamNeedData = true;
	}
	else
	{
		for(int i = 0; i < STREAM_BUFFER_SIZE; i++)
		{
			streamDstBuffer[i] = streamBufferA[i] ^ 0x7F;
		}
		streamDstBufferId = STREAM_BUFFER_A;
		streamNeedData = true;
	}
	streamTimerUpdate = false;
}

void soundIO_StartStream(int channel, FILE* handle, uint32_t offset, SoundFormat format, u16 freq, u8 volume, u8 pan, bool loop, u16 loopPoint)
{
	if(streamChannel != -1)
	{
		soundIO_StopSound(streamChannel);
		//timerStop(2);
		//kill stream here!
	}
	streamHandle = handle;
	streamOffset = offset;
	streamChannel = channel;
	fseek(handle, offset, SEEK_SET);
	readBytes(handle, &streamBufferA[0], STREAM_BUFFER_SIZE);
	readBytes(handle, &streamBufferB[0], STREAM_BUFFER_SIZE);
	for(int i = 0; i < STREAM_BUFFER_SIZE; i++)
	{
		streamDstBuffer[i] = streamBufferA[i] ^ 0x7F;
	}
	streamOffset += STREAM_BUFFER_SIZE * 2;
	streamDstBufferId = STREAM_BUFFER_A;
	streamTimerUpdate = false;

	uint16_t timerval = (uint16_t)((-16756991 / freq));
	uint16_t alarmval = TIMER_FREQ_1024(0.5);///*0.5*/freq / ((uint16_t)(-16756991 / timerval) + 2) / 2);//*/timerval * STREAM_BUFFER_SIZE / 2u / 512u;

	soundIO_WriteReg32(SOUNDxSAD(channel), (uint32_t)&streamDstBuffer[0]);
	soundIO_WriteReg16(SOUNDxPNT(channel), 0);
	soundIO_WriteReg32(SOUNDxLEN(channel), STREAM_BUFFER_SIZE >> 2);
	soundIO_WriteReg16(SOUNDxTMR(channel), timerval);
	soundIO_WriteReg32(SOUNDxCNT(channel), (1 << 31) | volume | (pan << 16) | (format << 29) | (1 << 27));
	timerStart(2, ClockDivider_1024, alarmval, soundIO_UpdateStream);
}

void soundIO_FillStreamBuffers()
{
	if(streamChannel == -1 || !streamNeedData) return;
	fseek(streamHandle, streamOffset, SEEK_SET);
	if(streamDstBufferId == STREAM_BUFFER_A) readBytes(streamHandle, &streamBufferB[0], STREAM_BUFFER_SIZE);
	else readBytes(streamHandle, &streamBufferA[0], STREAM_BUFFER_SIZE);
	streamOffset += STREAM_BUFFER_SIZE;
	streamNeedData = false;
}

void soundIO_CommandHandler(u32 command, void* userdata)
{
	
}

void soundIO_InstallFIFO()
{
	//fifoSetDatamsgHandler(FIFO_SOUND_IO, soundIO_DataHandler, 0);
	fifoSetValue32Handler(FIFO_SOUND_IO, soundIO_CommandHandler, 0);
}