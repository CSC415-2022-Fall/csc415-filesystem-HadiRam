#include "extent.h"

#include <stdio.h>
#include <stdlib.h>
#include "fsLow.h"
#include "bitMap.h"
#include "vcb.h"

extent* getExtentTable(int extentLocation){
    extent* extentTable = malloc(NUMBER_OF_EXTENT*sizeof(extent));
    LBAread(extentTable, EXTENT_BLOCK_SIZE, extentLocation);
    return extentTable;
}

void initExtentTable(int extentLocation){
    extent* extentTable = malloc(NUMBER_OF_EXTENT*sizeof(extent));
    LBAread(extentTable, EXTENT_BLOCK_SIZE, extentLocation);

    for(int i = 0; i < NUMBER_OF_EXTENT; i++){
        extentTable[i].location = -1;
        extentTable[i].count = -1;
    }
    LBAwrite(extentTable, EXTENT_BLOCK_SIZE, extentLocation);
}

int addToExtentTable(extent* extentTable, int location, int count){
    int flag = 0;
    for(int i = 0; i < NUMBER_OF_EXTENT; i++){
        if(extentTable[i].location == -1){
            extentTable[i].location = location;
            extentTable[i].count = count;
            //exit loop
            i = NUMBER_OF_EXTENT;
            flag = 1;
        }
    }
    if(flag == 0){
        printf("out of row in the extent table\n");
        return -1;
    };
    return 0;
}

int getLBAFromFile(extent* extentTable, int location){
    int i = 0;
    int result;
    int index = location;
    if(index == 0){
        return extentTable[0].location;
    }
    
    while( index > 0){
        if(index > extentTable[i].count){
            index = index - extentTable[i].count;
            i++;
        }else if(index == extentTable[i].count){
            i++;
            index = 0;
            result = extentTable[i].location;
        }else{
            result = extentTable[i].location + index;
            index = 0;
        }
    }
    return result;
}

void releaseFile(int extentLocation){
    extent* extentTable = getExtentTable(extentLocation);
    for(int i = 0; i < NUMBER_OF_EXTENT; i++){
        if(extentTable[i].location != -1){
            releaseFreeSpace(vcb.freeSpaceBitMap, extentTable[i].location, extentTable[i].count);
            //printf("Extent, File: %d, %d\n",extentLocation, extentTable[i].location);
            extentTable[i].location = -1;
        }else{
            //exit loop
            i = NUMBER_OF_EXTENT;
        }
    }
    updateBitMap(vcb.freeSpaceBitMap);
}

void updateExtentTable(extent* extentTable, int extentLocation){
    //printf("Overwrite %d\n", extentLocation);
    LBAwrite(extentTable, 1, extentLocation);
}

