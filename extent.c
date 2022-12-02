/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Names: Kimheng Peng, Hadi Ramlaoui, Manish Sharma, Rajan Shrestha
* Student IDs: 921847378, 922155413, 922220894, 922223741
* GitHub Name: TotifyKH, HadiRam, mscchapagain, rajshrestha015
* Group Name: File Explorers
* Project: Basic File System
*
* File: extent.c
*
* Description: Implementaion of the extent table.
*
**************************************************************/
#include "extent.h"

#include <stdio.h>
#include <stdlib.h>
#include "fsLow.h"
#include "bitMap.h"
#include "vcb.h"

//Helper Function
int getExtentTableSize(extent* extentTable){
    int size = 0;
    for(int i = 0; i < NUMBER_OF_EXTENT; i++){
        if(extentTable[i].location != -1){
            size++;
        }else{
            //break loop
            i = NUMBER_OF_EXTENT;
        }
    }
    return size;
}   

void mergeNewRow(extent* extentTable){
    int maxRow = getExtentTableSize(extentTable);
    for(int i = 0; i < maxRow - 1; i++){
        if(extentTable[i].location + extentTable[i].count == extentTable[maxRow -1].location){
            extentTable[i].count += extentTable[maxRow -1].count;
            extentTable[maxRow -1].location = -1;
            extentTable[maxRow -1].count = -1;
        }
    }

}

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
    free(extentTable);
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

    mergeNewRow(extentTable);
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

void releaseFreeBlocksExtent(extent* extentTable, int location){
    int maxRow = getExtentTableSize(extentTable);
    int lbaPosition = getLBAFromFile(extentTable, location);
    int found = 0;
    for(int i = 0; i < maxRow; i++){
        if(found == 0){
            int maxCount = extentTable[i].location + extentTable[i].count;
            for(int j = extentTable[i].location;j < maxCount; j++){
                if(lbaPosition == j){
                    found = 1;
                    extentTable[i].count = j - extentTable[i].location;
                    if(extentTable[i].count == 0){
                        extentTable[i].count = -1;
                        extentTable[i].location = -1;
                    }
                    releaseFreeSpace(vcb.freeSpaceBitMap, j, 1);
                }else if(found == 1){
                    releaseFreeSpace(vcb.freeSpaceBitMap, j, 1);
                }
            }
        }else{
            releaseFreeSpace(vcb.freeSpaceBitMap, extentTable[i].location, extentTable[i].count);
            extentTable[i].count = -1;
            extentTable[i].location = -1;
        }
    }
}

void updateExtentTable(extent* extentTable, int extentLocation){
    //printf("Overwrite %d\n", extentLocation);
    LBAwrite(extentTable, 1, extentLocation);
}

void printExtentTable(extent* extentTable){
    int maxRow = getExtentTableSize(extentTable);
    for(int i = 0; i < maxRow; i++){
        printf("Row: (%d, %d)\n",extentTable[i].location, extentTable[i].count);
    }
}


// int main(){
//     extent* ext = malloc(64*sizeof(extent));
//     for(int i = 0; i < NUMBER_OF_EXTENT; i++){
//         ext[i].location = -1;
//         ext[i].count = -1;
//     }
//     addToExtentTable(ext, 10, 4);
//     addToExtentTable(ext, 20, 4);
//     addToExtentTable(ext, 104, 2);
//     //addToExtentTable(ext, 106, 8);


//     printExtentTable(ext);
//     //releaseFreeBlocksExtent(ext, 15);
//     printf("\n\n");
//     printExtentTable(ext);
//     return 0;
// }
