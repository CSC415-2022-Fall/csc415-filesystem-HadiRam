/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Names: Kimheng Peng, Hadi Ramlaoui, Manish Sharma, Rajan Shrestha
* Student IDs: 921847378, 922155413, 922220894, 922223741
* GitHub Name: TotifyKH, HadiRam, mscchapagain, rajshrestha015
* Group Name: File Explorers
* Project: Basic File System
*
* File: b_io.c
*
* Description: 
*
**************************************************************/
#ifndef VCB_H
#define _VCB_H
//Size of VCB is 24 bytes
//Volume Control Block struct

typedef struct VCB{
	//unique magic number
	long signature;

	int numBlocks;
	int blockSize;
	int freeSpace;
	//Memory Pointer allocated at runtime
	unsigned char* freeSpaceBitMap;
	int bitMapByteSize;
	int RootDir;
} VCB;

extern VCB vcb;

#endif