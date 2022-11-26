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
#define INIT_FILE_SIZE 50

typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	//ADDED
	int flag; 		//holds the permission of the file
	dirEntry* DE; //holds the low level file info 
	extent* extentTable; //holds the extent table of the file

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
		printf("All fd in use\n");
		return -1;
	}

	//fcbArray[returnFd].DE = pi->DEPointer;

	if(flags | O_CREAT){
		if(pi->value != -1){
			printf("File does not exist\n");
			return -1;
		}

		int fileFreeSpace = getConsecFreeSpace(vcb.freeSpaceBitMap, vcb.bitMapByteSize, INIT_FILE_SIZE);

		if(fileFreeSpace == -1){
			printf("No more free space\n");
			return -1;
		}
		char* parentPath = getParentDirectory(pi->path);
		pathInfo* parentPi = parsePath(parentPath);

		dirEntry* tempDEntries = malloc(MAX_DIRENT_SIZE*sizeof(dirEntry));
		LBAread(tempDEntries, DIRECTORY_BLOCKSIZE, parentPi->DEPointer->location);

		//Find free directory Entry inside parent directory
		int index = -1;
		for(int i = 0; i < MAX_DIRENT_SIZE; i++){
			if(tempDEntries[i].dirType == -1 && index != -1){
				index = i;
			}else{
				if(strcmp(tempDEntries[i].name, filename) == 0){
					printf("Filename already exist\n");
					return -1;
				}
			}
		}

		if(index == -1){
			printf("Directory is full\n");
			return -1;
		}
		//Initialize File Directory Entry inside Parent Directory
		strcpy(tempDEntries[index].name, filename);
		tempDEntries[index].dirType = 0;
		tempDEntries[index].location = fileFreeSpace;
		tempDEntries[index].size = 0;
		time(&tempDEntries[index].created);
		time(&tempDEntries[index].lastModified);

		tempDEntries[index].extentLocation = 
			getConsecFreeSpace(vcb.freeSpaceBitMap, vcb.bitMapByteSize, EXTENT_BLOCK_SIZE);

		//Set up DEPointer in the FCB Struct
		// fcbArray[returnFd].DE = malloc(sizeof(dirEntry));
		// strcpy(fcbArray[returnFd].DE->name, tempDEntries[index].name);
		// fcbArray[returnFd].DE->dirType = tempDEntries[index].dirType;
		// fcbArray[returnFd].DE->location = tempDEntries[index].location ;
		// fcbArray[returnFd].DE->size = tempDEntries[index].size;
		// fcbArray[returnFd].DE->created = tempDEntries[index].created;
		// fcbArray[returnFd].DE->lastModified = tempDEntries[index].lastModified;
		// fcbArray[returnFd].DE->extentLocation = tempDEntries[index].extentLocation;

		initExtentTable(tempDEntries[index].extentLocation);
		extent* extentTable = getExtentTable(tempDEntries[index].extentLocation);
		//Adding the first extent
		addToExtentTable(extentTable, tempDEntries[index].location, INIT_FILE_SIZE);


		//Write to disk
		LBAwrite(extentTable, EXTENT_BLOCK_SIZE,tempDEntries[index].extentLocation);
		updateBitMap(vcb.freeSpaceBitMap);
		LBAwrite(tempDEntries, DIRECTORY_BLOCKSIZE, tempDEntries[0].location);

		// //Set up FCB
		// fcbArray[returnFd].buf = malloc(sizeof(char)*INIT_FILE_SIZE);
		// fcbArray[returnFd].index = 0;
		// fcbArray[returnFd].buflen = 0;
		// fcbArray[returnFd].flag = flags;
		// fcbArray[returnFd].extentTable = extentTable;

		free(tempDEntries);
		tempDEntries = NULL;
		 
	}

	
	
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
		
		
	return (0); //Change this
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
		
		
	return (0); //Change this
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
		
	return (0);	//Change this
	}
	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{

	}
