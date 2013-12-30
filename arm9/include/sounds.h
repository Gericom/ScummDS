#ifndef __SOUNDS_H__
#define __SOUNDS_H__

extern int32 _heSndSoundId;
extern int32 _heSndOffset;
extern int32 _heSndChannel;
extern int32 _heSndFlags;
extern int32 _heSndSoundFreq; 

extern uint _lastSound;

	struct {
		int16 sound;
		int32 offset;
		int16 channel;
		int16 flags;
	} _soundQue2[10]; 

	struct PlayingSound {
		int soundid;
		int16 sound;
		uint32_t length;
		void* data;
		int starttime;
		int endtime;
	} ;

extern PlayingSound* CurrentPlayingSounds[16];

void addSoundToQueue(int sound, int heOffset, int heChannel, int heFlags);
void addSoundToQueue2(int sound, int heOffset, int heChannel, int heFlags);
void doSound();
void stopSound(int id);
int isSoundRunning(int id);


#endif