#ifndef BITMAP_H
#define BITMAP_H

int bitCounter(unsigned char myByte);
unsigned char mask(int offset);
int freeSpaceCounter(unsigned char myByte);
void setABit(unsigned char *myByte, int offset);
void clearABit(unsigned char *myByte, int offset);
int checkABit(unsigned char myByte, int offset);
int checkForConsecFreeSpace(unsigned char myByte, int count);

int getConsecFreeSpace(unsigned char* bitMap, int bitMapSize, int numOfBlocks);

#endif