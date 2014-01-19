//777777777777777777
#include <nds.h>
#include <nds/fifocommon.h>
#include <nds/fifomessages.h>

#include <sound_io.h>

void enableSound()
{
	powerOn(POWER_SOUND);
	writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
	REG_SOUNDCNT = SOUND_ENABLE;
	REG_MASTER_VOLUME = 127;
}

void disableSound()
{
	REG_SOUNDCNT &= ~SOUND_ENABLE;
	writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_AMP ) | PM_SOUND_MUTE );
	powerOff((PM_Bits)POWER_SOUND);
} 

void soundIO_DataHandler(int bytes, void *user_data)
{
	SoundIOMessage7_t msg;

	fifoGetDatamsg(FIFO_SOUND_IO, bytes, (u8*)&msg);

	switch(msg.command)
	{
		case SOUND_IO_WRITE_REG16:
			*((vu16*)msg.WriteReg16.reg) = msg.WriteReg16.val;
			break;
		case SOUND_IO_WRITE_REG32:
			*((vu32*)msg.WriteReg32.reg) = msg.WriteReg32.val;
			break;
	}
}

void soundIO_CommandHandler(u32 command, void* userdata)
{
	switch(command & 0xFF)
	{
		case SOUND_IO_MASTER_DISABLE: disableSound(); break;
		case SOUND_IO_MASTER_ENABLE: enableSound(); break;
	}
}

void installSound_IOFIFO()
{
	fifoSetDatamsgHandler(FIFO_SOUND_IO, soundIO_DataHandler, 0);
	fifoSetValue32Handler(FIFO_SOUND_IO, soundIO_CommandHandler, 0);
}