#include <nds.h>

#include <HE1.h>
#include <room.h>

int getObjectIndex(int object) {
	int i;

	if (object < 1)
		return -1;

	for (i = (RoomResource->RMDA->RMHD.NrObjects - 1); i > 0; i--) {
		if (RoomResource->RMDA->OBCD[i]->ObjectId == object)
			return i;
	}
	return -1;
} 