/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>


#include "b_io.h"
#include "bitMap.h"
#include "mfs.h"
#include "vcb.h"
#include "fsLow.h"
#include "extent.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512
#define INIT_FILE_SIZE 10
#define ADDITIONAL_FILE_BLOCK 50

typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	//int buflen;		//holds how many valid bytes are in the buffer
	//ADDED
	int fileOffset;   //holds the index tracking the whole file
	int fileSize;	//holds the actual file size
	int fileBlocks;
	int flag; 		//holds the permission of the file
	int directoryLocation;
	int positionInDE;
	extent* extentTable; //holds the extent table of the file
	int extentLocation;

	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
// filename = /usr/manish/desktop/test.txt
b_io_fd b_open (char * filename, int flags)
	{
	b_io_fd returnFd;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//
		
	if (startup == 0) b_init();  //Initialize our system
	
	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's

	if (returnFd < 0)
	{
		return -1;
	}
	
	pathInfo* pi = parsePath(filename);

	if(pi->value == -2){
		printf("Path is invalid\n");
		return -1;
	}
	char* lastElement = getLastPathElement(pi->path);
	
	if(flags & O_CREAT == O_CREAT && pi->value == -1){

		//Find free directory Entry inside parent directory
		int index = -1;
		for(int i = 0; i < MAX_DIRENT_SIZE; i++){
			if(cwdEntries[i].dirType == -1 && index == -1){
				index = i;
			}
		}
		
		if(index == -1){
			printf("Directory is full\n");
			return -1;
		}
		//Initialize File Directory Entry inside Parent Directory
		strcpy(cwdEntries[index].name, lastElement);
		cwdEntries[index].dirType = 0;

		//Finding the free space
		cwdEntries[index].extentLocation = 
			getConsecFreeSpace(vcb.freeSpaceBitMap, vcb.bitMapByteSize, EXTENT_BLOCK_SIZE);

		initExtentTable(cwdEntries[index].extentLocation);
		int fileFreeSpace = getConsecFreeSpace(vcb.freeSpaceBitMap, vcb.bitMapByteSize, INIT_FILE_SIZE);
		
		if(fileFreeSpace == -1 || cwdEntries[index].extentLocation == -1){
			printf("No more free space\n");
			return -1;
		}
	
		cwdEntries[index].location = fileFreeSpace;
		cwdEntries[index].size = 0;
		time(&cwdEntries[index].created);
		time(&cwdEntries[index].lastModified);

		
		extent* extentTable = getExtentTable(cwdEntries[index].extentLocation);
		//Adding the first extent
		addToExtentTable(extentTable, cwdEntries[index].location, INIT_FILE_SIZE);
		updateExtentTable(extentTable, cwdEntries[index].extentLocation);

		cwdEntries[0].size += DE_STRUCT_SIZE;
		//Update .. if its the root directory
		if(cwdEntries[0].location == cwdEntries[1].location){
			cwdEntries[1].size += DE_STRUCT_SIZE;
		}else{
			//Update Parent Directory on the size
			dirEntry* tempDEntries = malloc(MAX_DIRENT_SIZE*sizeof(dirEntry));
			LBAread(tempDEntries, DIRECTORY_BLOCKSIZE, cwdEntries[1].location);
			for(int i = 0; i < MAX_DIRENT_SIZE; i++){
				if(strcmp(getLastPathElement(cwdPath), tempDEntries[i].name) == 0){
					tempDEntries[i].size += DE_STRUCT_SIZE;
					//Exit loop
					i = MAX_DIRENT_SIZE;
				}
			}
			LBAwrite(tempDEntries, DIRECTORY_BLOCKSIZE,  cwdEntries[1].location);
		}
		//Write to disk
		LBAwrite(extentTable, EXTENT_BLOCK_SIZE,cwdEntries[index].extentLocation);
		updateBitMap(vcb.freeSpaceBitMap);
		LBAwrite(cwdEntries, DIRECTORY_BLOCKSIZE, cwdEntries[0].location);
		//Reload cwd
		LBAread(cwdEntries, DIRECTORY_BLOCKSIZE, cwdEntries[0].location);
		// //Set up FCB
		fcbArray[returnFd].buf = malloc(B_CHUNK_SIZE);
		fcbArray[returnFd].buf[0] ='\0';
		fcbArray[returnFd].index = 0;
		fcbArray[returnFd].fileOffset = 0;
		fcbArray[returnFd].fileSize = 0;
		fcbArray[returnFd].fileBlocks = INIT_FILE_SIZE;
		fcbArray[returnFd].flag = flags;
		fcbArray[returnFd].directoryLocation = cwdEntries[1].location;
		fcbArray[returnFd].positionInDE = index;
		fcbArray[returnFd].extentTable = extentTable;
		fcbArray[returnFd].extentLocation = cwdEntries[index].extentLocation;
		//printf("Extent: %d,File: %d, fileSize: %d\n",cwdEntries[index].extentLocation, cwdEntries[index].location, fcbArray[returnFd].fileSize);
		return returnFd;

	}
	

	if(pi->value >= 0){
		fcbArray[returnFd].buf = malloc(B_CHUNK_SIZE);
		fcbArray[returnFd].buf[0] ='\0';
		fcbArray[returnFd].index = 0;
		fcbArray[returnFd].fileOffset = 0;
		fcbArray[returnFd].fileSize = pi->DEPointer->size;
		fcbArray[returnFd].positionInDE = pi->value;
		fcbArray[returnFd].fileBlocks = (fcbArray[returnFd].fileSize + B_CHUNK_SIZE -1)/B_CHUNK_SIZE;

		char* parentPath = getParentDirectory(pi->path);
		pathInfo* parentPi = parsePath(parentPath);
		fcbArray[returnFd].directoryLocation = parentPi->DEPointer->location;

		fcbArray[returnFd].flag = flags;
		fcbArray[returnFd].extentTable = getExtentTable(pi->DEPointer->extentLocation);
		fcbArray[returnFd].extentLocation = pi->DEPointer->extentLocation;
		// if(flags & O_APPEND == O_APPEND){
		// 	//Pointing the index to the end
		// 	b_seek(returnFd, fcbArray[returnFd].index, SEEK_END);
		// }

		if((flags & O_TRUNC) == O_TRUNC){
			if((flags & O_WRONLY) == O_WRONLY || (flags & O_RDWR) == O_RDWR){
				//TODO empty the file
				cwdEntries[pi->value].size = 0;
				fcbArray[returnFd].fileSize = 0;
				
			}else{
				printf("No write permission to truncate file\n");
				return -1;
			}	
		}
		time(&cwdEntries[pi->value].lastModified);
		LBAwrite(cwdEntries, DIRECTORY_BLOCKSIZE, cwdEntries[0].location);	
		//Reload cwd
		LBAread(cwdEntries, DIRECTORY_BLOCKSIZE, cwdEntries[0].location);

	}else{
		printf("Error opening file! File does not exist!\n");
		return -1;
	}
	printf("Extent: %d,File: %d, Size: %d\n",pi->DEPointer->extentLocation, pi->DEPointer->location, fcbArray[returnFd].fileSize);
	printf("Extent Location: %d, %d\n", fcbArray[returnFd].extentTable[0].location, fcbArray[returnFd].extentTable[0].count );
	return (returnFd);						// all set
	
	}


// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
	
	if(whence & SEEK_SET == SEEK_SET){
		fcbArray[fd].fileOffset = offset;
	}else if((whence & SEEK_CUR) == SEEK_CUR){
		fcbArray[fd].fileOffset += offset;
	}else if(whence & SEEK_END == SEEK_END){
		fcbArray[fd].fileOffset += fcbArray[fd].fileOffset + offset;
	}else{
		printf("Invalid SEEK flags\n");
	}
		
	return fcbArray[fd].fileOffset; //Change this
	}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
	

	int neededBytes = count;
	int callerBufferOffset = 0;

	//Check if there is a write permission
	if(!(((fcbArray[fd].flag & O_WRONLY) == O_WRONLY) || 
		((fcbArray[fd].flag & O_RDWR) == O_RDWR))){
		printf("No permission to write\n");
		return -1; 
	}

	
	int remainingBytes = fcbArray[fd].fileBlocks*B_CHUNK_SIZE - fcbArray[fd].fileSize;
	//Check if we need more blocks
	if(count > remainingBytes){
		int newFileLocation = getConsecFreeSpace(vcb.freeSpaceBitMap, vcb.bitMapByteSize, ADDITIONAL_FILE_BLOCK);
		if(newFileLocation == -1){
			printf("Disk is full\n");
			return -1;
		}
		updateBitMap(vcb.freeSpaceBitMap);
		fcbArray[fd].fileBlocks += ADDITIONAL_FILE_BLOCK;
		int result = addToExtentTable(fcbArray[fd].extentTable, newFileLocation, ADDITIONAL_FILE_BLOCK);
		if(result == -1){
			printf("Out of Extent\n");
			return -1;
		}
	}

	while(neededBytes > 0){
		remainingBytes = B_CHUNK_SIZE - fcbArray[fd].index;

		if(remainingBytes > 0){
			
			int currentBlock = fcbArray[fd].fileOffset/B_CHUNK_SIZE; 
			int lbaPosition = getLBAFromFile(fcbArray[fd].extentTable, currentBlock);
			int copyAmount = neededBytes;
			if(neededBytes >= remainingBytes){
				copyAmount = remainingBytes;
			}
			LBAread(fcbArray[fd].buf, 1, lbaPosition);
			memcpy(fcbArray[fd].buf + fcbArray[fd].index, buffer + callerBufferOffset, copyAmount);

			LBAwrite(fcbArray[fd].buf, 1, lbaPosition);
			//Update variables
			fcbArray[fd].fileOffset += copyAmount;
			fcbArray[fd].index = fcbArray[fd].fileOffset % B_CHUNK_SIZE;
			fcbArray[fd].fileSize += copyAmount;
			neededBytes -= copyAmount;
			callerBufferOffset += copyAmount;
			//printf("1. %d, %d, %d\n", callerBufferOffset, fcbArray[fd].index, currentBlock);
			
		}else{
			
			int currentBlock = fcbArray[fd].fileOffset/B_CHUNK_SIZE;
			int lbaPosition = getLBAFromFile(fcbArray[fd].extentTable, currentBlock);
			int copyAmount = 0;
			if(neededBytes >= B_CHUNK_SIZE){
				copyAmount = B_CHUNK_SIZE;
			}else{
				copyAmount = neededBytes;
			}
			memcpy(fcbArray[fd].buf + fcbArray[fd].index, buffer + callerBufferOffset, copyAmount);
			LBAwrite(fcbArray[fd].buf, 1, lbaPosition);
			//Update variables
			fcbArray[fd].fileOffset += copyAmount;
			fcbArray[fd].index = fcbArray[fd].fileOffset % B_CHUNK_SIZE;
			fcbArray[fd].fileSize += copyAmount;
			neededBytes -= copyAmount;
			callerBufferOffset += copyAmount;
			//printf("2. %d, %d\n", callerBufferOffset, currentBlock);
		}
	}

	

	dirEntry* tempDEntries = malloc(MAX_DIRENT_SIZE*sizeof(dirEntry));
	LBAread(tempDEntries, DIRECTORY_BLOCKSIZE, fcbArray[fd].directoryLocation);
	tempDEntries[fcbArray[fd].positionInDE].size += callerBufferOffset;
	updateExtentTable(fcbArray[fd].extentTable,
		tempDEntries[fcbArray[fd].positionInDE].extentLocation);

	LBAwrite(tempDEntries, DIRECTORY_BLOCKSIZE, fcbArray[fd].directoryLocation);
	//Update CWD
	LBAread(cwdEntries, DIRECTORY_BLOCKSIZE, cwdEntries[0].location);
	
	free(tempDEntries);
	tempDEntries = NULL;
	
	
	return callerBufferOffset; //Change this
	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
	
	//Keeping track of the offset in the caller buffer
	int callerBufferOffset = 0;
	//Keeping track of how much we want to memcpy
	int copyAmount = 0;
	//Keeping track of the remaining size of the file
	int remainingFileSize = fcbArray[fd].fileSize - fcbArray[fd].fileOffset;
	//Used to check return value of LBAread
	int returnValue = 0;
	//Keeping track of how many bytes the caller buffer still needs
	int neededBytes = count + 1;
	//Keeping track of which block we are in
	int fileBlockIndex = fcbArray[fd].fileOffset / B_CHUNK_SIZE;
	//Rechecking the buffer index
	fcbArray[fd].index = fcbArray[fd].fileOffset % B_CHUNK_SIZE;
	//Setting neededBytes to remainingFileSize instead if neededBytes is larger
	if(neededBytes > remainingFileSize){
		neededBytes = remainingFileSize;
	}

	while(neededBytes > 0){
		//Recalculating local variables
		remainingFileSize = fcbArray[fd].fileSize - fcbArray[fd].fileOffset;

		//Will do LBAread into the caller's buffer directly
		if(fcbArray[fd].index == 0 && (neededBytes) >= B_CHUNK_SIZE){
			// Number of blocks to be copied
			int startingLocation;
			int blockNeeded = neededBytes/B_CHUNK_SIZE;
			copyAmount = blockNeeded*B_CHUNK_SIZE;
		
			//LBAread the file block directly into the caller's buffer
			while(blockNeeded > 0){
				startingLocation = getLBAFromFile(fcbArray[fd].extentTable, fileBlockIndex);
				
				returnValue = LBAread(buffer + callerBufferOffset, 1, startingLocation);
				//Erro checking
				if(returnValue < 0){
					printf("Error LBAread in b_read\n");
					return -1;
				}
				//Updating the FCB and local variables
				fileBlockIndex++;
				fcbArray[fd].fileOffset += B_CHUNK_SIZE;
				callerBufferOffset += B_CHUNK_SIZE;
			}		
		}else{
			//Check if there are any data left in the FCB Buffer
			if(fcbArray[fd].index == 0){
				//LBAread into fcb buffer
				int startingLocation = getLBAFromFile(fcbArray[fd].extentTable, fileBlockIndex);
				returnValue = LBAread(fcbArray[fd].buf, 1, startingLocation);
				//Error checking
				if(returnValue < 0){
					return -1;
				}
			}
			//Check if the neededBytes is larger than the remaining data in the 
			//fcb buffer
			if(neededBytes > B_CHUNK_SIZE - fcbArray[fd].index){
				//Setting copyAmount (For memcpy)
				copyAmount = B_CHUNK_SIZE - fcbArray[fd].index;
				fileBlockIndex++;
			}else{
				//Setting copyAmount (For memcpy)
				copyAmount = neededBytes;
			}
			//Copy data into the caller's buffer with copyAmount set earlier
			memcpy(
			buffer + callerBufferOffset, 
			fcbArray[fd].buf + fcbArray[fd].index, copyAmount);
			//Updating variables
			fcbArray[fd].fileOffset += copyAmount;
			callerBufferOffset += copyAmount;
			fcbArray[fd].index = fcbArray[fd].fileOffset % B_CHUNK_SIZE;
		}
		
		neededBytes -= copyAmount;
	}
	
	//printf("\n-----printed %d, %d, %d-----\n",fcbArray[fd].fileOffset, callerBufferOffset, count);
	return callerBufferOffset;
	}
	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{	
		if(fcbArray[fd].fileBlocks > (fcbArray[fd].fileSize + B_CHUNK_SIZE -1)/B_CHUNK_SIZE){
			int location = (fcbArray[fd].fileSize + B_CHUNK_SIZE -1)/B_CHUNK_SIZE + 1;
			int extentLocation = fcbArray[fd].extentTable[0].location;
			//printExtentTable(fcbArray[fd].extentTable);
			releaseFreeBlocksExtent(fcbArray[fd].extentTable, location);
			//printExtentTable(fcbArray[fd].extentTable);
			updateExtentTable(fcbArray[fd].extentTable, fcbArray[fd].extentLocation);
			updateBitMap(vcb.freeSpaceBitMap);
		
			
		}
		free(fcbArray[fd].buf);
		fcbArray[fd].buf = NULL;
		free(fcbArray[fd].extentTable);
		return 0;
	}
