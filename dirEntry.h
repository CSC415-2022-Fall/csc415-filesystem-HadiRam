#ifndef DIRENTRY_H
#define DIRENTRY_H

#include <time.h>

#define DIRECTORY_BLOCKSIZE 6
#define MAX_DIRENT_SIZE 51

//Directory Entry struct with the size of 60 bytes
typedef struct dirEntry{
	char name[32];
	int location;
	int size;
	//Free state is -1 || is a file directory entry 0 || is a directory 1
	int dirType;
	time_t created;
	time_t lastModified;
	
} dirEntry;

#endif