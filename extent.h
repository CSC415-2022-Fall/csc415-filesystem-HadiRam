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
void addToExtentTable(extent* extentTable, int location, int count);
int getLBAFromFile(extent* extentTable, int location);
void releaseFile(int extentLocation);

#endif