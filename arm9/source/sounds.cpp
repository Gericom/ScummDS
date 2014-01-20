#include <nds.h>
#include <fat.h>
#include <maxmod9.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <ioutil.h>
#include <HE0.h>
#include <script.h>
#include <sounds.h>
#include <scumm.h>
#include <actor.h>

#include <sound_io.h>

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
#ifdef XOR
			dst[i] ^= 0x69;
#endif
			dst[i] ^= 0x7F;
		}
		return true;
	}
	return false;
}

void* readSound(FILE* handle, uint32_t* length, uint16_t* rate, bool readData = true, uint32_t* dataoffset = NULL)
{
	uint32_t sig;
	uint32_t size;
digi:
	readU32LE(handle, &sig);
	readU32LE(handle, &size);
	if(sig == MKTAG('D', 'I', 'G', 'I'))
	{
		readU32LE(handle, &sig);//HSHD
		//printf("Sound: %c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
		readU32LE(handle, &size);
		fseek(handle, 6, SEEK_CUR);//?
		readU16LE(handle, rate);
		fseek(handle, 8, SEEK_CUR);//?

		readU32LE(handle, &sig);
		//printf("Sound: %c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
		readU32LE(handle, &size);
		if(sig == MKTAG('S', 'B', 'N', 'G'))//?
		{
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			readU32LE(handle, &sig);
			readU32LE(handle, &size);
		}
		if(dataoffset != NULL)
		{
			fpos_t pos;
			fgetpos(handle, &pos);
			*dataoffset = (uint32_t)pos;
		}
		void* data;
#ifndef NOSOUND
		if(readData)
		{
			data = malloc(SWAP_CONSTANT_32(size) - 8);
			readSoundBytes(handle, (uint8_t*)data, SWAP_CONSTANT_32(size) - 8);
		}
		else
		{
#endif
			data = NULL;
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
#ifndef NOSOUND
		}	
#endif

		*length = SWAP_CONSTANT_32(size) - 8;
		//printf("Sound: OK\n");
		return data;
	}
	else if(sig == MKTAG('T', 'A', 'L', 'K'))
	{
		readU32LE(handle, &sig);//HSHD
		//printf("Sound: %c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
		readU32LE(handle, &size);
		fseek(handle, 6, SEEK_CUR);//?
		readU16LE(handle, rate);
		fseek(handle, 8, SEEK_CUR);//?

		readU32LE(handle, &sig);
		//printf("Sound: %c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
		readU32LE(handle, &size);
		if(dataoffset != NULL)
		{
			fpos_t pos;
			fgetpos(handle, &pos);
			*dataoffset = (uint32_t)pos;
		}
		void* data;
#ifndef NOSOUND
		if(readData)
		{
			data = malloc(SWAP_CONSTANT_32(size) - 8);
			readSoundBytes(handle, (uint8_t*)data, SWAP_CONSTANT_32(size) - 8);
		}
		else
		{
#endif
			data = NULL;
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
#ifndef NOSOUND
		}
#endif
		*length = SWAP_CONSTANT_32(size) - 8;
		//	printf("Sound: OK\n");
		return data;
	}
	else if(sig == MKTAG('S', 'O', 'U', 'N'))
	{
		goto digi;
	}
	else if(sig == MKTAG('L', 'E', 'C', 'F'))
	{
		printf("Sound: %c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
		*length = 0;
		*rate = 11025;
		if(dataoffset != NULL) *dataoffset = 0;
		return NULL;
	}
	else
	{
		printf("Sound: %c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
		while(1);
	}
}

void addSoundToQueue(int sound, int heOffset, int heChannel, int heFlags)
{
	//printf("sound: %d\n", sound);
	//printf("heOffset: %d\n", heOffset);
	//printf("heChannel: %d\n", heChannel);
	//printf("heFlags: %d\n", heFlags);
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

byte streambufferA[11017];
//byte streambufferB[11025];
//bool B = false;
//byte realstreambuffer[11025];
uint32_t streamoffs;
int streamslot = -1;
bool Streaming = false;
bool need = false;
bool started;
void RefreshStream()
{
	//memcpy(realstreambuffer, streambufferA, 11025);
	//B = !B;
	need = true;
	/*fpos_t pos;
	fgetpos(HE4_File, &pos);
	fseek(HE4_File, streamoffs, SEEK_SET);
	readSoundBytes(HE4_File, &streambuffer[0], 11025);
	fsetpos(HE4_File, &pos);
	streamoffs += 11025;*/
}

/*void ReadStream()
{
if(!Streaming || !need) return;
fseek(HE4_File, streamoffs, SEEK_SET);
readSoundBytes(HE4_File, &streambufferA[0], 11017);
if(!started)
{
soundEnable();
CurrentPlayingSounds[streamslot]->soundid = soundPlaySample(streambufferA, SoundFormat_8Bit, 11017, 11025, 127, 64, true, 0);
started = true;
}
streamoffs += 11017;
need = false;
}*/

void doSound()
{
	for(int i = 0; i < 16; i++)
	{
		if(CurrentPlayingSounds[i] != NULL)
		{
			if(CurrentPlayingSounds[i]->endtime <= getMillis())
			{	
				if(CurrentPlayingSounds[i]->sound == -1) 
				{
					stopTalk();
					continue;
				}
#ifndef NOSOUND
				//if(!CurrentPlayingSounds[i]->streaming)
				soundIO_StopSound(CurrentPlayingSounds[i]->soundid);
				//soundKill(CurrentPlayingSounds[i]->soundid);
				//else

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

		if(heOffset > 0 && heChannel != 2)
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
			if(heChannel == 2)//talk
			{
				printf("HE2!\n");
				printf("SoundId: %d\n", snd);
				printf("Channel: %d\n", heChannel);
				uint32_t offs = HE2_Start + heOffset;//getSoundOffset(HE0_File, &HE0_Data, snd);
				fseek(HE2_File, offs, SEEK_SET);
				uint32_t length;
				uint16_t rate;

				//while(scanKeys(), keysHeld() == 0);



				void* data = readSound(HE1_File, &length, &rate);

				int slot = findFreeSlot();

				CurrentPlayingSounds[slot]->sound = -1;
				CurrentPlayingSounds[slot]->length = length;
#ifndef NOSOUND
				CurrentPlayingSounds[slot]->data = data;
				//soundEnable();
				CurrentPlayingSounds[slot]->soundid = slot;//soundPlaySample(data, SoundFormat_8Bit, length, rate, 127, 64, false, 0);
				soundIO_PlaySound(slot, data, SoundFormat_8Bit, length, rate, 127, 64, false, 0);
#endif
				CurrentPlayingSounds[slot]->starttime = getMillis();
				CurrentPlayingSounds[slot]->endtime = CurrentPlayingSounds[slot]->starttime + (length * 1000) / rate;
			}
			else
			{
				printf("HE4!\n");
				printf("SoundId: %d\n", snd);
				printf("Channel: %d\n", heChannel);
				if(HE4_File != NULL)
				{
					fseek(HE4_File, HE4_Start + 12, SEEK_SET);
					uint32_t headerlength;
					readU32LE(HE4_File, &headerlength);
					headerlength = SWAP_CONSTANT_32(headerlength);
					uint32_t nrsongs;
					readU32LE(HE4_File, &nrsongs);
					if(headerlength == 0x28) fseek(HE4_File, HE4_Start + 0x38, SEEK_SET);
					for(int i = 0; i < nrsongs; i++)
					{
						uint32_t id;
						readU32LE(HE4_File, &id);
						//printf("id: %d\n", id);
						if(id == snd)
						{
							uint32_t offset;
							readU32LE(HE4_File, &offset);
							uint32_t length;
							uint16_t rate;
							uint32_t dataoffset;
							fseek(HE4_File, HE4_Start + offset, SEEK_SET);
							void* data = readSound(HE4_File, &length, &rate, false, &dataoffset);

							int slot;
							//if(streamslot != -1) 
							//{
							//	slot = streamslot;
							//	soundKill(CurrentPlayingSounds[slot]->soundid);
							//}
							/*else*/ slot = findFreeSlot();
							CurrentPlayingSounds[slot]->sound = snd;
							CurrentPlayingSounds[slot]->length = length;
							CurrentPlayingSounds[slot]->data = data;

							//streamslot = slot;
							//Streaming = true;
							//started = false;
							//streamoffs = dataoffset;
							//CurrentPlayingSounds[slot]->dataoffset = dataoffset;
							//CurrentPlayingSounds[slot]->streaming = true;
							//streamingslot = slot;
							//Do Streaming Start Here!
							//soundEnable();
							//CurrentPlayingSounds[slot]->soundid = soundPlaySample(streambufferA, SoundFormat_8Bit, 11025, rate, 127, 64, true, 0);
							//timerStart(2, ClockDivider_1024, TIMER_FREQ_1024(1), RefreshStream);
							soundIO_StartStream(slot, HE4_File, dataoffset, SoundFormat_8Bit, rate, 127, 64, false, 0);

							CurrentPlayingSounds[slot]->starttime = getMillis();
							CurrentPlayingSounds[slot]->endtime = CurrentPlayingSounds[slot]->starttime + (length * 1000) / rate;
							break;
						}
						else fseek(HE4_File, ((headerlength == 0x28) ? 0x11 : 0x15), SEEK_CUR);
					}
				}
			}
		}
		else if(snd > 0 && snd < getSoundCount(HE0_File, &HE0_Data))//Room Noise
		{
			uint32_t offs = getSoundOffset(HE0_File, &HE0_Data, snd);
			if(offs <= HE1_Start) continue;
			fseek(HE1_File, offs, SEEK_SET);
			uint32_t length;
			uint16_t rate;
			//printf("SoundId: %d\n", snd);

			//while(scanKeys(), keysHeld() == 0);



			void* data = readSound(HE1_File, &length, &rate);

			int slot = findFreeSlot();

			CurrentPlayingSounds[slot]->sound = snd;
			CurrentPlayingSounds[slot]->length = length;
#ifndef NOSOUND
			CurrentPlayingSounds[slot]->data = data;
			//soundEnable();
			CurrentPlayingSounds[slot]->soundid = slot;//soundPlaySample(data, SoundFormat_8Bit, length, rate, 127, 64, false, 0);
			soundIO_PlaySound(slot, data, SoundFormat_8Bit, length, rate, 127, 64, false, 0);
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
	if (id == 10000)
	{
		for(int i = 0; i < 16; i++)
		{
			if(CurrentPlayingSounds[i] != NULL)
			{
				if(CurrentPlayingSounds[i]->sound >= 8000)
				{
#ifndef NOSOUND
					//if(!CurrentPlayingSounds[i]->streaming)
					//soundKill(CurrentPlayingSounds[i]->soundid);
					soundIO_StopSound(CurrentPlayingSounds[i]->soundid);
					free(CurrentPlayingSounds[i]->data);
#endif
					free(CurrentPlayingSounds[i]);
					CurrentPlayingSounds[i] = NULL;
				}
			}
		}
		return;
	}
	for(int i = 0; i < 16; i++)
	{
		if(CurrentPlayingSounds[i] != NULL)
		{
			if(CurrentPlayingSounds[i]->sound == id)
			{
#ifndef NOSOUND
				//if(!CurrentPlayingSounds[i]->streaming)
				//soundKill(CurrentPlayingSounds[i]->soundid);
				soundIO_StopSound(CurrentPlayingSounds[i]->soundid);
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
	//printf("isSoundRunning %d\n", id);
	//if(id >= getSoundCount(HE0_File, &HE0_Data)) return 1;
	if (id == 10000)
	{
		for(int i = 0; i < 16; i++)
		{
			if(CurrentPlayingSounds[i] != NULL)
			{
				if(CurrentPlayingSounds[i]->sound >= 8000)
				{
					if(CurrentPlayingSounds[i]->endtime <= getMillis()) return 0;
					else return 1;
				}
			}
		}
		for (int i = 0; i <_soundQue2Pos; i++) 
		{
			if(_soundQue2[i].sound >= 8000) return 1;
		}
		return 0;
	}
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

int getSoundResourceSize(int snd)
{
	if(snd >= getSoundCount(HE0_File, &HE0_Data))//HE4 Music
	{
		if(HE4_File != NULL)
		{
			fseek(HE4_File, HE4_Start + 12, SEEK_SET);
			uint32_t headerlength;
			readU32LE(HE4_File, &headerlength);
			headerlength = SWAP_CONSTANT_32(headerlength);
			uint32_t nrsongs;
			readU32LE(HE4_File, &nrsongs);
			if(headerlength == 0x28) fseek(HE4_File, HE4_Start + 0x38, SEEK_SET);
			for(int i = 0; i < nrsongs; i++)
			{
				uint32_t id;
				readU32LE(HE4_File, &id);
				if(id == snd)
				{
					uint32_t offset;
					readU32LE(HE4_File, &offset);
					uint32_t length;
					uint16_t rate;
					uint32_t dataoffset;
					fseek(HE4_File, HE4_Start + offset, SEEK_SET);
					void* data = readSound(HE4_File, &length, &rate, false, &dataoffset);
					return length;
				}
				else fseek(HE4_File, ((headerlength == 0x28) ? 0x11 : 0x15), SEEK_CUR);
			}
		}
	}
	else if(snd > 0 && snd < getSoundCount(HE0_File, &HE0_Data))//Room Noise
	{
		uint32_t offs = getSoundOffset(HE0_File, &HE0_Data, snd);
		if(((int32)offs) <= 0) return 0;
		fseek(HE1_File, offs, SEEK_SET);
		uint32_t length;
		uint16_t rate;
		uint32_t dataoffset;
		void* data = readSound(HE1_File, &length, &rate, false, &dataoffset);
		return length;
	}
	else if(snd == 0) return 0;
	else
	{
		printf("SoundId: %d\n", snd);
		while(1);
	}
}