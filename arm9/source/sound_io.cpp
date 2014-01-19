//999999999999999999
#include <nds.h>
#include <nds/fifocommon.h>
#include <nds/fifomessages.h>

#include <stdio.h>

#include <sound_io.h>

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
	soundIO_WriteReg16(SOUNDxTMR(channel), (uint16_t)((-0x1000000 / freq)));
	soundIO_WriteReg32(SOUNDxCNT(channel), (1 << 31) | volume | (pan << 16) | (format << 29) | (1 << (loop ? 27 : 28))); 
}

void soundIO_StopSound(int channel)
{
	soundIO_WriteReg32(SOUNDxCNT(channel), 0); 
}

void soundIO_CommandHandler(u32 command, void* userdata)
{
	
}

void soundIO_InstallFIFO()
{
	//fifoSetDatamsgHandler(FIFO_SOUND_IO, soundIO_DataHandler, 0);
	fifoSetValue32Handler(FIFO_SOUND_IO, soundIO_CommandHandler, 0);
}