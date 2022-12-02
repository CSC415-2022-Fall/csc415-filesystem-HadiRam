/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Names: Kimheng Peng, Hadi Ramlaoui, Manish Sharma, Rajan Shrestha
* Student IDs: 921847378, 922155413, 922220894, 922223741
* GitHub Name: TotifyKH, HadiRam, mscchapagain, rajshrestha015
* Group Name: File Explorers
* Project: Basic File System
*
* File: extent.h
*
* Description: Interface for extent table.
*
**************************************************************/
#ifndef EXTENT_H
#define EXTENT_H

#define NUMBER_OF_EXTENT 64
#define EXTENT_BLOCK_SIZE 1

typedef struct extent{
	int location;
	int count;
	
} extent;

extent* getExtentTable(int extentLocation);
void initExtentTable(int extentLocation);
int addToExtentTable(extent* extentTable, int location, int count);
int getLBAFromFile(extent* extentTable, int location);
void releaseFile(int extentLocation);
void releaseFreeBlocksExtent(extent* extentTable, int location);
void updateExtentTable(extent* extentTable, int extentLocation);
void printExtentTable(extent* extentTable);

#endif