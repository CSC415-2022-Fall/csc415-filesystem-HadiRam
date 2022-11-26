#include "extent.h"

#include <stdio.h>
#include <stdlib.h>
#include "fsLow.h"

extent* getExtentTable(int extentLocation){
    extent* extentTable = malloc(NUMBER_OF_EXTENT*sizeof(extent));
    LBAread(extentTable, 1, extentLocation);
    return extentTable;
}

int getLBAFromFile(extent* extentTable, int index){
    int i = 0;
    while( extentTable[i].count > 0){

    }
}