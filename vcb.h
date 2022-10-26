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