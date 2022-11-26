#ifndef EXTENT_H
#define EXTENT_H

typedef struct dirEntry{
	int location;
	int count;
	
} dirEntry;

int getConsecFreeSpace(unsigned char* bitMap, int bitMapSize, int numOfBlocks);
int releaseFreeSpace(unsigned char* bitMap, int location, int size);

#endif