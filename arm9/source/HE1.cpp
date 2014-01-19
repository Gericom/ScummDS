#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <HE1.h>
#include <ioutil.h>

void freeLFLF(LFLF_t* Room)
{
	//free(Room->RMIM->RMIH);
	//free(Room->RMIM->IM->BMAP);
	//free(Room->RMIM->IM);
	//free(Room->RMIM);


	free(Room->RMDA->PALS->WRAP->OFFS);
	free(Room->RMDA->PALS->WRAP->APAL);
	free(Room->RMDA->PALS->WRAP);
	free(Room->RMDA->PALS);

	for(int i = 0; i < Room->RMDA->RMHD.NrObjects; i++)
	{
		free(Room->RMDA->OBCD[i]);
	}

	free(Room->RMDA->OBCD);

	//free(Room->RMDA->EXCD);
	//free(Room->RMDA->ENCD);
	free(Room->RMDA->LocalScriptIds);
	free(Room->RMDA->LocalScriptOffsets);
	free(Room->RMDA->LocalScriptLengths);
	//free(Room->RMDA->RMHD);
	free(Room->RMDA);

	//free(Room);
}

void readRoom(FILE* handle, LFLF_t* LFLF, uint32_t offset)
{
	uint32_t sig = 0;
	uint32_t size = 0;
	uint16_t a = 0;
	fpos_t pos = offset;
	uint32_t secsize;

	printf("offset: 0x%x\n", offset);

	//fseek(handle, 0, SEEK_SET);
	fseek(handle, offset, SEEK_SET);
	fpos_t pos_;
	fgetpos(handle, &pos_);
	printf("0x%x\n", (uint32)pos_);
	if(!readU32LE(handle, &sig)) return;
	printf("%c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
	if(sig != MKTAG('L', 'F', 'L', 'F')) return;
	readU32LE(handle, &size);
	uint32_t end = offset + SWAP_CONSTANT_32(size);
	//LFLF_t* LFLF = (LFLF_t*)malloc(sizeof(LFLF_t));
	while(pos < end)
	{
		//fgetpos(handle, &pos);
		//printf("0x%x\n", (uint32_t)pos);
		readU32LE(handle, &sig);
		//printf("%c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
		readU32LE(handle, &size);
		switch(sig)
		{
		case MKTAG('R', 'M', 'I', 'M'):
			readU32LE(handle, &sig);
			readU32LE(handle, &size);
			readU16LE(handle, &a);
			//LFLF->RMIM = (RMIM_t*)malloc(8/* + 4 * a*/);
			//LFLF->RMIM->RMIH = (RMIH_t*)malloc(sizeof(RMIH_t));
			LFLF->RMIM.RMIH.NrZBuffer = a;

			readU32LE(handle, &sig);
			readU32LE(handle, &size);
			//LFLF->RMIM->IM = (IM_t*)malloc(sizeof(IM_t));

			readU32LE(handle, &sig);
			readU32LE(handle, &size);

			//LFLF->RMIM.IM.BMAP = (BMAP_t*)malloc(sizeof(BMAP_t));

			fgetpos(handle, &LFLF->RMIM.IM.BMAP.dataOffset);

			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);

			for(int i = 0; i < a; i++)
			{
				readU32LE(handle, &sig);
				readU32LE(handle, &size);
				fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			}
			break;
		case MKTAG('R', 'M', 'D', 'A'):
			{
				//fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
				LFLF->RMDA = (RMDA_t*)malloc(sizeof(RMDA_t));
				fgetpos(handle, &pos);
				uint32_t end2 = pos + SWAP_CONSTANT_32(size) - 8;
				int scriptidx = 0;
				int objectidx = 0;
				while(pos < (end2 - 4))
				{
					readU32LE(handle, &sig);
					//printf("%c%c%c%c\n", ((char*)&sig)[0], ((char*)&sig)[1], ((char*)&sig)[2], ((char*)&sig)[3]);
					readU32LE(handle, &size);
					switch(sig)
					{
					case MKTAG('R', 'M', 'H', 'D'):
						//LFLF->RMDA->RMHD = (RMHD_t*)malloc(sizeof(RMHD_t));
						readU16LE(handle, &LFLF->RMDA->RMHD.RoomWidth);
						readU16LE(handle, &LFLF->RMDA->RMHD.RoomHeight);
						readU16LE(handle, &LFLF->RMDA->RMHD.NrObjects);
						LFLF->RMDA->OBCD = (OBCD_t**)malloc(4 * LFLF->RMDA->RMHD.NrObjects);
						break;
					case MKTAG('C', 'Y', 'C', 'L'):
						if(SWAP_CONSTANT_32(size) == 9)	fseek(handle, 1, SEEK_CUR);
						else fseek(handle, 2, SEEK_CUR);
						break;
					case MKTAG('T', 'R', 'N', 'S'):
						if(SWAP_CONSTANT_32(size) == 9)	readByte(handle, (uint8_t*)&LFLF->RMDA->TransClrIdx);
						else readU16LE(handle, &LFLF->RMDA->TransClrIdx);							
						break;
					case MKTAG('P', 'A', 'L', 'S'):
						LFLF->RMDA->PALS = (PALS_t*)malloc(sizeof(PALS_t));
						readU32LE(handle, &sig);
						readU32LE(handle, &size);
						LFLF->RMDA->PALS->WRAP = (WRAP_t*)malloc(sizeof(WRAP_t));

						readU32LE(handle, &sig);
						readU32LE(handle, &size);
						LFLF->RMDA->PALS->WRAP->OFFS = (OFFS_t*)malloc(SWAP_CONSTANT_32(size) - 8);
						readU32sLE(handle, &LFLF->RMDA->PALS->WRAP->OFFS->Offsets[0], (SWAP_CONSTANT_32(size) - 8) / 4);

						readU32LE(handle, &sig);
						readU32LE(handle, &size);
						LFLF->RMDA->PALS->WRAP->APAL = (APAL_t*)malloc(SWAP_CONSTANT_32(size) - 8);
						readBytes(handle, &LFLF->RMDA->PALS->WRAP->APAL->data[0], SWAP_CONSTANT_32(size) - 8);
						break;	
					case MKTAG('O', 'B', 'C', 'D'):
						LFLF->RMDA->OBCD[objectidx] = (OBCD_t*)malloc(sizeof(OBCD_t));

						readU32LE(handle, &sig);
						readU32LE(handle, &size);
						fgetpos(handle, &LFLF->RMDA->OBCD[objectidx]->OCDHOffset);
						readU16LE(handle, &LFLF->RMDA->OBCD[objectidx]->ObjectId);
						uint16 x;
						readU16LE(handle, &x);
						LFLF->RMDA->OBCD[objectidx]->X = (int16)x;
						uint16 y;
						readU16LE(handle, &y);
						LFLF->RMDA->OBCD[objectidx]->Y = (int16)y;
						readU16LE(handle, &LFLF->RMDA->OBCD[objectidx]->Width);
						readU16LE(handle, &LFLF->RMDA->OBCD[objectidx]->Height);
						readByte(handle, &LFLF->RMDA->OBCD[objectidx]->Flags);
						readByte(handle, &LFLF->RMDA->OBCD[objectidx]->Parent);
						fseek(handle, SWAP_CONSTANT_32(size) - 8 - 12, SEEK_CUR);

						readU32LE(handle, &sig);
						readU32LE(handle, &size);
						LFLF->RMDA->OBCD[objectidx]->VERBLength = SWAP_CONSTANT_32(size) - 8;
						fgetpos(handle, &LFLF->RMDA->OBCD[objectidx]->VERBOffset);
						fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);

						readU32LE(handle, &sig);
						readU32LE(handle, &size);
						fgetpos(handle, &LFLF->RMDA->OBCD[objectidx]->OBNAOffset);
						fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
						objectidx++;
						break;
					case MKTAG('E', 'X', 'C', 'D'):
						//LFLF->RMDA->EXCD = malloc(SWAP_CONSTANT_32(size) - 8);
						//readBytes(handle, (uint8_t*)LFLF->RMDA->EXCD, SWAP_CONSTANT_32(size) - 8);
						fgetpos(handle, &LFLF->RMDA->EXCD);
						fseek(handle,  SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
						break;
					case MKTAG('E', 'N', 'C', 'D'):
						//LFLF->RMDA->ENCD = malloc(SWAP_CONSTANT_32(size) - 8);
						//readBytes(handle, (uint8_t*)LFLF->RMDA->ENCD, SWAP_CONSTANT_32(size) - 8);
						fgetpos(handle, &LFLF->RMDA->ENCD);
						fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
						break;
					case MKTAG('N', 'L', 'S', 'C'):
						readU16LE(handle, &LFLF->RMDA->NrLocalScripts);
						LFLF->RMDA->LocalScriptIds = (uint16_t*)malloc(sizeof(uint16_t) * LFLF->RMDA->NrLocalScripts);//new uint16_t[LFLF->RMDA->NrLocalScripts];
						LFLF->RMDA->LocalScriptOffsets = (fpos_t*)malloc(sizeof(fpos_t) * LFLF->RMDA->NrLocalScripts);//new fpos_t[LFLF->RMDA->NrLocalScripts];
						LFLF->RMDA->LocalScriptLengths = (uint32_t*)malloc(sizeof(uint32_t) * LFLF->RMDA->NrLocalScripts);//new uint32_t[LFLF->RMDA->NrLocalScripts];
						printf("Local Scripts: %d\n", LFLF->RMDA->NrLocalScripts);
						break;
					default:
						if((sig & 0xFFFFFF) == MKTAG('L', 'S', 'C', 0))
						{
							if(sig == MKTAG('L', 'S', 'C', 'R')) 
							{
								uint8_t id;
								readByte(handle, &id);
								LFLF->RMDA->LocalScriptIds[scriptidx] = id;
							}
							else 
							{
								uint16_t id;
								readU16LE(handle, &id);
								LFLF->RMDA->LocalScriptIds[scriptidx] = id;
							}
							LFLF->RMDA->LocalScriptLengths[scriptidx] = SWAP_CONSTANT_32(size) - 8 - ((sig == MKTAG('L', 'S', 'C', 'R'))?1:2);
							fgetpos(handle, &LFLF->RMDA->LocalScriptOffsets[scriptidx]);
							fseek(handle,  SWAP_CONSTANT_32(size) - 8 - ((sig == MKTAG('L', 'S', 'C', 'R'))?1:2), SEEK_CUR);
							//LFLF->RMDA->LocalScripts[scriptidx] = malloc(SWAP_CONSTANT_32(size) - 8 - ((sig == MKTAG('L', 'S', 'C', 'R'))?1:2));
							//readBytes(handle, (uint8_t*)LFLF->RMDA->LocalScripts[scriptidx], SWAP_CONSTANT_32(size) - 8 - ((sig == MKTAG('L', 'S', 'C', 'R'))?1:2));
							scriptidx++;
							break;
						}
						//printf("Unknown Signature\n");
						fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
						break;
					}
					fgetpos(handle, &pos);
				}
				break;
			}
		default:
			//printf("Unknown Signature\n");
			fseek(handle, SWAP_CONSTANT_32(size) - 8, SEEK_CUR);
			break;
		}
		fgetpos(handle, &pos);
		//printf("0x%x\n", (uint32_t)pos);
	}
	printf("OK!\n");
	return;// LFLF;
}