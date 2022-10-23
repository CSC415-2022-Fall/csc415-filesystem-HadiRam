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

#include "fsLow.h"
#include "mfs.h"

#define MAGIC_NUMBER 69420

//Size of VCB is 24 bytes
typedef struct VCB{
	//unique magic number
	long signature;

	int numBlocks;
	int blockSize;
	int freeSpace;
	int RootDir;
}VCB;

//Global Volume Control Block
VCB vcb;


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
		vcb.freeSpace = 1;
		//Initialize RootDirectory
		vcb.RootDir;
		
	}
	

	memcpy(vcbBlock, vcb, sizeof(VCB));
	LBAwrite(vcbBlock, 1, 0);

	/*
	[
		


	]
	*/

	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}