#ifndef DIRENTRY_H
#define DIRENTRY_H

#include <time.h>


//Directory Entry struct with the size of 60 bytes
typedef struct dirEntry{
	char* name;
	int location;
	int size;
	//Free state is -1 || is a file directory entry 0 || is a directory 1
	int isDirectory;
	time_t created;
	time_t lastModified;
	
} dirEntry;

#endif