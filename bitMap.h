/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Names: Kimheng Peng, Hadi Ramlaoui, Manish Sharma, Rajan Shrestha
* Student IDs: 921847378, 922155413, 922220894, 922223741
* GitHub Name: TotifyKH, HadiRam, mscchapagain, rajshrestha015
* Group Name: File Explorers
* Project: Basic File System
*
* File: bitMap.h
*
* Description: Interface for bitMap
*
**************************************************************/
#ifndef BITMAP_H
#define BITMAP_H

#define BITMAP_SIZE 5
#define BITMAP_LOCATION 1
// int bitCounter(unsigned char myByte);
// unsigned char mask(int offset);
// int freeSpaceCounter(unsigned char myByte);
// int checkABit(unsigned char myByte, int offset);
// int checkForConsecFreeSpace(unsigned char myByte, int count);

// void setABit(unsigned char* bitMap, int offset);
// void clearABit(unsigned char* bitMap, int offset);


int getConsecFreeSpace(unsigned char* bitMap, int bitMapSize, int numOfBlocks);
int releaseFreeSpace(unsigned char* bitMap, int location, int size);
void updateBitMap(unsigned char* bitMap);

#endif