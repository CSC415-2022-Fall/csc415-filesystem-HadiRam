#include "extent.h"

#include <stdio.h>
#include <stdlib.h>
#include "fsLow.h"

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

void addToExtentTable(extent* extentTable, int location, int count){
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
    };
}

int getLBAFromFile(extent* extentTable, int location){
    int i = 0;
    int result;
    int index = location;
    
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

