#ifndef EXTENT_H
#define EXTENT_H

#define NUMBER_OF_EXTENT 64

typedef struct extent{
	int location;
	int count;
	
} extent;

extent* getExtentTable(int extentLocation);

#endif