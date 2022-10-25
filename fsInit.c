/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "fsLow.h"
#include "mfs.h"
#include "bitMap.h"

#define MAGIC_NUMBER 0xEFB112C2EFB112C1





//Helper Function
void initBitMap(char* bitMapPointer, u_int64_t blockSize){
    unsigned char tempByte = 0xFC;
	//1111 1100 Represent the first 6 blocks used by VCB and the Bitmap itself
	//Setting the first byte at index 0
    bitMapPointer[0] = 0xFC;
	//Initializing all the other bytes as free space
    for(int i = 1; i < 5*blockSize; i++){
        bitMapPointer[i] = 0x00;
    }
    LBAwrite(bitMapPointer, 5, 1);
}

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	char* vcbBlock = malloc(blockSize);
	LBAread(vcbBlock, 1, 0);
	memcpy(&vcb, vcbBlock, sizeof(VCB));
    //TODO: grab the vcb struct
	
	if(vcb.signature != MAGIC_NUMBER){
		//Initialize VCB
	
		vcb.signature = MAGIC_NUMBER;
		vcb.numBlocks = numberOfBlocks;
		vcb.blockSize = blockSize;
		//Initialize Freespace
		int bitMapBlockSize = ((numberOfBlocks + 7)/8 + (blockSize -1))/blockSize;
		vcb.freeSpaceBitMap = malloc(bitMapBlockSize*blockSize);
		vcb.bitMapByteSize = bitMapBlockSize*blockSize;
		initBitMap(vcb.freeSpaceBitMap, blockSize);
		vcb.freeSpace = 1;

		
		//Initialize RootDirectory
		//Our Directory Entry is 60 bytes long
		int numOfDirEntries = 51; //6 blocks 
		dirEntry* rootDir = malloc(numOfDirEntries*sizeof(dirEntry));
		//Setting the directory entries to their free state
		for(int i = 0; i < numOfDirEntries; i++){
			rootDir[i].name = malloc(32*sizeof(char));
			rootDir[i].isDirectory = -1; //free state
			rootDir[i].size = 0;
			rootDir[i].location = i;
		}
		int freeBlockIndex = getConsecFreeSpace(vcb.freeSpaceBitMap, vcb.bitMapByteSize, 6);
		
		//Set up the "." Directory Entry
		rootDir[0].name = ".";
		rootDir[0].size = (int) numOfDirEntries*sizeof(dirEntry);
		rootDir[0].isDirectory = 1;
		rootDir[0].location = freeBlockIndex;
		time(&rootDir[0].created);
		time(&rootDir[0].lastModified);
		//Set up the ".." Directory Entry
		rootDir[0].name = "..";
		rootDir[0].size = (int) numOfDirEntries*sizeof(dirEntry);
		rootDir[0].isDirectory = 1;
		rootDir[0].location = freeBlockIndex;
		time(&rootDir[0].created);
		time(&rootDir[0].lastModified);

		vcb.RootDir = freeBlockIndex;
		LBAwrite(rootDir, 6, freeBlockIndex);
		
	}
	
	memcpy(vcbBlock, &vcb, sizeof(VCB));
	LBAwrite(vcbBlock, 1, 0);

	

	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}