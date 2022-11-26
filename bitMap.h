#ifndef BITMAP_H
#define BITMAP_H

// int bitCounter(unsigned char myByte);
// unsigned char mask(int offset);
// int freeSpaceCounter(unsigned char myByte);
// int checkABit(unsigned char myByte, int offset);
// int checkForConsecFreeSpace(unsigned char myByte, int count);

// void setABit(unsigned char* bitMap, int offset);
// void clearABit(unsigned char* bitMap, int offset);


int getConsecFreeSpace(unsigned char* bitMap, int bitMapSize, int numOfBlocks);
int releaseFreeSpace(unsigned char* bitMap, int location, int size);

#endif