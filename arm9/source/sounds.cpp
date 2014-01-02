#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <ioutil.h>
#include <HE0.h>
#include <script.h>
#include <sounds.h>
#include <scumm.h>

//#define NOSOUND

int32 _heSndSoundId;
int32 _heSndOffset;
int32 _heSndChannel;
int32 _heSndFlags;
int32 _heSndSoundFreq; 

int16 _soundQuePos;//, _soundQue[0x100];
int16 _soundQue2Pos;

uint _lastSound;

PlayingSound* CurrentPlayingSounds[16];

//uint8_t* SoundData[1][11025 * 5];

bool readSoundBytes(FILE* handle, uint8_t* dst, int count)
{
	if(fread(dst, 1, count, handle))
	{
		for(int i = 0; i < count; i++)
		{
			dst[i] ^= 0x69;
			dst[i] ^= 0x7F;
		}
		return true;
	}
	return false;
}

void* readSound(FILE* handle, uint32_t* length, uint16_t* rate)
{
	uint32_t sig;
	uint32_t size;
digi:
	readU32LE(handle, &sig);
	printf("Sound: %c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
	readU32LE(handle, &size);
	if(sig == MKTAG('D', 'I', 'G', 'I'))
	{
		readU32LE(handle, &sig);//HSHD
		printf("Sound: %c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
		readU32LE(handle, &size);
		fseek(handle, 6, SEEK_CUR);//?
		readU16LE(handle, rate);
		fseek(handle, 8, SEEK_CUR);//?

		readU32LE(handle, &sig);
		printf("Sound: %c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
		readU32LE(handle, &size);
		if(sig == MKTAG('S', 'B', 'N', 'G'))//?
		{
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			readU32LE(handle, &sig);
			readU32LE(handle, &size);
		}
#ifndef NOSOUND
		void* data = malloc(SWAP_CONSTANT_32(size) - 8);
		readSoundBytes(handle, (uint8_t*)data, SWAP_CONSTANT_32(size) - 8);
#else
		void* data = NULL;
		fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
#endif
		*length = SWAP_CONSTANT_32(size) - 8;
		printf("Sound: OK\n");
		return data;
	}
	else if(sig == MKTAG('T', 'A', 'L', 'K'))
	{
		readU32LE(handle, &sig);//HSHD
		printf("Sound: %c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
		readU32LE(handle, &size);
		fseek(handle, 6, SEEK_CUR);//?
		readU16LE(handle, rate);
		fseek(handle, 8, SEEK_CUR);//?

		readU32LE(handle, &sig);
		printf("Sound: %c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
		readU32LE(handle, &size);
#ifndef NOSOUND
		void* data = malloc(SWAP_CONSTANT_32(size) - 8);
		readSoundBytes(handle, (uint8_t*)data, SWAP_CONSTANT_32(size) - 8);
#else
		void* data = NULL;
		fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
#endif
		*length = SWAP_CONSTANT_32(size) - 8;
		printf("Sound: OK\n");
		return data;
	}
	else if(sig == MKTAG('S', 'O', 'U', 'N'))
	{
		goto digi;
	}
	else
	{
		while(1);
	}
}

void addSoundToQueue(int sound, int heOffset, int heChannel, int heFlags)
{
	printf("sound: %d\n", sound);
	printf("heOffset: %d\n", heOffset);
	printf("heChannel: %d\n", heChannel);
	printf("heFlags: %d\n", heFlags);
	//while(scanKeys(), keysHeld() == 0);
	//swiDelay(5000000);

	VAR(VAR_LAST_SOUND) = sound;

	if (heFlags & 16) {
		//playHESound(sound, heOffset, heChannel, heFlags);
		printf("playHESound?\n");
		while(1);
		return;
	} else {
		VAR(VAR_LAST_SOUND) = sound;

		_lastSound = sound;

		

		// HE music resources are in separate file
		//if (sound <= _vm->_numSounds)
		//{
		//_vm->ensureResourceLoaded(rtSound, sound);
		//}

		addSoundToQueue2(sound, heOffset, heChannel, heFlags);
	}
}

void addSoundToQueue2(int sound, int heOffset, int heChannel, int heFlags)
{
	_soundQue2[_soundQue2Pos].sound = sound;
	_soundQue2[_soundQue2Pos].offset = heOffset;
	_soundQue2[_soundQue2Pos].channel = heChannel;
	_soundQue2[_soundQue2Pos].flags = heFlags;
	_soundQue2Pos++;
}

int findFreeSlot()
{
	for(int i = 0; i < 16; i++)
	{
		if(CurrentPlayingSounds[i] == NULL)
		{
			CurrentPlayingSounds[i] = (PlayingSound*)malloc(sizeof(PlayingSound));
			return i;
		}
	}
	return 0;
}

void doSound()
{
	for(int i = 0; i < 16; i++)
	{
		if(CurrentPlayingSounds[i] != NULL)
		{
			if(CurrentPlayingSounds[i]->endtime <= getMillis())
			{
				soundKill(CurrentPlayingSounds[i]->soundid);
#ifndef NOSOUND
				free(CurrentPlayingSounds[i]->data);
#endif
				free(CurrentPlayingSounds[i]);
				CurrentPlayingSounds[i] = NULL;
			}
		}
	}
	int snd, heOffset, heChannel, heFlags;
	for (int i = 0; i <_soundQue2Pos; i++) 
	{
		snd = _soundQue2[i].sound;
		heOffset = _soundQue2[i].offset;
		heChannel = _soundQue2[i].channel;
		heFlags = _soundQue2[i].flags;

		if(heOffset > 0)
		{
			printf("heOffset > 0: %d\n", heOffset);
			while(1);
		}

		/*if(snd)
		{
			printf("Sound Count: %d\n", getSoundCount(HE0_File, &HE0_Data));
			printf("SoundId: %d\n", snd);
			//while(scanKeys(), keysHeld() == 0);
			//swiDelay(5000000);
		}*/

		if(snd >= getSoundCount(HE0_File, &HE0_Data))//HE4 Music
		{
			printf("HE4!\n");
			printf("SoundId: %d\n", snd);
			printf("Channel: %d\n", heChannel);
			if(HE4_File != NULL)
			{

			}
		}
		else if(snd > 0 && snd < getSoundCount(HE0_File, &HE0_Data))//Room Noise
		{
			uint32_t offs = getSoundOffset(HE0_File, &HE0_Data, snd);
			if(((int32)offs) <= 0) continue;
			fseek(HE1_File, offs, SEEK_SET);
			uint32_t length;
			uint16_t rate;
			printf("SoundId: %d\n", snd);

			//while(scanKeys(), keysHeld() == 0);

			

			void* data = readSound(HE1_File, &length, &rate);

			int slot = findFreeSlot();
			
			CurrentPlayingSounds[slot]->sound = snd;
			CurrentPlayingSounds[slot]->length = length;
#ifndef NOSOUND
			CurrentPlayingSounds[slot]->data = data;
			soundEnable();
			CurrentPlayingSounds[slot]->soundid = soundPlaySample(data, SoundFormat_8Bit, length, rate, 127, 64, false, 0);
#endif
			CurrentPlayingSounds[slot]->starttime = getMillis();
			CurrentPlayingSounds[slot]->endtime = CurrentPlayingSounds[slot]->starttime + (length * 1000) / rate;
			//printf("Starttime: %d\n", CurrentPlayingSounds[slot]->starttime);
			//printf("Endtime: %d\n", CurrentPlayingSounds[slot]->endtime);
			//printf("Rate: %d\n", rate);
			//printf("Length: %d\n", length);
			//while(1);
		}
		else if(snd == 0) continue;
		else
		{
			printf("SoundId: %d\n", snd);
			while(1);
		}
		//if (snd) playHESound(snd, heOffset, heChannel, heFlags);
	}
	_soundQue2Pos = 0; 
}

void stopSound(int id)
{
	for(int i = 0; i < 16; i++)
	{
		if(CurrentPlayingSounds[i] != NULL)
		{
			if(CurrentPlayingSounds[i]->sound == id)
			{
				soundKill(CurrentPlayingSounds[i]->soundid);
#ifndef NOSOUND
				free(CurrentPlayingSounds[i]->data);
#endif
				free(CurrentPlayingSounds[i]);
				CurrentPlayingSounds[i] = NULL;
			}
		}
	}
}

int isSoundRunning(int id)
{
	if(id >= 8000) return 1;
	for(int i = 0; i < 16; i++)
	{
		if(CurrentPlayingSounds[i] != NULL)
		{
			if(CurrentPlayingSounds[i]->sound == id)
			{
				if(CurrentPlayingSounds[i]->endtime <= getMillis()) return 0;
				else return 1;
			}
		}
	}
	for (int i = 0; i <_soundQue2Pos; i++) 
	{
		if(_soundQue2[i].sound == id) return 1;
	}
	return 0;
}